from aiohttp import web
from common_utils import dumps_bytestr
from redfish_base import validate_keys


async def get_account_service(request) -> web.Response:
<<<<<<< HEAD
=======
    headers = {
        "Link": "</redfish/v1/schemas/AccountService.v1_0_0.json>; rel=describedby"
    }
>>>>>>> facebook/helium
    body = {
        "@odata.context": "/redfish/v1/$metadata#AccountService.AccountService",
        "@odata.id": "/redfish/v1/AccountService",
        "@odata.type": "#AccountService.v1_0_0.AccountService",
        "Id": "AccountService",
        "Name": "Account Service",
        "Description": "Account Service",
        "Accounts": {"@odata.id": "/redfish/v1/AccountService/Accounts"},
        "Roles": {"@odata.id": "/redfish/v1/AccountService/Roles"},
    }
    await validate_keys(body)
<<<<<<< HEAD
    return web.json_response(body, dumps=dumps_bytestr)


async def get_accounts(request) -> web.Response:
=======
    return web.json_response(body, headers=headers, dumps=dumps_bytestr)


async def get_accounts(request) -> web.Response:
    headers = {"Link": "</redfish/v1/AccountService/Accounts>; rel=describedby"}
>>>>>>> facebook/helium
    body = {
        "@odata.type": "#ManagerAccountCollection.ManagerAccountCollection",
        "Name": "Accounts Collection",
        "Members@odata.count": 0,
        "Members": [],
        "@odata.id": "/redfish/v1/AccountService/Accounts",
    }
    await validate_keys(body)
<<<<<<< HEAD
    return web.json_response(body, dumps=dumps_bytestr)
=======
    return web.json_response(body, headers=headers, dumps=dumps_bytestr)
>>>>>>> facebook/helium


async def get_roles(request) -> web.Response:
    body = {
        "@odata.type": "#RoleCollection.RoleCollection",
        "Name": "Roles Collection",
        "Members@odata.count": 0,
        "Members": [],
        "@odata.id": "/redfish/v1/AccountService/Roles",
    }
    await validate_keys(body)
    return web.json_response(body, dumps=dumps_bytestr)
