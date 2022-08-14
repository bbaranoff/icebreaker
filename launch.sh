#!/bin/bash
tmux new-window -n Orange-nitb
tmux new-window -n trxs
tmux new-window -n Orange-bts
tmux new-window -n Evil-nitb
tmux new-window -n Evil-bts
tmux new-window -n Victim-trxcon
tmux new-window -n Evil-trxcon
tmux new-window -n Victim-MS
tmux new-window -n Evil-MS
tmux send-keys -t "Orange-nitb" '/home/ubuntu/icebreaker/nitb.sh' Enter
tmux send-keys -t "trxs" '/home/ubuntu/IMSI_Catcher/osmocom-bb/src/target/trx_toolkit/fake_trx.py --trx 127.0.0.2:5700 --trx 127.0.0.2:6700' Enter
tmux send-keys -t "Orange-bts" '/opt/IMSI_Catcher/osmo-bts/src/osmo-bts-trx/osmo-bts-trx -c /opt/IMSI_Catcher/osmo-bts/doc/examples/trx/osmo-bts.cfg' Enter
tmux send-keys -t "Evil-nitb" '/home/ubuntu/icebreaker/bsc-2rfa/openbsc/src/osmo-nitb/osmo-nitb -c /home/ubuntu/icebreaker/evil-bsc.cfg' Enter
tmux send-keys -t "Evil-bts" '/opt/IMSI_Catcher/osmo-bts/src/osmo-bts-trx/osmo-bts-trx -c /home/ubuntu/icebreaker/evil-bts.cfg' Enter
tmux send-keys -t "Victim-trxcon" '/home/ubuntu/IMSI_Catcher/osmocom-bb/src/host/trxcon/trxcon -s /tmp/osmocom_l2' Enter
tmux send-keys -t "Victim-MS" '/home/ubuntu/icebreaker/kc_debug/src/host/layer23/src/mobile/mobile -s /tmpp/osmocom_l2' Enter
tmux send-keys -t "Evil-trxcon" '/home/ubuntu/IMSI_Catcher/osmocom-bb/src/host/trxcon/trxcon -s /tmp/osmocom_l2.2' Enter
tmux send-keys -t "Evil-MS" '/home/ubuntu/icebreaker/bb-2rfa/src/host/layer23/src/mobile/mobile -s /tmp/osmocom_l2.2 -u 127.0.0.2' Enter

