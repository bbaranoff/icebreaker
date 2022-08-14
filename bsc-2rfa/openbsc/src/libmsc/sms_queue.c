/* SMS queue to continously attempt to deliver SMS */
/*
 * (C) 2010 by Holger Hans Peter Freyther <zecke@selfish.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * The difficulty of such a queue is to send a lot of SMS without
 * overloading the paging subsystem and the database and other users
 * of the MSC. To make the best use we would need to know the number
 * of pending paging requests, then throttle the number of SMS we
 * want to send and such.
 * We will start with a very simple SMS Queue and then try to speed
 * things up by collecting data from other parts of the system.
 */

#include <openbsc/sms_queue.h>
#include <openbsc/chan_alloc.h>
#include <openbsc/db.h>
#include <openbsc/debug.h>
#include <openbsc/gsm_data.h>
#include <openbsc/gsm_04_11.h>
#include <openbsc/gsm_subscriber.h>
#include <openbsc/signal.h>

#include <osmocom/core/talloc.h>

#include <osmocom/vty/vty.h>

/*
 * One pending SMS that we wait for.
 */
struct gsm_sms_pending {
	struct llist_head entry;

	struct gsm_subscriber *subscr;
	unsigned long long sms_id;
	int failed_attempts;
	int resend;
};

struct gsm_sms_queue {
	struct osmo_timer_list resend_pending;
	struct osmo_timer_list push_queue;
	struct gsm_network *network;
	int max_fail;
	int max_pending;
	int pending;

	struct llist_head pending_sms;
	unsigned long long last_subscr_id;
};

static int sms_subscr_cb(unsigned int, unsigned int, void *, void *);
static int sms_sms_cb(unsigned int, unsigned int, void *, void *);

static struct gsm_sms_pending *sms_find_pending(struct gsm_sms_queue *smsq,
						struct gsm_sms *sms)
{
	struct gsm_sms_pending *pending;

	llist_for_each_entry(pending, &smsq->pending_sms, entry) {
		if (pending->sms_id == sms->id)
			return pending;
	}

	return NULL;
}

static int sms_is_in_pending(struct gsm_sms_queue *smsq, struct gsm_sms *sms)
{
	return sms_find_pending(smsq, sms) != NULL;
}

static struct gsm_sms_pending *sms_subscriber_find_pending(
					struct gsm_sms_queue *smsq,
					struct gsm_subscriber *subscr)
{
	struct gsm_sms_pending *pending;

	llist_for_each_entry(pending, &smsq->pending_sms, entry) {
		if (pending->subscr == subscr)
			return pending;
	}

	return NULL;
}

static int sms_subscriber_is_pending(struct gsm_sms_queue *smsq,
				     struct gsm_subscriber *subscr)
{
	return sms_subscriber_find_pending(smsq, subscr) != NULL;
}

static struct gsm_sms_pending *sms_pending_from(struct gsm_sms_queue *smsq,
						struct gsm_sms *sms)
{
	struct gsm_sms_pending *pending;

	pending = talloc_zero(smsq, struct gsm_sms_pending);
	if (!pending)
		return NULL;

	pending->subscr = subscr_get(sms->receiver);
	pending->sms_id = sms->id;
	return pending;
}

static void sms_pending_free(struct gsm_sms_pending *pending)
{
	subscr_put(pending->subscr);
	llist_del(&pending->entry);
	talloc_free(pending);
}

static void sms_pending_resend(struct gsm_sms_pending *pending)
{
	struct gsm_sms_queue *smsq;
	LOGP(DLSMS, LOGL_DEBUG,
	     "Scheduling resend of SMS %llu.\n", pending->sms_id);

	pending->resend = 1;

	smsq = pending->subscr->group->net->sms_queue;
	if (osmo_timer_pending(&smsq->resend_pending))
		return;

	osmo_timer_schedule(&smsq->resend_pending, 1, 0);
}

static void sms_pending_failed(struct gsm_sms_pending *pending, int paging_error)
{
	struct gsm_sms_queue *smsq;

	LOGP(DLSMS, LOGL_NOTICE, "Sending SMS %llu failed %d times.\n",
	     pending->sms_id, pending->failed_attempts);

	smsq = pending->subscr->group->net->sms_queue;
	if (++pending->failed_attempts < smsq->max_fail)
		return sms_pending_resend(pending);

	sms_pending_free(pending);
	smsq->pending -= 1;
	sms_queue_trigger(smsq);
}

/*
 * Resend all SMS that are scheduled for a resend. This is done to
 * avoid an immediate failure.
 */
static void sms_resend_pending(void *_data)
{
	struct gsm_sms_pending *pending, *tmp;
	struct gsm_sms_queue *smsq = _data;

	llist_for_each_entry_safe(pending, tmp, &smsq->pending_sms, entry) {
		struct gsm_sms *sms;
		if (!pending->resend)
			continue;

		sms = db_sms_get(smsq->network, pending->sms_id);

		/* the sms is gone? Move to the next */
		if (!sms) {
			sms_pending_free(pending);
			smsq->pending -= 1;
			sms_queue_trigger(smsq);
		} else {
			pending->resend = 0;
			gsm411_send_sms_subscr(sms->receiver, sms);
		}
	}
}

static struct gsm_sms *take_next_sms(struct gsm_sms_queue *smsq)
{
	struct gsm_sms *sms;

	sms = db_sms_get_unsent_by_subscr(smsq->network, smsq->last_subscr_id, 10);
	if (sms) {
		smsq->last_subscr_id = sms->receiver->id + 1;
		return sms;
	}

	/* need to wrap around */
	smsq->last_subscr_id = 0;
	sms = db_sms_get_unsent_by_subscr(smsq->network,
					  smsq->last_subscr_id, 10);
	if (sms)
		smsq->last_subscr_id = sms->receiver->id + 1;
	return sms;
}

/**
 * I will submit up to max_pending - pending SMS to the
 * subsystem.
 */
static void sms_submit_pending(void *_data)
{
	struct gsm_sms_queue *smsq = _data;
	int attempts = smsq->max_pending - smsq->pending;
	int initialized = 0;
	unsigned long long first_sub = 0;
	int attempted = 0, rounds = 0;

	LOGP(DLSMS, LOGL_DEBUG, "Attempting to send %d SMS\n", attempts);

	do {
		struct gsm_sms_pending *pending;
		struct gsm_sms *sms;


		sms = take_next_sms(smsq);
		if (!sms) {
			LOGP(DLSMS, LOGL_DEBUG, "Sending SMS done (%d attempted)\n",
			     attempted);
			break;
		}

		rounds += 1;
		LOGP(DLSMS, LOGL_DEBUG, "Sending SMS round %d\n", rounds);

		/*
		 * This code needs to detect a loop. It assumes that no SMS
		 * will vanish during the time this is executed. We will remember
		 * the id of the first GSM subscriber we see and then will
		 * compare this. The Database code should make sure that we will
		 * see all other subscribers first before seeing this one again.
		 *
		 * It is always scary to have an infinite loop like this.
		 */
		if (!initialized) {
			first_sub = sms->receiver->id;
			initialized = 1;
		} else if (first_sub == sms->receiver->id) {
			LOGP(DLSMS, LOGL_DEBUG, "Sending SMS done (loop) (%d attempted)\n",
			     attempted);
			sms_free(sms);
			break;
		}

		/* no need to send a pending sms */
		if (sms_is_in_pending(smsq, sms)) {
			LOGP(DLSMS, LOGL_DEBUG,
			     "SMSqueue with pending sms: %llu. Skipping\n", sms->id);
			sms_free(sms);
			continue;
		}

		/* no need to send a SMS with the same receiver */
		if (sms_subscriber_is_pending(smsq, sms->receiver)) {
			LOGP(DLSMS, LOGL_DEBUG,
			     "SMSqueue with pending sub: %llu. Skipping\n", sms->receiver->id);
			sms_free(sms);
			continue;
		}

		pending = sms_pending_from(smsq, sms);
		if (!pending) {
			LOGP(DLSMS, LOGL_ERROR,
			     "Failed to create pending SMS entry.\n");
			sms_free(sms);
			continue;
		}

		attempted += 1;
		smsq->pending += 1;
		llist_add_tail(&pending->entry, &smsq->pending_sms);
		gsm411_send_sms_subscr(sms->receiver, sms);
	} while (attempted < attempts && rounds < 1000);

	LOGP(DLSMS, LOGL_DEBUG, "SMSqueue added %d messages in %d rounds\n", attempted, rounds);
}

/**
 * Send the next SMS or trigger the queue
 */
static void sms_send_next(struct gsm_subscriber *subscr)
{
	struct gsm_sms_queue *smsq = subscr->group->net->sms_queue;
	struct gsm_sms_pending *pending;
	struct gsm_sms *sms;

	/* the subscriber should not be in the queue */
	OSMO_ASSERT(!sms_subscriber_is_pending(smsq, subscr));

	/* check for more messages for this subscriber */
	sms = db_sms_get_unsent_for_subscr(subscr);
	if (!sms)
		goto no_pending_sms;

	/* No sms should be scheduled right now */
	OSMO_ASSERT(!sms_is_in_pending(smsq, sms));

	/* Remember that we deliver this SMS and send it */
	pending = sms_pending_from(smsq, sms);
	if (!pending) {
		LOGP(DLSMS, LOGL_ERROR,
			"Failed to create pending SMS entry.\n");
		sms_free(sms);
		goto no_pending_sms;
	}

	smsq->pending += 1;
	llist_add_tail(&pending->entry, &smsq->pending_sms);
	gsm411_send_sms_subscr(sms->receiver, sms);
	return;

no_pending_sms:
	/* Try to send the SMS to avoid the queue being stuck */
	sms_submit_pending(subscr->group->net->sms_queue);
}

/*
 * Kick off the queue again.
 */
int sms_queue_trigger(struct gsm_sms_queue *smsq)
{
	LOGP(DLSMS, LOGL_DEBUG, "Triggering SMS queue\n");
	if (osmo_timer_pending(&smsq->push_queue))
		return 0;

	osmo_timer_schedule(&smsq->push_queue, 1, 0);
	return 0;
}

int sms_queue_start(struct gsm_network *network, int max_pending)
{
	struct gsm_sms_queue *sms = talloc_zero(network, struct gsm_sms_queue);
	if (!sms) {
		LOGP(DMSC, LOGL_ERROR, "Failed to create the SMS queue.\n");
		return -1;
	}

	osmo_signal_register_handler(SS_SUBSCR, sms_subscr_cb, network);
	osmo_signal_register_handler(SS_SMS, sms_sms_cb, network);

	network->sms_queue = sms;
	INIT_LLIST_HEAD(&sms->pending_sms);
	sms->max_fail = 1;
	sms->network = network;
	sms->max_pending = max_pending;
	osmo_timer_setup(&sms->push_queue, sms_submit_pending, sms);
	osmo_timer_setup(&sms->resend_pending, sms_resend_pending, sms);

	sms_submit_pending(sms);

	return 0;
}

static int sub_ready_for_sm(struct gsm_network *net, struct gsm_subscriber *subscr)
{
	struct gsm_sms *sms;
	struct gsm_sms_pending *pending;
	struct gsm_subscriber_connection *conn;

	/*
	 * The code used to be very clever and tried to submit
	 * a SMS during the Location Updating Request. This has
	 * two issues:
	 *   1.) The Phone might not be ready yet, e.g. the C155
	 *       will not respond to the Submit when it is booting.
	 *   2.) The queue is already trying to submit SMS to the
	 *	 user and by not responding to the paging request
	 *	 we will set the LAC back to 0. We would have to
	 *	 stop the paging and move things over.
	 *
	 * We need to be careful in what we try here.
	 */

	/* check if we have pending requests */
	pending = sms_subscriber_find_pending(net->sms_queue, subscr);
	if (pending) {
		LOGP(DMSC, LOGL_NOTICE,
		     "Pending paging while subscriber %llu attached.\n",
		      subscr->id);
		return 0;
	}

	conn = connection_for_subscr(subscr);
	if (!conn)
		return -1;

	/* Now try to deliver any pending SMS to this sub */
	sms = db_sms_get_unsent_for_subscr(subscr);
	if (!sms)
		return -1;
	gsm411_send_sms(conn, sms);
	return 0;
}

static int sms_subscr_cb(unsigned int subsys, unsigned int signal,
			 void *handler_data, void *signal_data)
{
	struct gsm_subscriber *subscr = signal_data;

	if (signal != S_SUBSCR_ATTACHED)
		return 0;

	/* this is readyForSM */
	return sub_ready_for_sm(handler_data, subscr);
}

static int sms_sms_cb(unsigned int subsys, unsigned int signal,
		      void *handler_data, void *signal_data)
{
	struct gsm_network *network = handler_data;
	struct sms_signal_data *sig_sms = signal_data;
	struct gsm_sms_pending *pending;
	struct gsm_subscriber *subscr;

	/* We got a new SMS and maybe should launch the queue again. */
	if (signal == S_SMS_SUBMITTED || signal == S_SMS_SMMA) {
		/* TODO: For SMMA we might want to re-use the radio connection. */
		sms_queue_trigger(network->sms_queue);
		return 0;
	}

	if (!sig_sms->sms)
		return -1;


	/*
	 * Find the entry of our queue. The SMS subsystem will submit
	 * sms that are not in our control as we just have a channel
	 * open anyway.
	 */
	pending = sms_find_pending(network->sms_queue, sig_sms->sms);
	if (!pending)
		return 0;

	switch (signal) {
	case S_SMS_DELIVERED:
		/* Remember the subscriber and clear the pending entry */
		network->sms_queue->pending -= 1;
		subscr = subscr_get(pending->subscr);
		sms_pending_free(pending);
		/* Attempt to send another SMS to this subscriber */
		sms_send_next(subscr);
		subscr_put(subscr);
		break;
	case S_SMS_MEM_EXCEEDED:
		network->sms_queue->pending -= 1;
		sms_pending_free(pending);
		sms_queue_trigger(network->sms_queue);
		break;
	case S_SMS_UNKNOWN_ERROR:
		/*
		 * There can be many reasons for this failure. E.g. the paging
		 * timed out, the subscriber was not paged at all, or there was
		 * a protocol error. The current strategy is to try sending the
		 * next SMS for busy/oom and to retransmit when we have paged.
		 *
		 * When the paging expires three times we will disable the
		 * subscriber. If we have some kind of other transmit error we
		 * should flag the SMS as bad.
		 */
		switch (sig_sms->paging_result) {
		case 0:
			/* BAD SMS? */
			db_sms_inc_deliver_attempts(sig_sms->sms);
			sms_pending_failed(pending, 0);
			break;
		case GSM_PAGING_EXPIRED:
			sms_pending_failed(pending, 1);
			break;

		case GSM_PAGING_OOM:
		case GSM_PAGING_BUSY:
			network->sms_queue->pending -= 1;
			sms_pending_free(pending);
			sms_queue_trigger(network->sms_queue);
			break;
		default:
			LOGP(DLSMS, LOGL_ERROR, "Unhandled result: %d\n",
			     sig_sms->paging_result);
		}
		break;
	default:
		LOGP(DLSMS, LOGL_ERROR, "Unhandled result: %d\n",
		     sig_sms->paging_result);
	}

	return 0;
}

/* VTY helper functions */
int sms_queue_stats(struct gsm_sms_queue *smsq, struct vty *vty)
{
	struct gsm_sms_pending *pending;

	vty_out(vty, "SMSqueue with max_pending: %d pending: %d%s",
		smsq->max_pending, smsq->pending, VTY_NEWLINE);

	llist_for_each_entry(pending, &smsq->pending_sms, entry)
		vty_out(vty, " SMS Pending for Subscriber: %llu SMS: %llu Failed: %d.%s",
			pending->subscr->id, pending->sms_id,
			pending->failed_attempts, VTY_NEWLINE);
	return 0;
}

int sms_queue_set_max_pending(struct gsm_sms_queue *smsq, int max_pending)
{
	LOGP(DLSMS, LOGL_NOTICE, "SMSqueue old max: %d new: %d\n",
	     smsq->max_pending, max_pending);
	smsq->max_pending = max_pending;
	return 0;
}

int sms_queue_set_max_failure(struct gsm_sms_queue *smsq, int max_fail)
{
	LOGP(DLSMS, LOGL_NOTICE, "SMSqueue max failure old: %d new: %d\n",
	     smsq->max_fail, max_fail);
	smsq->max_fail = max_fail;
	return 0;
}

int sms_queue_clear(struct gsm_sms_queue *smsq)
{
	struct gsm_sms_pending *pending, *tmp;

	llist_for_each_entry_safe(pending, tmp, &smsq->pending_sms, entry) {
		LOGP(DLSMS, LOGL_NOTICE,
		     "SMSqueue clearing for sub %llu\n", pending->subscr->id);
		sms_pending_free(pending);
	}

	smsq->pending = 0;
	return 0;
}
