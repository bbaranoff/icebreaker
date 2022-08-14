#!/usr/bin/env python3

# (C) 2013 by Katerina Barone-Adesi <kat.obsc@gmail.com>
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>


# Most systems won't be able to use these, so they're separated out
nitb_e1_configs = [
    "doc/examples/osmo-nitb/bs11/openbsc-2bts-2trx.cfg",
    "doc/examples/osmo-nitb/bs11/openbsc-1bts-2trx-hopping.cfg",
    "doc/examples/osmo-nitb/bs11/openbsc-1bts-2trx.cfg",
    "doc/examples/osmo-nitb/bs11/osmo-nitb.cfg",
    "doc/examples/osmo-nitb/nokia/openbsc_nokia_3trx.cfg",
    "doc/examples/osmo-nitb/nanobts/openbsc-multitrx.cfg",
    "doc/examples/osmo-nitb/rbs2308/osmo-nitb.cfg"
]


app_configs = {
    "osmo-bsc-sccplite": ["doc/examples/osmo-bsc-sccplite/osmo-bsc-sccplite.cfg"],
    "nat": ["doc/examples/osmo-bsc_nat/osmo-bsc-nat.cfg"],
    "mgcp": ["doc/examples/osmo-bsc_mgcp/osmo-bsc-mgcp.cfg"],
    "nitb": ["doc/examples/osmo-nitb/nanobts/openbsc-multitrx.cfg",
             "doc/examples/osmo-nitb/nanobts/osmo-nitb.cfg"],
}


apps = [(4242, "src/osmo-bsc/osmo-bsc-sccplite", "OsmoBSC", "osmo-bsc-sccplite"),
        (4244, "src/osmo-bsc_nat/osmo-bsc_nat",  "OsmoBSCNAT", "nat"),
        (4243, "src/osmo-bsc_mgcp/osmo-bsc_mgcp", "OpenBSC MGCP", "mgcp"),
        (4242, "src/osmo-nitb/osmo-nitb", "OpenBSC", "nitb"),
        ]

vty_command = ["./src/osmo-nitb/osmo-nitb", "-c",
               "doc/examples/osmo-nitb/nanobts/osmo-nitb.cfg"]

vty_app = apps[3] # reference apps[] entry for osmo-nitb
