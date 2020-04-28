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

#ifndef FRAGMENTCOLLECTOR_MPD_H_
#define FRAGMENTCOLLECTOR_MPD_H_

#include "StreamAbstractionAAMP.h"
#include <string>
#include <stdint.h>
#include "libdash/IMPD.h"
#include "libdash/INode.h"
#include "libdash/IDASHManager.h"
#include "libdash/xml/Node.h"
#include "libdash/helpers/Time.h"
#include "libdash/xml/DOMParser.h"
#include <libxml/xmlreader.h>
using namespace dash;
using namespace std;
using namespace dash::mpd;
using namespace dash::xml;
using namespace dash::helpers;
#define MAX_MANIFEST_DOWNLOAD_RETRY_MPD 2

/*Common MPD util functions*/
uint64_t aamp_GetPeriodNewContentDuration(IPeriod * period, uint64_t &curEndNumber);
uint64_t aamp_GetPeriodDuration(dash::mpd::IMPD *mpd, int periodIndex, uint64_t mpdDownloadTime = 0);
Node* aamp_ProcessNode(xmlTextReaderPtr *reader, std::string url, bool isAd = false);
uint64_t aamp_GetDurationFromRepresentation(dash::mpd::IMPD *mpd);

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
	long GetMaxBitrate(void) override;
	void StopInjection(void) override;
	void StartInjection(void) override;
	double GetBufferedDuration();
	void SeekPosUpdate(double secondsRelativeToTuneTime) { };
	void NotifyFirstVideoPTS(unsigned long long pts) { };
	virtual void SetCDAIObject(CDAIObject *cdaiObj) override;

protected:
	StreamInfo* GetStreamInfo(int idx) override;
private:
	class PrivateStreamAbstractionMPD* mPriv;
};

#endif //FRAGMENTCOLLECTOR_MPD_H_
/**
 * @}
 */


