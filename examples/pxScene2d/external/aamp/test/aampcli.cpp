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

/**
 * @file aampcli.cpp
 * @brief Stand alone AAMP player with command line interface.
 */

#ifdef AAMP_RENDER_IN_APP
#ifdef __APPLE__
	#define GL_SILENCE_DEPRECATION
	#include <OpenGL/gl3.h>
	#include <GLUT/glut.h>
#else	//Linux
	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/freeglut.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

#ifdef __APPLE__
#import <cocoa_window.h>
#endif
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <list>
#include <string.h>
#include <gst/gst.h>
#include <priv_aamp.h>
#include <main_aamp.h>
#include "../StreamAbstractionAAMP.h"

#ifdef IARM_MGR
#include "host.hpp"
#include "manager.hpp"
#include "libIBus.h"
#include "libIBusDaemon.h"
#endif

static PlayerInstanceAAMP *mSingleton;
static GMainLoop *AAMPGstPlayerMainLoop = NULL;

/**
 * @struct VirtualChannelInfo
 * @brief Holds information of a virtual channel
 */
struct VirtualChannelInfo
{
	VirtualChannelInfo() : channelNumber(0), name(), uri()
	{
	}
	int channelNumber;
	std::string name;
	std::string uri;
};

static std::list<VirtualChannelInfo> mVirtualChannelMap;

/**
 * @brief Thread to run mainloop (for standalone mode)
 * @param[in] arg user_data
 * @retval void pointer
 */
static void* AAMPGstPlayer_StreamThread(void *arg);
static bool initialized = false;
GThread *aampMainLoopThread = NULL;


/**
 * @brief trim a string
 * @param[in][out] cmd Buffer containing string
 */
static void trim(char **cmd)
{
    std::string src = *cmd;
    size_t first = src.find_first_not_of(' ');
    if (first != std::string::npos)
    {
        size_t last = src.find_last_not_of(" \r\n");
        std::string dst = src.substr(first, (last - first + 1));
        strncpy(*cmd, (char*)dst.c_str(), dst.size());
        (*cmd)[dst.size()] = '\0';
    }
}

/**
 * @brief check if the char array is having numbers only
 * @param s
 * @retval true or false
 */
static bool isNumber(const char *s)
{
    if (*s)
    {
        if (*s == '-')
        { // skip leading minus
            s++;
        }
        for (;;)
        {
            if (*s >= '0' && *s <= '9')
            {
                s++;
                continue;
            }
            if (*s == 0x00)
            {
                return true;
            }
            break;
        }
    }
    return false;
}



/**
 * @brief Thread to run mainloop (for standalone mode)
 * @param[in] arg user_data
 * @retval void pointer
 */
static void* AAMPGstPlayer_StreamThread(void *arg)
{
	if (AAMPGstPlayerMainLoop)
	{
		g_main_loop_run(AAMPGstPlayerMainLoop); // blocks
		logprintf("AAMPGstPlayer_StreamThread: exited main event loop\n");
	}
	g_main_loop_unref(AAMPGstPlayerMainLoop);
	AAMPGstPlayerMainLoop = NULL;
	return NULL;
}

/**
 * @brief To initialize Gstreamer and start mainloop (for standalone mode)
 * @param[in] argc number of arguments
 * @param[in] argv array of arguments
 */
void InitPlayerLoop(int argc, char **argv)
{
	if (!initialized)
	{
		initialized = true;
		gst_init(&argc, &argv);
		AAMPGstPlayerMainLoop = g_main_loop_new(NULL, FALSE);
		aampMainLoopThread = g_thread_new("AAMPGstPlayerLoop", &AAMPGstPlayer_StreamThread, NULL );
	}
}

/**
 * @brief Stop mainloop execution (for standalone mode)
 */
void TermPlayerLoop()
{
	if(AAMPGstPlayerMainLoop)
	{
		g_main_loop_quit(AAMPGstPlayerMainLoop);
		g_thread_join(aampMainLoopThread);
		gst_deinit ();
		logprintf("%s(): Exit\n", __FUNCTION__);
	}
}

/**
 * @brief Show help menu with aamp command line interface
 */
static void ShowHelp(void)
{
	int i = 0;
	if (!mVirtualChannelMap.empty())
	{
		logprintf("\nChannel Map from aampcli.cfg\n*************************\n");

		for (std::list<VirtualChannelInfo>::iterator it = mVirtualChannelMap.begin(); it != mVirtualChannelMap.end(); ++it, ++i)
		{
			VirtualChannelInfo &pChannelInfo = *it;
			printf("%4d: %s", pChannelInfo.channelNumber, pChannelInfo.name.c_str());
			if ((i % 4) == 3)
			{
				printf("\n");
			}
			else
			{
				printf("\t");
			}
		}
		printf("\n");
	}

	logprintf("List of Commands\n****************\n");
	logprintf("<channelNumber> // Play selected channel from guide\n");
	logprintf("<url> // Play arbitrary stream\n");
	logprintf("info gst trace curl progress // Logging toggles\n");
	logprintf("pause play stop status flush // Playback options\n");
	logprintf("sf, ff<x> rw<y> // Trickmodes (x- 16, 32. y- 4, 8, 16, 32)\n");
	logprintf("+ - // Change profile\n");
	logprintf("sap // Use SAP track (if avail)\n");
	logprintf("seek <seconds> // Specify start time within manifest\n");
	logprintf("live // Seek to live point\n");
	logprintf("underflow // Simulate underflow\n");
	logprintf("help // Show this list again\n");
	logprintf("exit // Exit from application\n");
}


//#define LOG_CLI_EVENTS
#ifdef LOG_CLI_EVENTS
static class PlayerInstanceAAMP *mpPlayerInstanceAAMP;

/**
 * @class myAAMPEventListener
 * @brief
 */
class myAAMPEventListener :public AAMPEventListener
{
public:

	/**
	 * @brief Implementation of event callback
	 * @param e Event
	 */
	void Event(const AAMPEvent & e)
	{
		switch (e.type)
		{
		case AAMP_EVENT_TUNED:
			logprintf("AAMP_EVENT_TUNED\n");
			break;
		case AAMP_EVENT_TUNE_FAILED:
			logprintf("AAMP_EVENT_TUNE_FAILED\n");
			break;
		case AAMP_EVENT_SPEED_CHANGED:
			logprintf("AAMP_EVENT_SPEED_CHANGED\n");
			break;
		case AAMP_EVENT_DRM_METADATA:
                        logprintf("AAMP_DRM_FAILED\n");
                        break;
		case AAMP_EVENT_EOS:
			logprintf("AAMP_EVENT_EOS\n");
			break;
		case AAMP_EVENT_PLAYLIST_INDEXED:
			logprintf("AAMP_EVENT_PLAYLIST_INDEXED\n");
			break;
		case AAMP_EVENT_PROGRESS:
			//			logprintf("AAMP_EVENT_PROGRESS\n");
			break;
		case AAMP_EVENT_CC_HANDLE_RECEIVED:
			logprintf("AAMP_EVENT_CC_HANDLE_RECEIVED\n");
			break;
		case AAMP_EVENT_BITRATE_CHANGED:
			logprintf("AAMP_EVENT_BITRATE_CHANGED\n");
			break;
		}
	}
}; // myAAMPEventListener

static class myAAMPEventListener *myEventListener;
#endif

/**
 * @brief Parse config entries for aamp-cli, and update gpGlobalConfig params
 *        based on the config.
 * @param cfg config to process
 */
static void ProcessCLIConfEntry(char *cfg)
{
	trim(&cfg);
	if (cfg[0] == '*')
	{
			char *delim = strchr(cfg, ' ');
			if (delim)
			{
				//Populate channel map from aampcli.cfg
				VirtualChannelInfo channelInfo;
				channelInfo.channelNumber = INT_MIN;
				char *channelStr = &cfg[1];
				char *token = strtok(channelStr, " ");
				while (token != NULL)
				{
					if (isNumber(token))
						channelInfo.channelNumber = atoi(token);
					else if (memcmp(token, "http", 4) == 0)
							channelInfo.uri = token;
					else
						channelInfo.name = token;
					token = strtok(NULL, " ");
				}
				if (!channelInfo.uri.empty())
				{
					if (INT_MIN == channelInfo.channelNumber)
					{
						channelInfo.channelNumber = mVirtualChannelMap.size() + 1;
					}
					if (channelInfo.name.empty())
					{
						channelInfo.name = "CH" + std::to_string(channelInfo.channelNumber);
					}
					mVirtualChannelMap.push_back(channelInfo);
				}
				else
				{
					logprintf("%s(): Could not parse uri of %s\n", __FUNCTION__, cfg);
				}
			}
	}
}

/**
 * @brief Process command
 * @param cmd command
 */
static void ProcessCliCommand(char *cmd)
{
	double seconds = 0;
	int rate = 0;
	trim(&cmd);
	if (cmd[0] == 0)
	{
		if (mSingleton->aamp->mpStreamAbstractionAAMP)
		{
			mSingleton->aamp->mpStreamAbstractionAAMP->DumpProfiles();
		}
		logprintf("current bitrate ~= %ld\n", mSingleton->aamp->GetCurrentlyAvailableBandwidth());
	}
	else if (strcmp(cmd, "help") == 0)
	{
		ShowHelp();
	}
	else if (memcmp(cmd, "http", 4) == 0)
	{
		mSingleton->Tune(cmd);
	}
	else if (isNumber(cmd))
	{
		int channelNumber = atoi(cmd);
		logprintf("channel number: %d\n", channelNumber);
		for (std::list<VirtualChannelInfo>::iterator it = mVirtualChannelMap.begin(); it != mVirtualChannelMap.end(); ++it)
		{
			VirtualChannelInfo &channelInfo = *it;
			if(channelInfo.channelNumber == channelNumber)
			{
			//	logprintf("Found %d tuning to %s\n",channelInfo.channelNumber, channelInfo.uri.c_str());
				mSingleton->Tune(channelInfo.uri.c_str());
				break;
			}
		}
	}
	else if (sscanf(cmd, "seek %lf", &seconds) == 1)
	{
		mSingleton->Seek(seconds);
	}
	else if (strcmp(cmd, "sf") == 0)
	{
		mSingleton->SetRate(0.5);
	}
	else if (sscanf(cmd, "ff%d", &rate) == 1)
	{
		if (rate != 4 && rate != 16 && rate != 32)
		{
			logprintf("Speed not supported.\n");
		}
		else
		{
			mSingleton->SetRate((float)rate);
		}
	}
	else if (strcmp(cmd, "play") == 0)
	{
		mSingleton->SetRate(1);
	}
	else if (strcmp(cmd, "pause") == 0)
	{
		mSingleton->SetRate(0);
	}
	else if (sscanf(cmd, "rw%d", &rate) == 1)
	{
		if ((rate < 4 || rate > 32) || (rate % 4))
		{
			logprintf("Speed not supported.\n");
		}
		else
		{
			mSingleton->SetRate((float)(-rate));
		}
	}
	else if (sscanf(cmd, "bps %d", &rate) == 1)
	{
		logprintf("Set video bitrate %d.\n", rate);
		mSingleton->SetVideoBitrate(rate);
	}
	else if (strcmp(cmd, "flush") == 0)
	{
		mSingleton->aamp->mStreamSink->Flush();
	}
	else if (strcmp(cmd, "stop") == 0)
	{
		mSingleton->Stop();
	}
	else if (strcmp(cmd, "underflow") == 0)
	{
		mSingleton->aamp->ScheduleRetune(eGST_ERROR_UNDERFLOW, eMEDIATYPE_VIDEO);
	}
	else if (strcmp(cmd, "retune") == 0)
	{
		mSingleton->aamp->ScheduleRetune(eDASH_ERROR_STARTTIME_RESET, eMEDIATYPE_VIDEO);
	}
	else if (strcmp(cmd, "status") == 0)
	{
		mSingleton->aamp->mStreamSink->DumpStatus();
	}
	else if (strcmp(cmd, "live") == 0)
	{
		mSingleton->SeekToLive();
	}
	else if (strcmp(cmd, "exit") == 0)
	{
		mSingleton->Stop();
		delete mSingleton;
		mVirtualChannelMap.clear();
		TermPlayerLoop();
		exit(0);
	}
	else if (memcmp(cmd, "rect", 4) == 0)
	{
		int x, y, w, h;
		if (sscanf(cmd, "rect %d %d %d %d", &x, &y, &w, &h) == 4)
		{
			mSingleton->SetVideoRectangle(x, y, w, h);
		}
	}
	else if (memcmp(cmd, "zoom", 4) == 0)
	{
		int zoom;
		if (sscanf(cmd, "zoom %d", &zoom) == 1)
		{
			if (zoom)
			{
				logprintf("Set zoom to full\n");
				mSingleton->SetVideoZoom(VIDEO_ZOOM_FULL);
			}
			else
			{
				logprintf("Set zoom to none\n");
				mSingleton->SetVideoZoom(VIDEO_ZOOM_NONE);
			}
		}
	}
	else if (strcmp(cmd, "sap") == 0)
	{
		gpGlobalConfig->SAP = !gpGlobalConfig->SAP;
		logprintf("SAP %s\n", gpGlobalConfig->SAP ? "on" : "off");
		if (gpGlobalConfig->SAP)
		{
			mSingleton->SetLanguage("es");
		}
		else
		{
			mSingleton->SetLanguage("en");
		}
	}
}

static void * run_commnds(void *arg)
{
    char cmd[MAX_URI_LENGTH * 2];
    ShowHelp();
    char *ret = NULL;
    do
    {
        logprintf("aamp-cli> ");
        if((ret = fgets(cmd, sizeof(cmd), stdin))!=NULL)
            ProcessCliCommand(cmd);
    } while (ret != NULL);
    return NULL;
}

#ifdef AAMP_RENDER_IN_APP
#define ATTRIB_VERTEX 0
#define ATTRIB_TEXTURE 1

typedef struct{
	int width = 0;
	int height = 0;
	uint8_t *yuvBuffer = NULL;
	std::mutex mutex;
}AppsinkData;

static AppsinkData appsinkData;

GLuint mProgramID;
GLuint id_y, id_u, id_v; // texture id
GLuint textureUniformY, textureUniformU,textureUniformV;
static GLuint _vertexArray;
static GLuint _vertexBuffer[2];
static const int FPS = 60;
GLfloat currentAngleOfRotation = 0.0;

static const char *VSHADER =
	"attribute vec2 vertexIn;"
	"attribute vec2 textureIn;"
	"varying vec2 textureOut;"
	"uniform mat4 trans;"
	"void main() {"
		"gl_Position = trans * vec4(vertexIn,0, 1);"
		"textureOut = textureIn;"
	"}";

static const char *FSHADER =
	"#ifdef GL_ES \n"
		"  precision mediump float; \n"
	"#endif \n"
	"varying vec2 textureOut;"
	"uniform sampler2D tex_y;"
	"uniform sampler2D tex_u;"
	"uniform sampler2D tex_v;"
	"void main() {"
		"vec3 yuv;"
		"vec3 rgb;"
		"yuv.x = texture2D(tex_y, textureOut).r;"
		"yuv.y = texture2D(tex_u, textureOut).r - 0.5;"
		"yuv.z = texture2D(tex_v, textureOut).r - 0.5;"
		"rgb = mat3( 1, 1, 1, 0, -0.39465, 2.03211, 1.13983, -0.58060,  0) * yuv;"
		"gl_FragColor = vec4(rgb, 1);"
	"}";

static GLuint LoadShader( GLenum type )
{
	GLuint shaderHandle = 0;
	const char *sources[1];

	if(GL_VERTEX_SHADER == type)
	{
		sources[0] = VSHADER;
	}
	else
	{
		sources[0] = FSHADER;
	}

	if( sources[0] )
	{
		shaderHandle = glCreateShader(type);
		glShaderSource(shaderHandle, 1, sources, 0);
		glCompileShader(shaderHandle);
		GLint compileSuccess;
		glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
		if (compileSuccess == GL_FALSE)
		{
			GLchar msg[1024];
			glGetShaderInfoLog(shaderHandle, sizeof(msg), 0, &msg[0]);
			logprintf( "%s\n", msg );
		}
	}

	return shaderHandle;
}

void InitShaders()
{
	GLint linked;

	GLint vShader = LoadShader(GL_VERTEX_SHADER);
	GLint fShader = LoadShader(GL_FRAGMENT_SHADER);
	mProgramID = glCreateProgram();
	glAttachShader(mProgramID,vShader);
	glAttachShader(mProgramID,fShader);

	glBindAttribLocation(mProgramID, ATTRIB_VERTEX, "vertexIn");
	glBindAttribLocation(mProgramID, ATTRIB_TEXTURE, "textureIn");
	glLinkProgram(mProgramID);
	glValidateProgram(mProgramID);

	glGetProgramiv(mProgramID, GL_LINK_STATUS, &linked);
	if( linked == GL_FALSE )
	{
		GLint logLen;
		glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &logLen);
		GLchar *msg = (GLchar *)malloc(sizeof(GLchar)*logLen);
		glGetProgramInfoLog(mProgramID, logLen, &logLen, msg );
		printf( "%s\n", msg );
		free( msg );
	}
	glUseProgram(mProgramID);
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	//Get Uniform Variables Location
	textureUniformY = glGetUniformLocation(mProgramID, "tex_y");
	textureUniformU = glGetUniformLocation(mProgramID, "tex_u");
	textureUniformV = glGetUniformLocation(mProgramID, "tex_v");

	typedef struct _vertex
	{
		float p[2];
		float uv[2];
	} Vertex;

	static const Vertex vertexPtr[4] =
	{
		{{-1,-1}, {0.0,1 } },
		{{ 1,-1}, {1,1 } },
		{{ 1, 1}, {1,0.0 } },
		{{-1, 1}, {0.0,0.0} }
	};
	static const unsigned short index[6] =
	{
		0,1,2, 2,3,0
	};

	glGenVertexArrays(1, &_vertexArray);
	glBindVertexArray(_vertexArray);
	glGenBuffers(2, _vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPtr), vertexPtr, GL_STATIC_DRAW );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vertexBuffer[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW );
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE,
							sizeof(Vertex), (const GLvoid *)offsetof(Vertex,p) );
	glEnableVertexAttribArray(ATTRIB_VERTEX);

	glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, GL_FALSE,
						  sizeof(Vertex), (const GLvoid *)offsetof(Vertex, uv ) );
	glEnableVertexAttribArray(ATTRIB_TEXTURE);
	glBindVertexArray(0);

	glGenTextures(1, &id_y);
	glBindTexture(GL_TEXTURE_2D, id_y);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &id_u);
	glBindTexture(GL_TEXTURE_2D, id_u);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &id_v);
	glBindTexture(GL_TEXTURE_2D, id_v);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void glRender(void){
	/** Input in I420 (YUV420) format.
	  * Buffer structure:
	  * ----------
	  * |        |
	  * |   Y    | size = w*h
	  * |        |
	  * |________|
	  * |   U    |size = w*h/4
	  * |--------|
	  * |   V    |size = w*h/4
	  * ----------*
	  */
	int pixel_w = 0;
	int pixel_h = 0;
	uint8_t *yuvBuffer = NULL;
	unsigned char *yPlane, *uPlane, *vPlane;

	{
		std::lock_guard<std::mutex> lock(appsinkData.mutex);
		yuvBuffer = appsinkData.yuvBuffer;
		appsinkData.yuvBuffer = NULL;
		pixel_w = appsinkData.width;
		pixel_h = appsinkData.height;
	}
	if(yuvBuffer)
	{
		yPlane = yuvBuffer;
		uPlane = yPlane + (pixel_w*pixel_h);
		vPlane = uPlane + (pixel_w*pixel_h)/4;

		glClearColor(0.0,0.0,0.0,0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		//Y
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id_y);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w, pixel_h, 0, GL_RED, GL_UNSIGNED_BYTE, yPlane);
		glUniform1i(textureUniformY, 0);

		//U
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, id_u);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w/2, pixel_h/2, 0, GL_RED, GL_UNSIGNED_BYTE, uPlane);
		glUniform1i(textureUniformU, 1);

		//V
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, id_v);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, pixel_w/2, pixel_h/2, 0, GL_RED, GL_UNSIGNED_BYTE, vPlane);
		glUniform1i(textureUniformV, 2);

		//Rotate
		glm::mat4 trans = glm::rotate(
			glm::mat4(1.0f),
			currentAngleOfRotation * 360,
			glm::vec3(1.0f, 1.0f, 1.0f)
		);
		GLint uniTrans = glGetUniformLocation(mProgramID, "trans");
		glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

		glBindVertexArray(_vertexArray);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0 );
		glBindVertexArray(0);

		glutSwapBuffers();
		delete yuvBuffer;
	}
}

void updateYUVFrame(uint8_t *buffer, int size, int width, int height)
{
	uint8_t* frameBuf = new uint8_t[size];
	memcpy(frameBuf, buffer, size);

	{
		std::lock_guard<std::mutex> lock(appsinkData.mutex);
		if(appsinkData.yuvBuffer)
		{
			logprintf("Drops frame.\n");
			delete appsinkData.yuvBuffer;
		}
		appsinkData.yuvBuffer = frameBuf;
		appsinkData.width = width;
		appsinkData.height = height;
	}
}

void timer(int v) {
	currentAngleOfRotation += 0.0001;
	if (currentAngleOfRotation >= 1.0)
	{
		currentAngleOfRotation = 0.0;
	}
	glutPostRedisplay();

	glutTimerFunc(1000/FPS, timer, v);
}
#endif

/**
 * @brief
 * @param argc
 * @param argv
 * @retval
 */
int main(int argc, char **argv)
{

#ifdef IARM_MGR
	char Init_Str[] = "aamp-cli";
	IARM_Bus_Init(Init_Str);
	IARM_Bus_Connect();
	try
	{
		device::Manager::Initialize();
		logprintf("device::Manager::Initialize() succeeded\n");

	}
	catch (...)
	{
		logprintf("device::Manager::Initialize() failed\n");
	}
#endif
	char driveName = (*argv)[0];
	AampLogManager mLogManager;
	AampLogManager::disableLogRedirection = true;
	ABRManager mAbrManager;

	/* Set log directory path for AAMP and ABR Manager */
	mLogManager.setLogAndCfgDirectory(driveName);
	mAbrManager.setLogDirectory(driveName);

	logprintf("**************************************************************************\n");
	logprintf("** ADVANCED ADAPTIVE MICRO PLAYER (AAMP) - COMMAND LINE INTERFACE (CLI) **\n");
	logprintf("**************************************************************************\n");

	InitPlayerLoop(0,NULL);

	mSingleton = new PlayerInstanceAAMP(
#ifdef AAMP_RENDER_IN_APP
			NULL
			,updateYUVFrame
#endif
			);
#ifdef LOG_CLI_EVENTS
	myEventListener = new myAAMPEventListener();
	mSingleton->RegisterEvents(myEventListener);
#endif

#ifdef WIN32
	FILE *f = fopen(mLogManager.getAampCliCfgPath(), "rb");
#elif defined(__APPLE__)
	std::string cfgPath(getenv("HOME"));
	cfgPath += "/aampcli.cfg";
	FILE *f = fopen(cfgPath.c_str(), "rb");
#else
	FILE *f = fopen("/opt/aampcli.cfg", "rb");
#endif
	if (f)
	{
		logprintf("opened aampcli.cfg\n");
		char buf[MAX_URI_LENGTH * 2];
		while (fgets(buf, sizeof(buf), f))
		{
			ProcessCLIConfEntry(buf);
		}
		fclose(f);
	}

	pthread_t cmdThreadId;
	pthread_create(&cmdThreadId,NULL,run_commnds,NULL);
#ifdef AAMP_RENDER_IN_APP
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(640, 480);
	glutCreateWindow("AAMP Texture Player");
	logprintf("OpenGL Version[%s] GLSL Version[%s]\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
#ifndef __APPLE__
	glewInit();
#endif
	InitShaders();
	glutDisplayFunc(glRender);
	glutTimerFunc(40, timer, 0);

	glutMainLoop();
#else
#ifdef __APPLE__
	createAndRunCocoaWindow();
#endif
#endif
	void *value_ptr = NULL;
	pthread_join(cmdThreadId, &value_ptr);
	mSingleton->Stop();
	delete mSingleton;
}
