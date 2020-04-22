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
 * @file fragmentcollector_progressive.cpp
 * @brief Fragment collector implementation for progressive mp3/mp4 playback
 */

#include "fragmentcollector_progressive.h"
#include "priv_aamp.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>

struct StreamWriteCallbackContext
{
    bool sentTunedEvent;
    PrivateInstanceAAMP *aamp;
    StreamWriteCallbackContext() : aamp(NULL), sentTunedEvent(false)
    {
    }
};

/**
Test Content Examples:
http://127.0.0.1:8080/overlay360.mp4
- unable to play on OSX with error:
/GstPipeline:AAMPGstPlayerPipeline/GstPlayBin:playbin0/GstURIDecodeBin:uridecodebin0/GstDecodeBin:decodebin0/GstQTDemux:qtdemux0:
 no 'moov' atom within the first 10 MB

http://d3rlna7iyyu8wu.cloudfront.net/Atmos/MP4/shattered-3Mb.mp4
- plays on OSX but without audio

http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4
- plays on OSX with video/audio

 TODO: consider harvesting fixed size chunks instead of immediately tiny ranges of bytes to gstreamer
 TODO: consider config for required bytes to collect before starting gstreamer pipeline
 TODO: if we can't keep up with required bandwidth we don't have luxury of ABR ramp down; need to inform app about buffering status
 TODO: consider config for required bytes to collect after buffer runs dry before updating state
 TODO: consider harvesting additional configurable buffer of bytes even while throttled so they are "on deck" and ready to inject

 TODO: pause/play testing
 
 TODO: we are not yet reporting a valid duration, either for entire stream or for each injected chunk
 is there a nice way to get real mp4 file rate from initial headers?
 if we can't determine mp4 logical duration, can/should we report -1 as length?

 TODO: progress reporting/testing
 TODO: what to return for available bitrates?
 TODO: FF/REW should be disabled, with out any available rates
 TODO: trickplay requests must return error

 TODO: errors that can occur at tune time or mid-stream
 TODO: extend StreamFile with applicable features from aamp->GetFile
 TODO: profiling - stream based, not fragment based
 */

/**
 * @param ptr
 * @param size always 1, per curl documentation
 * @param nmemnb number of bytes advertised in this callback
 * @param user app-specific context
 */
static size_t StreamWriteCallback( void *ptr, size_t size, size_t nmemb, void *userdata )
{
    StreamWriteCallbackContext *context = (StreamWriteCallbackContext *)userdata;
    struct PrivateInstanceAAMP *aamp = context->aamp;
    //pthread_mutex_lock(&context->aamp->mLock);
    if( context->aamp->mDownloadsEnabled)
    {
       // TODO: info logging is normally only done up until first frame rendered, but even so is too noisy for below, since CURL write callback yields many small chunks
        AAMPLOG_INFO("StreamWriteCallback(%d bytes)\n", nmemb);
        // throttle download speed if gstreamer isn't hungry
        aamp->BlockUntilGstreamerWantsData( NULL/*CB*/, 0.0/*periodMs*/, eMEDIATYPE_VIDEO );
        double fpts = 0.0;
        double fdts = 0.0;
        double fDuration = 2.0; // HACK!
        float position = 0.0;
        if( nmemb>0 )
        {
           aamp->SendStream( eMEDIATYPE_VIDEO, ptr, nmemb, fpts, fdts, fDuration);
           if( !context->sentTunedEvent )
           { // send TunedEvent after first chunk injected - this is hint for XRE to hide the "tuning overcard"
               aamp->SendTunedEvent(false);
               context->sentTunedEvent = true;
           }
       }
   }
   else
   {
       logprintf("write_callback - interrupted\n");
       nmemb = 0;
   }
   //pthread_mutex_unlock(&context->aamp->mLock);
   return nmemb;
}

void StreamAbstractionAAMP_PROGRESSIVE::StreamFile( const char *uri, long *http_error )
{ // TODO: move to main_aamp


    long http_code = -1;
    AAMPLOG_INFO("StreamFile: %s\n", uri );
    CURL *curl = curl_easy_init();
    if (curl)
    {
        StreamWriteCallbackContext context;
        context.aamp = aamp;
        context.sentTunedEvent = false;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamWriteCallback );
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&context );
        curl_easy_setopt(curl, CURLOPT_URL, uri );
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "aamp-progressive/1.0"); // TODO: use same user agent string normally used by AAMP
        CURLcode res = curl_easy_perform(curl); // synchronous; callbacks allow interruption
        if( res == CURLE_OK)
        { // all data collected
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        }
        if (http_error)
        {
            *http_error = http_code;
        }
        curl_easy_cleanup(curl);
    }
}
/**
  * TODO: harvest chunks from large mp3/mp4
 */
void StreamAbstractionAAMP_PROGRESSIVE::FetcherLoop()
{
    std::string contentUrl = aamp->GetManifestUrl();
    std::string effectiveUrl;
    long http_error;
    
    if(gpGlobalConfig->useAppSrcForProgressivePlayback)
    {
	    StreamFile( contentUrl.c_str(), &http_error );
    }
    else
    {
	    // send TunedEvent after first chunk injected - this is hint for XRE to hide the "tuning overcard"
	    aamp->SendTunedEvent(false);
    }

    while( aamp->DownloadsAreEnabled() )
    {
        aamp->InterruptableMsSleep( 1000 );
    }
}

/**
 * @brief Fragment collector thread
 * @param arg Pointer to PrivateStreamAbstractionMPD object
 * @retval NULL
 */
static void * FragmentCollector(void *arg)
{
    if(aamp_pthread_setname(pthread_self(), "aampProgressiveFetch"))
    {
        logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
    }
    StreamAbstractionAAMP_PROGRESSIVE *context = (StreamAbstractionAAMP_PROGRESSIVE *)arg;
    context->FetcherLoop();
    return NULL;
}

/**
 *   @brief  Initialize a newly created object.
 *   @note   To be implemented by sub classes
 *   @param  tuneType to set type of object.
 *   @retval true on success
 *   @retval false on failure
 */
AAMPStatusType StreamAbstractionAAMP_PROGRESSIVE::Init(TuneType tuneType)
{
    AAMPStatusType retval = eAAMPSTATUS_OK;
    aamp->CurlInit(eCURLINSTANCE_VIDEO, AAMP_TRACK_COUNT,aamp->GetNetworkProxy());
    bool newTune = aamp->IsNewTune();
    aamp->IsTuneTypeNew = false;
    for (int i = 0; i < AAMP_TRACK_COUNT; i++)
    {
        aamp->SetCurlTimeout(aamp->mNetworkTimeoutMs, (AampCurlInstance) i);
    }
    return retval;
}

/**
 *   @brief  Initialize a newly created object.
 *   @note   To be implemented by sub classes
 *   @param  tuneType to set type of object.
 *   @retval true on success
 *   @retval false on failure
 */
/*
AAMPStatusType StreamAbstractionAAMP_PROGRESSIVE::Init(TuneType tuneType)
{
    AAMPStatusType retval = eAAMPSTATUS_OK;
    bool newTune = aamp->IsNewTune();
    aamp->IsTuneTypeNew = false;
    return retval;
}
 */

/**
 * @brief StreamAbstractionAAMP_MPD Constructor
 * @param aamp pointer to PrivateInstanceAAMP object associated with player
 * @param seek_pos Seek position
 * @param rate playback rate
 */
StreamAbstractionAAMP_PROGRESSIVE::StreamAbstractionAAMP_PROGRESSIVE(class PrivateInstanceAAMP *aamp,double seek_pos, float rate): StreamAbstractionAAMP(aamp),
fragmentCollectorThreadStarted(false), fragmentCollectorThreadID(0)
{
    trickplayMode = (rate != AAMP_NORMAL_PLAY_RATE);
}

/**
 * @brief StreamAbstractionAAMP_PROGRESSIVE Destructor
 */
StreamAbstractionAAMP_PROGRESSIVE::~StreamAbstractionAAMP_PROGRESSIVE()
{
}

/**
 *   @brief  Starts streaming.
 */
void StreamAbstractionAAMP_PROGRESSIVE::Start(void)
{
    pthread_create(&fragmentCollectorThreadID, NULL, &FragmentCollector, this);
    fragmentCollectorThreadStarted = true;
}

/**
*   @brief  Stops streaming.
*/
void StreamAbstractionAAMP_PROGRESSIVE::Stop(bool clearChannelData)
{
    if(fragmentCollectorThreadStarted)
    {
        aamp->DisableDownloads();

        int rc = pthread_join(fragmentCollectorThreadID, NULL);
        if (rc != 0)
        {
            logprintf("%s:%d ***pthread_join failed, returned %d\n", __FUNCTION__, __LINE__, rc);
        }
        fragmentCollectorThreadStarted = false;

        aamp->EnableDownloads();
    }
 }

/**
 * @brief Stub implementation
 */
void StreamAbstractionAAMP_PROGRESSIVE::DumpProfiles(void)
{ // STUB
}

/**
 * @brief Get output format of stream.
 *
 * @param[out]  primaryOutputFormat - format of primary track
 * @param[out]  audioOutputFormat - format of audio track
 */
void StreamAbstractionAAMP_PROGRESSIVE::GetStreamFormat(StreamOutputFormat &primaryOutputFormat, StreamOutputFormat &audioOutputFormat)
{
    primaryOutputFormat = FORMAT_ISO_BMFF;
    audioOutputFormat = FORMAT_NONE;
}

/**
 *   @brief Return MediaTrack of requested type
 *
 *   @param[in]  type - track type
 *   @retval MediaTrack pointer.
 */
MediaTrack* StreamAbstractionAAMP_PROGRESSIVE::GetMediaTrack(TrackType type)
{
    return NULL;//mPriv->GetMediaTrack(type);
}

/**
 * @brief Get current stream position.
 *
 * @retval current position of stream.
 */
double StreamAbstractionAAMP_PROGRESSIVE::GetStreamPosition()
{
    return 0.0;
}

/**
 *   @brief Get stream information of a profile from subclass.
 *
 *   @param[in]  idx - profile index.
 *   @retval stream information corresponding to index.
 */
StreamInfo* StreamAbstractionAAMP_PROGRESSIVE::GetStreamInfo(int idx)
{
    return NULL;
}

/**
 *   @brief  Get PTS of first sample.
 *
 *   @retval PTS of first sample
 */
double StreamAbstractionAAMP_PROGRESSIVE::GetFirstPTS()
{
    return 0.0;
}

double StreamAbstractionAAMP_PROGRESSIVE::GetBufferedDuration()
{
	return -1.0;
}

/**
 * @brief Get index of profile corresponds to bandwidth
 * @param[in] bitrate Bitrate to lookup profile
 * @retval profile index
 */
int StreamAbstractionAAMP_PROGRESSIVE::GetBWIndex(long bitrate)
{
    return 0;
}

/**
 * @brief To get the available video bitrates.
 * @ret available video bitrates
 */
std::vector<long> StreamAbstractionAAMP_PROGRESSIVE::GetVideoBitrates(void)
{ // STUB
    return std::vector<long>();
}

/*
* @brief Gets Max Bitrate avialable for current playback.
* @ret long MAX video bitrates
*/
long StreamAbstractionAAMP_PROGRESSIVE::GetMaxBitrate()
{ // STUB
    return 0;
}

/**
 * @brief To get the available audio bitrates.
 * @ret available audio bitrates
 */
std::vector<long> StreamAbstractionAAMP_PROGRESSIVE::GetAudioBitrates(void)
{ // STUB
    return std::vector<long>();
}

/**
*   @brief  Stops injecting fragments to StreamSink.
*/
void StreamAbstractionAAMP_PROGRESSIVE::StopInjection(void)
{ // STUB - discontinuity related
}

/**
*   @brief  Start injecting fragments to StreamSink.
*/
void StreamAbstractionAAMP_PROGRESSIVE::StartInjection(void)
{ // STUB - discontinuity related
}







