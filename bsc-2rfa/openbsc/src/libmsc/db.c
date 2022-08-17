/* Simple HLR/VLR database backend using dbi */
/* (C) 2008 by Jan Luebbe <jluebbe@debian.org>
 * (C) 2009 by Holger Hans Peter Freyther <zecke@selfish.org>
 * (C) 2009 by Harald Welte <laforge@gnumonks.org>
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

#include <stdint.h>
#include <inttypes.h>
#include <libgen.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dbi/dbi.h>

#include <openbsc/gsm_data.h>
#include <openbsc/gsm_subscriber.h>
#include <openbsc/gsm_04_11.h>
#include <openbsc/db.h>
#include <openbsc/debug.h>

#include <osmocom/gsm/protocol/gsm_23_003.h>
#include <osmocom/core/talloc.h>
#include <osmocom/core/statistics.h>
#include <osmocom/core/rate_ctr.h>
#include <osmocom/core/utils.h>

/* Semi-Private-Interface (SPI) for the subscriber code */
void subscr_direct_free(struct gsm_subscriber *subscr);

static char *db_basename = NULL;
static char *db_dirname = NULL;
static dbi_conn conn;

#define SCHEMA_REVISION "5"

enum {
	SCHEMA_META,
	INSERT_META,
	SCHEMA_SUBSCRIBER,
	SCHEMA_AUTH,
	SCHEMA_EQUIPMENT,
	SCHEMA_EQUIPMENT_WATCH,
	SCHEMA_SMS,
	SCHEMA_VLR,
	SCHEMA_APDU,
	SCHEMA_COUNTERS,
	SCHEMA_RATE,
	SCHEMA_AUTHKEY,
	SCHEMA_AUTHLAST,
};

static const char *create_stmts[] = {
	[SCHEMA_META] = "CREATE TABLE IF NOT EXISTS Meta ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"key TEXT UNIQUE NOT NULL, "
		"value TEXT NOT NULL"
		")",
	[INSERT_META] = "INSERT OR IGNORE INTO Meta "
		"(key, value) "
		"VALUES "
		"('revision', " SCHEMA_REVISION ")",
	[SCHEMA_SUBSCRIBER] = "CREATE TABLE IF NOT EXISTS Subscriber ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"created TIMESTAMP NOT NULL, "
		"updated TIMESTAMP NOT NULL, "
		"imsi NUMERIC UNIQUE NOT NULL, "
		"name TEXT, "
		"extension TEXT UNIQUE, "
		"authorized INTEGER NOT NULL DEFAULT 0, "
		"tmsi TEXT UNIQUE, "
		"lac INTEGER NOT NULL DEFAULT 0, "
		"expire_lu TIMESTAMP DEFAULT NULL"
		")",
	[SCHEMA_AUTH] = "CREATE TABLE IF NOT EXISTS AuthToken ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"subscriber_id INTEGER UNIQUE NOT NULL, "
		"created TIMESTAMP NOT NULL, "
		"token TEXT UNIQUE NOT NULL"
		")",
	[SCHEMA_EQUIPMENT] = "CREATE TABLE IF NOT EXISTS Equipment ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"created TIMESTAMP NOT NULL, "
		"updated TIMESTAMP NOT NULL, "
		"name TEXT, "
		"classmark1 NUMERIC, "
		"classmark2 BLOB, "
		"classmark3 BLOB, "
		"imei NUMERIC UNIQUE NOT NULL"
		")",
	[SCHEMA_EQUIPMENT_WATCH] = "CREATE TABLE IF NOT EXISTS EquipmentWatch ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"created TIMESTAMP NOT NULL, "
		"updated TIMESTAMP NOT NULL, "
		"subscriber_id NUMERIC NOT NULL, "
		"equipment_id NUMERIC NOT NULL, "
		"UNIQUE (subscriber_id, equipment_id) "
		")",
	[SCHEMA_SMS] = "CREATE TABLE IF NOT EXISTS SMS ("
		/* metadata, not part of sms */
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"created TIMESTAMP NOT NULL, "
		"sent TIMESTAMP, "
		"deliver_attempts INTEGER NOT NULL DEFAULT 0, "
		/* data directly copied/derived from SMS */
		"valid_until TIMESTAMP, "
		"reply_path_req INTEGER NOT NULL, "
		"status_rep_req INTEGER NOT NULL, "
		"is_report INTEGER NOT NULL, "
		"msg_ref INTEGER NOT NULL, "
		"protocol_id INTEGER NOT NULL, "
		"data_coding_scheme INTEGER NOT NULL, "
		"ud_hdr_ind INTEGER NOT NULL, "
		"src_addr TEXT NOT NULL, "
		"src_ton INTEGER NOT NULL, "
		"src_npi INTEGER NOT NULL, "
		"dest_addr TEXT NOT NULL, "
		"dest_ton INTEGER NOT NULL, "
		"dest_npi INTEGER NOT NULL, "
		"user_data BLOB, "	/* TP-UD */
		/* additional data, interpreted from SMS */
		"header BLOB, "		/* UD Header */
		"text TEXT "		/* decoded UD after UDH */
		")",
	[SCHEMA_VLR] = "CREATE TABLE IF NOT EXISTS VLR ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"created TIMESTAMP NOT NULL, "
		"updated TIMESTAMP NOT NULL, "
		"subscriber_id NUMERIC UNIQUE NOT NULL, "
		"last_bts NUMERIC NOT NULL "
		")",
	[SCHEMA_APDU] = "CREATE TABLE IF NOT EXISTS ApduBlobs ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"created TIMESTAMP NOT NULL, "
		"apdu_id_flags INTEGER NOT NULL, "
		"subscriber_id INTEGER NOT NULL, "
		"apdu BLOB "
		")",
	[SCHEMA_COUNTERS] = "CREATE TABLE IF NOT EXISTS Counters ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"timestamp TIMESTAMP NOT NULL, "
		"value INTEGER NOT NULL, "
		"name TEXT NOT NULL "
		")",
	[SCHEMA_RATE] = "CREATE TABLE IF NOT EXISTS RateCounters ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"timestamp TIMESTAMP NOT NULL, "
		"value INTEGER NOT NULL, "
		"name TEXT NOT NULL, "
		"idx INTEGER NOT NULL "
		")",
	[SCHEMA_AUTHKEY] = "CREATE TABLE IF NOT EXISTS AuthKeys ("
		"subscriber_id INTEGER PRIMARY KEY, "
		"algorithm_id INTEGER NOT NULL, "
		"a3a8_ki BLOB "
		")",
	[SCHEMA_AUTHLAST] = "CREATE TABLE IF NOT EXISTS AuthLastTuples ("
		"subscriber_id INTEGER PRIMARY KEY, "
		"issued TIMESTAMP NOT NULL, "
		"use_count INTEGER NOT NULL DEFAULT 0, "
		"key_seq INTEGER NOT NULL, "
		"rand BLOB NOT NULL, "
		"sres BLOB NOT NULL, "
		"kc BLOB NOT NULL "
		")",
};

static inline int next_row(dbi_result result)
{
	if (!dbi_result_has_next_row(result))
		return 0;
	return dbi_result_next_row(result);
}

void db_error_func(dbi_conn conn, void *data)
{
	const char *msg;
	dbi_conn_error(conn, &msg);
	LOGP(DDB, LOGL_ERROR, "DBI: %s\n", msg);
	osmo_log_backtrace(DDB, LOGL_ERROR);
}

static int update_db_revision_2(void)
{
	dbi_result result;

	result = dbi_conn_query(conn,
				"ALTER TABLE Subscriber "
				"ADD COLUMN expire_lu "
				"TIMESTAMP DEFAULT NULL");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to alter table Subscriber (upgrade from rev 2).\n");
		return -EINVAL;
	}
	dbi_result_free(result);

	result = dbi_conn_query(conn,
				"UPDATE Meta "
				"SET value = '3' "
				"WHERE key = 'revision'");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to update DB schema revision  (upgrade from rev 2).\n");
		return -EINVAL;
	}
	dbi_result_free(result);

	return 0;
}

/**
 * Copied from the normal sms_from_result_v3 to avoid having
 * to make sure that the real routine will remain backward
 * compatible.
 */
static struct gsm_sms *sms_from_result_v3(dbi_result result)
{
	struct gsm_sms *sms = sms_alloc();
	long long unsigned int sender_id;
	struct gsm_subscriber *sender;
	const char *text, *daddr;
	const unsigned char *user_data;
	char buf[32];

	if (!sms)
		return NULL;

	sms->id = dbi_result_get_ulonglong(result, "id");

	sender_id = dbi_result_get_ulonglong(result, "sender_id");
	snprintf(buf, sizeof(buf), "%llu", sender_id);
	sender = db_get_subscriber(GSM_SUBSCRIBER_ID, buf);
	OSMO_ASSERT(sender);
	osmo_strlcpy(sms->src.addr, sender->extension, sizeof(sms->src.addr));
	subscr_direct_free(sender);
	sender = NULL;

	sms->reply_path_req = dbi_result_get_ulonglong(result, "reply_path_req");
	sms->status_rep_req = dbi_result_get_ulonglong(result, "status_rep_req");
	sms->ud_hdr_ind = dbi_result_get_ulonglong(result, "ud_hdr_ind");
	sms->protocol_id = dbi_result_get_ulonglong(result, "protocol_id");
	sms->data_coding_scheme = dbi_result_get_ulonglong(result,
						  "data_coding_scheme");

	daddr = dbi_result_get_string(result, "dest_addr");
	if (daddr)
		osmo_strlcpy(sms->dst.addr, daddr, sizeof(sms->dst.addr));

	sms->user_data_len = dbi_result_get_field_length(result, "user_data");
	user_data = dbi_result_get_binary(result, "user_data");
	if (sms->user_data_len > sizeof(sms->user_data))
		sms->user_data_len = (uint8_t) sizeof(sms->user_data);
	memcpy(sms->user_data, user_data, sms->user_data_len);

	text = dbi_result_get_string(result, "text");
	if (text)
		osmo_strlcpy(sms->text, text, sizeof(sms->text));
	return sms;
}

static int update_db_revision_3(void)
{
	dbi_result result;
	struct gsm_sms *sms;

	LOGP(DDB, LOGL_NOTICE, "Going to migrate from revision 3\n");

	result = dbi_conn_query(conn, "BEGIN EXCLUSIVE TRANSACTION");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to begin transaction (upgrade from rev 3)\n");
		return -EINVAL;
	}
	dbi_result_free(result);

	/* Rename old SMS table to be able create a new one */
	result = dbi_conn_query(conn, "ALTER TABLE SMS RENAME TO SMS_3");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to rename the old SMS table (upgrade from rev 3).\n");
		goto rollback;
	}
	dbi_result_free(result);

	/* Create new SMS table with all the bells and whistles! */
	result = dbi_conn_query(conn, create_stmts[SCHEMA_SMS]);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to create a new SMS table (upgrade from rev 3).\n");
		goto rollback;
	}
	dbi_result_free(result);

	/* Cycle through old messages and convert them to the new format */
	result = dbi_conn_query(conn, "SELECT * FROM SMS_3");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed fetch messages from the old SMS table (upgrade from rev 3).\n");
		goto rollback;
	}
	while (next_row(result)) {
		sms = sms_from_result_v3(result);
		if (db_sms_store(sms) != 0) {
			LOGP(DDB, LOGL_ERROR, "Failed to store message to the new SMS table(upgrade from rev 3).\n");
			sms_free(sms);
			dbi_result_free(result);
			goto rollback;
		}
		sms_free(sms);
	}
	dbi_result_free(result);

	/* Remove the temporary table */
	result = dbi_conn_query(conn, "DROP TABLE SMS_3");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to drop the old SMS table (upgrade from rev 3).\n");
		goto rollback;
	}
	dbi_result_free(result);

	/* We're done. Bump DB Meta revision to 4 */
	result = dbi_conn_query(conn,
				"UPDATE Meta "
				"SET value = '4' "
				"WHERE key = 'revision'");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to update DB schema revision (upgrade from rev 3).\n");
		goto rollback;
	}
	dbi_result_free(result);

	result = dbi_conn_query(conn, "COMMIT TRANSACTION");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to commit the transaction (upgrade from rev 3)\n");
		return -EINVAL;
	} else {
		dbi_result_free(result);
	}

	/* Shrink DB file size by actually wiping out SMS_3 table data */
	result = dbi_conn_query(conn, "VACUUM");
	if (!result)
		LOGP(DDB, LOGL_ERROR,
			"VACUUM failed. Ignoring it (upgrade from rev 3).\n");
	else
		dbi_result_free(result);

	return 0;

rollback:
	result = dbi_conn_query(conn, "ROLLBACK TRANSACTION");
	if (!result)
		LOGP(DDB, LOGL_ERROR,
			"Rollback failed (upgrade from rev 3).\n");
	else
		dbi_result_free(result);
	return -EINVAL;
}

/* Just like v3, but there is a new message reference field for status reports,
 * that is set to zero for existing entries since there is no way we can infer
 * this.
 */
static struct gsm_sms *sms_from_result_v4(dbi_result result)
{
	struct gsm_sms *sms = sms_alloc();
	const unsigned char *user_data;
	const char *text, *addr;

	if (!sms)
		return NULL;

	sms->id = dbi_result_get_ulonglong(result, "id");

	sms->reply_path_req = dbi_result_get_ulonglong(result, "reply_path_req");
	sms->status_rep_req = dbi_result_get_ulonglong(result, "status_rep_req");
	sms->ud_hdr_ind = dbi_result_get_ulonglong(result, "ud_hdr_ind");
	sms->protocol_id = dbi_result_get_ulonglong(result, "protocol_id");
	sms->data_coding_scheme = dbi_result_get_ulonglong(result,
						  "data_coding_scheme");

	addr = dbi_result_get_string(result, "src_addr");
	osmo_strlcpy(sms->src.addr, addr, sizeof(sms->src.addr));
	sms->src.ton = dbi_result_get_ulonglong(result, "src_ton");
	sms->src.npi = dbi_result_get_ulonglong(result, "src_npi");

	addr = dbi_result_get_string(result, "dest_addr");
	osmo_strlcpy(sms->dst.addr, addr, sizeof(sms->dst.addr));
	sms->dst.ton = dbi_result_get_ulonglong(result, "dest_ton");
	sms->dst.npi = dbi_result_get_ulonglong(result, "dest_npi");

	sms->user_data_len = dbi_result_get_field_length(result, "user_data");
	user_data = dbi_result_get_binary(result, "user_data");
	if (sms->user_data_len > sizeof(sms->user_data))
		sms->user_data_len = (uint8_t) sizeof(sms->user_data);
	memcpy(sms->user_data, user_data, sms->user_data_len);

	text = dbi_result_get_string(result, "text");
	if (text)
		osmo_strlcpy(sms->text, text, sizeof(sms->text));
	return sms;
}

static int update_db_revision_4(void)
{
	dbi_result result;
	struct gsm_sms *sms;

	LOGP(DDB, LOGL_NOTICE, "Going to migrate from revision 4\n");

	result = dbi_conn_query(conn, "BEGIN EXCLUSIVE TRANSACTION");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to begin transaction (upgrade from rev 4)\n");
		return -EINVAL;
	}
	dbi_result_free(result);

	/* Rename old SMS table to be able create a new one */
	result = dbi_conn_query(conn, "ALTER TABLE SMS RENAME TO SMS_4");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to rename the old SMS table (upgrade from rev 4).\n");
		goto rollback;
	}
	dbi_result_free(result);

	/* Create new SMS table with all the bells and whistles! */
	result = dbi_conn_query(conn, create_stmts[SCHEMA_SMS]);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to create a new SMS table (upgrade from rev 4).\n");
		goto rollback;
	}
	dbi_result_free(result);

	/* Cycle through old messages and convert them to the new format */
	result = dbi_conn_query(conn, "SELECT * FROM SMS_4");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed fetch messages from the old SMS table (upgrade from rev 4).\n");
		goto rollback;
	}
	while (next_row(result)) {
		sms = sms_from_result_v4(result);
		if (db_sms_store(sms) != 0) {
			LOGP(DDB, LOGL_ERROR, "Failed to store message to the new SMS table(upgrade from rev 4).\n");
			sms_free(sms);
			dbi_result_free(result);
			goto rollback;
		}
		sms_free(sms);
	}
	dbi_result_free(result);

	/* Remove the temporary table */
	result = dbi_conn_query(conn, "DROP TABLE SMS_4");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to drop the old SMS table (upgrade from rev 4).\n");
		goto rollback;
	}
	dbi_result_free(result);

	/* We're done. Bump DB Meta revision to 4 */
	result = dbi_conn_query(conn,
				"UPDATE Meta "
				"SET value = '5' "
				"WHERE key = 'revision'");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
		     "Failed to update DB schema revision (upgrade from rev 4).\n");
		goto rollback;
	}
	dbi_result_free(result);

	result = dbi_conn_query(conn, "COMMIT TRANSACTION");
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to commit the transaction (upgrade from rev 4)\n");
		return -EINVAL;
	} else {
		dbi_result_free(result);
	}

	/* Shrink DB file size by actually wiping out SMS_4 table data */
	result = dbi_conn_query(conn, "VACUUM");
	if (!result)
		LOGP(DDB, LOGL_ERROR,
			"VACUUM failed. Ignoring it (upgrade from rev 4).\n");
	else
		dbi_result_free(result);

	return 0;

rollback:
	result = dbi_conn_query(conn, "ROLLBACK TRANSACTION");
	if (!result)
		LOGP(DDB, LOGL_ERROR,
			"Rollback failed (upgrade from rev 4).\n");
	else
		dbi_result_free(result);
	return -EINVAL;
}

static int check_db_revision(void)
{
	dbi_result result;
	const char *rev_s;
	int db_rev = 0;

	/* Make a query */
	result = dbi_conn_query(conn,
		"SELECT value FROM Meta "
		"WHERE key = 'revision'");

	if (!result)
		return -EINVAL;

	if (!next_row(result)) {
		dbi_result_free(result);
		return -EINVAL;
	}

	/* Fetch the DB schema revision */
	rev_s = dbi_result_get_string(result, "value");
	if (!rev_s) {
		dbi_result_free(result);
		return -EINVAL;
	}

	if (!strcmp(rev_s, SCHEMA_REVISION)) {
		/* Everything is fine */
		dbi_result_free(result);
		return 0;
	}

	db_rev = atoi(rev_s);
	dbi_result_free(result);

	/* Incremental migration waterfall */
	switch (db_rev) {
	case 2:
		if (update_db_revision_2())
			goto error;
	case 3:
		if (update_db_revision_3())
			goto error;
	case 4:
		if (update_db_revision_4())
			goto error;

	/* The end of waterfall */
	break;
	default:
		LOGP(DDB, LOGL_FATAL,
			"Invalid database schema revision '%d'.\n", db_rev);
		return -EINVAL;
	}

	return 0;

error:
	LOGP(DDB, LOGL_FATAL, "Failed to update database "
		"from schema revision '%d'.\n", db_rev);
	return -EINVAL;
}

static int db_configure(void)
{
	dbi_result result;

	result = dbi_conn_query(conn,
				"PRAGMA synchronous = FULL");
	if (!result)
		return -EINVAL;

	dbi_result_free(result);
	return 0;
}

int db_init(const char *name)
{
	dbi_initialize(NULL);

	conn = dbi_conn_new("sqlite3");
	if (conn == NULL) {
		LOGP(DDB, LOGL_FATAL, "Failed to create connection.\n");
		return 1;
	}

	dbi_conn_error_handler( conn, db_error_func, NULL );

	/* MySQL
	dbi_conn_set_option(conn, "host", "localhost");
	dbi_conn_set_option(conn, "username", "your_name");
	dbi_conn_set_option(conn, "password", "your_password");
	dbi_conn_set_option(conn, "dbname", "your_dbname");
	dbi_conn_set_option(conn, "encoding", "UTF-8");
	*/

	/* SqLite 3 */
	db_basename = strdup(name);
	db_dirname = strdup(name);
	dbi_conn_set_option(conn, "sqlite3_dbdir", dirname(db_dirname));
	dbi_conn_set_option(conn, "dbname", basename(db_basename));

	if (dbi_conn_connect(conn) < 0)
		goto out_err;

	return 0;

out_err:
	free(db_dirname);
	free(db_basename);
	db_dirname = db_basename = NULL;
	return -1;
}


int db_prepare(void)
{
	dbi_result result;
	int i;

	for (i = 0; i < ARRAY_SIZE(create_stmts); i++) {
		result = dbi_conn_query(conn, create_stmts[i]);
		if (!result) {
			LOGP(DDB, LOGL_ERROR,
			     "Failed to create some table.\n");
			return 1;
		}
		dbi_result_free(result);
	}

	if (check_db_revision() < 0) {
		LOGP(DDB, LOGL_FATAL, "Database schema revision invalid, "
			"please update your database schema\n");
                return -1;
	}

	db_configure();

	return 0;
}

int db_fini(void)
{
	dbi_conn_close(conn);
	dbi_shutdown();

	free(db_dirname);
	free(db_basename);
	return 0;
}

struct gsm_subscriber *db_create_subscriber(const char *imsi, uint64_t smin,
					    uint64_t smax, bool alloc_exten)
{
	dbi_result result;
	struct gsm_subscriber *subscr;

	/* Is this subscriber known in the db? */
	subscr = db_get_subscriber(GSM_SUBSCRIBER_IMSI, imsi);
	if (subscr) {
		subscr_put(subscr);
		return NULL;
	}

	subscr = subscr_alloc();
	if (!subscr)
		return NULL;
	subscr->flags |= GSM_SUBSCRIBER_FIRST_CONTACT;
	result = dbi_conn_queryf(conn,
		"INSERT INTO Subscriber "
		"(imsi, created, updated) "
		"VALUES "
		"(%s, datetime('now'), datetime('now')) ",
		imsi
	);
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to create Subscriber by IMSI.\n");
		subscr_put(subscr);
		return NULL;
	}
	subscr->id = dbi_conn_sequence_last(conn, NULL);
	osmo_strlcpy(subscr->imsi, imsi, sizeof(subscr->imsi));
	dbi_result_free(result);
	LOGP(DDB, LOGL_INFO, "New Subscriber: ID %llu, IMSI %s\n", subscr->id, subscr->imsi);
	if (alloc_exten)
		db_subscriber_alloc_exten(subscr, smin, smax);
	return subscr;
}

osmo_static_assert(sizeof(unsigned char) == sizeof(struct gsm48_classmark1), classmark1_size);

static int get_equipment_by_subscr(struct gsm_subscriber *subscr)
{
	dbi_result result;
	const char *string;
	unsigned char cm1;
	const unsigned char *cm2, *cm3;
	struct gsm_equipment *equip = &subscr->equipment;

	result = dbi_conn_queryf(conn,
		"SELECT Equipment.* "
			"FROM Equipment JOIN EquipmentWatch ON "
				"EquipmentWatch.equipment_id=Equipment.id "
			"WHERE EquipmentWatch.subscriber_id = %llu "
			"ORDER BY EquipmentWatch.updated DESC", subscr->id);
	if (!result)
		return -EIO;

	if (!next_row(result)) {
		dbi_result_free(result);
		return -ENOENT;
	}

	equip->id = dbi_result_get_ulonglong(result, "id");

	string = dbi_result_get_string(result, "imei");
	if (string)
		osmo_strlcpy(equip->imei, string, sizeof(equip->imei));

	string = dbi_result_get_string(result, "classmark1");
	if (string) {
		cm1 = atoi(string) & 0xff;
		memcpy(&equip->classmark1, &cm1, sizeof(equip->classmark1));
	}

	equip->classmark2_len = dbi_result_get_field_length(result, "classmark2");
	cm2 = dbi_result_get_binary(result, "classmark2");
	if (equip->classmark2_len > sizeof(equip->classmark2))
		equip->classmark2_len = sizeof(equip->classmark2);
	if (cm2)
		memcpy(equip->classmark2, cm2, equip->classmark2_len);

	equip->classmark3_len = dbi_result_get_field_length(result, "classmark3");
	cm3 = dbi_result_get_binary(result, "classmark3");
	if (equip->classmark3_len > sizeof(equip->classmark3))
		equip->classmark3_len = sizeof(equip->classmark3);
	if (cm3)
		memcpy(equip->classmark3, cm3, equip->classmark3_len);

	dbi_result_free(result);

	return 0;
}

int db_get_authinfo_for_subscr(struct gsm_auth_info *ainfo,
                               struct gsm_subscriber *subscr)
{
	dbi_result result;
	const unsigned char *a3a8_ki;

	result = dbi_conn_queryf(conn,
			"SELECT * FROM AuthKeys WHERE subscriber_id=%llu",
			 subscr->id);
	if (!result)
		return -EIO;

	if (!next_row(result)) {
		dbi_result_free(result);
		return -ENOENT;
	}

	ainfo->auth_algo = dbi_result_get_ulonglong(result, "algorithm_id");
	ainfo->a3a8_ki_len = dbi_result_get_field_length(result, "a3a8_ki");
	a3a8_ki = dbi_result_get_binary(result, "a3a8_ki");
	if (ainfo->a3a8_ki_len > sizeof(ainfo->a3a8_ki))
		ainfo->a3a8_ki_len = sizeof(ainfo->a3a8_ki);
	memcpy(ainfo->a3a8_ki, a3a8_ki, ainfo->a3a8_ki_len);

	dbi_result_free(result);

	return 0;
}

int db_sync_authinfo_for_subscr(struct gsm_auth_info *ainfo,
                                struct gsm_subscriber *subscr)
{
	dbi_result result;
	struct gsm_auth_info ainfo_old;
	int rc, upd;
	unsigned char *ki_str;

	/* Deletion ? */
	if (ainfo == NULL) {
		result = dbi_conn_queryf(conn,
			"DELETE FROM AuthKeys WHERE subscriber_id=%llu",
			subscr->id);

		if (!result)
			return -EIO;

		dbi_result_free(result);

		return 0;
	}

	/* Check if already existing */
	rc = db_get_authinfo_for_subscr(&ainfo_old, subscr);
	if (rc && rc != -ENOENT)
		return rc;
	upd = rc ? 0 : 1;

	/* Update / Insert */
	dbi_conn_quote_binary_copy(conn,
		ainfo->a3a8_ki, ainfo->a3a8_ki_len, &ki_str);

	if (!upd) {
		result = dbi_conn_queryf(conn,
				"INSERT INTO AuthKeys "
				"(subscriber_id, algorithm_id, a3a8_ki) "
				"VALUES (%llu, %u, %s)",
				subscr->id, ainfo->auth_algo, ki_str);
	} else {
		result = dbi_conn_queryf(conn,
				"UPDATE AuthKeys "
				"SET algorithm_id=%u, a3a8_ki=%s "
				"WHERE subscriber_id=%llu",
				ainfo->auth_algo, ki_str, subscr->id);
	}

	free(ki_str);

	if (!result)
		return -EIO;

	dbi_result_free(result);

	return 0;
}

int db_get_lastauthtuple_for_subscr(struct gsm_auth_tuple *atuple,
                                    struct gsm_subscriber *subscr)
{
	dbi_result result;
	int len;
	const unsigned char *blob;

	result = dbi_conn_queryf(conn,
			"SELECT * FROM AuthLastTuples WHERE subscriber_id=%llu",
			subscr->id);
	if (!result)
		return -EIO;

	if (!next_row(result)) {
		dbi_result_free(result);
		return -ENOENT;
	}

	memset(atuple, 0, sizeof(*atuple));

	atuple->use_count = dbi_result_get_ulonglong(result, "use_count");
	atuple->key_seq = dbi_result_get_ulonglong(result, "key_seq");

	len = dbi_result_get_field_length(result, "rand");
	if (len != sizeof(atuple->vec.rand))
		goto err_size;

	blob = dbi_result_get_binary(result, "rand");
	memcpy(atuple->vec.rand, blob, len);

	len = dbi_result_get_field_length(result, "sres");
	if (len != sizeof(atuple->vec.sres))
		goto err_size;

	blob = dbi_result_get_binary(result, "sres");
	memcpy(atuple->vec.sres, blob, len);

	len = dbi_result_get_field_length(result, "kc");
	if (len != sizeof(atuple->vec.kc))
		goto err_size;

	blob = dbi_result_get_binary(result, "kc");
	memcpy(atuple->vec.kc, blob, len);

	dbi_result_free(result);

	return 0;

err_size:
	dbi_result_free(result);
	return -EIO;
}

int db_sync_lastauthtuple_for_subscr(struct gsm_auth_tuple *atuple,
                                     struct gsm_subscriber *subscr)
{
	dbi_result result;
	int rc, upd;
	struct gsm_auth_tuple atuple_old;
	unsigned char *rand_str, *sres_str, *kc_str;

	/* Deletion ? */
	if (atuple == NULL) {
		result = dbi_conn_queryf(conn,
			"DELETE FROM AuthLastTuples WHERE subscriber_id=%llu",
			subscr->id);

		if (!result)
			return -EIO;

		dbi_result_free(result);

		return 0;
	}

	/* Check if already existing */
	rc = db_get_lastauthtuple_for_subscr(&atuple_old, subscr);
	if (rc && rc != -ENOENT)
		return rc;
	upd = rc ? 0 : 1;

	/* Update / Insert */
	dbi_conn_quote_binary_copy(conn,
		atuple->vec.rand, sizeof(atuple->vec.rand), &rand_str);
	dbi_conn_quote_binary_copy(conn,
		atuple->vec.sres, sizeof(atuple->vec.sres), &sres_str);
	dbi_conn_quote_binary_copy(conn,
		atuple->vec.kc, sizeof(atuple->vec.kc), &kc_str);

	if (!upd) {
		result = dbi_conn_queryf(conn,
				"INSERT INTO AuthLastTuples "
				"(subscriber_id, issued, use_count, "
				 "key_seq, rand, sres, kc) "
				"VALUES (%llu, datetime('now'), %u, "
				 "%u, %s, %s, %s ) ",
				subscr->id, atuple->use_count, atuple->key_seq,
				rand_str, sres_str, kc_str);
	} else {
		char *issued = atuple->key_seq == atuple_old.key_seq ?
					"issued" : "datetime('now')";
		result = dbi_conn_queryf(conn,
				"UPDATE AuthLastTuples "
				"SET issued=%s, use_count=%u, "
				 "key_seq=%u, rand=%s, sres=%s, kc=%s "
				"WHERE subscriber_id = %llu",
				issued, atuple->use_count, atuple->key_seq,
				rand_str, sres_str, kc_str, subscr->id);
	}

	free(rand_str);
	free(sres_str);
	free(kc_str);

	if (!result)
		return -EIO;

	dbi_result_free(result);

	return 0;
}

static void db_set_from_query(struct gsm_subscriber *subscr, dbi_conn result)
{
	const char *string;
	string = dbi_result_get_string(result, "imsi");
	if (string)
		osmo_strlcpy(subscr->imsi, string, sizeof(subscr->imsi));

	string = dbi_result_get_string(result, "tmsi");
	if (string)
		subscr->tmsi = tmsi_from_string(string);

	string = dbi_result_get_string(result, "name");
	if (string)
		osmo_strlcpy(subscr->name, string, sizeof(subscr->name));

	string = dbi_result_get_string(result, "extension");
	if (string)
		osmo_strlcpy(subscr->extension, string, sizeof(subscr->extension));

	subscr->lac = dbi_result_get_ulonglong(result, "lac");

	if (!dbi_result_field_is_null(result, "expire_lu"))
		subscr->expire_lu = dbi_result_get_datetime(result, "expire_lu");
	else
		subscr->expire_lu = GSM_SUBSCRIBER_NO_EXPIRATION;

	subscr->authorized = dbi_result_get_ulonglong(result, "authorized");

}

#define BASE_QUERY "SELECT * FROM Subscriber "
struct gsm_subscriber *db_get_subscriber(enum gsm_subscriber_field field,
					 const char *id)
{
	dbi_result result;
	char *quoted;
	struct gsm_subscriber *subscr;

	switch (field) {
	case GSM_SUBSCRIBER_IMSI:
		dbi_conn_quote_string_copy(conn, id, &quoted);
		result = dbi_conn_queryf(conn,
			BASE_QUERY
			"WHERE imsi = %s ",
			quoted
		);
		free(quoted);
		break;
	case GSM_SUBSCRIBER_TMSI:
		dbi_conn_quote_string_copy(conn, id, &quoted);
		result = dbi_conn_queryf(conn,
			BASE_QUERY
			"WHERE tmsi = %s ",
			quoted
		);
		free(quoted);
		break;
	case GSM_SUBSCRIBER_EXTENSION:
		dbi_conn_quote_string_copy(conn, id, &quoted);
		result = dbi_conn_queryf(conn,
			BASE_QUERY
			"WHERE extension = %s ",
			quoted
		);
		free(quoted);
		break;
	case GSM_SUBSCRIBER_ID:
		dbi_conn_quote_string_copy(conn, id, &quoted);
		result = dbi_conn_queryf(conn,
			BASE_QUERY
			"WHERE id = %s ", quoted);
		free(quoted);
		break;
	default:
		LOGP(DDB, LOGL_NOTICE, "Unknown query selector for Subscriber.\n");
		return NULL;
	}
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to query Subscriber.\n");
		return NULL;
	}
	if (!next_row(result)) {
		DEBUGP(DDB, "Failed to find the Subscriber. '%u' '%s'\n",
			field, id);
		dbi_result_free(result);
		return NULL;
	}

	subscr = subscr_alloc();
	subscr->id = dbi_result_get_ulonglong(result, "id");

	db_set_from_query(subscr, result);
	DEBUGP(DDB, "Found Subscriber: ID %llu, IMSI %s, NAME '%s', TMSI %x, EXTEN '%s', LAC %hu, AUTH %u\n",
		subscr->id, subscr->imsi, subscr->name, subscr->tmsi, subscr->extension,
		subscr->lac, subscr->authorized);
	dbi_result_free(result);

	get_equipment_by_subscr(subscr);

	return subscr;
}

int db_subscriber_update(struct gsm_subscriber *subscr)
{
	char buf[32];
	dbi_result result;

	/* Copy the id to a string as queryf with %llu is failing */
	sprintf(buf, "%llu", subscr->id);
	result = dbi_conn_queryf(conn,
			BASE_QUERY
			"WHERE id = %s", buf);

	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to query Subscriber: %llu\n", subscr->id);
		return -EIO;
	}
	if (!next_row(result)) {
		DEBUGP(DDB, "Failed to find the Subscriber. %llu\n",
			subscr->id);
		dbi_result_free(result);
		return -EIO;
	}

	db_set_from_query(subscr, result);
	dbi_result_free(result);
	get_equipment_by_subscr(subscr);

	return 0;
}

int db_sync_subscriber(struct gsm_subscriber *subscriber)
{
	dbi_result result;
	char tmsi[14];
	char *q_tmsi, *q_name, *q_extension;

	dbi_conn_quote_string_copy(conn, 
				   subscriber->name, &q_name);
	if (subscriber->extension[0] != '\0')
		dbi_conn_quote_string_copy(conn,
					   subscriber->extension, &q_extension);
	else
		q_extension = strdup("NULL");
	
	if (subscriber->tmsi != GSM_RESERVED_TMSI) {
		sprintf(tmsi, "%u", subscriber->tmsi);
		dbi_conn_quote_string_copy(conn,
				   tmsi,
				   &q_tmsi);
	} else
		q_tmsi = strdup("NULL");

	if (subscriber->expire_lu == GSM_SUBSCRIBER_NO_EXPIRATION) {
		result = dbi_conn_queryf(conn,
			"UPDATE Subscriber "
			"SET updated = datetime('now'), "
			"name = %s, "
			"extension = %s, "
			"authorized = %i, "
			"tmsi = %s, "
			"lac = %i, "
			"expire_lu = NULL "
			"WHERE imsi = %s ",
			q_name,
			q_extension,
			subscriber->authorized,
			q_tmsi,
			subscriber->lac,
			subscriber->imsi);
	} else {
		result = dbi_conn_queryf(conn,
			"UPDATE Subscriber "
			"SET updated = datetime('now'), "
			"name = %s, "
			"extension = %s, "
			"authorized = %i, "
			"tmsi = %s, "
			"lac = %i, "
			"expire_lu = datetime(%i, 'unixepoch') "
			"WHERE imsi = %s ",
			q_name,
			q_extension,
			subscriber->authorized,
			q_tmsi,
			subscriber->lac,
			(int) subscriber->expire_lu,
			subscriber->imsi);
	}

	free(q_tmsi);
	free(q_name);
	free(q_extension);

	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to update Subscriber (by IMSI).\n");
		return 1;
	}

	dbi_result_free(result);

	return 0;
}

int db_subscriber_delete(struct gsm_subscriber *subscr)
{
	dbi_result result;

	result = dbi_conn_queryf(conn,
			"DELETE FROM AuthKeys WHERE subscriber_id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete Authkeys for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	result = dbi_conn_queryf(conn,
			"DELETE FROM AuthLastTuples WHERE subscriber_id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete AuthLastTuples for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	result = dbi_conn_queryf(conn,
			"DELETE FROM AuthToken WHERE subscriber_id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete AuthToken for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	result = dbi_conn_queryf(conn,
			"DELETE FROM EquipmentWatch WHERE subscriber_id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete EquipmentWatch for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	if (subscr->extension[0] != '\0') {
		result = dbi_conn_queryf(conn,
			    "DELETE FROM SMS WHERE src_addr=%s OR dest_addr=%s",
					 subscr->extension, subscr->extension);
		if (!result) {
			LOGP(DDB, LOGL_ERROR,
			     "Failed to delete SMS for %llu\n", subscr->id);
			return -1;
		}
		dbi_result_free(result);
	}

	result = dbi_conn_queryf(conn,
			"DELETE FROM VLR WHERE subscriber_id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete VLR for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	result = dbi_conn_queryf(conn,
			"DELETE FROM ApduBlobs WHERE subscriber_id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete ApduBlobs for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	result = dbi_conn_queryf(conn,
			"DELETE FROM Subscriber WHERE id=%llu",
			subscr->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR,
			"Failed to delete Subscriber for %llu\n", subscr->id);
		return -1;
	}
	dbi_result_free(result);

	return 0;
}

/**
 * List all the authorized and non-expired subscribers. The callback will
 * be called one by one. The subscr argument is not fully initialize and
 * subscr_get/subscr_put must not be called. The passed in pointer will be
 * deleted after the callback by the database call.
 */
int db_subscriber_list_active(void (*cb)(struct gsm_subscriber*,void*), void *closure)
{
	dbi_result result;

	result = dbi_conn_query(conn,
		       "SELECT * from Subscriber WHERE LAC != 0 AND authorized = 1");
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to list active subscribers\n");
		return -1;
	}

	while (next_row(result)) {
		struct gsm_subscriber *subscr;

		subscr = subscr_alloc();
		subscr->id = dbi_result_get_ulonglong(result, "id");
		db_set_from_query(subscr, result);
		cb(subscr, closure);
		OSMO_ASSERT(subscr->use_count == 1);
		llist_del(&subscr->entry);
		talloc_free(subscr);
	}

	dbi_result_free(result);
	return 0;
}

int db_sync_equipment(struct gsm_equipment *equip)
{
	dbi_result result;
	unsigned char *cm2, *cm3;
	char *q_imei;
	uint8_t classmark1;

	memcpy(&classmark1, &equip->classmark1, sizeof(classmark1));
	DEBUGP(DDB, "Sync Equipment IMEI=%s, classmark1=%02x",
		equip->imei, classmark1);
	if (equip->classmark2_len)
		DEBUGPC(DDB, ", classmark2=%s",
			osmo_hexdump(equip->classmark2, equip->classmark2_len));
	if (equip->classmark3_len)
		DEBUGPC(DDB, ", classmark3=%s",
			osmo_hexdump(equip->classmark3, equip->classmark3_len));
	DEBUGPC(DDB, "\n");

	dbi_conn_quote_binary_copy(conn, equip->classmark2,
				   equip->classmark2_len, &cm2);
	dbi_conn_quote_binary_copy(conn, equip->classmark3,
				   equip->classmark3_len, &cm3);
	dbi_conn_quote_string_copy(conn, equip->imei, &q_imei);

	result = dbi_conn_queryf(conn,
		"UPDATE Equipment SET "
			"updated = datetime('now'), "
			"classmark1 = %u, "
			"classmark2 = %s, "
			"classmark3 = %s "
		"WHERE imei = %s ",
		classmark1, cm2, cm3, q_imei);

	free(cm2);
	free(cm3);
	free(q_imei);

	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to update Equipment\n");
		return -EIO;
	}

	dbi_result_free(result);
	return 0;
}

int db_subscriber_expire(void *priv, void (*callback)(void *priv, long long unsigned int id))
{
	dbi_result result;

	result = dbi_conn_query(conn,
			"SELECT id "
			"FROM Subscriber "
			"WHERE lac != 0 AND "
				"( expire_lu is NOT NULL "
				"AND expire_lu < datetime('now') ) "
			"LIMIT 1");
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to get expired subscribers\n");
		return -EIO;
	}

	while (next_row(result))
		callback(priv, dbi_result_get_ulonglong(result, "id"));

	dbi_result_free(result);
	return 0;
}

int db_subscriber_alloc_tmsi(struct gsm_subscriber *subscriber)
{
	dbi_result result = NULL;
	char tmsi[14];
	char *tmsi_quoted;

	for (;;) {
		int rc = osmo_get_rand_id((uint8_t *) &subscriber->tmsi, sizeof(subscriber->tmsi));
		if (rc < 0) {
			LOGP(DDB, LOGL_ERROR, "osmo_get_rand_id() failed: %s\n", strerror(-rc));
			return 1;
		}
		if (subscriber->tmsi == GSM_RESERVED_TMSI)
			continue;

		sprintf(tmsi, "%u", subscriber->tmsi);
		dbi_conn_quote_string_copy(conn, tmsi, &tmsi_quoted);
		result = dbi_conn_queryf(conn,
			"SELECT * FROM Subscriber "
			"WHERE tmsi = %s ",
			tmsi_quoted);

		free(tmsi_quoted);

		if (!result) {
			LOGP(DDB, LOGL_ERROR, "Failed to query Subscriber "
				"while allocating new TMSI.\n");
			return 1;
		}
		if (dbi_result_get_numrows(result)) {
			dbi_result_free(result);
			continue;
		}
		if (!next_row(result)) {
			dbi_result_free(result);
			DEBUGP(DDB, "Allocated TMSI %u for IMSI %s.\n",
				subscriber->tmsi, subscriber->imsi);
			return db_sync_subscriber(subscriber);
		}
		dbi_result_free(result);
	}
	return 0;
}

int db_subscriber_alloc_exten(struct gsm_subscriber *subscriber, uint64_t smin,
			      uint64_t smax)
{
	dbi_result result = NULL;
	uint64_t try;

	for (;;) {
		try = (rand() % (smax - smin + 1) + smin);
		result = dbi_conn_queryf(conn,
			"SELECT * FROM Subscriber "
			"WHERE extension = %"PRIu64,
			try
		);
		if (!result) {
			LOGP(DDB, LOGL_ERROR, "Failed to query Subscriber "
				"while allocating new extension.\n");
			return 1;
		}
		if (dbi_result_get_numrows(result)){
			dbi_result_free(result);
			continue;
		}
		if (!next_row(result)) {
			dbi_result_free(result);
			break;
		}
		dbi_result_free(result);
	}
	sprintf(subscriber->extension, "%"PRIu64, try);
	DEBUGP(DDB, "Allocated extension %"PRIu64 " for IMSI %s.\n", try, subscriber->imsi);
	return db_sync_subscriber(subscriber);
}
/*
 * try to allocate a new unique token for this subscriber and return it
 * via a parameter. if the subscriber already has a token, return
 * an error.
 */

int db_subscriber_alloc_token(struct gsm_subscriber *subscriber, uint32_t *token)
{
	dbi_result result;
	uint32_t try;

	for (;;) {
		int rc = osmo_get_rand_id((uint8_t *) &try, sizeof(try));
		if (rc < 0) {
			LOGP(DDB, LOGL_ERROR, "osmo_get_rand_id() failed: %s\n", strerror(-rc));
			return 1;
		}
		if (!try) /* 0 is an invalid token */
			continue;
		result = dbi_conn_queryf(conn,
			"SELECT * FROM AuthToken "
			"WHERE subscriber_id = %llu OR token = \"%08X\" ",
			subscriber->id, try);
		if (!result) {
			LOGP(DDB, LOGL_ERROR, "Failed to query AuthToken "
				"while allocating new token.\n");
			return 1;
		}
		if (dbi_result_get_numrows(result)) {
			dbi_result_free(result);
			continue;
		}
		if (!next_row(result)) {
			dbi_result_free(result);
			break;
		}
		dbi_result_free(result);
	}
	result = dbi_conn_queryf(conn,
		"INSERT INTO AuthToken "
		"(subscriber_id, created, token) "
		"VALUES "
		"(%llu, datetime('now'), \"%08X\") ",
		subscriber->id, try);
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to create token %08X for "
			"IMSI %s.\n", try, subscriber->imsi);
		return 1;
	}
	dbi_result_free(result);
	*token = try;
	DEBUGP(DDB, "Allocated token %08X for IMSI %s.\n", try, subscriber->imsi);

	return 0;
}

int db_subscriber_assoc_imei(struct gsm_subscriber *subscriber, char imei[GSM23003_IMEISV_NUM_DIGITS])
{
	unsigned long long equipment_id, watch_id;
	dbi_result result;

	osmo_strlcpy(subscriber->equipment.imei, imei, sizeof(subscriber->equipment.imei));

	result = dbi_conn_queryf(conn,
		"INSERT OR IGNORE INTO Equipment "
		"(imei, created, updated) "
		"VALUES "
		"(%s, datetime('now'), datetime('now')) ",
		imei);
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to create Equipment by IMEI.\n");
		return 1;
	}

	equipment_id = 0;
	if (dbi_result_get_numrows_affected(result)) {
		equipment_id = dbi_conn_sequence_last(conn, NULL);
	}
	dbi_result_free(result);

	if (equipment_id)
		DEBUGP(DDB, "New Equipment: ID %llu, IMEI %s\n", equipment_id, imei);
	else {
		result = dbi_conn_queryf(conn,
			"SELECT id FROM Equipment "
			"WHERE imei = %s ",
			imei
		);
		if (!result) {
			LOGP(DDB, LOGL_ERROR, "Failed to query Equipment by IMEI.\n");
			return 1;
		}
		if (!next_row(result)) {
			LOGP(DDB, LOGL_ERROR, "Failed to find the Equipment.\n");
			dbi_result_free(result);
			return 1;
		}
		equipment_id = dbi_result_get_ulonglong(result, "id");
		dbi_result_free(result);
	}

	result = dbi_conn_queryf(conn,
		"INSERT OR IGNORE INTO EquipmentWatch "
		"(subscriber_id, equipment_id, created, updated) "
		"VALUES "
		"(%llu, %llu, datetime('now'), datetime('now')) ",
		subscriber->id, equipment_id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to create EquipmentWatch.\n");
		return 1;
	}

	watch_id = 0;
	if (dbi_result_get_numrows_affected(result))
		watch_id = dbi_conn_sequence_last(conn, NULL);

	dbi_result_free(result);
	if (watch_id)
		DEBUGP(DDB, "New EquipmentWatch: ID %llu, IMSI %s, IMEI %s\n",
			equipment_id, subscriber->imsi, imei);
	else {
		result = dbi_conn_queryf(conn,
			"UPDATE EquipmentWatch "
			"SET updated = datetime('now') "
			"WHERE subscriber_id = %llu AND equipment_id = %llu ",
			subscriber->id, equipment_id);
		if (!result) {
			LOGP(DDB, LOGL_ERROR, "Failed to update EquipmentWatch.\n");
			return 1;
		}
		dbi_result_free(result);
		DEBUGP(DDB, "Updated EquipmentWatch: ID %llu, IMSI %s, IMEI %s\n",
			equipment_id, subscriber->imsi, imei);
	}

	return 0;
}

/* store an [unsent] SMS to the database */
int db_sms_store(struct gsm_sms *sms)
{
	dbi_result result;
	char *q_text, *q_daddr, *q_saddr;
	unsigned char *q_udata;
	char *validity_timestamp = "2222-2-2";

	/* FIXME: generate validity timestamp based on validity_minutes */

	dbi_conn_quote_string_copy(conn, (char *)sms->text, &q_text);
	dbi_conn_quote_string_copy(conn, (char *)sms->dst.addr, &q_daddr);
	dbi_conn_quote_string_copy(conn, (char *)sms->src.addr, &q_saddr);
	dbi_conn_quote_binary_copy(conn, sms->user_data, sms->user_data_len,
				   &q_udata);

	/* FIXME: correct validity period */
	result = dbi_conn_queryf(conn,
		"INSERT INTO SMS "
		"(created, valid_until, "
		 "reply_path_req, status_rep_req, is_report, "
		 "msg_ref, protocol_id, data_coding_scheme, "
		 "ud_hdr_ind, "
		 "user_data, text, "
		 "dest_addr, dest_ton, dest_npi, "
		 "src_addr, src_ton, src_npi) VALUES "
		"(datetime('now'), %u, "
		"%u, %u, %u, "
		"%u, %u, %u, "
		"%u, "
		"%s, %s, "
		"%s, %u, %u, "
		"%s, %u, %u)",
		validity_timestamp,
		sms->reply_path_req, sms->status_rep_req, sms->is_report,
		sms->msg_ref, sms->protocol_id, sms->data_coding_scheme,
		sms->ud_hdr_ind,
		q_udata, q_text,
		q_daddr, sms->dst.ton, sms->dst.npi,
		q_saddr, sms->src.ton, sms->src.npi);
	free(q_text);
	free(q_udata);
	free(q_daddr);
	free(q_saddr);

	if (!result)
		return -EIO;

	dbi_result_free(result);
	return 0;
}

static struct gsm_sms *sms_from_result(struct gsm_network *net, dbi_result result)
{
	struct gsm_sms *sms = sms_alloc();
	const char *text, *daddr, *saddr;
	const unsigned char *user_data;

	if (!sms)
		return NULL;

	sms->id = dbi_result_get_ulonglong(result, "id");

	/* FIXME: validity */
	/* FIXME: those should all be get_uchar, but sqlite3 is braindead */
	sms->created = dbi_result_get_datetime(result, "created");
	sms->reply_path_req = dbi_result_get_ulonglong(result, "reply_path_req");
	sms->status_rep_req = dbi_result_get_ulonglong(result, "status_rep_req");
	sms->is_report = dbi_result_get_ulonglong(result, "is_report");
	sms->msg_ref = dbi_result_get_ulonglong(result, "msg_ref");
	sms->ud_hdr_ind = dbi_result_get_ulonglong(result, "ud_hdr_ind");
	sms->protocol_id = dbi_result_get_ulonglong(result, "protocol_id");
	sms->data_coding_scheme = dbi_result_get_ulonglong(result,
						  "data_coding_scheme");

	sms->dst.npi = dbi_result_get_ulonglong(result, "dest_npi");
	sms->dst.ton = dbi_result_get_ulonglong(result, "dest_ton");
	daddr = dbi_result_get_string(result, "dest_addr");
	if (daddr)
		osmo_strlcpy(sms->dst.addr, daddr, sizeof(sms->dst.addr));
	sms->receiver = subscr_get_by_extension(net->subscr_group, sms->dst.addr);

	sms->src.npi = dbi_result_get_ulonglong(result, "src_npi");
	sms->src.ton = dbi_result_get_ulonglong(result, "src_ton");
	saddr = dbi_result_get_string(result, "src_addr");
	if (saddr)
		osmo_strlcpy(sms->src.addr, saddr, sizeof(sms->src.addr));

	sms->user_data_len = dbi_result_get_field_length(result, "user_data");
	user_data = dbi_result_get_binary(result, "user_data");
	if (sms->user_data_len > sizeof(sms->user_data))
		sms->user_data_len = (uint8_t) sizeof(sms->user_data);
	memcpy(sms->user_data, user_data, sms->user_data_len);

	text = dbi_result_get_string(result, "text");
	if (text)
		osmo_strlcpy(sms->text, text, sizeof(sms->text));
	return sms;
}

struct gsm_sms *db_sms_get(struct gsm_network *net, unsigned long long id)
{
	dbi_result result;
	struct gsm_sms *sms;

	result = dbi_conn_queryf(conn,
		"SELECT * FROM SMS WHERE SMS.id = %llu", id);
	if (!result)
		return NULL;

	if (!next_row(result)) {
		dbi_result_free(result);
		return NULL;
	}

	sms = sms_from_result(net, result);

	dbi_result_free(result);

	return sms;
}

/* retrieve the next unsent SMS with ID >= min_id */
struct gsm_sms *db_sms_get_unsent(struct gsm_network *net, unsigned long long min_id)
{
	dbi_result result;
	struct gsm_sms *sms;

	result = dbi_conn_queryf(conn,
		"SELECT SMS.* "
			"FROM SMS JOIN Subscriber ON "
				"SMS.dest_addr = Subscriber.extension "
			"WHERE SMS.id >= %llu AND SMS.sent IS NULL "
				"AND Subscriber.lac > 0 "
			"ORDER BY SMS.id LIMIT 1",
		min_id);
	if (!result)
		return NULL;

	if (!next_row(result)) {
		dbi_result_free(result);
		return NULL;
	}

	sms = sms_from_result(net, result);

	dbi_result_free(result);

	return sms;
}

struct gsm_sms *db_sms_get_unsent_by_subscr(struct gsm_network *net,
					    unsigned long long min_subscr_id,
					    unsigned int failed)
{
	dbi_result result;
	struct gsm_sms *sms;

	result = dbi_conn_queryf(conn,
		"SELECT SMS.* "
			"FROM SMS JOIN Subscriber ON "
				"SMS.dest_addr = Subscriber.extension "
			"WHERE Subscriber.id >= %llu AND SMS.sent IS NULL "
				"AND Subscriber.lac > 0 AND SMS.deliver_attempts < %u "
			"ORDER BY Subscriber.id, SMS.id LIMIT 1",
		min_subscr_id, failed);
	if (!result)
		return NULL;

	if (!next_row(result)) {
		dbi_result_free(result);
		return NULL;
	}

	sms = sms_from_result(net, result);

	dbi_result_free(result);

	return sms;
}

/* retrieve the next unsent SMS for a given subscriber */
struct gsm_sms *db_sms_get_unsent_for_subscr(struct gsm_subscriber *subscr)
{
	dbi_result result;
	struct gsm_sms *sms;

	result = dbi_conn_queryf(conn,
		"SELECT SMS.* "
			"FROM SMS JOIN Subscriber ON "
				"SMS.dest_addr = Subscriber.extension "
			"WHERE Subscriber.id = %llu AND SMS.sent IS NULL "
				"AND Subscriber.lac > 0 "
			"ORDER BY SMS.id LIMIT 1",
		subscr->id);
	if (!result)
		return NULL;

	if (!next_row(result)) {
		dbi_result_free(result);
		return NULL;
	}

	sms = sms_from_result(subscr->group->net, result);

	dbi_result_free(result);

	return sms;
}

/* mark a given SMS as delivered */
int db_sms_mark_delivered(struct gsm_sms *sms)
{
	dbi_result result;

	result = dbi_conn_queryf(conn,
		"UPDATE SMS "
		"SET sent = datetime('now') "
		"WHERE id = %llu", sms->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to mark SMS %llu as sent.\n", sms->id);
		return 1;
	}

	dbi_result_free(result);
	return 0;
}

/* increase the number of attempted deliveries */
int db_sms_inc_deliver_attempts(struct gsm_sms *sms)
{
	dbi_result result;

	result = dbi_conn_queryf(conn,
		"UPDATE SMS "
		"SET deliver_attempts = deliver_attempts + 1 "
		"WHERE id = %llu", sms->id);
	if (!result) {
		LOGP(DDB, LOGL_ERROR, "Failed to inc deliver attempts for "
			"SMS %llu.\n", sms->id);
		return 1;
	}

	dbi_result_free(result);
	return 0;
}

int db_apdu_blob_store(struct gsm_subscriber *subscr,
			uint8_t apdu_id_flags, uint8_t len,
			uint8_t *apdu)
{
	dbi_result result;
	unsigned char *q_apdu;

	dbi_conn_quote_binary_copy(conn, apdu, len, &q_apdu);

	result = dbi_conn_queryf(conn,
		"INSERT INTO ApduBlobs "
		"(created,subscriber_id,apdu_id_flags,apdu) VALUES "
		"(datetime('now'),%llu,%u,%s)",
		subscr->id, apdu_id_flags, q_apdu);

	free(q_apdu);

	if (!result)
		return -EIO;

	dbi_result_free(result);
	return 0;
}

int db_store_counter(struct osmo_counter *ctr)
{
	dbi_result result;
	char *q_name;

	dbi_conn_quote_string_copy(conn, ctr->name, &q_name);

	result = dbi_conn_queryf(conn,
		"INSERT INTO Counters "
		"(timestamp,name,value) VALUES "
		"(datetime('now'),%s,%lu)", q_name, ctr->value);

	free(q_name);

	if (!result)
		return -EIO;

	dbi_result_free(result);
	return 0;
}

static int db_store_rate_ctr(struct rate_ctr_group *ctrg, unsigned int num,
			     char *q_prefix)
{
	dbi_result result;
	char *q_name;

	dbi_conn_quote_string_copy(conn, ctrg->desc->ctr_desc[num].name,
				   &q_name);

	result = dbi_conn_queryf(conn,
		"Insert INTO RateCounters "
		"(timestamp,name,idx,value) VALUES "
		"(datetime('now'),%s.%s,%u,%"PRIu64")",
		q_prefix, q_name, ctrg->idx, ctrg->ctr[num].current);

	free(q_name);

	if (!result)
		return -EIO;

	dbi_result_free(result);
	return 0;
}

int db_store_rate_ctr_group(struct rate_ctr_group *ctrg)
{
	unsigned int i;
	char *q_prefix;

	dbi_conn_quote_string_copy(conn, ctrg->desc->group_name_prefix, &q_prefix);

	for (i = 0; i < ctrg->desc->num_ctr; i++)
		db_store_rate_ctr(ctrg, i, q_prefix);

	free(q_prefix);

	return 0;
}
