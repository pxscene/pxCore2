#ifndef RTNODE_THREAD_H
#define RTNODE_THREAD_H

#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"


#include <string>
#include <map>

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "uv.h"
#include "include/v8.h"
#include "include/libplatform/libplatform.h"
//#include "jsbindings/node_headers.h"

#if 1
 #ifndef WIN32
  #pragma GCC diagnostic pop
 #endif
#endif

#ifdef WIN32
static DWORD __rt_main_thread__;
#else
static pthread_t __rt_main_thread__;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef RUNINMAIN
// rtIsMainThread() - Previously:  identify the MAIN thread of 'node' which running JS code.
//
// rtIsMainThread() - Currently:   identify BACKGROUND thread which running JS code.
//
bool rtIsMainThread()
{
  // Since this is single threaded version we're always on the js thread
  return true;
}
#else
// !CLF: Need to sort this out!  Main thread will not be node thread anymore!!!!
bool rtIsMainThread()
{
#ifdef WIN32
  return (GetCurrentThreadId() == __rt_main_thread__);
#else
  //printf("rtIsMainThread returning %d\n",pthread_equal( pthread_self(), __rt_main_thread__));
  return  pthread_equal(pthread_self(), __rt_main_thread__);
#endif
}
#endif // RUNINMAIN


uv_loop_t *nodeLoop = uv_default_loop();
uv_async_t asyncNewScript;
uv_async_t gcTrigger;

rtNode * nodeLib = NULL;

struct asyncWindowInfo {
    void * window;
    void * view;
};
class AsyncScriptInfo;
extern vector<AsyncScriptInfo*> scriptsInfo;
// !CLF: You will have to translate this for Win32 and OSX
uv_mutex_t moreScriptsMutex;
uv_timer_t nodeTimer;
uv_mutex_t threadMutex;

static void timerCallback(uv_timer_t* )
{
rtLogDebug("uv timer callback");
}

void processNewScript(uv_async_t *handle)
{
    rtLogInfo(__FUNCTION__);
    //printf("processNewScript\n");
    vector<AsyncScriptInfo*> localInfo;
    uv_mutex_lock(&moreScriptsMutex);
    for(AsyncScriptInfo* info: scriptsInfo)
    {
        localInfo.push_back(info);
        //printf("processNewScript pushed one\n");
    }
    //printf("processNewScript clearing\n");
    scriptsInfo.clear(); 
    uv_mutex_unlock(&moreScriptsMutex);
    // Process the scripts we picked off the global vector
    for(AsyncScriptInfo* info: localInfo)
    {
        //printf("processNewScript running script!\n"); 
        info->m_pView->runScript();
        delete info;

    }

    localInfo.clear();

}

void garbageCollect(uv_async_t *handle)
{
    rtLogInfo(__FUNCTION__);

    uv_mutex_lock(&threadMutex);
    nodeLib->garbageCollect();
    uv_mutex_unlock(&threadMutex);
}

void nodeIsEndingCallback(uv_work_t *req, int status)
{
    printf("nodeIsEndingCallback\n");
    if( nodeLib != 0) {
        uv_mutex_lock(&threadMutex);
        nodeLib->pump();

        //garbageCollect(NULL);

        delete nodeLib;
        nodeLib = 0;
        uv_mutex_unlock(&threadMutex);
    }
}

void nodeThread(uv_work_t *req)
{
    rtLogInfo(__FUNCTION__);
    // Node initialization runs once here!
    nodeLib = new rtNode();

    printf("Done with rtNode init\n");
   
    while(!nodeLib->needsToEnd()) {

        if(nodeLib->isInitialized() )
        {
            //printf("nodeThread locking\n");
            uv_mutex_lock(&threadMutex); 
            //printf("nodeThread GOT LOCK\n");
            Locker locker(nodeLib->getIsolate());
            Isolate::Scope isolate_scope(nodeLib->getIsolate());
            HandleScope handle_scope(nodeLib->getIsolate());
            uv_run(nodeLoop, UV_RUN_NOWAIT);
            //printf("nodeThread unlocking\n");
            uv_mutex_unlock(&threadMutex);
        }
        sleep(0.05);
    }
    printf("nodeThread is terminating\n");
    
    nodeIsEndingCallback(NULL,0 );
 
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_THREAD_H