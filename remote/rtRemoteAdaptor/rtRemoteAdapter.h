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

#ifdef __cplusplus 
extern "C" {
#endif 
#include <libparodus.h>
const char *rdk_logger_module_fetch(void);
#ifdef __cplusplus
}
#endif 

#include "rtLog.h"
#include "rtRemoteMessage.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteObjectCache.h"

#define PAYLOAD_SIZE            2048
#define PARAMS_SIZE             128
#define VALUE_SIZE              512

#define CONTENT_TYPE_JSON       "application/json"

using namespace std;
using namespace std::chrono;

class rtRemoteAdapter {

public:
    rtRemoteAdapter();
    ~rtRemoteAdapter();
    rtError init();
    rtError connectParodus();
    void disconnectParodus(); 
    void startProcess();

    /* APIs to process Request */
    rtError processRequest(wrp_msg_t *wrp_payload, char *payload);
    rtError processGetByNameRequest(char *payload,  rtRemoteMessagePtr msg, const char *key, const char *obj_id);
    rtError processMethodCallRequest(char *payload, rtRemoteMessagePtr msg, const char *key, const char *obj_id);
    rtError processSetByNameRequest(char *payload,  rtRemoteMessagePtr msg, char const*key, const char *obj_id);

    /* APIs to create different response */
    void createFailureResponse(char *payload, const char *msg_type, const char *key, const char *obj_id, rtError err);
    void createGetByNameResponse(char *payload, const char *key, const char *obj_id, rtValue value);
    void createSetByNameResponse(char *payload, const char *key, const char *obj_id);
    void createMethodCallResponse(char *payload, const char *key, const char *obj_id, const char* method_name, rtValue result);

    void parodusSend(char *payload, char *source, char *destination, char *trans_uuid, wrp_msg_type msg_type);

private:
    libpd_instance_t m_current_instance;
    rtRemoteEnvironment* m_env;
    char *m_content_type;
    wrp_msg_t m_wrp_msg;
    rtObjectRef m_obj;
    char *m_payload;
    int m_wait;
};

#endif /* __RT_REMOTE_ADAPTER_H__ */

