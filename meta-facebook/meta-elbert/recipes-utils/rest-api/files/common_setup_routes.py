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
from aiohttp.log import server_logger
from aiohttp.web import Application
from common_endpoint import commonApp_Handler
from redfish_common_routes import Redfish
from rest_utils import common_routes


def setup_common_routes(app: Application, write_enabled: bool):
    chandler = commonApp_Handler()
    app.router.add_get(common_routes[0], chandler.rest_api)
    app.router.add_get(common_routes[1], chandler.rest_sys)
    app.router.add_get(common_routes[2], chandler.rest_mb_sys)
    app.router.add_get(common_routes[3], chandler.rest_fruid_pim_hdl)
    app.router.add_get(common_routes[4], chandler.rest_bmc_hdl)
    app.router.add_get(common_routes[5], chandler.rest_server_hdl)
    app.router.add_post(common_routes[5], chandler.rest_server_act_hdl)
    app.router.add_get(common_routes[6], chandler.rest_sensors_hdl)
    app.router.add_get(common_routes[7], chandler.rest_gpios_hdl)
    app.router.add_get(common_routes[8], chandler.rest_fcpresent_hdl)
    app.router.add_get(common_routes[9], chandler.psu_update_hdl)
    app.router.add_post(common_routes[9], chandler.psu_update_hdl_post)
    server_logger.info("Adding Redfish common routes")
    redfish = Redfish()
    redfish.setup_redfish_common_routes(app)
