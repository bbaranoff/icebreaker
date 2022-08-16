#!/bin/bash
tmux new-window -n trxs
tmux new-window -n Evil-nitb
tmux new-window -n Evil-bts
tmux new-window -n Orange-trxcon
tmux new-window -n Orange-MS
tmux send-keys -t "trxs" '/home/ubuntu/icebreaker/osmocom-bb/src/target/trx_toolkit/fake_trx.py' Enter
sleep 1
tmux send-keys -t "Evil-nitb" '/home/ubuntu/icebreaker/nitb.sh' Enter
tmux send-keys -t "Evil-bts" '/home/ubuntu/icebreaker/osmo-bts/src/osmo-bts-trx/osmo-bts-trx -c /home/ubuntu/icebreaker/evil-bts.cfg' Enter
tmux send-keys -t "Orange-trxcon" '/home/ubuntu/icebreaker/osmocom-bb/src/host/trxcon/trxcon' Enter
sleep 3
tmux send-keys -t "Orange-MS" '/home/ubuntu/icebreaker/osmocom-bb/src/host/layer23/src/mobile/mobile' Enter

