#include "rt_remote_client.h"

const char *rdk_logger_module_fetch(void)
{
        return "LOG.RDK.PARODUS";
}

parodusclient::parodusclient(): env(NULL), e(NULL) {
    
}

void parodusclient::connectParodus()
{
    printf("*********** connectParodus ******************\n");
      
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
    printf("*********** Parodus Receive ******************\n");
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
            rtRemoteProcess(wrp_msg, payload);
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

void parodusclient::rtRemoteProcess(wrp_msg_t *wrp_msg, char *payload) {
    
    printf("***********rtRemoteProcess******************\n");
    char *msg_type      = NULL;
    char *corrltn_key   = NULL;
    char *obj_id        = NULL; 
    char *prop_name     = NULL;
    char *val           = NULL;
    char *val_type      = NULL;

    msg_type        = (char*)malloc(PARAMS_SIZE);
    corrltn_key     = (char*)malloc(PARAMS_SIZE);
    obj_id          = (char*)malloc(PARAMS_SIZE);
    prop_name       = (char*)malloc(PARAMS_SIZE);
    val             = (char*)malloc(VALUE_SIZE);
    val_type        = (char*)malloc(VALUE_SIZE);
    
    memset(msg_type,    0, PARAMS_SIZE);
    memset(obj_id,      0, PARAMS_SIZE);
    memset(prop_name,   0, PARAMS_SIZE);
    memset(val,         0, PARAMS_SIZE);
    memset(val_type,    0, PARAMS_SIZE);
    memset(corrltn_key, 0, PARAMS_SIZE);

    /* Get params and process accordingly */
    getParsed(wrp_msg->u.req.payload, corrltn_key, msg_type, obj_id, prop_name, val, val_type);

    if(NULL != msg_type) {
    
        if (e != RT_OK)
        {
            printf("rtRemoteInit:%s", rtStrError(e));
            exit(1);
        }

        while ((e = rtRemoteLocateObject(env, "some_name", obj)) != RT_OK)
        {
            printf("still looking:%s", rtStrError(e));
        }
       
        if(strcmp(msg_type, "set.byname.request") == 0){
            printf("setting the value....\n");
            rtValue v(atoi(val));
            obj->Set(prop_name, &v); 
            getResponse(payload, corrltn_key, msg_type, obj_id,val, val_type);
        } else if(strcmp(msg_type, "get.byname.request") == 0){
            rtValue v;
            obj->Get(prop_name, &v);
            sprintf(val_type, "%d", static_cast<int>(v.getType()));
            sprintf(val, "%d", v.toInt32());
            printf("Get the value ..%s , %s\n",val, val_type);
            getResponse(payload, corrltn_key, msg_type, obj_id,val, val_type);
        } else if(strcmp(msg_type, "method.call.request") == 0){
        /*
                rtFunctionRef callback(new rtFunctionCallback(upload_complete));
                obj.set("onUploadComplete", callback);   
                rtValue vl;              

                e = obj->Get("onUploadComplete", &vl);
                printf("Get:\n%s\n", rtStrError(e));
                printf("Type:%s\n", vl.getTypeStr());
                printf("Addr:%p\n", vl.toFunction().getPtr());   
                sprintf(payload, "%s", (const char*)cJSON_Print(params));
            */
        } else {
            printf("Invalid msg_type");
        }
    } else {
        payload = NULL;
        printf("Message type null");    
    }
    
    free(msg_type);
    free(obj_id);
    free(prop_name);
    free(corrltn_key);
    free(val);
    free(val_type);

}

void parodusclient::getParsed(void *wrp_payload, char *corr_key, char *msg_type, char *obj_id, char *prop_name, char *value_v, char* v_type) {
    
    printf("***********Parsing....******************\n");
    
    const char* payload =  reinterpret_cast<const char*>(wrp_payload);
    rapidjson::Document *doc = new rapidjson::Document();
    doc->Parse(payload);
    
    rtRemoteMessagePtr msg =  std::shared_ptr<rtRemoteMessage>(doc);
    
    char const* msgType         = rtMessage_GetMessageType(*msg);
    char const* objId           = rtMessage_GetObjectId(*msg);
    char  const* propName       = rtMessage_GetPropertyName(*msg);
    char  const* key            = rtMessage_GetCorrelationKey(*msg).toString().c_str();
    
    if(msgType  != NULL) sprintf(msg_type,  "%s", msgType);
    if(objId    != NULL) sprintf(obj_id,    "%s", objId);
    if(propName != NULL) sprintf(prop_name, "%s", propName);
    if(key      != NULL) sprintf(corr_key,  "%s", key);
    
    if (strcmp(msg_type,"set.byname.request") == 0) {
        int val         = -1;
        int val_type    = -1;

        auto itr = msg->FindMember(kFieldNameValue);
        if (itr != msg->MemberEnd()) {
            auto val_itr = itr->value.FindMember(kFieldNameValue);
            auto vat_itr = itr->value.FindMember(kFieldNameValueType);
        
            if (val_itr != msg->MemberEnd()) { val      = val_itr->value.GetInt();}
            if (vat_itr != msg->MemberEnd()) { val_type = vat_itr->value.GetInt();}
            sprintf(value_v, "%d", val);
            sprintf(v_type,  "%d", val_type);
        }
    }

}
void parodusclient::getResponse(char *payload, char *key, char *msg_type, char *obj_id, char *value, char *val_type) {
        
    rtRemoteMessagePtr res(new rapidjson::Document());
    res->SetObject();
    if(strcmp(msg_type, "set.byname.request") == 0){
        res->AddMember(kFieldNameMessageType, kMessageTypeSetByNameResponse, res->GetAllocator());
    } else if(strcmp(msg_type, "get.byname.request") == 0){
        rapidjson::Value val;
        val.SetObject();
        val.AddMember(kFieldNameValueType, atoi(val_type), res->GetAllocator());
        val.AddMember(kFieldNameValue, atoi(value), res->GetAllocator());
        
        res->AddMember(kFieldNameValue, val, res->GetAllocator());
        res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
        
    }
    
    res->AddMember(kFieldNameCorrelationKey, rapidjson::StringRef((const char*)key, strlen((const char*)key)), res->GetAllocator());
    res->AddMember(kFieldNameObjectId,  rapidjson::StringRef((char const*)obj_id, strlen((char const*)obj_id)), res->GetAllocator());
    res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());

    rapidjson::StringBuffer buff;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    res->Accept(writer);

    sprintf(payload, "%s", buff.GetString());
}

void parodusclient::parodusSend(char *payload, char *source, char *destination, char *trans_uuid, wrp_msg_type msg_type)
{
    
    printf("***********parodus send ******************\n");
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
    printf("*********** rtRemoteClientMgr ******************\n");
    connectParodus();
}

int main(int argc, char** argv)
{
   printf("************************ In main **********************\n");
   parodusclient *client = new parodusclient();
   client->rtRemoteClientMgr();
   client->parodusReceive();

   return 0;
}
