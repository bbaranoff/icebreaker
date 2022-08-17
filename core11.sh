#!/bin/bash
cd /home/ubuntu/icebreaker/libosmocore
git checkout 1.1.0
make
make install
ldconfig
