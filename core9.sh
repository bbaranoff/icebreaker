#!/bin/bash
cd /home/ubuntu/icebreaker/libosmocore
git checkout 0.9.0
make
make install
ldconfig
