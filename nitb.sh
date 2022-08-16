#!/bin/bash
exec osmo-nitb  --yes-i-really-want-to-run-prehistoric-software -c /home/ubuntu/icebreaker/openbsc.cfg -l /home/ubuntu/icebreaker/hlr.sqlite3 -P -M /tmp/bsc_mncc -C --debug=DRLL:DCC:DMM:DRR:DRSL:DNM
