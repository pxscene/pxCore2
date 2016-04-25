// pxCore Copyright 2007-2015 John Robinson
// main.cpp

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <pthread.h>


#include "pxCore.h"


#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "node.h"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxScene2d.h"
#include "pxViewWindow.h"

#include "rtNode.h"
#include "jsbindings/rtWrapperUtils.h"
#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"

using namespace v8;
using namespace node;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pxEventLoop  eventLoop;
pxEventLoop* gLoop = &eventLoop;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rtError getScene(int numArgs, const rtValue* args, rtValue* result, void* ctx); // fwd

//static void disposeNode(const FunctionCallbackInfo<Value>& args); //fwd


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//static void disposeNode(const FunctionCallbackInfo<Value>& args)
//{
//  printf("DEBUG:  disposeNode() ... ENTER \n");
//
//  if (args.Length() < 1)
//    return;
//
//  if (!args[0]->IsObject())
//    return;
//
//  Local<Object> obj = args[0]->ToObject();

//  rtObjectWrapper* wrapper = static_cast<rtObjectWrapper *>(obj->GetAlignedPointerFromInternalField(0));
//  if (wrapper)
//    wrapper->dispose();
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MAIN()
//

void testWindows();
void testContexts();
void testContextsLeak();

args_t *s_gArgs;

int pxMain()
{
  #pragma GCC diagnostic ignored "-Wwrite-strings"
  
                       //012345678 90ABCDEF0 1234567890ABCDEF
  static char *args   = "rtNode\0-e\0var pxArg_url=\"browser.js\"\0\0";
  static char *argv[] = {&args[0], &args[7],  &args[10], NULL};
  int          argc   = sizeof(argv)/sizeof(char*) - 1;

  static args_t aa(argc, argv);
  s_gArgs = &aa;

  testWindows(); /// multi threaded
 // testContextsLeak();  /// single threaded

   return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class testWindow: public pxViewWindow
{
public:

  std::string debug_name;

  void setScene(rtNodeContextRef ctx, pxScene2dRef s)
  {
    rtValue v = new rtFunctionCallback(getScene, s.getPtr());

    ctx->add("getScene", v);

    mScene = s;
  }

  void onClose()
  {
    printf("\n\n #############\n #############  onClose() \n #############\n\n");
  }

  void onCloseRequest()
  {
    printf("\n\n #############\n #############  onCloseRequest() \n #############\n\n");

    // When someone clicks the close box no policy is predefined.
    // so we need to explicitly tell the event loop to exit
  //  eventLoop.exit();
  }

private:

  pxScene2dRef        mScene;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rtError getScene(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  // We don't use the arguments so just return the scene object reference
  if (result)
  {
    pxScene2dRef s = (pxScene2d*)ctx;

    *result = s; // return the scene reference
  }

  return RT_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define USE_CONTEXT_1
#define USE_WINDOW_1

//#define USE_CONTEXT_2
//#define USE_WINDOW_2

//#define USE_CONTEXT_3
//#define USE_WINDOW_3

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testWindows()
{
  rtNode node1(s_gArgs->argc, s_gArgs->argv);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Setup node CONTEXT...
  //

#ifdef USE_CONTEXT_1

  rtNodeContextRef ctx1 = node1.createContext();

#endif

#ifdef USE_CONTEXT_2

  rtNodeContextRef ctx2 = node1.createContext();

#endif

#ifdef USE_CONTEXT_3

  rtNodeContextRef ctx3 = node1.createContext();

#endif

  printf("\n Setup WINDOW and SCENE");  fflush(stdout);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Setup WINDOW and SCENE
  //
#ifdef USE_WINDOW_1

  static testWindow win1;

  pxScene2dRef scene1 = new pxScene2d;

  win1.init(0, 0, 1280, 720);

  win1.setTitle(">> Window 1 <<");
  win1.setVisibility(true);
  win1.setView(scene1);
  win1.setAnimationFPS(60);

  win1.debug_name = "WindowOne";

  win1.setScene(ctx1, scene1);

 // The ADD "scene" approach requires a change to pxRoot.js 
 //
 // ctx1->add("scene", rtValue( scene1.getPtr() ));

  scene1->init();

#endif

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Setup WINDOW and SCENE
  //
#ifdef USE_WINDOW_2

  static testWindow win2;

  pxScene2dRef scene2 = new pxScene2d;

  win2.init(810, 10, 750, 550);

  win2.setTitle(">> Window 2 <<");
  win2.setVisibility(true);
  win2.setView(scene2);
  win2.setAnimationFPS(60);

  win2.debug_name = "WindowTwo";

  win2.setScene(ctx2, scene2);

  scene2->init();

 // ctx2->add("scene", rtValue( scene2.getPtr() ));
  
#endif


  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Setup WINDOW and SCENE
  //
#ifdef USE_WINDOW_3

  static testWindow win3;

  pxScene2dRef scene3 = new pxScene2d;

  win3.init(10, 610, 750, 550);

  win3.setTitle(">> Window 3 <<");
  win3.setVisibility(true);
  win3.setView(scene3);
  win3.setAnimationFPS(60);

  win3.debug_name = "WindowThree";

  win3.setScene(ctx3, scene3);

  scene3->init();

#endif

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // RUN !
  //

#ifdef USE_WINDOW_1
  printf("\n### Window Run 1A"); // ##############################

//  ctx1->runScript("console.log(\"Hello\")");
//  ctx1->runScript("sayHello");

  ctx1->runThread("start.js");

//  ctx1->runThread("test1sec.js");

//  ctx1->Release();

#endif

#ifdef USE_WINDOW_2
//  printf("\n### Window Run 2A"); // ##############################

  ctx2->runFile("start.js");
//  ctx2->runThread("fancyp.js");

  printf("\n### Window Run 2B"); // ##############################
#endif

#ifdef USE_WINDOW_3
//  printf("\n### Window Run 3A"); // ##############################

//  ctx3->runFile("start.js");
  ctx3->runFile("fancyp.js");

  printf("\n### Window Run 3B"); // ##############################
#endif

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//  use_debug_agent = true;
//  debug_wait_connect = true;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if defined(USE_WINDOW_1) || defined(USE_WINDOW_2) || defined(USE_WINDOW_3)

  eventLoop.run(); // BLOCKS
  
  printf("\n INFO:  eventLoop() .... exited !");
  
#endif
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//  while(true)
//  {
//    usleep(1000); // 1 second
//    printf("\n###########################  TICK1 always !!");
//  }

  getchar();
  printf("\n INFO:  EXITING 11111");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Timer
{
public:
  Timer()
  {
    clock_gettime(CLOCK_REALTIME, &beg_);
  }

  double elapsed()
  {
    clock_gettime(CLOCK_REALTIME, &end_);
    return end_.tv_sec  - beg_.tv_sec +
          (end_.tv_nsec - beg_.tv_nsec) / 1000000000.;
  }

  void reset()
  {
    clock_gettime(CLOCK_REALTIME, &beg_);
  }

private:
    timespec beg_, end_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void testContextsLeak()
{
 rtNode node1;
 node1.init(s_gArgs->argc, s_gArgs->argv);
      
  for (int i = 0; i< 2; i++)
  { 
    //printf("\n\nCREATE ctx >>>  Press ENTER key...");
    //getchar();
    
    {//scope
      // rtNode node1;

      // printf("\nINFO: Calling  node1.init() ..."); fflush(stdout);
     
      // node1.init(s_gArgs->argc, s_gArgs->argv);
      
      printf("\nINFO: Calling  node1.createContext() ..."); fflush(stdout);

      rtNodeContextRef ctx1 = node1.createContext();

     // printf("\n\nDESTROY ctx >>>  Press ENTER key...");
     // getchar();

      printf("\nINFO: Calling   ctx1->Release() ..."); fflush(stdout);
      
      ctx1->Release();
    }//scope   
  }//FOR
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//#define NODE_PER

void testContexts()
{
  rtNodeContextRef ctx1;

  double elapsed = 0.0;

  Timer tm_node;

  static rtNode node1(s_gArgs->argc, s_gArgs->argv);  // MUST BE STATIC !

  elapsed = tm_node.elapsed();

  printf("\n NODE() ... took %f seconds", elapsed);

  #define MYNODE node1

#ifdef NODE_PER
  double     min_node = 990.0;
  double     max_node = 0.0;
  double elapsed_node = 0.0;

#endif

  double     min_ctx = 990.0;
  double     max_ctx = 0.0;
  double elapsed_ctx = 0.0;

  int    total_iterations = 4;

  for (int i = 0; i< total_iterations; i++)
  {

#ifdef NODE_PER
    Timer tm_node;
    rtNode node_per;
    elapsed = tm_node.elapsed();

    elapsed_node += elapsed;

    max_node = (max_node >= elapsed) ? max_node : elapsed;
    min_node = (min_node <  elapsed) ? min_node : elapsed;

#define MYNODE node_per

#endif

    Timer tm_ctx;
    ctx1 = MYNODE.createContext();
    elapsed = tm_ctx.elapsed();

    if(i < 3)  printf("\n createContext() ... took %f seconds", elapsed);

    elapsed_ctx += elapsed;

    max_ctx = (max_ctx >= elapsed) ? max_ctx : elapsed;
    min_ctx = (min_ctx <  elapsed) ? min_ctx : elapsed;

    ctx1->runFile("test1sec.js");

    ctx1->Release();
  }//FOR

  printf("\n");
  printf("\n");

#ifdef NODE_PER
  double avg_node = (elapsed_node / (double) total_iterations);

  printf("\n###    NODE: %d iterations took %f seconds  (%f sec average)", total_iterations,  elapsed_node, avg_node);
  printf("\n###          Max =  %f seconds  Min = %f seconds", max_node, min_node);
  printf("\n");
#endif

  double avg_ctx  = (elapsed_ctx  / (double) total_iterations);

  printf("\n### CONTEXT: %d iterations took %f seconds  (%f sec average)", total_iterations,  elapsed_ctx, avg_ctx);
  printf("\n###          Max =  %f seconds  Min = %f seconds", max_ctx, min_ctx);


  printf("\n");
  printf("\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
