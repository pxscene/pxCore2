#include "rt_remote_client.h"

const char *rdk_logger_module_fetch(void)
{
        return "LOG.RDK.PARODUS";
}

parodusclient::parodusclient(): env(NULL), e(RT_FAIL) {
    
}

void parodusclient::connectParodus()
{
      
    /* Pass NULL to the Parodus and client URL so that the loop back address (127.0.0.1:6666) will be used
       for communication between Parodus and LibParodus within the same chip set.*/

    libpd_cfg_t cfg1 = {.service_name = "iot", .receive = true, .keepalive_timeout_secs = 64,
                           .parodus_url = NULL, .client_url = "tcp://127.0.0.1:6668", 
                          };
   
    printf("libparodus_init with parodus url %s and client url %s\n",cfg1.parodus_url,cfg1.client_url);
    
    while(1)
    {
        int ret =libparodus_init (&current_instance, &cfg1);
        printf("ret is %d\n",ret);
        if(ret ==0)
        {
            printf("Init for parodus Success..!!\n");
            env = rtEnvironmentGetGlobal();
            e = rtRemoteInit(env);
            if (e != RT_OK)
            {
                printf("rtRemoteInit:%s", rtStrError(e));
            }
            break;
        } else {
            printf("Init for parodus failed: '%d'\n",ret);
        }

        disconnectParodus();
    }
} 

void parodusclient::disconnectParodus() {
    
    int res = RDKC_FAILURE;
       
    res =  libparodus_shutdown(&current_instance);
    printf("libparodus_shutdown respone %d\n", res);
}

void parodusclient::parodusReceive()
{
    int rtn;
    wrp_msg_t *wrp_msg;
    char *payload = NULL;

    while (1)
    {
        rtn = libparodus_receive (current_instance, &wrp_msg, 2000);
        if (rtn == 1)
        {
            printf("Return : %d, Continuing ....\n", rtn);
            continue;
        }
        if (rtn != 0)
        {
            printf("Libparodus failed to recieve message: '%d'\n",rtn);
            sleep(WAIT_TIME);
            continue;
        }
        if ((NULL != wrp_msg) && (wrp_msg->msg_type == WRP_MSG_TYPE__REQ))
        {

            payload = (char*)malloc(PAYLOAD_SIZE);
            memset(payload, 0, sizeof(PAYLOAD_SIZE));
            processRequest(wrp_msg, payload);
            if(NULL != payload) {
                parodusSend(payload, wrp_msg->u.req.source, wrp_msg->u.req.dest, 
                                        wrp_msg->u.req.transaction_uuid, wrp_msg->msg_type );
            } else {
                printf("Invalid request...\n");    
            }
            
        }
        wrp_free_struct (wrp_msg);
        free(payload);
   }
   
   disconnectParodus();
   printf("End of parodus_upstream\n");
   
}

void parodusclient::processGetByNameRequest(char *payload, rtRemoteMessagePtr msg, const char *key, const char *objId) {
    
    char *prop_name     = NULL;
    char *val           = NULL;
    char *val_type      = NULL;
    
    prop_name       = (char*)malloc(PARAMS_SIZE);
    val             = (char*)malloc(VALUE_SIZE);
    val_type        = (char*)malloc(VALUE_SIZE);
    
    memset(prop_name,   0, PARAMS_SIZE);
    memset(val,         0, VALUE_SIZE);
    memset(val_type,    0, VALUE_SIZE);
    
    char const* propName = rtMessage_GetPropertyName(*msg);
    if(propName != NULL) sprintf(prop_name, "%s", propName);
    
    /*Getting the value*/
    rtValue v;
    obj->Get(prop_name, &v);
    sprintf(val_type, "%d", static_cast<int>(v.getType()));
    sprintf(val, "%d", v.toInt32());
    printf("Value : %s , Value Type : %s\n",val, val_type);
     
    /*Getting Response*/
    getByNameResponse(payload, key, objId, val, val_type);

    if(val          != NULL) { free(val); val = NULL;}
    if(val_type     != NULL) { free(val_type); val_type = NULL;}
    if(prop_name    != NULL) { free(prop_name); prop_name = NULL;}   
            
}

void parodusclient::processMethodCallRequest(char *payload, rtRemoteMessagePtr msg, const char *key, const char *objId) {
    
    vector<rtValue> argv;        
    char *method_name   = NULL;
    char *result        = NULL;
    
    method_name     = (char*)malloc(PARAMS_SIZE);
    result          = (char*)malloc(PARAMS_SIZE);
    
    memset(method_name, 0, PARAMS_SIZE);
    memset(result, 0, PARAMS_SIZE);
    
    auto func_itr = msg->FindMember(kFieldNameFunctionName);
    auto args_itr = msg->FindMember(kFieldNameFunctionArgs);
        
    if (func_itr != msg->MemberEnd()) { 
        auto funcName = func_itr->value.GetString();
        if(funcName != NULL) sprintf(method_name, "%s", funcName);
    }
        
    if (args_itr != msg->MemberEnd()) { 
        for (rapidjson::Value::ConstValueIterator itr = args_itr->value.Begin(); itr != args_itr->value.End(); ++itr) { 
            auto val_itr = itr->FindMember(kFieldNameValue);
            auto vat_itr = itr->FindMember(kFieldNameValueType);
            
            if (val_itr != msg->MemberEnd()) { 
                rtValue arg(val_itr->value.GetInt());
                argv.push_back(arg);
            } 
        } 
    } 

    /* Method Call */
    rtValue res;
    std::string methodName((const char*)method_name);
    if(!argv.empty()) {
 
        rtFunctionRef func;
        rtError err = RT_OK; 
               
        /* result "RT_FAIL=1"  as default */
        sprintf(result, "%d", RT_FAIL);
                
        if (obj) {
            err = obj.get<rtFunctionRef>(method_name, func);
        } else {
            func = env->ObjectCache->findFunction(methodName);
        }

        if (err == RT_OK && !!func)
        {
            rtError e = func->Send(static_cast<int>(argv.size()), (rtValue const*)&argv[0], &res);
                    
            if(e == RT_OK) {
                sprintf(result, "%d", res.toInt32());
                printf("result : %s\n", result); 
            } 
        } 
    }
       
    /*Getting response */        
    methodCallResponse(payload, key, objId, method_name, result);
    
    if(method_name  != NULL) { free(method_name); method_name = NULL;}
    if(result       != NULL) { free(result); result = NULL;}
    
}    
void parodusclient::processSetByNameRequest(char *payload,  rtRemoteMessagePtr msg, const char *key, const char *objId) {
    int val         = -1;
    int val_type    = -1;
    char *prop_name     = NULL;
    
    prop_name       = (char*)malloc(PARAMS_SIZE);
    memset(prop_name,   0, PARAMS_SIZE);
  
    char const* propName = rtMessage_GetPropertyName(*msg);
    if(propName != NULL) sprintf(prop_name, "%s", propName);
        
    auto itr = msg->FindMember(kFieldNameValue);
    if (itr != msg->MemberEnd()) {
        auto val_itr = itr->value.FindMember(kFieldNameValue);
        auto vat_itr = itr->value.FindMember(kFieldNameValueType);
        
        if (val_itr != msg->MemberEnd()) { val      = val_itr->value.GetInt();}
        if (vat_itr != msg->MemberEnd()) { val_type = vat_itr->value.GetInt();}
    }
            
    /* Setting the value */
    rtValue v(val);
    obj->Set(prop_name, &v); 
    
    /* Getting response  */
    setByNameResponse(payload, key, objId);
    
    if(prop_name    != NULL) { free(prop_name); prop_name = NULL;}
}


void parodusclient::processRequest(wrp_msg_t *wrp_msg, char *payload) {
    
    const char* wrpPayload =  reinterpret_cast<const char*>(wrp_msg->u.req.payload);
    rapidjson::Document *doc = new rapidjson::Document();
    doc->Parse(wrpPayload);
    rtRemoteMessagePtr msg =  std::shared_ptr<rtRemoteMessage>(doc);
    
    char const* msgType = rtMessage_GetMessageType(*msg);
    char const* objId   = rtMessage_GetObjectId(*msg);
    char const* key     = rtMessage_GetCorrelationKey(*msg).toString().c_str();
    
    while ((e = rtRemoteLocateObject(env, "some_name", obj)) != RT_OK)
    {
        printf("still looking:%s", rtStrError(e));
    }

    if(msgType != NULL && key != NULL && objId != NULL) { 
        if (strcmp(msgType,"set.byname.request") == 0) {
            processSetByNameRequest(payload, msg, key, objId);
        } else if (strcmp(msgType,"get.byname.request") == 0) {
            processGetByNameRequest(payload, msg, key, objId);
        } else if (strcmp(msgType, "method.call.request") == 0) {
            processMethodCallRequest(payload, msg, key, objId);
        }
    } else {
        printf("Invalid data....\n");
    }
}

void parodusclient::methodCallResponse(char *payload, const char *key, const char *obj_id, const char* method_name, const char* result) {
    rtRemoteMessagePtr res(new rapidjson::Document());
    res->SetObject();
    
    res->AddMember(kFieldNameMessageType, kMessageTypeMethodCallResponse, res->GetAllocator());
    res->AddMember(kFieldNameFunctionName, rapidjson::StringRef(method_name, strlen(method_name)), res->GetAllocator());
    res->AddMember(kFieldNameFunctionReturn, rapidjson::StringRef(result, strlen(result)), res->GetAllocator());
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef(key, strlen(key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef(obj_id, strlen(obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());

    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

void parodusclient::setByNameResponse(char *payload, const char *key, const char *obj_id) {
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

void parodusclient::getByNameResponse(char *payload, const char *key, const char *obj_id, const char *value, const char *val_type) {
    rtRemoteMessagePtr res(new rapidjson::Document());
    rapidjson::Value val;
    val.SetObject();
    res->SetObject();
    
    val.AddMember(kFieldNameValueType, atoi(val_type), res->GetAllocator());
    val.AddMember(kFieldNameValue, atoi(value), res->GetAllocator());

    res->AddMember(kFieldNameValue, val, res->GetAllocator());
    res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef(key, strlen(key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef(obj_id, strlen(obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator()); 
    res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());
    
    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

void parodusclient::parodusSend(char *payload, char *source, char *destination, char *trans_uuid, wrp_msg_type msg_type)
{
    
    wrp_msg_t *res_wrp_msg;
    char *contentType = NULL;
    int res = RDKC_FAILURE; 
    
    res_wrp_msg = (wrp_msg_t *)malloc(sizeof(wrp_msg_t));
    memset(res_wrp_msg, 0, sizeof(wrp_msg_t));
    res_wrp_msg->u.req.payload = (char *)malloc(sizeof(char)*(1096));
    res_wrp_msg->u.req.payload = (void *)payload;
    printf("Response payload is %s\n",(char *)(res_wrp_msg->u.req.payload));
    res_wrp_msg->u.req.payload_size = strlen((const char*)res_wrp_msg->u.req.payload);
    res_wrp_msg->msg_type = msg_type;
    res_wrp_msg->u.req.source = destination;
    res_wrp_msg->u.req.dest = source;
    res_wrp_msg->u.req.transaction_uuid = trans_uuid;
    contentType = (char *)malloc(sizeof(char)*(strlen(CONTENT_TYPE_JSON)+1));
    strncpy(contentType,CONTENT_TYPE_JSON,strlen(CONTENT_TYPE_JSON)+1);
    res_wrp_msg->u.req.content_type = contentType;
    res = libparodus_send(current_instance, res_wrp_msg);
    if(res == 0)
    {
        printf("Sent message successfully to parodus\n");
    } else {
        printf("Failed to send message: '%d'\n",res);   
    }
}


void parodusclient::rtRemoteClientMgr()
{
    connectParodus();
}

int main(int argc, char** argv)
{
   parodusclient *client = new parodusclient();
   client->rtRemoteClientMgr();
   client->parodusReceive();

   return 0;
}
