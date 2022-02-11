<<<<<<< HEAD
=======
from typing import List

>>>>>>> facebook/helium
import pal
from aiohttp import web
from common_utils import dumps_bytestr
from redfish_base import validate_keys


<<<<<<< HEAD
class RedfishComputerSystems:
    def __init__(self):
        self.collection = []
        if not hasattr(pal, "pal_fru_name_map"):
            return
        fru_name_map = pal.pal_fru_name_map()
        for key, fruid in fru_name_map.items():
            if (
                hasattr(pal, "FruCapability")
                and pal.FruCapability.FRU_CAPABILITY_SERVER
                not in pal.pal_get_fru_capability(fruid)
            ) or (not hasattr(pal, "FruCapability") and key.find("slot") != 0):
                continue

            server_name = key.replace("slot", "server")
            self.collection.append(
                {
                    "@odata.id": "/redfish/v1/Systems/{}".format(server_name),
                    "Name": server_name,
=======
def pal_supported() -> bool:
    UNSUPPORTED_PLATFORM_BUILDNAMES = ["yamp", "wedge", "wedge100"]
    if not hasattr(pal, "pal_get_platform_name"):
        return False
    return pal.pal_get_platform_name() not in UNSUPPORTED_PLATFORM_BUILDNAMES


def get_compute_system_names() -> List[str]:
    if not pal_supported():
        return ["1"]

    if not hasattr(pal, "pal_fru_name_map"):
        return [""]
    fru_name_map = pal.pal_fru_name_map()

    # The fallback way
    if not hasattr(pal, "FruCapability") or not hasattr(pal, "pal_get_fru_capability"):
        compute_system_names = []
        for key, _ in fru_name_map.items():
            if key.find("slot") != 0:
                continue
            server_name = key.replace("slot", "server")
            compute_system_names.append(server_name)
        return compute_system_names

    # The right way
    compute_system_names = []
    for key, fruid in fru_name_map.items():
        if pal.FruCapability.FRU_CAPABILITY_SERVER not in pal.pal_get_fru_capability(
            fruid
        ):
            continue
        server_name = key.replace("slot", "server")
        compute_system_names.append(server_name)
    return compute_system_names


class RedfishComputerSystems:
    def __init__(self):
        self.collection = []
        for system_name in get_compute_system_names():
            self.collection.append(
                {
                    "@odata.id": "/redfish/v1/Systems/{}".format(system_name),
                    "Name": system_name,
>>>>>>> facebook/helium
                }
            )

    async def get_collection_descriptor(self, request: web.Request) -> web.Response:
<<<<<<< HEAD
        body = {
            "@odata.id": "/redfish/v1/Systems",
            "@odata.context": "/redfish/v1/$metadata#ComputerSystemCollection.ComputerSystemCollection",
=======
        headers = {
            "Link": "</redfish/v1/schemas/v1/ComputerSystemCollection.json>; rel=describedby"  # noqa: E501
        }
        body = {
            "@odata.id": "/redfish/v1/Systems",
            "@odata.context": "/redfish/v1/$metadata#ComputerSystemCollection.ComputerSystemCollection",  # noqa: E501
>>>>>>> facebook/helium
            "@odata.type": "#ComputerSystemCollection.ComputerSystemCollection",
            "Name": "Computer System Collection",
            "Members@odata.count": len(self.collection),
            "Members": self.collection,
        }
        await validate_keys(body)
<<<<<<< HEAD
        return web.json_response(body, dumps=dumps_bytestr)
=======
        return web.json_response(body, headers=headers, dumps=dumps_bytestr)
>>>>>>> facebook/helium

    def has_server(self, server_name) -> bool:
        for server in self.collection:
            if server["Name"] == server_name:
                return True
        return False

<<<<<<< HEAD
    async def get_system_descriptor(
        self, server_name: str, request: web.Request
    ) -> web.Response:
=======
    async def get_system_descriptor(self, request: web.Request) -> web.Response:
        server_name = request.match_info["server_name"]
>>>>>>> facebook/helium
        if not self.has_server(server_name):
            return web.Response(status=404)

        body = {
            "@odata.id": "/redfish/v1/Systems/{}".format(server_name),
<<<<<<< HEAD
            "@odata.type": "#ComputerSystem.ComputerSystem",
            "Name": server_name,
            "Bios": {
                "@odata.id": "/redfish/v1/Systems/{}/Bios".format(server_name),
            },
=======
            "@odata.type": "#ComputerSystem.v1_16_1.ComputerSystem",
            "Id": server_name,
            "Name": server_name,
>>>>>>> facebook/helium
            "Actions": {
                "#ComputerSystem.Reset": {
                    "target": "/redfish/v1/Systems/{}/Actions/ComputerSystem.Reset".format(  # noqa B950
                        server_name
                    ),
                    "ResetType@Redfish.AllowableValues": [
                        "On",
                        "ForceOff",
                        "GracefulShutdown",
                        "GracefulRestart",
                        "ForceRestart",
                        "ForceOn",
                        "PushPowerButton",
                    ],
                },
            },
        }
<<<<<<< HEAD
        await validate_keys(body)
        return web.json_response(body, dumps=dumps_bytestr)

    async def get_bios_descriptor(
        self, server_name: str, request: web.Request
    ) -> web.Response:
=======
        if pal_supported():
            body["Bios"] = {
                "@odata.id": "/redfish/v1/Systems/{}/Bios".format(server_name),
            }
        await validate_keys(body)
        return web.json_response(body, dumps=dumps_bytestr)

    async def get_bios_descriptor(self, request: web.Request) -> web.Response:
        server_name = request.match_info["server_name"]
>>>>>>> facebook/helium
        if not self.has_server(server_name):
            return web.Response(status=404)

        body = {
            "@odata.id": "/redfish/v1/Systems/{}/Bios".format(server_name),
<<<<<<< HEAD
            "@odata.type": "#Bios.Bios",
            "Name": "{} BIOS".format(server_name),
=======
            "@odata.type": "#Bios.v1_1_0.Bios",
            "Name": "{} BIOS".format(server_name),
            "Id": "{} BIOS".format(server_name),
>>>>>>> facebook/helium
            "Links": {
                "ActiveSoftwareImage": {
                    "@odata.id": "/redfish/v1/Systems/{}/Bios/FirmwareInventory".format(
                        server_name,
<<<<<<< HEAD
                    )
=======
                    ),
                    "@odata.type": "#SoftwareInventory.v1_0_0.SoftwareInventory",
                    "Id": "{}/Bios".format(server_name),
                    "Name": "{} BIOS Firmware".format(server_name),
>>>>>>> facebook/helium
                },
            },
        }
        await validate_keys(body)
        return web.json_response(body, dumps=dumps_bytestr)

<<<<<<< HEAD
    async def get_bios_firmware_inventory(self, server_name: str, request: web.Request):
=======
    async def get_bios_firmware_inventory(self, request: web.Request):
        server_name = request.match_info["server_name"]

>>>>>>> facebook/helium
        if not self.has_server(server_name):
            return web.Response(status=404)

        body = {
            "@odata.id": "/redfish/v1/Systems/{}/Bios/FirmwareInventory".format(
                server_name
            ),
<<<<<<< HEAD
            "@odata.type": "#SoftwareInventory.SoftwareInventory",
=======
            "@odata.type": "#SoftwareInventory.v1_0_0.SoftwareInventory",
>>>>>>> facebook/helium
            "Id": "{}/Bios".format(server_name),
            "Name": "{} BIOS Firmware".format(server_name),
            "Oem": {
                "Dumps": {
<<<<<<< HEAD
                    "@odata.id": "/redfish/v1/Systems/{}/Bios/FirmwareDumps".format(
                        server_name
                    ),
=======
                    "@odata.type": "#FirmwareDumps.v1_0_0.FirmwareDumps",
                    "@odata.context": "/redfish/v1/$metadata#FirmwareDumps.FirmwareDumps",
                    "@odata.id": "/redfish/v1/Systems/{}/Bios/FirmwareDumps".format(
                        server_name
                    ),
                    "Id": "{}/Bios Dumps".format(server_name),
                    "Name": "{}/Bios Dumps".format(server_name),
>>>>>>> facebook/helium
                }
            },
        }
        await validate_keys(body)
        return web.json_response(body, dumps=dumps_bytestr)
<<<<<<< HEAD

    def get_server(self, server_name: str) -> "RedfishComputerSystem":
        return RedfishComputerSystem(self, server_name=server_name)


class RedfishComputerSystem:
    def __init__(self, collection: RedfishComputerSystems, server_name: str):
        self.collection = collection
        self.server_name = server_name

    async def get_system_descriptor(self, request: web.Request):
        return await self.collection.get_system_descriptor(self.server_name, request)

    async def get_bios_descriptor(self, request: web.Request):
        return await self.collection.get_bios_descriptor(self.server_name, request)

    async def get_bios_firmware_inventory(self, request: web.Request):
        return await self.collection.get_bios_firmware_inventory(
            self.server_name, request
        )
=======
>>>>>>> facebook/helium
