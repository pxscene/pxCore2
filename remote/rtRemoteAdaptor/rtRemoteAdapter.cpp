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

#include "rtRemoteAdapter.h"

const char *rdk_logger_module_fetch(void)
{
        return "LOG.RDK.PARODUS";
}

rtRemoteAdapter::rtRemoteAdapter()
    : m_env(NULL)
    , m_current_instance(NULL) 
    , m_obj(NULL)
    , m_wait(2) {
    
}

/** @description : connect the parodus.
 *  @param : none.
 *  @return : void.
 */
rtError rtRemoteAdapter::connectParodus() {
      
    /* Pass NULL to the Parodus and client URL so that the loop back address (127.0.0.1:6666) will be used
       for communication between Parodus and LibParodus within the same chip set.*/
    rtError err = RT_FAIL;
    libpd_cfg_t cfg1 = {.service_name = "iot", .receive = true, .keepalive_timeout_secs = 64,
                           .parodus_url = NULL, .client_url = "tcp://127.0.0.1:6668", 
                          };
   
    rtLogDebug("libparodus_init with parodus url %s and client url %s",cfg1.parodus_url,cfg1.client_url);
    
    while(1) {
        int ret =libparodus_init (&m_current_instance, &cfg1);
        if(ret ==0) {
            rtLogInfo("Init for parodus Success..!!");
            break;
        } else {
            rtLogError("Init for parodus failed: '%d'",ret);
        }
        disconnectParodus();
    }
    
    m_env = rtEnvironmentGetGlobal();
    err = rtRemoteInit(m_env);
    return err;

} 

/** @description : disconnect the parodus.
 *  @param : none.
 *  @return : void.
 */
void rtRemoteAdapter::disconnectParodus() {
    int res = RT_FAIL;       
    res =  libparodus_shutdown(&m_current_instance);
    rtLogDebug("Failed to disconnect parodus: '%d'",res);
}

/** @description : receive data from parodus server.
 *  process the recieve data. 
 *  @param : none.
 *  @return : void.
 */
void rtRemoteAdapter::startProcess() {
    int rtn;
    rtError err = RT_FAIL;
    wrp_msg_t *wrp_msg;
    char *payload = new char[PAYLOAD_SIZE];

    while (1) {
        rtn = libparodus_receive (m_current_instance, &wrp_msg, 2000);
        if (rtn == 1) {
            rtLogInfo("Return : %d, Continuing ....", rtn);
            continue;
        }
        if (rtn != 0) {
            rtLogDebug("Libparodus failed to recieve message: '%d'",rtn);
            std::this_thread::sleep_for(std::chrono::seconds(m_wait));
            continue;
        }
        if ((wrp_msg != NULL) && (wrp_msg->msg_type == WRP_MSG_TYPE__REQ)) {
            memset(payload, 0, PAYLOAD_SIZE);
            err = processRequest(wrp_msg, payload);
            if(err == RT_OK && payload != NULL) {
                parodusSend(payload, wrp_msg->u.req.source, wrp_msg->u.req.dest, 
                                        wrp_msg->u.req.transaction_uuid, wrp_msg->msg_type );
            } else {
                rtLogError("Failed to process the request.");    
            }
            
        }
        wrp_free_struct (wrp_msg);
   }
  
   delete []payload;
   disconnectParodus();
   
}

/** @description : process the GetByName request.
 *  @param : payload recived, rtRemote message pointer
 *  correlation key, object id.
 *  @return : rtError.
 */
rtError rtRemoteAdapter::processGetByNameRequest(char *payload, rtRemoteMessagePtr msg, const char *key, const char *objId) {
    rtValue v;
    rtError err = RT_FAIL;
    char const* propName = rtMessage_GetPropertyName(*msg);
    
    /*Getting the value*/
    err = m_obj->Get(propName, &v);
    if(err == RT_OK) { 
        getByNameResponse(payload, key, objId, v);
        return err;        
    }
    failureResponse(payload, kMessageTypeGetByNameResponse, key, objId);
    return err;
}

/** @description : process the MethodCall request.
 *  @param : payload recived, rtRemote message pointer
 *  correlation key, object id.
 *  @return : rtError.
 */
rtError rtRemoteAdapter::processMethodCallRequest(char *payload, rtRemoteMessagePtr msg, const char *key, const char *objId) {
    
    vector<rtValue> argv;        
    rtValue res;
    rtFunctionRef func;
    rtError err = RT_FAIL; 
    
    auto func_itr = msg->FindMember(kFieldNameFunctionName);
        
    if (func_itr != msg->MemberEnd()) { 
        auto args_itr = msg->FindMember(kFieldNameFunctionArgs);
        auto funcName = func_itr->value.GetString();
        
        if (args_itr != msg->MemberEnd()) { 
            for (rapidjson::Value::ConstValueIterator itr = args_itr->value.Begin(); itr != args_itr->value.End(); ++itr) { 
                rtValue arg;
                rtRemoteValueReader::read(m_env, arg, *itr, NULL); 
                argv.push_back(arg);
            } 

            /* Method Call */
            if(!argv.empty()) {
                if (m_obj) {
                    err = m_obj.get<rtFunctionRef>(funcName, func);
                } else {
                    func = m_env->ObjectCache->findFunction(funcName);
                }
                if (err == RT_OK && !!func) {
                    err = func->Send(static_cast<int>(argv.size()), (rtValue const*)&argv[0], &res);
                    if(err == RT_OK) {
                        methodCallResponse(payload, key, objId, funcName, res);
                        return err;
                    }
                }
            }
        }
    } 
    failureResponse(payload, kMessageTypeMethodCallResponse, key, objId);    
    return err;   
    
}    

/** @description : process the SetByName request.
 *  @param : payload recived, rtRemote message pointer
 *  correlation key, object id.
 *  @return : rtError.
 */
rtError rtRemoteAdapter::processSetByNameRequest(char *payload,  rtRemoteMessagePtr msg, char const* key, const char *objId) {
  
    rtValue value;
    rtError err = RT_FAIL;
    const char *propName = rtMessage_GetPropertyName(*msg);
        
    auto itr = msg->FindMember(kFieldNameValue);
    if (itr != msg->MemberEnd()) {
        err = rtRemoteValueReader::read(m_env, value, itr->value, NULL);
        
        if(err == RT_OK) { 
           /* Setting the value */
            err = m_obj->Set(propName, &value); 
            if(err == RT_OK) { 
                setByNameResponse(payload, key, objId);
                return err;
            }
        }
    }
    failureResponse(payload, kMessageTypeSetByNameResponse, key, objId);    
    return err;    
}

/** @description : process the rtRemote request.
 *  @param : msg recived, payload.
 *  @return : rtError.
 */
rtError rtRemoteAdapter::processRequest(wrp_msg_t *wrp_msg, char *payload) {
    
    rtError err = RT_FAIL; 
    
    const char *wrpPayload =  reinterpret_cast<const char*>(wrp_msg->u.req.payload);
    rtRemoteMessagePtr msg(new rapidjson::Document());
    msg->Parse(wrpPayload);

    const char *objId   = rtMessage_GetObjectId(*msg);
    while ((err = rtRemoteLocateObject(m_env, objId, m_obj)) != RT_OK) {
        rtLogDebug("still looking:%s", rtStrError(err));
    }

    const char *msgType = rtMessage_GetMessageType(*msg);
    const char *key= rtMessage_GetCorrelationKey(*msg).toString().c_str();
    if(msgType != NULL && key != NULL && objId != NULL) { 
        if (strcmp(msgType,"set.byname.request") == RT_OK) {
            err = processSetByNameRequest(payload, msg, key, objId);
        } else if (strcmp(msgType,"get.byname.request") == RT_OK) {
            err = processGetByNameRequest(payload, msg, key, objId);
        } else if (strcmp(msgType, "method.call.request") == RT_OK) {
            err = processMethodCallRequest(payload, msg, key, objId);
        }
    } else {
        rtLogError("Invalid data....");
    }

    delete []key;
    return err;
}

/** @description : creating response payload in case of error.
 *  @param : response payload, correlation key, object id.
 *  @return : void.
 */
void rtRemoteAdapter::failureResponse(char *payload, const char *msg_type, const char *key, const char *obj_id) {
    rtRemoteMessagePtr res(new rapidjson::Document());
    res->SetObject();

    res->AddMember(kFieldNameMessageType,  rapidjson::StringRef(msg_type, strlen(msg_type)), res->GetAllocator());
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef(key, strlen(key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef(obj_id, strlen(obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameStatusCode, 1, res->GetAllocator());
    
    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

/** @description : creating response payload for method request.
 *  @param : response payload, correlation key, 
 *  object id, method_name. result.
 *  @return : void.
 */
void rtRemoteAdapter::methodCallResponse(char *payload, const char *key, const char *obj_id, const char* method_name, rtValue result) {
    rtRemoteMessagePtr res(new rapidjson::Document());
    res->SetObject();
    
    rapidjson::Value val;
    rtRemoteValueWriter::write(m_env, result, val, *res);
    res->AddMember(kFieldNameMessageType, kMessageTypeMethodCallResponse, res->GetAllocator());
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef(key, strlen(key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef(obj_id, strlen(obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameFunctionName, rapidjson::StringRef(method_name, strlen(method_name)), res->GetAllocator());
    res->AddMember(kFieldNameFunctionReturn, val, res->GetAllocator());
    res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());

    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

/** @description : creating response payload for SetByName request.
 *  @param : response payload, correlation key, object id.
 *  @return : void.
 */
void rtRemoteAdapter::setByNameResponse(char *payload, const char *key, const char *obj_id) {
    
    rtRemoteMessagePtr res(new rapidjson::Document());
    res->SetObject();
    
    res->AddMember(kFieldNameMessageType, kMessageTypeSetByNameResponse, res->GetAllocator());
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef(key, strlen(key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef(obj_id, strlen(obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());
    
    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

/** @description : creating response payload for GetByName request.
 *  @param : response payload, correlation key, object id.
 *  @return : void.
 */
void rtRemoteAdapter::getByNameResponse(char *payload, const char *key, const char *obj_id, rtValue value) {
    rtRemoteMessagePtr res(new rapidjson::Document());
    res->SetObject();

    rapidjson::Value val;
    rtRemoteValueWriter::write(m_env, value, val, *res);
    res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef(key, strlen(key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef(obj_id, strlen(obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameValue, val, res->GetAllocator());
    res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());
    
    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

/** @description : sending the response.
 *  @param : response payload, source, destination, uuid, msg type.
 *  @return : void.
 */
void rtRemoteAdapter::parodusSend(char *payload, char *source, char *destination, char *trans_uuid, wrp_msg_type msg_type) {
    
    wrp_msg_t *res_wrp_msg;
    char *contentType = NULL;
    int res = RT_FAIL; 
    
    res_wrp_msg = new wrp_msg_t();
    res_wrp_msg->u.req.payload = (void *)payload;
    res_wrp_msg->u.req.payload_size = strlen((const char*)res_wrp_msg->u.req.payload);
    res_wrp_msg->msg_type = msg_type;
    res_wrp_msg->u.req.source = destination;
    res_wrp_msg->u.req.dest = source;
    res_wrp_msg->u.req.transaction_uuid = trans_uuid;
    contentType = new char[sizeof(char)*(strlen(CONTENT_TYPE_JSON)+1)];
    strncpy(contentType,CONTENT_TYPE_JSON,strlen(CONTENT_TYPE_JSON)+1);
    res_wrp_msg->u.req.content_type = contentType;
    res = libparodus_send(m_current_instance, res_wrp_msg);
    if(res == 0) {
        rtLogInfo("Sent message successfully");
    } else {
        rtLogError("Failed to send message: '%d'",res);   
    }

    delete []contentType;
    delete res_wrp_msg; 
}

/** @description : initializes the process.
 *  @param : none.
 *  @return : rtError.
 */
rtError rtRemoteAdapter::init() {
    return connectParodus();
}

int main(int argc, char** argv) {
    
    rtError err = RT_FAIL;
    rtRemoteAdapter adapter;
    rtLogLevel logLevel = RT_LOG_INFO;
    rtLogSetLevel(logLevel);
    
    err = adapter.init();
    if(err == RT_OK) { adapter.startProcess();}
    else { rtLogError("Failed to initialized");}

   return 0;
}

