#!/bin/bash
exec osmo-nitb -c /home/ubuntu/icebreaker/evil-bsc.cfg -l hlr.sqlite3 -P -M /tmp/bsc_mncc -C --debug=DRLL:DCC:DMM:DRR:DRSL:DNM
