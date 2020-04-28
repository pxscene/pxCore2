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
 * @file aampgstplayer.cpp
 * @brief Gstreamer based player impl for AAMP
 */


#include "aampgstplayer.h"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h> // for sprintf
#include "priv_aamp.h"
#include <pthread.h>
#include <atomic>


#ifdef __APPLE__
	#include "gst/video/videooverlay.h"
	guintptr (*gCbgetWindowContentView)() = NULL;
#endif

#ifdef AAMP_MPD_DRM
#include "aampoutputprotection.h"
#endif


/**
 * @enum GstPlayFlags 
 * @brief Enum of configuration flags used by playbin
 */
typedef enum {
	GST_PLAY_FLAG_VIDEO = (1 << 0), // 0x001
	GST_PLAY_FLAG_AUDIO = (1 << 1), // 0x002
	GST_PLAY_FLAG_TEXT = (1 << 2), // 0x004
	GST_PLAY_FLAG_VIS = (1 << 3), // 0x008
	GST_PLAY_FLAG_SOFT_VOLUME = (1 << 4), // 0x010
	GST_PLAY_FLAG_NATIVE_AUDIO = (1 << 5), // 0x020
	GST_PLAY_FLAG_NATIVE_VIDEO = (1 << 6), // 0x040
	GST_PLAY_FLAG_DOWNLOAD = (1 << 7), // 0x080
	GST_PLAY_FLAG_BUFFERING = (1 << 8), // 0x100
	GST_PLAY_FLAG_DEINTERLACE = (1 << 9), // 0x200
	GST_PLAY_FLAG_SOFT_COLORBALANCE = (1 << 10) // 0x400
} GstPlayFlags;

//#define SUPPORT_MULTI_AUDIO
#define GST_ELEMENT_GET_STATE_RETRY_CNT_MAX 5

/*Playersinkbin events*/
#define GSTPLAYERSINKBIN_EVENT_HAVE_VIDEO 0x01
#define GSTPLAYERSINKBIN_EVENT_HAVE_AUDIO 0x02
#define GSTPLAYERSINKBIN_EVENT_FIRST_VIDEO_FRAME 0x03
#define GSTPLAYERSINKBIN_EVENT_FIRST_AUDIO_FRAME 0x04
#define GSTPLAYERSINKBIN_EVENT_ERROR_VIDEO_UNDERFLOW 0x06
#define GSTPLAYERSINKBIN_EVENT_ERROR_AUDIO_UNDERFLOW 0x07
#define GSTPLAYERSINKBIN_EVENT_ERROR_VIDEO_PTS 0x08
#define GSTPLAYERSINKBIN_EVENT_ERROR_AUDIO_PTS 0x09

#ifdef INTELCE
#define INPUT_GAIN_DB_MUTE  (gdouble)-145
#define INPUT_GAIN_DB_UNMUTE  (gdouble)0
#define DEFAULT_VIDEO_RECTANGLE "0,0,0,0"
#else
#define DEFAULT_VIDEO_RECTANGLE "0,0,1280,720"
#endif
#define DEFAULT_BUFFERING_TO_MS 10                       // TimeOut interval to check buffer fullness
#define DEFAULT_BUFFERING_QUEUED_BYTES_MIN  (128 * 1024) // prebuffer in bytes
#define DEFAULT_BUFFERING_QUEUED_FRAMES_MIN (5)          // if the video decoder has this many queued frames start.. even at 60fps, close to 100ms...
#define DEFAULT_BUFFERING_MAX_MS (1000)                  // max buffering time
#define DEFAULT_BUFFERING_MAX_CNT (DEFAULT_BUFFERING_MAX_MS/DEFAULT_BUFFERING_TO_MS)   // max buffering timeout count
#define AAMP_MIN_PTS_UPDATE_INTERVAL 4000
#define AAMP_DELAY_BETWEEN_PTS_CHECK_FOR_EOS_ON_UNDERFLOW 500

/**
 * @struct media_stream
 * @brief Holds stream(A/V) specific variables.
 */
struct media_stream
{
	GstElement *sinkbin;
	GstElement *source;
	StreamOutputFormat format;
	gboolean using_playersinkbin;
	bool flush;
	bool resetPosition;
	bool bufferUnderrun;
	bool eosReached;
};

/**
 * @struct AAMPGstPlayerPriv
 * @brief Holds private variables of AAMPGstPlayer
 */
struct AAMPGstPlayerPriv
{
	bool gstPropsDirty; //Flag used to check if gst props need to be set at start.
	media_stream stream[AAMP_TRACK_COUNT];
	GstElement *pipeline; //GstPipeline used for playback.
	GstBus *bus; //Bus for receiving GstEvents from pipeline.
	int current_rate; 
	guint64 total_bytes;
	gint n_audio; //Number of audio tracks.
	gint current_audio; //Offset of current audio track.
	guint firstProgressCallbackIdleTaskId; //ID of idle handler created for notifying first progress event.
	std::atomic<bool> firstProgressCallbackIdleTaskPending; //Set if any first progress callback is pending.
	guint periodicProgressCallbackIdleTaskId; //ID of timed handler created for notifying progress events.
	guint bufferingTimeoutTimerId; //ID of timer handler created for buffering timeout.
	guint id3MetadataCallbackIdleTaskId; //ID of handler created to send ID3 metadata events
	std::atomic<bool> id3MetadataCallbackTaskPending; //Set if an id3 metadata callback is pending
	GstElement *video_dec; //Video decoder used by pipeline.
	GstElement *audio_dec; //Audio decoder used by pipeline.
	GstElement *video_sink; //Video sink used by pipeline.
	GstElement *audio_sink; //Audio sink used by pipeline.
#ifdef INTELCE_USE_VIDRENDSINK
	GstElement *video_pproc; //Video element used by pipeline.(only for Intel).
#endif

	int rate; //Current playback rate.
	VideoZoomMode zoom; //Video-zoom setting.
	bool videoMuted; //Video mute status.
	bool audioMuted; //Audio mute status.
	double audioVolume; //Audio volume.
	guint eosCallbackIdleTaskId; //ID of idle handler created for notifying EOS event.
	std::atomic<bool> eosCallbackIdleTaskPending; //Set if any eos callback is pending.
	bool firstFrameReceived; //Flag that denotes if first frame was notified.
	char videoRectangle[32]; //Video-rectangle co-ordinates in format x,y,w,h.
	bool pendingPlayState; //Flag that denotes if set pipeline to PLAYING state is pending.
	bool decoderHandleNotified; //Flag that denotes if decoder handle was notified.
	guint firstFrameCallbackIdleTaskId; //ID of idle handler created for notifying first frame event.
	GstEvent *protectionEvent[AAMP_TRACK_COUNT]; //GstEvent holding the pssi data to be sent downstream.
	std::atomic<bool> firstFrameCallbackIdleTaskPending; //Set if any first frame callback is pending.
	bool using_westerossink; //true if westros sink is used as video sink
	guint busWatchId;
	std::atomic<bool> eosSignalled; /** Indicates if EOS has signaled */
	gboolean buffering_enabled; // enable buffering based on multiqueue
	gboolean buffering_in_progress; // buffering is in progress
	guint buffering_timeout_cnt;    // make sure buffering_timout doesn't get stuck
	GstState buffering_target_state; // the target state after buffering
#ifdef INTELCE
	bool keepLastFrame; //Keep last frame over next pipeline delete/ create cycle
#endif
	gint64 lastKnownPTS; //To store the PTS of last displayed video
	long long ptsUpdatedTimeMS; //Timestamp when PTS was last updated
	guint ptsCheckForEosOnUnderflowIdleTaskId; //ID of task to ensure video PTS is not moving before notifying EOS on underflow.
	int numberOfVideoBuffersSent; //Number of video buffers sent to pipeline
	gint64 segmentStart; // segment start value; required when qtdemux is enabled and restamping is disabled
	GstQuery *positionQuery; // pointer that holds a position query object
	bool paused; // if pipeline is deliberately put in PAUSED state due to user interaction
	GstState pipelineState; // current state of pipeline
};

/**
 * @class Id3CallbackData
 * @brief Holds id3 metadata callback specific variables.
 */
class Id3CallbackData
{
public:
	class AAMPGstPlayer* _this; // AAMPGstPlayer instance
	uint8_t* data; // Pointer to start of id3 metadata
	int32_t len; // Length of id3 metadata
};



static const char* GstPluginNamePR = "aampplayreadydecryptor";
static const char* GstPluginNameWV = "aampwidevinedecryptor";
static const char* GstPluginNameCK = "aampclearkeydecryptor";

/**
 * @brief Called from the mainloop when a message is available on the bus
 * @param[in] bus the GstBus that sent the message
 * @param[in] msg the GstMessage
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval FALSE if the event source should be removed.
 */
static gboolean bus_message(GstBus * bus, GstMessage * msg, AAMPGstPlayer * _this);

/**
 * @brief Invoked synchronously when a message is available on the bus
 * @param[in] bus the GstBus that sent the message
 * @param[in] msg the GstMessage
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval FALSE if the event source should be removed.
 */
static GstBusSyncReply bus_sync_handler(GstBus * bus, GstMessage * msg, AAMPGstPlayer * _this);

/**
 * @brief g_timeout callback to wait for buffering to change
 *        pipeline from paused->playing
 */
static gboolean buffering_timeout (gpointer data);

/** 
 * @brief check if elemement is instance (BCOM-3563)
 */
void type_check_instance(char * str, GstElement * elem);

/**
 * @brief AAMPGstPlayer Constructor
 * @param[in] aamp pointer to PrivateInstanceAAMP object associated with player
 */
AAMPGstPlayer::AAMPGstPlayer(PrivateInstanceAAMP *aamp
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
	, std::function< void(uint8_t *, int, int, int) > exportFrames
#endif
	) : aamp(NULL) , privateContext(NULL), mBufferingLock()
{
	privateContext = (AAMPGstPlayerPriv *)malloc(sizeof(*privateContext));
	memset(privateContext, 0, sizeof(*privateContext));
	privateContext->audioVolume = 1.0;
	privateContext->gstPropsDirty = true; //Have to set audioVolume on gst startup
	privateContext->pipelineState = GST_STATE_NULL;
	this->aamp = aamp;
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
	this->cbExportYUVFrame = exportFrames;
#endif

	pthread_mutex_init(&mBufferingLock, NULL);

	CreatePipeline();
	privateContext->rate = AAMP_NORMAL_PLAY_RATE;
	strcpy(privateContext->videoRectangle, DEFAULT_VIDEO_RECTANGLE);
}


/**
 * @brief AAMPGstPlayer Destructor
 */
AAMPGstPlayer::~AAMPGstPlayer()
{
	DestroyPipeline();
	free(privateContext);
	pthread_mutex_destroy(&mBufferingLock);
}

/**
 * @brief Analyze stream info from the GstPipeline
 * @param[in] _this pointer to AAMPGstPlayer instance
 */
static void analyze_streams(AAMPGstPlayer *_this)
{
#ifdef SUPPORT_MULTI_AUDIO
	GstElement *sinkbin = _this->privateContext->stream[eMEDIATYPE_VIDEO].sinkbin;

	g_object_get(sinkbin, "n-audio", &_this->privateContext->n_audio, NULL);
	g_print("audio:\n");
	for (gint i = 0; i < _this->privateContext->n_audio; i++)
	{
		GstTagList *tags = NULL;
		g_signal_emit_by_name(sinkbin, "get-audio-tags", i, &tags);
		if (tags)
		{
			gchar *str;
			guint rate;

			g_print("audio stream %d:\n", i);
			if (gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC, &str)) {
				g_print("  codec: %s\n", str);
				g_free(str);
			}
			if (gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &str)) {
				g_print("  language: %s\n", str);
				g_free(str);
			}
			if (gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &rate)) {
				g_print("  bitrate: %d\n", rate);
			}
			gst_tag_list_free(tags);
		}
	}
	g_object_get(sinkbin, "current-audio", &_this->privateContext->current_audio, NULL);
#endif
}


/**
 * @brief Callback for appsrc "need-data" signal
 * @param[in] source pointer to appsrc instance triggering "need-data" signal
 * @param[in] size size of data required
 * @param[in] _this pointer to AAMPGstPlayer instance associated with the playback
 */
static void need_data(GstElement *source, guint size, AAMPGstPlayer * _this)
{
	if (source == _this->privateContext->stream[eMEDIATYPE_SUBTITLE].source)
	{
		_this->aamp->ResumeTrackDownloads(eMEDIATYPE_SUBTITLE); // signal fragment downloader thread
	}
	else if (source == _this->privateContext->stream[eMEDIATYPE_AUDIO].source)
	{
		_this->aamp->ResumeTrackDownloads(eMEDIATYPE_AUDIO); // signal fragment downloader thread
	}
        else
	{
		_this->aamp->ResumeTrackDownloads(eMEDIATYPE_VIDEO); // signal fragment downloader thread
	}
}


/**
 * @brief Callback for appsrc "enough-data" signal
 * @param[in] source pointer to appsrc instance triggering "enough-data" signal
 * @param[in] _this pointer to AAMPGstPlayer instance associated with the playback
 */
static void enough_data(GstElement *source, AAMPGstPlayer * _this)
{
	if (source == _this->privateContext->stream[eMEDIATYPE_SUBTITLE].source)
	{
		_this->aamp->StopTrackDownloads(eMEDIATYPE_SUBTITLE); // signal fragment downloader thread
	}
	else if (source == _this->privateContext->stream[eMEDIATYPE_AUDIO].source)
	{
		_this->aamp->StopTrackDownloads(eMEDIATYPE_AUDIO); // signal fragment downloader thread
	}
        else
	{
		_this->aamp->StopTrackDownloads(eMEDIATYPE_VIDEO); // signal fragment downloader thread
	}
}


/**
 * @brief Callback for appsrc "seek-data" signal
 * @param[in] src pointer to appsrc instance triggering "seek-data" signal 
 * @param[in] offset seek position offset
 * @param[in] _this pointer to AAMPGstPlayer instance associated with the playback
 */
static gboolean  appsrc_seek  (GstAppSrc *src, guint64 offset, AAMPGstPlayer * _this)
{
#ifdef TRACE
	logprintf("appsrc %p seek-signal - offset %" G_GUINT64_FORMAT, src, offset);
#endif
	return TRUE;
}


/**
 * @brief Initialize properties/callback of appsrc
 * @param[in] _this pointer to AAMPGstPlayer instance associated with the playback
 * @param[in] source pointer to appsrc instance to be initialized
 * @param[in] mediaType stream type
 */
static void InitializeSource( AAMPGstPlayer *_this,GObject *source, MediaType mediaType = eMEDIATYPE_VIDEO )
{
	g_signal_connect(source, "need-data", G_CALLBACK(need_data), _this);
	g_signal_connect(source, "enough-data", G_CALLBACK(enough_data), _this);
	g_signal_connect(source, "seek-data", G_CALLBACK(appsrc_seek), _this);
	gst_app_src_set_stream_type(GST_APP_SRC(source), GST_APP_STREAM_TYPE_SEEKABLE);
	if (eMEDIATYPE_VIDEO == mediaType )
	{
#ifdef CONTENT_4K_SUPPORTED
		g_object_set(source, "max-bytes", 4194304 * 3, NULL); // 4096k * 3
#else
		g_object_set(source, "max-bytes", (guint64)4194304, NULL); // 4096k
#endif
	}
	else
	{
#ifdef CONTENT_4K_SUPPORTED
		g_object_set(source, "max-bytes", 512000 * 3, NULL); // 512k * 3 for audio
#else
		g_object_set(source, "max-bytes", (guint64)512000, NULL); // 512k for audio
#endif
	}
	g_object_set(source, "min-percent", 50, NULL);
	g_object_set(source, "format", GST_FORMAT_TIME, NULL);
}


/**
 * @brief Parse format to generate GstCaps
 * @param[in] format stream format to generate caps
 * @retval GstCaps for the input format
 */
static GstCaps* GetGstCaps(StreamOutputFormat format)
{
	GstCaps * caps = NULL;
	switch (format)
	{
		case FORMAT_MPEGTS:
			caps = gst_caps_new_simple ("video/mpegts",
					"systemstream", G_TYPE_BOOLEAN, TRUE,
					"packetsize", G_TYPE_INT, 188, NULL);
			break;
		case FORMAT_ISO_BMFF:
			caps = gst_caps_new_simple("video/quicktime", NULL, NULL);
			break;
		case FORMAT_AUDIO_ES_AAC:
			caps = gst_caps_new_simple ("audio/mpeg",
					"mpegversion", G_TYPE_INT, 2,
					"stream-format", G_TYPE_STRING, "adts", NULL);
			break;
		case FORMAT_AUDIO_ES_AC3:
			caps = gst_caps_new_simple ("audio/ac3", NULL, NULL);
			break;
		case FORMAT_AUDIO_ES_ATMOS:
			// Todo :: a) Test with all platforms if atmos works 
			//	   b) Test to see if x-eac3 config is enough for atmos stream.
			//	 	if x-eac3 is enough then both switch cases can be combined
			caps = gst_caps_new_simple ("audio/x-eac3", NULL, NULL);
                        break;
		case FORMAT_AUDIO_ES_EC3:
			caps = gst_caps_new_simple ("audio/x-eac3", NULL, NULL);
			break;
		case FORMAT_VIDEO_ES_H264:
#ifdef INTELCE
			caps = gst_caps_new_simple ("video/x-h264",
					"stream-format", G_TYPE_STRING, "avc",
					"width", G_TYPE_INT, 1920,
					"height", G_TYPE_INT, 1080,
					NULL);
#elif (defined(RPI) || defined(__APPLE__))
			caps = gst_caps_new_simple ("video/x-h264",
                                       "alignment", G_TYPE_STRING, "au",
                                       "stream-format", G_TYPE_STRING, "avc",
                                       NULL);
#else
			caps = gst_caps_new_simple ("video/x-h264", NULL, NULL);
#endif
			break;
		case FORMAT_VIDEO_ES_HEVC:
			caps = gst_caps_new_simple ("video/x-h265", NULL, NULL);
			break;
		case FORMAT_VIDEO_ES_MPEG2:
			caps = gst_caps_new_simple ("video/mpeg",
					"mpegversion", G_TYPE_INT, 2,
					"systemstream", G_TYPE_BOOLEAN, FALSE, NULL);
		case FORMAT_INVALID:
		case FORMAT_NONE:
		default:
			logprintf("Unsupported format %d", format);
			break;
	}
	return caps;
}


/**
 * @brief Callback when source is added by playbin
 * @param[in] object a GstObject
 * @param[in] orig the object that originated the signal
 * @param[in] pspec the property that changed
 * @param[in] _this pointer to AAMPGstPlayer instance associated with the playback
 */
static void found_source(GObject * object, GObject * orig, GParamSpec * pspec, AAMPGstPlayer * _this )
{
	logprintf("AAMPGstPlayer: found_source");
	MediaType mediaType;
	media_stream *stream;
	GstCaps * caps;
	if (object == G_OBJECT(_this->privateContext->stream[eMEDIATYPE_VIDEO].sinkbin))
	{
		logprintf("Found source for video");
		mediaType = eMEDIATYPE_VIDEO;
	}
	else if (object == G_OBJECT(_this->privateContext->stream[eMEDIATYPE_AUDIO].sinkbin))
	{
		logprintf("Found source for audio");
		mediaType = eMEDIATYPE_AUDIO;
	}
	else
	{
		logprintf("Found source for subtitle");
		mediaType = eMEDIATYPE_SUBTITLE;
	}
	stream = &_this->privateContext->stream[mediaType];
	g_object_get(orig, pspec->name, &stream->source, NULL);
	InitializeSource(_this, G_OBJECT(stream->source), mediaType);
	caps = GetGstCaps(stream->format);
	gst_app_src_set_caps(GST_APP_SRC(stream->source), caps);
	gst_caps_unref(caps);
}

static void httpsoup_source_setup (GstElement * element, GstElement * source, gpointer data)
{
	AAMPGstPlayer * _this = (AAMPGstPlayer *)data;

	if (!strcmp(GST_ELEMENT_NAME(source), "source"))
	{
		const char *proxy = _this->aamp->GetNetworkProxy();
		if(proxy)
		{
			g_object_set(source, "proxy", proxy, NULL);
			logprintf("%s() : httpsoup -> Set network proxy '%s'", __FUNCTION__, proxy);
		}
	}
}


/**
 * @brief Idle callback to notify first frame rendered event
 * @param[in] user_data pointer to AAMPGstPlayer instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
 */
static gboolean IdleCallbackOnFirstFrame(gpointer user_data)
{
        AAMPGstPlayer *_this = (AAMPGstPlayer *)user_data;
		if (_this){
			_this->aamp->NotifyFirstFrameReceived();
			_this->privateContext->firstFrameCallbackIdleTaskPending = false;
			_this->privateContext->firstFrameCallbackIdleTaskId = 0;
		}
        return G_SOURCE_REMOVE;
}


/**
 * @brief Idle callback to notify end-of-stream event
 * @param[in] user_data pointer to AAMPGstPlayer instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
 */
static gboolean IdleCallbackOnEOS(gpointer user_data)
{
	AAMPGstPlayer *_this = (AAMPGstPlayer *)user_data;
	if (_this){
		_this->privateContext->eosCallbackIdleTaskPending = false;
		logprintf("%s:%d  eosCallbackIdleTaskId %d", __FUNCTION__, __LINE__, _this->privateContext->eosCallbackIdleTaskId);
		_this->aamp->NotifyEOSReached();
		_this->privateContext->eosCallbackIdleTaskId = 0;
	}
	return G_SOURCE_REMOVE;
}

/**
 * @brief Idle callback to notify ID3 metadata event
 * @param[in] user_data pointer to Id3CallbackData object containing AAMPGstPlayer instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
 */
static gboolean IdleCallbackOnId3Metadata(gpointer user_data)
{
	Id3CallbackData *id3 = (Id3CallbackData*)user_data;

	id3->_this->aamp->SendId3MetadataEvent(id3->data, id3->len);
	id3->_this->privateContext->id3MetadataCallbackTaskPending = false;
	id3->_this->privateContext->id3MetadataCallbackIdleTaskId = 0;

	delete user_data;

	return G_SOURCE_REMOVE;
}


/**
 * @brief Timer's callback to notify playback progress event
 * @param[in] user_data pointer to AAMPGstPlayer instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
 */
static gboolean ProgressCallbackOnTimeout(gpointer user_data)
{
	AAMPGstPlayer *_this = (AAMPGstPlayer *)user_data;
	if (_this){
		_this->aamp->ReportProgress();
		traceprintf("%s:%d current %d, stored %d ", __FUNCTION__, __LINE__, g_source_get_id(g_main_current_source()), _this->privateContext->periodicProgressCallbackIdleTaskId);
	}
	return G_SOURCE_CONTINUE;
}


/**
 * @brief Idle callback to start progress notifier timer
 * @param[in] user_data pointer to AAMPGstPlayer instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
 */
static gboolean IdleCallback(gpointer user_data)
{
	AAMPGstPlayer *_this = (AAMPGstPlayer *)user_data;
	if (_this){
		_this->aamp->ReportProgress();
		_this->privateContext->firstProgressCallbackIdleTaskPending = false;
		_this->privateContext->firstProgressCallbackIdleTaskId = 0;
		if (0 == _this->privateContext->periodicProgressCallbackIdleTaskId)
		{
			_this->privateContext->periodicProgressCallbackIdleTaskId = g_timeout_add(_this->aamp->mReportProgressInterval, ProgressCallbackOnTimeout, user_data);
			AAMPLOG_WARN("%s:%d current %d, periodicProgressCallbackIdleTaskId %d", __FUNCTION__, __LINE__, g_source_get_id(g_main_current_source()), _this->privateContext->periodicProgressCallbackIdleTaskId);
		}
		else
		{
			AAMPLOG_INFO("%s:%d Progress callback already available: periodicProgressCallbackIdleTaskId %d", __FUNCTION__, __LINE__, _this->privateContext->periodicProgressCallbackIdleTaskId);
		}
	}
	return G_SOURCE_REMOVE;
}

/**
 * @brief Notify first Audio and Video frame through an idle function to make the playersinkbin halding same as normal(playbin) playback.
 * @param[in] type media type of the frame which is decoded, either audio or video.
 */
void AAMPGstPlayer::NotifyFirstFrame(MediaType type)
{
	if(!privateContext->firstFrameReceived)
	{
		privateContext->firstFrameReceived = true;
		aamp->LogFirstFrame();
		aamp->LogTuneComplete();
		aamp->NotifyFirstBufferProcessed();
	}

	if (eMEDIATYPE_VIDEO == type)
	{
		if (!privateContext->decoderHandleNotified)
		{
			privateContext->decoderHandleNotified = true;
			privateContext->firstFrameCallbackIdleTaskPending = true;
			privateContext->firstFrameCallbackIdleTaskId = g_idle_add(IdleCallbackOnFirstFrame, this);
			if (!privateContext->firstFrameCallbackIdleTaskPending)
			{
				logprintf("%s:%d firstFrameCallbackIdleTask already finished, reset id", __FUNCTION__, __LINE__);
				privateContext->firstFrameCallbackIdleTaskId = 0;
			}
		}
		if (privateContext->firstProgressCallbackIdleTaskId == 0)
		{
			privateContext->firstProgressCallbackIdleTaskPending = true;
			privateContext->firstProgressCallbackIdleTaskId = g_idle_add(IdleCallback, this);
			if (!privateContext->firstProgressCallbackIdleTaskPending)
			{
				logprintf("%s:%d firstProgressCallbackIdleTask already finished, reset id", __FUNCTION__, __LINE__);
				privateContext->firstProgressCallbackIdleTaskId = 0;
			}
		}
	}
}

/**
 * @brief Callback invoked after first video frame decoded
 * @param[in] object pointer to element raising the callback
 * @param[in] arg0 number of arguments
 * @param[in] arg1 array of arguments
 * @param[in] _this pointer to AAMPGstPlayer instance
 */
static void AAMPGstPlayer_OnFirstVideoFrameCallback(GstElement* object, guint arg0, gpointer arg1,
	AAMPGstPlayer * _this)

{
	logprintf("AAMPGstPlayer_OnFirstVideoFrameCallback. got First Video Frame");
	_this->NotifyFirstFrame(eMEDIATYPE_VIDEO);

}

/**
 * @brief Callback invoked after first audio buffer decoded
 * @param[in] object pointer to element raising the callback
 * @param[in] arg0 number of arguments
 * @param[in] arg1 array of arguments
 * @param[in] _this pointer to AAMPGstPlayer instance
 */
static void AAMPGstPlayer_OnAudioFirstFrameBrcmAudDecoder(GstElement* object, guint arg0, gpointer arg1,
        AAMPGstPlayer * _this)
{
	logprintf("AAMPGstPlayer_OnAudioFirstFrameBrcmAudDecoder. got First Audio Frame");
	_this->NotifyFirstFrame(eMEDIATYPE_AUDIO);
}

/**
 * @brief Check if gstreamer element is video decoder
 * @param[in] name Name of the element
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval TRUE if element name is that of the decoder
 */
bool AAMPGstPlayer_isVideoDecoder(const char* name, AAMPGstPlayer * _this)
{
	return	(!_this->privateContext->using_westerossink && aamp_StartsWith(name, "brcmvideodecoder") == true) ||
			( _this->privateContext->using_westerossink && aamp_StartsWith(name, "westerossink") == true);
}

/**
 * @brief Check if gstreamer element is video sink
 * @param[in] name Name of the element
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval TRUE if element name is that of video sink
 */
bool AAMPGstPlayer_isVideoSink(const char* name, AAMPGstPlayer * _this)
{
	return	(!_this->privateContext->using_westerossink && aamp_StartsWith(name, "brcmvideosink") == true) || // brcmvideosink0, brcmvideosink1, ...
			( _this->privateContext->using_westerossink && aamp_StartsWith(name, "westerossink") == true);
}

/**
 * @brief Check if gstreamer element is audio decoder
 * @param[in] name Name of the element
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval TRUE if element name is that of audio decoder
 */
bool AAMPGstPlayer_isVideoOrAudioDecoder(const char* name, AAMPGstPlayer * _this)
{
	return	(!_this->privateContext->using_westerossink && !_this->privateContext->stream[eMEDIATYPE_VIDEO].using_playersinkbin &&
			(aamp_StartsWith(name, "brcmvideodecoder") == true || aamp_StartsWith(name, "brcmaudiodecoder") == true)) ||
			(_this->privateContext->using_westerossink && aamp_StartsWith(name, "westerossink") == true);
}

/**
 * @brief Notifies EOS if video decoder pts is stalled
 * @param[in] user_data pointer to AAMPGstPlayer instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
 */
static gboolean VideoDecoderPtsCheckerForEOS(gpointer user_data)
{
	AAMPGstPlayer *_this = (AAMPGstPlayer *) user_data;
	AAMPGstPlayerPriv *privateContext = _this->privateContext;
#ifndef INTELCE
	gint64 currentPTS = 0;
	if (privateContext->video_dec)
	{
		g_object_get(privateContext->video_dec, "video-pts", &currentPTS, NULL);
	}

	if (currentPTS == privateContext->lastKnownPTS)
	{
		logprintf("%s:%d : PTS not changed", __FUNCTION__, __LINE__);
		_this->NotifyEOS();
	}
	else
	{
		logprintf("%s:%d : Video PTS still moving lastKnownPTS %" G_GUINT64_FORMAT " currentPTS %" G_GUINT64_FORMAT " ##", __FUNCTION__, __LINE__, privateContext->lastKnownPTS, currentPTS);
	}
#endif
	privateContext->ptsCheckForEosOnUnderflowIdleTaskId = 0;
	return G_SOURCE_REMOVE;
}

#ifdef RENDER_FRAMES_IN_APP_CONTEXT

/**
 * @brief Callback function to get video frames
 * @param[in] object - pointer to appsink instance triggering "new-sample" signal
 * @param[in] _this  - pointer to AAMPGstPlayer instance
 * @retval GST_FLOW_OK
 */
GstFlowReturn AAMPGstPlayer::AAMPGstPlayer_OnVideoSample(GstElement* object, AAMPGstPlayer * _this)
{
	GstSample *sample;
	GstBuffer *buffer;
	GstMapInfo map;

	if(_this && _this->cbExportYUVFrame)
	{
		sample = gst_app_sink_pull_sample (GST_APP_SINK (object));
		if (sample)
		{
			int width, height;
			GstCaps *caps = gst_sample_get_caps(sample);
			GstStructure *capsStruct = gst_caps_get_structure(caps,0);
			gst_structure_get_int(capsStruct,"width",&width);
			gst_structure_get_int(capsStruct,"height",&height);
			//logprintf("StrCAPS=%s", gst_caps_to_string(caps));
			buffer = gst_sample_get_buffer (sample);
			if (buffer)
			{
				if (gst_buffer_map(buffer, &map, GST_MAP_READ))
				{
					_this->cbExportYUVFrame(map.data, map.size, width, height);

					gst_buffer_unmap(buffer, &map);
				}
				else
				{
					logprintf("%s:%d buffer map failed", __FUNCTION__, __LINE__);
				}
			}
			else
			{
				logprintf("%s:%d buffer NULL", __FUNCTION__, __LINE__);
			}
			gst_sample_unref (sample);
		}
		else
		{
			logprintf("%s:%d sample NULL", __FUNCTION__, __LINE__);
		}
	}
	return GST_FLOW_OK;
}
#endif

/**
 * @brief Callback invoked when facing an underflow
 * @param[in] object pointer to element raising the callback
 * @param[in] arg0 number of arguments
 * @param[in] arg1 array of arguments
 * @param[in] _this pointer to AAMPGstPlayer instance
 */
static void AAMPGstPlayer_OnGstBufferUnderflowCb(GstElement* object, guint arg0, gpointer arg1,
        AAMPGstPlayer * _this)
{
	//TODO - Handle underflow
	MediaType type;
	AAMPGstPlayerPriv *privateContext = _this->privateContext;
	logprintf("## %s() : Got Underflow message from %s ##", __FUNCTION__, GST_ELEMENT_NAME(object));
	if (AAMPGstPlayer_isVideoDecoder(GST_ELEMENT_NAME(object), _this))
	{
		type = eMEDIATYPE_VIDEO;
	}
	else if (aamp_StartsWith(GST_ELEMENT_NAME(object), "brcmaudiodecoder") == true)
	{
		type = eMEDIATYPE_AUDIO;
	}
	_this->privateContext->stream[type].bufferUnderrun = true;
	if (_this->privateContext->stream[type].eosReached)
	{
		if (_this->privateContext->rate > 0)
		{
			if (privateContext->video_dec)
			{
				if (!privateContext->ptsCheckForEosOnUnderflowIdleTaskId)
				{
					g_object_get(privateContext->video_dec, "video-pts", &privateContext->lastKnownPTS, NULL);
					privateContext->ptsUpdatedTimeMS = NOW_STEADY_TS_MS;
					privateContext->ptsCheckForEosOnUnderflowIdleTaskId = g_timeout_add(AAMP_DELAY_BETWEEN_PTS_CHECK_FOR_EOS_ON_UNDERFLOW, VideoDecoderPtsCheckerForEOS, _this);
				}
				else
				{
					logprintf("%s:%d : ptsCheckForEosOnUnderflowIdleTask ID %d already running, ignore underflow", __FUNCTION__, __LINE__, (int)privateContext->ptsCheckForEosOnUnderflowIdleTaskId);
				}
			}
			else
			{
				logprintf("%s:%d : video_dec not available", __FUNCTION__, __LINE__);
				_this->NotifyEOS();
			}
		}
		else
		{
			_this->aamp->ScheduleRetune(eGST_ERROR_UNDERFLOW, type);
		}
	}
	else
	{
		_this->aamp->ScheduleRetune(eGST_ERROR_UNDERFLOW, type);
	}
}

/**
 * @brief Callback invoked a PTS error is encountered
 * @param[in] object pointer to element raising the callback
 * @param[in] arg0 number of arguments
 * @param[in] arg1 array of arguments
 * @param[in] _this pointer to AAMPGstPlayer instance
 */
static void AAMPGstPlayer_OnGstPtsErrorCb(GstElement* object, guint arg0, gpointer arg1,
        AAMPGstPlayer * _this)
{
	logprintf("## %s() : Got PTS error message from %s ##", __FUNCTION__, GST_ELEMENT_NAME(object));
	if (AAMPGstPlayer_isVideoDecoder(GST_ELEMENT_NAME(object), _this))
	{
		_this->aamp->ScheduleRetune(eGST_ERROR_PTS, eMEDIATYPE_VIDEO);
	}
	else if (aamp_StartsWith(GST_ELEMENT_NAME(object), "brcmaudiodecoder") == true)
	{
		_this->aamp->ScheduleRetune(eGST_ERROR_PTS, eMEDIATYPE_AUDIO);
	}
}

static gboolean buffering_timeout (gpointer data)
{
	AAMPGstPlayer * _this = (AAMPGstPlayer *) data;
	if (_this && _this->privateContext)
	{
		AAMPGstPlayerPriv * privateContext = _this->privateContext;
		if (_this->privateContext->buffering_in_progress)
		{
			guint bytes = 0, frames = DEFAULT_BUFFERING_QUEUED_FRAMES_MIN+1; // if queue_depth property, or video_dec, doesn't exist move to next state.
			if (_this->privateContext->video_dec)
			{
				g_object_get(_this->privateContext->video_dec,"buffered_bytes",&bytes,NULL);
				g_object_get(_this->privateContext->video_dec,"queued_frames",&frames,NULL);
			}
			/* DELIA-34654: Disable re-tune on buffering timeout for DASH as unlike HLS,
			DRM key acquisition can end after injection, and buffering is not expected
			to be completed by the 1 second timeout
			*/
			if (G_UNLIKELY(( _this->aamp->getStreamType() < 20) && (privateContext->buffering_timeout_cnt == 0 ) && gpGlobalConfig->reTuneOnBufferingTimeout && (privateContext->numberOfVideoBuffersSent > 0)))
			{
				logprintf("%s:%d Schedule retune. numberOfVideoBuffersSent %d  bytes %u  frames %u", __FUNCTION__, __LINE__, privateContext->numberOfVideoBuffersSent, bytes, frames);
				privateContext->buffering_in_progress = false;
				_this->DumpDiagnostics();
				_this->aamp->ScheduleRetune(eGST_ERROR_VIDEO_BUFFERING, eMEDIATYPE_VIDEO);
			}
			else if (bytes > DEFAULT_BUFFERING_QUEUED_BYTES_MIN || frames > DEFAULT_BUFFERING_QUEUED_FRAMES_MIN || privateContext->buffering_timeout_cnt-- == 0)
			{
				logprintf("%s: Set pipeline state to %s - buffering_timeout_cnt %u  bytes %u  frames %u", __FUNCTION__, gst_element_state_get_name(_this->privateContext->buffering_target_state), (_this->privateContext->buffering_timeout_cnt+1), bytes, frames);
				gst_element_set_state (_this->privateContext->pipeline, _this->privateContext->buffering_target_state);
				_this->privateContext->buffering_in_progress = false;
			}
		}
		if (!_this->privateContext->buffering_in_progress)
		{
			//reset timer id after buffering operation is completed
			_this->privateContext->bufferingTimeoutTimerId = 0;
		}
		return _this->privateContext->buffering_in_progress;
		
	}
	else
	{
		logprintf("%s:%d in buffering_timeout got invalid or NULL handle ! _this =  %p   _this->privateContext = %p ", __FUNCTION__, __LINE__,
		_this, (_this? _this->privateContext: NULL) );
		_this->privateContext->bufferingTimeoutTimerId = 0;
		return false;
	}
	
}

/**
 * @brief Called from the mainloop when a message is available on the bus
 * @param[in] bus the GstBus that sent the message
 * @param[in] msg the GstMessage
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval FALSE if the event source should be removed.
 */
static gboolean bus_message(GstBus * bus, GstMessage * msg, AAMPGstPlayer * _this)
{
	GError *error;
	gchar *dbg_info;
	bool isPlaybinStateChangeEvent;

	switch (GST_MESSAGE_TYPE(msg))
	{ // see https://developer.gnome.org/gstreamer/stable/gstreamer-GstMessage.html#GstMessage
	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &error, &dbg_info);
		g_printerr("GST_MESSAGE_ERROR %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
		char errorDesc[MAX_ERROR_DESCRIPTION_LENGTH];
		memset(errorDesc, '\0', MAX_ERROR_DESCRIPTION_LENGTH);
		strncpy(errorDesc, "GstPipeline Error:", 18);
		strncat(errorDesc, error->message, MAX_ERROR_DESCRIPTION_LENGTH - 18 - 1);
		if (strstr(error->message, "video decode error") != NULL)
		{
			_this->aamp->SendErrorEvent(AAMP_TUNE_GST_PIPELINE_ERROR, errorDesc, false);
		}
		else if(strstr(error->message, "HDCP Compliance Check Failure") != NULL)
		{
			// Trying to play a 4K content on a non-4K TV .Report error to XRE with no retune
			_this->aamp->SendErrorEvent(AAMP_TUNE_HDCP_COMPLIANCE_ERROR, errorDesc, false);
		}
		else
		{
			_this->aamp->SendErrorEvent(AAMP_TUNE_GST_PIPELINE_ERROR, errorDesc);
		}
		g_printerr("Debug Info: %s\n", (dbg_info) ? dbg_info : "none");
		g_clear_error(&error);
		g_free(dbg_info);
		break;

	case GST_MESSAGE_WARNING:
		gst_message_parse_warning(msg, &error, &dbg_info);
		g_printerr("GST_MESSAGE_WARNING %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
		if (gpGlobalConfig->decoderUnavailableStrict && strstr(error->message, "No decoder available") != NULL)
		{
			char warnDesc[MAX_ERROR_DESCRIPTION_LENGTH];
			snprintf( warnDesc, MAX_ERROR_DESCRIPTION_LENGTH, "GstPipeline Error:%s", error->message );
			// decoding failures due to unsupported codecs are received as warnings, i.e.
			// "No decoder available for type 'video/x-gst-fourcc-av01"
			_this->aamp->SendErrorEvent(AAMP_TUNE_GST_PIPELINE_ERROR, warnDesc, false);
		}
		g_printerr("Debug Info: %s\n", (dbg_info) ? dbg_info : "none");
		g_clear_error(&error);
		g_free(dbg_info);
		break;
		
	case GST_MESSAGE_EOS:
		/**
		 * pipeline event: end-of-stream reached
		 * application may perform flushing seek to resume playback
		 */
		logprintf("GST_MESSAGE_EOS");
		_this->NotifyEOS();
		break;

	case GST_MESSAGE_STATE_CHANGED:
		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);

		isPlaybinStateChangeEvent = (GST_MESSAGE_SRC(msg) == GST_OBJECT(_this->privateContext->pipeline));

		if (gpGlobalConfig->logging.gst || isPlaybinStateChangeEvent)
		{
			logprintf("%s %s -> %s (pending %s)",
				GST_OBJECT_NAME(msg->src),
				gst_element_state_get_name(old_state),
				gst_element_state_get_name(new_state),
				gst_element_state_get_name(pending_state));

			if (isPlaybinStateChangeEvent && new_state == GST_STATE_PLAYING)
			{
#if defined(INTELCE) || (defined(__APPLE__))
				if(!_this->privateContext->firstFrameReceived)
				{
					_this->privateContext->firstFrameReceived = true;
					_this->aamp->LogFirstFrame();
					_this->aamp->LogTuneComplete();
				}
				_this->aamp->NotifyFirstFrameReceived();
#endif

#if defined(INTELCE) || defined(__APPLE__)
				//Note: Progress event should be sent after the decoderAvailable event only.
				//BRCM platform sends progress event after AAMPGstPlayer_OnFirstVideoFrameCallback.
				if (_this->privateContext->firstProgressCallbackIdleTaskId == 0)
				{
					_this->privateContext->firstProgressCallbackIdleTaskId = g_idle_add(IdleCallback, _this);
				}
#endif
				analyze_streams(_this);

				if (gpGlobalConfig->logging.gst )
				{
					GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)_this->privateContext->pipeline, GST_DEBUG_GRAPH_SHOW_ALL, "myplayer");
					// output graph to .dot format which can be visualized with Graphviz tool if:
					// gstreamer is configured with --gst-enable-gst-debug
					// and "gst" is enabled in aamp.cfg
					// and environment variable GST_DEBUG_DUMP_DOT_DIR is set to a basepath(e.g. /opt).
				}
			}
		}
		if ((!_this->privateContext->stream[eMEDIATYPE_VIDEO].using_playersinkbin) && (_this->privateContext->gstPropsDirty))
		{
#ifndef INTELCE
			if (new_state == GST_STATE_PAUSED && old_state == GST_STATE_READY)
			{
				if (AAMPGstPlayer_isVideoSink(GST_OBJECT_NAME(msg->src), _this))
				{ // video scaling patch
					/*
					brcmvideosink doesn't sets the rectangle property correct by default
					gst-inspect-1.0 brcmvideosink
					g_object_get(_this->privateContext->pipeline, "video-sink", &videoSink, NULL); - reports NULL
					note: alternate "window-set" works as well
					*/
					_this->privateContext->video_sink = (GstElement *) msg->src;
					if (_this->privateContext->using_westerossink && !_this->aamp->mEnableRectPropertyEnabled)
					{
						logprintf("AAMPGstPlayer - using westerossink, setting cached video mute and zoom");
						g_object_set(msg->src, "zoom-mode", VIDEO_ZOOM_FULL == _this->privateContext->zoom ? 0 : 1, NULL);
						g_object_set(msg->src, "show-video-window", !_this->privateContext->videoMuted, NULL);
					}
					else
					{
						logprintf("AAMPGstPlayer setting cached rectangle, video mute and zoom");
						g_object_set(msg->src, "rectangle", _this->privateContext->videoRectangle, NULL);
						g_object_set(msg->src, "zoom-mode", VIDEO_ZOOM_FULL == _this->privateContext->zoom ? 0 : 1, NULL);
						g_object_set(msg->src, "show-video-window", !_this->privateContext->videoMuted, NULL);
					}
				}
				else if (aamp_StartsWith(GST_OBJECT_NAME(msg->src), "brcmaudiosink") == true)
				{
					_this->privateContext->audio_sink = (GstElement *) msg->src;

					_this->setVolumeOrMuteUnMute();
				}
				else if (strstr(GST_OBJECT_NAME(msg->src), "brcmaudiodecoder"))
				{
					GstElement * audio_dec = (GstElement *) msg->src;

					// this reduces amount of data in the fifo, which is flushed/lost when transition from expert to normal modes
					g_object_set(msg->src, "limit_buffering_ms", 1500, NULL);   /* default 500ms was a bit low.. try 1500ms */
					g_object_set(msg->src, "limit_buffering", 1, NULL);
					logprintf("Found brcmaudiodecoder, limiting audio decoder buffering");
					g_object_set(msg->src, "stream_sync_mode", 0, NULL); /* tell decoder not to look for 2nd/next frame sync, decode if it finds a single frame sync */
				}

				StreamOutputFormat audFormat = _this->privateContext->stream[eMEDIATYPE_AUDIO].format;

				if ((audFormat == FORMAT_NONE || _this->privateContext->audio_sink != NULL) &&
					(_this->privateContext->video_sink != NULL))
				{
					_this->privateContext->gstPropsDirty = false;
				}
			}
#endif
		}
		if ((NULL != msg->src) && AAMPGstPlayer_isVideoOrAudioDecoder(GST_OBJECT_NAME(msg->src), _this))
		{
#ifdef AAMP_MPD_DRM
			// This is the video decoder, send this to the output protection module
			// so it can get the source width/height
			if (AAMPGstPlayer_isVideoDecoder(GST_OBJECT_NAME(msg->src), _this))
			{
				if(AampOutputProtection::IsAampOutputProcectionInstanceActive())
				{
					AampOutputProtection *pInstance = AampOutputProtection::GetAampOutputProcectionInstance();
					pInstance->setGstElement((GstElement *)(msg->src));
					pInstance->Release();
				}
			}
#endif
			if (old_state == GST_STATE_NULL && new_state == GST_STATE_READY)
			{
				g_signal_connect(msg->src, "buffer-underflow-callback",
					G_CALLBACK(AAMPGstPlayer_OnGstBufferUnderflowCb), _this);
				g_signal_connect(msg->src, "pts-error-callback",
					G_CALLBACK(AAMPGstPlayer_OnGstPtsErrorCb), _this);
			}
		}
		break;

	case GST_MESSAGE_ASYNC_DONE:
		{
			if (_this->privateContext->buffering_in_progress)
			{
				if (buffering_timeout(_this)) { // call immediately and if already buffered enough don't start timer.
				    if (0 == _this->privateContext->bufferingTimeoutTimerId)
						_this->privateContext->bufferingTimeoutTimerId = g_timeout_add((guint)DEFAULT_BUFFERING_TO_MS, buffering_timeout, _this);
				}
			}
		}
		break;

	case GST_MESSAGE_TAG:
		break;

	case GST_MESSAGE_QOS:
	{
		gboolean live;
		guint64 running_time;
		guint64 stream_time;
		guint64 timestamp;
		guint64 duration;
		gst_message_parse_qos(msg, &live, &running_time, &stream_time, &timestamp, &duration);
		break;
	}

	case GST_MESSAGE_CLOCK_LOST:
		logprintf("GST_MESSAGE_CLOCK_LOST");
		// get new clock - needed?
		gst_element_set_state(_this->privateContext->pipeline, GST_STATE_PAUSED);
		gst_element_set_state(_this->privateContext->pipeline, GST_STATE_PLAYING);
		break;

#ifdef TRACE
	case GST_MESSAGE_RESET_TIME:
		GstClockTime running_time;
		gst_message_parse_reset_time (msg, &running_time);
		printf("GST_MESSAGE_RESET_TIME %llu", (unsigned long long)running_time);
		break;
#endif

#ifdef USE_GST1
	case GST_MESSAGE_NEED_CONTEXT:

		/*
		 * Code to avoid logs flooding with NEED-CONTEXT message for DRM systems
		 */
		/*
		const gchar* contextType;
		gst_message_parse_context_type(msg, &contextType);
		if (!g_strcmp0(contextType, "drm-preferred-decryption-system-id"))
		{
			logprintf("Setting Playready context");
			GstContext* context = gst_context_new("drm-preferred-decryption-system-id", FALSE);
			GstStructure* contextStructure = gst_context_writable_structure(context);
			gst_structure_set(contextStructure, "decryption-system-id", G_TYPE_STRING, "9a04f079-9840-4286-ab92-e65be0885f95", NULL);
			gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(msg)), context);
		}
		*/
		break;
#endif

	case GST_MESSAGE_STREAM_STATUS:
	case GST_MESSAGE_ELEMENT: // can be used to collect pts, dts, pid
	case GST_MESSAGE_DURATION:
	case GST_MESSAGE_LATENCY:
	case GST_MESSAGE_NEW_CLOCK:
		break;
	case GST_MESSAGE_APPLICATION:
		const GstStructure *msgS;
		msgS = gst_message_get_structure (msg);
		if (gst_structure_has_name (msgS, "HDCPProtectionFailure")) {
			logprintf("Received HDCPProtectionFailure event.Schedule Retune ");
			_this->Flush(0, AAMP_NORMAL_PLAY_RATE, true);
			_this->aamp->ScheduleRetune(eGST_ERROR_OUTPUT_PROTECTION_ERROR,eMEDIATYPE_VIDEO);
		}
		break;
	default:
		logprintf("msg type: %s", gst_message_type_get_name(msg->type));
		break;
	}
	return TRUE;
}


/**
 * @brief Invoked synchronously when a message is available on the bus
 * @param[in] bus the GstBus that sent the message
 * @param[in] msg the GstMessage
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @retval FALSE if the event source should be removed.
 */
static GstBusSyncReply bus_sync_handler(GstBus * bus, GstMessage * msg, AAMPGstPlayer * _this)
{
	switch(GST_MESSAGE_TYPE(msg))
	{
	case GST_MESSAGE_STATE_CHANGED:
		GstState old_state, new_state;
		gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);

		if (GST_MESSAGE_SRC(msg) == GST_OBJECT(_this->privateContext->pipeline))
		{
			_this->privateContext->pipelineState = new_state;
		}

		if (old_state == GST_STATE_NULL && new_state == GST_STATE_READY)
		{
#ifndef INTELCE
			if ((NULL != msg->src) && AAMPGstPlayer_isVideoOrAudioDecoder(GST_OBJECT_NAME(msg->src), _this))
			{
				if (AAMPGstPlayer_isVideoDecoder(GST_OBJECT_NAME(msg->src), _this))
				{
					_this->privateContext->video_dec = (GstElement *) msg->src;
					type_check_instance("bus_sync_handle: video_dec ", _this->privateContext->video_dec);
					g_signal_connect(_this->privateContext->video_dec, "first-video-frame-callback",
									G_CALLBACK(AAMPGstPlayer_OnFirstVideoFrameCallback), _this);
				}
				else
				{
					_this->privateContext->audio_dec = (GstElement *) msg->src;
					type_check_instance("bus_sync_handle: audio_dec ", _this->privateContext->audio_dec);
					g_signal_connect(msg->src, "first-audio-frame-callback",
									G_CALLBACK(AAMPGstPlayer_OnAudioFirstFrameBrcmAudDecoder), _this);
				}
			}

#else
			if (aamp_StartsWith(GST_OBJECT_NAME(msg->src), "ismdgstaudiosink") == true)
			{
				_this->privateContext->audio_sink = (GstElement *) msg->src;

				logprintf("AAMPGstPlayer setting audio-sync");
				g_object_set(msg->src, "sync", TRUE, NULL);

				_this->setVolumeOrMuteUnMute();
			}
			else
			{
#ifndef INTELCE_USE_VIDRENDSINK
				if (aamp_StartsWith(GST_OBJECT_NAME(msg->src), "ismdgstvidsink") == true)
#else
				if (aamp_StartsWith(GST_OBJECT_NAME(msg->src), "ismdgstvidrendsink") == true)
#endif
				{
					AAMPGstPlayerPriv *privateContext = _this->privateContext;
					privateContext->video_sink = (GstElement *) msg->src;
					logprintf("AAMPGstPlayer setting stop-keep-frame %d", (int)(privateContext->keepLastFrame));
					g_object_set(msg->src, "stop-keep-frame", privateContext->keepLastFrame, NULL);
#if defined(INTELCE) && !defined(INTELCE_USE_VIDRENDSINK)
					logprintf("AAMPGstPlayer setting rectangle %s", privateContext->videoRectangle);
					g_object_set(msg->src, "rectangle", privateContext->videoRectangle, NULL);
					logprintf("AAMPGstPlayer setting zoom %s", (VIDEO_ZOOM_FULL == privateContext->zoom) ? "FULL" : "NONE");
					g_object_set(msg->src, "scale-mode", (VIDEO_ZOOM_FULL == privateContext->zoom) ? 0 : 3, NULL);
					logprintf("AAMPGstPlayer setting crop-lines to FALSE");
					g_object_set(msg->src, "crop-lines", FALSE, NULL);
#endif
					logprintf("AAMPGstPlayer setting video mute %d", privateContext->videoMuted);
					g_object_set(msg->src, "mute", privateContext->videoMuted, NULL);
				}
				else if (aamp_StartsWith(GST_OBJECT_NAME(msg->src), "ismdgsth264viddec") == true)
				{
					_this->privateContext->video_dec = (GstElement *) msg->src;
				}
#ifdef INTELCE_USE_VIDRENDSINK
				else if (aamp_StartsWith(GST_OBJECT_NAME(msg->src), "ismdgstvidpproc") == true)
				{
					_this->privateContext->video_pproc = (GstElement *) msg->src;
					logprintf("AAMPGstPlayer setting rectangle %s", _this->privateContext->videoRectangle);
					g_object_set(msg->src, "rectangle", _this->privateContext->videoRectangle, NULL);
					logprintf("AAMPGstPlayer setting zoom %d", _this->privateContext->zoom);
					g_object_set(msg->src, "scale-mode", (VIDEO_ZOOM_FULL == _this->privateContext->zoom) ? 0 : 3, NULL);
				}
#endif
			}
#endif
			/*This block is added to share the PrivateInstanceAAMP object
			  with PlayReadyDecryptor Plugin, for tune time profiling

			  AAMP is added as a property of playready plugin
			*/
			if(aamp_StartsWith(GST_OBJECT_NAME(msg->src), GstPluginNamePR) == true ||
			   aamp_StartsWith(GST_OBJECT_NAME(msg->src), GstPluginNameWV) == true || 
			   aamp_StartsWith(GST_OBJECT_NAME(msg->src), GstPluginNameCK) == true) 
			{
				logprintf("AAMPGstPlayer setting aamp instance for %s decryptor", GST_OBJECT_NAME(msg->src));
				GValue val = { 0, };
				g_value_init(&val, G_TYPE_POINTER);
				g_value_set_pointer(&val, (gpointer) _this->aamp);
				g_object_set_property(G_OBJECT(msg->src), "aamp",&val);
			}
		}
		break;
#ifdef USE_GST1
	case GST_MESSAGE_NEED_CONTEXT:
		
		/*
		 * Code to avoid logs flooding with NEED-CONTEXT message for DRM systems
		 */
		const gchar* contextType;
		gst_message_parse_context_type(msg, &contextType);
		if (!g_strcmp0(contextType, "drm-preferred-decryption-system-id"))
		{
			logprintf("Setting %s as preferred drm",GetDrmSystemName((DRMSystems)gpGlobalConfig->preferredDrm));
			GstContext* context = gst_context_new("drm-preferred-decryption-system-id", FALSE);
			GstStructure* contextStructure = gst_context_writable_structure(context);
			gst_structure_set(contextStructure, "decryption-system-id", G_TYPE_STRING, GetDrmSystemID((DRMSystems)gpGlobalConfig->preferredDrm),  NULL);
			gst_element_set_context(GST_ELEMENT(GST_MESSAGE_SRC(msg)), context);
			_this->aamp->setCurrentDrm((DRMSystems)gpGlobalConfig->preferredDrm);
		}

		break;
#endif
#ifdef __APPLE__
	case GST_MESSAGE_ELEMENT:
		if (
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
				(nullptr == _this->cbExportYUVFrame) &&
#endif
			gCbgetWindowContentView && gst_is_video_overlay_prepare_window_handle_message(msg))
		{
			logprintf("Recieved prepare-window-handle. Attaching video to window handle=%llu",(*gCbgetWindowContentView)());
			gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (msg)), (*gCbgetWindowContentView)());
			gst_message_unref (msg);
		}
		break;
#endif
	default:
		break;
	}

	return GST_BUS_PASS;
}


/**
 * @brief Create a new Gstreamer pipeline
 */
bool AAMPGstPlayer::CreatePipeline()
{
	bool ret = false;
	logprintf("%s(): Creating gstreamer pipeline", __FUNCTION__);

	if (privateContext->pipeline || privateContext->bus)
	{
		DestroyPipeline();
	}

	privateContext->pipeline = gst_pipeline_new("AAMPGstPlayerPipeline");
	if (privateContext->pipeline)
	{
		privateContext->bus = gst_pipeline_get_bus(GST_PIPELINE(privateContext->pipeline));
		if (privateContext->bus)
		{
			privateContext->busWatchId = gst_bus_add_watch(privateContext->bus, (GstBusFunc) bus_message, this);
#ifdef USE_GST1
			gst_bus_set_sync_handler(privateContext->bus, (GstBusSyncHandler) bus_sync_handler, this, NULL);
#else
			gst_bus_set_sync_handler(privateContext->bus, (GstBusSyncHandler) bus_sync_handler, this);
#endif
			privateContext->buffering_enabled = gpGlobalConfig->gstreamerBufferingBeforePlay;
			privateContext->buffering_in_progress = false;
			privateContext->buffering_timeout_cnt = DEFAULT_BUFFERING_MAX_CNT;
			privateContext->buffering_target_state = GST_STATE_NULL;
#ifdef INTELCE
			privateContext->buffering_enabled = false;
			logprintf("%s buffering_enabled forced 0, INTELCE", GST_ELEMENT_NAME(privateContext->pipeline));
#else
			logprintf("%s buffering_enabled %u", GST_ELEMENT_NAME(privateContext->pipeline), privateContext->buffering_enabled);
#endif
			if (privateContext->positionQuery == NULL)
			{
				privateContext->positionQuery = gst_query_new_position(GST_FORMAT_TIME);
			}
			ret = true;
		}
		else
		{
			logprintf("AAMPGstPlayer - gst_pipeline_get_bus failed");
		}
	}
	else
	{
		logprintf("AAMPGstPlayer - gst_pipeline_new failed");
	}

	return ret;
}


/**
 * @brief Cleanup an existing Gstreamer pipeline and associated resources
 */
void AAMPGstPlayer::DestroyPipeline()
{
	if (privateContext->pipeline)
	{
		gst_object_unref(privateContext->pipeline);
		privateContext->pipeline = NULL;
	}
	if (privateContext->busWatchId != 0)
	{
		g_source_remove(privateContext->busWatchId);
		privateContext->busWatchId = 0;
	}
	if (privateContext->bus)
	{
		gst_object_unref(privateContext->bus);
		privateContext->bus = NULL;
	}

	if (privateContext->positionQuery)
	{
		gst_query_unref(privateContext->positionQuery);
		privateContext->positionQuery = NULL;
	}

	//video decoder handle will change with new pipeline
	privateContext->decoderHandleNotified = false;

	logprintf("%s(): Destroying gstreamer pipeline", __FUNCTION__);
}


/**
 * @brief Retrieve the video decoder handle from pipeline
 * @retval the decoder handle
 */
unsigned long AAMPGstPlayer::getCCDecoderHandle()
{
	gpointer dec_handle = NULL;
	if (this->privateContext->stream[eMEDIATYPE_VIDEO].using_playersinkbin && this->privateContext->stream[eMEDIATYPE_VIDEO].sinkbin != NULL)
	{
		logprintf("Querying playersinkbin for handle");
		g_object_get(this->privateContext->stream[eMEDIATYPE_VIDEO].sinkbin, "video-decode-handle", &dec_handle, NULL);
	}
	else if(this->privateContext->video_dec != NULL)
	{
		logprintf("Querying video decoder for handle");
#ifndef INTELCE
		g_object_get(this->privateContext->video_dec, "videodecoder", &dec_handle, NULL);
#else
		g_object_get(privateContext->video_dec, "decode-handle", &dec_handle, NULL);
#endif
	}
	logprintf("video decoder handle received %p for video_dec %p", dec_handle, privateContext->video_dec);
	return (unsigned long)dec_handle;
}

/**
 * @brief Generate a protection event
 * @param[in] protSystemId keysystem to be used
 * @param[in] initData DRM initialization data
 * @param[in] initDataSize DRM initialization data size
 */
void AAMPGstPlayer::QueueProtectionEvent(const char *protSystemId, const void *initData, size_t initDataSize, MediaType type)
{
#ifdef AAMP_MPD_DRM
  	GstBuffer *pssi;

	// There is a possibility that only single protection event is queued for multiple type
	// since they are encrypted using same id. Don'tt worry if you see only one protection event queued here
	logprintf("queueing protection event for type:%d keysystem: %s initdata size: %d", type, protSystemId, initDataSize);

	if (privateContext->protectionEvent[type] != NULL)
	{
		AAMPLOG_WARN("%s:%d Previously cached protection event is present, clearing!", __FUNCTION__, __LINE__);
		gst_event_unref(privateContext->protectionEvent[type]);
		privateContext->protectionEvent[type] = NULL;
	}

	pssi = gst_buffer_new_wrapped(g_memdup (initData, initDataSize), initDataSize);
	if (this->aamp->IsDashAsset())
	{
		privateContext->protectionEvent[type] = gst_event_new_protection (protSystemId, pssi, "dash/mpd");
	}
	else
	{
		privateContext->protectionEvent[type] = gst_event_new_protection (protSystemId, pssi, "hls/m3u8");
	}

	gst_buffer_unref (pssi);
#endif
}

/**
 * @brief Cleanup generated protection event
 */
void AAMPGstPlayer::ClearProtectionEvent()
{
	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		if(privateContext->protectionEvent[i])
		{
			logprintf("%s removing protection event [type:%d]! ", __FUNCTION__, i);
			gst_event_unref(privateContext->protectionEvent[i]);
			privateContext->protectionEvent[i] = NULL;
		}
	}
}

/**
 * @brief Callback for receiving playersinkbin gstreamer events
 * @param[in] playersinkbin instance of playersinkbin
 * @param[in] status event name
 * @param[in] arg user data (pointer to AAMPGstPlayer instance)
 */
static void AAMPGstPlayer_PlayersinkbinCB(GstElement * playersinkbin, gint status,  void* arg)
{
	AAMPGstPlayer *_this = (AAMPGstPlayer *)arg;
	switch (status)
	{
		case GSTPLAYERSINKBIN_EVENT_HAVE_VIDEO:
			GST_INFO("got Video PES.");
			break;
		case GSTPLAYERSINKBIN_EVENT_HAVE_AUDIO:
			GST_INFO("got Audio PES");
			break;
		case GSTPLAYERSINKBIN_EVENT_FIRST_VIDEO_FRAME:
			GST_INFO("got First Video Frame");
			_this->NotifyFirstFrame(eMEDIATYPE_VIDEO);
			break;
		case GSTPLAYERSINKBIN_EVENT_FIRST_AUDIO_FRAME:
			GST_INFO("got First Audio Sample");
			_this->NotifyFirstFrame(eMEDIATYPE_AUDIO);
			break;
		case GSTPLAYERSINKBIN_EVENT_ERROR_VIDEO_UNDERFLOW:
			//TODO - Handle underflow
			logprintf("## %s() : Got Underflow message from video pipeline ##", __FUNCTION__);
			break;
		case GSTPLAYERSINKBIN_EVENT_ERROR_AUDIO_UNDERFLOW:
			//TODO - Handle underflow
			logprintf("## %s() : Got Underflow message from audio pipeline ##", __FUNCTION__);
			break;
		case GSTPLAYERSINKBIN_EVENT_ERROR_VIDEO_PTS:
			//TODO - Handle PTS error
			logprintf("## %s() : Got PTS error message from video pipeline ##", __FUNCTION__);
			break;
		case GSTPLAYERSINKBIN_EVENT_ERROR_AUDIO_PTS:
			//TODO - Handle PTS error
			logprintf("## %s() : Got PTS error message from audio pipeline ##", __FUNCTION__);
			break;
		default:
			GST_INFO("%s status = 0x%x (Unknown)", __FUNCTION__, status);
			break;
	}
}


/**
 * @brief Create an appsrc element for a particular format
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @param[in] format data format for setting src pad caps
 * @retval pointer to appsrc instance
 */
static GstElement* AAMPGstPlayer_GetAppSrc(AAMPGstPlayer *_this, StreamOutputFormat format)
{
	GstElement *source;
	GstCaps * caps;
	source = gst_element_factory_make("appsrc", NULL);
	if (NULL == source)
	{
		logprintf("AAMPGstPlayer_GetAppSrc Cannot create source");
		return NULL;
	}
	InitializeSource( _this, G_OBJECT(source) );

	caps = GetGstCaps(format);
	gst_app_src_set_caps(GST_APP_SRC(source), caps);
	gst_caps_unref(caps);
	return source;
}


/**
 * @brief Cleanup resources and flags for a particular stream type
 * @param[in] mediaType stream type
 */
void AAMPGstPlayer::TearDownStream(MediaType mediaType)
{
	media_stream* stream = &privateContext->stream[mediaType];
	stream->bufferUnderrun = false;
	stream->eosReached = false;
	stream->flush = false;
	if ((stream->format != FORMAT_INVALID) && (stream->format != FORMAT_NONE))
	{
		logprintf("AAMPGstPlayer::TearDownStream: mediaType %d ", (int)mediaType);
		if (privateContext->pipeline)
		{
			privateContext->buffering_in_progress = false;   /* stopping pipeline, don't want to change state if GST_MESSAGE_ASYNC_DONE message comes in */
			/* set the playbin state to NULL before detach it */
			if (stream->sinkbin && (GST_STATE_CHANGE_FAILURE == gst_element_set_state(GST_ELEMENT(stream->sinkbin), GST_STATE_NULL)))
			{
				logprintf("AAMPGstPlayer::TearDownStream: Failed to set NULL state for sinkbin");
			}

			if (stream->sinkbin && (!gst_bin_remove(GST_BIN(privateContext->pipeline), GST_ELEMENT(stream->sinkbin))))
			{
				logprintf("AAMPGstPlayer::TearDownStream:  Unable to remove sinkbin from pipeline");
			}
			if (stream->using_playersinkbin)
			{
				if (!gst_bin_remove(GST_BIN(privateContext->pipeline), GST_ELEMENT(stream->source)))
				{
					logprintf("AAMPGstPlayer::TearDownStream:  Unable to remove source from pipeline");
				}
			}
		}
		//After sinkbin is removed from pipeline, a new decoder handle may be generated
		if (mediaType == eMEDIATYPE_VIDEO)
		{
			privateContext->decoderHandleNotified = false;
		}
		stream->format = FORMAT_INVALID;
		stream->sinkbin = NULL;
		stream->source = NULL;
	}
	if (mediaType == eMEDIATYPE_VIDEO)
	{
		privateContext->video_dec = NULL;
#if !defined(INTELCE) || defined(INTELCE_USE_VIDRENDSINK)
		privateContext->video_sink = NULL;
#endif

#ifdef INTELCE_USE_VIDRENDSINK
		privateContext->video_pproc = NULL;
#endif
	}
	else if (mediaType == eMEDIATYPE_AUDIO)
	{
		privateContext->audio_dec = NULL;
		privateContext->audio_sink = NULL;
	}
	logprintf("AAMPGstPlayer::TearDownStream:  exit mediaType = %d", mediaType);
}


/**
 * @brief Setup pipeline for a particular stream type
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @param[in] streamId stream type
 * @retval 0, if setup successfully. -1, for failure
 */
static int AAMPGstPlayer_SetupStream(AAMPGstPlayer *_this, int streamId)
{
	media_stream* stream = &_this->privateContext->stream[streamId];

	if (!stream->using_playersinkbin)
	{
#ifdef USE_GST1
		logprintf("AAMPGstPlayer_SetupStream - using playbin");
		stream->sinkbin = gst_element_factory_make("playbin", NULL);
		if (_this->privateContext->using_westerossink && eMEDIATYPE_VIDEO == streamId)
		{
			logprintf("AAMPGstPlayer_SetupStream - using westerossink");
			GstElement* vidsink = gst_element_factory_make("westerossink", NULL);
#ifdef CONTENT_4K_SUPPORTED
			g_object_set(vidsink, "secure-video", TRUE, NULL);
#endif
			g_object_set(stream->sinkbin, "video-sink", vidsink, NULL);
		}
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
		else if(_this->cbExportYUVFrame)
		{
			if (eMEDIATYPE_VIDEO == streamId)
			{
				logprintf("AAMPGstPlayer_SetupStream - using appsink");
				GstElement* appsink = gst_element_factory_make("appsink", NULL);
				assert(appsink);
				GstCaps *caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", NULL);
				gst_app_sink_set_caps (GST_APP_SINK(appsink), caps);
				g_object_set (G_OBJECT (appsink), "emit-signals", TRUE, "sync", TRUE, NULL);
				g_signal_connect (appsink, "new-sample", G_CALLBACK (AAMPGstPlayer::AAMPGstPlayer_OnVideoSample), _this);
				g_object_set(stream->sinkbin, "video-sink", appsink, NULL);
				_this->privateContext->video_sink = appsink;
			}
		}
#endif
#else
		logprintf("AAMPGstPlayer_SetupStream - using playbin2");
		stream->sinkbin = gst_element_factory_make("playbin2", NULL);
#endif
#if defined(INTELCE) && !defined(INTELCE_USE_VIDRENDSINK)
		if (eMEDIATYPE_VIDEO == streamId)
		{
			logprintf("%s:%d - using ismd_vidsink", __FUNCTION__, __LINE__);
			GstElement* vidsink = _this->privateContext->video_sink;
			if(NULL == vidsink)
			{
				vidsink = gst_element_factory_make("ismd_vidsink", NULL);
				if(!vidsink)
				{
					logprintf("%s:%d - Could not create ismd_vidsink element", __FUNCTION__, __LINE__);
				}
				else
				{
					_this->privateContext->video_sink = GST_ELEMENT(gst_object_ref( vidsink));
				}
			}
			else
			{
				logprintf("%s:%d Reusing existing vidsink element", __FUNCTION__, __LINE__);
			}
			logprintf("%s:%d Set video-sink %p to playbin %p", __FUNCTION__, __LINE__, vidsink, stream->sinkbin);
			g_object_set(stream->sinkbin, "video-sink", vidsink, NULL);
		}
#endif
		gst_bin_add(GST_BIN(_this->privateContext->pipeline), stream->sinkbin);
		gint flags;
		g_object_get(stream->sinkbin, "flags", &flags, NULL);
		logprintf("playbin flags1: 0x%x", flags); // 0x617 on settop
#if defined NO_NATIVE_AV || (defined(__APPLE__))
		flags = GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
#else
		flags = GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_NATIVE_AUDIO | GST_PLAY_FLAG_NATIVE_VIDEO;
#endif
		g_object_set(stream->sinkbin, "flags", flags, NULL); // needed?
		if((_this->aamp->getStreamType() != 30) ||  gpGlobalConfig->useAppSrcForProgressivePlayback)
		{
			g_object_set(stream->sinkbin, "uri", "appsrc://", NULL);
			g_signal_connect(stream->sinkbin, "deep-notify::source", G_CALLBACK(found_source), _this);
		}else
		{
			g_object_set(stream->sinkbin, "uri", _this->aamp->GetManifestUrl().c_str(), NULL);
			g_signal_connect (stream->sinkbin, "source-setup", G_CALLBACK (httpsoup_source_setup), _this);
		}
		gst_element_sync_state_with_parent(stream->sinkbin);
		_this->privateContext->gstPropsDirty = true;
	}
	else
	{
		stream->source = AAMPGstPlayer_GetAppSrc(_this,stream->format);
		gst_bin_add(GST_BIN(_this->privateContext->pipeline), stream->source);
		gst_element_sync_state_with_parent(stream->source);
		stream->sinkbin = gst_element_factory_make("playersinkbin", NULL);
		if (NULL == stream->sinkbin)
		{
			logprintf("AAMPGstPlayer_SetupStream Cannot create sink");
			return -1;
		}
		g_signal_connect(stream->sinkbin, "event-callback", G_CALLBACK(AAMPGstPlayer_PlayersinkbinCB), _this);
		gst_bin_add(GST_BIN(_this->privateContext->pipeline), stream->sinkbin);
		gst_element_link(stream->source, stream->sinkbin);
		gst_element_sync_state_with_parent(stream->sinkbin);

		logprintf("AAMPGstPlayer_SetupStream:  Created playersinkbin. Setting rectangle");
		g_object_set(stream->sinkbin, "rectangle",  _this->privateContext->videoRectangle, NULL);
		g_object_set(stream->sinkbin, "zoom", _this->privateContext->zoom, NULL);
		g_object_set(stream->sinkbin, "video-mute", _this->privateContext->videoMuted, NULL);
		g_object_set(stream->sinkbin, "volume", _this->privateContext->audioVolume, NULL);
		_this->privateContext->gstPropsDirty = false;
	}
	return 0;
}


/**
 * @brief Send any pending/cached events to pipeline
 * @param[in] privateContext pointer to AAMPGstPlayerPriv instance
 * @param[in] mediaType stream type
 * @param[in] pts PTS of next buffer
 */
static void AAMPGstPlayer_SendPendingEvents(PrivateInstanceAAMP *aamp, AAMPGstPlayerPriv *privateContext, MediaType mediaType, GstClockTime pts)
{
	media_stream* stream = &privateContext->stream[mediaType];
	gboolean enableOverride = FALSE;
	GstPad* sourceEleSrcPad = gst_element_get_static_pad(GST_ELEMENT(stream->source), "src");
	if(stream->flush)
	{
		logprintf("%s:%d flush pipeline", __FUNCTION__, __LINE__);
		gboolean ret = gst_pad_push_event(sourceEleSrcPad, gst_event_new_flush_start());
		if (!ret) logprintf("%s: flush start error", __FUNCTION__);
#ifdef USE_GST1
		GstEvent* event = gst_event_new_flush_stop(FALSE);
#else
		GstEvent* event = gst_event_new_flush_stop();
#endif
		ret = gst_pad_push_event(sourceEleSrcPad, event);
		if (!ret) logprintf("%s: flush stop error", __FUNCTION__);
		stream->flush = false;
	}

	if (stream->format == FORMAT_ISO_BMFF)
	{
#if (defined(INTELCE) || defined(RPI) || defined(__APPLE__))
		enableOverride = TRUE;
#else
		enableOverride = (privateContext->rate != AAMP_NORMAL_PLAY_RATE);
#endif
		GstStructure * eventStruct = gst_structure_new("aamp_override", "enable", G_TYPE_BOOLEAN, enableOverride, "rate", G_TYPE_FLOAT, (float)privateContext->rate, "aampplayer", G_TYPE_BOOLEAN, TRUE, NULL);
#if (defined(INTELCE) || defined(RPI) || defined(__APPLE__))
		if ((privateContext->rate == AAMP_NORMAL_PLAY_RATE))
		{
			guint64 basePTS = aamp->GetFirstPTS() * GST_SECOND;
			logprintf("%s: Set override event's basePTS [ %" G_GUINT64_FORMAT "]", __FUNCTION__, basePTS);
			gst_structure_set (eventStruct, "basePTS", G_TYPE_UINT64, basePTS, NULL);
		}
#endif
		if (!gst_pad_push_event(sourceEleSrcPad, gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, eventStruct)))
		{
			logprintf("%s: Error on sending rate override event", __FUNCTION__);
		}
	}

	if (mediaType == eMEDIATYPE_VIDEO)
	{
		// DELIA-39530 - Westerossink gives position as an absolute value from segment.start. In AAMP's GStreamer pipeline
		// appsrc's base class - basesrc sends an additional segment event since we performed a flushing seek.
		// To figure out the new segment.start, we need to send a segment query which will be replied
		// by basesrc to get the updated segment event values.
		// When override is enabled qtdemux internally restamps and sends segment.start = 0 which is part of
		// AAMP's change in qtdemux so we don't need to query segment.start
		// Enabling position query based progress reporting for non-westerossink configurations
		if (gpGlobalConfig->bPositionQueryEnabled && enableOverride == FALSE)
		{
			privateContext->segmentStart = -1;
		}
		else
		{
			privateContext->segmentStart = 0;
		}
	}

#ifdef USE_GST1
	GstSegment segment;
	gst_segment_init(&segment, GST_FORMAT_TIME);
	segment.start = pts;
	segment.position = 0;
	segment.rate = AAMP_NORMAL_PLAY_RATE;
	segment.applied_rate = AAMP_NORMAL_PLAY_RATE;
	logprintf("Sending segment event for mediaType[%d]. start %" G_GUINT64_FORMAT " stop %" G_GUINT64_FORMAT" rate %f applied_rate %f", mediaType, segment.start, segment.stop, segment.rate, segment.applied_rate);
	GstEvent* event = gst_event_new_segment (&segment);
#else
	GstEvent* event = gst_event_new_new_segment (FALSE, 1.0, GST_FORMAT_TIME, pts, GST_CLOCK_TIME_NONE, 0);
#endif
	if (!gst_pad_push_event(sourceEleSrcPad, event))
	{
		logprintf("%s: gst_pad_push_event segment error", __FUNCTION__);
	}

	if (stream->format == FORMAT_ISO_BMFF)
	{
		// There is a possibility that only single protection event is queued for multiple type
		// since they are encrypted using same id. Hence check if proection event is queued for
		// other types
		GstEvent* event = privateContext->protectionEvent[mediaType];
		if (event == NULL)
		{
			// Check protection event for other types
			for (int i = 0; i < AAMP_TRACK_COUNT; i++)
			{
				if (i != mediaType && privateContext->protectionEvent[i] != NULL)
				{
					event = privateContext->protectionEvent[i];
					break;
				}
			}
		}
		if(event)
		{
			logprintf("%s pushing protection event! mediatype: %d", __FUNCTION__, mediaType);
			if (!gst_pad_push_event(sourceEleSrcPad, gst_event_ref(event)))
			{
				logprintf("%s push protection event failed!", __FUNCTION__);
			}
		}
	}
#ifdef INTELCE
	if (!gst_pad_push_event(sourceEleSrcPad, gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, gst_structure_new("discard-segment-event-with-zero-start", "enable", G_TYPE_BOOLEAN, TRUE, NULL))))
	{
		logprintf("%s: Error on sending discard-segment-event-with-zero-start custom event", __FUNCTION__);
	}
#endif

	gst_object_unref(sourceEleSrcPad);
	stream->resetPosition = false;
	stream->flush = false;
}


/**
 * @brief Check if segment starts with an ID3 section
 * @param[in] data pointer to segment buffer
 * @param[in] length length of segment buffer
 * @retval true if segment has an ID3 section
 */
bool hasId3Header(MediaType mediaType, StreamOutputFormat format, const uint8_t* data, int32_t length)
{
	if ((mediaType == eMEDIATYPE_AUDIO || mediaType == eMEDIATYPE_VIDEO) && length >= 3)
	{
		/* Check file identifier ("ID3" = ID3v2) and major revision matches (>= ID3v2.2.x). */
		if (*data++ == 'I' && *data++ == 'D' && *data++ == '3' && *data++ >= 2)
		{
			return true;
		}
	}

	return false;
}

#define ID3_HEADER_SIZE 10

/**
 * @brief Get the size of the ID3v2 tag.
 * @param[in] ptr buffer pointer
 * @param[in] len0 length of buffer
 */
uint32_t getId3TagSize(const uint8_t *data, size_t &len0)
{
	uint32_t bufferSize = 0;
	uint8_t tagSize[4];

	memcpy(tagSize, data+6, 4);

	// bufferSize is encoded as a syncsafe integer - this means that bit 7 is always zeroed
	// Check for any 1s in bit 7
	if (tagSize[0] > 0x7f || tagSize[1] > 0x7f || tagSize[2] > 0x7f || tagSize[3] > 0x7f)
	{
		AAMPLOG_WARN("%s:%d Bad header format", __FUNCTION__, __LINE__);
		return 0;
	}

	bufferSize = tagSize[0] << 21;
	bufferSize += tagSize[1] << 14;
	bufferSize += tagSize[2] << 7;
	bufferSize += tagSize[3];
	bufferSize += ID3_HEADER_SIZE;

	return bufferSize;
}



/**
 * @brief Inject buffer of a stream type to its pipeline
 * @param[in] mediaType stream type
 * @param[in] ptr buffer pointer
 * @param[in] len0 length of buffer
 * @param[in] fpts PTS of buffer (in sec)
 * @param[in] fdts DTS of buffer (in sec)
 * @param[in] fDuration duration of buffer (in sec)
 */
void AAMPGstPlayer::Send(MediaType mediaType, const void *ptr, size_t len0, double fpts, double fdts, double fDuration)
{
#define MAX_BYTES_TO_SEND (128*1024)
	GstClockTime pts = (GstClockTime)(fpts * GST_SECOND);
	GstClockTime dts = (GstClockTime)(fdts * GST_SECOND);
	GstClockTime duration = (GstClockTime)(fDuration * 1000000000LL);

	if (aamp->GetEventListenerStatus(AAMP_EVENT_ID3_METADATA) &&
		hasId3Header(mediaType, privateContext->stream[eMEDIATYPE_AUDIO].format,
								static_cast<const uint8_t*>(ptr), len0))
	{
		Id3CallbackData* id3Metadata = new Id3CallbackData;
		id3Metadata->_this = this;
		id3Metadata->len = getId3TagSize(static_cast<const uint8_t*>(ptr), len0);
		if (id3Metadata->len) {
			id3Metadata->data = (uint8_t*)g_malloc(id3Metadata->len);
			//TODO: Consider maximum length for ID3 data - spec allows 256MB
			if (id3Metadata->data) {
				memcpy(id3Metadata->data, ptr, id3Metadata->len);
			}

			privateContext->id3MetadataCallbackTaskPending = true;
			privateContext->id3MetadataCallbackIdleTaskId = g_idle_add(IdleCallbackOnId3Metadata, id3Metadata);
		}
	}

	gboolean discontinuity = FALSE;
	size_t maxBytes;
	GstFlowReturn ret;
	bool isFirstBuffer = privateContext->stream[mediaType].resetPosition;

	if (privateContext->stream[eMEDIATYPE_VIDEO].format == FORMAT_ISO_BMFF)
	{
		//For mpeg-dash, sent the entire fragment.
		maxBytes = len0;
	}
	else
	{
		//For Dash, if using playersinkbin, broadcom plugins has buffer size limitation.
		maxBytes = MAX_BYTES_TO_SEND;
	}
#ifdef TRACE_VID_PTS
	if (mediaType == eMEDIATYPE_VIDEO && privateContext->rate != AAMP_NORMAL_PLAY_RATE)
	{
		logprintf("AAMPGstPlayer %s : rate %d fpts %f pts %llu pipeline->stream_time %lu ", (mediaType == eMEDIATYPE_VIDEO)?"vid":"aud", privateContext->rate, fpts, (unsigned long long)pts, GST_PIPELINE(this->privateContext->pipeline)->stream_time);
		GstClock* clock = gst_pipeline_get_clock(GST_PIPELINE(this->privateContext->pipeline));
		if (clock)
		{
			GstClockTime curr = gst_clock_get_time(clock);
			logprintf("  clock time %lu diff %lu (%f sec)", curr, pts-curr, (float)(pts-curr)/GST_SECOND);
			gst_object_unref(clock);
		}
	}
#endif

#ifdef DUMP_STREAM
	static FILE* fp = NULL;
	if (!fp)
	{
		fp = fopen("AAMPGstPlayerdump.ts", "w");
	}
	fwrite(ptr, 1, len0, fp );
#endif
	if (isFirstBuffer)
	{
		AAMPGstPlayer_SendPendingEvents(aamp, privateContext, mediaType, pts);
		discontinuity = TRUE;
	}

	while (aamp->DownloadsAreEnabled())
	{
		size_t len = len0;
		if (len > maxBytes)
		{
			len = maxBytes;
		}
		GstBuffer *buffer = gst_buffer_new_and_alloc((guint)len);
		if (discontinuity )
		{
			GST_BUFFER_FLAG_SET(buffer, GST_BUFFER_FLAG_DISCONT);
			discontinuity = FALSE;
		}
#ifdef USE_GST1
		GstMapInfo map;
		gst_buffer_map(buffer, &map, GST_MAP_WRITE);
		memcpy(map.data, ptr, len);
		gst_buffer_unmap(buffer, &map);
		GST_BUFFER_PTS(buffer) = pts;
		GST_BUFFER_DTS(buffer) = dts;
		//GST_BUFFER_DURATION(buffer) = duration;
#else
		memcpy(GST_BUFFER_DATA(buffer), ptr, len);
		GST_BUFFER_TIMESTAMP(buffer) = pts;
		GST_BUFFER_DURATION(buffer) = duration;
#endif
		ret = gst_app_src_push_buffer(GST_APP_SRC(privateContext->stream[mediaType].source), buffer);
		if (ret != GST_FLOW_OK)
		{
			logprintf("gst_app_src_push_buffer error: %d[%s] mediaType %d", ret, gst_flow_get_name (ret), (int)mediaType);
			assert(false);
		}
		else if (privateContext->stream[mediaType].bufferUnderrun)
		{
			privateContext->stream[mediaType].bufferUnderrun = false;
		}
		ptr = len + (unsigned char *)ptr;
		len0 -= len;
		if (len0 == 0)
		{
			break;
		}
	}
	if (eMEDIATYPE_VIDEO == mediaType)
	{
		if (isFirstBuffer)
		{
			aamp->NotifyFirstBufferProcessed();
		}
		privateContext->numberOfVideoBuffersSent++;
		StopBuffering(false);
	}
}


/**
 * @brief Inject buffer of a stream type to its pipeline
 * @param[in] mediaType stream type
 * @param[in] pBuffer buffer as GrowableBuffer pointer
 * @param[in] fpts PTS of buffer (in sec)
 * @param[in] fdts DTS of buffer (in sec)
 * @param[in] fDuration duration of buffer (in sec)
 */
void AAMPGstPlayer::Send(MediaType mediaType, GrowableBuffer* pBuffer, double fpts, double fdts, double fDuration)
{
	GstClockTime pts = (GstClockTime)(fpts * GST_SECOND);
	GstClockTime dts = (GstClockTime)(fdts * GST_SECOND);
	GstClockTime duration = (GstClockTime)(fDuration * 1000000000LL);
	gboolean discontinuity = FALSE;
	bool isFirstBuffer = privateContext->stream[mediaType].resetPosition;

#ifdef TRACE_VID_PTS
	if (mediaType == eMEDIATYPE_VIDEO && privateContext->rate != AAMP_NORMAL_PLAY_RATE)
	{
		logprintf("AAMPGstPlayer %s : rate %d fpts %f pts %llu pipeline->stream_time %lu ", (mediaType == eMEDIATYPE_VIDEO)?"vid":"aud", privateContext->rate, fpts, (unsigned long long)pts, GST_PIPELINE(this->privateContext->pipeline)->stream_time);
		GstClock* clock = gst_pipeline_get_clock(GST_PIPELINE(this->privateContext->pipeline));
		if (clock)
		{
			GstClockTime curr = gst_clock_get_time(clock);
			logprintf("  clock time %lu diff %lu (%f sec)", curr, pts-curr, (float)(pts-curr)/GST_SECOND);
			gst_object_unref(clock);
		}
	}
#endif

#ifdef DUMP_STREAM
	static FILE* fp = NULL;
	if (!fp)
	{
		fp = fopen("AAMPGstPlayerdump.ts", "w");
	}
	fwrite(pBuffer->ptr , 1, pBuffer->len, fp );
#endif
	if (isFirstBuffer)
	{
		AAMPGstPlayer_SendPendingEvents(aamp, privateContext, mediaType, pts);
		discontinuity = TRUE;
	}

#ifdef USE_GST1
	GstBuffer* buffer = gst_buffer_new_wrapped (pBuffer->ptr ,pBuffer->len);
	GST_BUFFER_PTS(buffer) = pts;
	GST_BUFFER_DTS(buffer) = dts;
#else
	GstBuffer* buffer = gst_buffer_new();
	GST_BUFFER_SIZE (buffer) = pBuffer->len;
	GST_BUFFER_MALLOCDATA (buffer) = (guint8*)pBuffer->ptr;
	GST_BUFFER_DATA (buffer) = GST_BUFFER_MALLOCDATA (buffer);
	GST_BUFFER_TIMESTAMP(buffer) = pts;
	GST_BUFFER_DURATION(buffer) = duration;
#endif

	GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(privateContext->stream[mediaType].source), buffer);
	if (ret != GST_FLOW_OK)
	{
		logprintf("gst_app_src_push_buffer error: %d[%s] mediaType %d", ret, gst_flow_get_name (ret), (int)mediaType);
		assert(false);
	}
	else if (privateContext->stream[mediaType].bufferUnderrun)
	{
		privateContext->stream[mediaType].bufferUnderrun = false;
	}

	/*Since ownership of buffer is given to gstreamer, reset pBuffer */
	memset(pBuffer, 0x00, sizeof(GrowableBuffer));
	if (eMEDIATYPE_VIDEO == mediaType)
	{
		if (isFirstBuffer)
		{
			aamp->NotifyFirstBufferProcessed();
		}
		privateContext->numberOfVideoBuffersSent++;
		StopBuffering(false);
	}

}



/**
 * @brief To start playback
 */
void AAMPGstPlayer::Stream()
{
}


/**
 * @brief Configure pipeline based on A/V formats
 * @param[in] format video format
 * @param[in] audioFormat audio format
 * @param[in] bESChangeStatus
 */
void AAMPGstPlayer::Configure(StreamOutputFormat format, StreamOutputFormat audioFormat, bool bESChangeStatus)
{
	logprintf("AAMPGstPlayer::%s %d > format %d audioFormat %d", __FUNCTION__, __LINE__, format, audioFormat);
	StreamOutputFormat newFormat[AAMP_TRACK_COUNT];
	newFormat[eMEDIATYPE_VIDEO] = format;
	newFormat[eMEDIATYPE_AUDIO] = audioFormat;
	newFormat[eMEDIATYPE_SUBTITLE] = FORMAT_NONE;

	if (!aamp->mWesterosSinkEnabled)
	{
		privateContext->using_westerossink = false;
	}
	else
	{
		privateContext->using_westerossink = true;
	}

#ifdef AAMP_STOP_SINK_ON_SEEK
	privateContext->rate = aamp->rate;
#endif

	if (privateContext->pipeline == NULL || privateContext->bus == NULL)
	{
		CreatePipeline();
	}

	bool configureStream = false;

	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		media_stream *stream = &privateContext->stream[i];
		if (stream->format != newFormat[i])
		{
			if ((newFormat[i] != FORMAT_INVALID) && (newFormat[i] != FORMAT_NONE))
			{
				logprintf("AAMPGstPlayer::%s %d > Closing stream %d old format = %d, new format = %d",
								__FUNCTION__, __LINE__, i, stream->format, newFormat[i]);
				configureStream = true;
			}
		}

		/* Force configure the bin for mid stream audio type change */
		if (!configureStream && bESChangeStatus && (eMEDIATYPE_AUDIO == i))
		{
			logprintf("AAMPGstPlayer::%s %d > AudioType Changed. Force configure pipeline", __FUNCTION__, __LINE__);
			configureStream = true;
		}

		stream->resetPosition = true;
		stream->eosReached = false;
	}

	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		media_stream *stream = &privateContext->stream[i];
		if (configureStream && (newFormat[i] != FORMAT_INVALID) && (newFormat[i] != FORMAT_NONE))
		{
			TearDownStream((MediaType) i);
			stream->format = newFormat[i];
	#ifdef USE_PLAYERSINKBIN
			if (FORMAT_MPEGTS == stream->format )
			{
				logprintf("AAMPGstPlayer::%s %d > - using playersinkbin, track = %d", __FUNCTION__, __LINE__, i);
				stream->using_playersinkbin = TRUE;
			}
			else
	#endif
			{
				stream->using_playersinkbin = FALSE;
			}
			if (0 != AAMPGstPlayer_SetupStream(this, (MediaType) i))
			{
				logprintf("AAMPGstPlayer::%s %d > track %d failed", __FUNCTION__, __LINE__, i);
				return;
			}
		}
	}
	if(aamp->IsFragmentBufferingRequired())
	{
		if (gst_element_set_state(this->privateContext->pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
		{
			logprintf("AAMPGstPlayer::%s %d > GST_STATE_PAUSED failed", __FUNCTION__, __LINE__);
		}
		privateContext->pendingPlayState = true;
	}
	else
	{
		if (this->privateContext->buffering_enabled && format != FORMAT_NONE && format != FORMAT_INVALID && AAMP_NORMAL_PLAY_RATE == privateContext->rate)
		{
			if (gst_element_set_state(this->privateContext->pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
			{
				logprintf("AAMPGstPlayer_Configure GST_STATE_PLAYING failed");
			}
			this->privateContext->buffering_target_state = GST_STATE_PLAYING;
			this->privateContext->buffering_in_progress = true;
			this->privateContext->buffering_timeout_cnt = DEFAULT_BUFFERING_MAX_CNT;
			privateContext->pendingPlayState = false;
		}
		else
		{
			if (gst_element_set_state(this->privateContext->pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
			{
				logprintf("AAMPGstPlayer::%s %d > GST_STATE_PLAYING failed", __FUNCTION__, __LINE__);
			}
			privateContext->pendingPlayState = false;
		}
	}
	privateContext->eosSignalled = false;
	privateContext->numberOfVideoBuffersSent = 0;
	privateContext->paused = false;
#ifdef TRACE
	logprintf("exiting AAMPGstPlayer::%s", __FUNCTION__);
#endif
}


/**
 * @brief To signal EOS to a particular appsrc instance
 * @param[in] source pointer to appsrc instance
 */
static void AAMPGstPlayer_SignalEOS(GstElement *source )
{
	if (source )
	{
		GstFlowReturn ret;
		g_signal_emit_by_name(source, "end-of-stream", &ret);
		if (ret != GST_FLOW_OK)
		{
			logprintf("gst_app_src_push_buffer  error: %d", ret);
		}
	}
}


/**
 * @brief Starts processing EOS for a particular stream type
 * @param[in] type stream type
 */
void AAMPGstPlayer::EndOfStreamReached(MediaType type)
{
	logprintf("entering AAMPGstPlayer_EndOfStreamReached type %d", (int)type);

	media_stream *stream = &privateContext->stream[type];
	stream->eosReached = true;
	if ((stream->format != FORMAT_NONE && stream->format != FORMAT_INVALID) && stream->resetPosition == true)
	{
		logprintf("%s(): EOS received as first buffer ", __FUNCTION__);
		NotifyEOS();
	}
	else
	{
		NotifyFragmentCachingComplete();
		AAMPGstPlayer_SignalEOS(stream->source);
		/*For trickmodes, give EOS to audio source*/
		if (AAMP_NORMAL_PLAY_RATE != privateContext->rate)
		{
			AAMPGstPlayer_SignalEOS(privateContext->stream[eMEDIATYPE_AUDIO].source);
			if (privateContext->stream[eMEDIATYPE_SUBTITLE].source)
			{
				AAMPGstPlayer_SignalEOS(privateContext->stream[eMEDIATYPE_SUBTITLE].source);
			}
		}
		// We are in buffering, but we received end of stream, un-pause pipeline
		StopBuffering(true);
	}
}

void AAMPGstPlayer::DisconnectCallbacks()
{
	if(privateContext->video_dec)
	{
		type_check_instance("AAMPGstPlayer::DisconnectCallbacks: video_dec ", privateContext->video_dec);
		g_signal_handlers_disconnect_by_data(privateContext->video_dec, this);
	}
	if(privateContext->audio_dec)
	{
		type_check_instance("AAMPGstPlayer::DisconnectCallbacks: audio_dec ", privateContext->audio_dec);
		g_signal_handlers_disconnect_by_data(privateContext->audio_dec, this);
	}
}


/**
 * @brief Stop playback and any idle handlers active at the time
 * @param[in] keepLastFrame denotes if last video frame should be kept
 */
void AAMPGstPlayer::Stop(bool keepLastFrame)
{
	logprintf("entering AAMPGstPlayer_Stop keepLastFrame %d", keepLastFrame);
#ifdef INTELCE
	if (privateContext->video_sink)
	{
		privateContext->keepLastFrame = keepLastFrame;
		g_object_set(privateContext->video_sink,  "stop-keep-frame", keepLastFrame, NULL);
#if !defined(INTELCE_USE_VIDRENDSINK)
		if  (!keepLastFrame)
		{
			gst_object_unref(privateContext->video_sink);
			privateContext->video_sink = NULL;
		}
		else
		{
			g_object_set(privateContext->video_sink,  "reuse-vidrend", keepLastFrame, NULL);
		}
#endif
	}
#endif
	if(!keepLastFrame)
	{
		privateContext->firstFrameReceived = false;
	}
	if (privateContext->firstProgressCallbackIdleTaskPending)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove firstProgressCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->firstProgressCallbackIdleTaskId);
		g_source_remove(privateContext->firstProgressCallbackIdleTaskId);
		privateContext->firstProgressCallbackIdleTaskPending = false;
		privateContext->firstProgressCallbackIdleTaskId = 0;
	}
	if (this->privateContext->periodicProgressCallbackIdleTaskId)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove periodicProgressCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->periodicProgressCallbackIdleTaskId);
		g_source_remove(privateContext->periodicProgressCallbackIdleTaskId);
		privateContext->periodicProgressCallbackIdleTaskId = 0;
	}
	if (this->privateContext->bufferingTimeoutTimerId)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove bufferingTimeoutTimerId %d", __FUNCTION__, __LINE__, privateContext->bufferingTimeoutTimerId);
		g_source_remove(privateContext->bufferingTimeoutTimerId);
		privateContext->bufferingTimeoutTimerId = 0;
	}
	if (privateContext->ptsCheckForEosOnUnderflowIdleTaskId)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove ptsCheckForEosCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->ptsCheckForEosOnUnderflowIdleTaskId);
		g_source_remove(privateContext->ptsCheckForEosOnUnderflowIdleTaskId);
		privateContext->ptsCheckForEosOnUnderflowIdleTaskId = 0;
	}
	if (this->privateContext->eosCallbackIdleTaskPending)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove eosCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->eosCallbackIdleTaskId);
		g_source_remove(privateContext->eosCallbackIdleTaskId);
		privateContext->eosCallbackIdleTaskPending = false;
		privateContext->eosCallbackIdleTaskId = 0;
	}
	if (this->privateContext->firstFrameCallbackIdleTaskPending)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove firstFrameCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->firstFrameCallbackIdleTaskId);
		g_source_remove(privateContext->firstFrameCallbackIdleTaskId);
		privateContext->firstFrameCallbackIdleTaskPending = false;
		privateContext->firstFrameCallbackIdleTaskId = 0;
	}
	if (this->privateContext->id3MetadataCallbackTaskPending)
	{
		logprintf("AAMPGstPlayer::%s %d > Remove id3MetadataCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->id3MetadataCallbackIdleTaskId);
		g_source_remove(privateContext->id3MetadataCallbackIdleTaskId);
		privateContext->id3MetadataCallbackTaskPending = false;
		privateContext->id3MetadataCallbackIdleTaskId = 0;
	}
	if (this->privateContext->pipeline)
	{
		GstState current;
		GstState pending;
		privateContext->buffering_in_progress = false;   /* stopping pipeline, don't want to change state if GST_MESSAGE_ASYNC_DONE message comes in */
#ifndef INTELCE
		DisconnectCallbacks();
#endif
		if(GST_STATE_CHANGE_FAILURE == gst_element_get_state(privateContext->pipeline, &current, &pending, 0))
		{
			logprintf("AAMPGstPlayer::%s: Pipeline is in FAILURE state : current %s  pending %s", __FUNCTION__,gst_element_state_get_name(current), gst_element_state_get_name(pending));
		}
		gst_element_set_state(this->privateContext->pipeline, GST_STATE_NULL);
		logprintf("AAMPGstPlayer::%s: Pipeline state set to null", __FUNCTION__);
	}
#ifdef AAMP_MPD_DRM
	if(AampOutputProtection::IsAampOutputProcectionInstanceActive())
	{
		AampOutputProtection *pInstance = AampOutputProtection::GetAampOutputProcectionInstance();
		pInstance->setGstElement((GstElement *)(NULL));
		pInstance->Release();
	}
#endif
	TearDownStream(eMEDIATYPE_VIDEO);
	TearDownStream(eMEDIATYPE_AUDIO);
	TearDownStream(eMEDIATYPE_SUBTITLE);
	DestroyPipeline();
	privateContext->rate = AAMP_NORMAL_PLAY_RATE;
	privateContext->lastKnownPTS = 0;
	privateContext->segmentStart = 0;
	privateContext->paused = false;
	privateContext->pipelineState = GST_STATE_NULL;
	logprintf("exiting AAMPGstPlayer_Stop");
}


/**
 * @brief Log the various info related to playback
 */
void AAMPGstPlayer::DumpStatus(void)
{
	GstElement *source = this->privateContext->stream[eMEDIATYPE_VIDEO].source;
	gboolean rcBool;
	guint64 rcUint64;
	gint64 rcInt64;
	GstFormat rcFormat;

	//https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-plugins/html/gst-plugins-base-plugins-appsrc.html
	
	rcBool = 0;
	g_object_get(source, "block", &rcBool, NULL);
	logprintf("\tblock=%d", (int)rcBool); // 0

	rcBool = 0;
	g_object_get(source, "emit-signals", &rcBool, NULL);
	logprintf("\temit-signals=%d", (int)rcBool); // 1

	rcFormat = (GstFormat)0;
	g_object_get(source, "format", &rcFormat, NULL);
	logprintf("\tformat=%d", (int)rcFormat); // 2
	
	rcBool = 0;
	g_object_get(source, "is-live", &rcBool, NULL);
	logprintf("\tis-live=%d", (int)rcBool); // 0
	
	rcUint64 = 0;
	g_object_get(source, "max-bytes", &rcUint64, NULL);
	logprintf("\tmax-bytes=%d", (int)rcUint64); // 200000
	
	rcInt64 = 0;
	g_object_get(source, "max-latency", &rcInt64, NULL);
	logprintf("\tmax-latency=%d", (int)rcInt64); // -1

	rcInt64 = 0;
	g_object_get(source, "min-latency", &rcInt64, NULL);
	logprintf("\tmin-latency=%d", (int)rcInt64); // -1

	rcInt64 = 0;
	g_object_get(source, "size", &rcInt64, NULL);
	logprintf("\tsize=%d", (int)rcInt64); // -1

	gint64 pos, len;
	GstFormat format = GST_FORMAT_TIME;
#ifdef USE_GST1
	if (gst_element_query_position(privateContext->pipeline, format, &pos) &&
		gst_element_query_duration(privateContext->pipeline, format, &len))
#else
	if (gst_element_query_position(privateContext->pipeline, &format, &pos) &&
		gst_element_query_duration(privateContext->pipeline, &format, &len))
#endif
	{
		logprintf("Position: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
			GST_TIME_ARGS(pos), GST_TIME_ARGS(len));
	}
}


/**
 * @brief Validate pipeline state transition within a max timeout
 * @param[in] _this pointer to AAMPGstPlayer instance
 * @param[in] stateToValidate state to be validated
 * @param[in] msTimeOut max timeout in MS
 * @retval Current pipeline state
 */
static GstState validateStateWithMsTimeout( AAMPGstPlayer *_this, GstState stateToValidate, guint msTimeOut)
{
	GstState gst_current;
	GstState gst_pending;
	float timeout = 100.0;
	gint gstGetStateCnt = GST_ELEMENT_GET_STATE_RETRY_CNT_MAX;

	do
	{
		if ((GST_STATE_CHANGE_SUCCESS
				== gst_element_get_state(_this->privateContext->pipeline, &gst_current, &gst_pending, timeout * GST_MSECOND))
				&& (gst_current == stateToValidate))
		{
			GST_WARNING(
					"validateStateWithMsTimeout - PIPELINE gst_element_get_state - SUCCESS : State = %d, Pending = %d",
					gst_current, gst_pending);
			return gst_current;
		}
		g_usleep (msTimeOut * 1000); // Let pipeline safely transition to required state
	}
	while ((gst_current != stateToValidate) && (gstGetStateCnt-- != 0));

	logprintf("validateStateWithMsTimeout - PIPELINE gst_element_get_state - FAILURE : State = %d, Pending = %d",
			gst_current, gst_pending);
	return gst_current;
}


/**
 * @brief Flush the buffers in pipeline
 */
void AAMPGstPlayer::Flush(void)
{
	if (privateContext->pipeline)
	{
		PauseAndFlush(false);
	}
}


/**
 * @brief Pause pipeline and flush 
 * @param playAfterFlush denotes if it should be set to playing at the end
 */
void AAMPGstPlayer::PauseAndFlush(bool playAfterFlush)
{
	aamp->SyncBegin();
	logprintf("Entering AAMPGstPlayer::PauseAndFlush() pipeline state %s",
			gst_element_state_get_name(GST_STATE(privateContext->pipeline)));
	GstStateChangeReturn rc;
	GstState stateBeforeFlush = GST_STATE_PAUSED;
#ifndef USE_PLAYERSINKBIN
	/*On pc, tsdemux requires null transition*/
	stateBeforeFlush = GST_STATE_NULL;
#endif
	rc = gst_element_set_state(this->privateContext->pipeline, stateBeforeFlush);
	if (GST_STATE_CHANGE_ASYNC == rc)
	{
		if (GST_STATE_PAUSED != validateStateWithMsTimeout(this,GST_STATE_PAUSED, 50))
		{
			logprintf("AAMPGstPlayer_Flush - validateStateWithMsTimeout - FAILED GstState %d", GST_STATE_PAUSED);
		}
	}
	else if (GST_STATE_CHANGE_SUCCESS != rc)
	{
		logprintf("AAMPGstPlayer_Flush - gst_element_set_state - FAILED rc %d", rc);
	}
#ifdef USE_GST1
	gboolean ret = gst_element_send_event( GST_ELEMENT(privateContext->pipeline), gst_event_new_flush_start());
	if (!ret) logprintf("AAMPGstPlayer_Flush: flush start error");
	ret = gst_element_send_event(GST_ELEMENT(privateContext->pipeline), gst_event_new_flush_stop(TRUE));
	if (!ret) logprintf("AAMPGstPlayer_Flush: flush stop error");
#else
	for (int iTrack = 0; iTrack < AAMP_TRACK_COUNT; iTrack++)
	{
		media_stream *stream = &this->privateContext->stream[iTrack];
		if (stream->source)
		{
			GstPad* sourceEleSrcPad = gst_element_get_static_pad(stream->source, "src");
			gboolean ret = gst_pad_push_event(sourceEleSrcPad, gst_event_new_flush_start());
			if (!ret) logprintf("AAMPGstPlayer_Flush: flush start error");

			ret = gst_pad_push_event(sourceEleSrcPad, gst_event_new_flush_stop());
			if (!ret) logprintf("AAMPGstPlayer_Flush: flush stop error");

			gst_object_unref(sourceEleSrcPad);
		}
	}
#endif
	if (playAfterFlush)
	{
		rc = gst_element_set_state(this->privateContext->pipeline, GST_STATE_PLAYING);

		if (GST_STATE_CHANGE_ASYNC == rc)
		{
#ifdef AAMP_WAIT_FOR_PLAYING_STATE
			if (GST_STATE_PLAYING != validateStateWithMsTimeout( GST_STATE_PLAYING, 50))
			{
				logprintf("AAMPGstPlayer_Flush - validateStateWithMsTimeout - FAILED GstState %d",
						GST_STATE_PLAYING);
			}
#endif
		}
		else if (GST_STATE_CHANGE_SUCCESS != rc)
		{
			logprintf("AAMPGstPlayer_Flush - gst_element_set_state - FAILED rc %d", rc);
		}
	}
	this->privateContext->total_bytes = 0;
	privateContext->pendingPlayState = false;
	//privateContext->total_duration = 0;
	logprintf("exiting AAMPGstPlayer_FlushEvent");
	aamp->SyncEnd();
}

/**
 * @brief Get playback position in MS
 * @retval playback position in MS
 */
long AAMPGstPlayer::GetPositionMilliseconds(void)
{
	long rc = 0;
	if (privateContext->pipeline == NULL)
	{
		AAMPLOG_ERR("%s(): Pipeline is NULL", __FUNCTION__);
		return rc;
	}

	if (privateContext->positionQuery == NULL)
	{
		AAMPLOG_ERR("%s(): Position query is NULL", __FUNCTION__);
		return rc;
	}

	// Perform gstreamer query and related operation only when pipeline is playing or if deliberately put in paused
	if (privateContext->pipelineState != GST_STATE_PLAYING && !(privateContext->pipelineState == GST_STATE_PAUSED && privateContext->paused))
	{
		AAMPLOG_INFO("%s(): Pipeline is in %s state, returning position as %ld", __FUNCTION__, gst_element_state_get_name(privateContext->pipelineState), rc);
		return rc;
	}

	media_stream* video = &privateContext->stream[eMEDIATYPE_VIDEO];

	// segment.start needs to be queried
	if (privateContext->segmentStart == -1)
	{
		GstQuery *segmentQuery = gst_query_new_segment(GST_FORMAT_TIME);
		// DELIA-39530 - send query to video playbin in pipeline.
		// Special case include trickplay, where only video playbin is active
		if (gst_element_query(video->source, segmentQuery) == TRUE)
		{
			gint64 start;
			gst_query_parse_segment(segmentQuery, NULL, NULL, &start, NULL);
			privateContext->segmentStart = GST_TIME_AS_MSECONDS(start);
			AAMPLOG_WARN("AAMPGstPlayer::%s()%d Segment start: %" G_GINT64_FORMAT, __FUNCTION__, __LINE__, privateContext->segmentStart);
		}
		else
		{
			AAMPLOG_ERR("AAMPGstPlayer::%s()%d segment query failed", __FUNCTION__, __LINE__);
		}
		gst_query_unref(segmentQuery);
	}

	if (gst_element_query(video->sinkbin, privateContext->positionQuery) == TRUE)
	{
		gint64 pos = 0;
		gst_query_parse_position(privateContext->positionQuery, NULL, &pos);

		if (privateContext->segmentStart > 0)
		{
			// DELIA-39530 - Deduct segment.start to find the actual time of media that's played.
			rc = (GST_TIME_AS_MSECONDS(pos) - privateContext->segmentStart) * privateContext->rate;
		}
		else
		{
			rc = GST_TIME_AS_MSECONDS(pos) * privateContext->rate;
		}
		//AAMPLOG_WARN("AAMPGstPlayer::%s()%d pos - %" G_GINT64_FORMAT " rc - %ld", __FUNCTION__, __LINE__, GST_TIME_AS_MSECONDS(pos), rc);

		//positionQuery is not unref-ed here, because it could be reused for future position queries
	}
	return rc;
}


/**
 * @brief To pause/play pipeline
 * @param[in] Pause flag to pause/play the pipeline
 * @retval true if content successfully paused
 */
bool AAMPGstPlayer::Pause( bool pause )
{
	bool retValue = true;

	aamp->SyncBegin();

	logprintf("entering AAMPGstPlayer_Pause");

	if (privateContext->pipeline != NULL)
	{
		GstState nextState = pause ? GST_STATE_PAUSED : GST_STATE_PLAYING;
		gst_element_set_state(this->privateContext->pipeline, nextState);
		privateContext->buffering_target_state = nextState;
		privateContext->paused = pause;
	}
	else
	{
		logprintf("%s(): Pipeline is NULL", __FUNCTION__);
		retValue = false;
	}

#if 0
	GstStateChangeReturn rc;
	for (int iTrack = 0; iTrack < AAMP_TRACK_COUNT; iTrack++)
	{
		media_stream *stream = &privateContext->stream[iTrack];
		if (stream->source)
		{
			rc = gst_element_set_state(privateContext->stream->sinkbin, GST_STATE_PAUSED);
		}
	}
#endif

	aamp->SyncEnd();

	return retValue;
}


/**
 * @brief Set video display rectangle co-ordinates
 * @param[in] x x co-ordinate of display rectangle
 * @param[in] y y co-ordinate of display rectangle
 * @param[in] w width of display rectangle
 * @param[in] h height of display rectangle
 */
void AAMPGstPlayer::SetVideoRectangle(int x, int y, int w, int h)
{
	media_stream *stream = &privateContext->stream[eMEDIATYPE_VIDEO];
	sprintf(privateContext->videoRectangle, "%d,%d,%d,%d", x,y,w,h);
	logprintf("SetVideoRectangle :: Rect %s, using_playersinkbin = %d, video_sink =%p",
			privateContext->videoRectangle, stream->using_playersinkbin, privateContext->video_sink);
	if (aamp->mEnableRectPropertyEnabled) //As part of DELIA-37804
	{
		if (stream->using_playersinkbin)
		{
			g_object_set(stream->sinkbin, "rectangle", privateContext->videoRectangle, NULL);
		}
#ifndef INTELCE
		else if (privateContext->video_sink)
		{
			g_object_set(privateContext->video_sink, "rectangle", privateContext->videoRectangle, NULL);
		}
#else
#if defined(INTELCE_USE_VIDRENDSINK)
		else if (privateContext->video_pproc)
		{
			g_object_set(privateContext->video_pproc, "rectangle", privateContext->videoRectangle, NULL);
		}
#else
		else if (privateContext->video_sink)
		{
			g_object_set(privateContext->video_sink, "rectangle", privateContext->videoRectangle, NULL);
		}
#endif
#endif	
		else
		{
			AAMPLOG_WARN("[%s] Scaling not possible at this time",__FUNCTION__);
			privateContext->gstPropsDirty = true;
		}
	}
	else
	{
		AAMPLOG_WARN("SetVideoRectangle ignored since westerossink is used");
	}
}


/**
 * @brief Set video zoom
 * @param[in] zoom zoom setting to be set
 */
void AAMPGstPlayer::SetVideoZoom(VideoZoomMode zoom)
{
	media_stream *stream = &privateContext->stream[eMEDIATYPE_VIDEO];
	AAMPLOG_INFO("SetVideoZoom :: ZoomMode %d, using_playersinkbin = %d, video_sink =%p",
			zoom, stream->using_playersinkbin, privateContext->video_sink);

	privateContext->zoom = zoom;
	if (stream->using_playersinkbin && stream->sinkbin)
	{
		g_object_set(stream->sinkbin, "zoom", zoom, NULL);
	}
#ifndef INTELCE
	else if (privateContext->video_sink)
	{
		g_object_set(privateContext->video_sink, "zoom-mode", VIDEO_ZOOM_FULL == zoom ? 0 : 1, NULL);
	}
#elif defined(INTELCE_USE_VIDRENDSINK)
	else if (privateContext->video_pproc)
	{
		g_object_set(privateContext->video_pproc, "scale-mode", VIDEO_ZOOM_FULL == zoom ? 0 : 3, NULL);
	}
#else
	else if (privateContext->video_sink)
	{
		g_object_set(privateContext->video_sink, "scale-mode", VIDEO_ZOOM_FULL == zoom ? 0 : 3, NULL);
	}
#endif
	else
	{
		privateContext->gstPropsDirty = true;
	}
}


/**
 * @brief Set video mute
 * @param[in] muted true to mute video otherwise false
 */
void AAMPGstPlayer::SetVideoMute(bool muted)
{
	media_stream *stream = &privateContext->stream[eMEDIATYPE_VIDEO];
	AAMPLOG_INFO("%s: muted %d, using_playersinkbin = %d, video_sink =%p", __FUNCTION__, muted, stream->using_playersinkbin, privateContext->video_sink);

	privateContext->videoMuted = muted;
	if (stream->using_playersinkbin && stream->sinkbin)
	{
		g_object_set(stream->sinkbin, "video-mute", privateContext->videoMuted, NULL);
	}
	else if (privateContext->video_sink)
	{
#ifndef INTELCE
		g_object_set(privateContext->video_sink, "show-video-window", !privateContext->videoMuted, NULL);
#else
		g_object_set(privateContext->video_sink, "mute", privateContext->videoMuted, NULL);
#endif
	}
	else
	{
		privateContext->gstPropsDirty = true;
	}
}


/**
 * @brief Set audio volume
 * @param[in] volume audio volume value (0-100)
 */
void AAMPGstPlayer::SetAudioVolume(int volume)
{
	privateContext->audioVolume = volume / 100.0;

	setVolumeOrMuteUnMute();
}


/**
 * @brief Set audio volume or mute
 * @note set privateContext->audioVolume before calling this function
 */
void AAMPGstPlayer::setVolumeOrMuteUnMute(void)
{
	GstElement *gSource = NULL;
	char *propertyName = NULL;
	media_stream *stream = &privateContext->stream[eMEDIATYPE_VIDEO];

	AAMPLOG_INFO("AAMPGstPlayer::%s() %d > volume = %f, using_playersinkbin = %d, audio_sink = %p", __FUNCTION__, __LINE__, privateContext->audioVolume, stream->using_playersinkbin, privateContext->audio_sink);

	if (stream->using_playersinkbin && stream->sinkbin)
	{
		gSource = stream->sinkbin;
		propertyName = (char*)"audio-mute";
	}
	else if (privateContext->audio_sink)
	{
		gSource = privateContext->audio_sink;
		propertyName = (char*)"mute";
	}
	else
	{
		privateContext->gstPropsDirty = true;
		return; /* Return here if the sinkbin or audio_sink is not valid, no need to proceed further */
	}

	/* Muting the audio decoder in general to avoid audio passthrough in expert mode for locked channel */
	if (0 == privateContext->audioVolume)
	{
		logprintf("AAMPGstPlayer::%s() %d > Audio Muted", __FUNCTION__, __LINE__);
#ifdef INTELCE
		if (!stream->using_playersinkbin)
		{
			logprintf("AAMPGstPlayer::%s() %d > Setting input-gain to %f", __FUNCTION__, __LINE__, INPUT_GAIN_DB_MUTE);
			g_object_set(privateContext->audio_sink, "input-gain", INPUT_GAIN_DB_MUTE, NULL);
		}
		else
#endif
		{
			g_object_set(gSource, propertyName, true, NULL);
		}
		privateContext->audioMuted = true;
	}
	else
	{
		if (privateContext->audioMuted)
		{
			logprintf("AAMPGstPlayer::%s() %d > Audio Unmuted after a Mute", __FUNCTION__, __LINE__);
#ifdef INTELCE
			if (!stream->using_playersinkbin)
			{
				logprintf("AAMPGstPlayer::%s() %d > Setting input-gain to %f", __FUNCTION__, __LINE__, INPUT_GAIN_DB_UNMUTE);
				g_object_set(privateContext->audio_sink, "input-gain", INPUT_GAIN_DB_UNMUTE, NULL);
			}
			else
#endif
			{
				g_object_set(gSource, propertyName, false, NULL);
			}
			privateContext->audioMuted = false;
		}
		
		logprintf("AAMPGstPlayer::%s %d > Setting Volume %f",	__FUNCTION__, __LINE__, privateContext->audioVolume);
		g_object_set(gSource, "volume", privateContext->audioVolume, NULL);
	}
}


/**
 * @brief Flush cached GstBuffers and set seek position & rate
 * @param[in] position playback seek position
 * @param[in] rate playback rate
 * @param[in] shouldTearDown flag indicates if pipeline should be destroyed if in invalid state
 */
void AAMPGstPlayer::Flush(double position, int rate, bool shouldTearDown)
{
	media_stream *stream = &privateContext->stream[eMEDIATYPE_VIDEO];
	privateContext->rate = rate;
	privateContext->stream[eMEDIATYPE_VIDEO].bufferUnderrun = false;
	privateContext->stream[eMEDIATYPE_AUDIO].bufferUnderrun = false;

	if (privateContext->eosCallbackIdleTaskPending)
	{
		logprintf("AAMPGstPlayer::%s:%d Remove eosCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->eosCallbackIdleTaskId);
		g_source_remove(privateContext->eosCallbackIdleTaskId);
		privateContext->eosCallbackIdleTaskId = 0;
		privateContext->eosCallbackIdleTaskPending = false;
	}

	if (privateContext->ptsCheckForEosOnUnderflowIdleTaskId)
	{
		logprintf("AAMPGstPlayer::%s:%d Remove ptsCheckForEosCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->ptsCheckForEosOnUnderflowIdleTaskId);
		g_source_remove(privateContext->ptsCheckForEosOnUnderflowIdleTaskId);
		privateContext->ptsCheckForEosOnUnderflowIdleTaskId = 0;
	}

	if (privateContext->bufferingTimeoutTimerId)
	{
		logprintf("AAMPGstPlayer::%s:%d Remove bufferingTimeoutTimerId %d", __FUNCTION__, __LINE__, privateContext->bufferingTimeoutTimerId);
		g_source_remove(privateContext->bufferingTimeoutTimerId);
		privateContext->bufferingTimeoutTimerId = 0;
	}

	if (stream->using_playersinkbin)
	{
		Flush();
	}
	else
	{
		if (privateContext->pipeline == NULL)
		{
			logprintf("AAMPGstPlayer::%s:%d Pipeline is NULL", __FUNCTION__, __LINE__);
			return;
		}

		//Check if pipeline is in playing/paused state. If not flush doesn't work
		GstState current, pending;
		bool bPauseNeeded = false;

		gst_element_get_state(privateContext->pipeline, &current, &pending, 100 * GST_MSECOND);

		if (current != GST_STATE_PLAYING && current != GST_STATE_PAUSED)
		{
			if (shouldTearDown)
			{
				logprintf("AAMPGstPlayer::%s:%d Pipeline is not in playing/paused state, hence resetting it", __FUNCTION__, __LINE__);
				Stop(true);
			}
			return;
		}
		else
		{
			logprintf("AAMPGstPlayer::%s:%d Pipeline is in %s state position %f", __FUNCTION__, __LINE__, gst_element_state_get_name(current), position);

			if ((aamp->getStreamType() < 20) && (current == GST_STATE_PAUSED))
			{
				/*
				 * Changing the Pipeline state to GST_STATE_PLAYING temporarily to keep Gstreamer continue sending data to Decoder during gst_element_seek().
				 * Reason : Because if Pipeline is in GST_STATE_PAUSED state then the Gstreamer will stop sending data to the decoder during gst_element_seek() call.
				 * In that case while doing PageUp/Down after Pause enter into video buffering logic; and querying video decoder status for buffered bytes (or)
				 * frames result in 0 count; that results internal retune during Video Buffering.
				 */
				logprintf("AAMPGstPlayer::%s:%d Pipeline state change ( PAUSED -> PLAYING )", __FUNCTION__, __LINE__);

				if (gst_element_set_state(privateContext->pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
				{
					logprintf("AAMPGstPlayer::%s:%d Pipeline state change (PLAYING) failed", __FUNCTION__, __LINE__);
				}
				else
				{
					bPauseNeeded = true;
				}
			}
		}

		for (int i = 0; i < AAMP_TRACK_COUNT; i++)
		{
			privateContext->stream[i].resetPosition = true;
			privateContext->stream[i].flush = true;
			privateContext->stream[i].eosReached = false;
		}

		AAMPLOG_INFO("AAMPGstPlayer::%s:%d Pipeline flush seek - start = %f", __FUNCTION__, __LINE__, position);

		if (!gst_element_seek(privateContext->pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET,
				position * GST_SECOND, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
		{
			logprintf("Seek failed");
		}

		if (bPauseNeeded)
		{
			/* Reseting Pipeline state to Paused from Playing */
			logprintf("AAMPGstPlayer::%s:%d Pipeline state change ( PLAYING -> PAUSED )", __FUNCTION__, __LINE__, gst_element_state_get_name(current), position);

			if (gst_element_set_state(privateContext->pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
			{
				logprintf("AAMPGstPlayer::%s:%d Pipeline state change (PAUSED) failed", __FUNCTION__, __LINE__);
			}
		}

	}
	privateContext->eosSignalled = false;
	privateContext->numberOfVideoBuffersSent = 0;
}


/**
 * @brief Process discontinuity for a stream type
 * @param type stream type
 * @retval true if discontinuity processed
 */
bool AAMPGstPlayer::Discontinuity(MediaType type)
{
	bool ret = false;
	logprintf("Entering AAMPGstPlayer::%s type %d", __FUNCTION__, (int)type);
	media_stream *stream = &privateContext->stream[type];
	/*Handle discontinuity only if atleast one buffer is pushed*/
	if (stream->format != FORMAT_NONE && stream->resetPosition == true)
	{
		logprintf("%s(): Discontinuity received before first buffer - ignoring", __FUNCTION__);
	}
	else
	{
		traceprintf("%s(): stream->format %d, stream->resetPosition %d, stream->flush %d", __FUNCTION__,stream->format , stream->resetPosition, stream->flush);
		AAMPGstPlayer_SignalEOS(stream->source);
		// We are in buffering, but we received discontinuity, un-pause pipeline
		StopBuffering(true);
		ret = true;
	}
	return ret;
}

/**
 * @brief Check if PTS is changing
 * @retval true if PTS changed from lastKnown PTS, will optimistically return true
 * 			if video-pts attribute is not available from decoder
 */
bool AAMPGstPlayer::CheckForPTSChange()
{
	bool ret = true;
#ifndef INTELCE
	gint64 currentPTS = 0;
	if (privateContext->video_dec)
	{
		g_object_get(privateContext->video_dec, "video-pts", &currentPTS, NULL);
	}
	if (currentPTS != 0)
	{
		if (currentPTS != privateContext->lastKnownPTS)
		{
			AAMPLOG_WARN("AAMPGstPlayer::%s():%d There is an update in PTS prevPTS:%" G_GINT64_FORMAT " newPTS: %" G_GINT64_FORMAT,
							__FUNCTION__, __LINE__, privateContext->lastKnownPTS, currentPTS);
			privateContext->ptsUpdatedTimeMS = NOW_STEADY_TS_MS;
			privateContext->lastKnownPTS = currentPTS;
		}
		else
		{
			ret = false;
		}
	}
	else
	{
		AAMPLOG_WARN("AAMPGstPlayer::%s():%d video-pts parsed is: %" G_GINT64_FORMAT,
			__FUNCTION__, __LINE__, currentPTS);
	}
#endif
	return ret;
}

/**
 * @brief Gets Video PTS
 * @param none
 * @retval Video PTS value
 */
long long AAMPGstPlayer::GetVideoPTS(void)
{
	gint64 currentPTS = 0;
	if (privateContext->video_dec)
	{
		g_object_get(privateContext->video_dec, "video-pts", &currentPTS, NULL);
		//Westeros sink sync returns PTS in 90Khz format where as BCM returns in 45 KHz, 
		// hence converting to 90Khz for BCM
		if(!privateContext->using_westerossink)
		{
			currentPTS = currentPTS * 2; // convert from 45 KHz to 90 Khz PTS
		}
	}

	return (long long) currentPTS;
}

/**
 * @brief Check if cache empty for a media type
 * @param[in] mediaType stream type
 * @retval true if cache empty
 */
bool AAMPGstPlayer::IsCacheEmpty(MediaType mediaType)
{
	bool ret = true;
#ifdef USE_GST1
	media_stream *stream = &privateContext->stream[mediaType];
	if (stream->source)
	{
		guint64 cacheLevel = gst_app_src_get_current_level_bytes (GST_APP_SRC(stream->source));
		if(0 != cacheLevel)
		{
			traceprintf("AAMPGstPlayer::%s():%d Cache level  %" G_GUINT64_FORMAT, __FUNCTION__, __LINE__, cacheLevel);
			ret = false;
		}
		else
		{
			// Changed from logprintf to traceprintf, to avoid log flooding (seen on xi3 and xid).
			// We're seeing this logged frequently during live linear playback, despite no user-facing problem.
			traceprintf("AAMPGstPlayer::%s():%d Cache level empty", __FUNCTION__, __LINE__);
			if (privateContext->stream[eMEDIATYPE_VIDEO].bufferUnderrun == true ||
					privateContext->stream[eMEDIATYPE_AUDIO].bufferUnderrun == true)
			{
				logprintf("AAMPGstPlayer::%s():%d Received buffer underrun signal for video(%d) or audio(%d) previously",
					__FUNCTION__, __LINE__, privateContext->stream[eMEDIATYPE_VIDEO].bufferUnderrun,
					privateContext->stream[eMEDIATYPE_AUDIO].bufferUnderrun);
			}
#ifndef INTELCE
			else
			{
				bool ptsChanged = CheckForPTSChange();
				if(!ptsChanged)
				{
					long long deltaMS = NOW_STEADY_TS_MS - privateContext->ptsUpdatedTimeMS;
					if (deltaMS <= AAMP_MIN_PTS_UPDATE_INTERVAL)
					{
						//Timeout hasn't expired. Need to wait for PTS min update interval to expire
						ret = false;
					}
					else
					{
						logprintf("AAMPGstPlayer::%s():%d Appsrc cache is empty and PTS hasn't been updated for: %lldms and ret(%d)",
								__FUNCTION__, __LINE__, deltaMS, ret);
					}
				}
				else
				{
					ret = false;
				}
			}
#endif
		}
	}
#endif
	return ret;
}

/**
 * @brief Set pipeline to PLAYING state once fragment caching is complete
 */
void AAMPGstPlayer::NotifyFragmentCachingComplete()
{
	if(privateContext->pendingPlayState)
	{
		logprintf("AAMPGstPlayer::%s():%d Setting pipeline to PLAYING state ", __FUNCTION__, __LINE__);
		if (gst_element_set_state(privateContext->pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
		{
			logprintf("AAMPGstPlayer_Configure GST_STATE_PLAYING failed");
		}
		privateContext->pendingPlayState = false;
	}
	else
	{
		logprintf("AAMPGstPlayer::%s():%d No pending PLAYING state", __FUNCTION__, __LINE__);
	}
}


/**
 * @brief Get video display's width and height
 * @param[in] width video width
 * @param[in] height video height
 */
void AAMPGstPlayer::GetVideoSize(int &width, int &height)
{
	int x, y, w, h;
	sscanf(privateContext->videoRectangle, "%d,%d,%d,%d", &x, &y, &w, &h);
	if (w > 0 && h > 0)
	{
		width = w;
		height = h;
	}
}


/**
 * @brief Increase the rank of AAMP decryptor plugins
 */
void AAMPGstPlayer::InitializeAAMPGstreamerPlugins()
{
#ifdef AAMP_MPD_DRM
	GstRegistry* registry = gst_registry_get();

	GstPluginFeature* pluginFeature = gst_registry_lookup_feature(registry, GstPluginNamePR);

	if (pluginFeature == NULL)
	{
		AAMPLOG_ERR("AAMPGstPlayer::%s():%d %s plugin feature not available; reloading aamp plugin", __FUNCTION__, __LINE__, GstPluginNamePR);
		GstPlugin * plugin = gst_plugin_load_by_name ("aamp");
		if(plugin)
		{
			gst_object_unref(plugin);
		}
		pluginFeature = gst_registry_lookup_feature(registry, GstPluginNamePR);
		if(pluginFeature == NULL)
			AAMPLOG_ERR("AAMPGstPlayer::%s():%d %s plugin feature not available", __FUNCTION__, __LINE__, GstPluginNamePR);
	}
	if(pluginFeature)
	{
		gst_registry_remove_feature (registry, pluginFeature);//Added as a work around to handle DELIA-31716
		gst_registry_add_feature (registry, pluginFeature);


		AAMPLOG_WARN("AAMPGstPlayer::%s():%d %s plugin priority set to GST_RANK_PRIMARY + 111", __FUNCTION__, __LINE__, GstPluginNamePR);
		gst_plugin_feature_set_rank(pluginFeature, GST_RANK_PRIMARY + 111);
		gst_object_unref(pluginFeature);
	}

	pluginFeature = gst_registry_lookup_feature(registry, GstPluginNameWV);

	if (pluginFeature == NULL)
	{
		AAMPLOG_ERR("AAMPGstPlayer::%s():%d %s plugin feature not available", __FUNCTION__, __LINE__, GstPluginNameWV);
	}
	else
	{
		AAMPLOG_WARN("AAMPGstPlayer::%s():%d %s plugin priority set to GST_RANK_PRIMARY + 111", __FUNCTION__, __LINE__, GstPluginNameWV);
		gst_plugin_feature_set_rank(pluginFeature, GST_RANK_PRIMARY + 111);
		gst_object_unref(pluginFeature);
	}

	pluginFeature = gst_registry_lookup_feature(registry, GstPluginNameCK);

	if (pluginFeature == NULL)
	{
		AAMPLOG_ERR("AAMPGstPlayer::%s():%d %s plugin feature not available", __FUNCTION__, __LINE__, GstPluginNameCK);
	}
	else
	{
		AAMPLOG_WARN("AAMPGstPlayer::%s():%d %s plugin priority set to GST_RANK_PRIMARY + 111", __FUNCTION__, __LINE__, GstPluginNameCK);
		gst_plugin_feature_set_rank(pluginFeature, GST_RANK_PRIMARY + 111);
		gst_object_unref(pluginFeature);
	}
#endif
}


/**
 * @brief Notify EOS to core aamp asynchronously if required.
 * @note Used internally by AAMPGstPlayer
 */
void AAMPGstPlayer::NotifyEOS()
{
	if (!privateContext->eosSignalled)
	{
		if (!privateContext->eosCallbackIdleTaskPending)
		{
			privateContext->eosCallbackIdleTaskPending = true;
			privateContext->eosCallbackIdleTaskId = g_idle_add(IdleCallbackOnEOS, this);
			if (!privateContext->eosCallbackIdleTaskPending)
			{
				logprintf("%s:%d eosCallbackIdleTask already finished, reset id", __FUNCTION__, __LINE__);
				privateContext->eosCallbackIdleTaskId = 0;
			}
			else
			{
				logprintf("%s:%d eosCallbackIdleTask scheduled, eosCallbackIdleTaskId %d", __FUNCTION__, __LINE__, privateContext->eosCallbackIdleTaskId);
			}
		}
		else
		{
			logprintf("%s()%d: IdleCallbackOnEOS already registered previously, hence skip!", __FUNCTION__, __LINE__);
		}
		privateContext->eosSignalled = true;
	}
	else
	{
		logprintf("%s()%d: EOS already signaled, hence skip!", __FUNCTION__, __LINE__);
	}
}

/**
 * @brief Dump a file to log
 *
 * @param fileName file name
 */
static void DumpFile(const char* fileName)
{
	int c;
	FILE *fp = fopen(fileName, "r");
	if (fp)
	{
		printf("\n************************Dump %s **************************\n", fileName);
		c = getc(fp);
		while (c != EOF)
		{
			printf("%c", c);
			c = getc(fp);
		}
		fclose(fp);
		printf("\n**********************Dump %s end *************************\n", fileName);
	}
	else
	{
		logprintf("%s:%d: Could not open %s", __FUNCTION__, __LINE__, fileName);
	}
}

/**
 * @brief Dump diagnostic information
 *
 */
void AAMPGstPlayer::DumpDiagnostics()
{
	logprintf("%s:%d video_dec %p audio_dec %p video_sink %p audio_sink %p numberOfVideoBuffersSent %d", __FUNCTION__,
			__LINE__, privateContext->video_dec, privateContext->audio_dec, privateContext->video_sink,
			privateContext->audio_sink, privateContext->numberOfVideoBuffersSent);
#ifndef INTELCE
	DumpFile("/proc/brcm/transport");
	DumpFile("/proc/brcm/video_decoder");
	DumpFile("/proc/brcm/audio");
#endif
}

/**
 *   @brief Signal trick mode discontinuity to gstreamer pipeline
 *
 */
void AAMPGstPlayer::SignalTrickModeDiscontinuity()
{
	media_stream* stream = &privateContext->stream[eMEDIATYPE_VIDEO];
	if (stream && (privateContext->rate != AAMP_NORMAL_PLAY_RATE) )
	{
		GstPad* sourceEleSrcPad = gst_element_get_static_pad(GST_ELEMENT(stream->source), "src");
		GstStructure * eventStruct = gst_structure_new("aamp-tm-disc", "fps", G_TYPE_UINT, (guint)gpGlobalConfig->vodTrickplayFPS, NULL);
		if (!gst_pad_push_event(sourceEleSrcPad, gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, eventStruct)))
		{
			logprintf("%s:%d Error on sending aamp-tm-disc", __FUNCTION__, __LINE__);
		}
		else
		{
			logprintf("%s:%d Sent aamp-tm-disc event", __FUNCTION__, __LINE__);
		}
		gst_object_unref(sourceEleSrcPad);
	}
}

void AAMPGstPlayer::SeekStreamSink(double position, double rate)
{
	// shouldTearDown is set to false, because in case of a new tune pipeline
	// might not be in a playing/paused state which causes Flush() to destroy
	// pipeline. This has to be avoided.
	Flush(position, rate, false);

	// Flushing seek will flush buffers in pipeline
	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		privateContext->stream[i].flush = false;
	}
}

/**
 *   @brief Get the video rectangle co-ordinates
 *
 */
std::string AAMPGstPlayer::GetVideoRectangle()
{
	return std::string(privateContext->videoRectangle);
}


/**
 * @brief Un-pause pipeline and notify buffer end event to player.
 *
 * @param[in] forceStop - true to force end buffering
 */
void AAMPGstPlayer::StopBuffering(bool forceStop)
{
	pthread_mutex_lock(&mBufferingLock);
	//Check if we are in buffering
	if (gpGlobalConfig->reportBufferEvent && privateContext->video_dec && aamp->GetBufUnderFlowStatus())
	{
		bool stopBuffering = forceStop;
#if ( !defined(INTELCE) && !defined(RPI) && !defined(__APPLE__) )
		uint bytes = 0, frames = DEFAULT_BUFFERING_QUEUED_FRAMES_MIN+1;
	        g_object_get(privateContext->video_dec,"buffered_bytes",&bytes,NULL);
	        g_object_get(privateContext->video_dec,"queued_frames",&frames,NULL);
		stopBuffering = stopBuffering || (bytes > DEFAULT_BUFFERING_QUEUED_BYTES_MIN) || (frames > DEFAULT_BUFFERING_QUEUED_FRAMES_MIN); //TODO: the minimum byte and frame values should be configurable from aamp.cfg
#else
		stopBuffering = true;
#endif
		if (stopBuffering)
		{
			if( true != aamp->PausePipeline(false) )
			{
				AAMPLOG_ERR("%s(): Failed to un-pause pipeline for stop buffering!", __FUNCTION__);
			}
			else
			{
				aamp->SendBufferChangeEvent();
			}
	        }
		else
		{
#if ( !defined(INTELCE) && !defined(RPI) && !defined(__APPLE__) )
			AAMPLOG_WARN("%s:%d Not enough data available to stop buffering, bytes %u, frames %u !", __FUNCTION__, __LINE__, bytes, frames);
#endif
		}
	}
	pthread_mutex_unlock(&mBufferingLock);
}

void type_check_instance(char * str, GstElement * elem)
{
	logprintf("%s %p type_check %d", str, elem, G_TYPE_CHECK_INSTANCE (elem));
}
/**
 * @}
 */

