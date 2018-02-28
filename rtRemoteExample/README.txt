
BUILDING client and server on linux (rtRemote is supported only on linux):
1) build pxCore as usual
2) cd ${PXCOREDIR}/remote
3) make
4) cd ../rtRemoteExample
5) make
6) run ./sample_client and ./sample_server

ARCHITECTURE:
rtRemote is peer to peer remote wrapper around pxCore objects.
there is no explicit server or client.
the communication is as follows:
peers are hosting their objects and communicate (set/get their properties) with remote objects.

the communication is done via udp and packet format is json

there are 2 types of objects:
1) host objects (are registered by rtRemoteRegisterObject)
2) remote objects(are located by rtRemoteLocateObject)

objects are identified by name.
locating object host peer can be done via multicast or external name server,
by default it is done by multicast (rt.rpc.resolver.type config-var)

host objects are located on its host peer, 
remote objects are used to get/set properties of host objects

in pxcore there several kinds of properties:

when you set a property via remote object the behavior on host object is as follows:

a) properties of these types
    RT_voidType
    RT_boolType
    RT_int8_tType
    RT_uint8_tType
    RT_int32_tType
    RT_uint32_tType
    RT_int64_tType
    RT_uint64_tType
    RT_floatType
    RT_doubleType
    RT_stringType
are set immediately to their corresponding data fields in host object

b) RT_functionType
on host object this is represented as rtRemoteFunction,
and when host calls it - it is invoked on remote peer who set it previously

c) RT_objectType
on host object this is represented as rtRemoteObject (remote object analog),

rtRemote is configured via external config (rtremote.conf):
which must be located in current directory,
here are explanations of some important variables:

- rt.rpc.resolver.type - multicast|file|unicast (default: multicast)

unicast - resolving objectId to its host peer (hostname,port) is done
via external server (rt.rpc.resolver.unicast.address and rt.rpc.resolver.unicast.port correspondingly)
