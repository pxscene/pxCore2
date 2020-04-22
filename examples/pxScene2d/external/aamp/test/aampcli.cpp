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

#ifdef RENDER_FRAMES_IN_APP_CONTEXT
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
#include <string>
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

#define MAX_BUFFER_LENGTH 4096
static PlayerInstanceAAMP *mSingleton = NULL;
static PlayerInstanceAAMP *mBackgroundPlayer = NULL;
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

/**
 * @enum AAMPGetTypes
 * @brief Define the enum values of get types
 */
typedef enum {
	eAAMP_GET_CurrentAudioLan = 1,
	eAAMP_GET_CurrentDrm,
	eAAMP_GET_PlaybackPosition,
	eAAMP_GET_PlaybackDuration,
	eAAMP_GET_VideoBitrate,
	eAAMP_GET_AudioBitrate,
	eAAMP_GET_AudioVolume,
	eAAMP_GET_PlaybackRate,
	eAAMP_GET_VideoBitrates,
	eAAMP_GET_AudioBitrates,
	eAAMP_GET_CurrentPreferredLanguages
}AAMPGetTypes;

/**
 * @enum AAMPSetTypes
 * @brief Define the enum values of set types
 */
typedef enum{
	eAAMP_SET_RateAndSeek = 1,
	eAAMP_SET_VideoRectangle,
	eAAMP_SET_VideoZoom,
	eAAMP_SET_VideoMute,
	eAAMP_SET_AudioVolume,
	eAAMP_SET_Language,
	eAAMP_SET_SubscribedTags,
	eAAMP_SET_LicenseServerUrl,
	eAAMP_SET_AnonymouseRequest,
	eAAMP_SET_VodTrickplayFps,
	eAAMP_SET_LinearTrickplayFps,
	eAAMP_SET_LiveOffset,
	eAAMP_SET_StallErrorCode,
	eAAMP_SET_StallTimeout,
	eAAMP_SET_ReportInterval,
	eAAMP_SET_VideoBitarate,
	eAAMP_SET_InitialBitrate,
	eAAMP_SET_InitialBitrate4k,
	eAAMP_SET_NetworkTimeout,
	eAAMP_SET_ManifestTimeout,
	eAAMP_SET_DownloadBufferSize,
	eAAMP_SET_PreferredDrm,
	eAAMP_SET_StereoOnlyPlayback,
	eAAMP_SET_AlternateContent,
	eAAMP_SET_NetworkProxy,
	eAAMP_SET_LicenseReqProxy,
	eAAMP_SET_DownloadStallTimeout,
	eAAMP_SET_DownloadStartTimeout,
	eAAMP_SET_PreferredSubtitleLang,
	eAAMP_SET_ParallelPlaylistDL,
	eAAMP_SET_PreferredLanguages,
	eAAMP_SET_RampDownLimit,
	eAAMP_SET_MinimumBitrate,
	eAAMP_SET_MaximumBitrate,
	eAAMP_SET_MaximumSegmentInjFailCount,
	eAAMP_SET_MaximumDrmDecryptFailCount,
	eAAMP_SET_RegisterForID3MetadataEvents,
}AAMPSetTypes;

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
		logprintf("AAMPGstPlayer_StreamThread: exited main event loop");
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
		logprintf("%s(): Exit", __FUNCTION__);
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
		logprintf("Channel Map from aampcli.cfg\n*************************");

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

	logprintf("List of Commands\n****************");
	logprintf("<channelNumber>\t\t// Play selected channel from guide");
	logprintf("<url>\t\t\t// Play arbitrary stream");
	logprintf("pause play stop status flush // Playback options");
	logprintf("sf, ff<x> rw<y>\t\t// Trickmodes (x <= 128. y <= 128)");
	logprintf("cache <url>/<channelNumer>\t// Cache a channel in the background");
	logprintf("toggle\t\t\t// Toggle the background channel & foreground channel");
	logprintf("stopb\t\t\t// Stop background channel.");
	logprintf("+ -\t\t\t// Change profile");
	logprintf("bps <x>\t\t\t// set bitrate ");
	logprintf("sap\t\t\t// Use SAP track (if avail)");
	logprintf("seek <seconds>\t\t// Specify start time within manifest");
	logprintf("live\t\t\t// Seek to live point");
	logprintf("underflow\t\t\t// Simulate underflow");
	logprintf("retune\t\t\t// schedule retune");
	logprintf("help\t\t\t// Show this list again");
	logprintf("get help // Show help of get command");
	logprintf("set help // Show help of set command");
	logprintf("exit\t\t\t// Exit from application");
}

/**
 * @brief Display Help menu for set
 * @param none
 */
void ShowHelpGet(){
	logprintf("*******************************************************************");
	logprintf("*   get <command> <arguments> ");
	logprintf("*   List of Commands, arguemnts expected in ()");
	logprintf("*******************************************************************");
	logprintf("1 - Print Current audio language ()");
	logprintf("2 - Print Current DRM ()");
	logprintf("3 - Print Current Playback position ()");
	logprintf("4 - Print Playback Duration ()");
	logprintf("5 - Print current video bitrate ()");
	logprintf("6 - Print current Audio bitrate ()");
	logprintf("7 - Print current Audio voulume ()");
	logprintf("8 - Print Current Playback rate ()");
	logprintf("9 - Print Video bitrates supported ()");
	logprintf("10 - Print Audio bitrates supported ()");
	logprintf("11 - Print Current preferred languages ()");
	
}

/**
 * @brief Display Help menu for set
 * @param none
 */
void ShowHelpSet(){
	logprintf("*******************************************************************");
	logprintf("*   set <command> <arguments> ");
	logprintf("*   List of Commands, arguemnts expected in ()");
	logprintf("*******************************************************************");
	logprintf("1 - Set Rate and Seek (int rate, double secondsRelativeToTuneTime)");
	logprintf("2 - Set Video Rectangle (int x,y,w,h)");
	logprintf("3 - Set Video zoom  ( 1 - full, 0 - normal)");
	logprintf("4 - Set Video Mute ( 1 - Mute , 0 - Unmute)");
	logprintf("5 - Set Audio Voulume (int volume)");
	logprintf("6 - Set Language (string lang)");
	logprintf("7 - Set Subscribed Tag () - dummy");
	logprintf("8 - Set License Server URL (String url)");
	logprintf("9 - Set Anonymouse Request  (0/1)");
	logprintf("10 - Set VOD Trickplay FPS (int trickPlayFPS)");
	logprintf("11 - Set Linear Trickplay FPS (int trickPlayFPS)");
	logprintf("12 - Set Live offset (int offset)");
	logprintf("13 - Set Stall error code (int errorCode)");
	logprintf("14 - Set Stall timeout (int timeout)");
	logprintf("15 - Set Report Interval (int interval)");
	logprintf("16 - Set Video Bitrate (long bitrate)");
	logprintf("17 - Set Initial Bitrate (long bitrate)");
	logprintf("18 - Set Initial Bitrate 4K (long bitrate4k)");
	logprintf("19 - Set Network Timeout (long timeout in ms)");
	logprintf("20 - Set Manifest Timeout (long timeout in ms)");
	logprintf("21 - Set Download Buffer Size (int bufferSize)");
	logprintf("22 - Set Preferred DRM (1 - Wideine, 2 - Playready, 4- Adobe_Access, 5 - Vanila AES, 6 - Clear Key)"); 
	logprintf("23 - Set Stereo only playback (1/0)");
	logprintf("24 - Set Alternate Contents - dummy ()");
	logprintf("25 - Set Set Network Proxy (string url)");
	logprintf("26 - Set License Request Proxy (string url)");
	logprintf("27 - Set Download Stall timeout (long timeout)");
	logprintf("28 - Set Download Start timeout (long timeout)");
	logprintf("29 - Set Preferred Subtitle language (string lang)");
	logprintf("30 - Set Parallel Playlist download (0/1)");
	logprintf("31 - Set Preferred languages (string \"lang1, lang2, ...\")");
	logprintf("32 - Set Ramp Down limit");
	logprintf("33 - Set Minimum bitrate");
	logprintf("34 - Set Maximum bitrate");
	logprintf("35 - Set Maximum segment injection fail count");
	logprintf("36 - Set Maximum DRM Decryption fail count");
	logprintf("37 - Set Listen for ID3_METADATA events (1 - add listener, 0 - remove listener) ");
}

#define LOG_CLI_EVENTS
#ifdef LOG_CLI_EVENTS
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
		case AAMP_EVENT_MEDIA_METADATA:
			logprintf("AAMP_EVENT_MEDIA_METADATA\n" );
			for( int i=0; i<e.data.metadata.languageCount; i++ )
			{
				logprintf( "language: %s\n", e.data.metadata.languages[i] );
			}
			break;
		case AAMP_EVENT_TUNED:
			logprintf("AAMP_EVENT_TUNED");
			break;
		case AAMP_EVENT_TUNE_FAILED:
			logprintf("AAMP_EVENT_TUNE_FAILED");
			break;
		case AAMP_EVENT_SPEED_CHANGED:
			logprintf("AAMP_EVENT_SPEED_CHANGED");
			break;
		case AAMP_EVENT_DRM_METADATA:
                        logprintf("AAMP_DRM_FAILED");
                        break;
		case AAMP_EVENT_EOS:
			logprintf("AAMP_EVENT_EOS");
			break;
		case AAMP_EVENT_PLAYLIST_INDEXED:
			logprintf("AAMP_EVENT_PLAYLIST_INDEXED");
			break;
		case AAMP_EVENT_PROGRESS:
			//			logprintf("AAMP_EVENT_PROGRESS");
			break;
		case AAMP_EVENT_CC_HANDLE_RECEIVED:
			logprintf("AAMP_EVENT_CC_HANDLE_RECEIVED");
			break;
		case AAMP_EVENT_BITRATE_CHANGED:
			logprintf("AAMP_EVENT_BITRATE_CHANGED");
			break;
		case AAMP_EVENT_ID3_METADATA:
			logprintf("AAMP_EVENT_ID3_METADATA");

			logprintf("ID3 payload, length %d bytes:", e.data.id3Metadata.length);
			printf("\t");
			for (int i = 0; i < e.data.id3Metadata.length; i++)
			{
				printf("%c", *(e.data.id3Metadata.data+i));
			}
			printf("\n");
			break;
		default:
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
					else if (memcmp(token, "http", 4) == 0 || memcmp(token, "https", 5) == 0)
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
					logprintf("%s(): Could not parse uri of %s", __FUNCTION__, cfg);
				}
			}
	}
}

inline void StopCachedChannel()
{
	if(mBackgroundPlayer)
	{
		mBackgroundPlayer->Stop();
		delete mBackgroundPlayer;
		mBackgroundPlayer = NULL;
	}
}

void CacheChannel(const char *url)
{
	StopCachedChannel();
	mBackgroundPlayer = new PlayerInstanceAAMP(
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
			NULL
			,updateYUVFrame
#endif
			);
	mBackgroundPlayer->RegisterEvents(myEventListener);
	mBackgroundPlayer->Tune(url, false);
}

/**
 * @brief Process command
 * @param cmd command
 */
static void ProcessCliCommand(char *cmd)
{
	double seconds = 0;
	int rate = 0;
	char lang[MAX_LANGUAGE_TAG_LENGTH];
	char cacheUrl[200];
	trim(&cmd);
	if (cmd[0] == 0)
	{
		if (mSingleton->aamp->mpStreamAbstractionAAMP)
		{
			mSingleton->aamp->mpStreamAbstractionAAMP->DumpProfiles();
		}
		logprintf("current bitrate ~= %ld", mSingleton->aamp->GetCurrentlyAvailableBandwidth());
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
		logprintf("channel number: %d", channelNumber);
		for (std::list<VirtualChannelInfo>::iterator it = mVirtualChannelMap.begin(); it != mVirtualChannelMap.end(); ++it)
		{
			VirtualChannelInfo &channelInfo = *it;
			if(channelInfo.channelNumber == channelNumber)
			{
			//	logprintf("Found %d tuning to %s",channelInfo.channelNumber, channelInfo.uri.c_str());
				mSingleton->Tune(channelInfo.uri.c_str());
				break;
			}
		}
	}
	else if (sscanf(cmd, "cache %s", cacheUrl) == 1)
	{
		if (memcmp(cacheUrl, "http", 4) ==0)
		{
			CacheChannel(cacheUrl);
		}
		else
		{
			int channelNumber = atoi(cacheUrl);
			logprintf("channel number: %d", channelNumber);
			for (std::list<VirtualChannelInfo>::iterator it = mVirtualChannelMap.begin(); it != mVirtualChannelMap.end(); ++it)
			{
				VirtualChannelInfo &channelInfo = *it;
				if(channelInfo.channelNumber == channelNumber)
				{
					CacheChannel(channelInfo.uri.c_str());
					break;
				}
			}
		}
	}
	else if (strcmp(cmd, "toggle") == 0)
	{
		if(mBackgroundPlayer && mSingleton)
		{
			mSingleton->detach();
			mBackgroundPlayer->SetRate(AAMP_NORMAL_PLAY_RATE);

			PlayerInstanceAAMP *tmpPlayer = mSingleton;
			mSingleton = mBackgroundPlayer;
			mBackgroundPlayer = tmpPlayer;
			StopCachedChannel();
		}
	}
	else if (strcmp(cmd, "stopb") == 0)
	{
		StopCachedChannel();
	}
	else if (sscanf(cmd, "seek %lf", &seconds) == 1)
	{
		mSingleton->Seek(seconds);
	}
	else if (strcmp(cmd, "sf") == 0)
	{
		mSingleton->SetRate((int)0.5);
	}
	else if (sscanf(cmd, "ff%d", &rate) == 1)
	{
		if (rate >= 128)
		{
			logprintf("Speed not supported.");
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
		if (rate >= 128)
		{
			logprintf("Speed not supported.");
		}
		else
		{
			mSingleton->SetRate((float)(-rate));
		}
	}
	else if (sscanf(cmd, "bps %d", &rate) == 1)
	{
		logprintf("Set video bitrate %d.", rate);
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
		if (mBackgroundPlayer)
			delete mBackgroundPlayer;
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
				logprintf("Set zoom to full");
				mSingleton->SetVideoZoom(VIDEO_ZOOM_FULL);
			}
			else
			{
				logprintf("Set zoom to none");
				mSingleton->SetVideoZoom(VIDEO_ZOOM_NONE);
			}
		}
	}
	else if( sscanf(cmd, "sap %s",lang ) )
	{
		size_t len = strlen(lang);
		logprintf("aamp cli sap called for language %s\n",lang);
		if( len>0 )
		{
			mSingleton->SetLanguage( lang );
		}
		else
		{
			logprintf( "GetCurrentAudioLanguage: '%s'\n", mSingleton->GetCurrentAudioLanguage() );
		}
	}
    else if( strcmp(cmd,"getplaybackrate") == 0 )
	{
		logprintf("Playback Rate: %d\n", mSingleton->GetPlaybackRate());
	}
	else if (memcmp(cmd, "islive", 6) == 0 )
	{
		logprintf(" VIDEO IS %s ", 
		(mSingleton->IsLive() == true )? "LIVE": "NOT LIVE");
	}
	else if (memcmp(cmd, "customheader", 12) == 0 )
	{
		//Dummy implimenations
		std::vector<std::string> headerValue;
		logprintf("customheader Command is %s " , cmd); 
		mSingleton->AddCustomHTTPHeader("", headerValue, false);
	}
	else if (memcmp(cmd, "set", 3) == 0 )
	{
		char help[8];
		int opt;
		if (sscanf(cmd, "set %d", &opt) == 1){
			switch(opt){
				case eAAMP_SET_RateAndSeek:
                                {
					int rate;
					double ralatineTuneTime;
					logprintf("Matchde Command eAAMP_SET_RateAndSeek - %s ", cmd);
					if (sscanf(cmd, "set %d %d %lf", &opt, &rate, &ralatineTuneTime ) == 3){
						mSingleton->SetRateAndSeek(rate, ralatineTuneTime);
					}
					break;
                                }
				case eAAMP_SET_VideoRectangle:
                                {
                                        int x,y,w,h;
					logprintf("Matchde Command eAAMP_SET_VideoRectangle - %s ", cmd);
					if (sscanf(cmd, "set %d %d %d %d %d", &opt, &x, &y, &w, &h) == 5){
						mSingleton->SetVideoRectangle(x,y,w,h);
					}
					break;
                                }
				case eAAMP_SET_VideoZoom:
                                {
					int videoZoom;
					logprintf("Matchde Command eAAMP_SET_VideoZoom - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &videoZoom) == 2){
						mSingleton->SetVideoZoom((videoZoom > 0 )? VIDEO_ZOOM_FULL : VIDEO_ZOOM_NONE );
					}
					break;
                                }

				case eAAMP_SET_VideoMute:
                                {
					int videoMute;
					logprintf("Matchde Command eAAMP_SET_VideoMute - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &videoMute) == 2){
						mSingleton->SetVideoMute((videoMute == 1 )? true : false );
					}
					break;	
                                }

				case eAAMP_SET_AudioVolume:
                                {
                                        int vol;
					logprintf("Matchde Command eAAMP_SET_AudioVolume - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &vol) == 2){
						mSingleton->SetAudioVolume(vol);
					}
					break;
                                }

				case eAAMP_SET_Language:
                                {
					char lang[12];
					logprintf("Matchde Command eAAMP_SET_Language - %s ", cmd);
					if (sscanf(cmd, "set %d %s", &opt, lang) == 2){
						mSingleton->SetLanguage(lang);
					}
					break;
                                }
				case eAAMP_SET_SubscribedTags:
			        {
                                        //Dummy implimentation
					std::vector<std::string> subscribedTags;
					logprintf("Matchde Command eAAMP_SET_SubscribedTags - %s ", cmd);
					mSingleton->SetSubscribedTags(subscribedTags);
					break;
                                }
				case eAAMP_SET_LicenseServerUrl:
                                {
                                        char lisenceUrl[1024];
					int drmType;
					logprintf("Matchde Command eAAMP_SET_LicenseServerUrl - %s ", cmd);
					if (sscanf(cmd, "set %d %s %d", &opt, lisenceUrl, &drmType) == 3){
						mSingleton->SetLicenseServerURL(lisenceUrl, 
						(drmType == eDRM_PlayReady)?eDRM_PlayReady:eDRM_WideVine);
					}
					break;
                                }
				case eAAMP_SET_AnonymouseRequest:
                                {
                                        int isAnonym;
					logprintf("Matchde Command eAAMP_SET_AnonymouseRequest - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &isAnonym) == 2){
						mSingleton->SetAnonymousRequest((isAnonym == 1)?true:false);
					}
					break;
                                }
				case eAAMP_SET_VodTrickplayFps:
                                {
                                        int vodTFps;
					logprintf("Matchde Command eAAMP_SET_VodTrickplayFps - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &vodTFps) == 2){
						mSingleton->SetVODTrickplayFPS(vodTFps);
					}
					break;
                                }
				case eAAMP_SET_LinearTrickplayFps:
                                {
                                        int linearTFps;
					logprintf("Matchde Command eAAMP_SET_LinearTrickplayFps - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &linearTFps) == 2){
						mSingleton->SetLinearTrickplayFPS(linearTFps);
					}
					break;
                                }
				case eAAMP_SET_LiveOffset:
                                {
                                        int liveOffset;
					logprintf("Matchde Command eAAMP_SET_LiveOffset - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &liveOffset) == 2){
						mSingleton->SetLiveOffset(liveOffset);
					}
					break;
                                }
				case eAAMP_SET_StallErrorCode:
                                {
                                        int stallErrorCode;
					logprintf("Matchde Command eAAMP_SET_StallErrorCode - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &stallErrorCode) == 2){
						mSingleton->SetStallErrorCode(stallErrorCode);
					}
					break;
                                }
				case eAAMP_SET_StallTimeout:
                                {
                                        int stallTimeout;
					logprintf("Matchde Command eAAMP_SET_StallTimeout - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &stallTimeout) == 2){
						mSingleton->SetStallTimeout(stallTimeout);
					}
					break;
                                }

				case eAAMP_SET_ReportInterval:
                                {
                                        int reportInterval;
					logprintf("Matchde Command eAAMP_SET_ReportInterval - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &reportInterval) == 2){
						mSingleton->SetReportInterval(reportInterval);
					}
					break;
                                }
				case eAAMP_SET_VideoBitarate:
                                {
                                        long videoBitarate;
					logprintf("Matchde Command eAAMP_SET_VideoBitarate - %s ", cmd);
					if (sscanf(cmd, "set %d %ld", &opt, &videoBitarate) == 2){
						mSingleton->SetVideoBitrate(videoBitarate);
					}
					break;
                                }
                                case eAAMP_SET_InitialBitrate:
                                {
                                        long initialBitrate;
					logprintf("Matchde Command eAAMP_SET_InitialBitrate - %s ", cmd);
					if (sscanf(cmd, "set %d %ld", &opt, &initialBitrate) == 2){
						mSingleton->SetInitialBitrate(initialBitrate);
					}
					break;
                                }
				case eAAMP_SET_InitialBitrate4k:
                                {
                                        long initialBitrate4k;
					logprintf("Matchde Command eAAMP_SET_InitialBitrate4k - %s ", cmd);
					if (sscanf(cmd, "set %d %ld", &opt, &initialBitrate4k) == 2){
						mSingleton->SetInitialBitrate4K(initialBitrate4k);
					}
					break;
                                }

				case eAAMP_SET_NetworkTimeout:
                                {
                                        double networkTimeout;
					logprintf("Matchde Command eAAMP_SET_NetworkTimeout - %s ", cmd);
					if (sscanf(cmd, "set %d %f", &opt, &networkTimeout) == 2){
						mSingleton->SetNetworkTimeout(networkTimeout);
					}
					break;
                                }
				case eAAMP_SET_ManifestTimeout:
                                {
                                        double manifestTimeout;
					logprintf("Matchde Command eAAMP_SET_ManifestTimeout - %s ", cmd);
					if (sscanf(cmd, "set %d %lf", &opt, &manifestTimeout) == 2){
						mSingleton->SetManifestTimeout(manifestTimeout);
					}
					break;
                                }

				case eAAMP_SET_DownloadBufferSize:
                                {
                                        int downloadBufferSize;
					logprintf("Matchde Command eAAMP_SET_DownloadBufferSize - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &downloadBufferSize) == 2){
						mSingleton->SetDownloadBufferSize(downloadBufferSize);
					}
					break;
                                }

				case eAAMP_SET_PreferredDrm:
                                {
                                        int preferredDrm;
					logprintf("Matchde Command eAAMP_SET_PreferredDrm - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &preferredDrm) == 2){
						mSingleton->SetPreferredDRM((DRMSystems)preferredDrm);
					}
					break;
                                }

				case eAAMP_SET_StereoOnlyPlayback:
                                {
                                        int stereoOnlyPlayback;
					logprintf("Matchde Command eAAMP_SET_StereoOnlyPlayback - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &stereoOnlyPlayback) == 2){
						mSingleton->SetStereoOnlyPlayback(
							(stereoOnlyPlayback == 1 )? true:false);
					}
					break;
                                }

				case eAAMP_SET_AlternateContent:
                                {
                                        //Dummy implimentation
					std::string adBrkId = "";
					std::string adId = "";
					std::string url = "";
					logprintf("Matchde Command eAAMP_SET_AlternateContent - %s ", cmd);
					mSingleton->SetAlternateContents(adBrkId, adId, url);
					break;
                                }

				case eAAMP_SET_NetworkProxy:
                                {
                                        char networkProxy[128];
					logprintf("Matchde Command eAAMP_SET_NetworkProxy - %s ", cmd);
					if (sscanf(cmd, "set %d %s", &opt, networkProxy) == 2){
						mSingleton->SetNetworkProxy(networkProxy);
					}
					break;
                                }
				case eAAMP_SET_LicenseReqProxy:
                                {
                                        char licenseReqProxy[128];
					logprintf("Matchde Command eAAMP_SET_LicenseReqProxy - %s ", cmd);
					if (sscanf(cmd, "set %d %s", &opt, licenseReqProxy) == 2){
						mSingleton->SetLicenseReqProxy(licenseReqProxy);
					}
					break;
                                }
				case eAAMP_SET_DownloadStallTimeout:
                                {
                                        long downloadStallTimeout;
					logprintf("Matchde Command eAAMP_SET_DownloadStallTimeout - %s ", cmd);
					if (sscanf(cmd, "set %d %ld", &opt, &downloadStallTimeout) == 2){
						mSingleton->SetDownloadStallTimeout(downloadStallTimeout);
					}
					break;
                                }
				case eAAMP_SET_DownloadStartTimeout:
                                {
                                        long downloadStartTimeout;
					logprintf("Matchde Command eAAMP_SET_DownloadStartTimeout - %s ", cmd);
					if (sscanf(cmd, "set %d %ld", &opt, &downloadStartTimeout) == 2){
						mSingleton->SetDownloadStartTimeout(downloadStartTimeout);
					}
					break;
                                }

				case eAAMP_SET_PreferredSubtitleLang:
                                {
					char preferredSubtitleLang[12];
                                        logprintf("Matchde Command eAAMP_SET_PreferredSubtitleLang - %s ", cmd);
					if (sscanf(cmd, "set %d %s", &opt, preferredSubtitleLang) == 2){
						mSingleton->SetPreferredSubtitleLanguage(preferredSubtitleLang);
					}
					break;
                                }
				
				case eAAMP_SET_ParallelPlaylistDL:
                                {
					int parallelPlaylistDL;
					logprintf("Matchde Command eAAMP_SET_ParallelPlaylistDL - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &parallelPlaylistDL) == 2){
						mSingleton->SetParallelPlaylistDL( (parallelPlaylistDL == 1)? true:false );
					}
					break;
                                }
				case eAAMP_SET_PreferredLanguages:
				{
					logprintf("Matched Command eAAMP_SET_PreferredLanguages - %s ", cmd);
					const char* listStart = NULL;
					const char* listEnd = NULL;
					if((listStart = strchr(cmd, '"')) == NULL
							|| ( strlen(listStart+1) && (listEnd = strchr(listStart+1, '"')) ) == NULL)
					{
						logprintf("preferred languages string has to be wrapped with \" \" ie \"eng, ger\"");
						break;
					}

					std::string preferredLanguages(listStart+1, listEnd-listStart-1);
					if(!preferredLanguages.empty())
						mSingleton->SetPreferredLanguages(preferredLanguages.c_str());
					else
						mSingleton->SetPreferredLanguages(NULL);
					break;
				}

				case eAAMP_SET_RampDownLimit:
                                {
					int rampDownLimit;
					logprintf("Matched Command eAAMP_SET_RampDownLimit - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &rampDownLimit) == 2){
						mSingleton->SetRampDownLimit(rampDownLimit);
					}
					break;
                                }

				case eAAMP_SET_MinimumBitrate:
                                {
					long minBitrate;
					logprintf("Matched Command eAAMP_SET_MinimumBitrate - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &minBitrate) == 2){
						mSingleton->SetMinimumBitrate(minBitrate);
					}
					break;
                                }

				case eAAMP_SET_MaximumBitrate:
                                {
					long maxBitrate;
					logprintf("Matched Command eAAMP_SET_MaximumBitrate - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &maxBitrate) == 2){
						mSingleton->SetMaximumBitrate(maxBitrate);
					}
					break;
                                }

				case eAAMP_SET_MaximumSegmentInjFailCount:
                                {
					int failCount;
					logprintf("Matched Command eAAMP_SET_MaximumSegmentInjFailCount - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &failCount) == 2){
						mSingleton->SetSegmentInjectFailCount(failCount);
					}
					break;
                                }

				case eAAMP_SET_MaximumDrmDecryptFailCount:
                                {
					int failCount;
					logprintf("Matched Command eAAMP_SET_MaximumDrmDecryptFailCount - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &failCount) == 2){
						mSingleton->SetSegmentDecryptFailCount(failCount);
					}
					break;
                                }

				case eAAMP_SET_RegisterForID3MetadataEvents:
                                {
					bool id3MetadataEventsEnabled;
					logprintf("Matched Command eAAMP_SET_RegisterForID3MetadataEvents - %s ", cmd);
					if (sscanf(cmd, "set %d %d", &opt, &id3MetadataEventsEnabled) == 2){
						if (id3MetadataEventsEnabled)
						{
							mSingleton->AddEventListener(AAMP_EVENT_ID3_METADATA, myEventListener);
						}
						else
						{
							mSingleton->RemoveEventListener(AAMP_EVENT_ID3_METADATA, myEventListener);
						}

					}
					break;
                                }

				default:
					logprintf("Invalid set command %d\n", opt);
			}

		}
		else if (sscanf(cmd, "set %s", help) == 1)
		{
			if(0 == strncmp("help", help, 4))
			{
				ShowHelpSet();
			}else
			{
				logprintf("Invalid usage of set operations %s", help);
			}
		}
		else
		{
			logprintf("Invalid set command = %s", cmd);
		}
	}
	else if (memcmp(cmd, "get", 3) == 0 )
	{
		char help[8];
		int opt;
		if (sscanf(cmd, "get %d", &opt) == 1){
			switch(opt){
				case eAAMP_GET_CurrentAudioLan:
					logprintf(" CURRRENT AUDIO LANGUAGE = %s ",
					mSingleton->GetCurrentAudioLanguage());
					break;

				case eAAMP_GET_CurrentDrm:
					logprintf(" CURRRENT DRM  = %s ",
					mSingleton->GetCurrentDRM());
					break;

				case eAAMP_GET_PlaybackPosition:
					logprintf(" PLAYBACK POSITION = %lf ",
					mSingleton->GetPlaybackPosition());
					break;

				case eAAMP_GET_PlaybackDuration:
					logprintf(" PLAYBACK DURATION = %lf ",
					mSingleton->GetPlaybackDuration());
					break;

				case eAAMP_GET_VideoBitrate:
					logprintf(" VIDEO BITRATE = %ld ",
					mSingleton->GetVideoBitrate());
					break;

				case eAAMP_GET_AudioBitrate:
					logprintf(" AUDIO BITRATE = %ld ",
					mSingleton->GetAudioBitrate());
					break;

				case eAAMP_GET_AudioVolume:
					logprintf(" AUDIO VOULUME = %d ",
					mSingleton->GetAudioVolume());
					break;
				
				case eAAMP_GET_PlaybackRate:
					logprintf(" PLAYBACK RATE = %d ",
					mSingleton->GetPlaybackRate());
					break;

				case eAAMP_GET_VideoBitrates:
                                {
					std::vector<long int> videoBitrates;
					printf("[AAMP-PLAYER] VIDEO BITRATES = [ ");
                                        videoBitrates = mSingleton->GetVideoBitrates();
					for(int i=0; i < videoBitrates.size(); i++){
						printf("%ld, ", videoBitrates[i]);
					}
					printf(" ] \n");
					break;
                                }

				case eAAMP_GET_AudioBitrates:
                                {
					std::vector<long int> audioBitrates;
					printf("[AAMP-PLAYER] AUDIO BITRATES = [ ");
                                        audioBitrates = mSingleton->GetAudioBitrates();
					for(int i=0; i < audioBitrates.size(); i++){
						printf("%ld, ", audioBitrates[i]);
					}
					printf(" ] \n");
					break;
			        }	
				case eAAMP_GET_CurrentPreferredLanguages:
				{
					const char *prefferedLanguages = mSingleton->GetPreferredLanguages();
					logprintf(" PREFERRED LANGUAGES = \"%s\" ", prefferedLanguages? prefferedLanguages : "<NULL>");
					break;
				}
				default:
					logprintf("Invalid get command %d\n", opt);
			}

		}
		else if (sscanf(cmd, "get %s", help) == 1)
		{
			if(0 == strncmp("help", help, 4)){
				ShowHelpGet();
			}else
			{
				logprintf("Invalid usage of get operations %s", help);
			}
		}
		else
		{
			logprintf("Invalid get command = %s", cmd);
		}
	}
}

static void * run_command(void *arg)
{
    char cmd[MAX_BUFFER_LENGTH];
    ShowHelp();
    char *ret = NULL;
    do
    {
        printf("[AAMP-PLAYER] aamp-cli> ");
        if((ret = fgets(cmd, sizeof(cmd), stdin))!=NULL)
            ProcessCliCommand(cmd);
    } while (ret != NULL);
    return NULL;
}

#ifdef RENDER_FRAMES_IN_APP_CONTEXT
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
			logprintf( "%s", msg );
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
			logprintf("Drops frame.");
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
		logprintf("device::Manager::Initialize() succeeded");

	}
	catch (...)
	{
		logprintf("device::Manager::Initialize() failed");
	}
#endif
	char driveName = (*argv)[0];
	AampLogManager mLogManager;
	AampLogManager::disableLogRedirection = true;
	ABRManager mAbrManager;

	/* Set log directory path for AAMP and ABR Manager */
	mLogManager.setLogAndCfgDirectory(driveName);
	mAbrManager.setLogDirectory(driveName);

	logprintf("**************************************************************************");
	logprintf("** ADVANCED ADAPTIVE MICRO PLAYER (AAMP) - COMMAND LINE INTERFACE (CLI) **");
	logprintf("**************************************************************************");

	InitPlayerLoop(0,NULL);

	mSingleton = new PlayerInstanceAAMP(
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
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
		logprintf("opened aampcli.cfg");
		char buf[MAX_BUFFER_LENGTH];
		while (fgets(buf, sizeof(buf), f))
		{
			ProcessCLIConfEntry(buf);
		}
		fclose(f);
	}

	pthread_t cmdThreadId;
	pthread_create(&cmdThreadId,NULL,run_command,NULL);
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
	// Render frames in graphics plane using opengl
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(640, 480);
	glutCreateWindow("AAMP Texture Player");
	logprintf("OpenGL Version[%s] GLSL Version[%s]", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
#ifndef __APPLE__
	glewInit();
#endif
	InitShaders();
	glutDisplayFunc(glRender);
	glutTimerFunc(40, timer, 0);

	glutMainLoop();
#else
	// Render frames in video plane - default behavior
#ifdef __APPLE__
	createAndRunCocoaWindow();
#endif
#endif
	void *value_ptr = NULL;
	pthread_join(cmdThreadId, &value_ptr);
	if(mBackgroundPlayer)
	{
		mBackgroundPlayer->Stop();
		delete mBackgroundPlayer;
	}
	mSingleton->Stop();
	delete mSingleton;
}




