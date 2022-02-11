#!/usr/bin/env python

<<<<<<< HEAD
from subprocess import *
=======
from typing import Any, Dict, Optional
>>>>>>> facebook/helium

from common_utils import async_exec
from kv import FPERSIST, kv_get
from node import node
<<<<<<< HEAD
from rest_pal_legacy import *
=======
from rest_pal_legacy import pal_get_platform_name
>>>>>>> facebook/helium

identify_name = {"FBTTN": "identify_slot1", "Grand Canyon": "system_identify_server"}


class identifyNode(node):
    def __init__(self, name, info=None, actions=None):
        self.name = name

        if info == None:
            self.info = {}
        else:
            self.info = info
        if actions == None:
            self.actions = []
        else:
            self.actions = actions

<<<<<<< HEAD
    async def getInformation(self, param={}):
=======
    async def getInformation(self, param: Optional[Dict[Any, Any]] = None):
>>>>>>> facebook/helium
        # Get Platform Name
        plat_name = pal_get_platform_name()

        if plat_name in identify_name:
            identify_status = kv_get(identify_name[plat_name], FPERSIST)
        else:
            identify_status = kv_get("identify_slot1", FPERSIST)
        info = {"Status of identify LED": identify_status}

        return info

<<<<<<< HEAD
    async def doAction(self, data, param={}):
=======
    async def doAction(self, data, param: Optional[Dict[Any, Any]] = None):
>>>>>>> facebook/helium
        if data["action"] == "on":
            cmd = "/usr/bin/fpc-util --identify on"
            _, stdout, _ = await async_exec(cmd, shell=True)
            if stdout.startswith("Usage"):
                res = "failure"
            else:
                res = "success"
        elif data["action"] == "off":
            cmd = "/usr/bin/fpc-util --identify off"
            _, stdout, _ = await async_exec(cmd, shell=True)
            if stdout.startswith("Usage"):
                res = "failure"
            else:
                res = "success"
        else:
            res = "not support this action"

        result = {"result": res}

        return result


def get_node_identify(name):
    actions = ["on", "off"]
    return identifyNode(name=name, actions=actions)
