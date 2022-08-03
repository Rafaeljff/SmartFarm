#!/usr/bin/env python3

# This file is part of the Python aiocoap library project.
#
# Copyright (c) 2012-2014 Maciej Wasilak <http://sixpinetrees.blogspot.com/>,
#               2013-2014 Christian Amsüss <c.amsuess@energyharvesting.at>
#
# aiocoap is free software, this file is published under the MIT license as
# described in the accompanying LICENSE file.

"""This is a usage example of aiocoap that demonstrates how to implement a
simple server. See the "Usage Examples" section in the aiocoap documentation
for some more information."""

import datetime
import logging

import asyncio

import aiocoap.resource as resource
import aiocoap


savedResource="empty"
temp1= "empty"
temp2 = "empty"
temp3 = "empty"
hum = "empty"
soil= "empty"
mensagem=""

class BlockResource(resource.Resource):
    """Example resource which supports the GET and PUT methods. It sends large
    responses, which trigger blockwise transfer."""

    def __init__(self):
        super().__init__()
        self.set_content(b"This is the resource's default content. It is padded "
                b"with numbers to be large enough to trigger blockwise "
                b"transfer.\n")

    def set_content(self, content):
        self.content = content
        while len(self.content) <= 1024:
            self.content = self.content + b"0123456789\n"

    async def render_get(self, request):
        return aiocoap.Message(payload=self.content)

    async def render_put(self, request):
        print('PUT payload: %s' % request.payload)
        self.set_content(request.payload)
        return aiocoap.Message(code=aiocoap.CHANGED, payload=self.content)


class SeparateLargeResource(resource.Resource):
    """Example resource which supports the GET method. It uses asyncio.sleep to
    simulate a long-running operation, and thus forces the protocol to send
    empty ACK first. """

    def get_link_description(self):
        # Publish additional data in .well-known/core
        return dict(**super().get_link_description(), title="A large resource")

    async def render_get(self, request):
        await asyncio.sleep(3)

        payload = "Three rings for the elven kings under the sky, seven rings "\
                "for dwarven lords in their halls of stone, nine rings for "\
                "mortal men doomed to die, one ring for the dark lord on his "\
                "dark throne.".encode('ascii')
        return aiocoap.Message(payload=payload)

class TimeResource(resource.ObservableResource):
    """Example resource that can be observed. The `notify` method keeps
    scheduling itself, and calles `update_state` to trigger sending
    notifications."""

    def __init__(self):
        super().__init__()

        self.handle = None

    def notify(self):
        self.updated_state()
        self.reschedule()

    def reschedule(self):
        self.handle = asyncio.get_event_loop().call_later(5, self.notify)

    def update_observation_count(self, count):
        if count and self.handle is None:
            print("Starting the clock")
            self.reschedule()
        if count == 0 and self.handle:
            print("Stopping the clock")
            self.handle.cancel()
            self.handle = None

    async def render_get(self, request):
        payload = datetime.datetime.now().\
                strftime("%Y-%m-%d %H:%M").encode('ascii')
        return aiocoap.Message(payload=payload)

    
class LocalProcessing(resource.Resource):
    def __init__(self):
        super().__init__()
        self._savedResource="empty"
    
    async def render_get(self, request): 
        value=int(mensagem)
        print("mensagemeeeeeeeeeeeeeeeeeee"+mensagem)    
        if value > 30 :
            self._savedResource="atuador_on"
        elif value < 30  :
            self._savedResource="atuador_off"
        else:
            self._savedResource="inconcrete"
        return aiocoap.Message(content_format=0,
                payload=self._savedResource.encode('utf8'))

    #async def render_put(self, request):
     #   self._savedResource2 = request.payload.decode("utf8")
      #  return aiocoap.Message(content_format=0,
       #         payload="OK".encode('utf8'))

    
class ResourceTemperature1(resource.Resource):
    def __init__(self):
        super().__init__()
        self._temp1="empty"
        
   
    async def render_get(self, request):

        return aiocoap.Message(content_format=0,
                payload=self._temp1.encode('utf8'))
    async def render_put(self, request):
        global mensagem
        self._temp1= request.payload.decode("utf8")
        mensagem=self._temp1
        return aiocoap.Message(content_format=0,
                payload="received temperature of sensor 1".encode('utf8'))

class ResourceTemperature2(resource.Resource):
    def __init__(self):
        super().__init__()
        self._temp2= "empty"

    async def render_get(self, request):
        return aiocoap.Message(content_format=0,
                payload=self._temp2.encode('utf8'))
    
    async def render_put(self, request):
        self._temp2= request.payload.decode("utf8")
        return aiocoap.Message(content_format=0,
                payload="received temperature of sensor 2".encode('utf8'))


class ResourceTemperature3(resource.Resource):
    def __init__(self):
        super().__init__()
        self._temp3= "empty"

    async def render_get(self, request):
        return aiocoap.Message(content_format=0,
                payload=self._temp3.encode('utf8'))
    
    async def render_put(self, request):
        self._temp3= request.payload.decode("utf8")
        return aiocoap.Message(content_format=0,
                payload="received temperature of sensor 3".encode('utf8'))

class ResourcePressure(resource.Resource):
    def __init__(self):
        super().__init__()
        self._hum= "empty"

    async def render_get(self, request):
        return aiocoap.Message(content_format=0,
                payload=self._hum.encode('utf8'))
    
    async def render_put(self, request):
        self.hum= request.payload.decode("utf8")
        return aiocoap.Message(content_format=0,
                payload="received humity ".encode('utf8'))

class ResourceSoil(resource.Resource):
    def __init__(self):
        super().__init__()
        self._soil="empty"

    async def render_get(self, request):
        return aiocoap.Message(content_format=0,
                payload=self._soil.encode('utf8'))
    
    async def render_put(self, request):
        self._soil=request.payload.decode('utf8')
        return aiocoap.Message(content_format=0,
                payload="received soil moisture".encode('utf8'))


class WhoAmI(resource.Resource):
    async def render_get(self, request):
        text = ["Used protocol: %s." % request.remote.scheme]

        text.append("Request came from %s." % request.remote.hostinfo)
        text.append("The server address used %s." % request.remote.hostinfo_local)

        claims = list(request.remote.authenticated_claims)
        if claims:
            text.append("Authenticated claims of the client: %s." % ", ".join(repr(c) for c in claims))
        else:
            text.append("No claims authenticated.")

        return aiocoap.Message(content_format=0,
                payload="\n".join(text).encode('utf8'))

# logging setup
logging.basicConfig(level=logging.INFO)
logging.getLogger("coap-server").setLevel(logging.DEBUG)

async def main():

    # Resource tree creation
    root = resource.Site()

    root.add_resource(['.well-known', 'core'],
            resource.WKCResource(root.get_resources_as_linkheader))
    root.add_resource(['time'], TimeResource())
    root.add_resource(['other', 'block'], BlockResource())
    root.add_resource(['other', 'separate'], SeparateLargeResource())
    root.add_resource(['whoami'], WhoAmI())
    root.add_resource(['managertemp1'], ResourceTemperature1())
    root.add_resource(['managertemp2'], ResourceTemperature2())
    root.add_resource(['managertemp3'], ResourceTemperature3())
    root.add_resource(['managerpressure'], ResourcePressure())
    root.add_resource(['managersoil'], ResourceSoil())
    root.add_resource(['manager'],LocalProcessing())
    await aiocoap.Context.create_server_context(root)

    # Run forever
    await asyncio.get_running_loop().create_future()

if __name__ == "__main__":
    asyncio.run(main())
