#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "westeros-compositor.h"

#define WST_MAX_ERROR_DETAIL (512)

#define MAX_NESTED_NAME_LEN (32)

#define DEFAULT_FRAME_RATE (60)
#define DEFAULT_OUTPUT_WIDTH (1280)
#define DEFAULT_OUTPUT_HEIGHT (720)
#define DEFAULT_NESTED_WIDTH (1280)
#define DEFAULT_NESTED_HEIGHT (720)

typedef struct _WstCompositor 
{
   const char *displayName;
   unsigned int frameRate;
   const char *rendererModule;
   bool isNested;
   bool isRepeater;
   bool isEmbedded;
   const char *nestedDisplayName;
   unsigned int nestedWidth;
   unsigned int nestedHeight;
   bool allowModifyCursor;
   int outputWidth;
   int outputHeight;

   bool running;

   char lastErrorDetail[WST_MAX_ERROR_DETAIL];
}
WstCompositor;


static const char* wstGetNextNestedDisplayName(void);


static int g_pid= 1000;
static int g_nextNestedId= 0;


WstCompositor* WstCompositorCreate()
{
  WstCompositor *ctx= (WstCompositor*)calloc( 1, sizeof(WstCompositor) );

  if ( ctx )
  {
      ctx->frameRate= DEFAULT_FRAME_RATE;
      
      ctx->nestedWidth= DEFAULT_NESTED_WIDTH;
      ctx->nestedHeight= DEFAULT_NESTED_HEIGHT;
      
      ctx->outputWidth= DEFAULT_OUTPUT_WIDTH;
      ctx->outputHeight= DEFAULT_OUTPUT_HEIGHT;
  }    
  
  return ctx;
}

void WstCompositorDestroy( WstCompositor *ctx )
{
   if ( ctx )
   {
      if ( ctx->running )
      {
         WstCompositorStop( ctx );
      }
      
      if ( ctx->displayName )
      {
         free( (void*)ctx->displayName );
         ctx->displayName= 0;
      }
      
      if ( ctx->rendererModule )
      {
         free( (void*)ctx->rendererModule );
         ctx->rendererModule= 0;
      }
      
      if ( ctx->nestedDisplayName )
      {
         free( (void*)ctx->nestedDisplayName );
         ctx->nestedDisplayName= 0;
      }
      
      free( ctx );
   }
}

const char *WstCompositorGetLastErrorDetail( WstCompositor *ctx )
{
   const char *msg= 0;
   
   if ( ctx )
   {
      msg= ctx->lastErrorDetail;
   }
   
   return msg;
}

bool WstCompositorSetDisplayName( WstCompositor *ctx, const char *displayName )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set display name while compositor is running" );
         goto exit;
      }
      
      if ( ctx->displayName )
      {
         free( (void*)ctx->displayName );
      }
      
      ctx->displayName= strdup( displayName );
      
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetFrameRate( WstCompositor *ctx, unsigned int frameRate )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( frameRate == 0 )
      {
         sprintf( ctx->lastErrorDetail,
                  "Invalid argument.  The frameRate (%u) must be greater than 0 fps", frameRate );
         goto exit;      
      }

      ctx->frameRate= frameRate;
      
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetRendererModule( WstCompositor *ctx, const char *rendererModule )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set renderer module while compositor is running" );
         goto exit;
      }
               
      if ( ctx->rendererModule )
      {
         free( (void*)ctx->rendererModule );
      }
      
      ctx->rendererModule= strdup( rendererModule );
      
      result= true;
   }
   
exit:

   return result;
}

bool WstCompositorSetIsNested( WstCompositor *ctx, bool isNested )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set isNested while compositor is running" );
         goto exit;
      }
                     
      ctx->isNested= isNested;
            
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetIsRepeater( WstCompositor *ctx, bool isRepeater )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set isRepeater while compositor is running" );
         goto exit;
      }
                     
      ctx->isRepeater= isRepeater;
      if ( isRepeater )
      {
         ctx->isNested= true;
         ctx->nestedWidth= 0;
         ctx->nestedHeight= 0;
      }
            
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetIsEmbedded( WstCompositor *ctx, bool isEmbedded )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set isEmbedded while compositor is running" );
         goto exit;
      }
                     
      ctx->isEmbedded= isEmbedded;
            
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetOutputSize( WstCompositor *ctx, int width, int height )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set output size while compositor is running" );
         goto exit;
      }      

      if ( width == 0 )
      {
         sprintf( ctx->lastErrorDetail,
                  "Invalid argument.  The output width (%u) must be greater than zero", width );
         goto exit;      
      }

      if ( height == 0 )
      {
         sprintf( ctx->lastErrorDetail,
                  "Invalid argument.  The output height (%u) must be greater than zero", height );
         goto exit;      
      }
               
      ctx->outputWidth= width;
      ctx->outputHeight= height;
            
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetNestedDisplayName( WstCompositor *ctx, const char *nestedDisplayName )
{
   bool result= false;
   int len;
   const char *name= 0;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set nested display name while compositor is running" );
         goto exit;
      }
      
      if ( nestedDisplayName )
      {
         len= strlen(nestedDisplayName);
         
         if ( (len == 0) || (len > MAX_NESTED_NAME_LEN) )
         {
            sprintf( ctx->lastErrorDetail,
                     "Invalid argument.  The nested name length (%u) must be > 0 and < %u in length", 
                     len, MAX_NESTED_NAME_LEN );
            goto exit;      
         }
         
         name= strdup( nestedDisplayName );
      }
                     
      if ( ctx->nestedDisplayName )
      {
         free( (void*)ctx->nestedDisplayName );
      }
      
      ctx->nestedDisplayName= name;
      
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetNestedSize( WstCompositor *ctx, unsigned int width, unsigned int height )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set nested size while compositor is running" );
         goto exit;
      }      

      if ( width == 0 )
      {
         sprintf( ctx->lastErrorDetail,
                  "Invalid argument.  The nested width (%u) must be greater than zero", width );
         goto exit;      
      }

      if ( height == 0 )
      {
         sprintf( ctx->lastErrorDetail,
                  "Invalid argument.  The nested height (%u) must be greater than zero", height );
         goto exit;      
      }
               
      ctx->nestedWidth= width;
      ctx->nestedHeight= height;
            
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorSetAllowCursorModification( WstCompositor *ctx, bool allow )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set allow cursor modification while compositor is running" );
         goto exit;
      }      

      ctx->allowModifyCursor= allow;
            
      result= true;
   }

exit:
   
   return result;
}

const char *WstCompositorGetDisplayName( WstCompositor *ctx )
{
   const char *displayName= 0;
   
   if ( ctx )
   {
      // If no display name was provided, then generate a name.
      if ( !ctx->displayName )
      {
         ctx->displayName= wstGetNextNestedDisplayName();
      }

      displayName= ctx->displayName;
   }
   
   return displayName;
}

unsigned int WstCompositorGetFrameRate( WstCompositor *ctx )
{
   unsigned int frameRate= 0;
   
   if ( ctx )
   {               
      frameRate= ctx->frameRate;
   }
   
   return frameRate;
}

const char *WstCompositorGetRendererModule( WstCompositor *ctx )
{
   const char *rendererModule= 0;
   
   if ( ctx )
   {               
      rendererModule= ctx->rendererModule;
   }
   
   return rendererModule;
}

bool WstCompositorGetIsNested( WstCompositor *ctx )
{
   bool isNested= false;
   
   if ( ctx )
   {               
      isNested= ctx->isNested;
   }
   
   return isNested;
}

bool WstCompositorGetIsRepeater( WstCompositor *ctx )
{
   bool isRepeater= false;
   
   if ( ctx )
   {               
      isRepeater= ctx->isRepeater;
   }
   
   return isRepeater;
}

bool WstCompositorGetIsEmbedded( WstCompositor *ctx )
{
   bool isEmbedded= false;
   
   if ( ctx )
   {               
      isEmbedded= ctx->isEmbedded;
   }
   
   return isEmbedded;
}

void WstCompositorGetOutputSize( WstCompositor *ctx, unsigned int *width, unsigned int *height )
{
   int outputWidth= 0;
   int outputHeight= 0;
   
   if ( ctx )
   {               
      outputWidth= ctx->outputWidth;
      outputHeight= ctx->outputHeight;
   }

   if ( width )
   {
      *width= outputWidth;
   }
   if ( height )
   {
      *height= outputHeight;
   }
}

const char *WstCompositorGetNestedDisplayName( WstCompositor *ctx )
{
   const char *nestedDisplayName= 0;
   
   if ( ctx )
   {               
      nestedDisplayName= ctx->nestedDisplayName;
   }
   
   return nestedDisplayName;
}

void WstCompositorGetNestedSize( WstCompositor *ctx, unsigned int *width, unsigned int *height )
{
   int nestedWidth= 0;
   int nestedHeight= 0;
   
   if ( ctx )
   {               
      nestedWidth= ctx->nestedWidth;
      nestedHeight= ctx->nestedHeight;
   }

   if ( width )
   {
      *width= nestedWidth;
   }
   if ( height )
   {
      *height= nestedHeight;
   }
}

bool WstCompositorGetAllowCursorModification( WstCompositor *ctx )
{
   bool allow= false;
   
   if ( ctx )
   {               
      allow= ctx->allowModifyCursor;
   }
   
   return allow;
}

bool WstCompositorSetTerminatedCallback( WstCompositor *ctx, WstTerminatedCallback cb, void *userData )
{
   bool result= false;
   
   if ( ctx )
   {
      result= true;
   }

   return result;   
}

bool WstCompositorSetInvalidateCallback( WstCompositor *ctx, WstInvalidateSceneCallback cb, void *userData )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( !ctx->isEmbedded )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not embedded" );
         goto exit;
      }
      
      result= true;
   }

exit:

   return result;   
}

bool WstCompositorSetHidePointerCallback( WstCompositor *ctx, WstHidePointerCallback cb, void *userData )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( !ctx->isEmbedded )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not embedded" );
         goto exit;
      }
      
      result= true;
   }

exit:

   return result;   
}

bool WstCompositorSetClientStatusCallback( WstCompositor *ctx, WstClientStatus cb, void *userData )
{
  bool result= false;
   
   if ( ctx )
   {
      if ( !ctx->isEmbedded )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not embedded" );
         goto exit;
      }
      
      result= true;
   }

exit:

   return result;   
}

bool WstCompositorSetKeyboardNestedListener( WstCompositor *ctx, WstKeyboardNestedListener *listener, void *userData )
{
  bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set keyboard nested listener while compositor is running" );
         goto exit;
      }      

      if ( !ctx->isNested )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not nested" );
         goto exit;
      }
      
      result= true;
   }

exit:

   return result;   
}

bool WstCompositorSetPointerNestedListener( WstCompositor *ctx, WstPointerNestedListener *listener, void *userData )
{
  bool result= false;
   
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Cannot set pointer nested listener while compositor is running" );
         goto exit;
      }      

      if ( !ctx->isNested )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not nested" );
         goto exit;
      }
      
      result= true;
   }

exit:

   return result;   
}

bool WstCompositorComposeEmbedded( WstCompositor *ctx, int width, int height, int resW, int resH, float *matrix, float alpha )
{
   bool result= false;

   if ( ctx )
   {
      if ( !ctx->isEmbedded )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not embedded" );
         goto exit;
      }
      
      if ( !ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not running" );
         goto exit;
      }
         
      result= true;
   }

exit:
   
   return result;
}

bool WstCompositorStart( WstCompositor *ctx )
{
   bool result= false;
                  
   if ( ctx )
   {
      if ( ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is already running" );
         goto exit;
      }
      
      if ( !ctx->rendererModule && ctx->isEmbedded )
      {
         ctx->rendererModule= strdup("libwesteros_render_embedded.so.0");
      }
      
      if ( !ctx->rendererModule && !ctx->isRepeater )
      {
         sprintf( ctx->lastErrorDetail,
                  "Error.  A renderer module must be supplied" );
         goto exit;      
      }
      
      if ( ctx->isNested )
      {
         // If we are operating as a nested compostitor the name
         // of the wayland display we are to pass our composited output
         // to must be provided
         if ( !ctx->nestedDisplayName )
         {
            char *var= getenv("WAYLAND_DISPLAY");
            if ( var )
            {
               ctx->nestedDisplayName= strdup(var);
            }
         }
         if ( !ctx->nestedDisplayName )
         {
            sprintf( ctx->lastErrorDetail,
                     "Error.  Nested composition requested but no target display name provided" );
            goto exit;      
         }
      }
         
      // If no display name was provided, then generate a name.
      if ( !ctx->displayName )
      {
         ctx->displayName= wstGetNextNestedDisplayName();
      }

      ctx->running= true;

      result= true;      
   }

exit:
   
   return result;
}

void WstCompositorStop( WstCompositor *ctx )
{
   if ( ctx )
   {
      if ( ctx->running )
      {
         ctx->running= false;
      }
   }
}

void WstCompositorKeyEvent( WstCompositor *ctx, int keyCode, unsigned int keyState, unsigned int modifiers )
{
   if ( ctx )
   {
   }
}

void WstCompositorPointerEnter( WstCompositor *ctx )
{
   if ( ctx )
   {
   }
}

void WstCompositorPointerLeave( WstCompositor *ctx )
{
   if ( ctx )
   {
   }
}

void WstCompositorPointerMoveEvent( WstCompositor *ctx, int x, int y )
{
   if ( ctx )
   {
   }
}

void WstCompositorPointerButtonEvent( WstCompositor *ctx, unsigned int button, unsigned int buttonState )
{
   if ( ctx )
   {
   }
}

bool WstCompositorLaunchClient( WstCompositor *ctx, const char *cmd )
{
   bool result= false;
   
   if ( ctx )
   {
      if ( !ctx->running )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad state.  Compositor is not running" );
         goto exit;
      }

      int i= (cmd ? strlen(cmd) : 0);
      if ( !cmd || (i > 255) )
      {
         sprintf( ctx->lastErrorDetail,
                  "Bad argument.  cmd (%p len %d) rejected", cmd, i );
         goto exit;
      }            
      
      result= true;
   }
   
exit:

   return result;
}

static const char* wstGetNextNestedDisplayName(void)
{
   char *name= 0;
   char work[32];
   int id;
   
   id= g_nextNestedId;
   
   ++g_nextNestedId;
   
   sprintf( work, "westeros-%u-%u", g_pid, id );
   
   name= strdup(work);
   
   return name;
}

