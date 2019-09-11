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
 * @file fragmentcollector_mpd.h
 * @brief Fragment collector MPEG DASH declarations
 */

#include "StreamAbstractionAAMP.h"
#include <string>
#include <stdint.h>

/**
 * @class StreamAbstractionAAMP_MPD
 * @brief Fragment collector for MPEG DASH
 */
class StreamAbstractionAAMP_MPD : public StreamAbstractionAAMP
{
public:
	StreamAbstractionAAMP_MPD(class PrivateInstanceAAMP *aamp,double seekpos, float rate);
	~StreamAbstractionAAMP_MPD();
	StreamAbstractionAAMP_MPD(const StreamAbstractionAAMP_MPD&) = delete;
	StreamAbstractionAAMP_MPD& operator=(const StreamAbstractionAAMP_MPD&) = delete;
	void DumpProfiles(void) override;
	void Start() override;
	void Stop(bool clearChannelData) override;
	AAMPStatusType Init(TuneType tuneType) override;
	void GetStreamFormat(StreamOutputFormat &primaryOutputFormat, StreamOutputFormat &audioOutputFormat) override;
	double GetStreamPosition() override;
	MediaTrack* GetMediaTrack(TrackType type) override;
	double GetFirstPTS() override;
	int GetBWIndex(long bitrate) override;
	std::vector<long> GetVideoBitrates(void) override;
	std::vector<long> GetAudioBitrates(void) override;
	void StopInjection(void) override;
	void StartInjection(void) override;
	virtual void SetCDAIObject(CDAIObject *cdaiObj) override;

protected:
	StreamInfo* GetStreamInfo(int idx) override;
private:
	class PrivateStreamAbstractionMPD* mPriv;
};

/**
 * @class CDAIObjectMPD
 * @brief Client Side DAI object implementation for DASH
 */
class CDAIObjectMPD: public CDAIObject
{
	class PrivateCDAIObjectMPD* mPrivObj;
public:
	CDAIObjectMPD(PrivateInstanceAAMP* aamp);
	virtual ~CDAIObjectMPD();
	CDAIObjectMPD(const CDAIObjectMPD&) = delete;
	CDAIObjectMPD& operator= (const CDAIObjectMPD&) = delete;

	PrivateCDAIObjectMPD* GetPrivateCDAIObjectMPD()
	{
		return mPrivObj;
	}

	virtual void SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url, uint64_t startMS=0) override;
};

/**
 * @}
 */


