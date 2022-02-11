#!/usr/bin/env python3
#
# Copyright 2014-present Facebook. All Rights Reserved.
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

<<<<<<< HEAD
import json
import re
=======
>>>>>>> facebook/helium
import subprocess

from rest_utils import DEFAULT_TIMEOUT_SEC


# Handler for FRUID resource endpoint
<<<<<<< HEAD
def get_fruid(cmd=["weutil"]):
=======
def get_fruid(cmd=["weutil"]):  # noqa: B006
>>>>>>> facebook/helium
    result = {}
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        data, err = proc.communicate(timeout=DEFAULT_TIMEOUT_SEC)
        data = data.decode(errors="ignore")
    except proc.TimeoutError as ex:
        data = ex.output
<<<<<<< HEAD
        err = ex.error
=======
>>>>>>> facebook/helium

    # need to remove the first info line from weutil
    adata = data.split("\n", 1)
    for sdata in adata[1].split("\n"):
        tdata = sdata.split(":", 1)
        if len(tdata) < 2:
            continue
        result[tdata[0].strip()] = tdata[1].strip()
    fresult = {"Information": result, "Actions": [], "Resources": []}
    return fresult
