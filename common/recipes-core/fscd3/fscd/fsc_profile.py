# Copyright 2015-present Facebook. All Rights Reserved.
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
from fsc_control import (
    PID,
    TTable,
    IncrementPID,
    TTable4Curve,
    IndependentPID,
    Feedforward,
)
from fsc_sensor import FscSensorSourceSysfs, FscSensorSourceUtil, FscSensorSourceJson
from fsc_util import Logger


class Sensor(object):
    def __init__(self, sensor_name, pTable):
        self.fru = None
        self.offset = None
        self.offset_table = None
        try:
            self.fru = pTable.get("fru", None)
            if "read_source" in pTable:
                if "sysfs" in pTable["read_source"]:
                    self.source = FscSensorSourceSysfs(
                        name=sensor_name, read_source=pTable["read_source"]["sysfs"]
                    )
                if "util" in pTable["read_source"]:
                    self.source = FscSensorSourceUtil(
                        name=sensor_name, read_source=pTable["read_source"]["util"]
                    )
                if "json" in pTable["read_source"]:
                    filter = None
                    if "filter" in pTable["read_source"]:
                        filter = pTable["read_source"]["filter"]
                    self.source = FscSensorSourceJson(
                        name=sensor_name,
                        read_source=pTable["read_source"]["json"],
                        filter=filter,
                    )
                if "offset" in pTable["read_source"]:
                    self.offset = pTable["read_source"]["offset"]
                elif "offset_table" in pTable["read_source"]:
                    self.offset_table = pTable["read_source"]["offset_table"]
        except Exception:
            Logger.error("Unknown Sensor source type")


def profile_constructor(data):
    return lambda: make_controller(data)


def make_controller(pTable):
    if pTable["type"] == "linear":
        controller = TTable(
            pTable["data"],
            pTable.get("negative_hysteresis", 0),
            pTable.get("positive_hysteresis", 0),
        )
        return controller
    if pTable["type"] == "pid":
        controller = PID(
            pTable["setpoint"],
            pTable["kp"],
            pTable["ki"],
            pTable["kd"],
            pTable["negative_hysteresis"],
            pTable["positive_hysteresis"],
        )
        return controller
    if pTable["type"] == "linear_4curve":
        controller = TTable4Curve(
            pTable["data_normal_up"],
            pTable["data_normal_down"],
            pTable["data_onefail_up"],
            pTable["data_onefail_down"],
        )
        return controller
    if pTable["type"] == "incrementpid":
        controller = IncrementPID(
            pTable["setpoint"],
            pTable["kp"],
            pTable["ki"],
            pTable["kd"],
            pTable["negative_hysteresis"],
            pTable["positive_hysteresis"],
        )
        return controller
    if pTable["type"] == "independentpid":
        controller = IndependentPID(pTable)
        return controller
    if pTable["type"] == "feedforward":
        controller = Feedforward(pTable)
        return controller
    err = "Don't understand profile type '%s'" % (pTable["type"])
    Logger.error(err)
