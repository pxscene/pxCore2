/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef __RT_REMOTE_ADAPTER_H__
#define __RT_REMOTE_ADAPTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <memory>
#include <algorithm>
#ifdef __cplusplus 
extern "C" {
#endif 
#include <libparodus.h>
const char *rdk_logger_module_fetch(void);
#ifdef __cplusplus
}
#endif 

#include "cJSON.h"
#include "rtRemote.h"
#include "rtRemoteMessage.h"
#include "rtRemoteCorrelationKey.h"
#include "rtRemoteFunction.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteValueWriter.h"
#include "rtRemoteObjectCache.h"

#include <cimplog/cimplog.h>
#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


#define RDKC_SUCCESS            0
#define RDKC_FAILURE            -1

#define WAIT_TIME               2
#define PAYLOAD_SIZE            2048
#define PARAMS_SIZE             128
#define VALUE_SIZE              512


#define ENDPOINT                "tcp://127.0.0.1:6666"
#define CONTENT_TYPE_JSON       "application/json"
#define DEVICE_PROPS_FILE       "/etc/device.properties"
#define CLIENT_PORT_NUM         6667
#define URL_SIZE                64

using namespace std;

class rtRemoteAdapter {

public:
    rtRemoteAdapter();
    void rtRemoteClientMgr();

    void connectParodus();
    void disconnectParodus(); 
    void startProcess();
    void parodusSend(char *payload, char *source, char *destination, char *trans_uuid, wrp_msg_type msg_type);

    void getByNameResponse(char *payload, const char *corrltn_key, const char *obj_id, const char *value, const char *val_type);
    void setByNameResponse(char *payload, const char *corrltn_key, const char *obj_id);
    void methodCallResponse(char *payload, const char *corrltn_key, const char *obj_id, const char* method_name, const char* result);
    void processRequest(wrp_msg_t *wrp_payload, char *payload);
    void processGetByNameRequest(char *payload,  rtRemoteMessagePtr msg, const char *key, const char *objId);
    void processMethodCallRequest(char *payload, rtRemoteMessagePtr msg, const char *key, const char *objId);
    void processSetByNameRequest(char *payload,  rtRemoteMessagePtr msg, const char *key, const char *objId);

private:
    libpd_instance_t current_instance;
    rtRemoteEnvironment* env;
    rtError e;
    rtObjectRef obj;

};

#endif /* __RT_REMOTE_ADAPTER_H__ */

