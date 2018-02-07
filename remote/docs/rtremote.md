## [CMCSA-RTREMOTE] 
## Remote Object Location and Communication Protocol (rtRemote)

 
                                                                                                                  									
**TABLE OF CONTENTS**
 - Introduction
 - Flow Chart
 - Functional Specification
	 - 	Object Discovery
	 - Property Get/Set
	 - Method Invocation
 - Messages
	 - 	 Search
	 - Locate
	 - Session Open Request
	 - Session Open Response
	 - Get Property by Name Request
	 - Get Property by Name Response
 - Glossary
 - References

																													

## INTRODUCTION

rtRemote is a protocol and system for object oriented inter-process communication. It is based on the rt object model which uses a dynamically typed system. rtRemote allows an application to register rt objects so that they can be located on a network. It provides a network transparent way for client applications to interact with these objects using a proxy interface.
This document describes the core actors in the system along with the underlying protocol.

// SENDER ID should be pid@host/starttime

----------

## SEQUENCE DIAGRAM
	//TODO.......
	    
----------
## FUNCTIONAL SPECIFICATION
**Object Discovery** : Client can located the object registered (by server) using rtRemoteLocateObject() method. But before registering or locating both server & client should initialize the environment.

Server Side :	
	
	/** Getting a global Environment & Intializing remote environment **/
	rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
	rtRemoteInit(env);

	/** Creating & Registering the Object **/ 
	rtObjectRef obj(new ObjectName());
	rtRemoteRegisterObject(env, "some_name", obj);

Client Side :	
	
	/** Getting a global Environment & Intializing remote environment **/
	rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
	rtRemoteInit(env);

	/** Locating the Object  **/ 
	rtRemoteLocateObject(env, “ObjectName", obj);
	
---	
**Property Get/Set** : Client can set value for the property or get the value of any property 			       using Set() or Get() method respectively.    

	/** Declaring object reference & Value to Set/Get **/ 
	rtObjectRef obj;
	rtValue val;
	
	/** Setting the val of property count **/
	val = 1; 
	obj->Set("count", &val);
	
	/** Getting the val of property count **/
	obj->Get("count", &val);
---
**Method Invocation** 

	//TODO


----------
## MESSAGES

**Search**  : When a client wishes to discover the location of a remote object, it should send a search message.

|Field Name       |Type				    |Description                    |
|-----------------|---------------------|-------------------------------|
|message.type	  |string	            |Message Type                 	|
|object.id		  |string               |Object Identifier            	|
|sender.id	      |int			        |Process ID of Sender		 	|
|correlation.key  |string(uuid)         |Correlation Key				|
|reply-to		  |string			    |Client Address					|

Example : 
		
	{"message.type":"search","object.id":"test.lcd","sender.id":6926,"correlation.key":"62cb9e6b-7c3a-466d-8929-00fdac1e4370","reply-to":"inet:127.0.0.1:40605"}

---		
**Locate** : When a server receive a *search* request from client for remote object, it should response with locate message. 

|Field Name       |Type				    |Description                    |
|-----------------|---------------------|-------------------------------|
|uri			  |string	            |                 				|

Example :

	{"message.type":"locate","object.id":"test.lcd","uri":"unix:///tmp/rt_remote_soc.6922","sender.id":6926,"correlation.key":"62cb9e6b-7c3a-466d-8929-00fdac1e4370"}

---
**Session Open Request** : When a client wishes to start a session, it should send a session open request message.

Example :

	{"message.type":"session.open.request","correlation.key":"dcb73864-b7df-49b5-8c41-66335bf94a34","object.id":"test.lcd"}

---
**Session Open Response** : When a server receive a session open request from client, it should response with session open response message. 

Example :

	{"message.type":"session.open.response","object.id":"test.lcd","correlation.key":"dcb73864-b7df-49b5-8c41-66335bf94a34"}

---
**Keep Alive Request** : When a client/server wishes to keep the session alive, it should send a  keep alive request message.

Example :

	{"message.type":"keep_alive.request","correlation.key":"52dea93c-5aac-4124-9937-a2fc3c50f9f9"}

---
**Keep Alive Response** :  When a server/client receive a keep alive request from client/server, it should response with  keep alive response message.

Example :

	{"correlation.key":"52dea93c-5aac-4124-9937-a2fc3c50f9f9","message.type":"keep_alive.response"}

---
**Set Byname Request** : When a client wishes to set property byname, it should send a  set byname request message.

Example :

	{"message.type":"set.byname.request","object.id":"some_name","property.name":"bigprop","correlation.key":"6cf01ceb-983f-48b8-9413-4d1f5fefde69","value":{"type":111,"value":{"object.id":"obj://99bd49d7-835c-4fbd-a0e8-f6e1376dd827"}}}


---
**Set Byname Response** : When a server receive a set property byname request, it should response by set byname response message.

Example :

	{"message.type":"set.byname.response","correlation.key":"6cf01ceb-983f-48b8-9413-4d1f5fefde69","object.id":"some_name","status.code":0}

---
**Set Byindex Request** : When a client wishes to set property byindex, it should send a  set byindex request message.

Example : #Note : Need to update 

	{"message.type":"set.byindex.request","object.id":"some_name","property.name":"bigprop","correlation.key":"6cf01ceb-983f-48b8-9413-4d1f5fefde69","value":{"type":111,"value":{"object.id":"obj://99bd49d7-835c-4fbd-a0e8-f6e1376dd827"}}}


---
**Set Byindex Response** : When a server receive a set property byindex request, it should response by set byindex response message.

Example : #Note : Need to update

	{"message.type":"set.byindex.response","correlation.key":"6cf01ceb-983f-48b8-9413-4d1f5fefde69","object.id":"some_name","status.code":0}

---
**Get Byname Request** : When a client wishes to get property byname, it should send a  get byname request message.

Example :

	{"message.type":"get.byname.request","object.id":"some_name","property.name":"prop","correlation.key":"b5c7a4be-a750-4e46-8ac3-08adca3e3087"}



---
**Get Byname Response** : When a server receive a get property byname request, it should response by get byname response message.

Example :

	{"message.type":"get.byname.response","correlation.key":"b5c7a4be-a750-4e46-8ac3-08adca3e3087","object.id":"some_name","value":{"type":52,"value":1234},"status.code":0}


---
**Get Byindex Request** : When a client wishes to get property byindex, it should send a  get byindex request message.

Example : 

	{"message.type":"set.byindex.request","object.id":"some_name","property.index":5,"correlation.key":"6a015fbb-a320-4ca6-9f59-91d16b7c96d5","value":{"type":52,"value":10}}


---
**Set Byindex Response** : When a server receive a set property byindex request, it should response by set byindex response message.

Example : 

	{"message.type":"get.byindex.response","correlation.key":"6a015fbb-a320-4ca6-9f59-91d16b7c96d5","object.id":"some_name","value":{"type":52,"value":10},"status.code":0}

---

OTHERS : 

 - Invalid Response
 - Method Call Response
 - Method Call Request
 - Ns Lookup
 - Ns Lookup Response
 - Ns Deregister
 - Ns Deregister Response
 - Ns Update
 - Ns Update Response
 - Ns Register
 - Ns Register Response

----------
## Glossary


----------
## References

----------

