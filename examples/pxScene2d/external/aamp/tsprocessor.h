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
* @file tsprocessor.h
* @brief Header file for play context
*/

#ifndef _TSPROCESSOR_H
#define _TSPROCESSOR_H

#include <stdio.h>
#include <pthread.h>

#include <vector>

#define MAX_PIDS (8) //PMT Parsing

/**
* @struct RecordingComponent
* @brief Stores information of a audio/video component.
*/
struct RecordingComponent
{
	/** Service information type such as PAT, PMT, NIT, SDT, BAT, etc */
	int siType;
	/** Elementary stream type of Audio or Video, such as 1B=H.264, 0x01=MPEG1, 0x10=MPEG4, 0x03=MPEG1 Audio, 0x04=MPEG2 Audio, etc */
	int elemStreamType;
	/** PID associated to the audio, video or data component */
	int pid;
	/** Language such as eng, hin, etc */
	char *associatedLanguage;
	/** Descriptor tags, each byte value will represent one descriptor tag up to max MAX_DESCRIPTOR (4)*/
	unsigned int descriptorTags;
};

/**
* @enum _PlayMode
* @brief Defines the parameters required for Recording Playback
*/
typedef enum _PlayMode
{
	/** Playing a recording in normal mode */
	PlayMode_normal,
	/** Playing with I-Frame, P-Frame and B-Frame */
	PlayMode_retimestamp_IPB,
	/** Playing with I-Frame and P-Frame */
	PlayMode_retimestamp_IandP,
	/** Playing a recording with I-Frame only */
	PlayMode_retimestamp_Ionly,
	/** Playing a recording with rewind mode */
	PlayMode_reverse_GOP,
} PlayMode;


// Maximum number of bytes needed to examine in a start code
#define SCAN_REMAINDER_SIZE_MPEG2 (7)
#define SCAN_REMAINDER_SIZE_H264 (29)
#if (SCAN_REMAINDER_SIZE_MPEG2 > SCAN_REMAINDER_SIZE_H264)
#define MAX_SCAN_REMAINDER_SIZE SCAN_REMAINDER_SIZE_MPEG2
#else
#define MAX_SCAN_REMAINDER_SIZE SCAN_REMAINDER_SIZE_H264
#endif

class Demuxer;

/**
 * @enum StreamOperation
 * @brief Operation done by TSProcessor
 */
typedef enum
{
	/** Normal operation when no demuxing is required. Used with playersinkbin */
	eStreamOp_NONE,
	/** Demux and inject audio only*/
	eStreamOp_DEMUX_AUDIO,
	/** Demux and inject video only*/
	eStreamOp_DEMUX_VIDEO,
	/** Demux and inject audio and video*/
	eStreamOp_DEMUX_ALL,
	/** When video contains PAT/PMT, audio needs to be queued until video is processed
	 * used with playersinkbin*/
	eStreamOp_QUEUE_AUDIO,
	/** Send queued audio after video*/
	eStreamOp_SEND_VIDEO_AND_QUEUED_AUDIO
}StreamOperation;

/**
 * @enum TrackToDemux
 * @brief Track to demux
 */
typedef enum
{
	/** Demux and send video only*/
	ePC_Track_Video,
	/** Demux and send audio only*/
	ePC_Track_Audio,
	/** Demux and send audio and video*/
	ePC_Track_Both
}TrackToDemux;

/**
* @class TSProcessor
* @brief MPEG TS Processor. Supports software Demuxer/ PTS re-stamping for trickmode.
*/
class TSProcessor
{
   public:
      TSProcessor(class PrivateInstanceAAMP *aamp, StreamOperation streamOperation, int track = 0, TSProcessor* peerTSProcessor = NULL);
      TSProcessor(const TSProcessor&) = delete;
      TSProcessor& operator=(const TSProcessor&) = delete;
      ~TSProcessor();
      bool sendSegment( char *segment, size_t& size, double position, double duration, bool discontinuous, bool &ptsError);
      void setRate(double rate, PlayMode mode);
      void setThrottleEnable(bool enable);

      /**
       * @brief Set frame rate for trick mode
       * @param[in] frameRate rate per second
       */
      void setFrameRateForTM( int frameRate)
      {
         m_apparentFrameRate = frameRate;
      }
      void abort();
      void reset();
      void flush();

   protected:
      void getAudioComponents(const RecordingComponent** audioComponentsPtr, int &count);
      void sendQueuedSegment(long long basepts = 0, double updatedStartPositon = -1);
      void setBasePTS(double position, long long pts);

   private:
      class PrivateInstanceAAMP *aamp;
      void setPlayMode( PlayMode mode );
      void processPMTSection( unsigned char* section, int sectionLength );
      void reTimestamp( unsigned char *&packet, int length );
      int insertPatPmt( unsigned char *buffer, bool trick, int bufferSize );
      void insertPCR( unsigned char *packet, int pid );
      bool generatePATandPMT( bool trick, unsigned char **buff, int *bufflen, bool bHandleMCTrick = false);
      void putPmtByte( unsigned char* &pmt, int& index, unsigned char byte, int pmtPid );
      bool processStartCode( unsigned char *buffer, bool& keepScanning, int length, int base );
      void checkIfInterlaced( unsigned char *packet, int length );
      bool readTimeStamp( unsigned char *p, long long &value );
      void writeTimeStamp( unsigned char *p, int prefix, long long TS );
      long long readPCR( unsigned char *p );
      void writePCR( unsigned char *p, long long PCR, bool clearExtension );
      unsigned char* createNullPFrame( int width, int height, int *nullPFrameLen );
      bool processSeqParameterSet( unsigned char *p, int length );
      void processPictureParameterSet( unsigned char *p, int length );
      void processScalingList( unsigned char *& p, int& mask, int size );
      unsigned int getBits( unsigned char *& p, int& mask, int bitCount );
      void putBits( unsigned char *& p, int& mask, int bitCount, unsigned int value );
      unsigned int getUExpGolomb( unsigned char *& p, int& mask );
      int getSExpGolomb( unsigned char *& p, int& mask );
      void updatePATPMT();
      void abortUnlocked();

      bool m_needDiscontinuity;
      long long m_currStreamOffset;
      long long m_currPTS;
      long long m_currTimeStamp;
      int m_currFrameNumber;
      int m_currFrameLength;
      long long m_currFrameOffset;

      bool m_trickExcludeAudio;      
      int m_PatPmtLen;
      unsigned char *m_PatPmt;
      int m_PatPmtTrickLen;
      unsigned char *m_PatPmtTrick;
      int m_PatPmtPcrLen;
      unsigned char *m_PatPmtPcr;
      int m_patCounter;
      int m_pmtCounter;

      PlayMode m_playMode;
      PlayMode m_playModeNext;
      double m_playRate;
      double m_absPlayRate;
      double m_playRateNext;
      double m_apparentFrameRate;
      int m_packetSize;
      int m_ttsSize;
      int m_pcrPid;
      int m_videoPid;
      bool m_haveBaseTime;
      bool m_haveEmittedIFrame;
      bool m_haveUpdatedFirstPTS;
      int m_pcrPerPTSCount;
      long long m_baseTime;
      long long m_segmentBaseTime;
      long long m_basePCR;
      long long m_prevRateAdjustedPCR;
      long long m_currRateAdjustedPCR;
      long long m_currRateAdjustedPTS;
      unsigned char m_continuityCounters[8192];
      unsigned char m_pidFilter[8192];
      unsigned char m_pidFilterTrick[8192];
      
      unsigned char *m_nullPFrame;
      int m_nullPFrameLength;
      int m_nullPFrameNextCount;
      int m_nullPFrameOffset;
      int m_nullPFrameWidth;
      int m_nullPFrameHeight;
      int m_frameWidth;
      int m_frameHeight;
      bool m_scanForFrameSize;
      int m_scanRemainderSize;
      int m_scanRemainderLimit;
      unsigned char m_scanRemainder[MAX_SCAN_REMAINDER_SIZE*3];
      bool m_isH264;
      bool m_isMCChannel;
      bool m_isInterlaced;
      bool m_isInterlacedKnown;
      bool m_throttle;
      bool m_haveThrottleBase;
      long long m_lastThrottleContentTime;
      long long m_lastThrottleRealTime;
      long long m_baseThrottleContentTime;
      long long m_baseThrottleRealTime;
      long long m_throttlePTS;
      bool m_insertPCR;
      int m_emulationPreventionCapacity;
      int m_emulationPreventionOffset;
      unsigned char * m_emulationPrevention;
      bool m_scanSkipPacketsEnabled;

      /**
       * @struct H264SPS
       * @brief Holds SPS parameters
       */
      typedef struct _H264SPS
      {
         int picOrderCountType;
         int maxPicOrderCount;
         int log2MaxFrameNumMinus4;
         int log2MaxPicOrderCntLsbMinus4;
         int separateColorPlaneFlag;
         int frameMBSOnlyFlag;
      } H264SPS;

      /**
       * @struct H264PPS
       * @brief Holds PPS parameters
       */
      typedef struct _H264PPS
      {
         int spsId;
      } H264PPS;

      H264SPS m_SPS[32];
      H264PPS m_PPS[256];
      int m_currSPSId;      
      int m_picOrderCount;
      bool m_updatePicOrderCount;

      bool processBuffer(unsigned char *buffer, int size, bool &insPatPmt);
      long long getCurrentTime();
      bool throttle(); 
      void sendDiscontinuity(double position);
      void setupThrottle(int segmentDurationMs);
      bool demuxAndSend(const void *ptr, size_t len, double fTimestamp, double fDuration, bool discontinuous, TrackToDemux trackToDemux = ePC_Track_Both);
      bool msleep(long long throttleDiff);

      bool m_havePAT; //!< Set to 1 when PAT buffer examined and loaded all program specific information
      int m_versionPAT; //!< Pat Version number
      int m_program; //!< Program number in the corresponding program map table
      int m_pmtPid; //!< For which PID the program information is available such as, audio pid, video pid, stream types, etc

      bool m_havePMT; //!< When PMT buffer examined the value is set to 1
      int m_versionPMT; //!< Version number for PMT which is being examined

      bool m_indexAudio; //!< If PCR Pid matches with any Audio PIDs associated for a recording, the value will be set to 1
      bool m_haveAspect; //!< Set to 1 when it found aspect ratio of current video
      bool m_haveFirstPTS; //!< The value is set to 1 if first PTS found from a recording after examining few KB of initial data
      long long m_currentPTS; //!< Store the current PTS value of a recording
      int m_pmtCollectorNextContinuity; //!< Keeps next continuity counter for PMT packet at the time of examine the TS Buffer
      int m_pmtCollectorSectionLength; //!< Update section length while examining PMT table
      int m_pmtCollectorOffset; //!< If it is set, process subsequent parts of multi-packet PMT
      unsigned char *m_pmtCollector; //!< A buffer pointer to hold PMT data at the time of examining TS buffer
      bool m_scrambledWarningIssued;
      bool m_checkContinuity;
      int videoComponentCount, audioComponentCount;
      RecordingComponent videoComponents[MAX_PIDS], audioComponents[MAX_PIDS];

      long long m_actualStartPTS;

      int m_throttleMaxDelayMs;
      int m_throttleMaxDiffSegments;
      int m_throttleDelayIgnoredMs;
      int m_throttleDelayForDiscontinuityMs;
      pthread_cond_t m_throttleCond;
      pthread_cond_t m_basePTSCond;
      pthread_mutex_t m_mutex;
      bool m_enabled;
      bool m_processing;
      int m_framesProcessedInSegment;
      long long m_lastPTSOfSegment;
      StreamOperation m_streamOperation;
      Demuxer* m_vidDemuxer;
      Demuxer* m_audDemuxer;
      bool m_demux;
      TSProcessor* m_peerTSProcessor;
      int m_packetStartAfterFirstPTS;
      unsigned char * m_queuedSegment;
      double m_queuedSegmentPos;
      double m_queuedSegmentDuration;
      size_t m_queuedSegmentLen;
      bool m_queuedSegmentDiscontinuous;
      double m_startPosition;
      int m_track;
      long long m_last_frame_time;
      bool m_demuxInitialized;
      long long m_basePTSFromPeer;
};

#endif

