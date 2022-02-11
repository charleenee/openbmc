#!/usr/bin/env python3
#
# Copyright 2018-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#
import re
import time
import unittest
from abc import abstractmethod

from utils.cit_logger import Logger
from utils.shell_util import run_cmd
from utils.test_utils import check_fru_availability


class BasePowerUtilTest(unittest.TestCase):
    @abstractmethod
    def set_slots(self):
        pass

    def setUp(self):
        Logger.start(name=self._testMethodName)
        self.set_slots()
        self.slots_present = {}
        for slot in self.slots:
            self.slots_present[slot] = True if check_fru_availability(slot) else False

    def tearDown(self):
        Logger.info("turn all slots back to ON state")
        for slot, present in self.slots_present.items():
            if present:
                if self.slot_status(slot, status="off"):
                    self.turn_slot(slot, status="on")
                elif self.slot_12V_status(slot, status="off"):
                    self.turn_12V_slot(slot, status="on")
        Logger.info("Finished logging for {}".format(self._testMethodName))

    def slot_status(self, slot, status=None):
        cmd = ["/usr/local/bin/power-util", slot, "status"]
        cli_out = run_cmd(cmd).split(":")
        if len(cli_out) < 2:
            raise Exception("unexpected output: {}".format(cli_out))
        if cli_out[1].strip() == status.upper():
            return True
        return False

    def slot_12V_status(self, slot, status=None):
        cmd = ["/usr/local/bin/power-util", slot, "status"]
        cli_out = run_cmd(cmd).split(":")
        if len(cli_out) < 2:
            raise Exception("unexpected output: {}".format(cli_out))
        if cli_out[1].strip() == status.upper() + " (12V-{})".format(status.upper()):
            return True
        return False

    def turn_slot(self, slot, status=None):
        cmd = ["/usr/local/bin/power-util", slot, status]
        Logger.info("turn {} {} now..".format(slot, status))
        cli_out = run_cmd(cmd)
        pattern = r"^Powering fru.*to {} state.*$"
        self.assertIsNotNone(
            re.match(pattern.format(status.upper()), cli_out),
            "unexpected output: {}".format(cli_out),
        )
        count = 3
        while count > 0:
            if self.slot_status(slot, status=status):
                return True
            else:
                count -= 1
                time.sleep(2)
        return False

    def turn_12V_slot(self, slot, status=None):
        cmd = ["/usr/local/bin/power-util", slot, "12V-{}".format(status)]
        Logger.info("turn {} {} now..".format(slot, status))
        cli_out = run_cmd(cmd)
        pattern = r"^12V Powering fru.*to {} state.*$"
        self.assertIsNotNone(
            re.match(pattern.format(status.upper()), cli_out),
            "unexpected output: {}".format(cli_out),
        )
        count = 3
        while count > 0:
            if self.slot_status(slot, status=status):
                return True
            else:
                count -= 1
                time.sleep(2)
        return False

    def test_show_slot_status(self):
        """
        test power-util can show slot status
        """
        pattern = r"^Power status for fru.*: (ON|OFF)$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                cmd = ["/usr/local/bin/power-util", slot, "status"]
                cli_out = run_cmd(cmd)
                self.assertIsNotNone(
                    r.match(cli_out),
                    "unexpected output: {}".format(cli_out),
                )

    def test_slot_off(self):
        """
        test power-util [slot] off
        """
        pattern = r"^Power status for fru.*: OFF$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                # turn on the slot if its OFF
                if self.slot_status(slot, status="off"):
                    self.turn_slot(slot, status="on")
                self.turn_slot(slot, status="off")
                cmd = ["/usr/local/bin/power-util", slot, "status"]
                count = 3
                while count > 0:
                    cli_out = run_cmd(cmd)
                    m = r.search(cli_out)
                    if m:
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "Unexcpected output: {}".format(cli_out))
                else:
                    self.assertTrue(True)

    def test_slot_on(self):
        """
        test power-util [slot] on
        """
        pattern = r"^Power status for fru.*: ON$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                # turn off the slot if its ON
                if self.slot_status(slot, status="on"):
                    self.turn_slot(slot, status="off")
                self.turn_slot(slot, status="on")
                cmd = ["/usr/local/bin/power-util", slot, "status"]
                count = 3
                while count > 0:
                    cli_out = run_cmd(cmd)
                    m = r.search(cli_out)
                    if m:
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "Unexcpected output: {}".format(cli_out))
                else:
                    self.assertTrue(True)

    def test_slot_reset(self):
        """
        test power-util [slot] reset
        """
        pattern = r"^Power reset fru.*$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                cmd = ["/usr/local/bin/power-util", slot, "reset"]
                cli_out = run_cmd(cmd)
                # check if BMC is doing reset
                self.assertIsNotNone(
                    r.match(cli_out),
                    "unexpected output: {}".format(cli_out),
                )
                # check if slot back to ON status
                count = 3
                while count > 0:
                    if self.slot_status(slot, status="on"):
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "slot {} is not ON after reset".format(slot))
                else:
                    self.assertTrue(True)

    def test_slot_cycle(self):
        """
        test power-util [slot] cycle
        """
        pattern = r"^Power cycling fru.*$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                cmd = ["/usr/local/bin/power-util", slot, "cycle"]
                cli_out = run_cmd(cmd)
                # check if BMC is doing cycling
                self.assertIsNotNone(
                    r.match(cli_out),
                    "unexpected output: {}".format(cli_out),
                )
                # check if slot back to ON status
                count = 3
                while count > 0:
                    if self.slot_status(slot, status="on"):
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "slot {} is not ON after cycle".format(slot))
                else:
                    self.assertTrue(True)

    def test_12V_slot_off(self):
        """
        test power-util [slot] 12V-off
        """
        pattern = r"^Power status for fru.*: OFF \(12V-OFF\)$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                # turn on the slot if its OFF or 12V-off
                if self.slot_status(slot, status="off"):
                    self.turn_slot(slot, status="on")
                elif self.slot_12V_status(slot, status="off"):
                    self.turn_12V_slot(slot, status="on")
                self.turn_12V_slot(slot, status="off")
                cmd = ["/usr/local/bin/power-util", slot, "status"]
                count = 3
                while count > 0:
                    cli_out = run_cmd(cmd)
                    m = r.search(cli_out)
                    if m:
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "Unexcpected output: {}".format(cli_out))
                else:
                    self.assertTrue(True)

    def test_12V_slot_on(self):
        """
        test power-util [slot] 12V-on
        """
        pattern = r"^Power status for fru.*: ON$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                # turn off the slot if its ON
                if self.slot_status(slot, status="on"):
                    self.turn_12V_slot(slot, status="off")
                self.turn_12V_slot(slot, status="on")
                cmd = ["/usr/local/bin/power-util", slot, "status"]
                count = 3
                while count > 0:
                    cli_out = run_cmd(cmd)
                    m = r.search(cli_out)
                    if m:
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "Unexcpected output: {}".format(cli_out))
                else:
                    self.assertTrue(True)

    def test_12V_slot_cycle(self):
        """
        test power-util [slot] cycle
        """
        pattern = r"^12V Power cycling fru.*$"
        r = re.compile(pattern)
        for slot in self.slots:
            with self.subTest(slot=slot):
                if not check_fru_availability(slot):
                    self.skipTest("skip test due to {} not available".format(slot))
                cmd = ["/usr/local/bin/power-util", slot, "12V-cycle"]
                cli_out = run_cmd(cmd)
                # check if BMC is doing cycling
                self.assertIsNotNone(
                    r.match(cli_out),
                    "unexpected output: {}".format(cli_out),
                )
                # check if slot back to ON status
                count = 3
                while count > 0:
                    if self.slot_status(slot, status="on"):
                        break
                    else:
                        count -= 1
                        time.sleep(2)
                if count == 0:
                    self.assertTrue(False, "slot {} is not ON after cycle".format(slot))
                else:
                    self.assertTrue(True)
