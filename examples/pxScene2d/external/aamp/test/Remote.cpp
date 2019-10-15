/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <string.h>
#include "libIBus.h"
#include "libIBusDaemon.h"
#include "irMgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include "comcastIrKeyCodes.h"

#define IR_REMOTE_APP_INIT_STR "IrRemoteApp"

int gPipefd[2];
int uinp_fd;
struct uinput_user_dev uinp;
struct input_event event;

static void _evtHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
static void KeyProcessHandler(int,int);
static void IRkeyhandler(int,int);
static void SendKeyToKernel(int);
static void RegisterDevice();

int main(int argc, char** argv)
{

    (void) argc;
    (void) argv;

    char Init_Str[] = IR_REMOTE_APP_INIT_STR;

    IARM_Bus_Init(Init_Str);
    IARM_Bus_Connect();
    IARM_Bus_RegisterEventHandler(IARM_BUS_IRMGR_NAME, IARM_BUS_IRMGR_EVENT_IRKEY, _evtHandler);
    RegisterDevice();

    while(1)
    {
        sleep(5);
    }
}

void KeyProcessHandler(int KeyType,int KeyCode)
{

   switch (KeyType)
   {
         case KET_KEYDOWN:
             printf("KET_KEYDOWN\n");
             IRkeyhandler(KeyType, KeyCode);
             break;
         case KET_KEYUP:
            // IRkeyhandler(KeyType, KeyCode);
             break;
         case KET_KEYREPEAT:
            // IRkeyhandler(KeyCode, KeyType);
            break;
         default:
             /* TBD: Support for mouse and keypad */
            break;
     }


}

//Open uinput device node and register this application
void RegisterDevice()
{

    uinp_fd = open("/dev/uinput", O_WRONLY|O_NDELAY);

    if(uinp_fd < 0)
    {
       printf("Unable to open /dev/uinput\n");
       return;
    }

    memset(&uinp,0,sizeof(uinp));

    snprintf(uinp.name, UINPUT_MAX_NAME_SIZE, "Ir Keyboard");
    uinp.id.bustype = BUS_USB;
    uinp.id.version = 1;
    uinp.id.vendor = 0x1234;
    uinp.id.product = 0xfedc;

    write(uinp_fd, &uinp, sizeof(uinp));

    //ioctl(uinp_fd, UI_DEV_CREATE)
    /****** Setup the uinput keyboard device section: **********/

    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_SYN);
    //ioctl(uinp_fd, UI_SET_EVBIT, EV_REP);

    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_H);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_R);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_P);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_U);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_D);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_S);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_F);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_X);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_Z);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_UP);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_DOWN);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_SPACE);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_ESC);

    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_0);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_1);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_2);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_3);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_4);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_5);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_6);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_7);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_8);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_9);
 
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_VOLUMEUP);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_VOLUMEDOWN);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_MUTE);
  
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_F2);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_F3);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_F5);

    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_ENTER);
    ioctl(uinp_fd, UI_SET_KEYBIT, KEY_BACKSPACE);

    if (ioctl(uinp_fd, UI_DEV_CREATE, NULL) < 0)
    {
         printf("Unable to create UINPUT device.\n");
         return;
    }
    sleep(2);
}

void IRkeyhandler(int KeyType,int KeyCode)
{
    (void) KeyType;

    //test check
    switch(KeyCode)
    {
        case 192:
                   SendKeyToKernel(KEY_H);
                   break;
        case KED_ARROWUP:
                   SendKeyToKernel(KEY_UP);
                   break;

        case KED_ARROWDOWN:
                  SendKeyToKernel(KEY_DOWN);
                  break;

        case KED_ARROWLEFT:
		  SendKeyToKernel(KEY_LEFT);
                  break;

        case KED_ARROWRIGHT:
                  SendKeyToKernel(KEY_RIGHT);
                  break;
        case 133:
                 SendKeyToKernel(KEY_SPACE);                   
                 break;
        case 135:
                 SendKeyToKernel(KEY_ESC);
                 break;
        case KED_DIGIT0 :
                   SendKeyToKernel(KEY_0); 
                   break;
        case KED_DIGIT1 :
                   SendKeyToKernel(KEY_1);
                   break;
        case KED_DIGIT2 :
                   SendKeyToKernel(KEY_2); 
                   break;
        case KED_DIGIT3 :
                   SendKeyToKernel(KEY_3);
                   break;
        case KED_DIGIT4 :
                   SendKeyToKernel(KEY_4);
                   break;
        case KED_DIGIT5 :
                   SendKeyToKernel(KEY_5);
                   break;
        case KED_DIGIT6 :
                   SendKeyToKernel(KEY_6); 
                   break;
        case KED_DIGIT7 :
                   SendKeyToKernel(KEY_7);
                   break;
        case KED_DIGIT8 :
                   SendKeyToKernel(KEY_8);
                   break;
        case KED_DIGIT9 :
                   SendKeyToKernel(KEY_9); 
                   break;
        case KED_CHANNELUP :
                   SendKeyToKernel(KEY_U);
                   break;
        case KED_CHANNELDOWN :
                   SendKeyToKernel(KEY_D);
                   break;
        case KED_VOLUMEUP :
                   SendKeyToKernel(KEY_VOLUMEUP);   
                   break;
        case KED_VOLUMEDOWN :
                   SendKeyToKernel(KEY_VOLUMEDOWN);
                   break;
        case KED_MUTE: 
                   SendKeyToKernel(KEY_MUTE); 
                   break;
        case KED_PLAY:
                   SendKeyToKernel(KEY_P); 
                   break;
        case KED_PAUSE:
                   SendKeyToKernel(KEY_P);
                   break;
        case KED_LAST:
                   SendKeyToKernel(KEY_ENTER);
                   break;
        case KED_FASTFORWARD:
                   SendKeyToKernel(KEY_F);
                   break;
        case KED_REWIND:
                   SendKeyToKernel(KEY_R);
                   break;
        case KED_STOP:
                   SendKeyToKernel(KEY_STOP);
                   break;
        case KED_REPLAY:
                   SendKeyToKernel(KEY_S);
                   break;
        case KED_PAGEUP:
                   SendKeyToKernel(KEY_Z);
                   break;
        case KED_PAGEDOWN:
                   SendKeyToKernel(KEY_X);
                   break;
        default:
                 break;
 }

}

//send Events to kernel
void SendKeyToKernel(int code)
{
    printf("Inside SendKeytoKernel\n");
    memset(&event, 0, sizeof(event));
    //gettimeofday(&event.time, NULL);

    //Send key down event
    event.type = EV_KEY;
    event.code = code;
    event.value = 1;
    write(uinp_fd, &event, sizeof(event));
  
    memset(&event, 0, sizeof(event));
    event.type = EV_KEY;
    event.code = code;
    event.value = 0;
    write(uinp_fd, &event, sizeof(event));

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 1;
    write(uinp_fd, &event, sizeof(event));

    printf("keyevent send to the kernel\n");

}

//IR remote integration
void _evtHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    (void) len;

    if (strcmp(owner, IARM_BUS_IRMGR_NAME)  == 0)
    {
        switch (eventId) {
            case IARM_BUS_IRMGR_EVENT_IRKEY:
            {
                IARM_Bus_IRMgr_EventData_t *irEventData = (IARM_Bus_IRMgr_EventData_t*)data;
                int keyCode = irEventData->data.irkey.keyCode;
                int keyType = irEventData->data.irkey.keyType;
                printf("Receiver Get IR KeyCode: %d-",keyCode);
                KeyProcessHandler(keyType, keyCode);
            }
                break;
            default:
                break;
        }

    }
}






