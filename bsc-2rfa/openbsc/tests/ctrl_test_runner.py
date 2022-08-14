#!/usr/bin/env python3

# (C) 2013 by Jacob Erlbeck <jerlbeck@sysmocom.de>
# (C) 2014 by Holger Hans Peter Freyther
# based on vty_test_runner.py:
# (C) 2013 by Katerina Barone-Adesi <kat.obsc@gmail.com>
# (C) 2013 by Holger Hans Peter Freyther
# based on bsc_control.py.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import time
import unittest
import socket
import sys
import struct

import osmopy.obscvty as obscvty
import osmopy.osmoutil as osmoutil

# add $top_srcdir/contrib to find ipa.py
sys.path.append(os.path.join(sys.path[0], '..', 'contrib'))

from ipa import Ctrl, IPA

# to be able to find $top_srcdir/doc/...
confpath = os.path.join(sys.path[0], '..')
verbose = False

class TestCtrlBase(unittest.TestCase):

    def ctrl_command(self):
        raise Exception("Needs to be implemented by a subclass")

    def ctrl_app(self):
        raise Exception("Needs to be implemented by a subclass")

    def setUp(self):
        osmo_ctrl_cmd = self.ctrl_command()[:]
        config_index = osmo_ctrl_cmd.index('-c')
        if config_index:
            cfi = config_index + 1
            osmo_ctrl_cmd[cfi] = os.path.join(confpath, osmo_ctrl_cmd[cfi])

        try:
            self.proc = osmoutil.popen_devnull(osmo_ctrl_cmd)
        except OSError:
            print("Current directory: %s" % os.getcwd(), file=sys.stderr)
            print("Consider setting -b", file=sys.stderr)
        time.sleep(2)

        appstring = self.ctrl_app()[2]
        appport = self.ctrl_app()[0]
        self.connect("127.0.0.1", appport)
        self.next_id = 1000

    def tearDown(self):
        self.disconnect()
        osmoutil.end_proc(self.proc)

    def disconnect(self):
        if not (self.sock is None):
            self.sock.close()

    def connect(self, host, port):
        if verbose:
            print("Connecting to host %s:%i" % (host, port))

        retries = 30
        while True:
            try:
                sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sck.setblocking(1)
                sck.connect((host, port))
            except IOError:
                retries -= 1
                if retries <= 0:
                    raise
                time.sleep(.1)
                continue
            break
        self.sock = sck
        return sck

    def send(self, data):
        if verbose:
            print("Sending \"%s\"" %(data))
        data = Ctrl().add_header(data)
        return self.sock.send(data) == len(data)

    def send_set(self, var, value, id):
        setmsg = "SET %s %s %s" %(id, var, value)
        return self.send(setmsg)

    def send_get(self, var, id):
        getmsg = "GET %s %s" %(id, var)
        return self.send(getmsg)

    def do_set(self, var, value):
        id = self.next_id
        self.next_id += 1
        self.send_set(var, value, id)
        return self.recv_msgs()[id]

    def do_get(self, var):
        id = self.next_id
        self.next_id += 1
        self.send_get(var, id)
        return self.recv_msgs()[id]

    def recv_msgs(self):
        responses = {}
        data = self.sock.recv(4096)
        while (len(data)>0):
            (head, data) = IPA().split_combined(data)
            answer = Ctrl().rem_header(head).decode()
            if verbose:
                print("Got message:", answer)
            (mtype, id, msg) = answer.split(None, 2)
            id = int(id)
            rsp = {'mtype': mtype, 'id': id}
            if mtype == "ERROR":
                rsp['error'] = msg
            else:
                split = msg.split(None, 1)
                rsp['var'] = split[0]
                if len(split) > 1:
                    rsp['value'] = split[1]
                else:
                    rsp['value'] = None
            responses[id] = rsp

        if verbose:
            print("Decoded replies: ", responses)

        return responses


class TestCtrlBSC(TestCtrlBase):

    def tearDown(self):
        TestCtrlBase.tearDown(self)
        os.unlink("tmp_dummy_sock")

    def ctrl_command(self):
        return ["./src/osmo-bsc/osmo-bsc-sccplite", "-r", "tmp_dummy_sock", "-c",
                "doc/examples/osmo-bsc-sccplite/osmo-bsc-sccplite.cfg"]

    def ctrl_app(self):
        return (4249, "./src/osmo-bsc/osmo-bsc-sccplite", "OsmoBSC", "bsc")

    def testCtrlErrs(self):
        r = self.do_get('invalid')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Command not found')

        r = self.do_set('rf_locked', '999')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Value failed verification.')

        r = self.do_get('bts')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Error while parsing the index.')

        r = self.do_get('bts.999')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Error while resolving object')

    def testBtsLac(self):
        r = self.do_get('bts.0.location-area-code')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.location-area-code')
        self.assertEqual(r['value'], '1')

        r = self.do_set('bts.0.location-area-code', '23')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'bts.0.location-area-code')
        self.assertEqual(r['value'], '23')

        r = self.do_get('bts.0.location-area-code')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.location-area-code')
        self.assertEqual(r['value'], '23')

        r = self.do_set('bts.0.location-area-code', '-1')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Input not within the range')

    def testBtsCi(self):
        r = self.do_get('bts.0.cell-identity')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.cell-identity')
        self.assertEqual(r['value'], '0')

        r = self.do_set('bts.0.cell-identity', '23')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'bts.0.cell-identity')
        self.assertEqual(r['value'], '23')

        r = self.do_get('bts.0.cell-identity')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.cell-identity')
        self.assertEqual(r['value'], '23')

        r = self.do_set('bts.0.cell-identity', '-1')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Input not within the range')

    def testBtsGenerateSystemInformation(self):
        r = self.do_get('bts.0.send-new-system-informations')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Write Only attribute')

        # No RSL links so it will fail
        r = self.do_set('bts.0.send-new-system-informations', '1')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Failed to generate SI')

    def testBtsChannelLoad(self):
        r = self.do_set('bts.0.channel-load', '1')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Read Only attribute')

        # No RSL link so everything is 0
        r = self.do_get('bts.0.channel-load')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['value'],
		'CCCH+SDCCH4,0,0 TCH/F,0,0 TCH/H,0,0 SDCCH8,0,0'
		+ ' TCH/F_PDCH,0,0 CCCH+SDCCH4+CBCH,0,0'
		+ ' SDCCH8+CBCH,0,0 TCH/F_TCH/H_PDCH,0,0')

    def testBtsOmlConnectionState(self):
        """Check OML state. It will not be connected"""
        r = self.do_set('bts.0.oml-connection-state', '1')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Read Only attribute')

        # No RSL link so everything is 0
        r = self.do_get('bts.0.oml-connection-state')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['value'], 'disconnected')

    def testTrxPowerRed(self):
        r = self.do_get('bts.0.trx.0.max-power-reduction')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.trx.0.max-power-reduction')
        self.assertEqual(r['value'], '20')

        r = self.do_set('bts.0.trx.0.max-power-reduction', '22')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'bts.0.trx.0.max-power-reduction')
        self.assertEqual(r['value'], '22')

        r = self.do_get('bts.0.trx.0.max-power-reduction')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.trx.0.max-power-reduction')
        self.assertEqual(r['value'], '22')

        r = self.do_set('bts.0.trx.0.max-power-reduction', '1')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Value must be even')

    def testTrxArfcn(self):
        r = self.do_get('bts.0.trx.0.arfcn')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.trx.0.arfcn')
        self.assertEqual(r['value'], '871')

        r = self.do_set('bts.0.trx.0.arfcn', '873')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'bts.0.trx.0.arfcn')
        self.assertEqual(r['value'], '873')

        r = self.do_get('bts.0.trx.0.arfcn')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.trx.0.arfcn')
        self.assertEqual(r['value'], '873')

        r = self.do_set('bts.0.trx.0.arfcn', '2000')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Input not within the range')

    def testRfLock(self):
        r = self.do_get('bts.0.rf_state')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.rf_state')
        self.assertEqual(r['value'], 'inoperational,unlocked,on')

        r = self.do_set('rf_locked', '1')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'rf_locked')
        self.assertEqual(r['value'], '1')

        time.sleep(1.5)

        r = self.do_get('bts.0.rf_state')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.rf_state')
        self.assertEqual(r['value'], 'inoperational,locked,off')

        r = self.do_get('rf_locked')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'rf_locked')
        self.assertEqual(r['value'], 'state=off,policy=off')

        r = self.do_set('rf_locked', '0')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'rf_locked')
        self.assertEqual(r['value'], '0')

        time.sleep(1.5)

        r = self.do_get('bts.0.rf_state')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.rf_state')
        self.assertEqual(r['value'], 'inoperational,unlocked,on')

        r = self.do_get('rf_locked')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'rf_locked')
        self.assertEqual(r['value'], 'state=off,policy=on')

    def testTimezone(self):
        r = self.do_get('timezone')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'timezone')
        self.assertEqual(r['value'], 'off')

        r = self.do_set('timezone', '-2,15,2')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'timezone')
        self.assertEqual(r['value'], '-2,15,2')

        r = self.do_get('timezone')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'timezone')
        self.assertEqual(r['value'], '-2,15,2')

        # Test invalid input
        r = self.do_set('timezone', '-2,15,2,5,6,7')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'timezone')
        self.assertEqual(r['value'], '-2,15,2')

        r = self.do_set('timezone', '-2,15')
        self.assertEqual(r['mtype'], 'ERROR')
        r = self.do_set('timezone', '-2')
        self.assertEqual(r['mtype'], 'ERROR')
        r = self.do_set('timezone', '1')

        r = self.do_set('timezone', 'off')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'timezone')
        self.assertEqual(r['value'], 'off')

        r = self.do_get('timezone')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'timezone')
        self.assertEqual(r['value'], 'off')

    def testMcc(self):
        r = self.do_set('mcc', '23')
        r = self.do_get('mcc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mcc')
        self.assertEqual(r['value'], '023')

        r = self.do_set('mcc', '023')
        r = self.do_get('mcc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mcc')
        self.assertEqual(r['value'], '023')

    def testMnc(self):
        r = self.do_set('mnc', '9')
        r = self.do_get('mnc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mnc')
        self.assertEqual(r['value'], '09')

        r = self.do_set('mnc', '09')
        r = self.do_get('mnc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mnc')
        self.assertEqual(r['value'], '09')

        r = self.do_set('mnc', '009')
        r = self.do_get('mnc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mnc')
        self.assertEqual(r['value'], '009')

    def testMccMncApply(self):
        # Test some invalid input
        r = self.do_set('mcc-mnc-apply', 'WRONG')
        self.assertEqual(r['mtype'], 'ERROR')

        r = self.do_set('mcc-mnc-apply', '1,')
        self.assertEqual(r['mtype'], 'ERROR')

        r = self.do_set('mcc-mnc-apply', '200,3')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'mcc-mnc-apply')
        self.assertEqual(r['value'], 'Tried to drop the BTS')

        # Set it again
        r = self.do_set('mcc-mnc-apply', '200,3')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'mcc-mnc-apply')
        self.assertEqual(r['value'], 'Nothing changed')

        # Change it
        r = self.do_set('mcc-mnc-apply', '200,4')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'mcc-mnc-apply')
        self.assertEqual(r['value'], 'Tried to drop the BTS')

        # Change it
        r = self.do_set('mcc-mnc-apply', '201,4')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'mcc-mnc-apply')
        self.assertEqual(r['value'], 'Tried to drop the BTS')

        # Verify
        r = self.do_get('mnc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mnc')
        self.assertEqual(r['value'], '04')

        r = self.do_get('mcc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mcc')
        self.assertEqual(r['value'], '201')

        # Change it
        r = self.do_set('mcc-mnc-apply', '202,03')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'mcc-mnc-apply')
        self.assertEqual(r['value'], 'Tried to drop the BTS')

        r = self.do_get('mnc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mnc')
        self.assertEqual(r['value'], '03')

        r = self.do_get('mcc')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'mcc')
        self.assertEqual(r['value'], '202')

class TestCtrlNITB(TestCtrlBase):

    def tearDown(self):
        TestCtrlBase.tearDown(self)
        os.unlink("test_hlr.sqlite3")

    def ctrl_command(self):
        return ["./src/osmo-nitb/osmo-nitb", "-c",
                "doc/examples/osmo-nitb/nanobts/osmo-nitb.cfg", "-l", "test_hlr.sqlite3"]

    def ctrl_app(self):
        return (4249, "./src/osmo-nitb/osmo-nitb", "OsmoBSC", "nitb")

    def testNumberOfBTS(self):
        r = self.do_get('number-of-bts')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'number-of-bts')
        self.assertEqual(r['value'], '1')

    def testSubscriberAddWithKi(self):
        """Test that we can set the algorithm to none, xor, comp128v1"""

        r = self.do_set('subscriber-modify-v1', '2620345,445566')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        r = self.do_set('subscriber-modify-v1', '2620345,445566,none')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        r = self.do_set('subscriber-modify-v1', '2620345,445566,xor')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Value failed verification.')

        r = self.do_set('subscriber-modify-v1', '2620345,445566,comp128v1,00112233445566778899AABBCCDDEEFF')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        r = self.do_set('subscriber-modify-v1', '2620345,445566,comp128v2,00112233445566778899AABBCCDDEEFF')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        r = self.do_set('subscriber-modify-v1', '2620345,445566,comp128v3,00112233445566778899AABBCCDDEEFF')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        r = self.do_set('subscriber-modify-v1', '2620345,445566,none')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

    def testSubscriberAddRemove(self):
        r = self.do_set('subscriber-modify-v1', '2620345,445566')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        r = self.do_set('subscriber-modify-v1', '2620345,445567')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'subscriber-modify-v1')
        self.assertEqual(r['value'], 'OK')

        # TODO. verify that the entry has been created and modified? Invoke
        # the sqlite3 CLI or do it through the DB libraries?

        r = self.do_set('subscriber-delete-v1', '2620345')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['value'], 'Removed')

        r = self.do_set('subscriber-delete-v1', '2620345')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Failed to find subscriber')

    def testSubscriberList(self):
        # TODO. Add command to mark a subscriber as active
        r = self.do_get('subscriber-list-active-v1')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'subscriber-list-active-v1')
        self.assertEqual(r['value'], None)

    def testApplyConfiguration(self):
        r = self.do_get('bts.0.apply-configuration')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Write Only attribute')

        r = self.do_set('bts.0.apply-configuration', '1')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['value'], 'Tried to drop the BTS')

    def testGprsMode(self):
        r = self.do_get('bts.0.gprs-mode')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.gprs-mode')
        self.assertEqual(r['value'], 'none')

        r = self.do_set('bts.0.gprs-mode', 'bla')
        self.assertEqual(r['mtype'], 'ERROR')
        self.assertEqual(r['error'], 'Mode is not known')

        r = self.do_set('bts.0.gprs-mode', 'egprs')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['value'], 'egprs')

        r = self.do_get('bts.0.gprs-mode')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'bts.0.gprs-mode')
        self.assertEqual(r['value'], 'egprs')

class TestCtrlNAT(TestCtrlBase):

    def ctrl_command(self):
        return ["./src/osmo-bsc_nat/osmo-bsc_nat", "-c",
                "doc/examples/osmo-bsc_nat/osmo-bsc-nat.cfg"]

    def ctrl_app(self):
        return (4250, "./src/osmo-bsc_nat/osmo-bsc_nat", "OsmoNAT", "nat")

    def testAccessList(self):
        r = self.do_get('net.0.bsc_cfg.0.access-list-name')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'net')
        self.assertEqual(r['value'], None)

        r = self.do_set('net.0.bsc_cfg.0.access-list-name', 'bla')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'net')
        self.assertEqual(r['value'], 'bla')

        r = self.do_get('net.0.bsc_cfg.0.access-list-name')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'net')
        self.assertEqual(r['value'], 'bla')

        r = self.do_set('net.0.bsc_cfg.0.no-access-list-name', '1')
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'net')
        self.assertEqual(r['value'], None)

        r = self.do_get('net.0.bsc_cfg.0.access-list-name')
        self.assertEqual(r['mtype'], 'GET_REPLY')
        self.assertEqual(r['var'], 'net')
        self.assertEqual(r['value'], None)

    def testAccessListManagement(self):
        r = self.do_set("net.0.add.allow.access-list.404", "abc")
        self.assertEqual(r['mtype'], 'ERROR')

        r = self.do_set("net.0.add.allow.access-list.bla", "^234$")
        self.assertEqual(r['mtype'], 'SET_REPLY')
        self.assertEqual(r['var'], 'net.0.add.allow.access-list.bla')
        self.assertEqual(r['value'], 'IMSI allow added to access list')

        # TODO.. find a way to actually see if this rule has been
        # added. e.g. by implementing a get for the list.

def add_bsc_test(suite, workdir):
    if not os.path.isfile(os.path.join(workdir, "src/osmo-bsc/osmo-bsc-sccplite")):
        print("Skipping the BSC test")
        return
    test = unittest.TestLoader().loadTestsFromTestCase(TestCtrlBSC)
    suite.addTest(test)

def add_nitb_test(suite, workdir):
    test = unittest.TestLoader().loadTestsFromTestCase(TestCtrlNITB)
    suite.addTest(test)

def add_nat_test(suite, workdir):
    if not os.path.isfile(os.path.join(workdir, "src/osmo-bsc_nat/osmo-bsc_nat")):
        print("Skipping the NAT test")
        return
    test = unittest.TestLoader().loadTestsFromTestCase(TestCtrlNAT)
    suite.addTest(test)

if __name__ == '__main__':
    import argparse
    import sys

    workdir = '.'

    parser = argparse.ArgumentParser()
    parser.add_argument("-v", "--verbose", dest="verbose",
                        action="store_true", help="verbose mode")
    parser.add_argument("-p", "--pythonconfpath", dest="p",
                        help="searchpath for config")
    parser.add_argument("-w", "--workdir", dest="w",
                        help="Working directory")
    args = parser.parse_args()

    verbose_level = 1
    if args.verbose:
        verbose_level = 2
        verbose = True

    if args.w:
        workdir = args.w

    if args.p:
        confpath = args.p

    print("confpath %s, workdir %s" % (confpath, workdir))
    os.chdir(workdir)
    print("Running tests for specific control commands")
    suite = unittest.TestSuite()
    add_bsc_test(suite, workdir)
    add_nitb_test(suite, workdir)
    add_nat_test(suite, workdir)
    res = unittest.TextTestRunner(verbosity=verbose_level).run(suite)
    sys.exit(len(res.errors) + len(res.failures))
