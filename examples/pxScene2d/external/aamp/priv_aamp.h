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
 * @file priv_aamp.h
 * @brief Private functions and types used internally by AAMP
 */

#ifndef PRIVAAMP_H
#define PRIVAAMP_H

#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include "main_aamp.h"
#include <curl/curl.h>
#include <string.h> // for memset
#include <glib.h>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <map>
#include <set>
#include <list>
#include <sstream>
#include <mutex>
#include <VideoStat.h>
#include <queue>

#ifdef __APPLE__
#define aamp_pthread_setname(tid,name) pthread_setname_np(name)
#else
#define aamp_pthread_setname(tid,name) pthread_setname_np(tid,name)
#endif

#ifdef WIN32
#define AAMP_PACKED
#else
#define AAMP_PACKED __attribute__((__packed__))
#endif

#define MAX_URI_LENGTH (2048)           /**< Increasing size to include longer urls */
#define AAMP_TRACK_COUNT 2              /**< internal use - audio+video track */
#define AAMP_DRM_CURL_COUNT 2           /**< audio+video track DRMs */
#define AAMP_DAI_CURL_COUNT 1           /**< Download Ad manifest */
#define AAMP_DAI_CURL_IDX (AAMP_TRACK_COUNT + AAMP_DRM_CURL_COUNT)                                /**< CURL Index for DAI */
#define MAX_CURL_INSTANCE_COUNT (AAMP_TRACK_COUNT + AAMP_DRM_CURL_COUNT + AAMP_DAI_CURL_COUNT)    /**< Maximum number of CURL instances */
#define AAMP_MAX_PIPE_DATA_SIZE 1024    /**< Max size of data send across pipe */
#define AAMP_LIVE_OFFSET 15             /**< Live offset in seconds */
#define AAMP_CDVR_LIVE_OFFSET 30 	/**< Live offset in seconds for CDVR hot recording */
#define CURL_FRAGMENT_DL_TIMEOUT 10L     /**< Curl timeout for fragment download */
#define DEFAULT_CURL_TIMEOUT 5L         /**< Default timeout for Curl downloads */
#define DEFAULT_CURL_CONNECTTIMEOUT 3L  /**< Curl socket connection timeout */
#define EAS_CURL_TIMEOUT 3L             /**< Curl timeout for EAS manifest downloads */
#define EAS_CURL_CONNECTTIMEOUT 2L      /**< Curl timeout for EAS connection */
#define DEFAULT_INTERVAL_BETWEEN_PLAYLIST_UPDATES_MS (6*1000)   /**< Interval between playlist refreshes */
#define TRICKPLAY_NETWORK_PLAYBACK_FPS 4            /**< Frames rate for trickplay from CDN server */
#define TRICKPLAY_TSB_PLAYBACK_FPS 8                /**< Frames rate for trickplay from TSB */
#define DEFAULT_INIT_BITRATE     2500000            /**< Initial bitrate: 2.5 mb - for non-4k playback */
#define DEFAULT_INIT_BITRATE_4K 13000000            /**< Initial bitrate for 4K playback: 13mb ie, 3/4 profile */
#define DEFAULT_MINIMUM_CACHE_VOD_SECONDS  0        /**< Default cache size of VOD playback */
#define BITRATE_ALLOWED_VARIATION_BAND 500000       /**< NW BW change beyond this will be ignored */
#define AAMP_ABR_THRESHOLD_SIZE 50000               /**< ABR threshold size - 50K */
#define DEFAULT_ABR_CACHE_LIFE 5000                 /**< Default ABR cache life */
#define DEFAULT_ABR_CACHE_LENGTH 3                  /**< Default ABR cache length */
#define DEFAULT_ABR_OUTLIER 5000000                 /**< ABR outlier: 5 MB */
#define DEFAULT_ABR_SKIP_DURATION 6                 /**< Initial skip duration of ABR - 6 sec */
#define DEFAULT_ABR_NW_CONSISTENCY_CNT 2            /**< ABR network consistency count */
#define MAX_SEG_DOWNLOAD_FAIL_COUNT 10              /**< Max segment download failures to identify a playback failure. */
#define MAX_AD_SEG_DOWNLOAD_FAIL_COUNT 2            /**< Max Ad segment download failures to identify as the ad playback failure. */
#define MAX_SEG_DRM_DECRYPT_FAIL_COUNT 10           /**< Max segment decryption failures to identify a playback failure. */
#define MAX_SEG_INJECT_FAIL_COUNT 10                /**< Max segment injection failure to identify a playback failure. */
#define DEF_LICENSE_REQ_RETRY_WAIT_TIME 500			/**< Wait time in milliseconds before retrying for DRM license */

#define DEFAULT_CACHED_FRAGMENTS_PER_TRACK  3       /**< Default cached fragements per track */
#define DEFAULT_BUFFER_HEALTH_MONITOR_DELAY 10
#define DEFAULT_BUFFER_HEALTH_MONITOR_INTERVAL 5

#define DEFAULT_STALL_ERROR_CODE (7600)             /**< Default stall error code: 7600 */
#define DEFAULT_STALL_DETECTION_TIMEOUT (10000)     /**< Stall detection timeout: 10sec */
#define FRAGMENT_DOWNLOAD_WARNING_THRESHOLD 2000    /**< MAX Fragment download threshold time in Msec*/

#define DEFAULT_REPORT_PROGRESS_INTERVAL (1000)     /**< Progress event reporting interval: 1sec */
#define NOW_SYSTEM_TS_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()     /**< Getting current system clock in milliseconds */
#define NOW_STEADY_TS_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()     /**< Getting current steady clock in milliseconds */

#define AAMP_SEEK_TO_LIVE_POSITION (-1)

#define MAX_PTS_ERRORS_THRESHOLD 4

#define MANIFEST_TEMP_DATA_LENGTH 100				/**< Manifest temp data length */

#define AAMP_USERAGENT_SUFFIX		"AAMP/2.0.0"    /**< Version string of AAMP Player */
#define AAMP_USERAGENT_BASE_STRING	"Mozilla/5.0 (Linux; x86_64 GNU/Linux) AppleWebKit/601.1 (KHTML, like Gecko) Version/8.0 Safari/601.1 WPE"	/**< Base User agent string,it will be appneded with AAMP_USERAGENT_SUFFIX */
#define AAMP_USER_AGENT_MAX_CONFIG_LEN  512    /**< Max Chars allowed in aamp.cfg for user-agent */
// HLS CDVR/VOD playlist size for 1hr -> 225K , 2hr -> 450-470K , 3hr -> 670K . Most played CDVR/Vod < 2hr
#define MAX_PLAYLIST_CACHE_SIZE    (3*1024*1024) // Approx 3MB -> 2 video profiles + one audio profile + one iframe profile, 500-700K MainManifest
#define DEFAULT_WAIT_TIME_BEFORE_RETRY_HTTP_5XX_MS (1000)    /**< Wait time in milliseconds before retry for 5xx errors */

//Upper and lower limit for dash drm sessions
#define MIN_DASH_DRM_SESSIONS 2
#define MAX_DASH_DRM_SESSIONS 30

// VSS Service Zone identifier in url 
#define VSS_MARKER			"?sz="
#define VSS_MARKER_FOG		"\%3Fsz\%3D"


//#define PLACEMENT_EMULATION 1    //Only for Dev testing. Can remove later.
/*1 for debugging video track, 2 for audio track and 3 for both*/
/*#define AAMP_DEBUG_FETCH_INJECT 0x01*/


/**
 * @brief Structure of GrowableBuffer
 */
struct GrowableBuffer
{
	char *ptr;      /**< Pointer to buffer's memory location */
	size_t len;     /**< Buffer size */
	size_t avail;   /**< Available buffer size */
};

/**
 * @brief Enumeration for TUNED Event Configuration
 */
enum TunedEventConfig
{
	eTUNED_EVENT_ON_PLAYLIST_INDEXED,           /**< Send TUNED event after playlist indexed*/
	eTUNED_EVENT_ON_FIRST_FRAGMENT_DECRYPTED,    /**< Send TUNED event after first fragment decryption*/
	eTUNED_EVENT_ON_GST_PLAYING,                /**< Send TUNED event on gstreamer's playing event*/
        eTUNED_EVENT_MAX
};

/**
 * @brief Playback Error Type
 */
enum PlaybackErrorType
{
	eGST_ERROR_PTS,                 /**< PTS error from gstreamer */
	eGST_ERROR_UNDERFLOW,           /**< Underflow error from gstreamer */
	eGST_ERROR_VIDEO_BUFFERING,     /**< Video buffering error */
	eDASH_ERROR_STARTTIME_RESET     /**< Start time reset of DASH */
};

/**
 * @brief Tune Typea
 */
enum TuneType
{
	eTUNETYPE_NEW_NORMAL,   /**< Play from live point for live streams, from start for VOD*/
	eTUNETYPE_NEW_SEEK,     /**< A new tune with valid seek position*/
	eTUNETYPE_SEEK,         /**< Seek to a position. Not a new channel, so resources can be reused*/
	eTUNETYPE_SEEKTOLIVE,   /**< Seek to live point. Not a new channel, so resources can be reused*/
	eTUNETYPE_RETUNE,       /**< Internal retune for error handling.*/
	eTUNETYPE_LAST          /**< Use the tune mode used in last tune*/
};

/**
 * @brief Asset's content types
 */
enum ContentType
{
	ContentType_UNKNOWN,        /**< 0 - Unknown type */
	ContentType_CDVR,           /**< 1 - CDVR */
	ContentType_VOD,            /**< 2 - VOD */
	ContentType_LINEAR,         /**< 3 - Linear */
	ContentType_IVOD,           /**< 4 - IVOD */
	ContentType_EAS,            /**< 5 - EAS */
	ContentType_CAMERA,         /**< 6 - Camera */
	ContentType_DVR,            /**< 7 - DVR */
	ContentType_MDVR,           /**< 8 - MDVR */
	ContentType_IPDVR,          /**< 8 - IPDVR */
	ContentType_PPV,            /**< 10 - PPV */
	ContentType_MAX             /**< 11 - Type Count*/
};

/**
 * @brief AAMP Function return values
*/
enum AAMPStatusType
{
	eAAMPSTATUS_OK,
	eAAMPSTATUS_GENERIC_ERROR,
	eAAMPSTATUS_MANIFEST_DOWNLOAD_ERROR,
	eAAMPSTATUS_MANIFEST_PARSE_ERROR,
	eAAMPSTATUS_MANIFEST_CONTENT_ERROR,
	eAAMPSTATUS_MANIFEST_INVALID_TYPE,
	eAAMPSTATUS_SEEK_RANGE_ERROR,
	eAAMPSTATUS_TRACKS_SYNCHRONISATION_ERROR
};


/**
 * @brief Http Header Type
 */
enum HttpHeaderType
{
	eHTTPHEADERTYPE_COOKIE,     /**< Cookie Header */
	eHTTPHEADERTYPE_XREASON,    /**< X-Reason Header */
	eHTTPHEADERTYPE_FOG_REASON, /**< X-Reason Header */
	eHTTPHEADERTYPE_EFF_LOCATION, /**< Effective URL location returned */
	eHTTPHEADERTYPE_UNKNOWN=-1  /**< Unkown Header */
};


/**
 * @brief Http Header Type
 */
enum CurlAbortReason
{
	eCURL_ABORT_REASON_NONE = 0,
	eCURL_ABORT_REASON_STALL_TIMEDOUT,
	eCURL_ABORT_REASON_START_TIMEDOUT
};

/*================================== AAMP Log Manager =========================================*/

/**
 * @brief Direct call for trace printf, can be enabled b defining TRACE here
 */
#ifdef TRACE
#define traceprintf logprintf
#else
#define traceprintf (void)
#endif

/**
 * @brief Macro for validating the log level to be enabled
 */
#define AAMPLOG(LEVEL,FORMAT, ...) \
		do { if (gpGlobalConfig->logging.isLogLevelAllowed(LEVEL)) { \
				logprintf(FORMAT, ##__VA_ARGS__); \
		} } while (0)

/**
 * @brief Macro for Triage Level Logging Support
 */
#define AAMP_LOG_NETWORK_LATENCY	gpGlobalConfig->logging.LogNetworkLatency
#define AAMP_LOG_NETWORK_ERROR		gpGlobalConfig->logging.LogNetworkError
#define AAMP_LOG_DRM_ERROR			gpGlobalConfig->logging.LogDRMError
#define AAMP_LOG_ABR_INFO			gpGlobalConfig->logging.LogABRInfo
#define AAMP_IS_LOG_WORTHY_ERROR	gpGlobalConfig->logging.isLogworthyErrorCode

/**
 * @brief AAMP logging defines, this can be enabled through setLogLevel() as per the need
 */
#define AAMPLOG_TRACE(FORMAT, ...) AAMPLOG(eLOGLEVEL_TRACE, FORMAT, ##__VA_ARGS__)
#define AAMPLOG_INFO(FORMAT, ...) AAMPLOG(eLOGLEVEL_INFO,  FORMAT, ##__VA_ARGS__)
#define AAMPLOG_WARN(FORMAT, ...) AAMPLOG(eLOGLEVEL_WARN, FORMAT, ##__VA_ARGS__)
#define AAMPLOG_ERR(FORMAT, ...) AAMPLOG(eLOGLEVEL_ERROR,  FORMAT, ##__VA_ARGS__)

#define AAMPLOG_FAILOVER(FORMAT, ...) \
		if (gpGlobalConfig->logging.failover) { \
				logprintf(FORMAT, ##__VA_ARGS__); \
		}

/**
 * @brief maximum supported mediatype for latency logging
 */
#define MAX_SUPPORTED_LATENCY_LOGGING_TYPES	(eMEDIATYPE_IFRAME+1)

/**
 * @brief Log level's of AAMP
 */
enum AAMP_LogLevel
{
	eLOGLEVEL_TRACE,    /**< Trace level */
	eLOGLEVEL_INFO,     /**< Info level */
	eLOGLEVEL_WARN,     /**< Warn level */
	eLOGLEVEL_ERROR     /**< Error level */
};

/**
 * @brief Log level network error enum
 */
enum AAMPNetworkErrorType
{
	/* 0 */ AAMPNetworkErrorNone,
	/* 1 */ AAMPNetworkErrorHttp,
	/* 2 */ AAMPNetworkErrorTimeout,
	/* 3 */ AAMPNetworkErrorCurl
};

/**
 * @brief ABR type enum
 */
enum AAMPAbrType
{
	/* 0 */ AAMPAbrBandwidthUpdate,
	/* 1 */ AAMPAbrManifestDownloadFailed,
	/* 2 */ AAMPAbrFragmentDownloadFailed,
	/* 3 */ AAMPAbrUnifiedVideoEngine
};

/**
 * @brief ABR info structure
 */
struct AAMPAbrInfo
{
	AAMPAbrType abrCalledFor;
	int currentProfileIndex;
	int desiredProfileIndex;
	long currentBandwidth;
	long desiredBandwidth;
	long networkBandwidth;
	AAMPNetworkErrorType errorType;
	int errorCode;
};



/**
 * @}
 */

/**
 * @brief AampLogManager Class
 */
class AampLogManager
{
public:

	bool info;       /**< Info level*/
	bool debug;      /**< Debug logs*/
	bool trace;      /**< Trace level*/
	bool gst;        /**< Gstreamer logs*/
	bool curl;       /**< Curl logs*/
	bool progress;   /**< Download progress logs*/
	bool failover;	 /**< server fail over logs*/
	bool latencyLogging[MAX_SUPPORTED_LATENCY_LOGGING_TYPES]; /**< Latency logging for Video, Audio, Manifest download - Refer MediaType on main_aamp.h */ 
	static bool disableLogRedirection;

	/**
	 * @brief AampLogManager constructor
	 */
	AampLogManager() : aampLoglevel(eLOGLEVEL_WARN), info(false), debug(false), trace(false), gst(false), curl(false), progress(false), failover(false)
	{
		memset(latencyLogging, 0 , sizeof(latencyLogging));
	}

	/* ---------- Triage Level Logging Support ---------- */

	/**
	 * @brief Print the network latency level logging for triage purpose
	 *
	 * @param[in] url - content url
	 * @param[in] downloadTime - download time of the fragment or manifest
	 * @param[in] downloadThresholdTimeoutMs - specified download threshold time out value
	 * @retuen void
	 */
	void LogNetworkLatency(const char* url, int downloadTime, int downloadThresholdTimeoutMs);

	/**
	 * @brief Print the network error level logging for triage purpose
	 *
	 * @param[in] url - content url
	 * @param[in] errorType - it can be http or curl errors
	 * @param[in] errorCode - it can be http error or curl error code
	 * @retuen void
	 */
	void LogNetworkError(const char* url, AAMPNetworkErrorType errorType, int errorCode);

	/**
	 * @brief To get the issue symptom based on the error type for triage purpose
	 *
	 * @param[in] url - content url
	 * @param[out] contentType - it could be a manifest or other audio/video/iframe tracks
	 * @param[out] location - server location
	 * @param[out] symptom - issue exhibiting scenario for error case
	 * @retuen void
	 */
	void ParseContentUrl(const char* url, std::string& contentType, std::string& location, std::string& symptom);

	/**
	 * @brief Print the DRM error level logging for triage purpose
	 *
	 * @param[in] major - drm major error code
	 * @param[in] minor - drm minor error code
	 * @retuen void
	 */
	void LogDRMError(int major, int minor);

	/**
	 * @brief Log ABR info for triage purpose
	 *
	 * @param[in] pstAbrInfo - pointer to a structure which will have abr info to be logged
	 * @retuen void
	 */
	void LogABRInfo(AAMPAbrInfo *pstAbrInfo);
	/* !---------- Triage Level Logging Support ---------- */

	/**
	 * @brief To check the given log level is allowed to print mechanism
	 *
	 * @param[in] chkLevel - log level
	 * @retval true if the log level allowed for print mechanism
	 */
	bool isLogLevelAllowed(AAMP_LogLevel chkLevel);

	/**
	 * @brief Set the log level for print mechanism
	 *
	 * @param[in] newLevel - log level new value
	 * @retuen void
	 */
	void setLogLevel(AAMP_LogLevel newLevel);

	/**
	 * @brief Set log file and cfg directory index.
	 */
	void setLogAndCfgDirectory(char driveName);

	/**
	 * @brief Check curl error before log on console.
	 */
	bool isLogworthyErrorCode(int errorCode);

	/**
	 * @brief Get aamp cfg directory.
	 */
	const char* getAampCfgPath(void);

	/**
	 * @brief Get aampcli cfg directory.
	 */
	const char* getAampCliCfgPath(void);

private:
	AAMP_LogLevel aampLoglevel;
};

/* Context-free utility function */

/**
 * @brief Print logs to console / log file
 * @param[in] format - printf style string
 * @retuen void
 */
extern void logprintf(const char *format, ...);

/**
 * @brief Compactly log blobs of binary data
 *
 * @param[in] ptr to the buffer
 * @param[in] size_t  length of buffer
 *
 * @return void
 */
void DumpBlob(const unsigned char *ptr, size_t len);

/*!================================== AAMP Log Manager =========================================*/

/**
 * @brief Class for AAMP's global configuration
 */
class GlobalConfigAAMP
{
public:
	long defaultBitrate;        /**< Default bitrate*/
	long defaultBitrate4K;      /**< Default 4K bitrate*/
	bool bEnableABR;            /**< Enable/Disable adaptive bitrate logic*/
	bool SAP;                   /**< Enable/Disable Secondary Audio Program*/
	bool noFog;                 /**< Disable FOG*/
	int mapMPD;                 /**< Mapping of HLS to MPD: 0=Disable, 1=Rename m3u8 to mpd, 2=COAM mapping, 3='*-nat-*.comcast.net/' to 'ctv-nat-slivel4lb-vip.cmc.co.ndcwest.comcast.net/'*/
	bool fogSupportsDash;       /**< Enable FOG support for DASH*/
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
	int harvest;                /**< Save decrypted fragments for debugging*/
#endif

	AampLogManager logging;             	/**< Aamp log manager class*/
	int gPreservePipeline;                  /**< Flush instead of teardown*/
	int gAampDemuxHLSAudioTsTrack;          /**< Demux Audio track from HLS transport stream*/
	int gAampDemuxHLSVideoTsTrack;          /**< Demux Video track from HLS transport stream*/
	int gAampMergeAudioTrack;               /**< Merge audio track and queued till video processed*/
	int gThrottle;                          /**< Regulate output data flow*/
	TunedEventConfig tunedEventConfigLive;  /**< When to send TUNED event for LIVE*/
	TunedEventConfig tunedEventConfigVOD;   /**< When to send TUNED event for VOD*/
	int demuxHLSVideoTsTrackTM;             /**< Demux video track from HLS transport stream track mode*/
	int demuxedAudioBeforeVideo;            /**< Send demuxed audio before video*/
	bool playlistsParallelFetch;            /**< Enabled parallel fetching of audio & video playlists*/
	bool prefetchIframePlaylist;            /**< Enabled prefetching of I-Frame playlist*/
	int forceEC3;                           /**< Forcefully enable DDPlus*/
	int disableEC3;                         /**< Disable DDPlus*/
	int disableATMOS;                       /**< Disable Dolby ATMOS*/
	int liveOffset;                         /**< Current LIVE offset*/
	int cdvrliveOffset;                     /**< CDVR LIVE offset*/
	int disablePlaylistIndexEvent;          /**< Disable playlist index event*/
	int enableSubscribedTags;               /**< Enabled subscribed tags*/
	bool dashIgnoreBaseURLIfSlash;          /**< Ignore the constructed URI of DASH, if it is / */
	long networkTimeout;                 /**< Fragment download timeout*/
	bool licenseAnonymousRequest;           /**< Acquire license without token*/
	int minVODCacheSeconds;                 /**< Minimum VOD caching duration in seconds*/
	int abrCacheLife;                       /**< Adaptive bitrate cache life in seconds*/
	int abrCacheLength;                     /**< Adaptive bitrate cache length*/
	int maxCachedFragmentsPerTrack;         /**< fragment cache length*/
	int abrOutlierDiffBytes;                /**< Adaptive bitrate outlier, if values goes beyond this*/
	int abrNwConsistency;                   /**< Adaptive bitrate network consistency*/
	int bufferHealthMonitorDelay;           /**< Buffer health monitor start delay after tune/ seek*/
	int bufferHealthMonitorInterval;        /**< Buffer health monitor interval*/
	bool hlsAVTrackSyncUsingStartTime;      /**< HLS A/V track to be synced with start time*/
	char* licenseServerURL;                 /**< License server URL*/
	bool licenseServerLocalOverride;        /**< Enable license server local overriding*/
	int vodTrickplayFPS;                    /**< Trickplay frames per second for VOD*/
	bool vodTrickplayFPSLocalOverride;      /**< Enabled VOD Trickplay FPS local overriding*/
	int linearTrickplayFPS;                 /**< Trickplay frames per second for LIVE*/
	bool linearTrickplayFPSLocalOverride;   /**< Enabled LIVE Trickplay FPS local overriding*/
	int stallErrorCode;                     /**< Stall error code*/
	int stallTimeoutInMS;                   /**< Stall timeout in milliseconds*/
	const char* httpProxy;                  /**< HTTP proxy address*/
	int reportProgressInterval;             /**< Interval of progress reporting*/
	DRMSystems preferredDrm;                /**< Preferred DRM*/
	bool  isUsingLocalConfigForPreferredDRM;          /**< Preferred DRM configured as part of aamp.cfg */
	bool mpdDiscontinuityHandling;          /**< Enable MPD discontinuity handling*/
	bool mpdDiscontinuityHandlingCdvr;      /**< Enable MPD discontinuity handling for CDVR*/
	bool bForceHttp;                        /**< Force HTTP*/
	int abrSkipDuration;                    /**< Initial duration for ABR skip*/
	bool internalReTune;                    /**< Internal re-tune on underflows/ pts errors*/
	int ptsErrorThreshold;                       /**< Max number of back-to-back PTS errors within designated time*/
	bool bAudioOnlyPlayback;                /**< AAMP Audio Only Playback*/
	bool gstreamerBufferingBeforePlay;      /**< Enable pre buffering logic which ensures minimum buffering is done before pipeline play*/
	int licenseRetryWaitTime;
	long iframeBitrate;                     /**< Default bitrate for iframe track selection for non-4K assets*/
	long iframeBitrate4K;                   /**< Default bitrate for iframe track selection for 4K assets*/
	char *prLicenseServerURL;               /**< Playready License server URL*/
	char *wvLicenseServerURL;               /**< Widevine License server URL*/
	bool enableMicroEvents;                 /**< Enabling the tunetime micro events*/
	long curlStallTimeout;                  /**< Timeout value for detection curl download stall in seconds*/
	long curlDownloadStartTimeout;          /**< Timeout value for curl download to start after connect in seconds*/
	bool enablePROutputProtection;          /**< Playready output protection config */
	char *pUserAgentString;			/**< Curl user-agent string */
	bool reTuneOnBufferingTimeout;          /**< Re-tune on buffering timeout */
	int gMaxPlaylistCacheSize;              /**< Max Playlist Cache Size  */
	int waitTimeBeforeRetryHttp5xxMS;		/**< Wait time in milliseconds before retry for 5xx errors*/
	int dash_MaxDRMSessions;				/** < Max drm sessions that can be cached by AampDRMSessionManager*/
	bool enableClientDai;                   /**< Enabling the client side DAI*/
	bool playAdFromCDN;                     /**< Play Ad from CDN. Not from FOG.*/
public:

	/**
	 * @brief GlobalConfigAAMP Constructor
	 */
	GlobalConfigAAMP() :defaultBitrate(DEFAULT_INIT_BITRATE), defaultBitrate4K(DEFAULT_INIT_BITRATE_4K), bEnableABR(true), SAP(false), noFog(false), mapMPD(0), fogSupportsDash(true),abrCacheLife(DEFAULT_ABR_CACHE_LIFE),abrCacheLength(DEFAULT_ABR_CACHE_LENGTH),maxCachedFragmentsPerTrack(DEFAULT_CACHED_FRAGMENTS_PER_TRACK),
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
		harvest(0),
#endif
		gPreservePipeline(0), gAampDemuxHLSAudioTsTrack(1), gAampMergeAudioTrack(1), forceEC3(0),
		gAampDemuxHLSVideoTsTrack(1), demuxHLSVideoTsTrackTM(1), gThrottle(0), demuxedAudioBeforeVideo(0),
		playlistsParallelFetch(false), prefetchIframePlaylist(false),
		disableEC3(0), disableATMOS(0),abrOutlierDiffBytes(DEFAULT_ABR_OUTLIER),abrSkipDuration(DEFAULT_ABR_SKIP_DURATION),
		liveOffset(AAMP_LIVE_OFFSET),cdvrliveOffset(AAMP_CDVR_LIVE_OFFSET), abrNwConsistency(DEFAULT_ABR_NW_CONSISTENCY_CNT),
		disablePlaylistIndexEvent(1), enableSubscribedTags(1), dashIgnoreBaseURLIfSlash(false),networkTimeout(CURL_FRAGMENT_DL_TIMEOUT),
		licenseAnonymousRequest(false), minVODCacheSeconds(DEFAULT_MINIMUM_CACHE_VOD_SECONDS),
		bufferHealthMonitorDelay(DEFAULT_BUFFER_HEALTH_MONITOR_DELAY), bufferHealthMonitorInterval(DEFAULT_BUFFER_HEALTH_MONITOR_INTERVAL),
		preferredDrm(eDRM_PlayReady), hlsAVTrackSyncUsingStartTime(false), licenseServerURL(NULL), licenseServerLocalOverride(false),
		vodTrickplayFPS(TRICKPLAY_NETWORK_PLAYBACK_FPS),vodTrickplayFPSLocalOverride(false),
		linearTrickplayFPS(TRICKPLAY_TSB_PLAYBACK_FPS),linearTrickplayFPSLocalOverride(false),
		stallErrorCode(DEFAULT_STALL_ERROR_CODE), stallTimeoutInMS(DEFAULT_STALL_DETECTION_TIMEOUT), httpProxy(0),
		reportProgressInterval(DEFAULT_REPORT_PROGRESS_INTERVAL), mpdDiscontinuityHandling(true), mpdDiscontinuityHandlingCdvr(true),bForceHttp(false),
		internalReTune(true), bAudioOnlyPlayback(false), gstreamerBufferingBeforePlay(true),licenseRetryWaitTime(DEF_LICENSE_REQ_RETRY_WAIT_TIME),
		iframeBitrate(0), iframeBitrate4K(0),ptsErrorThreshold(MAX_PTS_ERRORS_THRESHOLD),
		prLicenseServerURL(NULL), wvLicenseServerURL(NULL)
		,curlStallTimeout(0), curlDownloadStartTimeout(0)
		,enableMicroEvents(false),enablePROutputProtection(false), reTuneOnBufferingTimeout(true), gMaxPlaylistCacheSize(MAX_PLAYLIST_CACHE_SIZE)
		,waitTimeBeforeRetryHttp5xxMS(DEFAULT_WAIT_TIME_BEFORE_RETRY_HTTP_5XX_MS),
		dash_MaxDRMSessions(MIN_DASH_DRM_SESSIONS),
		tunedEventConfigLive(eTUNED_EVENT_ON_PLAYLIST_INDEXED), tunedEventConfigVOD(eTUNED_EVENT_ON_PLAYLIST_INDEXED),
		isUsingLocalConfigForPreferredDRM(false), pUserAgentString(NULL), logging()
		, enableClientDai(false), playAdFromCDN(false)
	{
		//XRE sends onStreamPlaying while receiving onTuned event.
		//onVideoInfo depends on the metrics received from pipe.
        // considering round trip delay to remove overlay
        // onStreamPlaying is sent optimistically in advance
		aamp_SetBaseUserAgentString(AAMP_USERAGENT_BASE_STRING);
	}

	/**
	 * @brief GlobalConfigAAMP Destructor
	 */
	~GlobalConfigAAMP(){}

	GlobalConfigAAMP(const GlobalConfigAAMP&) = delete;

	GlobalConfigAAMP& operator=(const GlobalConfigAAMP&) = delete;

	/**
	 * @brief Get SAP status
	 *
	 * @return Enabled/Disabled
	 */
	bool aamp_GetSAP(void)
	{
		return SAP;
	}

	/**
	 * @brief Setting secondary audio program
	 *
	 * @param[in] on - New SAP status
	 * @return void
	 */
	void aamp_SetSAP(bool on)
	{
		SAP = on;
	}

	/**
	@brief Sets user agent string
	*
	* @return none
	*/
	void aamp_SetBaseUserAgentString(const char * newUserAgent)
	{
		int iLen = strlen(newUserAgent) + strlen(AAMP_USERAGENT_SUFFIX) + 2;
		if(pUserAgentString)
		{
			free(pUserAgentString);
		}
		pUserAgentString =(char*) malloc(iLen);
		sprintf(pUserAgentString, "%s %s",newUserAgent,AAMP_USERAGENT_SUFFIX);
	}
};

extern GlobalConfigAAMP *gpGlobalConfig;

// context-free utility functions

/**
 * @brief Create file URL from the base and file path
 *
 * @param[out] dst - Created URL
 * @param[in] base - Base URL
 * @param[in] uri - File path
 * @return void
 */
void aamp_ResolveURL(char *dst, const char *base, const char *uri);

/**
 * @brief Get current time from epoch is milliseconds
 *
 * @return Current time in milliseconds
 */
long long aamp_GetCurrentTimeMS(void); //TODO: Use NOW_STEADY_TS_MS/NOW_SYSTEM_TS_MS instead

/**
 * @brief Log error
 *
 * @param[in] msg - Error message
 * @return void
 */
void aamp_Error(const char *msg);

/**
 * @brief AAMP's custom implementation of memory deallocation
 *
 * @param[in] pptr - Buffer to be deallocated
 * @return void
 */
void aamp_Free(char **pptr);

/**
 * @brief Append bytes to the GrowableBuffer
 *
 * @param[in,out] buffer - GrowableBuffer to be appended
 * @param[in] ptr - Array of bytes to be appended
 * @param[in] len - Number of bytes
 * @return void
 */
void aamp_AppendBytes(struct GrowableBuffer *buffer, const void *ptr, size_t len);

/**
 * @brief
 * @param[in] buffer
 * @return void
 */
void aamp_AppendNulTerminator(struct GrowableBuffer *buffer);

/**
 * @brief Memory allocation method for GrowableBuffer
 *
 * @param[out] buffer - Allocated GrowableBuffer
 * @param[in] len - Allocation size
 * @return void
 */
void aamp_Malloc(struct GrowableBuffer *buffer, size_t len);

/**
 * @brief Get DRM system ID
 *
 * @param[in] drmSystem - DRM system type
 * @return DRM system ID
 */
const char * GetDrmSystemID(DRMSystems drmSystem);

/**
 * @brief Get DRM system name
 *
 * @param[in] drmSystem - DRM system type
 * @return DRM system name
 */
const char * GetDrmSystemName(DRMSystems drmSystem);

/**
 * @brief Encode URL
 * @param[in] inSrc - Input URL
 * @param[out] outStr - Encoded URL
 * @return Encoding status
 *
 */

bool UrlEncode(const char *inSrc, std::string &outStr);

/**
 * @}
 */

/**
 * @addtogroup AAMP_COMMON_TYPES
 * @{
 */
/**
 * @brief Bucket types of AAMP profiler
 */
typedef enum
{
	PROFILE_BUCKET_MANIFEST,            /**< Manifest download bucket*/

	PROFILE_BUCKET_PLAYLIST_VIDEO,      /**< Video playlist download bucket*/
	PROFILE_BUCKET_PLAYLIST_AUDIO,      /**< Audio playlist download bucket*/

	PROFILE_BUCKET_INIT_VIDEO,          /**< Video init fragment download bucket*/
	PROFILE_BUCKET_INIT_AUDIO,          /**< Audio init fragment download bucket*/

	PROFILE_BUCKET_FRAGMENT_VIDEO,      /**< Video fragment download bucket*/
	PROFILE_BUCKET_FRAGMENT_AUDIO,      /**< Audio fragment download bucket*/

	PROFILE_BUCKET_DECRYPT_VIDEO,       /**< Video decryption bucket*/
	PROFILE_BUCKET_DECRYPT_AUDIO,       /**< Audio decryption bucket*/

	PROFILE_BUCKET_LA_TOTAL,            /**< License acquisition total bucket*/
	PROFILE_BUCKET_LA_PREPROC,          /**< License acquisition pre-processing bucket*/
	PROFILE_BUCKET_LA_NETWORK,          /**< License acquisition network operation bucket*/
	PROFILE_BUCKET_LA_POSTPROC,         /**< License acquisition post-processing bucket*/

	PROFILE_BUCKET_FIRST_BUFFER,        /**< First buffer to gstreamer bucket*/
	PROFILE_BUCKET_FIRST_FRAME,         /**< First frame displaye bucket*/
	PROFILE_BUCKET_TYPE_COUNT           /**< Bucket count*/
} ProfilerBucketType;

/**
 * @brief Bucket types of classic profiler
 */
typedef enum
 {
	TuneTimeBaseTime,           /**< Tune time base*/
	TuneTimeBeginLoad,          /**< Player load time*/
	TuneTimePrepareToPlay,      /**< Manifest ready time*/
	TuneTimePlay,               /**< Profiles ready time*/
	TuneTimeDrmReady,           /**< DRM ready time*/
	TuneTimeStartStream,        /**< First buffer insert time*/
	TuneTimeStreaming,          /**< First frame display time*/
	TuneTimeBackToXre,          /**< Tune status back to XRE time*/
	TuneTimeMax                 /**< Max bucket type*/
 }ClassicProfilerBucketType;

/**
 * @enum AudioType
 *
 * @brief Type of audio ES for MPD
 */
enum AudioType
{
	eAUDIO_UNKNOWN,
	eAUDIO_AAC,
	eAUDIO_DDPLUS,
	eAUDIO_ATMOS,
	eAUDIO_UNSUPPORTED
};

/**
 * @brief Class for AAMP event Profiling
 */
class ProfileEventAAMP
{
private:
	// TODO: include settop type (to distinguish settop performance)
	// TODO: include flag to indicate whether FOG used (to isolate FOG overhead)

    /**
     * @brief Class corresponding to tune time events.
     */
    class TuneEvent
	{
	public:
		ProfilerBucketType id;      /**< Event identifier */
		unsigned int start;         /**< Event start time */
		unsigned int duration;      /**< Event duration */
		int result;                 /**< Event result */

		/**
		 * @brief TuneEvent Constructor
		 * @param[in] i - Event id
		 * @param[in] s - Event start time
		 * @param[in] d - Event duration
		 * @param[in] r - Event result
		 */
		TuneEvent(ProfilerBucketType i, unsigned int s,
				unsigned int d, int r):id(i),start(s),duration(d),result(r)
		{}
	};

	/**
	 * @brief Data structure corresponding to profiler bucket
	 */
	struct ProfilerBucket
	{
		unsigned int tStart;    /**< Relative start time of operation, based on tuneStartMonotonicBase */
		unsigned int tFinish;   /**< Relative end time of operation, based on tuneStartMonotonicBase */
		int errorCount;         /**< non-zero if errors/retries occured during this operation */
		bool complete;          /**< true if this step already accounted for, and further profiling should be ignored */
	} buckets[PROFILE_BUCKET_TYPE_COUNT];

	/**
	 * @brief Calculating effecting duration of overlapping buckets, id1 & id2
	 */
#define bucketsOverlap(id1,id2) \
		buckets[id1].complete && buckets[id2].complete && \
		(buckets[id1].tStart <= buckets[id2].tFinish) && (buckets[id2].tStart <= buckets[id1].tFinish)

	/**
	 * @brief Calculating total duration a bucket id
	 */
#define bucketDuration(id) \
		(buckets[id].complete?(buckets[id].tFinish - buckets[id].tStart):0)

	long long tuneStartMonotonicBase;       /**< Base time from Monotonic clock for interval calculation */

	long long tuneStartBaseUTCMS;           /**< common UTC base for start of tune */
	long long xreTimeBuckets[TuneTimeMax];  /**< Start time of each buckets for classic metrics conversion */
	long bandwidthBitsPerSecondVideo;       /**< Video bandwidth in bps */
	long bandwidthBitsPerSecondAudio;       /**< Audio bandwidth in bps */
	int drmErrorCode;                       /**< DRM error code */
	bool enabled;                           /**< Profiler started or not */
	std::list<TuneEvent> tuneEventList;     /**< List of events happened during tuning */
	std::mutex tuneEventListMtx;            /**< Mutex protecting tuneEventList */

	/**
	 * @brief Calculating effective time of two overlapping buckets.
	 *
	 * @param[in] id1 - Bucket type 1
	 * @param[in] id2 - Bucket type 2
	 * @return void
	 */
	inline unsigned int effectiveBucketTime(ProfilerBucketType id1, ProfilerBucketType id2)
	{
#if 0
		if(bucketsOverlap(id1, id2))
			return MAX(buckets[id1].tFinish, buckets[id2].tFinish) - fmin(buckets[id1].tStart, buckets[id2].tStart);
#endif
		return bucketDuration(id1) + bucketDuration(id2);
	}
public:

	/**
	 * @brief ProfileEventAAMP Constructor
	 */
	ProfileEventAAMP() : tuneStartMonotonicBase(0), tuneStartBaseUTCMS(0), bandwidthBitsPerSecondVideo(0),
        bandwidthBitsPerSecondAudio(0), drmErrorCode(0), enabled(false), xreTimeBuckets(), tuneEventList(), tuneEventListMtx()
	{
	}

	/**
	 * @brief ProfileEventAAMP Destructor
	 */
	~ProfileEventAAMP(){}

	/**
	 * @brief Setting video bandwidth in bps
	 *
	 * @param[in] bw - Bandwidth in bps
	 * @return void
	 */
	void SetBandwidthBitsPerSecondVideo(long bw)
	{
		bandwidthBitsPerSecondVideo = bw;
	}

	/**
	 * @brief Setting audio bandwidth in bps
	 *
	 * @param[in] bw - Bandwidth in bps
	 * @return void
	 */
	void SetBandwidthBitsPerSecondAudio(long bw)
	{
		bandwidthBitsPerSecondAudio = bw;
	}

	/**
	 * @brief Setting DRM error code
	 *
	 * @param[in] errCode - Error code
	 * @return void
	 */
	void SetDrmErrorCode(int errCode)
	{
		drmErrorCode = errCode;
	}

	/**
	 * @brief Record a new tune time event.
	 *
	 * @param[in] pbt - Profiler bucket type
	 * @param[in] start - Start time
	 * @param[in] dur - Duration
	 * @param[in] res - Event result
	 * @return void
	 */
	void addtuneEvent(ProfilerBucketType pbt, unsigned int start,
					  unsigned int dur, int res)
	{
		if(pbt >= PROFILE_BUCKET_TYPE_COUNT)
		{
			logprintf("WARN: bucketId=%d > PROFILE_BUCKET_TYPE_COUNT. How did it happen?", pbt);
			return;
		}

		if(!(buckets[pbt].complete))
		{
			std::lock_guard<std::mutex> lock(tuneEventListMtx);
			tuneEventList.emplace_back(pbt,(start - tuneStartMonotonicBase),dur,res);
		}
	}

	/**
	 * @brief Get tune time events in JSON format
	 *
	 * @param[out] outSS - Output JSON string
	 * @param[in] streamType - Stream type
	 * @param[in] url - Tune URL
	 * @param[in] success - Tune success/failure
	 * @return void
	 */
	void getTuneEventsJSON(std::stringstream &outSS, const std::string &streamType, const char *url, bool success)
	{
		bool siblingEvent = false;
		unsigned int tEndTime = NOW_STEADY_TS_MS;

		outSS << "{\"s\":" << tuneStartBaseUTCMS
				//TODO: It should be the duration relative to XRE start time.
				<< ",\"td\":" << (tEndTime - tuneStartMonotonicBase)
				<< ",\"st\":\"" << streamType << "\",\"u\":\"" << url
				<< "\",\"r\":" << (success ? 1 : 0) << ",\"v\":[";

		std::lock_guard<std::mutex> lock(tuneEventListMtx);
		for(auto &te:tuneEventList)
		{
			if(siblingEvent)
			{
				outSS<<",";
			}
			outSS << "{\"i\":" << te.id << ",\"b\":"
					<< te.start << ",\"d\":" << te.duration << ",\"o\":"
					<< te.result << "}";

			siblingEvent = true;
		}
		outSS<<"]}";

		tuneEventList.clear();
	}

	/**
	 * @brief Profiler method to perform tune begin related operations.
	 *
	 * @return void
	 */
	void TuneBegin(void)
	{ // start tune
		memset(buckets, 0, sizeof(buckets));
		tuneStartBaseUTCMS = NOW_SYSTEM_TS_MS;
		tuneStartMonotonicBase = NOW_STEADY_TS_MS;
		bandwidthBitsPerSecondVideo = 0;
		bandwidthBitsPerSecondAudio = 0;
		drmErrorCode = 0;
		enabled = true;
		tuneEventList.clear();
	}

	/**
	 * @brief Logging performance metrics after successful tune completion. Metrics starts with IP_AAMP_TUNETIME
	 *
	 * <h4>Format of IP_AAMP_TUNETIME:</h4>
	 * version,	// version for this protocol, initially zero<br>
	 * build,		// incremented when there are significant player changes/optimizations<br>
	 * tunestartUtcMs,	// when tune logically started from AAMP perspective<br>
	 * <br>
	 * ManifestDownloadStartTime,  // offset in milliseconds from tunestart when main manifest begins download<br>
	 * ManifestDownloadTotalTime,  // time (ms) taken for main manifest download, relative to ManifestDownloadStartTime<br>
	 * ManifestDownloadFailCount,  // if >0 ManifestDownloadTotalTime spans multiple download attempts<br>
	 * <br>
	 * PlaylistDownloadStartTime,  // offset in milliseconds from tunestart when playlist subManifest begins download<br>
	 * PlaylistDownloadTotalTime,  // time (ms) taken for playlist subManifest download, relative to PlaylistDownloadStartTime<br>
	 * PlaylistDownloadFailCount,  // if >0 otherwise PlaylistDownloadTotalTime spans multiple download attempts<br>
	 * <br>
	 * InitFragmentDownloadStartTime, // offset in milliseconds from tunestart when init fragment begins download<br>
	 * InitFragmentDownloadTotalTime, // time (ms) taken for fragment download, relative to InitFragmentDownloadStartTime<br>
	 * InitFragmentDownloadFailCount, // if >0 InitFragmentDownloadTotalTime spans multiple download attempts<br>
	 * <br>
	 * Fragment1DownloadStartTime, // offset in milliseconds from tunestart when fragment begins download<br>
	 * Fragment1DownloadTotalTime, // time (ms) taken for fragment download, relative to Fragment1DownloadStartTime<br>
	 * Fragment1DownloadFailCount, // if >0 Fragment1DownloadTotalTime spans multiple download attempts<br>
	 * Fragment1Bandwidth,	    	// intrinsic bitrate of downloaded fragment<br>
	 * <br>
	 * drmLicenseRequestStart,	    // offset in milliseconds from tunestart<br>
	 * drmLicenseRequestTotalTime, // time (ms) for license acquisition relative to drmLicenseRequestStart<br>
	 * drmFailErrorCode,           // nonzero if drm license acquisition failed during tuning<br>
	 * <br>
	 * LAPreProcDuration,	    	// License acquisition pre-processing duration in ms<br>
	 * LANetworkDuration, 			// License acquisition network duration in ms<br>
	 * LAPostProcDuration,         // License acquisition post-processing duration in ms<br>
	 * <br>
	 * VideoDecryptDuration,		// Video fragment decrypt duration in ms<br>
	 * AudioDecryptDuration,		// Audio fragment decrypt duration in ms<br>
	 * <br>
	 * gstStart,	// offset in ms from tunestart when pipeline creation/setup begins<br>
	 * gstFirstFrame,  // offset in ms from tunestart when first frame of video is decoded/presented<br>
	 * <br>
	 * contentType, 	//Playback Mode. Values: CDVR, VOD, LINEAR, IVOD, EAS, CAMERA, DVR, MDVR, IPDVR, PPV<br>
	 * streamType, 	//Stream Type. Values: 10-HLS/Clear, 11-HLS/Consec, 12-HLS/Access, 13-HLS/Vanilla AES, 20-DASH/Clear, 21-DASH/WV, 22-DASH/PR<br>
	 * firstTune		//First tune after reboot/crash<br>
	 * @param[in] success - Tune status
	 * @param[in] contentType - Content Type. Eg: LINEAR, VOD, etc
	 * @param[in] streamType - Stream Type. Eg: HLS, DASH, etc
	 * @param[in] firstTune - Is it a first tune after reboot/crash.
	 * @return void
	 */
	void TuneEnd(bool success, ContentType contentType, int streamType, bool firstTune)
	{
		if(!enabled )
		{
			return;
		}
		enabled = false;
		unsigned int licenseAcqNWTime = bucketDuration(PROFILE_BUCKET_LA_NETWORK);
		if(licenseAcqNWTime == 0) licenseAcqNWTime = bucketDuration(PROFILE_BUCKET_LA_TOTAL); //A HACK for HLS

		logprintf("IP_AAMP_TUNETIME:%d,%d,%lld," // version, build, tuneStartBaseUTCMS
			"%d,%d,%d," 	// main manifest (start,total,err)
			"%d,%d,%d," 	// video playlist (start,total,err)
			"%d,%d,%d," 	// audio playlist (start,total,err)

			"%d,%d,%d," 	// video init-segment (start,total,err)
			"%d,%d,%d," 	// audio init-segment (start,total,err)

			"%d,%d,%d,%ld," 	// video fragment (start,total,err, bitrate)
			"%d,%d,%d,%ld," 	// audio fragment (start,total,err, bitrate)

			"%d,%d,%d," 	// licenseAcqStart, licenseAcqTotal, drmFailErrorCode
			"%d,%d,%d," 	// LAPreProcDuration, LANetworkDuration, LAPostProcDuration

			"%d,%d," 		// VideoDecryptDuration, AudioDecryptDuration
			"%d,%d," 		// gstPlayStartTime, gstFirstFrameTime
			"%d,%d,%d\n", 		// contentType, streamType, firstTune
			// TODO: settop type, flags, isFOGEnabled, isDDPlus, isDemuxed, assetDurationMs

			4, // version for this protocol, initially zero
			0, // build - incremented when there are significant player changes/optimizations
			tuneStartBaseUTCMS, // when tune logically started from AAMP perspective

			buckets[PROFILE_BUCKET_MANIFEST].tStart, bucketDuration(PROFILE_BUCKET_MANIFEST), buckets[PROFILE_BUCKET_MANIFEST].errorCount,
			buckets[PROFILE_BUCKET_PLAYLIST_VIDEO].tStart, bucketDuration(PROFILE_BUCKET_PLAYLIST_VIDEO), buckets[PROFILE_BUCKET_PLAYLIST_VIDEO].errorCount,
			buckets[PROFILE_BUCKET_PLAYLIST_AUDIO].tStart, bucketDuration(PROFILE_BUCKET_PLAYLIST_AUDIO), buckets[PROFILE_BUCKET_PLAYLIST_AUDIO].errorCount,

			buckets[PROFILE_BUCKET_INIT_VIDEO].tStart, bucketDuration(PROFILE_BUCKET_INIT_VIDEO), buckets[PROFILE_BUCKET_INIT_VIDEO].errorCount,
			buckets[PROFILE_BUCKET_INIT_AUDIO].tStart, bucketDuration(PROFILE_BUCKET_INIT_AUDIO), buckets[PROFILE_BUCKET_INIT_AUDIO].errorCount,

			buckets[PROFILE_BUCKET_FRAGMENT_VIDEO].tStart, bucketDuration(PROFILE_BUCKET_FRAGMENT_VIDEO), buckets[PROFILE_BUCKET_FRAGMENT_VIDEO].errorCount,bandwidthBitsPerSecondVideo,
			buckets[PROFILE_BUCKET_FRAGMENT_AUDIO].tStart, bucketDuration(PROFILE_BUCKET_FRAGMENT_AUDIO), buckets[PROFILE_BUCKET_FRAGMENT_AUDIO].errorCount,bandwidthBitsPerSecondAudio,

			buckets[PROFILE_BUCKET_LA_TOTAL].tStart, bucketDuration(PROFILE_BUCKET_LA_TOTAL), drmErrorCode,
			bucketDuration(PROFILE_BUCKET_LA_PREPROC), licenseAcqNWTime, bucketDuration(PROFILE_BUCKET_LA_POSTPROC),
			bucketDuration(PROFILE_BUCKET_DECRYPT_VIDEO),bucketDuration(PROFILE_BUCKET_DECRYPT_AUDIO),

			buckets[PROFILE_BUCKET_FIRST_BUFFER].tStart, // gstPlaying: offset in ms from tunestart when pipeline first fed data
			buckets[PROFILE_BUCKET_FIRST_FRAME].tStart,  // gstFirstFrame: offset in ms from tunestart when first frame of video is decoded/presented
			contentType, streamType, firstTune
			);
		fflush(stdout);
	}

	/**
	 * @brief Method converting the AAMP style tune performance data to IP_EX_TUNETIME style data
	 *
	 * @param[in] success - Tune status
	 * @param[in] tuneRetries - Number of tune attempts
	 * @param[in] playerLoadTime - Time at which the first tune request reached the AAMP player
	 * @param[in] streamType - Type of stream. eg: HLS, DASH, etc
	 * @param[in] isLive  - Live channel or not
	 * @param[in] durationinSec - Asset duration in seconds
	 * @param[out] TuneTimeInfoStr - Formatted output string
	 * @return void
	 */
	void GetClassicTuneTimeInfo(bool success, int tuneRetries, int firstTuneType, long long playerLoadTime, int streamType, bool isLive,unsigned int durationinSec, char *TuneTimeInfoStr)
	{
						// Prepare String for Classic TuneTime data
						// Note: Certain buckets won't be available; will take the tFinish of the previous bucket as the start & finish those buckets.
						xreTimeBuckets[TuneTimeBeginLoad]               =       tuneStartMonotonicBase ;
						xreTimeBuckets[TuneTimePrepareToPlay]           =       tuneStartMonotonicBase + buckets[PROFILE_BUCKET_MANIFEST].tFinish;
						xreTimeBuckets[TuneTimePlay]                    =       tuneStartMonotonicBase + MAX(buckets[PROFILE_BUCKET_MANIFEST].tFinish, MAX(buckets[PROFILE_BUCKET_PLAYLIST_VIDEO].tFinish, buckets[PROFILE_BUCKET_PLAYLIST_AUDIO].tFinish));
						xreTimeBuckets[TuneTimeDrmReady]                =       MAX(xreTimeBuckets[TuneTimePlay], (tuneStartMonotonicBase +  buckets[PROFILE_BUCKET_LA_TOTAL].tFinish));
						long long fragmentReadyTime                     =       tuneStartMonotonicBase + MAX(buckets[PROFILE_BUCKET_FRAGMENT_VIDEO].tFinish, buckets[PROFILE_BUCKET_FRAGMENT_AUDIO].tFinish);
						xreTimeBuckets[TuneTimeStartStream]             =       MAX(xreTimeBuckets[TuneTimeDrmReady],fragmentReadyTime);
						xreTimeBuckets[TuneTimeStreaming]               =       tuneStartMonotonicBase + buckets[PROFILE_BUCKET_FIRST_FRAME].tStart;

						unsigned int failRetryBucketTime                =       tuneStartMonotonicBase - playerLoadTime;
						unsigned int prepareToPlayBucketTime            =       (unsigned int)(xreTimeBuckets[TuneTimePrepareToPlay] - xreTimeBuckets[TuneTimeBeginLoad]);
						unsigned int playBucketTime                     =       (unsigned int)(xreTimeBuckets[TuneTimePlay]- xreTimeBuckets[TuneTimePrepareToPlay]);
						unsigned int drmReadyBucketTime                 =       (unsigned int)(xreTimeBuckets[TuneTimeDrmReady] - xreTimeBuckets[TuneTimePlay]) ;
						unsigned int fragmentBucketTime                 =       (unsigned int)(fragmentReadyTime - xreTimeBuckets[TuneTimePlay]) ;
						unsigned int decoderStreamingBucketTime         =       xreTimeBuckets[TuneTimeStreaming] - xreTimeBuckets[TuneTimeStartStream];
						/*Note: 'Drm Ready' to 'decrypt start' gap is not covered in any of the buckets.*/

						unsigned int manifestTotal      =       bucketDuration(PROFILE_BUCKET_MANIFEST);
						unsigned int profilesTotal      =       effectiveBucketTime(PROFILE_BUCKET_PLAYLIST_VIDEO, PROFILE_BUCKET_PLAYLIST_AUDIO);
						unsigned int initFragmentTotal  =       effectiveBucketTime(PROFILE_BUCKET_INIT_VIDEO, PROFILE_BUCKET_INIT_AUDIO);
						unsigned int fragmentTotal      =       effectiveBucketTime(PROFILE_BUCKET_FRAGMENT_VIDEO, PROFILE_BUCKET_FRAGMENT_AUDIO);
						unsigned int licenseTotal       =       bucketDuration(PROFILE_BUCKET_LA_TOTAL);
						unsigned int licenseNWTime      =       bucketDuration(PROFILE_BUCKET_LA_NETWORK);
						if(licenseNWTime == 0) licenseNWTime = licenseTotal;  //A HACK for HLS

						// Total Network Time
						unsigned int networkTime = manifestTotal + profilesTotal + initFragmentTotal + fragmentTotal + licenseNWTime;

						snprintf(TuneTimeInfoStr,AAMP_MAX_PIPE_DATA_SIZE,"%d,%lld,%d,%d," //totalNetworkTime, playerLoadTime , failRetryBucketTime, prepareToPlayBucketTime,
								"%d,%d,%d,"                                             //playBucketTime ,drmReadyBucketTime , decoderStreamingBucketTime
								"%d,%d,%d,%d,"                                          // manifestTotal,profilesTotal,fragmentTotal,effectiveFragmentDLTime
								"%d,%d,%d,%d,"                                          // licenseTotal,success,durationinMilliSec,isLive
								"%lld,%lld,%lld,"                                       // TuneTimeBeginLoad,TuneTimePrepareToPlay,TuneTimePlay,
								"%lld,%lld,%lld,"                                       //TuneTimeDrmReady,TuneTimeStartStream,TuneTimeStreaming
								"%d,%d,%d,%lld",                                             //streamType, tuneRetries, TuneType, TuneCompleteTime(UTC MSec)
								networkTime,playerLoadTime, failRetryBucketTime, prepareToPlayBucketTime,playBucketTime,drmReadyBucketTime,decoderStreamingBucketTime,
								manifestTotal,profilesTotal,(initFragmentTotal + fragmentTotal),fragmentBucketTime, licenseTotal,success,durationinSec*1000,isLive,
								xreTimeBuckets[TuneTimeBeginLoad],xreTimeBuckets[TuneTimePrepareToPlay],xreTimeBuckets[TuneTimePlay] ,xreTimeBuckets[TuneTimeDrmReady],
								xreTimeBuckets[TuneTimeStartStream],xreTimeBuckets[TuneTimeStreaming],streamType,tuneRetries,firstTuneType,NOW_SYSTEM_TS_MS
								);
#ifndef CREATE_PIPE_SESSION_TO_XRE
						logprintf("AAMP=>XRE: %s\n",TuneTimeInfoStr);
#endif

	}


	/**
	 * @brief Marking the beginning of a bucket
	 *
	 * @param[in] type - Bucket type
	 * @return void
	 */
	void ProfileBegin(ProfilerBucketType type )
	{
		struct ProfilerBucket *bucket = &buckets[type];
		if (!bucket->complete && (0==bucket->tStart))	//No other Begin should record before the End
		{
			bucket->tStart = NOW_STEADY_TS_MS - tuneStartMonotonicBase;
			bucket->tFinish = bucket->tStart;
		}
	}

	/**
	 * @brief Marking error while executing a bucket
	 *
	 * @param[in] type - Bucket type
	 * @param[in] result - Error code
	 * @return void
	 */
	void ProfileError(ProfilerBucketType type, int result = -1)
	{
		struct ProfilerBucket *bucket = &buckets[type];
		if (!bucket->complete && !(0==bucket->tStart))
		{
			bucket->errorCount++;
			if(gpGlobalConfig->enableMicroEvents && (type == PROFILE_BUCKET_DECRYPT_VIDEO || type == PROFILE_BUCKET_DECRYPT_AUDIO
												 || type == PROFILE_BUCKET_LA_TOTAL || type == PROFILE_BUCKET_LA_NETWORK))
			{
				long long start = bucket->tStart + tuneStartMonotonicBase;
				addtuneEvent(type, start, (unsigned int)(NOW_STEADY_TS_MS - start), result);
			}
		}
	}


	/**
	 * @brief Marking the end of a bucket
	 *
	 * @param[in] type - Bucket type
	 * @return void
	 */
	void ProfileEnd( ProfilerBucketType type )
	{
		struct ProfilerBucket *bucket = &buckets[type];
		if (!bucket->complete && !(0==bucket->tStart))
		{
			bucket->tFinish = NOW_STEADY_TS_MS - tuneStartMonotonicBase;
			if(gpGlobalConfig->enableMicroEvents && (type == PROFILE_BUCKET_DECRYPT_VIDEO || type == PROFILE_BUCKET_DECRYPT_AUDIO
												 || type == PROFILE_BUCKET_LA_TOTAL || type == PROFILE_BUCKET_LA_NETWORK))
			{
				long long start = bucket->tStart + tuneStartMonotonicBase;
				addtuneEvent(type, start, (unsigned int)(bucket->tFinish - bucket->tStart), 200);
			}
			/*
			static const char *bucketName[PROFILE_BUCKET_TYPE_COUNT] =
			{
			"manifest",
			"playlist",
			"fragment",
			"key",
			"decrypt"
			"first-frame"
			};

			logprintf("aamp %7d (+%6d): %s\n",
			bucket->tStart,
			bucket->tFinish - bucket->tStart,
			bucketName[type]);
			*/
			bucket->complete = true;
		}
	}

	/**
	 * @brief Method to mark the end of a bucket, for which beginning is not marked
	 *
	 * @param[in] type - Bucket type
	 * @return void
	 */
	void ProfilePerformed(ProfilerBucketType type)
	{
		ProfileBegin(type);
		buckets[type].complete = true;
	}
};

/**
 * @brief Class for Timed Metadata
 */
class TimedMetadata
{
public:

	/**
	 * @brief TimedMetadata Constructor
	 */
	TimedMetadata() : _timeMS(0), _name(""), _content(""), _id(""), _durationMS(0) {}

	/**
	 * @brief TimedMetadata Constructor
	 *
	 * @param[in] timeMS - Time in milliseconds
	 * @param[in] name - Metadata name
	 * @param[in] content - Metadata content
	 */
	TimedMetadata(double timeMS, std::string name, std::string content, std::string id, double durMS) : _timeMS(timeMS), _name(name), _content(content), _id(id), _durationMS(durMS) {}

public:
	double      _timeMS;     /**< Time in milliseconds */
	std::string _name;       /**< Metadata name */
	std::string _content;    /**< Metadata content */
	std::string _id;         /**< Id of the timedMetadata. If not available an Id will bre created */
	double      _durationMS; /**< Duration in milliseconds */
};


/**
 * @brief Function pointer for the idle task
 * @param[in] arg - Arguments
 * @return Idle task status
 */
typedef int(*IdleTask)(void* arg);

/**
 * @brief Function pointer for the destroy task
 *
 * @param[in] arg - Arguments
 *
 */
typedef void(*DestroyTask)(void * arg);

/**
 * @brief To store Set Cookie: headers and X-Reason headers in HTTP Response
 */
struct httpRespHeaderData {
	httpRespHeaderData() : type(0), data("")
	{
	}
	int type;             /**< Header type */
	std::string data;     /**< Header value */
};


/**
 * @brief  Structure of the event listener list
 */
struct ListenerData {
	AAMPEventListener* eventListener;   /**< Event listener */
	ListenerData* pNext;                /**< Next listener */
};

/**
 * @brief Class representing the AAMP player's private instance, which is not exposed to outside world.
 */
class PrivateInstanceAAMP
{

	enum AAMP2ReceiverMsgType
	{
	    E_AAMP2Receiver_TUNETIME,
	    E_AAMP2Receiver_EVENTS,
	    E_AAMP2Receiver_MsgMAX
	};

	typedef struct AAMP_PACKED _AAMP2ReceiverMsg
	{
	    unsigned int type;
	    unsigned int length;
	    char data[1];
	}AAMP2ReceiverMsg;

	#define AAMP2ReceiverMsgHdrSz (sizeof(AAMP2ReceiverMsg)-1)

public:
	/**
	 * @brief Get profiler bucket type
	 *
	 * @param[in] mediaType - Media type. eg: Video, Audio, etc
	 * @param[in] isInitializationSegment - Initialization segment or not
	 * @return Bucket type
	 */
	ProfilerBucketType GetProfilerBucketForMedia(MediaType mediaType, bool isInitializationSegment)
	{
		switch (mediaType)
		{
		case eMEDIATYPE_VIDEO:
			return isInitializationSegment ? PROFILE_BUCKET_INIT_VIDEO : PROFILE_BUCKET_FRAGMENT_VIDEO;
		case eMEDIATYPE_AUDIO:
		default:
			return isInitializationSegment ? PROFILE_BUCKET_INIT_AUDIO : PROFILE_BUCKET_FRAGMENT_AUDIO;
		}
	}

	/**
	 * @brief Tune API
	 *
	 * @param[in] url - Asset URL
	 * @param[in] contentType - Content Type
	 * @param[in] bFirstAttempt - External initiated tune
	 * @param[in] bFinalAttempt - Final retry/attempt.
	 * @return void
	 */
	void Tune(const char *url, const char *contentType, bool bFirstAttempt = true, bool bFinalAttempt = false, const char *sessionUUID = NULL);

	/**
	 * @brief The helper function which perform tuning
	 *
	 * @param[in] tuneType - Type of tuning. eg: Normal, trick, seek to live, etc
	 * @return void
	 */
	void TuneHelper(TuneType tuneType);

	/**
	 * @brief Terminate the stream
	 *
	 * @param[in] newTune - New tune or not
	 * @return void
	 */
	void TeardownStream(bool newTune);

	/**
	 * @brief Send messages to Receiver over PIPE
	 *
	 * @param[in] str - Pointer to the message
	 * @param[in] nToWrite - Number of bytes in the message
	 * @return void
	 */
	void SendMessageOverPipe(const char *str,int nToWrite);

	/**
	 * @brief Establish PIPE session with Receiver
	 *
	 * @return Success/Failure
	 */
	bool SetupPipeSession();

	/**
	 * @brief Close PIPE session with Receiver
	 *
	 * @return void
	 */
	void ClosePipeSession();

	/**
	 * @brief Send message to reciever over PIPE
	 *
	 * @param[in] type - Message type
	 * @param[in] data - Message data
	 * @return void
	 */
	void SendMessage2Receiver(AAMP2ReceiverMsgType type, const char *data);

	/**
	 * @brief Send tune events to receiver
	 *
	 * @param[in] success - Tune status
	 * @return void
	 */
	void sendTuneMetrics(bool success);

	/**
	 * @brief Convert media file type to profiler bucket type
	 *
	 * @param[in] fileType - Media filetype
	 * @return Profiler bucket type
	 */
	ProfilerBucketType mediaType2Bucket(MediaType fileType);

	std::vector< std::pair<long long,long> > mAbrBitrateData;

	pthread_mutex_t mLock;// = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutexattr_t mMutexAttr;

	class StreamAbstractionAAMP *mpStreamAbstractionAAMP; // HLS or MPD collector
	class CDAIObject *mCdaiObject;      // Client Side DAI Object
	std::queue<AAMPEvent> mAdEventsQ;   // A Queue of Ad events
	std::mutex mAdEventQMtx;            // Add events' queue protector
	bool mInitSuccess;	//TODO: Need to replace with player state
	StreamOutputFormat mFormat;
	StreamOutputFormat mAudioFormat;
	pthread_cond_t mDownloadsDisabled;
	bool mDownloadsEnabled;
	StreamSink* mStreamSink;

	ProfileEventAAMP profiler;
	bool licenceFromManifest;
	AudioType previousAudioType; /* Used to maintain previous audio type */

	CURL *curl[MAX_CURL_INSTANCE_COUNT];

	// To store Set Cookie: headers and X-Reason headers in HTTP Response
	httpRespHeaderData httpRespHeaders[MAX_CURL_INSTANCE_COUNT];
	//std::string cookieHeaders[MAX_CURL_INSTANCE_COUNT]; //To store Set-Cookie: headers in HTTP response
	char manifestUrl[MAX_URI_LENGTH];
	char tunedManifestUrl[MAX_URI_LENGTH];

	bool mbDownloadsBlocked;
	bool streamerIsActive;
	bool mTSBEnabled;
	bool mIscDVR;
	int mLiveOffset;
	bool mNewLiveOffsetflag;
	pthread_t fragmentCollectorThreadID;
	double seek_pos_seconds; // indicates the playback position at which most recent playback activity began
	int rate; // most recent (non-zero) play rate for non-paused content
	bool pipeline_paused; // true if pipeline is paused
	char language[MAX_LANGUAGE_TAG_LENGTH];  // current language set
	char mLanguageList[MAX_LANGUAGE_COUNT][MAX_LANGUAGE_TAG_LENGTH]; // list of languages in stream
	int mCurrentLanguageIndex; // Index of current selected lang in mLanguageList, this is used for VideoStat event data collection
	int  mMaxLanguageCount;
	VideoZoomMode zoom_mode;
	bool video_muted;
	int audio_volume;
	std::vector<std::string> subscribedTags;
	std::vector<TimedMetadata> timedMetadata;

	/* START: Added As Part of DELIA-28363 and DELIA-28247 */
	bool IsTuneTypeNew; /* Flag for the eTUNETYPE_NEW_NORMAL */
	/* END: Added As Part of DELIA-28363 and DELIA-28247 */

	long long trickStartUTCMS;
	long long playStartUTCMS;
	double durationSeconds;
	double culledSeconds;
	float maxRefreshPlaylistIntervalSecs;
	long long initialTuneTimeMs;
	AAMPEventListener* mEventListener;
	double mReportProgressPosn;
	long long mReportProgressTime;
	long long mAdPrevProgressTime;
	uint32_t mAdCurOffset;		//Start position in percentage
	uint32_t mAdDuration;
	std::string mAdProgressId;
	bool discardEnteringLiveEvt;
	bool mIsRetuneInProgress;
	pthread_cond_t mCondDiscontinuity;
	gint mDiscontinuityTuneOperationId;
	bool mIsVSS;       /**< Indicates if stream is VSS, updated during Tune*/
	long curlDLTimeout[MAX_CURL_INSTANCE_COUNT]; /**< To store donwload timeout of each curl instance*/

	/**
	 * @brief Curl initialization function
	 *
	 * @param[in] startIdx - Start index of the curl instance
	 * @param[in] instanceCount - Instance count
	 * @return void
	 */
	void CurlInit(int startIdx, unsigned int instanceCount);

	/**
	 *   @brief Sets Recorded URL from Manifest received form XRE.
	 *   @param[in] isrecordedUrl - flag to check for recordedurl in Manifest
	 */
	void SetTunedManifestUrl(bool isrecordedUrl = false);

	/**
	 *   @brief Gets Recorded URL from Manifest received form XRE.
	 *   @param[out] manifestUrl - for VOD and recordedUrl for FOG enabled
	 */
	const char *GetTunedManifestUrl();

	/**
	 * @brief Set curl timeout
	 *
	 * @param[in] timeout - Timeout value
	 * @param[in] instance - Curl instance
	 * @return void
	 */
	void SetCurlTimeout(long timeout, unsigned int instance = 0);

	/**
	 * @brief Storing audio language list
	 *
	 * @param[in] maxLangCount - Language count
	 * @param[in] langlist - Array of languages
	 * @return void
	 */
	void StoreLanguageList(int maxLangCount , char langlist[][MAX_LANGUAGE_TAG_LENGTH]);

	/**
	 * @brief Checking whether audio language supported
	 *
	 * @param[in] checkLanguage - Language to be checked
	 * @return True or False
	 */
	bool IsAudioLanguageSupported (const char *checkLanguage);

	/**
	 * @brief Terminate curl contexts
	 *
	 * @param[in] startIdx - First index
	 * @param[in] instanceCount - Instance count
	 * @return void
	 */
	void CurlTerm(int startIdx, unsigned int instanceCount);

	/**
	 * @brief Download a file from the server
	 *
	 * @param[in] remoteUrl - File URL
	 * @param[out] buffer - Pointer to the output buffer
	 * @param[out] effectiveUrl - Final URL after HTTP redirection
	 * @param[out] http_error - HTTP error code
	 * @param[in] range - Byte range
	 * @param[in] curlInstance - Curl instance to be used
	 * @param[in] resetBuffer - Flag to reset the out buffer
	 * @param[in] fileType - File type
	 * @return void
	 */
	bool GetFile(const char *remoteUrl, struct GrowableBuffer *buffer, char effectiveUrl[MAX_URI_LENGTH], long *http_error = NULL, const char *range = NULL,unsigned int curlInstance = 0, bool resetBuffer = true,MediaType fileType = eMEDIATYPE_DEFAULT, long *bitrate = NULL, int * fogError = NULL);

	/**
	 * @brief get Media Type in string
	 * @param[in] fileType - Type of Media
	 * @param[out] pointer to Media Type string
	 */
	const char* MediaTypeString(MediaType fileType);

	/**
	 * @brief Download fragment
	 *
	 * @param[in] bucketType - Bucket type of the profiler
	 * @param[in] fragmentUrl - Fragment URL
	 * @param[out] buffer - Pointer to the output buffer
	 * @param[out] len - Content length
	 * @param[in] curlInstance - Curl instance to be used
	 * @param[in] range - Byte range
	 * @param[in] fileType - File type
	 * @return void
	 */
	char *LoadFragment( ProfilerBucketType bucketType, const char *fragmentUrl, char *effectiveUrl, size_t *len, unsigned int curlInstance = 0, const char *range = NULL,long * http_code = NULL,MediaType fileType = eMEDIATYPE_MANIFEST,int * fogError = NULL);

	/**
	 * @brief Download fragment
	 *
	 * @param[in] bucketType - Bucket type of the profiler
	 * @param[in] fragmentUrl - Fragment URL
	 * @param[out] buffer - Pointer to the output buffer
	 * @param[in] curlInstance - Curl instance to be used
	 * @param[in] range - Byte range
	 * @param[in] fileType - File type
	 * @param[out] http_code - HTTP error code
	 * @return void
	 */
	bool LoadFragment( ProfilerBucketType bucketType, const char *fragmentUrl, char *effectiveUrl, struct GrowableBuffer *buffer, unsigned int curlInstance = 0, const char *range = NULL, MediaType fileType = eMEDIATYPE_MANIFEST, long * http_code = NULL, long *bitrate = NULL, int * fogError = NULL);

	/**
	 * @brief Push fragment to the gstreamer
	 *
	 * @param[in] mediaType - Media type
	 * @param[in] buffer - Pointer to the buffer
	 * @param[in] fragmentTime - Fragment start time
	 * @param[in] fragmentDuration - Fragment duration
	 * @return void
	 */
	void PushFragment(MediaType mediaType, char *ptr, size_t len, double fragmentTime, double fragmentDuration);

	/**
	 * @brief Push fragment to the gstreamer
	 *
	 * @param[in] mediaType - Media type
	 * @param[in] buffer - Pointer to the growable buffer
	 * @param[in] fragmentTime - Fragment start time
	 * @param[in] fragmentDuration - Fragment duration
	 * @return void
	 */
	void PushFragment(MediaType mediaType, GrowableBuffer* buffer, double fragmentTime, double fragmentDuration);

	/**
	 * @brief End of stream reached
	 *
	 * @param[in] mediaType - Media type
	 * @return void
	 */
	void EndOfStreamReached(MediaType mediaType);

	/**
	 * @brief Clip ended
	 *
	 * @param[in] mediaType - Media type
	 * @return void
	 */
	void EndTimeReached(MediaType mediaType);

	/**
	 * @brief Insert ad content
	 *
	 * @param[in] url - Ad url
	 * @param[in] positionSeconds - Ad start position in seconds
	 * @return void
	 */
	void InsertAd(const char *url, double positionSeconds);

	/**
	 * @brief Register event lister
	 *
	 * @param[in] eventType - Event type
	 * @param[in] eventListener - Event handler
	 * @return void
	 */
	void AddEventListener(AAMPEventType eventType, AAMPEventListener* eventListener);

	/**
	 * @brief Deregister event lister
	 *
	 * @param[in] eventType - Event type
	 * @param[in] eventListener - Event handler
	 * @return void
	 */
	void RemoveEventListener(AAMPEventType eventType, AAMPEventListener* eventListener);

	/**
	 * @brief Send events synchronously
	 *
	 * @param[in] e - Event object
	 * @return void
	 */
	void SendEventAsync(const AAMPEvent &e);

	/**
	 * @brief Handles errors and sends events to application if required.
	 * For download failures, use SendDownloadErrorEvent instead.
	 *
	 * @param[in] tuneFailure - Reason of error
	 * @param[in] description - Optional description of error
	 * @return void
	 */
	void SendErrorEvent(AAMPTuneFailure tuneFailure, const char *description = NULL, bool isRetryEnabled = true);

	

	void SendDRMMetaData(const AAMPEvent &e);

	/**
	 * @brief Handles DRM errors and sends events to application if required.
	 * @param[in] tuneFailure Reason of error
	 * @param[in] error_code Drm error code (http, curl or secclient)
	 */
	void SendDrmErrorEvent(AAMPTuneFailure tuneFailure,long error_code, bool isRetryEnabled = true);

	/**
	 * @brief Handles download errors and sends events to application if required.
	 *
	 * @param[in] tuneFailure - Reason of error
	 * @param[in] error_code - HTTP error code/ CURLcode
	 * @return void
	 */
	void SendDownloadErrorEvent(AAMPTuneFailure tuneFailure,long error_code);

	/**
	 * @brief Sends Anomaly Error/warning messages
	 *
	 * @param[in] type - severity of message
	 * @param[in] format - format string
	 * args [in]  - multiple arguments based on format
	 * @return void
	 */
	void SendAnomalyEvent(AAMPAnomalyMessageType type, const char* format, ...);

	/**
	 * @brief Send events synchronously
	 *
	 * @param[in] e - Event object
	 * @return void
	 */
	void SendEventSync(const AAMPEvent &e);

	/**
	 * @brief Notify speed change
	 *
	 * @param[in] rate - New speed
	 * @return void
	 */
	void NotifySpeedChanged(int rate);

	/**
	 * @brief Notify bit rate change
	 *
	 * @param[in] bitrate - New bitrate
	 * @param[in] description - Description
	 * @param[in] width - Video width
	 * @param[in] height - Video height
	 * @param[in] GetBWIndex - Flag to get the bandwidth index
	 * @return void
	 */
	void NotifyBitRateChangeEvent(int bitrate , const char *description ,int width ,int height, bool GetBWIndex = false);

	/**
	 * @brief Notify when end of stream reached
	 *
	 * @return void
	 */
	void NotifyEOSReached();

	/**
	 * @brief Notify when entering live point
	 *
	 * @return void
	 */
	void NotifyOnEnteringLive();

	/**
	 * @brief Get persisted profile index
	 *
	 * @return Profile index
	 */
	int  GetPersistedProfileIndex() {return mPersistedProfileIndex;}

	/**
	 * @brief Set persisted profile index
	 *
	 * @param[in] profile - Profile index
	 * @return void
	 */
	void SetPersistedProfileIndex(int profile){mPersistedProfileIndex = profile;}

	/**
	 * @brief Set persisted bandwidth
	 *
	 * @param[in] bandwidth - Bandwidth in bps
	 * @return void
	 */
	void SetPersistedBandwidth(long bandwidth) {mAvailableBandwidth = bandwidth;}

	/**
	 * @brief Get persisted bandwidth
	 *
	 * @return Bandwitdh
	 */
	long GetPersistedBandwidth(){return mAvailableBandwidth;}

	/**
	 * @brief Update playlist duration
	 *
	 * @param[in] seconds - Duration in seconds
	 * @return void
	 */
	void UpdateDuration(double seconds);

	/**
	 * @brief Update playlist culling
	 *
	 * @param[in] culledSeconds - Seconds to be culled
	 * @return void
	 */
	void UpdateCullingState(double culledSeconds);

	/**
	 *   @brief  Update playlist refresh inrerval
	 *
	 *   @param[in]  maxIntervalSecs - Interval in seconds
	 *   @return void
	 */
	void UpdateRefreshPlaylistInterval(float maxIntervalSecs);

	/**
	 *   @brief Report progress event
	 *
	 *   @return void
	 */
	void ReportProgress(void);

	/**
	 *   @brief Report Ad progress event
	 *
	 *   @return void
	 */
	void ReportAdProgress(void);

	/**
	 *   @brief Get asset duration in milliseconds
	 *
	 *   @return Duration in ms.
	 */
	long long GetDurationMs(void);

	/**
	 *   @brief Get playback position in milliseconds
	 *
	 *   @return Position in ms.
	 */
	long long GetPositionMs(void);

	/**
	 *   @brief  API to send audio/video stream into the sink.
	 *
	 *   @param[in]  mediaType - Type of the media.
	 *   @param[in]  ptr - Pointer to the buffer.
	 *   @param[in]  len - Buffer length.
	 *   @param[in]  fpts - Presentation Time Stamp.
	 *   @param[in]  fdts - Decode Time Stamp
	 *   @param[in]  duration - Buffer duration.
	 *   @return void
	 */
	void SendStream(MediaType mediaType, const void *ptr, size_t len, double fpts, double fdts, double fDuration);

	/**
	 *   @brief  API to send audio/video stream into the sink.
	 *
	 *   @param[in]  mediaType - Type of the media.
	 *   @param[in]  buffer - Pointer to the GrowableBuffer.
	 *   @param[in]  fpts - Presentation Time Stamp.
	 *   @param[in]  fdts - Decode Time Stamp
	 *   @param[in]  fDuration - Buffer duration.
	 *   @return void
	 */
	void SendStream(MediaType mediaType, GrowableBuffer* buffer, double fpts, double fdts, double fDuration);

	/**
	 * @brief Setting the stream sink
	 *
	 * @param[in] streamSink - Pointer to the stream sink
	 * @return void
	 */
	void SetStreamSink(StreamSink* streamSink);

	/**
	 * @brief Checking if the stream is live or not
	 *
	 * @return True or False
	 */
	bool IsLive(void);

	/**
	 * @brief Stop playback
	 *
	 * @return void
	 */
	void Stop(void);

	/**
	 * @brief Checking whether TSB enabled or not
	 *
	 * @return True or False
	 */
	bool IsTSBSupported() { return mTSBEnabled;}

	/**
	 * @brief Checking whether CDVR in progress
	 *
	 * @return True or False
	 */
	bool IsInProgressCDVR() {return (IsLive() && IsCDVRContent());}
	/**
	* @brief Checking whether CDVR Stream or not
	*
	* @return True or False
	*/
	bool IsCDVRContent() { return (mContentType==ContentType_CDVR || mIscDVR);}
	/**
	 * @brief Report timed metadata
	 *
	 * @param[in] timeMS - Time in milliseconds
	 * @param[in] szName - Metadata name
	 * @param[in] szContent - Metadata content
	 * @param[in] nb - ContentSize
	 * @param[in] id - Identifier of the TimedMetadata
	 * @param[in] durationMS - Duration in milliseconds
	 * @return void
	 */
	void ReportTimedMetadata(double timeMS, const char* szName, const char* szContent, int nb, const char* id = "", double durationMS = 0);

	/**
	 * @brief sleep only if aamp downloads are enabled.
	 * interrupted on aamp_DisableDownloads() call
	 *
	 * @param[in] timeInMs
	 * @return void
	 */
	void InterruptableMsSleep(int timeInMs);

#ifdef AAMP_HARVEST_SUPPORT_ENABLED

	/**
	 * @brief Collect decrypted fragments
	 *
	 * @param[in] modifyCount - Collect only the configured number of fragments
	 * @return void
	 */
	bool HarvestFragments(bool modifyCount = true);
#endif


	/**
	 * @brief Get download disable status
	 *
	 * @return void
	 */
	bool DownloadsAreEnabled(void);

	/**
	 * @brief Stop downloads of all tracks.
	 * Used by aamp internally to manage states
	 *
	 * @return void
	 */
	void StopDownloads();

	/**
 	 * @brief Resume downloads of all tracks.
	 * Used by aamp internally to manage states
	 *
	 * @return void
	 */
	void ResumeDownloads();

	/**
	 * @brief Stop downloads for a track.
	 * Called from StreamSink to control flow
	 *
	 * @param[in] Media type
	 * @return void
	 */
	void StopTrackDownloads(MediaType);

	/**
 	 * @brief Resume downloads for a track.
	 * Called from StreamSink to control flow
	 *
	 * @param[in] Media type
	 * @return void
	 */
	void ResumeTrackDownloads(MediaType);

	/**
	 *   @brief Block the injector thread until the gstreanmer needs buffer.
	 *
	 *   @param[in] cb - Callback helping to perform additional tasks, if gst doesn't need extra data
	 *   @param[in] periodMs - Delay between callbacks
	 *   @param[in] track - Track id
	 *   @return void
	 */
	void BlockUntilGstreamerWantsData(void(*cb)(void), int periodMs, int track);

	/**
	 *   @brief Notify the tune complete event
	 *
	 *   @return void
	 */
	void LogTuneComplete(void);

	/**
	 *   @brief Profile first frame displayed
	 *
	 *   @return void
	 */
	void LogFirstFrame(void);

	/**
	 *   @brief Drm license acquisition end profiling
	 *
	 *   @return void
	 */
	void LogDrmInitComplete(void);

	/**
	 *   @brief Drm decrypt begin profiling
	 *
	 *   @param[in] bucketType - Bucket Id
	 *   @return void
	 */
	void LogDrmDecryptBegin( ProfilerBucketType bucketType );

	/**
	 *   @brief Drm decrypt end profiling
	 *
	 *   @param[in] bucketType - Bucket Id
	 *   @return void
	 */
	void LogDrmDecryptEnd( ProfilerBucketType bucketType );
	
	/**
	 *   @brief Get manifest URL
	 *
	 *   @return Manifest URL
	 */
	char *GetManifestUrl(void)
	{
		return manifestUrl;
	}

	/**
	 *   @brief Set manifest URL
	 *
	 *   @param[in] url - Manifest URL
	 *   @return void
	 */
	void SetManifestUrl(const char *url)
	{
		strncpy(manifestUrl, url, MAX_URI_LENGTH);
		manifestUrl[MAX_URI_LENGTH-1]='\0';
	}

	/**
	 *   @brief First frame received notification
	 *
	 *   @return void
	 */
	void NotifyFirstFrameReceived(void);

	/**
	 *   @brief GStreamer operation start
	 *
	 *   @return void
	 */
	void SyncBegin(void);

	/**
	 * @brief GStreamer operation end
	 *
	 * @return void
	 */
	void SyncEnd(void);


	/**
	 * @brief Get seek position
	 *
	 * @return Position in seconds
	 */
	double GetSeekBase(void);

	/**
	 * @brief Reset bandwidth value
	 * Artificially resetting the bandwidth. Low for quicker tune times
	 *
	 * @param[in] bitsPerSecond - bps
	 * @param[in] trickPlay		- Is trickplay mode
	 * @param[in] profile		- Profile id.
	 * @return void
	 */
	void ResetCurrentlyAvailableBandwidth(long bitsPerSecond,bool trickPlay,int profile=0);

	/**
	 * @brief Get the current network bandwidth
	 *
	 * @return Available bandwidth in bps
	 */
	long GetCurrentlyAvailableBandwidth(void);


	/**
	 * @brief Abort ongoing downloads and returns error on future downloads
	 * Called while stopping fragment collector thread
	 *
	 * @return void
	 */
	void DisableDownloads(void);

	/**
	 * @brief Enable downloads after aamp_DisableDownloads.
	 * Called after stopping fragment collector thread
	 *
	 * @return void
	 */
	void EnableDownloads(void);

	/**
	 *   @brief Register event listener
	 *
	 *   @param[in] eventListener - Handle to event listener
	 *   @return void
	 */
	void RegisterEvents(AAMPEventListener* eventListener)
	{
		mEventListener = eventListener;
	}

	/**
	 *   @brief Schedule retune
	 *
	 *   @param[in] errorType - Current error type
	 *   @param[in] trackType - Video/Audio
	 *   @return void
	 */
	void ScheduleRetune(PlaybackErrorType errorType, MediaType trackType);

	/**
	 * @brief PrivateInstanceAAMP Constructor
	 */
	PrivateInstanceAAMP();

	/**
	 * @brief PrivateInstanceAAMP Destructor
	 */
	~PrivateInstanceAAMP();

	PrivateInstanceAAMP(const PrivateInstanceAAMP&) = delete;

	PrivateInstanceAAMP& operator=(const PrivateInstanceAAMP&) = delete;

	/**
	 *   @brief Set video rectangle
	 *
	 *   @param[in] x - Left
	 *   @param[in] y - Top
	 *   @param[in] w - Width
	 *   @param[in] h - Height
	 *   @return void
	 */
	void SetVideoRectangle(int x, int y, int w, int h);

	/**
	 *   @brief Signal discontinuity of track.
	 *   Called from StreamAbstractionAAMP to signal discontinuity
	 *
	 *   @param[in] track - Media type
	 *   @return true if discontinuity is handled.
	 */
	bool Discontinuity(MediaType);

	/**
	 *   @brief Set video zoom mode
	 *
	 *   @param[in] zoom - Video zoom mode
	 *   @return void
	 */
	void SetVideoZoom(VideoZoomMode zoom);

	/**
	 *   @brief Set video mute state
	 *
	 *   @param[in] muted - muted or unmuted
	 *   @return void
	 */
	void SetVideoMute(bool muted);

	/**
	 *   @brief Set audio volume
	 *
	 *   @param[in] volume - Volume level
	 *   @return void
	 */
	void SetAudioVolume(int volume);

	/**
	 *   @brief Set player state
	 *
	 *   @param[in] state - New state
	 *   @return void
	 */
	void SetState(PrivAAMPState state);

	/**
	 *   @brief Get player state
	 *
	 *   @param[out] state - Player state
	 *   @return void
	 */
	void GetState(PrivAAMPState &state);

	/**
	 *   @brief Add idle task to the gstreamer
	 *
	 *   @param[in] task - Task
	 *   @param[in] arg - Arguments
	 *   @return void
	 */
	static void AddIdleTask(IdleTask task, void* arg);
	/**
	*   @brief Add high priority idle task to the gstreamer
	*
	*   @param[in] task - Task
	*   @param[in] arg - Arguments
	*
	*   @return void
	*/
	static gint AddHighIdleTask(IdleTask task, void* arg,DestroyTask dtask=NULL);

	/**
	 *   @brief Check sink cache empty
	 *
	 *   @param[in] mediaType - Audio/Video
	 *   @return true: empty, false: not empty
	 */
	bool IsSinkCacheEmpty(MediaType mediaType);

	/**
	 *   @brief Notify fragment caching complete
	 *
	 *   @return void
	 */
	void NotifyFragmentCachingComplete();

	/**
	 *   @brief Send tuned event
	 *
	 *   @return success or failure
	 */
	bool SendTunedEvent();

	/**
	 *   @brief Send VideoEndEvent
	 *
	 *   @return success or failure
	 */
	bool SendVideoEndEvent();

	/**
	 *   @brief Check if fragment buffering needed
	 *
	 *   @return true: needed, false: not needed
	 */
	bool IsFragmentBufferingRequired();

	/**
	 *   @brief Get player video size
	 *
	 *   @param[out] w - Width
	 *   @param[out] h - Height
	 *   @return void
	 */
	void GetPlayerVideoSize(int &w, int &h);

	/**
	 *   @brief Set callback as event pending
	 *
	 *   @param[in] id - Callback id.
	 *   @return void
	 */
	void SetCallbackAsPending(gint id);

	/**
	 *   @brief Set callback as event dispatched
	 *
	 *   @param[in] id - Callback id.
	 *   @return void
	 */
	void SetCallbackAsDispatched(gint id);


	/**
	 *   @brief Add custom HTTP header
	 *
	 *   @param[in] headerName  - Header name
	 *   @param[in] headerValue - Header value
	 *   @return void
	 */
	void AddCustomHTTPHeader(std::string headerName, std::vector<std::string> headerValue);

	/**
	 *   @brief Set license server URL
	 *
	 *   @param[in] url - server URL
	 *   @param[in] drmType - DRM type (PR/WV) for which the URL has to be used, global by default
	 *   @return void
	 */
	void SetLicenseServerURL(const char* url, DRMSystems drmType = eDRM_MAX_DRMSystems);

	/**
	 *   @brief Set Preferred DRM.
	 *
	 *   @param[in] drmType - Preferred DRM type
	 *   @return void
	 */
	void SetPreferredDRM(DRMSystems drmType);

	/**
	 *   @brief Notification from the stream abstraction that a new SCTE35 event is found.
	 *
	 *   @param[in] Adbreak's unique identifier.
	 *   @param[in] Break start time in milli seconds.
	 *   @param[in] Break duration in milli seconds
	 *   @param[in] SCTE35 binary object.
	 */
	void FoundSCTE35(const std::string &adBreakId, uint64_t startMS, uint32_t breakdur, std::string &scte35);

	/**
	 *   @brief Setting the alternate contents' (Ads/blackouts) URL
	 *
	 *   @param[in] Adbreak's unique identifier.
	 *   @param[in] Individual Ad's id
	 *   @param[in] Ad URL
	 */
	void SetAlternateContents(const std::string &adBreakId, const std::string &adId, const std::string &url);

	/**
	 *   @brief Send status of Ad manifest downloading & parsing
	 *
	 *   @param[in] Ad's unique identifier.
	 *   @param[in] Manifest status (success/Failure)
	 *   @param[in] Ad playback start time in milliseconds
	 *   @param[in] Ad's duration in milliseconds
	 */
	void SendAdResolvedEvent(const std::string &adId, bool status, uint64_t startMS=0, uint64_t durationMs=0);

	/**
	 * @brief Send status of Events corresponding to Ad reservation
	 *
	 * @param[in] tuneFailure - Reason of error
	 * @param[in] error_code - HTTP error code/ CURLcode
	 *
	 * @return void
	 */
	void SendAdReservationEvent(AAMPEventType type, const std::string &adBreakId, uint64_t position, bool immediate=false);

	/**
	 * @brief Send status of Events corresponding to Ad placement
	 *
	 * @param[in] tuneFailure - Reason of error
	 * @param[in] error_code - HTTP error code/ CURLcode
	 *
	 * @return void
	 */
	void SendAdPlacementEvent(AAMPEventType type, const std::string &adId, uint32_t position, uint32_t adOffset, uint32_t adDuration, bool immediate=false, long error_code=0);

	/**
	 *   @brief Set anonymous request true or false
	 *
	 *   @param[in] isAnonymous - New status
	 *   @return void
	 */
	void SetAnonymousRequest(bool isAnonymous);

	/**
	 *   @brief Set frames per second for VOD trickplay
	 *
	 *   @param[in] vodTrickplayFPS - FPS count
	 *   @return void
	 */
	void SetVODTrickplayFPS(int vodTrickplayFPS);

	/**
	 *   @brief Set frames per second for linear trickplay
	 *
	 *   @param[in] linearTrickplayFPS - FPS count
	 *   @return void
	 */
	void SetLinearTrickplayFPS(int linearTrickplayFPS);

	/**
	 *   @brief Sets live offset [Sec]
	 *
	 *   @param[in] SetLiveOffset - Live Offset
	 *   @return void
	 */

	void SetLiveOffset(int SetLiveOffset);

	/**
	 *   @brief Set stall error code
	 *
	 *   @param[in] errorCode - Stall error code
	 *   @return void
	 */
	void SetStallErrorCode(int errorCode);

	/**
	 *   @brief Set stall timeout
	 *
	 *   @param[in] timeoutMS - Timeout in milliseconds
	 *   @return void
	 */
	void SetStallTimeout(int timeoutMS);

	/**
	 *   @brief To set the Playback Position reporting interval.
	 *
	 *   @param  reportIntervalMS - playback reporting interval in milliseconds.
	 */
	void SetReportInterval(int reportIntervalMS);
	/**
	 *   @brief Send stalled error
	 *
	 *   @return void
	 */
	void SendStalledErrorEvent();

	/**
	 *   @brief Is discontinuity pending to process
	 *
	 *   @return void
	 */
	bool IsDiscontinuityProcessPending();

	/**
	 *   @brief Process pending discontinuity
	 *
	 *   @return void
	 */
	void ProcessPendingDiscontinuity();

	/**
	 *   @brief Notify if first buffer processed by gstreamer
	 *
	 *   @return void
	 */
	void NotifyFirstBufferProcessed();

	/**
	 *   @brief Update audio language selection
	 *
	 *   @param[in] lang - Language
	 *   @return void
	 */
	void UpdateAudioLanguageSelection(const char *lang);

	/**
	 *   @brief Get stream type
	 *
	 *   @return Stream type
	 */
	int getStreamType();

	/**
	 *   @brief Get stream type as printable format
	 *
	 *   @return Stream type as string
	 */
	std::string getStreamTypeString();

	/**
	 *   @brief Get current drm
	 *
	 *   @return current drm
	 */
	DRMSystems GetCurrentDRM();

	/**
	 *   @brief Set DRM type
	 *
	 *   @param[in] drm - New DRM type
	 *   @return void
	 */
	void setCurrentDrm(DRMSystems drm) { mCurrentDrm = drm; }

	/**
	 *   @brief Check if current  playback is from local TSB
	 *
	 *   @return true: yes, false: no
	 */
	bool IsLocalPlayback() { return mIsLocalPlayback; }

#ifdef AAMP_MPD_DRM
	/**
	 * @brief Extracts / Generates MoneyTrace string
	 * @param[out] customHeader - Generated moneytrace is stored
	 *
	 * @return void
	 */
	void GetMoneyTraceString(std::string &);
#endif /* AAMP_MPD_DRM */

	/**
	 *   @brief Notify the decryption completion of the fist fragment.
	 *
	 *   @return void
	 */
	void NotifyFirstFragmentDecrypted();

	/**
	 *   @brief  Get PTS of first sample.
	 *
	 *   @return PTS of first sample
	 */
	double GetFirstPTS();

	/**
	 *   @brief Check if Live Adjust is required for current content. ( For "vod/ivod/ip-dvr/cdvr/eas", Live Adjust is not required ).
	 *
	 *   @return False if the content is either vod/ivod/cdvr/ip-dvr/eas
	 */
	bool IsLiveAdjustRequired();

	/**
	 *   @brief  Generate media metadata event based on args passed.
	 *
	 *   @param[in] durationMs - duration of playlist in milliseconds
	 *   @param[in] langList - list of audio language available in asset
	 *   @param[in] bitrateList - list of video bitrates available in asset
	 *   @param[in] hasDrm - indicates if asset is encrypted/clear
	 *   @param[in] isIframeTrackPresent - indicates if iframe tracks are available in asset
	 */
	void SendMediaMetadataEvent(double durationMs, std::set<std::string>langList, std::vector<long> bitrateList, bool hasDrm, bool isIframePresent);

	/**
	 *   @brief  Generate supported speeds changed event based on arg passed.
	 *
	 *   @param[in] isIframeTrackPresent - indicates if iframe tracks are available in asset
	 */
	void SendSupportedSpeedsChangedEvent(bool isIframeTrackPresent);

	/**
	 *   @brief  Get Sequence Number from URL
	 *
	 *   @param[in] fragmentUrl fragment Url
	 *   @returns Sequence Number if found in fragment Url else 0
	 */
	long long GetSeqenceNumberfromURL(const char *fragmentUrl);

	/**
	 *   @brief To set the initial bitrate value.
	 *
	 *   @param[in] initial bitrate to be selected
	 */
	void SetInitialBitrate(long bitrate);

	/**
	 *   @brief To set the initial bitrate value for 4K assets.
	 *
	 *   @param[in] initial bitrate to be selected for 4K assets
	 */
	void SetInitialBitrate4K(long bitrate4K);

	/**
	 *   @brief To set the network download timeout value.
	 *
	 *   @param[in] preferred timeout value
	 */
	void SetNetworkTimeout(long timeout);

	/**
	 *   @brief To set the download buffer size value
	 *
	 *   @param[in] preferred download buffer size
	 */
	void SetDownloadBufferSize(int bufferSize);

	/**
	 *   @brief  Check if tune completed or not.
	 *
	 *   @return true, if tune completed.
	 */
	bool IsTuneCompleted();

	/**
	 *   @brief Check if ABR enabled for this playback session.
	 *
	 *   @return true if ABR enabled.
	 */
	bool CheckABREnabled(void) { return mABREnabled; }
	/**
	 *   @brief Set a preferred bitrate for video.
	 *
	 *   @param[in] preferred bitrate.
	 */
	void SetVideoBitrate(long bitrate);

	/**
	 *   @brief Get preferred bitrate for video.
	 *
	 *   @return preferred bitrate.
	 */
	long GetVideoBitrate();

	/**
	 *   @brief To set the network proxy
	 *
	 *   @param[in] network proxy to use
	 */
	void SetNetworkProxy(const char * proxy);

	/**
	 *   @brief To set the proxy for license request
	 *
	 *   @param[in] proxy to use for license request
	 */
	void SetLicenseReqProxy(const char * licenseProxy);

	/**
	 *   @brief To get the proxy for license request
	 *
	 *   @return proxy to use for license request
	 */
	char * GetLicenseReqProxy() { return mLicenseProxy; };

	/**
	 *   @brief Set is Live flag
	 *
	 *   @param[in] isLive - is Live flag
	 *   @return void
	 */
	void SetIsLive(bool isLive)  {mIsLive = isLive; }

	/**
	 *   @brief Signal trick mode discontinuity to stream sink
	 *
	 *   @return void
	 */
	void SignalTrickModeDiscontinuity();

	/**
	 *   @brief IsNewTune Function to check if tune is New tune or retune
	 *
	 *   @return Bool True on new tune
	 */
	bool IsNewTune()  { return ((eTUNETYPE_NEW_NORMAL == mTuneType) || (eTUNETYPE_NEW_SEEK == mTuneType)); }

	/**
	 *   @brief  return service zone, extracted from locator &sz URI parameter
	 *   @return std::string
	 */
	std::string & getServiceZone() { return mServiceZone; }

	/**
	 *   @brief Check if current stream is muxed
	 *
	 *   @return true if current stream is muxed
	 */
	bool IsMuxedStream();

	/**
	 *   @brief To set the curl stall timeout value
	 *
	 *   @param[in] curl stall timeout
	 */
	void SetDownloadStallTimeout(long stallTimeout);

	/**
	 *   @brief To set the curl download start timeout value
	 *
	 *   @param[in] curl download start timeout
	 */
	void SetDownloadStartTimeout(long startTimeout);

	/**
	 *   @brief updates download metrics to VideoStat object,
	 *
	 *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
	 *   @param[in]  bitrate - bitrate ( bits per sec )
	 *   @param[in]  curlOrHTTPErrorCode - download curl or http error
     *   @param[in]  strUrl :  URL in case of faulures
	 *   @return void
	 */
	void UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, const char * strUrl);

	/**
	 *   @brief updates time shift buffer status
	 *
	 *   @param[in]  btsbAvailable - true if TSB supported
	 *   @return void
	 */
	void UpdateVideoEndTsbStatus(bool btsbAvailable);

	/**
	*   @brief updates download metrics to VideoStat object, this is used for VideoFragment as it takes duration for calcuation purpose.
	*
	*  @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
	*   @param[in]  bitrate - bitrate ( bits per sec )
	*   @param[in]  curlOrHTTPErrorCode - download curl or http error
	*   @param[in]  strUrl :  URL in case of faulures
	*   @param[in] keyChanged : if DRM key changed then it is set to true
	*   @param[in] isEncrypted : if fragment is encrypted then it is set to true
	*   @return void
	*/
	void UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, const char * strUrl, double duration, bool keyChanged, bool isEncrypted);
    
	/**
	*   @brief updates download metrics to VideoStat object, this is used for VideoFragment as it takes duration for calcuation purpose.
	*
	*   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
	*   @param[in]  bitrate - bitrate ( bits per sec )
	*   @param[in]  curlOrHTTPErrorCode - download curl or http error
	*   @param[in]  strUrl :  URL in case of faulures
	*   @return void
	*/
	void UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, const char * strUrl, double duration);


	/**
	 *   @brief updates abr metrics to VideoStat object,
	 *
	 *   @param[in]  AAMPAbrInfo - abr info
	 *   @return void
	 */
	void UpdateVideoEndMetrics(AAMPAbrInfo & info);

	/**
	 *   @brief Converts lang index to Audio Track type
	 *
	 *   @param[in] int - Audio Lang Index
	 *   @return VideoStatTrackType
	 */
	VideoStatTrackType ConvertAudioIndexToVideoStatTrackType(int Index);


	/**
	 * @brief Stop injection for a track.
	 * Called from StopInjection
	 *
	 * @param[in] Media type
	 * @return void
	 */
	void StopTrackInjection(MediaType type);

	/**
	 * @brief Resume injection for a track.
	 * Called from StartInjection
	 *
	 * @param[in] Media type
	 * @return void
	 */
	void ResumeTrackInjection(MediaType type);

private:

	/**
	 *   @brief Load the configuration lazily
	 *
	 *   @return void
	 */
	static void LazilyLoadConfigIfNeeded(void);

	/**
	 *   @brief updates mServiceZone ( service zone) member with string extracted from locator &sz URI parameter
	 *   @param  url - stream url with vss service zone info as query string
	 *   @return std::string
	 */
	void ExtractServiceZone(const char * url);

	/**
	 *   @brief Schedule Event
	 *
	 *   @param[in]  e - Pointer to the event descriptor
	 *   @return void
	 */
	void ScheduleEvent(struct AsyncEventDescriptor* e);

	/**
	 * @brief Deliver all pending Ad events to JSPP
	 *
	 *
	 * @return void
	 */
	void DeliverAdEvents();

	/**
	 *   @brief Set Content Type
	 *
	 *   @param[in]  url - Media URL
	 *   @param[in]  contentType - Content type
	 *   @return void
	 */
	void SetContentType(const char *url, const char *contentType);


    /**
     *   @brief Set Content Type
     *
     *   @return string
     */
	std::string GetContentTypString();

	ListenerData* mEventListeners[AAMP_MAX_NUM_EVENTS];
	TuneType mTuneType;
	int m_fd;
	bool mIsLive;
	bool mTuneCompleted;
	bool mFirstTune;			//To identify the first tune after load.
	int mfirstTuneFmt;			//First Tune Format HLS(0) or DASH(1)
	int  mTuneAttempts;			//To distinguish between new tune & retries with redundant over urls.
	long long mPlayerLoadTime;
	PrivAAMPState mState;
	long long lastUnderFlowTimeMs[AAMP_TRACK_COUNT];
	bool mbTrackDownloadsBlocked[AAMP_TRACK_COUNT];
	bool mIsDash;
	DRMSystems mCurrentDrm;
	int  mPersistedProfileIndex;
	long mAvailableBandwidth;
	bool mProcessingDiscontinuity;
	bool mDiscontinuityTuneOperationInProgress;
	ContentType mContentType;
	bool mTunedEventPending;
	bool mSeekOperationInProgress;
	std::map<gint, bool> mPendingAsyncEvents;
	std::unordered_map<std::string, std::vector<std::string>> mCustomHeaders;
	bool mIsFirstRequestToFOG;
	bool mIsLocalPlayback; /** indicates if the playback is from FOG(TSB/IP-DVR) */
	bool mABREnabled;                   /**< Flag that denotes if ABR is enabled */
	long mUserRequestedBandwidth;       /**< preferred bitrate set by user */
	char *mNetworkProxy;                /**< proxy for download requests */
	char *mLicenseProxy;                /**< proxy for license acquisition */
	// VSS license parameters
	std::string mServiceZone; // part of url

	CVideoStat * mVideoEnd;
	std::string  mTraceUUID; // Trace ID unique to tune
	double mTimeToTopProfile;
	double mTimeAtTopProfile;
	double mPlaybackDuration; // Stores Total of duration of VideoDownloaded, it is not accurate playback duration but best way to find playback duration.

	bool mTrackInjectionBlocked[AAMP_TRACK_COUNT];
#ifdef PLACEMENT_EMULATION
	int mNumAds2Place;
	std::string sampleAdBreakId;
#endif
};

#endif // PRIVAAMP_H
