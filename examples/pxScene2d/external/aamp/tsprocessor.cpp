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
* @file tsprocessor.cpp
* @brief Source file for player context
*/

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include "priv_aamp.h"

#include "tsprocessor.h"


/**
 * @brief NOP function used to controll logging
 * @param[in] format printf style format string
 */
void print_nop(const char *format, ...){}

#ifdef LOG_ENABLE_TRACE
#define TRACE1 logprintf("PC: TRACE1 %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define TRACE2 logprintf("PC: TRACE2 %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define TRACE3 logprintf("PC: TRACE3 %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define TRACE4 logprintf("PC: TRACE4 %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#else
#define TRACE1  print_nop
#define TRACE2  print_nop
#define TRACE3  print_nop
#define TRACE4  print_nop
#endif

#ifndef LOG_ENABLE
#define DEBUG print_nop
#define INFO print_nop
#else
#define INFO logprintf("PC: INFO %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define DEBUG logprintf("PC: DEBUG %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#endif

#define LOG_WARNINGS_AND_ERRORS

#undef ERROR
#ifndef LOG_WARNINGS_AND_ERRORS
#define NOTICE print_nop
#define WARNING print_nop
#define ERROR print_nop
#define FATAL print_nop
#else
#define NOTICE logprintf("PC: %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define WARNING logprintf("PC: WARNING %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define ERROR logprintf("PC: ERROR %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#define FATAL logprintf("PC: FATAL %s:%d:", __FUNCTION__, __LINE__ ); logprintf
#endif


#define DUMP_PACKET 0

#define PACKET_SIZE (188)
#define MAX_PACKET_SIZE (208)
#define FIXED_FRAME_RATE (10)
#define FRAME_WIDTH_MAX (1920)
#define FRAME_HEIGHT_MAX (1080)
#define WAIT_FOR_DATA_MILLISEC 100
#define WAIT_FOR_DATA_MAX_RETRIES 1
#define MAX_PMT_SECTION_SIZE (1021)
#define PATPMT_MAX_SIZE (2*1024)

/** Maximum PTS value */
#define MAX_PTS (0x1FFFFFFFF)

/** Maximum descriptor present for a elementary stream */
#define MAX_DESCRIPTOR (4)

#define I_FRAME (0x1)
#define P_FRAME (0x2)
#define B_FRAME (0x4)
#define ALL_FRAMES (0x7)
#define SEQ_START (0x10)
#define IDX_BUFF_SIZE (128*1024)

/*Throttle Default Parameters - From DVRManager*/
#define DEFAULT_THROTTLE_MAX_DELAY_MS 500
#define DEFAULT_THROTTLE_MAX_DIFF_SEGMENTS_MS 400
#define DEFAULT_THROTTLE_DELAY_IGNORED_MS 200
#define DEFAULT_THROTTLE_DELAY_FOR_DISCONTINUITY_MS 2000

#define PES_STATE_WAITING_FOR_HEADER  0
#define PES_STATE_GETTING_HEADER  1
#define PES_STATE_GETTING_HEADER_EXTENSION  2
#define PES_STATE_GETTING_ES  3

#define PAYLOAD_UNIT_START(packetStart) ( packetStart[1] & 0x40)
#define CONTAINS_PAYLOAD(packetStart) ( packetStart[3] & 0x10)
#define IS_PES_PACKET_START(a) ( (a[0] == 0 )&& (a[1] == 0 ) &&(a[2] == 1 ))
#define PES_OPTIONAL_HEADER_PRESENT(pesStart) ( ( pesStart[6] & 0xC0) == 0x80 )
#define PES_HEADER_LENGTH 6
#define PES_OPTIONAL_HEADER_LENGTH(pesStart) (pesStart[PES_HEADER_LENGTH+2])
#define PES_MIN_DATA (PES_HEADER_LENGTH+3)
#define ADAPTATION_FIELD_PRESENT(mpegbuf) ((mpegbuf[3] & 0x20) == 0x20)
#define PES_PAYLOAD_LENGTH(pesStart) (pesStart[4]<<8|pesStart[5])
#define MAX_FIRST_PTS_OFFSET (45000) /*500 ms*/

//#define DEBUG_DEMUX_TRACK 1
#ifdef DEBUG_DEMUX_TRACK
#define DEBUG_DEMUX(a...) { \
	if (type == DEBUG_DEMUX_TRACK || DEBUG_DEMUX_TRACK == 0xff) \
	{ \
		logprintf("PC: DEBUG_DEMUX %s:%d:Track %d : ", __FUNCTION__, __LINE__, DEBUG_DEMUX_TRACK );\
		logprintf(a); \
	}\
	else \
	{ \
		DEBUG(a); \
	}\
}
#else
#define DEBUG_DEMUX DEBUG
#endif


/**
 * @class Demuxer
 * @brief Software demuxer of MPEGTS
 */
class Demuxer
{
private:
	class PrivateInstanceAAMP *aamp;
	int pes_state;
	int pes_header_ext_len;
	int pes_header_ext_read;
	GrowableBuffer pes_header;
	GrowableBuffer es;
	double position;
	double duration;
	unsigned long long base_pts;
	unsigned long long current_pts;
	unsigned long long current_dts;
	MediaType type;
	bool trickmode;
	bool finalized_base_pts;
	int sentESCount;


	/**
	 * @brief Sends elementary stream with proper PTS
	 */
	void send()
	{
		if ((base_pts > current_pts) || (current_dts && base_pts > current_dts))
		{
			WARNING("Discard ES Type %d position %f base_pts %llu current_pts %llu diff %f seconds length %d\n", type, position, base_pts, current_pts, (double)(base_pts - current_pts) / 90000, (int)es.len );
		}
		else
		{
			double pts = position;
			double dts;
			if (!trickmode)
			{
				pts += (double)(current_pts - base_pts) / 90000;
			}
			if (!trickmode && current_dts)
			{
				dts = position + (double)(current_dts - base_pts) / 90000;
			}
			else
			{
				dts = pts;
			}
			DEBUG_DEMUX("Send : pts %f dts %f\n", pts, dts);
			DEBUG_DEMUX("position %f base_pts %llu current_pts %llu diff %f seconds length %d\n", position, base_pts, current_pts, (double)(current_pts - base_pts) / 90000, (int)es.len );
			aamp->SendStream(type, es.ptr, es.len, pts, dts, duration);
			if (gpGlobalConfig->logging.info)
			{
				sentESCount++;
				if(0 == (sentESCount % 150 ))
				{
					logprintf("Demuxer::%s:%d: type %d sent %d packets\n", __FUNCTION__, __LINE__, (int)type, sentESCount);
				}
			}
		}
		es.len = 0;
	}

public:
	/**
	 * @brief Demuxer Constructor
	 * @param[in] aamp pointer to PrivateInstanceAAMP object associated with demux
	 * @param[in] type Media type to be demuxed
	 */
	Demuxer(class PrivateInstanceAAMP *aamp,MediaType type) : aamp(aamp), pes_state(0),
		pes_header_ext_len(0), pes_header_ext_read(0), pes_header(),
		es(), position(0), duration(0), base_pts(0), current_pts(0),
		current_dts(0), type(type), trickmode(false), finalized_base_pts(false),
		sentESCount(0)
	{
		init(0, 0, false, true);
	}

	/**
	 * @brief Copy Constructor
	 */
	Demuxer(const Demuxer&) = delete;

	/**
	 * @brief Assignment operator overloading
	 */
	Demuxer& operator=(const Demuxer&) = delete;

	/**
	 * @brief Demuxer Destructor
	 */
	~Demuxer()
	{
		aamp_Free(&es.ptr);
		aamp_Free(&pes_header.ptr);
	}


	/**
	 * @brief Initialize demux
	 * @param[in] position start position
	 * @param[in] duration duration
	 * @param[in] trickmode true if trickmode
	 * @param[in] resetBasePTS true to reset base pts used for restamping
	 */
	void init(double position, double duration, bool trickmode, bool resetBasePTS)
	{
		this->position = position;
		this->duration = duration;
		this->trickmode = trickmode;
		if (resetBasePTS)
		{
			base_pts = -1;
		}
		current_dts = 0;
		current_pts = 0;
		finalized_base_pts = false;
		memset(&pes_header, 0x00, sizeof(GrowableBuffer));
		memset(&es, 0x00, sizeof(GrowableBuffer));
		sentESCount = 0;
		pes_state = PES_STATE_WAITING_FOR_HEADER;
		DEBUG_DEMUX("init : position %f, duration %f resetBasePTS %d\n", position, duration, resetBasePTS);
	}


	/**
	 * @brief flush es buffer and reset demux state
	 */
	void flush()
	{
		if (es.len > 0)
		{
			INFO("demux : sending remaining bytes. es.len %d\n", (int)es.len);
			send();
		}
		AAMPLOG_INFO("Demuxer::%s:%d: count %d in duration %f\n", __FUNCTION__, __LINE__, sentESCount, duration);
		reset();
	}


	/**
	 * @brief reset demux state
	 */
	void reset()
	{
		aamp_Free(&es.ptr);
		aamp_Free(&pes_header.ptr);
		memset(&pes_header, 0x00, sizeof(GrowableBuffer));
		memset(&es, 0x00, sizeof(GrowableBuffer));
		sentESCount = 0;
	}


	/**
	 * @brief Set base PTS used for re-stamping
	 * @param[in] basePTS new base PTS
	 * @param[in] final true if base PTS is finalized
	 */
	void setBasePTS(unsigned long long basePTS, bool final)
	{
		if (!trickmode)
		{
			NOTICE("Type[%d], basePTS %llu final %d\n", (int)type, basePTS, (int)final);
		}
		base_pts = basePTS;
		finalized_base_pts = final;
	}

	/**
	 * @brief Get base PTS used for re-stamping
	 * @retval base PTS used for re-stamping
	 */
	unsigned long long getBasePTS()
	{
		return base_pts;
	}


	/**
	 * @brief Process a TS packet
	 * @param[in] packetStart start of buffer containing packet
	 * @param[out] basePtsUpdated true if base PTS is updated
	 * @param[in] ptsError true if encountered PTS error.
	 */
	void processPacket(unsigned char * packetStart, bool &basePtsUpdated, bool &ptsError)
	{
		int adaptation_fieldlen = 0;
		basePtsUpdated = false;
		if (CONTAINS_PAYLOAD(packetStart))
		{
			if (ADAPTATION_FIELD_PRESENT(packetStart))
			{
				adaptation_fieldlen = packetStart[4];
			}
			int pesOffset = 4 + adaptation_fieldlen;
			if (ADAPTATION_FIELD_PRESENT(packetStart))
			{
				pesOffset++;
			}
			/*Store the pts/dts*/
			if (PAYLOAD_UNIT_START(packetStart))
			{
				if (es.len > 0)
				{
					send();
				}
				unsigned char* pesStart = packetStart + pesOffset;
				if (IS_PES_PACKET_START(pesStart))
				{
					if (PES_OPTIONAL_HEADER_PRESENT(pesStart))
					{
						if ((pesStart[7] & 0x80) && ((pesStart[9] & 0x20) == 0x20))
						{
							unsigned long long v; // this is to hold a 64bit integer, lowest 36 bits contain a timestamp with markers
							v = (unsigned long long) (pesStart[9] & 0x0F) << 32
								| (unsigned long long) pesStart[10] << 24 | (unsigned long long) pesStart[11] << 16
								| (unsigned long long) pesStart[12] << 8 | (unsigned long long) pesStart[13];
							unsigned long long timeStamp = 0;
							timeStamp |= (v >> 3) & (0x0007ULL << 30); // top 3 bits, shifted left by 3, other bits zeroed out
							timeStamp |= (v >> 2) & (0x7fff << 15); // middle 15 bits
							timeStamp |= (v >> 1) & (0x7fff << 0); // bottom 15 bits
							current_pts = timeStamp;
							DEBUG("PTS updated %llu\n", current_pts);
							if(!finalized_base_pts)
							{
								finalized_base_pts = true;
								if(!trickmode)
								{
									if (-1 == base_pts)
									{
										base_pts = current_pts - MAX_FIRST_PTS_OFFSET;
										WARNING("Type[%d] base_pts not initialized, updated to %llu\n", type, base_pts);
									}
									else
									{
										long long delta = current_pts - base_pts;
										if (delta > MAX_FIRST_PTS_OFFSET)
										{
											unsigned long long orig_base_pts = base_pts;
											base_pts = current_pts - MAX_FIRST_PTS_OFFSET;
											NOTICE("Type[%d] delta[%lld] > MAX_FIRST_PTS_OFFSET, current_pts[%llu] base_pts[%llu]->[%llu]\n", type, delta, current_pts, orig_base_pts, base_pts);
										}
										else if (delta < 0 )
										{
											WARNING("Type[%d] delta[%lld] < 0, base_pts[%llu]->[%llu]\n", type, delta, base_pts, current_pts);
											base_pts = current_pts;
										}
										else
										{
											NOTICE("Type[%d] PTS in range.delta[%lld] <= MAX_FIRST_PTS_OFFSET base_pts[%llu]\n", type, delta, base_pts);
										}
									}
								}
								if (-1 == base_pts)
								{
									base_pts = timeStamp;
									WARNING("base_pts not available, updated to pts %llu\n", timeStamp);
								}
								else if (base_pts > timeStamp)
								{
									WARNING("base_pts update from %llu to %llu\n", base_pts, timeStamp);
									base_pts = timeStamp;
								}
								basePtsUpdated = true;
							}
						}
						else
						{
							WARNING("PTS NOT present pesStart[7] & 0x80 = 0x%x pesStart[9]&0xF0 = 0x%x\n",
								pesStart[7] & 0x80, (pesStart[9] & 0x20));
						}

						if (((pesStart[7] & 0xC0) == 0xC0) && ((pesStart[14] & 0x1) == 0x01))
						{
							unsigned long long v; // this is to hold a 64bit integer, lowest 36 bits contain a timestamp with markers
							v = (unsigned long long) (pesStart[14] & 0x0F) << 32
								| (unsigned long long) pesStart[15] << 24 | (unsigned long long) pesStart[16] << 16
								| (unsigned long long) pesStart[17] << 8 | (unsigned long long) pesStart[18];
							unsigned long long timeStamp = 0;
							timeStamp |= (v >> 3) & (0x0007ULL << 30); // top 3 bits, shifted left by 3, other bits zeroed out
							timeStamp |= (v >> 2) & (0x7fff << 15); // middle 15 bits
							timeStamp |= (v >> 1) & (0x7fff << 0); // bottom 15 bits
							DEBUG("dts : %llu \n", timeStamp);

							current_dts = timeStamp;
						}
						else
						{
							DEBUG("DTS NOT present pesStart[7] & 0x80 = 0x%x pesStart[9]&0xF0 = 0x%x\n",
								pesStart[7] & 0x80, (pesStart[9] & 0x20));
						}
					}
					else
					{
						WARNING("Optional pes header NOT present pesStart[6] & 0xC0 = 0x%x\n", pesStart[6] & 0xC0);
					}
				}
				else
				{
					WARNING("Packet start prefix check failed 0x%x 0x%x 0x%x adaptation_fieldlen %d\n", pesStart[0],
						pesStart[1], pesStart[2], adaptation_fieldlen);
				}
				DEBUG(" PES_PAYLOAD_LENGTH %d\n", PES_PAYLOAD_LENGTH(pesStart));
			}
			if (current_pts < base_pts)
			{
				WARNING("current_pts[%llu] < base_pts[%llu]\n", current_pts, base_pts);
				ptsError = true;
				return;
			}
			/*PARSE PES*/
			{
				unsigned char * data = packetStart + pesOffset;
				int size = PACKET_SIZE - pesOffset;
				int bytes_to_read;
				if (PAYLOAD_UNIT_START(packetStart))
				{
					pes_state = PES_STATE_GETTING_HEADER;
					pes_header.len = 0;
					DEBUG_DEMUX("Payload Unit Start\n");
				}

				while (size > 0)
				{
					switch (pes_state)
					{
					case PES_STATE_WAITING_FOR_HEADER:
						WARNING("PES_STATE_WAITING_FOR_HEADER , discard data. type =%d size = %d\n", (int)type, size);
						size = 0;
						break;
					case PES_STATE_GETTING_HEADER:
						bytes_to_read = PES_MIN_DATA - pes_header.len;
						if (size < bytes_to_read)
						{
							bytes_to_read = size;
						}
						DEBUG("PES_STATE_GETTING_HEADER. size = %d, bytes_to_read =%d\n", size, bytes_to_read);
						aamp_AppendBytes(&pes_header, data, bytes_to_read);
						data += bytes_to_read;
						size -= bytes_to_read;
						if (pes_header.len == PES_MIN_DATA)
						{
							if (!IS_PES_PACKET_START(pes_header.ptr))
							{
								WARNING("Packet start prefix check failed 0x%x 0x%x 0x%x\n", pes_header.ptr[0],
									pes_header.ptr[1], pes_header.ptr[2]);
								pes_state = PES_STATE_WAITING_FOR_HEADER;
								break;
							}
							if (PES_OPTIONAL_HEADER_PRESENT(pes_header.ptr))
							{
								pes_state = PES_STATE_GETTING_HEADER_EXTENSION;
								pes_header_ext_len = PES_OPTIONAL_HEADER_LENGTH(pes_header.ptr);
								pes_header_ext_read = 0;
								DEBUG(
									"Optional header preset len = %d. Switching to PES_STATE_GETTING_HEADER_EXTENSION\n",
									pes_header_ext_len);
							}
							else
							{
								WARNING(
									"Optional header not preset pesStart[6] 0x%x bytes_to_read %d- switching to PES_STATE_WAITING_FOR_HEADER\n",
									pes_header.ptr[6], bytes_to_read);
								pes_state = PES_STATE_WAITING_FOR_HEADER;
							}
						}
						break;
					case PES_STATE_GETTING_HEADER_EXTENSION:
						bytes_to_read = pes_header_ext_len - pes_header_ext_read;
						if (bytes_to_read > size)
						{
							bytes_to_read = size;
						}
						data += bytes_to_read;
						size -= bytes_to_read;
						pes_header_ext_read += bytes_to_read;
						if (pes_header_ext_read == pes_header_ext_len)
						{
							pes_state = PES_STATE_GETTING_ES;
							DEBUG("Optional header read. switch to PES_STATE_GETTING_ES");
						}
						break;
					case PES_STATE_GETTING_ES:
						/*Handle padding?*/
						TRACE1("PES_STATE_GETTING_ES bytes_to_read = %d\n", size);
						aamp_AppendBytes(&es, data, size);
						size = 0;
						break;
					default:
						pes_state = PES_STATE_WAITING_FOR_HEADER;
						ERROR("Invalid pes_state. type =%d size = %d\n", (int)type, size);
						break;
					}
				}
			}
		}
		else
		{
			INFO("No payload in packet packetStart[3] 0x%x\n", packetStart[3]);
		}
		ptsError = false;
	}
};

#define rmf_osal_memcpy(d, s, n, dc, sc)  memcpy(d, s, n)

static unsigned long crc32_table[256];
static int crc32_initialized = 0;


/**
 * @brief Init CRC32 table
 */
static void init_crc32()
{
	if (crc32_initialized) return;
	unsigned int k, i, j;
	for (i = 0; i < 256; i++)
	{
		k = 0;
		for (j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1)
		{
			k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);
		}
		crc32_table[i] = k;
	}
	crc32_initialized = 1;
}


/**
 * @brief Get 32 bit CRC value
 * @param[in] data buffer containing data
 * @param[in] size length of data
 * @param[in] initial initial CRC
 * @retval 32 bit CRC value
 */
static uint32_t get_crc32(unsigned char *data, int size, uint32_t initial = 0xffffffff)
{
	int i;
	uint32_t result = initial;
	init_crc32();
	for (i = 0; i < size; i++)
	{
		result = (result << 8) ^ crc32_table[(result >> 24) ^ data[i]];
	}
	return result;
}


/**
 * @brief Dump TS packet
 * @param[in] packet buffer containing packet
 * @param[in] packetSize length of packet
 */
static void dumpPacket(unsigned char *packet, int packetSize)
{
	if (DUMP_PACKET)
	{
		int i;
		char buff[1024];

		int col = 0;
		int buffPos = 0;
		buffPos += sprintf(&buff[buffPos], "\n");
		for (i = 0; i < packetSize; ++i)
		{
			buffPos += snprintf(&buff[buffPos], (sizeof(buff) - buffPos), "%02X\n", packet[i]);
			++col;
			if (col == 8)
			{
				strcat(buff, " ");
				buffPos += 1;
			}
			if (col == 16)
			{
				buffPos += snprintf(&buff[buffPos], (sizeof(buff) - buffPos), "\n");
				col = 0;
			}
		}
		printf("%s\n", buff);
	}
}


/**
 * @brief dump TS packets
 * @param[in] packets buffer containing packets
 * @param[in] len length of packets
 * @param[in] packetSize size of TS packet
 */
static void dumpPackets(unsigned char *packets, int len, int packetSize)
{
	while (len)
	{
		dumpPacket(packets, packetSize);
		len -= packetSize;
		packets += packetSize;
	}
}

/**
 * @brief TSProcessor Constructor
 * @param[in] aamp Pointer to aamp associated with this TSProcessor
 * @param[in] streamOperation Operation to be done on injected data.
 * @param[in] track MediaType to be operated on. Not relavent for demux operation
 * @param[in] peerTSProcessor Peer TSProcessor used along with this in case of separate audio/video playlists
 */
TSProcessor::TSProcessor(class PrivateInstanceAAMP *aamp,StreamOperation streamOperation, int track, TSProcessor* peerTSProcessor)
	: m_needDiscontinuity(true),
	m_PatPmtLen(0), m_PatPmt(0), m_PatPmtTrickLen(0), m_PatPmtTrick(0), m_PatPmtPcrLen(0), m_PatPmtPcr(0),
	m_nullPFrame(0), m_nullPFrameLength(0), m_nullPFrameNextCount(0), m_nullPFrameOffset(0),
	m_emulationPreventionCapacity(0), m_emulationPreventionOffset(0), m_emulationPrevention(0), aamp(aamp),
	m_currStreamOffset(0), m_currPTS(-1), m_currTimeStamp(-1LL), m_currFrameNumber(-1),
	m_currFrameLength(0), m_currFrameOffset(-1LL), m_trickExcludeAudio(true), m_patCounter(0),
	m_pmtCounter(0), m_playMode(PlayMode_normal), m_playModeNext(PlayMode_normal), m_playRate(1.0f), m_absPlayRate(1.0f),
	m_playRateNext(1.0f), m_apparentFrameRate(FIXED_FRAME_RATE), m_packetSize(PACKET_SIZE), m_ttsSize(0),
	m_pcrPid(-1), m_videoPid(-1), m_haveBaseTime(false), m_haveEmittedIFrame(false), m_haveUpdatedFirstPTS(true),
	m_pcrPerPTSCount(0), m_baseTime(0), m_segmentBaseTime(0), m_basePCR(-1LL), m_prevRateAdjustedPCR(-1LL),
	m_currRateAdjustedPCR(0), m_currRateAdjustedPTS(-1LL), m_nullPFrameWidth(-1), m_nullPFrameHeight(-1),
	m_frameWidth(FRAME_WIDTH_MAX), m_frameHeight(FRAME_HEIGHT_MAX), m_scanForFrameSize(false), m_scanRemainderSize(0),
	m_scanRemainderLimit(SCAN_REMAINDER_SIZE_MPEG2), m_isH264(false), m_isMCChannel(false), m_isInterlaced(false), m_isInterlacedKnown(false),
	m_throttle(true), m_haveThrottleBase(false), m_lastThrottleContentTime(-1LL), m_lastThrottleRealTime(-1LL),
	m_baseThrottleContentTime(-1LL), m_baseThrottleRealTime(-1LL), m_throttlePTS(-1LL), m_insertPCR(false),
	m_scanSkipPacketsEnabled(false), m_currSPSId(0), m_picOrderCount(0), m_updatePicOrderCount(false),
	m_havePAT(false), m_versionPAT(0), m_program(0), m_pmtPid(0), m_havePMT(false), m_versionPMT(-1), m_indexAudio(false),
	m_haveAspect(false), m_haveFirstPTS(false), m_currentPTS(-1), m_pmtCollectorNextContinuity(0),
	m_pmtCollectorSectionLength(0), m_pmtCollectorOffset(0), m_pmtCollector(NULL),
	m_scrambledWarningIssued(false), m_checkContinuity(false), videoComponentCount(0), audioComponentCount(0),
	m_actualStartPTS(-1LL), m_throttleMaxDelayMs(DEFAULT_THROTTLE_MAX_DELAY_MS),
	m_throttleMaxDiffSegments(DEFAULT_THROTTLE_MAX_DIFF_SEGMENTS_MS),
	m_throttleDelayIgnoredMs(DEFAULT_THROTTLE_DELAY_IGNORED_MS), m_throttleDelayForDiscontinuityMs(DEFAULT_THROTTLE_DELAY_FOR_DISCONTINUITY_MS),
	m_throttleCond(), m_basePTSCond(), m_mutex(), m_enabled(true), m_processing(false), m_framesProcessedInSegment(0),
	m_lastPTSOfSegment(-1), m_streamOperation(streamOperation), m_vidDemuxer(NULL), m_audDemuxer(NULL),
	m_demux(NULL), m_peerTSProcessor(peerTSProcessor), m_packetStartAfterFirstPTS(-1), m_queuedSegment(NULL),
	m_queuedSegmentPos(0), m_queuedSegmentDuration(0), m_queuedSegmentLen(0), m_queuedSegmentDiscontinuous(false), m_startPosition(-1.0),
	m_track(track), m_last_frame_time(0), m_demuxInitialized(false), m_basePTSFromPeer(-1)
{
	INFO("constructor - %p\n", this);

	memset(m_SPS, 0, 32 * sizeof(H264SPS));
	memset(m_PPS, 0, 256 * sizeof(H264PPS));

	pthread_cond_init(&m_throttleCond, NULL);
	pthread_cond_init(&m_basePTSCond, NULL);
	pthread_mutex_init(&m_mutex, NULL);
	m_versionPMT = 0;

	if ((m_streamOperation == eStreamOp_DEMUX_ALL) || (m_streamOperation == eStreamOp_DEMUX_VIDEO))
	{
		m_vidDemuxer = new Demuxer(aamp,eMEDIATYPE_VIDEO);
		m_demux = true;
	}

	if ((m_streamOperation == eStreamOp_DEMUX_ALL) || (m_streamOperation == eStreamOp_DEMUX_AUDIO))
	{
		m_audDemuxer = new Demuxer(aamp, eMEDIATYPE_AUDIO);
		m_demux = true;
	}

	int compBufLen = MAX_PIDS*sizeof(RecordingComponent);
	memset(videoComponents, 0, compBufLen);
	memset(audioComponents, 0, compBufLen);
}

/**
 * @brief TSProcessor Destructor
 */
TSProcessor::~TSProcessor()
{
	INFO("destructor - %p\n", this);
	if (m_PatPmt)
	{
		free(m_PatPmt);
		m_PatPmt = 0;
	}
	if (m_PatPmtTrick)
	{
		free(m_PatPmtTrick);
		m_PatPmtTrick = 0;
	}
	if (m_PatPmtPcr)
	{
		free(m_PatPmtPcr);
		m_PatPmtPcr = 0;
	}

	if (m_nullPFrame)
	{
		free(m_nullPFrame);
		m_nullPFrame = 0;
	}
	if (m_pmtCollector)
	{
		free(m_pmtCollector);
		m_pmtCollector = 0;
	}
	if (m_emulationPrevention)
	{
		free(m_emulationPrevention);
		m_emulationPrevention = 0;
	}

	for (int i = 0; i < audioComponentCount; ++i)
	{
		if (audioComponents[i].associatedLanguage)
		{
			free(audioComponents[i].associatedLanguage);
		}
	}

	if (m_vidDemuxer)
	{
		delete m_vidDemuxer;
	}
	if (m_audDemuxer)
	{
		delete m_audDemuxer;
	}

	if (m_queuedSegment)
	{
		free(m_queuedSegment);
		m_queuedSegment = NULL;
	}

	pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_throttleCond);
	pthread_cond_destroy(&m_basePTSCond);
}


#define BUFFER_POOL_SIZE (10)
#define PLAY_BUFFER_AUDIO_MAX_PACKETS (100)
#define PLAY_BUFFER_MAX_PACKETS (512)
#define PLAY_BUFFER_SIZE (PLAY_BUFFER_MAX_PACKETS*MAX_PACKET_SIZE)
#define POOL_BUFFER_ALIGNMENT (16)
#define PLAY_BUFFER_CTX_OFFSET (0)
#define PLAY_BUFFER_SIGNATURE (('P')|(('L')<<8)|(('A')<<16)|(('Y')<<24))



/**
 * @brief Insert PAT and PMT sections
 * @param[out] buffer PAT and PMT is copied to this buffer
 * @param[in] trick true on trick mode, false on normal playback
 * @param[in] bufferSize size of buffer
 * @retval length of output buffer
 */
int TSProcessor::insertPatPmt(unsigned char *buffer, bool trick, int bufferSize)
{
	int len;

	if (trick && m_trickExcludeAudio)
	{
		len = m_PatPmtTrickLen;
		rmf_osal_memcpy(buffer, m_PatPmtTrick, len, bufferSize, m_PatPmtTrickLen);
	}
	else if (trick && m_isMCChannel)
	{
		len = m_PatPmtPcrLen;
		rmf_osal_memcpy(buffer, m_PatPmtPcr, len, bufferSize, m_PatPmtPcrLen);
	}
	else
	{
		len = m_PatPmtLen;
		rmf_osal_memcpy(buffer, m_PatPmt, len, bufferSize, m_PatPmtLen);
	}

	int index = 3 + m_ttsSize;
	buffer[index] = ((buffer[index] & 0xF0) | (m_patCounter++ & 0x0F));

	index += m_packetSize;
	while (index < len)
	{
		buffer[index] = ((buffer[index] & 0xF0) | (m_pmtCounter++ & 0x0F));
		index += m_packetSize;
	}

	return len;
}


/**
 * @brief insert PCR to the packet in case of PTS restamping
 * @param[in] packet[in,out] buffer to which PCR to be inserted
 * @param[in] pid[in] pcr pid
 */
void TSProcessor::insertPCR(unsigned char *packet, int pid)
{
	int i;
	long long currPCR;

	assert(m_playMode == PlayMode_retimestamp_Ionly);
	i = 0;
	if (m_ttsSize)
	{
		memset(packet, 0, m_ttsSize);
		i += m_ttsSize;
	}
	packet[i + 0] = 0x47;
	packet[i + 1] = (0x60 | (unsigned char)((pid >> 8) & 0x1F));
	packet[i + 2] = (unsigned char)(0xFF & pid);
	// 2 bits Scrambling = no; 2 bits adaptation field = has adaptation, no payload; 4 bits continuity counter
	// Don't increment continuity counter since there is no payload
	packet[i + 3] = (0x20 | (m_continuityCounters[pid] & 0x0F));
	packet[i + 4] = 0xB7; // 1 byte of adaptation data, but require length of 183 when no payload is indicated
	packet[i + 5] = 0x10; // PCR
	currPCR = ((m_currRateAdjustedPTS - 10000) & 0x1FFFFFFFFLL);
	TRACE1("TSProcessor::insertPCR: m_currRateAdjustedPTS= %llx currPCR= %llx\n", m_currRateAdjustedPTS, currPCR);
	writePCR(&packet[i + 6], currPCR, true);
	for (int i = 6 + 6 + m_ttsSize; i < m_packetSize; i++)
	{
		packet[i] = 0xFF;
	}
}

/**
 * @brief process PMT section and update media components.
 * @param[in] section character buffer containing PMT section
 * @param[in] sectionLength length of PMT section
 * @note Call with section pointing to first byte after section_length
 */
void TSProcessor::processPMTSection(unsigned char* section, int sectionLength)
{
	unsigned char *programInfo, *programInfoEnd;
	unsigned int dataDescTags[MAX_PIDS];
	int streamType = 0, pid = 0, len = 0;
	char work[32];

	int version = ((section[2] >> 1) & 0x1F);
	int pcrPid = (((section[5] & 0x1F) << 8) + section[6]);
	int infoLength = (((section[7] & 0x0F) << 8) + section[8]);



	for (int i = 0; i < audioComponentCount; ++i)
	{
		if (audioComponents[i].associatedLanguage)
		{
			free(audioComponents[i].associatedLanguage);
		}
	}

	memset(videoComponents, 0, sizeof(videoComponents));
	memset(audioComponents, 0, sizeof(audioComponents));

	memset(dataDescTags, 0, sizeof(dataDescTags));

	memset(work, 0, sizeof(work));

	videoComponentCount = audioComponentCount = 0;

	// Program loop starts after program info descriptor and continues
	// to the CRC at the end of the section
	programInfo = &section[9 + infoLength];
	programInfoEnd = section + sectionLength - 4;
	while (programInfo < programInfoEnd)
	{
		streamType = programInfo[0];
		pid = (((programInfo[1] & 0x1F) << 8) + programInfo[2]);
		len = (((programInfo[3] & 0x0F) << 8) + programInfo[4]);
		switch (streamType)
		{
		case 0x02: // MPEG2 Video
		case 0x24: // HEVC video
		case 0x80: // ATSC Video
			if (videoComponentCount < MAX_PIDS)
			{
				videoComponents[videoComponentCount].pid = pid;
				videoComponents[videoComponentCount].elemStreamType = streamType;
				++videoComponentCount;
			}
			else
			{
				WARNING("Warning: RecordContext: pmt contains more than %d video PIDs\n", MAX_PIDS);
			}
			break;
		case 0x1B: // H.264 Video
			if (videoComponentCount < MAX_PIDS)
			{
				videoComponents[videoComponentCount].pid = pid;
				videoComponents[videoComponentCount].elemStreamType = streamType;
				++videoComponentCount;
				m_isH264 = true;
				m_scanRemainderLimit = SCAN_REMAINDER_SIZE_H264;
			}
			else
			{
				WARNING("Warning: RecordContext: pmt contains more than %d video PIDs\n", MAX_PIDS);
			}
			break;
		case 0x03: // MPEG1 Audio
		case 0x04: // MPEG2 Audio
		case 0x0F: // MPEG2 AAC Audio
		case 0x11: // MPEG4 LATM AAC Audio
		case 0x81: // ATSC AC3 Audio
		case 0x82: // HDMV DTS Audio
		case 0x83: // LPCM Audio
		case 0x84: // SDDS Audio
		case 0x86: // DTS-HD Audio
		case 0x87: // ATSC E-AC3 Audio
		case 0x8A: // DTS Audio
		case 0x91: // A52b/AC3 Audio
		case 0x94: // SDDS Audio
			if (audioComponentCount < MAX_PIDS)
			{
				audioComponents[audioComponentCount].pid = pid;
				audioComponents[audioComponentCount].elemStreamType = streamType;
				audioComponents[audioComponentCount].associatedLanguage = 0;
				if (len > 2)
				{
					int descIdx, maxIdx;
					int descrTag, descrLen;

					descIdx = 5;
					maxIdx = descIdx + len;

					while (descIdx < maxIdx)
					{
						descrTag = programInfo[descIdx];
						descrLen = programInfo[descIdx + 1];

						switch (descrTag)
						{
							// ISO_639_language_descriptor
						case 0x0A:
							rmf_osal_memcpy(work, &programInfo[descIdx + 2], descrLen, 32, programInfoEnd - &programInfo[descIdx + 2]);
							work[descrLen] = '\0';
							audioComponents[audioComponentCount].associatedLanguage = strdup(work);
							break;
						}

						descIdx += (2 + descrLen);
					}
				}
				++audioComponentCount;
			}
			else
			{
				WARNING("Warning: RecordContext: pmt contains more than %d audio PIDs\n", MAX_PIDS);
			}
			break;

		default:
			DEBUG("RecordContext: pmt contains unused stream type 0x%x\n", streamType);
			break;
		}
		programInfo += (5 + len);
	}

	if (videoComponentCount > 0)
	{
		m_videoPid = videoComponents[0].pid;
		NOTICE( "[%p] found %d video pids in program %d with pcr pid %d video pid %d\n",
			this, videoComponentCount, m_program, pcrPid, m_videoPid);
	}
	if (audioComponentCount > 0)
	{
		NOTICE( "[%p] found %d audio pids in program %d with pcr pid %d audio pid %d\n",
			this, audioComponentCount, m_program, pcrPid, audioComponents[0].pid);
	}

	if (videoComponentCount == 0)
	{
		for (int audioIndex = 0; audioIndex < audioComponentCount; ++audioIndex)
		{
			if (pcrPid == audioComponents[audioIndex].pid)
			{
				INFO("RecordContext: indexing audio");
				m_indexAudio = true;

				break;
			}
		}
	}

	m_pcrPid = pcrPid;
	m_versionPMT = version;
	m_havePMT = true;
}


/**
 * @brief Generate and update PAT and PMT sections
 */
void TSProcessor::updatePATPMT()
{

	if (m_PatPmt)
	{
		free(m_PatPmt);
		m_PatPmt = 0;
	}
	if (m_PatPmtTrick)
	{
		free(m_PatPmtTrick);
		m_PatPmtTrick = 0;
	}

	if (m_PatPmtPcr)
	{
		free(m_PatPmtPcr);
		m_PatPmtPcr = 0;
	}

	generatePATandPMT(false, &m_PatPmt, &m_PatPmtLen);
	generatePATandPMT(true, &m_PatPmtTrick, &m_PatPmtTrickLen);
	generatePATandPMT(false, &m_PatPmtPcr, &m_PatPmtPcrLen, true);
}


/**
 * @brief Send discontinuity packet. Not relevant for demux operations
 * @param[in] position position in seconds
 */
void TSProcessor::sendDiscontinuity(double position)
{

	long long currPTS, currPCR, insertPCR = -1LL;
	bool haveInsertPCR = false;

	// Set inital PCR value based on first PTS
	currPTS = m_currentPTS;
	currPCR = ((currPTS - 10000) & 0x1FFFFFFFFLL);
	if (!m_haveBaseTime)
	{
		if (m_playModeNext == PlayMode_retimestamp_Ionly)
		{
			haveInsertPCR = true;
			insertPCR = currPCR;

			m_haveUpdatedFirstPTS = false;
			m_pcrPerPTSCount = 0;
			m_prevRateAdjustedPCR = -1LL;
			m_currRateAdjustedPCR = currPCR;
			m_currRateAdjustedPTS = currPTS;
			m_currPTS = -1LL;
		}
#if 0
		else
		{
			haveInsertPCR = findNextPCR(insertPCR);
			if (haveInsertPCR)
			{
				insertPCR -= 1;
			}
		}
		if (haveInsertPCR && (m_playRateNext != 1.0))
		{
			m_haveBaseTime = true;
			m_baseTime = insertPCR;
			m_segmentBaseTime = m_baseTime;
			INFO("have baseTime %llx from pid %x on signal of PCR discontinuity\n", m_baseTime, m_pcrPid);
		}
#endif
	}

	// Signal a discontinuity on the PCR pid and the video pid
	// if there is a video pid and it is not the same as the PCR
	int discCount = 1;
	if ((m_videoPid != -1) && (m_videoPid != m_pcrPid))
	{
		discCount += 1;
	}
	for (int discIndex = 0; discIndex < discCount; ++discIndex)
	{
		int discPid = (discIndex == 0) ? m_pcrPid : m_videoPid;
		unsigned char discontinuityPacket[MAX_PACKET_SIZE];
		int i = 0;
		if (m_ttsSize)
		{
			memset(discontinuityPacket, 0, m_ttsSize);
			i += m_ttsSize;
		}
		discontinuityPacket[i + 0] = 0x47;
		discontinuityPacket[i + 1] = 0x60;
		discontinuityPacket[i + 1] |= (unsigned char)((discPid >> 8) & 0x1F);
		discontinuityPacket[i + 2] = (unsigned char)(0xFF & discPid);
		discontinuityPacket[i + 3] = 0x20; // 2 bits Scrambling = no; 2 bits adaptation field = has adaptation, no cont; 4 bits continuity counter
		discontinuityPacket[i + 4] = 0xB7; // 1 byte of adaptation data, but require length of 183 when no payload is indicated
		discontinuityPacket[i + 5] = 0x80; // discontinuity
		for (int i = 6 + m_ttsSize; i < m_packetSize; i++)
		{
			discontinuityPacket[i] = 0xFF;
		}
		m_continuityCounters[discPid] = 0x00;

		TRACE1("emit pcr discontinuity");
		if (!m_demux)
		{
			aamp->SendStream((MediaType)m_track, discontinuityPacket, m_packetSize, position, position, 0);
		}
		if (haveInsertPCR)
		{
			i = 0;
			if (m_ttsSize)
			{
				memset(discontinuityPacket, 0, m_ttsSize);
				i += m_ttsSize;
			}
			discontinuityPacket[i + 0] = 0x47;
			discontinuityPacket[i + 1] = 0x60;
			discontinuityPacket[i + 1] |= (unsigned char)((discPid >> 8) & 0x1F);
			discontinuityPacket[i + 2] = (unsigned char)(0xFF & discPid);
			discontinuityPacket[i + 3] = 0x21; // 2 bits Scrambling = no; 2 bits adaptation field = has adaptation, no cont; 4 bits continuity counter
			discontinuityPacket[i + 4] = 0xB7; // 1 byte of adaptation data, but require length of 183 when no payload is indicated
			discontinuityPacket[i + 5] = 0x10; // PCR
			writePCR(&discontinuityPacket[i + 6], insertPCR, true);
			for (int i = 6 + 6 + m_ttsSize; i < m_packetSize; i++)
			{
				discontinuityPacket[i] = 0xFF;
			}
			m_continuityCounters[discPid] = 0x01;

			TRACE1("supply new pcr value");

			if (!m_demux)
			{
				aamp->SendStream((MediaType)m_track, discontinuityPacket, m_packetSize, position, position, 0);
			}
		}
	}
	m_needDiscontinuity = false;
}


/**
 * @brief Get current time stamp in milliseconds
 * @retval time stamp in milliseconds
 */
long long TSProcessor::getCurrentTime()
{
	struct timeval tv;
	long long currentTime;

	gettimeofday(&tv, 0);

	currentTime = (((unsigned long long)tv.tv_sec) * 1000 + ((unsigned long long)tv.tv_usec) / 1000);

	return currentTime;
}

/**
 * @brief sleep used internal by throttle logic
 * @param[in] throttleDiff time in milliseconds
 * @retval true on abort
 */
bool TSProcessor::msleep(long long throttleDiff)
{
	struct timespec ts;
	struct timeval tv;
	bool aborted = false;
	gettimeofday(&tv, NULL);
	ts.tv_sec = time(NULL) + throttleDiff / 1000;
	ts.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (throttleDiff % 1000));
	ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
	ts.tv_nsec %= (1000 * 1000 * 1000);
	pthread_mutex_lock(&m_mutex);
	pthread_cond_timedwait(&m_throttleCond, &m_mutex, &ts);
	if (!m_enabled)
	{
		aborted = true;
	}
	pthread_mutex_unlock(&m_mutex);
	return aborted;
}


/**
 * @brief Blocks based on PTS. Can be used for pacing injection.
 * @retval true if aborted
 */
bool TSProcessor::throttle()
{
	bool aborted = false;
	// If throttling is enabled, regulate output data rate
	if (m_throttle)
	{
		// Track content time via PTS values compared to real time and don't
		// let data get more than 200 ms ahead of real time.
		long long contentTime = ((m_playRate != 1.0) ? m_throttlePTS : m_actualStartPTS);
		if (contentTime != -1LL)
		{
			long long now, contentTimeDiff, realTimeDiff;
			TRACE2("contentTime %lld (%lld ms) m_playRate %f\n", contentTime, contentTime / 90, m_playRate);

			// Convert from 90KHz milliseconds
			contentTime = (contentTime / 90LL);

			now = getCurrentTime();
			if (m_haveThrottleBase)
			{

				if (m_lastThrottleContentTime != -1LL)
				{
					contentTimeDiff = contentTime - m_lastThrottleContentTime;
					realTimeDiff = now - m_lastThrottleRealTime;
					if (((contentTimeDiff > 0) && (contentTimeDiff < m_throttleMaxDiffSegments)) && ((realTimeDiff > 0) && (realTimeDiff < m_throttleMaxDiffSegments)))
					{
						contentTimeDiff = contentTime - m_baseThrottleContentTime;
						realTimeDiff = now - m_baseThrottleRealTime;
						if ((realTimeDiff > 0) && (realTimeDiff < contentTimeDiff))
						{
							long long throttleDiff = (contentTimeDiff - realTimeDiff - m_throttleDelayIgnoredMs);
							if (throttleDiff > 0)
							{
								if (throttleDiff > m_throttleMaxDelayMs)
								{
									// Don't delay more than 500 ms in any given request
									TRACE2("TSProcessor::fillBuffer: throttle: cap %lld to %d ms\n", throttleDiff, m_throttleMaxDelayMs);
									throttleDiff = m_throttleMaxDelayMs;
								}
								else
								{
									TRACE2("TSProcessor::fillBuffer: throttle: sleep %lld ms\n", throttleDiff);
								}
								aborted = msleep(throttleDiff);
							}
							else
							{
								INFO("throttleDiff negative? %lld\n", throttleDiff);
							}
						}
						else
						{
							TRACE2("realTimeDiff %lld\n", realTimeDiff);
						}
					}
					else if ((contentTimeDiff < -m_throttleDelayForDiscontinuityMs) || (contentTimeDiff > m_throttleDelayForDiscontinuityMs))
					{
						// There has been some timing irregularity such as a PTS discontinuity.
						// Establish a new throttling time base.
						m_haveThrottleBase = false;
						INFO(" contentTimeDiff( %lld) greater than threshold (%d) - probable pts discontinuity\n", contentTimeDiff, m_throttleDelayForDiscontinuityMs);
					}
					else
					{
						INFO(" Out of throttle window - contentTimeDiff %lld realTimeDiff  %lld\n", contentTimeDiff, realTimeDiff);
					}
				}
				else
				{
					TRACE2(" m_lastThrottleContentTime %lld\n", m_lastThrottleContentTime);
				}
			}
			else
			{
				INFO("Do not have ThrottleBase");
			}

			if (!m_haveThrottleBase)
			{
				m_haveThrottleBase = true;
				m_baseThrottleRealTime = now;
				m_baseThrottleContentTime = contentTime;
			}
			m_lastThrottleRealTime = now;
			m_lastThrottleContentTime = contentTime;
		}
		else if (m_demux && (1.0 != m_playRate))
		{
			if (m_last_frame_time)
			{
				long long now = getCurrentTime();
				long long nextFrameTime = m_last_frame_time + 1000 / m_apparentFrameRate;
				if (nextFrameTime > now)
				{
					long long throttleDiff = nextFrameTime - now;
					INFO("Wait %llu ms \n", throttleDiff);
					aborted = msleep(throttleDiff);
				}
			}
			m_last_frame_time = getCurrentTime();
		}
		else
		{
			INFO("contentTime not updated yet");
		}
	}
	else
	{
		TRACE1("Throttle not enabled");
	}
	TRACE4("Exit");
	return aborted;
}


/**
 * @brief Process buffers and update internal states related to media components
 * @param[in] buffer contains TS data
 * @param[in] size lenght of the buffer
 * @param[out] insPatPmt indicates if PAT and PMT needs to inserted
 * @retval false if operation is aborted.
 */
bool TSProcessor::processBuffer(unsigned char *buffer, int size, bool &insPatPmt)
{
	bool result = true;
	unsigned char *packet, *bufferEnd;
	int pid, payloadStart, adaptation, payloadOffset;
	int continuity, scramblingControl;
	int packetCount = 0;
	insPatPmt = false;
	bool removePatPmt = false;
	m_packetStartAfterFirstPTS = -1;
  
	if ((m_playRate != 1.0) || (eStreamOp_SEND_VIDEO_AND_QUEUED_AUDIO == m_streamOperation))
	{
		insPatPmt = true;
		removePatPmt = true;
		INFO("Replace PAT/PMT");
	}
	else if (eStreamOp_QUEUE_AUDIO == m_streamOperation)
	{
		removePatPmt = true;
		INFO("Remove PAT/PMT");
	}

	bool doThrottle = m_throttle;

	/*m_actualStartPTS stores the pts of  segment which will be used by throttle*/
	m_actualStartPTS = -1LL;

	packet = buffer + m_ttsSize;

	// For the moment, insist on buffers being TS packet aligned
	if (!((packet[0] == 0x47) && ((size%m_packetSize) == 0)))
	{
		FATAL("Error: data buffer not TS packet aligned\n");
		logprintf("packet=%p size=%d m_packetSize=%d\n", packet, size, m_packetSize);
		dumpPacket(packet, m_packetSize);
		assert(false);
	}

	bufferEnd = packet + size - m_ttsSize;
	while (packet < bufferEnd)
	{
		pid = (((packet[1] << 8) | packet[2]) & 0x1FFF);
		TRACE4("pid = %d, m_ttsSize %d\n", pid, m_ttsSize);

		if (m_checkContinuity)
		{
			if ((pid != 0x1FFF) && (packet[3] & 0x10))
			{
				continuity = (packet[3] & 0x0F);
				if (m_continuityCounters[pid] >= 0)
				{
					int expected = m_continuityCounters[pid];
					expected = ((expected + 1) & 0xF);
					if (expected != continuity)
					{
						WARNING("input SPTS discontinuity on pid %X (%d instead of %d) offset %llx\n",
							pid, continuity, expected, (long long)(packetCount*m_packetSize));
					}
				}
				m_continuityCounters[pid] = continuity;
			}
		}

		if (pid == 0)
		{
			adaptation = ((packet[3] & 0x30) >> 4);
			if (adaptation & 0x01)
			{
				payloadOffset = 4;

				if (adaptation & 0x02)
				{
					payloadOffset += (1 + packet[4]);
				}

				payloadStart = (packet[1] & 0x40);
				if (payloadStart)
				{
					int tableid = packet[payloadOffset + 1];
					if (tableid == 0x00)
					{
						int version = packet[payloadOffset + 6];
						int current = (version & 0x01);
						version = ((version >> 1) & 0x1F);

						TRACE4("PAT current version %d existing version %d\n", version, m_versionPAT);
						if (!m_havePAT || (current && (version != m_versionPAT)))
						{
							dumpPacket(packet, m_packetSize);
							int length = ((packet[payloadOffset + 2] & 0x0F) << 8) + (packet[payloadOffset + 3]);
							if (length == 13)
							{
								m_havePAT = true;
								m_versionPAT = version;
								m_program = ((packet[payloadOffset + 9] << 8) + packet[payloadOffset + 10]);
								m_pmtPid = (((packet[payloadOffset + 11] & 0x1F) << 8) + packet[payloadOffset + 12]);
								if ((m_program != 0) && (m_pmtPid != 0))
								{
									if (m_havePMT)
									{
										INFO("RecordContext: pmt change detected in pat");
										m_havePMT = false;
										goto done;
									}
									m_havePMT = false;
									DEBUG("RecordContext: acquired PAT version %d program %X pmt pid %X\n", version, m_program, m_pmtPid);
								}
								else
								{
									WARNING("Warning: RecordContext: ignoring pid 0 TS packet with suspect program %x and pmtPid %x\n", m_program, m_pmtPid);
									dumpPacket(packet, m_packetSize);
									m_program = -1;
									m_pmtPid = -1;
								}
							}
							else
							{
								WARNING("Warning: RecordContext: ignoring pid 0 TS packet with length of %d - not SPTS?\n", length);
							}
						}
					}
					else
					{
						WARNING("Warning: RecordContext: ignoring pid 0 TS packet with tableid of %x\n", tableid);
					}
				}
				else
				{
					WARNING("Warning: RecordContext: ignoring pid 0 TS packet with adaptation of %x\n", adaptation);
				}
			}
			else
			{
				WARNING("Warning: RecordContext: ignoring pid 0 TS with no payload indicator\n");
			}
			/*For trickmodes, remove PAT/PMT*/
			if (removePatPmt)
			{
				// Change to null packet
				packet[1] = ((packet[1] & 0xE0) | 0x1F);
				packet[2] = 0xFF;
			}
		}
		else if (pid == m_pmtPid)
		{
			TRACE4("Got PMT : m_pmtPid %d\n", m_pmtPid);
			adaptation = ((packet[3] & 0x30) >> 4);
			if (adaptation & 0x01)
			{
				payloadOffset = 4;

				if (adaptation & 0x02)
				{
					payloadOffset += (1 + packet[4]);
				}

				payloadStart = (packet[1] & 0x40);
				if (payloadStart)
				{
					int tableid = packet[payloadOffset + 1];
					if (tableid == 0x02)
					{
						int program = ((packet[payloadOffset + 4] << 8) + packet[payloadOffset + 5]);
						if (program == m_program)
						{
							int version = packet[payloadOffset + 6];
							int current = (version & 0x01);
							version = ((version >> 1) & 0x1F);

							TRACE4("PMT current version %d existing version %d\n", version, m_versionPMT);
							if (!m_havePMT || (current && (version != m_versionPMT)))
							{
								dumpPacket(packet, m_packetSize);

								if (m_havePMT && (version != m_versionPMT))
								{
									INFO("RecordContext: pmt change detected: version %d -> %d\n", m_versionPMT, version);
									m_havePMT = false;
									goto done;
								}

								if (!m_havePMT)
								{
									int sectionLength = (((packet[payloadOffset + 2] & 0x0F) << 8) + packet[payloadOffset + 3]);
									// Check if pmt payload fits in one TS packet:
									// section data starts after pointer_field (1), tableid (1), and section length (2)
									if (payloadOffset + 4 + sectionLength <= m_packetSize - m_ttsSize)
									{
										processPMTSection(packet + payloadOffset + 4, sectionLength);
									}
									else if (sectionLength <= MAX_PMT_SECTION_SIZE)
									{
										if (!m_pmtCollector)
										{
											m_pmtCollector = (unsigned char*)malloc(MAX_PMT_SECTION_SIZE);
											if (!m_pmtCollector)
											{
												ERROR("Error: unable to allocate pmt collector buffer - ignoring large pmt (section length %d)\n", sectionLength);
												goto done;
											}
											INFO("RecordContext: allocating pmt collector buffer %p\n", m_pmtCollector);
										}
										// section data starts after table id, section length and pointer field
										int sectionOffset = payloadOffset + 4;
										int sectionAvail = m_packetSize - m_ttsSize - sectionOffset;
										unsigned char *sectionData = packet + sectionOffset;
										rmf_osal_memcpy(m_pmtCollector, sectionData, sectionAvail, MAX_PMT_SECTION_SIZE, (bufferEnd - sectionData));
										m_pmtCollectorSectionLength = sectionLength;
										m_pmtCollectorOffset = sectionAvail;
										m_pmtCollectorNextContinuity = ((packet[3] + 1) & 0xF);
										INFO("RecordContext: starting to collect multi-packet pmt: section length %d\n", sectionLength);
									}
									else
									{
										WARNING("Warning: RecordContext: ignoring oversized pmt (section length %d)\n", sectionLength);
									}
								}
							}
						}
						else
						{
							WARNING("Warning: RecordContext: ignoring pmt TS packet with mismatched program of %x (expecting %x)\n", program, m_program);
							dumpPacket(packet, m_packetSize);
						}
					}
					else
					{
						TRACE1("Warning: RecordContext: ignoring pmt TS packet with tableid of %x\n", tableid);
					}
				}
				else
				{
					// process subsequent parts of multi-packet pmt
					if (m_pmtCollectorOffset)
					{
						int continuity = (packet[3] & 0xF);
						if (((continuity + 1) & 0xF) == m_pmtCollectorNextContinuity)
						{
							WARNING("Warning: RecordContext: next packet of multi-packet pmt has wrong continuity count %d (expecting %d)\n",
								m_pmtCollectorNextContinuity, continuity);
							// Assume continuity counts for all packets of this pmt will remain the same.
							// Allow this since it has been observed in the field
							m_pmtCollectorNextContinuity = continuity;
						}
						if (continuity == m_pmtCollectorNextContinuity)
						{
							int avail = m_packetSize - m_ttsSize - payloadOffset;
							int sectionAvail = m_pmtCollectorSectionLength - m_pmtCollectorOffset;
							int copylen = ((avail > sectionAvail) ? sectionAvail : avail);
							rmf_osal_memcpy(&m_pmtCollector[m_pmtCollectorOffset], &packet[payloadOffset], copylen, (MAX_PMT_SECTION_SIZE - m_pmtCollectorOffset), (bufferEnd - &packet[payloadOffset]));
							m_pmtCollectorOffset += copylen;
							if (m_pmtCollectorOffset == m_pmtCollectorSectionLength)
							{
								processPMTSection(m_pmtCollector, m_pmtCollectorSectionLength);
								m_pmtCollectorOffset = 0;
							}
							else
							{
								m_pmtCollectorNextContinuity = ((continuity + 1) & 0xF);
							}
						}
						else
						{
							ERROR("Error: RecordContext: aborting multi-packet pmt due to discontinuity error: expected %d got %d\n",
								m_pmtCollectorNextContinuity, continuity);
							m_pmtCollectorOffset = 0;
						}
					}
					else if (!m_havePMT)
					{
						WARNING("Warning: RecordContext: ignoring unexpected pmt TS packet without payload start indicator");
					}
				}
			}
			else
			{
				WARNING("Warning: RecordContext: ignoring unexpected pmt TS packet without payload indicator");
			}
			/*For trickmodes, remove PAT/PMT*/
			if (removePatPmt)
			{
				// Change to null packet
				packet[1] = ((packet[1] & 0xE0) | 0x1F);
				packet[2] = 0xFF;
			}
		}
		else if ((pid == m_videoPid) || (pid == m_pcrPid))
		{

			if ((m_actualStartPTS == -1LL) && doThrottle)
			{
				payloadOffset = 4;
				adaptation = ((packet[3] & 0x30) >> 4);
				payloadStart = (packet[1] & 0x40);

				scramblingControl = ((packet[3] & 0xC0) >> 6);
				if (scramblingControl)
				{
					if (!m_scrambledWarningIssued)
					{
						TRACE3("RecordingContext: video pid %x transport_scrambling_control bits are non-zero (%02x)- is data still scrambled?\n", pid, packet[3]);
						m_scrambledWarningIssued = true;
						TRACE3("[%s:%d] found scrambled data, NOT writing idx,mpg files, returning(true)... \n", __FUNCTION__, __LINE__);
					}
				}
				m_scrambledWarningIssued = false;

				if (adaptation & 0x02)
				{
					payloadOffset += (1 + packet[4]);
				}

				if (adaptation & 0x01)
				{
					if (payloadStart)
					{
						if ((packet[payloadOffset] == 0x00) && (packet[payloadOffset + 1] == 0x00) && (packet[payloadOffset + 2] == 0x01))
						{
							int pesHeaderDataLen = packet[payloadOffset + 8];
							if (packet[payloadOffset + 7] & 0x80)
							{
								// PTS
								long long PTS;

								bool validPTS = readTimeStamp(&packet[payloadOffset + 9], PTS);
								if (validPTS)
								{
									m_packetStartAfterFirstPTS = (packet - buffer) + PACKET_SIZE;
									long long diffPTS = 0;

									if (m_actualStartPTS != -1LL)
									{
										bool wrapAround = false;
										if ((m_currentPTS > MAX_PTS - 3LL * 90000LL) && (PTS < 3LL * 90000L))
										{
											wrapAround = true;
										}
										else if ((PTS > MAX_PTS - 3LL * 90000LL) && (m_currentPTS < 3LL * 90000LL))
										{
											wrapAround = true;
										}

										if (!wrapAround)
										{
											if (m_currentPTS <= PTS)
											{
												// Normal case
												diffPTS = PTS - m_currentPTS;
											}
											else
											{
												// Out of order PTS from transport ordering 
												diffPTS = m_currentPTS - PTS;
											}
										}
										else
										{
											INFO("RecordContext: pts rollover: 0x%llx to 0x%llx\n", m_currentPTS, PTS);
										}
									}

									// Consider a difference of 10 or more seconds a discontinuity
									if (diffPTS >= 10 * 90000LL)
									{
										if ((diffPTS < 12 * 90000LL))// && (m_recording->getGOPSize(0) < 4) )
										{
											// This is a music channel with a slow rate of video frames
											// so ignore the PTS jump
										}
										else
										{
											INFO("RecordContext: pts discontinuity: %llx to %llx\n", m_currentPTS, PTS);
											m_currentPTS = PTS;
										}
									}
									else
									{
										m_currentPTS = PTS;
										if (m_actualStartPTS == -1LL)
										{
											m_actualStartPTS = PTS;
											TRACE2("Updated m_actualStartPTS to %lld\n", m_actualStartPTS);
										}
									}

									m_haveFirstPTS = true;
								}
							}
							payloadOffset = payloadOffset + 9 + pesHeaderDataLen;
						}
					}
				}
			}
			if (doThrottle)
			{
				if (throttle())
				{
					INFO("throttle aborted");
					m_haveThrottleBase = false;
					m_lastThrottleContentTime = -1;
					result = false;
					break;
				}
				doThrottle = false;
			}
			if (1.0 != m_playRate && !m_demux)
			{
				/*reTimestamp updates packet, so pass a dummy variable*/
				unsigned char* tmpPacket = packet;
				reTimestamp(tmpPacket, m_packetSize);
			}
		}
		/*For trickmodes, keep only video pid*/
		else if (m_playRate != 1.0)
		{
			// Change to null packet
			packet[1] = ((packet[1] & 0xE0) | 0x1F);
			packet[2] = 0xFF;
		}

	done:
		packet += m_packetSize;
		++packetCount;
	}

	return result;
}


/**
 * @brief Update internal state variables to set up throttle
 * @param[in] segmentDurationMsSigned Duration of segment
 */
void TSProcessor::setupThrottle(int segmentDurationMsSigned)
{
	int segmentDurationMs = abs(segmentDurationMsSigned);
	m_throttleMaxDelayMs = segmentDurationMs + DEFAULT_THROTTLE_DELAY_IGNORED_MS;
	m_throttleMaxDiffSegments = m_throttleMaxDelayMs;
	m_throttleDelayIgnoredMs = DEFAULT_THROTTLE_DELAY_IGNORED_MS;
	m_throttleDelayForDiscontinuityMs = segmentDurationMs * 10;
	TRACE1("segmentDurationMs %d\n", segmentDurationMs);
}


/**
 * @brief Demux TS and send elementary streams
 * @param[in] ptr buffer containing TS data
 * @param[in] len lenght of buffer
 * @param[in] position position of segment in seconds
 * @param[in] duration duration of segment in seconds
 * @param[in] discontinuous true if segment is discontinous
 * @param[in] trackToDemux media track to do the operation
 * @retval true on success, false on PTS error
 */
bool TSProcessor::demuxAndSend(const void *ptr, size_t len, double position, double duration, bool discontinuous, TrackToDemux trackToDemux)
{
	int videoPid = -1, audioPid = -1;
	unsigned long long firstPcr = 0;
	bool isTrickMode = !( 1.0 == m_playRate);
	bool notifyPeerBasePTS = false;
	bool ret = true;

	if (m_vidDemuxer && ((trackToDemux == ePC_Track_Both) || (trackToDemux == ePC_Track_Video)))
	{
		if (videoComponentCount > 0)
		{
			videoPid = videoComponents[0].pid;
		}
		if (discontinuous || !m_demuxInitialized )
		{
			if (discontinuous && (1.0 == m_playRate))
			{
				NOTICE("TSProcessor:%p discontinuous buffer- flushing video demux\n", this);
			}
			m_vidDemuxer->flush();
			m_vidDemuxer->init(position, duration, isTrickMode, true);
		}
	}
	if (m_audDemuxer && ((trackToDemux == ePC_Track_Both) || (trackToDemux == ePC_Track_Audio)))
	{
		if (audioComponentCount > 0)
		{
			audioPid = audioComponents[0].pid;
		}
		if (discontinuous || !m_demuxInitialized )
		{

			if(discontinuous)
			{
				NOTICE("TSProcessor:%p discontinuous buffer- flushing audio demux\n", this);
			}
			m_audDemuxer->flush();
			m_audDemuxer->init(position, duration, isTrickMode, (eStreamOp_DEMUX_AUDIO != m_streamOperation));
		}
	}

	/*In the case of audio demux only operation, base_pts is already set before this function is called*/
	if (eStreamOp_DEMUX_AUDIO == m_streamOperation )
	{
		m_demuxInitialized = true;
	}
	INFO("demuxAndSend : len  %d videoPid %d audioPid %d m_pcrPid %d videoComponentCount %d m_demuxInitialized = %d\n", (int)len, videoPid, audioPid, m_pcrPid, videoComponentCount, m_demuxInitialized);

	unsigned char * packetStart = (unsigned char *)ptr;
	while (len >= PACKET_SIZE)
	{
		Demuxer* demuxer = NULL;
		int pid = (packetStart[1] & 0x1f) << 8 | packetStart[2];
		if (m_vidDemuxer && (pid == videoPid))
		{
			demuxer = m_vidDemuxer;
		}
		else if (m_audDemuxer && (pid == audioPid))
		{
			demuxer = m_audDemuxer;
		}
		if ((discontinuous || !m_demuxInitialized ) && !firstPcr && (pid == m_pcrPid))
		{
			int adaptation_fieldlen = 0;
			if ((packetStart[3] & 0x20) == 0x20)
			{
				adaptation_fieldlen = packetStart[4];
				if (0 != adaptation_fieldlen && (packetStart[5] & 0x10))
				{
					firstPcr = (unsigned long long) packetStart[6] << 25 | (unsigned long long) packetStart[7] << 17
						| (unsigned long long) packetStart[8] << 9 | packetStart[9] << 1 | (packetStart[10] & 80) >> 7;
					if (m_playRate == 1.0)
					{
						NOTICE("firstPcr %llu\n", firstPcr);
					}
					if (m_vidDemuxer)
					{
						m_vidDemuxer->setBasePTS(firstPcr, false);
					}
					if (m_audDemuxer)
					{
						m_audDemuxer->setBasePTS(firstPcr, false);
					}
					notifyPeerBasePTS = true;
					m_demuxInitialized = true;
				}
			}
		}

		if (demuxer)
		{
			bool ptsError, basePTSUpdated;
			demuxer->processPacket(packetStart, basePTSUpdated, ptsError);
			if(!m_demuxInitialized)
			{
				WARNING("PCR not available before ES packet, updating firstPCR\n");
				m_demuxInitialized = true;
				notifyPeerBasePTS = true;
				firstPcr = demuxer->getBasePTS();
			}

			if( basePTSUpdated && notifyPeerBasePTS)
			{
				if (m_audDemuxer && (m_audDemuxer!= demuxer) && (eStreamOp_DEMUX_AUDIO != m_streamOperation))
				{
					INFO("Using first video pts as base pts\n");
					m_audDemuxer->setBasePTS(demuxer->getBasePTS(), true);
				}
				else if (m_vidDemuxer && (m_vidDemuxer!= demuxer) && (eStreamOp_DEMUX_VIDEO != m_streamOperation))
				{
					WARNING("Using first audio pts as base pts\n");
					m_vidDemuxer->setBasePTS(demuxer->getBasePTS(), true);
				}
				if(m_peerTSProcessor)
				{
					m_peerTSProcessor->setBasePTS( position, demuxer->getBasePTS());
				}
				notifyPeerBasePTS = false;
			}
			if (ptsError)
			{
				WARNING("PTS error, discarding segment\n");
				ret = false;
				break;
			}
		}
		else
		{
			INFO("demuxAndSend : discarded packet with pid %d\n", pid);
		}

		packetStart += PACKET_SIZE;
		len -= PACKET_SIZE;
	}
	return ret;
}

/**
 * @brief Reset TS processor state
 */
void TSProcessor::reset()
{
	INFO("PC reset\n");
	pthread_mutex_lock(&m_mutex);
	if (m_vidDemuxer)
	{
		logprintf("TSProcessor[%p]%s:%d - reset video demux %p\n", this, __FUNCTION__, __LINE__, m_vidDemuxer);
		m_vidDemuxer->reset();
	}

	if (m_audDemuxer)
	{
		logprintf("TSProcessor[%p]%s:%d - reset audio demux %p\n", this, __FUNCTION__, __LINE__, m_audDemuxer);
		m_audDemuxer->reset();
	}
	m_enabled = true;
	m_demuxInitialized = false;
	m_basePTSFromPeer = -1;
	m_havePAT = false;
	m_havePMT = false;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * @brief Flush all buffered data to sink
 * @note Relevant only when s/w demux is used
 */
void TSProcessor::flush()
{
	INFO("PC flush\n");
	pthread_mutex_lock(&m_mutex);
	if (m_vidDemuxer)
	{
		logprintf("TSProcessor[%p]%s:%d - flush video demux %p\n", this, __FUNCTION__, __LINE__, m_vidDemuxer);
		m_vidDemuxer->flush();
	}

	if (m_audDemuxer)
	{
		logprintf("TSProcessor[%p]%s:%d - flush audio demux %p\n", this, __FUNCTION__, __LINE__, m_audDemuxer);
		m_audDemuxer->flush();
	}
	pthread_mutex_unlock(&m_mutex);
}


/**
 * @brief Send queued segment
 * @param[in] basepts new base pts to be set. Valid only for eStreamOp_DEMUX_AUDIO.
 * @param[in] updatedStartPositon New start position of queued segment.
 */
void TSProcessor::sendQueuedSegment(long long basepts, double updatedStartPositon)
{
	WARNING("PC %p basepts %lld\n", this, basepts);
	pthread_mutex_lock(&m_mutex);
	if (m_queuedSegment)
	{
		if (-1 != updatedStartPositon)
		{
			DEBUG("%s:%d Update position from %f to %f\n", __FUNCTION__, __LINE__, m_queuedSegmentPos, updatedStartPositon);
			m_queuedSegmentPos = updatedStartPositon;
		}
		if (eStreamOp_QUEUE_AUDIO == m_streamOperation)
		{
			aamp->SendStream((MediaType) m_track, m_queuedSegment, m_queuedSegmentLen, m_queuedSegmentPos,
			        m_queuedSegmentPos, m_queuedSegmentDuration);
		}
		else if (eStreamOp_DEMUX_AUDIO == m_streamOperation)
		{
			if(basepts)
			{
				m_audDemuxer->setBasePTS(basepts, true);
			}
			demuxAndSend(m_queuedSegment, m_queuedSegmentLen, m_queuedSegmentPos, m_queuedSegmentDuration, m_queuedSegmentDiscontinuous );
		}
		else
		{
			ERROR("sendQueuedSegment invoked in Invalid stream operation\n");
		}
		free(m_queuedSegment);
		m_queuedSegment = NULL;
	}
	else
	{
		WARNING("No pending buffer");
	}
	pthread_mutex_unlock(&m_mutex);
}


/**
 * @brief set base PTS for demux operations
 * @param[in] position start position of fragment
 * @param[in] pts base pts for demux operations.
 */
void TSProcessor::setBasePTS(double position, long long pts)
{
	pthread_mutex_lock(&m_mutex);
	m_basePTSFromPeer = pts;
	m_startPosition = position;
	INFO("pts = %lld\n", pts);
	if (m_audDemuxer)
	{
		m_audDemuxer->flush();
		m_audDemuxer->init(position, 0, false, true);
		m_audDemuxer->setBasePTS(pts, true);
		m_demuxInitialized = true;
	}
	pthread_cond_signal(&m_basePTSCond);
	pthread_mutex_unlock(&m_mutex);
}

/**
 * @brief Does configured operation on the segment and injects data to sink
 * @param[in] segment Buffer containing the data segment
 * @param[in] size Specifies size of the segment in bytes.
 * @param[in] position Position of the segment in seconds
 * @param[in] duration Duration of the segment in seconds
 * @param[in] discontinuous true if fragment is discontinuous
 * @param[out] true on PTS error
 * @retval true on success
 */
bool TSProcessor::sendSegment(char *segment, size_t& size, double position, double duration, bool discontinuous, bool &ptsError)
{
	bool insPatPmt;
	unsigned char * packetStart;
	int len = size;
	bool ret = false;
	ptsError = false;
	pthread_mutex_lock(&m_mutex);
	if (!m_enabled)
	{
		INFO("Not Enabled, Returning");
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	m_processing = true;
	if ((m_playModeNext != m_playMode) || (m_playRateNext != m_playRate))
	{
		TRACE1("change play mode");
		m_playMode = m_playModeNext;

		m_playRate = m_playRateNext;
		m_absPlayRate = fabs(m_playRate);
		INFO("playback changed to rate %f mode %d\n", m_playRate, m_playMode);
		m_haveEmittedIFrame = false;
		m_currFrameOffset = -1LL;
		m_nullPFrameNextCount = 0;
		m_needDiscontinuity = true;
	}
	pthread_mutex_unlock(&m_mutex);
	m_framesProcessedInSegment = 0;
	m_lastPTSOfSegment = -1;
	packetStart = (unsigned char *)segment;
	if ((packetStart[0] != 0x47) || ((packetStart[1] & 0x80) != 0x00) || ((packetStart[3] & 0xC0) != 0x00))
	{
		ERROR("Segment doesn't starts with valid TS packet, discarding. Dump first packet\n");
		for (int i = 0; i < PACKET_SIZE; i++)
		{
			printf("0x%2x ", packetStart[i]);
		}
		printf("\n");
		pthread_mutex_lock(&m_mutex);
		m_processing = false;
		pthread_cond_signal(&m_throttleCond);
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	if (len % m_packetSize)
	{
		int discardAtEnd = len % m_packetSize;
		INFO("Discarding %d bytes at end\n", discardAtEnd);
		len = len - discardAtEnd;
	}
	ret = processBuffer((unsigned char*)packetStart, len, insPatPmt);
	if (ret)
	{
		if (-1.0 == m_startPosition)
		{
			INFO("Reset m_startPosition to %f\n", position);
			m_startPosition = position;
		}
		double updatedPosition = m_startPosition + (position - m_startPosition) / m_playRate;
		INFO("updatedPosition = %f Position = %f m_startPosition = %f m_playRate = %f\n", updatedPosition, position, m_startPosition, m_playRate);
		position = updatedPosition;

		if (m_needDiscontinuity&& !m_demux)
		{
			sendDiscontinuity(position);
		}
		if (insPatPmt && !m_demux)
		{
			unsigned char *sec = (unsigned char *)malloc(PATPMT_MAX_SIZE);
			if (NULL != sec)
			{
				updatePATPMT();
				int secSize = insertPatPmt(sec, (m_playMode != PlayMode_normal), PATPMT_MAX_SIZE);
				aamp->SendStream((MediaType)m_track, sec, secSize, position, position, 0);
				free(sec);
				TRACE1("Send PAT/PMT");
			}
		}
		if (m_demux)
		{
			if (eStreamOp_DEMUX_AUDIO == m_streamOperation)
			{
                if(!gpGlobalConfig->bAudioOnlyPlayback)
                {
                    pthread_mutex_lock(&m_mutex);
                    if (-1 == m_basePTSFromPeer)
                    {
                        if (m_enabled)
                        {
                            logprintf("TSProcessor[%p]%s:%d - wait for base PTS. m_audDemuxer %p\n", this, __FUNCTION__, __LINE__, m_audDemuxer);
                            pthread_cond_wait(&m_basePTSCond, &m_mutex);
                        }

                        if (!m_enabled)
                        {
                            INFO("Not Enabled, Returning");
                            m_processing = false;
                            pthread_cond_signal(&m_throttleCond);
                            pthread_mutex_unlock(&m_mutex);
                            return false;
                        }
                        logprintf("TSProcessor[%p]%s:%d - got base PTS. m_audDemuxer %p\n", this, __FUNCTION__, __LINE__, m_audDemuxer);
                    }
                    pthread_mutex_unlock(&m_mutex);
                }
				ret = demuxAndSend(packetStart, len, m_startPosition, duration, discontinuous);
			}
			else if(!gpGlobalConfig->demuxedAudioBeforeVideo)
			{
				ret = demuxAndSend(packetStart, len, position, duration, discontinuous);
			}
			else
			{
				WARNING("Sending Audio First\n");
				ret = demuxAndSend(packetStart, len, position, duration, discontinuous, ePC_Track_Audio);
				ret |= demuxAndSend(packetStart, len, position, duration, discontinuous, ePC_Track_Video);
			}
			ptsError = !ret;
		}
		else if (eStreamOp_SEND_VIDEO_AND_QUEUED_AUDIO == m_streamOperation)
		{
			if (m_packetStartAfterFirstPTS != -1)
			{

				aamp->SendStream((MediaType)m_track, packetStart, m_packetStartAfterFirstPTS, position, position, duration);
				m_peerTSProcessor->sendQueuedSegment();
				aamp->SendStream((MediaType)m_track, packetStart + m_packetStartAfterFirstPTS,
				len - m_packetStartAfterFirstPTS, position, position, duration);
			}
			else
			{
				ERROR("m_packetStartAfterFirstPTS Not updated\n");
				aamp->SendStream((MediaType)m_track, packetStart + m_packetStartAfterFirstPTS,
				len - m_packetStartAfterFirstPTS, position, position, duration);
			}
		}
		else if (eStreamOp_QUEUE_AUDIO == m_streamOperation)
		{
			pthread_mutex_lock(&m_mutex);
			if (m_queuedSegment)
			{
				ERROR("Queued buffer not NULL\n");
				free(m_queuedSegment);
			}
			m_queuedSegment = (unsigned char *)malloc(len);
			if (!m_queuedSegment)
			{
				ERROR("Failed to allocate memory\n");
			}
			else
			{
				memcpy(m_queuedSegment, packetStart, len);
				m_queuedSegmentLen = len;
				m_queuedSegmentPos = position;
				m_queuedSegmentDuration = duration;
				m_queuedSegmentDiscontinuous = discontinuous;
			}
			pthread_mutex_unlock(&m_mutex);
		}
		else
		{
			aamp->SendStream((MediaType)m_track, packetStart, len, position, position, duration);
		}
	}
	if (-1 != duration)
	{
		int durationMs = (int)(duration * 1000);
		setupThrottle(durationMs);
	}
	pthread_mutex_lock(&m_mutex);
	m_processing = false;
	pthread_cond_signal(&m_throttleCond);
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

#define HEADER_SIZE 4
#define INDEX(i) (base+i < m_packetSize-m_ttsSize-HEADER_SIZE) ? i : i+m_ttsSize+HEADER_SIZE

// Call with buffer pointing to beginning of start code (iex 0x00, 0x00, 0x01, ...)

/**
 * @brief Process ES start code
 * @param[in] buffer buffer containing start code
 * @param[in] keepScanning true to keep on scanning
 * @param[in] length size of the buffer
 * @param[in] base Not used
 */
bool TSProcessor::processStartCode(unsigned char *buffer, bool& keepScanning, int length, int base)
{
	bool result = true;

	if (m_isH264)
	{
		int unitType = (buffer[INDEX(3)] & 0x1F);
		switch (unitType)
		{
		case 1:  // Non-IDR slice
		case 5:  // IDR slice
			if (m_isInterlacedKnown && (m_playMode == PlayMode_retimestamp_Ionly))
			{
				// Check if first_mb_in_slice is 0.  It will be 0 for the start of a frame, but could be non-zero
				// for frames with multiple slices.  This is encoded as a variable length Exp-Golomb code.  For the value
				// of zero this will be a single '1' bit.
				if (buffer[INDEX(4)] & 0x80)
				{
					H264SPS *pSPS;
					int mask = 0x40;
					unsigned char *p = &buffer[INDEX(4)];
					int slice_type = getUExpGolomb(p, mask);
					int pic_parameter_set_id = getUExpGolomb(p, mask);
					m_currSPSId = m_PPS[pic_parameter_set_id].spsId;
					pSPS = &m_SPS[m_currSPSId];

					if (pSPS->picOrderCountType == 0)
					{
						if (pSPS->separateColorPlaneFlag)
						{
							// color_plane_id
							getBits(p, mask, 2);
						}

						// frame_num      
						getBits(p, mask, pSPS->log2MaxFrameNumMinus4 + 4);

						if (!pSPS->frameMBSOnlyFlag)
						{
							int field_pic_flag = getBits(p, mask, 1);
							if (field_pic_flag)
							{
								//bottom_field_flag
								getBits(p, mask, 1);
							}
						}

						if (unitType == 5) // IdrPicFlag == 1
						{
							//idr_pic_id
							getUExpGolomb(p, mask);
						}

						// Update pic_order_cnt_lsb.  This field gives the frame display order.
						// If the original values are left they may be out of order so we replace
						// them with sequentially incrementing values.
						putBits(p, mask, pSPS->log2MaxPicOrderCntLsbMinus4 + 4, m_picOrderCount);
						m_picOrderCount = m_picOrderCount + 1;
						if (m_picOrderCount == pSPS->maxPicOrderCount)
						{
							m_picOrderCount = 0;
						}
						m_updatePicOrderCount = false;
						m_scanForFrameSize = false;
					}
				}
			}
			break;
		case 2:  // Slice data partiton A         
		case 3:  // Slice data partiton B         
		case 4:  // Slice data partiton C         
			break;
		case 6:  // SEI
			break;
		case 7:  // Sequence paramter set
		{
			bool scanForAspect = false;
			bool splitSPS = false;

			if (!m_emulationPrevention || (m_emulationPreventionCapacity < (m_emulationPreventionOffset + length)))
			{
				unsigned char *newBuff = 0;
				int newSize = m_emulationPreventionCapacity * 2 + length;
				newBuff = (unsigned char *)malloc(newSize*sizeof(char));
				if (!newBuff)
				{
					ERROR("Error: unable to allocate emulation prevention buffer\n");
					break;
				}
				if (m_emulationPrevention)
				{
					if (m_emulationPreventionOffset > 0)
					{
						rmf_osal_memcpy(newBuff, m_emulationPrevention, m_emulationPreventionOffset, newSize, m_emulationPreventionCapacity);
					}
					free(m_emulationPrevention);
				}
				m_emulationPreventionCapacity = newSize;
				m_emulationPrevention = newBuff;
			}
			if (m_emulationPreventionOffset > 0)
			{
				splitSPS = true;
			}
			for (int i = 4; i < length - 3; ++i)
			{
				if ((buffer[INDEX(i)] == 0x00) && (buffer[INDEX(i + 1)] == 0x00))
				{
					if (buffer[INDEX(i + 2)] == 0x01)
					{
						scanForAspect = true;
						break;
					}
					else if (buffer[INDEX(i + 2)] == 0x03)
					{
						m_emulationPrevention[m_emulationPreventionOffset++] = 0x00;
						m_emulationPrevention[m_emulationPreventionOffset++] = 0x00;
						i += 3;
					}
				}
				m_emulationPrevention[m_emulationPreventionOffset++] = buffer[INDEX(i)];
			}
			if (scanForAspect)
			{
				bool updateSPS = processSeqParameterSet(m_emulationPrevention, m_emulationPreventionOffset);
				if (updateSPS && !splitSPS)
				{
					int ppb = -1, pb = -1, b;
					int i = 0, j = 4;
					while (i < m_emulationPreventionOffset)
					{
						b = m_emulationPrevention[i];
						if ((ppb == 0) && (pb == 0) && (b == 1))
						{
							b = 3;
						}
						else
						{
							++i;
						}
						buffer[INDEX(j)] = b;
						j++;
						ppb = pb;
						pb = b;
					}
				}
				m_emulationPreventionOffset = 0;

				if ((m_playMode != PlayMode_retimestamp_Ionly) || !m_updatePicOrderCount)
				{
					// For IOnly we need to keep scanning in order to update
					// pic_order_cnt_lsb values                     
					m_scanForFrameSize = false;
					keepScanning = false;
				}
			}
		}
		break;
		case 8:  // Picture paramter set
			if (m_isInterlacedKnown && (m_playMode == PlayMode_retimestamp_Ionly))
			{
				bool processPPS = false;

				if (!m_emulationPrevention || (m_emulationPreventionCapacity < (m_emulationPreventionOffset + length)))
				{
					unsigned char *newBuff = 0;
					int newSize = m_emulationPreventionCapacity * 2 + length;
					newBuff = (unsigned char *)malloc(newSize*sizeof(char));
					if (!newBuff)
					{
						logprintf("Error: unable to allocate emulation prevention buffer\n");
						break;
					}
					if (m_emulationPrevention)
					{
						if (m_emulationPreventionOffset > 0)
						{
							rmf_osal_memcpy(newBuff, m_emulationPrevention, m_emulationPreventionOffset, newSize, m_emulationPreventionCapacity);
						}
						free(m_emulationPrevention);
					}
					m_emulationPreventionCapacity = newSize;
					m_emulationPrevention = newBuff;
				}
				for (int i = 4; i < length - 3; ++i)
				{
					if ((buffer[INDEX(i)] == 0x00) && (buffer[INDEX(i + 1)] == 0x00))
					{
						if (buffer[INDEX(i + 2)] == 0x01)
						{
							processPPS = true;
							break;
						}
						else if (buffer[INDEX(i + 2)] == 0x03)
						{
							m_emulationPrevention[m_emulationPreventionOffset++] = 0x00;
							m_emulationPrevention[m_emulationPreventionOffset++] = 0x00;
							i += 3;
						}
					}
					m_emulationPrevention[m_emulationPreventionOffset++] = buffer[INDEX(i)];
				}
				if (processPPS)
				{
					processPictureParameterSet(m_emulationPrevention, m_emulationPreventionOffset);

					m_emulationPreventionOffset = 0;
				}
			}
			break;
		case 9:  // NAL access unit delimiter
			break;
		case 10: // End of sequence
			break;
		case 11: // End of stream
			break;
		case 12: // Filler data
			break;
		case 13: // Sequence parameter set extension
			break;
		case 14: // Prefix NAL unit
			break;
		case 15: // Subset sequence parameter set
			break;
			// Reserved
		case 16:
		case 17:
		case 18:
			break;
			// unspecified
		case 0:
		case 21:
		case 22:
		case 23:
		default:
			break;
		}
	}
	else
	{
		switch (buffer[INDEX(3)])
		{
			// Sequence Header
		case 0xB3:
		{
			m_frameWidth = (((int)buffer[INDEX(4)]) << 4) | (((int)buffer[INDEX(5)]) >> 4);
			m_frameHeight = ((((int)buffer[INDEX(5)]) & 0x0F) << 8) | ((int)buffer[INDEX(6)]);
			if ((m_nullPFrameWidth != m_frameWidth) || (m_nullPFrameHeight != m_frameHeight))
			{
				INFO("TSProcessor: sequence frame size %dx%d\n", m_frameWidth, m_frameHeight);
			}
			keepScanning = false;
		}
		break;
		default:
			if ((buffer[INDEX(3)] >= 0x01) && (buffer[INDEX(3)] <= 0xAF))
			{
				// We have hit a slice.  Stop looking for the frame size.  This must
				// be an I-frame inside a sequence.
				keepScanning = false;
			}
			break;
		}
	}

	return result;
}


/**
 * @brief Updates state variables depending on interlaced
 * @param[in] packet buffer containing TS packet
 * @param[in] length length of buffer
 */
void TSProcessor::checkIfInterlaced(unsigned char *packet, int length)
{
	unsigned char* packetEnd = packet + length;
	for (int i = 0; i < length; i += m_packetSize)
	{
		packet += m_ttsSize;

		int pid = (((packet[1] & 0x1F) << 8) | (packet[2] & 0xFF));
		int payloadStart = (packet[1] & 0x40);
		int adaptation = ((packet[3] & 0x30) >> 4);
		int payload = 4;

		{
			if (pid == m_pcrPid)
			{
				if (adaptation & 0x02)
				{
					payload += (1 + packet[4]);
				}
			}
			if (adaptation & 0x01)
			{
				// Update PTS/DTS values
				if (payloadStart)
				{
					if ((packet[payload] == 0x00) && (packet[payload + 1] == 0x00) && (packet[payload + 2] == 0x01))
					{
						int streamid = packet[payload + 3];
						int pesHeaderDataLen = packet[payload + 8];
						int tsbase = payload + 9;

						if ((streamid >= 0xE0) && (streamid <= 0xEF))
						{
							// Video
							m_scanForFrameSize = true;
						}
						if (packet[payload + 7] & 0x80)
						{
							tsbase += 5;
						}
						if (packet[payload + 7] & 0x40)
						{
							tsbase += 5;
						}
						payload = payload + 9 + pesHeaderDataLen;
					}
				}

				if (m_scanForFrameSize && (m_videoPid != -1) && (pid == m_videoPid))
				{
					int j, jmax;

					if (m_scanRemainderSize)
					{
						unsigned char *remainder = m_scanRemainder;
						int copyLen;
						int srcCapacity;

						srcCapacity = packetEnd - (packet + payload);
						if (srcCapacity > 0)
						{
							copyLen = m_scanRemainderLimit + (m_scanRemainderLimit - m_scanRemainderSize);
							if (copyLen > packetEnd - (packet + payload))
							{
								INFO("scan copyLen adjusted from %d to %d\n", copyLen, (int)(packetEnd - (packet + payload)));
								copyLen = packetEnd - (packet + payload);
							}
							rmf_osal_memcpy(remainder + m_scanRemainderSize, packet + payload, copyLen, m_scanRemainderLimit * 3 - m_scanRemainderSize, packetEnd - (packet + payload));
							m_scanRemainderSize += copyLen;
							if (m_scanRemainderSize >= m_scanRemainderLimit * 2)
							{
								for (j = 0; j < m_scanRemainderLimit; ++j)
								{
									if ((remainder[j] == 0x00) && (remainder[j + 1] == 0x00) && (remainder[j + 2] == 0x01))
									{
										processStartCode(&remainder[j], m_scanForFrameSize, 2 * m_scanRemainderLimit - j, j);
									}
								}

								m_scanRemainderSize = 0;
							}
						}
						else
						{
							m_scanRemainderSize = 0;
							INFO("TSProcessor::checkIfInterlaced: scan skipped");
							//TEMP
							if (m_scanSkipPacketsEnabled){
								FILE *pFile = fopen("/opt/trick-scan.dat\n", "wb");
								if (pFile)
								{
									fwrite(packet, 1, m_packetSize - m_ttsSize, pFile);
									fclose(pFile);
									INFO("scan skipped: packet writing to /opt/trick-scan.dat");
								}
							}
							//TEMP
						}
					}

					if (m_scanForFrameSize)
					{
						// We need to stop scanning at the point
						// where we can no longer access all needed start code bytes.
						jmax = m_packetSize - m_scanRemainderLimit - m_ttsSize;

						for (j = payload; j < jmax; ++j)
						{
							if ((packet[j] == 0x00) && (packet[j + 1] == 0x00) && (packet[j + 2] == 0x01))
							{
								processStartCode(&packet[j], m_scanForFrameSize, jmax - j, j);

								if (!m_scanForFrameSize || m_isInterlacedKnown)
								{
									break;
								}
							}
						}

						if (m_scanForFrameSize)
						{
							unsigned char* packetScanPosition = packet + m_packetSize - m_scanRemainderLimit - m_ttsSize;
							m_scanRemainderSize = m_scanRemainderLimit < packetEnd - packetScanPosition ? m_scanRemainderLimit : packetEnd - packetScanPosition;
							rmf_osal_memcpy(m_scanRemainder, packetScanPosition, m_scanRemainderSize, m_scanRemainderLimit * 3, packetEnd - packetScanPosition);
						}
					}
				}
			}
		}

		if (m_isInterlacedKnown)
		{
			break;
		}

		packet += (m_packetSize - m_ttsSize);
	}

	m_scanRemainderSize = 0;
}


/**
 * @brief Does PTS re-stamping
 * @param[in,out] packet TS data to re-stamp
 * @param[in] length[in] TS data size
 */
void TSProcessor::reTimestamp(unsigned char *&packet, int length)
{
	long long PCR = 0;
	unsigned char *pidFilter;
	unsigned char* packetEnd = packet + length;

	if (m_isH264 && !m_isInterlacedKnown)
	{
		checkIfInterlaced(packet, length);
		TRACE1("m_isH264 = %s m_isInterlacedKnown = %s m_isInterlaced %s\n", m_isH264 ? "true" : "false",
			m_isInterlacedKnown ? "true" : "false", m_isInterlaced ? "true" : "false");
	}

	// For MPEG2 use twice the desired frame rate for IOnly re-timestamping since
	// we insert a null P-frame after every I-frame.
	float rm = ((m_isH264 && !m_isInterlaced) ? 1.0 : 2.0);

	if (!m_haveBaseTime) m_basePCR = -1LL;

	TRACE4("reTimestamp: packet %p length %d\n", packet, length);
	for (int i = 0; i < length; i += m_packetSize)
	{
		packet += m_ttsSize;

		int pid = (((packet[1] & 0x1F) << 8) | (packet[2] & 0xFF));
		int payloadStart = (packet[1] & 0x40);
		int adaptation = ((packet[3] & 0x30) >> 4);
		int payload = 4;
		int updatePCR = 0;

		// Apply pid filter
		pidFilter = (m_trickExcludeAudio ? m_pidFilterTrick : m_pidFilter);
		if (!pidFilter[pid])
		{
			// Change to null packet
			packet[1] = ((packet[1] & 0xE0) | 0x1F);
			packet[2] = 0xFF;
		}

		// Update continuity counter
		unsigned char byte3 = packet[3];
		if (byte3 & 0x10)
		{
			packet[3] = ((byte3 & 0xF0) | (m_continuityCounters[pid]++ & 0x0F));
		}

	  {
		  // Update PCR values
		  if (pid == m_pcrPid)
		  {
			  if (payloadStart) m_basePCR = -1LL;

			  if (adaptation & 0x02)
			  {
				  int adaptationFlags = packet[5];
				  if (adaptationFlags & 0x10)
				  {
					  long long timeOffset, timeOffsetBase, rateAdjustedPCR;

					  PCR = readPCR(&packet[6]);
					  if (!m_haveBaseTime)
					  {
						  m_haveBaseTime = true;
						  m_baseTime = PCR;
						  m_segmentBaseTime = m_baseTime;
						  INFO("have baseTime %llx from pid %x PCR\n", m_baseTime, pid);
					  }
					  if (m_basePCR < 0) m_basePCR = PCR;
					  if (m_playMode == PlayMode_retimestamp_Ionly)
					  {
						  rateAdjustedPCR = ((m_currRateAdjustedPTS - 10000) & 0x1FFFFFFFFLL);
					  }
					  else
					  {
						  timeOffset = PCR - m_baseTime;
						  if (m_playRate < 0.0)
						  {
							  timeOffset = -timeOffset;
							  if (m_basePCR >= 0)
							  {
								  timeOffsetBase = m_baseTime - m_basePCR;
								  if (timeOffset < timeOffsetBase)
								  {
									  // When playing in reverse, a gven frame may contain multiple PCR values 
									  // and we must keep them all increasing in value
									  timeOffset = timeOffsetBase + (timeOffsetBase - timeOffset);
								  }
							  }
						  }
						  rateAdjustedPCR = (((long long)(timeOffset / m_absPlayRate + 0.5) + m_segmentBaseTime) & 0x1FFFFFFFFLL);
					  }
					  m_currRateAdjustedPCR = rateAdjustedPCR;
					  ++m_pcrPerPTSCount;
					  updatePCR = 1;
				  }
				  payload += (1 + packet[4]);
			  }
		  }

		  if (adaptation & 0x01)
		  {
			  // Update PTS/DTS values
			  if (payloadStart)
			  {
				  if (m_haveBaseTime || (m_playMode == PlayMode_retimestamp_Ionly))
				  {
					  if ((packet[payload] == 0x00) && (packet[payload + 1] == 0x00) && (packet[payload + 2] == 0x01))
					  {
						  int streamid = packet[payload + 3];
						  int pesHeaderDataLen = packet[payload + 8];
						  int tsbase = payload + 9;

						  if ((streamid >= 0xE0) && (streamid <= 0xEF))
						  {
							  // Video
							  if (m_playMode == PlayMode_retimestamp_Ionly)
							  {
								  m_scanForFrameSize = true;
								  if (m_isH264)
								  {
									  m_updatePicOrderCount = true;
								  }
							  }
						  }
						  else if (((streamid >= 0xC0) && (streamid <= 0xDF)) || (streamid == 0xBD))
						  {
							  // Audio
						  }

						  long long timeOffset;
						  long long PTS = 0, DTS;
						  long long rateAdjustedPTS = 0, rateAdjustedDTS;
						  bool validPTS = false;
						  if (packet[payload + 7] & 0x80)
						  {
							  validPTS = readTimeStamp(&packet[tsbase], PTS);
							  if (validPTS)
							  {
								  if (pid == m_pcrPid)
								  {
									  m_pcrPerPTSCount = 0;
								  }
								  if (!m_haveBaseTime && (m_playMode == PlayMode_retimestamp_Ionly))
								  {
									  m_haveBaseTime = true;
									  m_baseTime = ((PTS - ((long long)(90000 / (m_apparentFrameRate*rm)))) & 0x1FFFFFFFFLL);
									  m_segmentBaseTime = m_baseTime;
									  TRACE2("have baseTime %llx from pid %x PTS\n", m_baseTime, pid);
								  }
								  timeOffset = PTS - m_baseTime;
								  if (m_playRate < 0) timeOffset = -timeOffset;
								  if ((pid == m_pcrPid) && (m_playMode == PlayMode_retimestamp_Ionly))
								  {
									  long long interFrameDelay = 90000 / (m_apparentFrameRate*rm);
									  if (!m_haveUpdatedFirstPTS)
									  {
										  if (m_currRateAdjustedPCR != 0)
										  {
											  rateAdjustedPTS = ((m_currRateAdjustedPCR + 10000) & 0x1FFFFFFFFLL);
											  TRACE2("Updated rateAdjustedPTS to %lld m_currRateAdjustedPCR %lld\n", rateAdjustedPTS, m_currRateAdjustedPCR);
										  }
										  else
										  {
											  rateAdjustedPTS = PTS;
											  TRACE2("Updated rateAdjustedPTS to %lld m_currRateAdjustedPCR %lld\n", rateAdjustedPTS, m_currRateAdjustedPCR);
										  }
										  m_haveUpdatedFirstPTS = true;
									  }
									  else
									  {
										  rateAdjustedPTS = ((m_currRateAdjustedPTS + interFrameDelay) & 0x1FFFFFFFFLL);
										  TRACE2("Updated rateAdjustedPTS to %lld m_currRateAdjustedPTS %lld interFrameDelay %lld\n", rateAdjustedPTS, m_currRateAdjustedPTS, interFrameDelay);
										  /*Don't increment pts with interFrameDelay if already done for the segment.*/
										  if (m_framesProcessedInSegment > 0)
										  {
											  TRACE4("Not incrementing pts with interFrameDelay as already done for the segment");
											  if (m_playRate != 0)
											  {
												  rateAdjustedPTS = m_currRateAdjustedPTS + ((PTS - m_lastPTSOfSegment) / m_playRate);
											  }
											  else
											  {
												  rateAdjustedPTS = m_currRateAdjustedPTS + (PTS - m_lastPTSOfSegment);
											  }
										  }

										  if (updatePCR)
										  {
											  long long rateAdjustedPCR = ((rateAdjustedPTS - 10000) & 0x1FFFFFFFFLL);

											  m_currRateAdjustedPCR = rateAdjustedPCR;
										  }
									  }
									  m_currRateAdjustedPTS = rateAdjustedPTS;
								  }
								  else
								  {
									  rateAdjustedPTS = (((long long)(timeOffset / m_absPlayRate + 0.5) + m_segmentBaseTime) & 0x1FFFFFFFFLL);
								  }
								  if (pid == m_pcrPid)
								  {
									  m_throttlePTS = rateAdjustedPTS;
									  TRACE2("Updated throttlePTS to %lld\n", m_throttlePTS);
								  }
								  writeTimeStamp(&packet[tsbase], packet[tsbase] >> 4, rateAdjustedPTS);
								  m_lastPTSOfSegment = PTS;
								  m_framesProcessedInSegment++;

								  TRACE2("rateAdjustedPTS %lld (%lld ms)\n", rateAdjustedPTS, rateAdjustedPTS / 90);
							  }
							  tsbase += 5;
						  }
						  if (packet[payload + 7] & 0x40)
						  {
							  if (validPTS)
							  {
								  bool validDTS = false;
								  if ((pid == m_pcrPid) && (m_playMode == PlayMode_retimestamp_Ionly))
								  {
									  rateAdjustedDTS = rateAdjustedPTS - (2 * 750);
									  validDTS = true;
								  }
								  else
								  {
									  bool validDTS = readTimeStamp(&packet[tsbase], DTS);
									  if (validDTS)
									  {
										  timeOffset = DTS - m_baseTime;
										  if (m_playRate < 0) timeOffset = -timeOffset;
										  rateAdjustedDTS = (((long long)(timeOffset / m_absPlayRate + 0.5) + m_segmentBaseTime) & 0x1FFFFFFFFLL);
									  }
								  }
								  if (validDTS)
								  {
									  writeTimeStamp(&packet[tsbase], packet[tsbase] >> 4, rateAdjustedDTS);
								  }
							  }
							  tsbase += 5;
						  }
						  if (packet[payload + 7] & 0x02)
						  {
							  // CRC flag is set.  Following the PES header will be the CRC for the previous PES packet
							  WARNING("Warning: PES packet has CRC flag set");
						  }
						  payload = payload + 9 + pesHeaderDataLen;
					  }
				  }
			  }
			  if (m_scanForFrameSize && (m_videoPid != -1) && (pid == m_videoPid))
			  {
				  int j, jmax;

				  if (m_scanRemainderSize)
				  {
					  unsigned char *remainder = m_scanRemainder;
					  int srcCapacity;
					  int copyLen;

					  srcCapacity = packetEnd - (packet + payload);
					  if (srcCapacity > 0)
					  {
						  copyLen = 2 * m_scanRemainderLimit + (m_scanRemainderLimit - m_scanRemainderSize);
						  if (copyLen > packetEnd - (packet + payload))
						  {
							  INFO("scan copyLen adjusted from %d to %d\n", copyLen, (int)(packetEnd - (packet + payload)));
							  copyLen = packetEnd - (packet + payload);
						  }
						  rmf_osal_memcpy(remainder + m_scanRemainderSize, packet + payload, copyLen, m_scanRemainderLimit * 3 - m_scanRemainderSize, packetEnd - (packet + payload));
						  m_scanRemainderSize += copyLen;

						  if (m_scanRemainderSize >= m_scanRemainderLimit * 3)
						  {
							  for (j = 0; j < m_scanRemainderLimit; ++j)
							  {
								  if ((remainder[j] == 0x00) && (remainder[j + 1] == 0x00) && (remainder[j + 2] == 0x01))
								  {
									  processStartCode(&remainder[j], m_scanForFrameSize, 3 * m_scanRemainderLimit - j, j);
									  rmf_osal_memcpy(packet + payload, remainder + m_scanRemainderLimit, 2 * m_scanRemainderLimit, packetEnd - (packet + payload), m_scanRemainderLimit * 3 - m_scanRemainderLimit);
								  }
							  }

							  m_scanRemainderSize = 0;
						  }
					  }
					  else
					  {
						  m_scanRemainderSize = 0;
						  INFO("TSProcessor::reTimestamp: scan skipped");
						  //TEMP
						  if (m_scanSkipPacketsEnabled){
							  FILE *pFile = fopen("/opt/trick-scan.dat", "wb");
							  if (pFile)
							  {
								  fwrite(packet, 1, m_packetSize - m_ttsSize, pFile);
								  fclose(pFile);
								  INFO("scan skipped: packet writing to /opt/trick-scan.dat");
							  }
						  }
						  //TEMP
					  }
				  }

				  if (m_scanForFrameSize)
				  {
					  // We need to stop scanning at the point
					  // where we can no longer access all needed start code bytes.
					  jmax = m_packetSize - m_scanRemainderLimit - m_ttsSize;

					  for (j = payload; j < jmax; ++j)
					  {
						  if ((packet[j] == 0x00) && (packet[j + 1] == 0x00) && (packet[j + 2] == 0x01))
						  {
							  processStartCode(&packet[j], m_scanForFrameSize, jmax - j, j);

							  if (!m_scanForFrameSize)
							  {
								  break;
							  }
						  }
					  }

					  if (m_scanForFrameSize)
					  {
						  unsigned char* packetScanPosition = packet + m_packetSize - m_scanRemainderLimit - m_ttsSize;
						  if (packetScanPosition < packetEnd)
						  {
							  m_scanRemainderSize = m_scanRemainderLimit < packetEnd - packetScanPosition ? m_scanRemainderLimit : packetEnd - packetScanPosition;
						  }
						  else
						  {
							  INFO("Scan reached out of bound packet packetScanPosition=%p\n", packetScanPosition);
							  m_scanRemainderSize = 0;
							  packetScanPosition = packetEnd;
						  }
						  rmf_osal_memcpy(m_scanRemainder, packetScanPosition, m_scanRemainderSize, m_scanRemainderLimit * 3, packetEnd - packetScanPosition);
					  }
				  }
			  }
		  }
	  }
	  if (updatePCR)
	  {
		  // If we repeat an I-frame as part of achieving the desired rate, we need to make sure
		  // the PCR values don't repeat
		  if (m_currRateAdjustedPCR <= m_prevRateAdjustedPCR)
		  {
			  m_currRateAdjustedPCR = ((long long)(m_currRateAdjustedPCR + (90000 / (m_apparentFrameRate*rm) + m_pcrPerPTSCount * 8)) & 0x1FFFFFFFFLL);
		  }
		  TRACE2("m_currRateAdjustedPCR %lld(%lld ms) diff %lld ms\n", m_currRateAdjustedPCR, m_currRateAdjustedPCR / 90, (m_currRateAdjustedPCR - m_prevRateAdjustedPCR) / 90);
		  m_prevRateAdjustedPCR = m_currRateAdjustedPCR;
		  writePCR(&packet[6], m_currRateAdjustedPCR, ((m_absPlayRate >= 4.0) ? true : false));
	  }
	  packet += (m_packetSize - m_ttsSize);
	}
}

/**
* @brief Set to the playback mode.
*
* @param[in] mode play mode such as PlayMode_normal, PlayMode_retimestamp_Ionly,
* PlayMode_retimestamp_IPB, PlayMode_retimestamp_IandP or PlayMode_reverse_GOP.
*
* @note Not relevant for demux operations
*/
void TSProcessor::setPlayMode(PlayMode mode)
{
	INFO("setting playback mode to %s\n", (mode == PlayMode_normal) ? "PlayMode_normal" :
		(mode == PlayMode_retimestamp_IPB) ? "PlayMode_retimestamp_IPB" :
		(mode == PlayMode_retimestamp_IandP) ? "PlayMode_retimestamp_IandP" :
		(mode == PlayMode_retimestamp_Ionly) ? "PlayMode_retimestamp_Ionly" :
		"PlayMode_reverse_GOP");
	m_playModeNext = mode;
}


/**
 * @brief Abort TSProcessor operations and return blocking calls immediately
 * @note Make sure that caller holds m_mutex before invoking this function
 */
void TSProcessor::abortUnlocked()
{
	m_enabled = false;
	pthread_cond_signal(&m_basePTSCond);
	while (m_processing)
	{
		pthread_cond_signal(&m_throttleCond);
		INFO("Waiting for processing to end");
		pthread_cond_wait(&m_throttleCond, &m_mutex);
	}
}

/**
 * @brief Abort current operations and return all blocking calls immediately.
 */
void TSProcessor::abort()
{
	pthread_mutex_lock(&m_mutex);
	abortUnlocked();
	pthread_mutex_unlock(&m_mutex);
}


/**
* @brief Set the playback rate.
*
* @param[in] rate play rate could be 1.0=Normal Playback, 0.0=Pause, etc
* @param[in] mode play mode such as PlayMode_normal, PlayMode_retimestamp_Ionly,
* PlayMode_retimestamp_IPB, PlayMode_retimestamp_IandP or PlayMode_reverse_GOP.
*
* @note mode is not relevant for demux operations
*/
void TSProcessor::setRate(double rate, PlayMode mode)
{
	pthread_mutex_lock(&m_mutex);
	m_havePAT = false;
	m_havePMT = false;
	abortUnlocked();
	m_playRateNext = rate;
	INFO("set playback rate to %f\n", m_playRateNext);
	setPlayMode(mode);
	m_enabled = true;
	m_startPosition = -1.0;
	m_last_frame_time = 0;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * @brief Enable/ disable throttle
 * @param[in] enable true to enable throttle, false to disable
 */
void TSProcessor::setThrottleEnable(bool enable)
{
	INFO("TSProcessor::setThrottleEnable enable=%d\n", enable);
	m_throttle = enable;
}


/**
 * @brief generate PAT and PMT based on media components
 * @param[in] trick true on trickmode
 * @param[out] buff PAT and PMT copied to this buffer
 * @param[out] buflen Length of buff
 * @param[in] bHandleMCTrick true if audio pid is same as PCR pid
 */
bool TSProcessor::generatePATandPMT(bool trick, unsigned char **buff, int *buflen, bool bHandleMCTrick)
{
	bool result = false;
	int i, j;
	int prognum, pmtVersion, pmtPid, pcrPid;
	int pmtSectionLen = 0;
	bool pcrPidFound;
	int audioComponentCount;
	const RecordingComponent* audioComponents;

	if (eStreamOp_SEND_VIDEO_AND_QUEUED_AUDIO == m_streamOperation)
	{
		m_peerTSProcessor->getAudioComponents(&audioComponents, audioComponentCount);
		INFO("Got audioComponents from  audio track. audioComponentCount %d\n", audioComponentCount);
	}
	else
	{
		audioComponentCount = this->audioComponentCount;
		audioComponents = this->audioComponents;
	}

	prognum = m_program;
	pmtVersion = m_versionPMT;
	pmtPid = m_pmtPid;
	pcrPid = m_pcrPid;


	if (videoComponentCount == 0)
	{
		DEBUG("no video, so keep audio in trick mode PMT");
		trick = false;
	}
	if ((videoComponentCount == 0) && (audioComponentCount == 0))
	{
		ERROR("generatePATandPMT: insufficient stream information - no PAT/PMT?\n");
	}

	if (videoComponentCount > 0)
	{
		m_isH264 = (videoComponents[0].elemStreamType == 0x1B);
		m_scanRemainderLimit = (m_isH264 ? SCAN_REMAINDER_SIZE_H264 : SCAN_REMAINDER_SIZE_MPEG2);
	}

	if (pmtVersion == -1)
	{
		pmtVersion = 1;
	}

	// Establish pmt and pcr   
	pcrPidFound = false;
	if (pmtPid == -1)
	{
		// Choose a pmt pid if not known and check pcr pid.  Pids 0-F are reserved
		for (pmtPid = 0x10; pmtPid < 0x1fff; ++pmtPid)
		{
			if (pmtPid == pcrPid)
			{
				continue;
			}
			for (i = 0; i < videoComponentCount; ++i)
			{
				if (pcrPid == videoComponents[i].pid) pcrPidFound = true;
				if (pmtPid == videoComponents[i].pid) break;
			}
			if (i < videoComponentCount) continue;

			if (!trick)
			{
				for (i = 0; i < audioComponentCount; ++i)
				{
					if (pcrPid == audioComponents[i].pid) pcrPidFound = true;
					if (pmtPid == audioComponents[i].pid) break;
				}
				if (pcrPidFound)
				{
					INFO("It is possibly MC Channel..");
					m_isMCChannel = true;
				}
				if (i < audioComponentCount) continue;
			}
			break;
		}
	}
	else
	{
		// Check pcr pid
		for (i = 0; i < videoComponentCount; ++i)
		{
			if (pcrPid == videoComponents[i].pid) pcrPidFound = true;
		}

		if ((!trick) && (!pcrPidFound))
		{
			for (i = 0; i < audioComponentCount; ++i)
			{
				if (pcrPid == audioComponents[i].pid) pcrPidFound = true;
			}
			if (pcrPidFound)
			{
				INFO("It is possibly MC Channel..");
				m_isMCChannel = true;
			}
		}
	}

	if (bHandleMCTrick && (!trick))
	{
		DEBUG("For MC channel where in audio is the PCR PID, ensure that VID PID is used as PCR PID during trick mode and change the PMT Version.");
		pcrPidFound = false;
		pmtVersion++;
	}

	if (!pcrPidFound)
	{
		/* If it is MC channel where in audio is the PCR PID && trick is true (ie no audio during trick), then the VID PID will become new PCR PID; So please update the PMT Version */
		if (trick)
		{
			INFO("If it is MC channel where in audio is the PCR PID && trick is true (ie no audio during trick), then the VID PID will become new PCR PID; So update the PMT Version by 1");
			pmtVersion++;
		}

		// With older recordings, the pcr pid was set incorrectly in the
		// meta-data - it was actually the original pmt pid.  If the pcrPid 
		// wasn't found in the components, fall back to a pes pid.
		if (videoComponentCount)
		{
			pcrPid = videoComponents[0].pid;
		}
		else if (!trick && audioComponentCount)
		{
			pcrPid = audioComponents[0].pid;
		}
		else
		{
			pcrPid = 0x1fff;
		}
	}

	if ((pmtPid < 0x1fff) && (pcrPid < 0x1fff))
	{
		DEBUG("using pmt pid %04x pcr pid %04X\n", pmtPid, pcrPid);

		m_pcrPid = pcrPid;

		int pmtSize = 17 + m_ttsSize;
		pmtSize += videoComponentCount * 5;
		if (!trick)
		{
			for (i = 0; i < audioComponentCount; ++i)
			{
				pmtSize += 5;
				int nameLen = audioComponents[i].associatedLanguage ? strlen(audioComponents[i].associatedLanguage) : 0;
				if (nameLen)
				{
					pmtSize += (3 + nameLen);
				}
			}
		}

		pmtSize += 4; //crc

		DEBUG("pmt payload size %d bytes\n", pmtSize);
		int pmtPacketCount = 1;
		i = pmtSize - (m_packetSize - 17 - m_ttsSize);
		while (i > 0)
		{
			++pmtPacketCount;
			i -= (m_packetSize - m_ttsSize - 4 - 4);  // TTS header, 4 byte TS header, 4 byte CRC
		}
		if (pmtPacketCount > 1)
		{
			WARNING("================= pmt requires %d packets =====================\n", pmtPacketCount);
		}

		int patpmtLen = (pmtPacketCount + 1)*m_packetSize*sizeof(unsigned char);
		unsigned char *patpmt = (unsigned char*)malloc(patpmtLen);
		TRACE1("patpmtLen %d, patpmt %p\n", patpmtLen, patpmt);
		if (patpmt)
		{
			int temp;
			uint32_t crc;
			int version = 1;
			unsigned char *patPacket = &patpmt[0];

			if (prognum == 0)
			{
				// If program is not known use 1
				prognum = 1;
			}

			// Generate PAT
			i = 0;
			if (m_ttsSize)
			{
				memset(patPacket, 0, m_ttsSize);
				i += m_ttsSize;
			}
			patPacket[i + 0] = 0x47; // Sync Byte
			patPacket[i + 1] = 0x60; // TEI=no ; Payload Start=yes; Prio=0; 5 bits PId=0
			patPacket[i + 2] = 0x00; // 8 bits LSB PID = 0
			patPacket[i + 3] = 0x10; // 2 bits Scrambling = no; 2 bits adaptation field = no adaptation; 4 bits continuity counter

			patPacket[i + 4] = 0x00; // Payload start=yes, hence this is the offset to start of section
			patPacket[i + 5] = 0x00; // Start of section, Table ID = 0 (PAT)
			patPacket[i + 6] = 0xB0; // 4 bits fixed = 1011; 4 bits MSB section length
			patPacket[i + 7] = 0x0D; // 8 bits LSB section length (length = remaining bytes following this field including CRC)

			patPacket[i + 8] = 0x00; // TSID : Don't care
			patPacket[i + 9] = 0x01; // TSID : Don't care

			temp = version << 1;
			temp = temp & 0x3E; //Masking first 2 bits and last one bit : 0011 1110 (3E)
			patPacket[i + 10] = 0xC1 | temp; //C1 : 1100 0001 : setting reserved bits as 1, current_next_indicator as 1

			patPacket[i + 11] = 0x00; // Section #
			patPacket[i + 12] = 0x00; // Last section #

			// 16 bit program number of stream 
			patPacket[i + 13] = (prognum >> 8) & 0xFF;
			patPacket[i + 14] = prognum & 0xFF;

			// PMT PID
			// Reserved 3 bits are set to 1
			patPacket[i + 15] = 0xE0;
			// Copying bits 8 through 12..
			patPacket[i + 15] |= (unsigned char)((pmtPid >> 8) & 0x1F);
			//now copying bits 0 through 7..
			patPacket[i + 16] = (unsigned char)(0xFF & pmtPid);

			// 4 bytes of CRC
			crc = get_crc32(&patPacket[i + 5], 12);
			patPacket[i + 17] = (crc >> 24) & 0xFF;
			patPacket[i + 18] = (crc >> 16) & 0xFF;
			patPacket[i + 19] = (crc >> 8) & 0xFF;
			patPacket[i + 20] = crc & 0xFF;

			// Fill stuffing bytes for rest of TS packet
			for (i = 21 + m_ttsSize; i < m_packetSize; i++)
			{
				patPacket[i] = 0xFF;
			}

			// Generate PMT
			unsigned char *pmtPacket = &patpmt[m_packetSize];

			i = 0;
			if (m_ttsSize)
			{
				memset(pmtPacket, 0, m_ttsSize);
				i += m_ttsSize;
			}
			pmtPacket[i + 0] = 0x47;
			pmtPacket[i + 1] = 0x60;
			pmtPacket[i + 1] |= (unsigned char)((pmtPid >> 8) & 0x1F);
			pmtPacket[i + 2] = (unsigned char)(0xFF & pmtPid);
			pmtPacket[i + 3] = 0x10; // 2 bits Scrambling = no; 2 bits adaptation field = no adaptation; 4 bits continuity counter

			pmtSectionLen = pmtSize - m_ttsSize - 8;
			pmtPacket[i + 4] = 0x00;
			pmtPacket[i + 5] = 0x02;
			pmtPacket[i + 6] = (0xB0 | ((pmtSectionLen >> 8) & 0xF));
			pmtPacket[i + 7] = (pmtSectionLen & 0xFF); //lower 8 bits of Section length

			// 16 bit program number of stream 
			pmtPacket[i + 8] = (prognum >> 8) & 0xFF;
			pmtPacket[i + 9] = prognum & 0xFF;

			temp = pmtVersion << 1;
			temp = temp & 0x3E; //Masking first 2 bits and last one bit : 0011 1110 (3E)
			pmtPacket[i + 10] = 0xC1 | temp; //C1 : 1100 0001 : setting reserved bits as 1, current_next_indicator as 1

			pmtPacket[i + 11] = 0x00;
			pmtPacket[i + 12] = 0x00;

			pmtPacket[i + 13] = 0xE0;
			pmtPacket[i + 13] |= (unsigned char)((pcrPid >> 8) & 0x1F);
			pmtPacket[i + 14] = (unsigned char)(0xFF & pcrPid);
			pmtPacket[i + 15] = 0xF0;
			pmtPacket[i + 16] = 0x00; //pgm info length.  No DTCP descr here..

			int pi = i + 17;
			unsigned char byte;
			for (j = 0; j < videoComponentCount; ++j)
			{
				int videoPid = videoComponents[j].pid;
				m_videoPid = videoPid;
				putPmtByte(pmtPacket, pi, videoComponents[j].elemStreamType, pmtPid);
				byte = (0xE0 | (unsigned char)((videoPid >> 8) & 0x1F));
				putPmtByte(pmtPacket, pi, byte, pmtPid);
				byte = (unsigned char)(0xFF & videoPid);
				putPmtByte(pmtPacket, pi, byte, pmtPid);
				putPmtByte(pmtPacket, pi, 0xF0, pmtPid);
				putPmtByte(pmtPacket, pi, 0x00, pmtPid);
			}
			if (!trick)
			{
				for (j = 0; j < audioComponentCount; ++j)
				{
					int audioPid = audioComponents[j].pid;
					int nameLen = audioComponents[j].associatedLanguage ? strlen(audioComponents[j].associatedLanguage) : 0;
					putPmtByte(pmtPacket, pi, audioComponents[j].elemStreamType, pmtPid);
					byte = (0xE0 | (unsigned char)((audioPid >> 8) & 0x1F));
					putPmtByte(pmtPacket, pi, byte, pmtPid);
					byte = (unsigned char)(0xFF & audioPid);
					putPmtByte(pmtPacket, pi, byte, pmtPid);
					putPmtByte(pmtPacket, pi, 0xF0, pmtPid);
					if (nameLen)
					{
						putPmtByte(pmtPacket, pi, (3 + nameLen), pmtPid);
						putPmtByte(pmtPacket, pi, 0x0A, pmtPid);
						putPmtByte(pmtPacket, pi, (1 + nameLen), pmtPid);
						for (int k = 0; k < nameLen; ++k)
						{
							putPmtByte(pmtPacket, pi, audioComponents[j].associatedLanguage[k], pmtPid);
						}
					}
					putPmtByte(pmtPacket, pi, 0x00, pmtPid);
				}
			}
			// Calculate crc
			unsigned char *crcData = &patpmt[m_packetSize + m_ttsSize + 5];
			int crcLenTotal = pmtSize - m_ttsSize - 5 - 4;
			int crcLen = ((crcLenTotal > (m_packetSize - m_ttsSize - 5)) ? (m_packetSize - m_ttsSize - 5) : crcLenTotal);
			crc = 0xffffffff;
			while (crcLenTotal)
			{
				crc = get_crc32(crcData, crcLen, crc);
				crcData += crcLen;
				crcLenTotal -= crcLen;
				if (crcLenTotal < crcLen)
				{
					crcLen = crcLenTotal;
				}
			}
			putPmtByte(pmtPacket, pi, ((crc >> 24) & 0xFF), pmtPid);
			putPmtByte(pmtPacket, pi, ((crc >> 16) & 0xFF), pmtPid);
			putPmtByte(pmtPacket, pi, ((crc >> 8) & 0xFF), pmtPid);
			putPmtByte(pmtPacket, pi, (crc & 0xFF), pmtPid);

			// Fill stuffing bytes for rest of TS packet
			for (i = pi; i < m_packetSize; i++)
			{
				pmtPacket[i] = 0xFF;
			}

			TRACE1("generated PAT and PMT:");
			dumpPackets(patpmt, patpmtLen, m_packetSize);

			*buflen = patpmtLen;
			*buff = patpmt;

			if (trick)
			{
				// Setup pid filter for trick mode.  Block all pids except for
				// pat, pmt, pcr, video
				memset(m_pidFilterTrick, 0, sizeof(m_pidFilterTrick));
				TRACE1("pass pat %04x, pmt %04x pcr %04x\n", 0, pmtPid, pcrPid);
				m_pidFilterTrick[pcrPid] = 1;
				for (i = 0; i < videoComponentCount; ++i)
				{
					int videoPid = videoComponents[i].pid;
					TRACE1("video %04x\n", videoPid);
					m_pidFilterTrick[videoPid] = 1;
				}
				TRACE4("\n");
			}
			else
			{
				// Setup pid filter.  Block all pids except for
				// pcr, video, audio
				memset(m_pidFilter, 0, sizeof(m_pidFilter));
				TRACE1("pass pat %04x, pcr %04x\n", 0, pcrPid);
				m_pidFilter[pcrPid] = 1;
				for (i = 0; i < videoComponentCount; ++i)
				{
					int videoPid = videoComponents[i].pid;
					TRACE1("video %04x\n", videoPid);
					m_pidFilter[videoPid] = 1;
				}
				for (i = 0; i < audioComponentCount; ++i)
				{
					int audioPid = audioComponents[i].pid;
					TRACE1("audio %04x\n", audioPid);
					m_pidFilter[audioPid] = 1;
				}
				TRACE4("\n");
			}

			result = true;
		}
	}

	m_patCounter = m_pmtCounter = 0;

	INFO("TSProcessor::generatePATandPMT: trick %d prognum %d pmtpid: %X pcrpid: %X pmt section len %d video %d audio %d\n",
		trick, prognum, pmtPid, pcrPid, pmtSectionLen,
		videoComponentCount, audioComponentCount);

	return result;
}


/**
 * @brief Appends a byte to PMT buffer
 * @param[in,out] pmt buffer in which PMT is being constructed
 * @param[in,out] index current index of PMT construction.
 * @param[in] byte byte to be written at index
 * @param[in] pmtPid PID of PMT
 */
void TSProcessor::putPmtByte(unsigned char* &pmt, int& index, unsigned char byte, int pmtPid)
{
	int i;
	pmt[index++] = byte;
	if (index > m_packetSize - 1)
	{
		pmt += m_packetSize;
		i = 0;
		if (m_ttsSize)
		{
			memset(pmt, 0, m_ttsSize);
			i += m_ttsSize;
		}
		pmt[i + 0] = 0x47;
		pmt[i + 1] = 0x20;
		pmt[i + 1] = (unsigned char)((pmtPid >> 8) & 0x1F);
		pmt[i + 2] = (unsigned char)(0xFF & pmtPid);
		pmt[i + 3] = 0x10;  // 2 bits Scrambling = no; 2 bits adaptation field = no adaptation; 4 bits continuity counter
		index = 4;
	}
}

// PTS/DTS format:
// YYYY vvvM vvvv vvvv vvvv vvvM vvvv vvvv vvvv vvvM
//
// value formed by concatinating all 'v''s.
// M bits are 1, YYYY are 0010 for PTS only
// and for PTS+DTS 0011 and 0001 respectively
//
// From ISO 13818-1

/**
 * @brief Read time-stamp at the point
 * @param[in] p buffer position containing time-stamp
 * @param[out] TS time-stamp
 * @retval true if time-stamp is present.
 */
bool TSProcessor::readTimeStamp(unsigned char *p, long long& TS)
{
	bool result = true;

	if ((p[4] & 0x01) != 1)
	{
		result = false;
		WARNING("TS:============ TS p[4] bit 0 not 1");
	}
	if ((p[2] & 0x01) != 1)
	{
		result = false;
		WARNING("TS:============ TS p[2] bit 0 not 1");
	}
	if ((p[0] & 0x01) != 1)
	{
		result = false;
		WARNING("TS:============ TS p[0] bit 0 not 1");
	}
	switch ((p[0] & 0xF0) >> 4)
	{
	case 1:
	case 2:
	case 3:
		break;
	default:
		result = false;
		WARNING("TS:============ TS p[0] YYYY bits have value %X\n", p[0]);
		break;
	}

	TS = ((((long long)(p[0] & 0x0E)) << 30) >> 1) |
		(((long long)(p[1] & 0xFF)) << 22) |
		((((long long)(p[2] & 0xFE)) << 15) >> 1) |
		(((long long)(p[3] & 0xFF)) << 7) |
		(((long long)(p[4] & 0xFE)) >> 1);

	return result;
}


/**
 * @brief Write time-stamp to buffer
 * @param[out] p buffer to which TS to be written
 * @param[in] prefix of time-stamp
 * @param[in] TS time-stamp
 */
void TSProcessor::writeTimeStamp(unsigned char *p, int prefix, long long TS)
{
	p[0] = (((prefix & 0xF) << 4) | (((TS >> 30) & 0x7) << 1) | 0x01);
	p[1] = ((TS >> 22) & 0xFF);
	p[2] = ((((TS >> 15) & 0x7F) << 1) | 0x01);
	p[3] = ((TS >> 7) & 0xFF);
	p[4] = ((((TS)& 0x7F) << 1) | 0x01);
}


/**
 * @brief Read PCR from a buffer
 * @param[in] p start of PCR data
 */
long long TSProcessor::readPCR(unsigned char *p)
{
	long long PCR = (((long long)(p[0] & 0xFF)) << (33 - 8)) |
		(((long long)(p[1] & 0xFF)) << (33 - 8 - 8)) |
		(((long long)(p[2] & 0xFF)) << (33 - 8 - 8 - 8)) |
		(((long long)(p[3] & 0xFF)) << (33 - 8 - 8 - 8 - 8)) |
		(((long long)(p[4] & 0xFF)) >> 7);
	return PCR;
}


/**
 * @brief Write PCR to a buffer
 * @param[out] p buffer to write PCR
 * @param[in] PCR timestamp to be written
 * @param[in] clearExtension clear PCR extension
 */
void TSProcessor::writePCR(unsigned char *p, long long PCR, bool clearExtension)
{
	p[0] = ((PCR >> (33 - 8)) & 0xFF);
	p[1] = ((PCR >> (33 - 8 - 8)) & 0xFF);
	p[2] = ((PCR >> (33 - 8 - 8 - 8)) & 0xFF);
	p[3] = ((PCR >> (33 - 8 - 8 - 8 - 8)) & 0xFF);
	if (!clearExtension)
	{
		p[4] = ((PCR & 0x01) << 7) | (p[4] & 0x7F);
	}
	else
	{
		p[4] = ((PCR & 0x01) << 7) | (0x7E);
		p[5] = 0x00;
	}
}

/**
 * @struct MBAddrIncCode
 * @brief holds macro block address increment codes
 */
struct MBAddrIncCode
{
	int numBits;
	int code;
};

static MBAddrIncCode macroblockAddressIncrementCodes[34] =
{
	{ 1, 0x001 },  /*  1 */
	{ 3, 0x003 },  /*  2 */
	{ 3, 0x002 },  /*  3 */
	{ 4, 0x003 },  /*  4 */
	{ 4, 0x002 },  /*  5 */
	{ 5, 0x003 },  /*  6 */
	{ 5, 0x002 },  /*  7 */
	{ 7, 0x007 },  /*  8 */
	{ 7, 0x006 },  /*  9 */
	{ 8, 0x00B },  /* 10 */
	{ 8, 0x00A },  /* 11 */
	{ 8, 0x009 },  /* 12 */
	{ 8, 0x008 },  /* 13 */
	{ 8, 0x007 },  /* 14 */
	{ 8, 0x006 },  /* 15 */
	{ 10, 0x017 },  /* 16 */
	{ 10, 0x016 },  /* 17 */
	{ 10, 0x015 },  /* 18 */
	{ 10, 0x014 },  /* 19 */
	{ 10, 0x013 },  /* 20 */
	{ 10, 0x012 },  /* 21 */
	{ 11, 0x023 },  /* 22 */
	{ 11, 0x022 },  /* 23 */
	{ 11, 0x021 },  /* 24 */
	{ 11, 0x020 },  /* 25 */
	{ 11, 0x01F },  /* 26 */
	{ 11, 0x01E },  /* 27 */
	{ 11, 0x01D },  /* 28 */
	{ 11, 0x01C },  /* 29 */
	{ 11, 0x01B },  /* 30 */
	{ 11, 0x01A },  /* 31 */
	{ 11, 0x019 },  /* 32 */
	{ 11, 0x018 },  /* 33 */
	{ 11, 0x008 }   /* escape */
};

static unsigned char nullPFrameHeader[] =
{
	0x47, 0x40, 0x00, 0x10, 0x00, 0x00, 0x01, 0xE0,
	0x00, 0x00, 0x84, 0xC0, 0x0A, 0x31, 0x00, 0x01,
	0x00, 0x01, 0x11, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x00, 0x01, 0x00, 0x01, 0xD7, 0xFF, 0xFB, 0x80,
	0x00, 0x00, 0x01, 0xB5, 0x83, 0x3F, 0xF3, 0x5D,
	0x80
};

#define FLUSH_SLICE_BITS()                                \
		      while ( bitcount > 8 )                              \
			  		        {                                                   \
         slice[i]= (((accum<<(32-bitcount))>>24)&0xFF);   \
         ++i;                                             \
         bitcount -= 8;                                   \
			  		        }
#define FLUSH_ALL_SLICE_BITS()                            \
		      while ( bitcount > 0 )                              \
			  		        {                                                   \
         slice[i]= (((accum<<(32-bitcount))>>24)&0xFF);   \
         ++i;                                             \
         bitcount -= 8;                                   \
			  		        }


/**
 * @brief Create a Null P frame
 * @param[in] width width of P frame to be constructed
 * @param[in] height height of P frame
 * @param[out] nullPFrameLen length of constructed p frame
 * @retval Buffer containing P frame
 */
unsigned char* TSProcessor::createNullPFrame(int width, int height, int *nullPFrameLen)
{
	unsigned char *nullPFrame = 0;
	int requiredLen = 0;
	int blockWidth, skipWidth, escapeCount;
	int skipCode, skipCodeNumBits;
	int sliceBitLen, sliceLen, sliceCount;
	int numTSPackets;
	unsigned char slice[16];
	int i, j, accum, bitcount;

	// Start of Video (19) + Picture (9) + Picture coding extension (9) minus TS packet header
	requiredLen = sizeof(nullPFrameHeader) - 4;

	blockWidth = (width + 15) / 16;
	skipWidth = blockWidth - 1;
	escapeCount = 0;
	while (skipWidth > 33)
	{
		escapeCount += 1;
		skipWidth -= 33;
	}
	skipCodeNumBits = macroblockAddressIncrementCodes[skipWidth - 1].numBits;
	skipCode = macroblockAddressIncrementCodes[skipWidth - 1].code;

	sliceBitLen = 32;  // slice start (0000 0000 0000 0000 0000 0001 NNNN NNNN)
	sliceBitLen += 5; // quantiser_scale_code (00001)
	sliceBitLen += 1; // extra_slice_bit (0)
	sliceBitLen += 1; // macroblock_address_inc (1)
	sliceBitLen += 3; // macroblock_type (001) [MC not coded] 
	sliceBitLen += 1; // motion_code[0][0][0] (1) [+0 Horz]
	sliceBitLen += 1; // motion_code[0][0][1] (1) [+0 Vert]
	sliceBitLen += (escapeCount * 11 + skipCodeNumBits);  // macroblock_address_inc for frame blockWidth-1
	sliceBitLen += 3; // macroblock_type (001) [MC not coded] 
	sliceBitLen += 1; // motion_code[0][0][0] (1) [+0 Horz]
	sliceBitLen += 1; // motion_code[0][0][1] (1) [+0 Vert]

	// Ensure there is at least one pad bit at end
	if ((sliceBitLen % 8) == 0) ++sliceBitLen;

	// Convert to bytes
	sliceLen = (sliceBitLen + 7) / 8;

	// Determine required number of slices for frame height
	sliceCount = (height + 15) / 16;

	// Calculate total required payload size
	requiredLen += sliceCount*sliceLen;

	// Calculate number of requried TS packets
	numTSPackets = 0;
	while (requiredLen > 0)
	{
		++numTSPackets;
		requiredLen += (4 + m_ttsSize);
		requiredLen -= m_packetSize;
	}

	// Build slice
	slice[0] = 0x00;
	slice[1] = 0x00;
	slice[2] = 0x01;
	slice[3] = 0x01;
	slice[4] = 0x0A;  //quantiser_scale_factor (0000 1) extra_slice_bit (0), 
	//macroblock_address_inc (1), first bit of macroblock_type (001)
	i = 5;
	accum = 0x07;     //last two bits of macroblock_type (001), 
	//motion_code[0][0][0] (1) [+0 Horz] 
	//motion_code[0][0][1] (1) [+0 Vert]
	bitcount = 4;
	for (j = 0; j < escapeCount; ++j)
	{
		accum <<= 11;
		accum |= 0x008; //escape: 000 0000 1000
		bitcount += 11;
		FLUSH_SLICE_BITS();
	}
	accum <<= skipCodeNumBits;
	accum |= skipCode;
	bitcount += skipCodeNumBits;
	FLUSH_SLICE_BITS();
	accum <<= 5;
	accum |= 0x07; //macroblock_type (001)
	//motion_code[0][0][0] (1) [+0 Horz]
	//motion_code[0][0][1] (1) [+0 Vert]
	bitcount += 5;
	FLUSH_SLICE_BITS();
	if (bitcount == 8)
	{
		// No zero pad bits yet, add one
		accum <<= 1;
		bitcount += 1;
	}
	FLUSH_ALL_SLICE_BITS();
	assert(i == sliceLen);

	i = 0;
	nullPFrame = (unsigned char *)malloc(numTSPackets*m_packetSize);
	if (nullPFrame)
	{
		if (m_ttsSize)
		{
			memset(&nullPFrame[i], 0, m_ttsSize);
			i += m_ttsSize;
		}
		rmf_osal_memcpy(&nullPFrame[i], nullPFrameHeader, sizeof(nullPFrameHeader), numTSPackets*m_packetSize - i, sizeof(nullPFrameHeader));
		nullPFrame[i + 1] = ((nullPFrame[i + 1] & 0xE0) | ((m_videoPid >> 8) & 0x1F));
		nullPFrame[i + 2] = (m_videoPid & 0xFF);
		i += sizeof(nullPFrameHeader);
		for (j = 1; j <= sliceCount; ++j)
		{
			slice[3] = (j & 0xFF);
			rmf_osal_memcpy(&nullPFrame[i], slice, sliceLen, numTSPackets*m_packetSize - i, 16);
			i += sliceLen;
			if ((i%m_packetSize) < sliceLen)
			{
				int excess = (i%m_packetSize);
				memmove(&nullPFrame[i - excess + m_ttsSize + 4], &nullPFrame[i - excess], excess);
				if (m_ttsSize)
				{
					memset(&nullPFrame[i - excess], 0, m_ttsSize);
					i += m_ttsSize;
				}
				nullPFrame[i - excess + 0] = 0x47;
				nullPFrame[i - excess + 1] = ((m_videoPid >> 8) & 0xFF);
				nullPFrame[i - excess + 2] = (m_videoPid & 0xFF);
				nullPFrame[i - excess + 3] = 0x10;
				i += 4;
			}
		}
		memset(&nullPFrame[i], 0xFF, m_packetSize - (i%m_packetSize));

#if 1
		for (i = 0; i < numTSPackets; ++i)
		{
			dumpPacket(&nullPFrame[i*m_packetSize], m_packetSize);
		}
#endif

		*nullPFrameLen = (numTSPackets*m_packetSize);
	}

	return nullPFrame;
}

// Parse through the sequence parameter set data to determine the frame size

/**
 * @brief process sequence parameter set and update state variables
 * @param[in] p pointer containing SPS
 * @param[in] length size of SPS
 * @retval true if SPS is processed successfully
 */
bool TSProcessor::processSeqParameterSet(unsigned char *p, int length)
{
	bool result = false;
	int profile_idc;
	int seq_parameter_set_id;
	int mask = 0x80;

	profile_idc = p[0];

	// constraint_set0_flag : u(1)
	// constraint_set1_flag : u(1)
	// constraint_set2_flag : u(1)
	// constraint_set3_flag : u(1)
	// constraint_set4_flag : u(1)
	// constraint_set5_flag : u(1)
	// reserved_zero_2bits : u(2)
	// level_idc : u(8)

	p += 3;

	seq_parameter_set_id = getUExpGolomb(p, mask);

	m_SPS[seq_parameter_set_id].separateColorPlaneFlag = 0;
	switch (profile_idc)
	{
	case 44:  case 83:  case 86:
	case 100: case 110: case 118:
	case 122: case 128: case 244:
	{
		int chroma_format_idx = getUExpGolomb(p, mask);
		if (chroma_format_idx == 3)
		{
			// separate_color_plane_flag
			m_SPS[seq_parameter_set_id].separateColorPlaneFlag = getBits(p, mask, 1);
		}
		// bit_depth_luma_minus8
		getUExpGolomb(p, mask);
		// bit_depth_chroma_minus8
		getUExpGolomb(p, mask);
		// qpprime_y_zero_transform_bypass_flag
		getBits(p, mask, 1);

		int seq_scaling_matrix_present_flag = getBits(p, mask, 1);
		if (seq_scaling_matrix_present_flag)
		{
			int imax = ((chroma_format_idx != 3) ? 8 : 12);
			for (int i = 0; i < imax; ++i)
			{
				int seq_scaling_list_present_flag = getBits(p, mask, 1);
				if (seq_scaling_list_present_flag)
				{
					if (i < 6)
					{
						processScalingList(p, mask, 16);
					}
					else
					{
						processScalingList(p, mask, 64);
					}
				}
			}
		}
	}
	break;
	}

	// log2_max_frame_num_minus4
	int log2_max_frame_num_minus4 = getUExpGolomb(p, mask);
	m_SPS[seq_parameter_set_id].log2MaxFrameNumMinus4 = log2_max_frame_num_minus4;

	int pic_order_cnt_type = getUExpGolomb(p, mask);
	m_SPS[seq_parameter_set_id].picOrderCountType = pic_order_cnt_type;
	if (pic_order_cnt_type == 0)
	{
		// log2_max_pic_order_cnt_lsb_minus4
		int log2_max_pic_order_cnt_lsb_minus4 = getUExpGolomb(p, mask);
		m_SPS[seq_parameter_set_id].log2MaxPicOrderCntLsbMinus4 = log2_max_pic_order_cnt_lsb_minus4;
		m_SPS[seq_parameter_set_id].maxPicOrderCount = (2 << (log2_max_pic_order_cnt_lsb_minus4 + 4));
	}
	else if (pic_order_cnt_type == 1)
	{
		// delta_pic_order_always_zero_flag
		getBits(p, mask, 1);
		// offset_for_non_ref_pic
		getSExpGolomb(p, mask);
		// offset_for_top_top_bottom_field
		getSExpGolomb(p, mask);

		int num_ref_frames_in_pic_order_cnt_cycle = getUExpGolomb(p, mask);
		for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; ++i)
		{
			// offset_for_ref_frame[i]
			getUExpGolomb(p, mask);
		}
	}

	// max_num_ref_frames   
	getUExpGolomb(p, mask);

	// gaps_in_frame_num_value_allowed_flag
	getBits(p, mask, 1);

	int pic_width_in_mbs_minus1 = getUExpGolomb(p, mask);

	int pic_height_in_map_units_minus1 = getUExpGolomb(p, mask);

	int frame_mbs_only_flag = getBits(p, mask, 1);
	m_SPS[seq_parameter_set_id].frameMBSOnlyFlag = frame_mbs_only_flag;

	m_frameWidth = (pic_width_in_mbs_minus1 + 1) * 16;

	m_frameHeight = (pic_height_in_map_units_minus1 + 1) * 16;

	m_isInterlaced = (frame_mbs_only_flag == 0);
	m_isInterlacedKnown = true;

	if (m_isInterlaced)
	{
		m_frameHeight *= 2;

		if (!frame_mbs_only_flag)
		{
			// mb_adaptive_frame_field_flag
			getBits(p, mask, 1);
		}

		if (m_playMode == PlayMode_retimestamp_Ionly)
		{
			// direct_8x8_inference_flag
			getBits(p, mask, 1);

			int frame_cropping_flag = getBits(p, mask, 1);
			if (frame_cropping_flag)
			{
				// frame_crop_left_offset
				getUExpGolomb(p, mask);
				// frame_crop_right_offset
				getUExpGolomb(p, mask);
				// frame_crop_top_offset
				getUExpGolomb(p, mask);
				// frame_crop_bottom_offset
				getUExpGolomb(p, mask);
			}

			int vui_parameters_present_flag = getBits(p, mask, 1);
			if (vui_parameters_present_flag)
			{
				int aspect_ratio_info_present_flag = getBits(p, mask, 1);
				if (aspect_ratio_info_present_flag)
				{
					int aspect_ratio_idc = getBits(p, mask, 8);
				}

				int overscan_info_present_flag = getBits(p, mask, 1);
				if (overscan_info_present_flag)
				{
					// overscan_appropriate_flag
					getBits(p, mask, 1);
				}

				int video_signal_type_present_flag = getBits(p, mask, 1);
				if (video_signal_type_present_flag)
				{
					// video_format
					getBits(p, mask, 3);
					// video_full_range_flag
					getBits(p, mask, 1);
					int color_description_present_flag = getBits(p, mask, 1);
					if (color_description_present_flag)
					{
						// color_primaries
						getBits(p, mask, 8);
						// transfer_characteristics
						getBits(p, mask, 8);
						// matrix_coefficients
						getBits(p, mask, 8);
					}
				}

				int chroma_info_present_flag = getBits(p, mask, 1);
				if (chroma_info_present_flag)
				{
					// chroma_sample_loc_type_top_field
					getUExpGolomb(p, mask);
					// chroma_sample_loc_type_bottom_field
					getUExpGolomb(p, mask);
				}

				int timing_info_present_flag = getBits(p, mask, 1);
				if (timing_info_present_flag)
				{
					unsigned char *timeScaleP;
					int timeScaleMask;

					unsigned int num_units_in_tick = getBits(p, mask, 32);

					timeScaleP = p;
					timeScaleMask = mask;
					unsigned int time_scale = getBits(p, mask, 32);

					unsigned int trick_time_scale = m_apparentFrameRate * 2 * 1000;
					DEBUG("put trick_time_scale=%d at %p mask %X\n", trick_time_scale, timeScaleP, timeScaleMask);
					putBits(timeScaleP, timeScaleMask, 32, trick_time_scale);

					result = true;
				}
			}
		}
	}

	TRACE2("TSProcessor: H.264 sequence frame size %dx%d interlaced=%d update SPS %d\n", m_frameWidth, m_frameHeight, m_isInterlaced, result);

	return result;
}

/**
 * @brief Parse through the picture parameter set to get required items
 * @param[in] p buffer containing PPS
 * @param[in] length size of PPS
 */
void TSProcessor::processPictureParameterSet(unsigned char *p, int length)
{
	int mask = 0x80;
	int pic_parameter_set_id;
	int seq_parameter_set_id;
	H264PPS *pPPS = 0;

	pic_parameter_set_id = getUExpGolomb(p, mask);
	seq_parameter_set_id = getUExpGolomb(p, mask);

	pPPS = &m_PPS[pic_parameter_set_id];
	pPPS->spsId = seq_parameter_set_id;
}

/**
 * @brief Consume all bits used by the scaling list
 * @param[in] p buffer containing scaling list
 * @param[in] mask mask
 * @param[in] size lenght of scaling list
 */
void TSProcessor::processScalingList(unsigned char *& p, int& mask, int size)
{
	int nextScale = 8;
	int lastScale = 8;
	for (int j = 0; j < size; ++j)
	{
		if (nextScale)
		{
			int deltaScale = getSExpGolomb(p, mask);
			nextScale = (lastScale + deltaScale + 256) % 256;
		}
		lastScale = (nextScale == 0) ? lastScale : nextScale;
	}
}


/**
 * @brief get bits based on mask and count
 * @param[in,out] p pointer being processed, updated internally
 * @param[in,out] mask mask to be applied
 * @param[in] bitCount Number of bits to be processed.
 * @retval value of bits
 */
unsigned int TSProcessor::getBits(unsigned char *& p, int& mask, int bitCount)
{
	int bits = 0;
	while (bitCount)
	{
		--bitCount;
		bits <<= 1;
		if (*p & mask)
		{
			bits |= 1;
		}
		mask >>= 1;
		if (mask == 0)
		{
			++p;
			mask = 0x80;
		}
	}
	return bits;
}


/**
 * @brief Put bits based on mask and count
 * @param[in,out] p reference of buffer to which bits to be put
 * @param[in,out] mask mask to be applied
 * @param[in] bitCount count of bits to be put
 * @param[in] value bits to be put
 */
void TSProcessor::putBits(unsigned char *& p, int& mask, int bitCount, unsigned int value)
{
	unsigned int putmask;

	putmask = (1 << (bitCount - 1));
	while (bitCount)
	{
		--bitCount;
		*p &= ~mask;
		if (value & putmask)
		{
			*p |= mask;
		}
		mask >>= 1;
		putmask >>= 1;
		if (mask == 0)
		{
			++p;
			mask = 0x80;
		}
	}
}


/**
 * @brief Gets unsigned EXP Golomb
 * @param[in,out] p buffer
 * @param[in,out] mask bitmask
 * @retval Unsigned EXP Golomb
 */
unsigned int TSProcessor::getUExpGolomb(unsigned char *& p, int& mask)
{
	int codeNum = 0;
	int leadingZeros = 0;
	int factor = 2;
	bool bitClear;
	do
	{
		bitClear = !(*p & mask);
		mask >>= 1;
		if (mask == 0)
		{
			++p;
			mask = 0x80;
		}
		if (bitClear)
		{
			++leadingZeros;
			codeNum += (factor >> 1);
			factor <<= 1;
		}
	} while (bitClear);
	if (leadingZeros)
	{
		codeNum += getBits(p, mask, leadingZeros);
	}
	return codeNum;
}


/**
 * @brief Getss signed EXP Golomb
 * @param[in,out] p buffer
 * @param[in,out] bit mask
 * @retval signed EXP Golomb
 */
int TSProcessor::getSExpGolomb(unsigned char *& p, int& bit)
{
	unsigned int u = getUExpGolomb(p, bit);
	int n = (u + 1) >> 1;
	if (!(u & 1))
	{
		n = -n;
	}
	return n;
}

/**
 * @brief Get audio components
 * @param[out] audioComponentsPtr pointer to audio component array
 * @param[out] count Number of audio components
 */
void TSProcessor::getAudioComponents(const RecordingComponent** audioComponentsPtr, int &count)
{
	count = audioComponentCount;
	*audioComponentsPtr = audioComponents;
}
