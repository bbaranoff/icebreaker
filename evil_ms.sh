#!/bin/bash
tmux new-window -n trxs
tmux new-window -n Orange-nitb
tmux new-window -n Orange-bts
tmux new-window -n Evil-trxcon
tmux new-window -n Evil-MS
tmux send-keys -t "trxs" '/home/ubuntu/icebreaker/osmocom-bb/src/target/trx_toolkit/fake_trx.py' Enter
tmux send-keys -t "Orange-nitb" '/home/ubuntu/icebreaker/nitb.sh' Enter
sleep 1
tmux send-keys -t "Orange-bts" '/home/ubuntu/icebreaker/osmo-bts/src/osmo-bts-trx/osmo-bts-trx -c /home/ubuntu/icebreaker/evil-bts.cfg' Enter
tmux send-keys -t "Evil-trxcon" '/home/ubuntu/icebreaker/osmocom-bb/src/host/trxcon/trxcon' Enter
sleep 1
tmux send-keys -t "Evil-MS" '/home/ubuntu/icebreaker/bb-2rfa/src/host/layer23/src/mobile/mobile' Enter

