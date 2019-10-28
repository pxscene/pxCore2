/*
 *   Copyright 2018 RDK Management
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
*/

#include "ABRManager.h"
#include <cstdio>
#include <cstdarg>
#include <sys/time.h>
#include <cstring>

#if !(defined(WIN32) || defined(__APPLE__))
#if defined(USE_SYSTEMD_JOURNAL_PRINT)
#define ENABLE_RDK_LOGGER true
#include <systemd/sd-journal.h>
#elif defined(USE_SYSLOG_HELPER_PRINT)
#define ENABLE_RDK_LOGGER true
#include "syslog_helper_ifc.h"
#endif
#endif

// Loggers

/**
 * @brief Max log buffer size
 */
static const int MAX_LOG_BUFF_SIZE = 1024;

/**
 * @brief Log file directory index - To support dynamic directory configuration for abr logging 
 */
static char gsLogDirectory[] = "c:/tmp/aampabr.log";

/**
 * @brief Module name
 */
static const char *moduleName = "[ABRManager] ";

/**
 * @brief Module name string size
 */
static const int MODULE_NAME_SIZE = 13;

/**
 * @brief Empty logger (for disabling the logger)
 * @param fmt The format string
 * @param ... Variadic parameters
 * 
 * @return Number of printed characters 
 */
static int emptyLogger(const char* fmt, ...) {
  return 0;
}

/**
 * @brief Default logger
 * @param fmt The format string
 * @param ... Variadic parameters
 * 
 * @return Number of printed characters 
 */
static int defaultLogger(const char* fmt, ...) {
  int ret = 0;
  char logBuf[MAX_LOG_BUFF_SIZE] = {0};

  strcpy(logBuf, moduleName);

  va_list args;
  va_start(args, fmt);
  ret = vsnprintf(logBuf + MODULE_NAME_SIZE, (MAX_LOG_BUFF_SIZE - 1 - MODULE_NAME_SIZE), fmt, args);
  va_end(args);

#if defined(ENABLE_RDK_LOGGER)
#if defined(USE_SYSTEMD_JOURNAL_PRINT)
  sd_journal_print(LOG_NOTICE, "%s\n", logBuf);
#elif defined(USE_SYSLOG_HELPER_PRINT)
  send_logs_to_syslog(logBuf);
#endif
  return ret;
#else // ENABLE_RDK_LOGGER
#ifdef WIN32
  static bool init;
  FILE *f = fopen(gsLogDirectory, (init ? "a" : "w"));
  if (f)
  {
	init = true;
	fprintf(f, "%s", logBuf);
	fclose(f);
  }
  return printf("%s", logBuf);
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return printf("%ld:%3ld : %s\n", (long int)t.tv_sec, (long int)t.tv_usec / 1000, logBuf);
#endif
#endif
}

/**
 * Initialize the logger to printf
 */
ABRManager::LoggerFuncType ABRManager::sLogger = defaultLogger;

/**
 * @brief Constructor of ABRManager
 */
ABRManager::ABRManager() : 
  mDefaultInitBitrate(DEFAULT_BITRATE),
  mDesiredIframeProfile(0),
  mAbrProfileChangeUpCount(0),
  mAbrProfileChangeDownCount(0),
  mLowestIframeProfile(INVALID_PROFILE),
  mDefaultIframeBitrate(0) {

}

/**
 * @brief Initialize mSortedBWProfileList
 */
void ABRManager::initializeSortedBWProfileList() {
  if (!mSortedBWProfileList.size()) {
    int profileCount = getProfileCount();
    for (int cnt = 0; cnt < profileCount; cnt++) {
      // store all the bandwidth data and its index to map which will sort by itself
      if (!mProfiles[cnt].isIframeTrack) {
        mSortedBWProfileList[mProfiles[cnt].bandwidthBitsPerSecond] = cnt;
      }
    }
  }
}

/**
 * @brief Get initial profile index, choose the medium profile or
 * the profile whose bitrate >= the default bitrate.
 * 
 * @param chooseMediumProfile Boolean flag, true means
 * to choose the medium profile, otherwise to choose the profile whose
 * bitrate >= the default bitrate.
 * 
 * @return The initial profile index 
 */
int ABRManager::getInitialProfileIndex(bool chooseMediumProfile) {
  // This function will be called only once during session creation to get default profile
  // check if any profiles are added (incase), remove it before adding fresh
  // populate the container with sorted order of BW vs its index
  if (mSortedBWProfileList.size()) {
    mSortedBWProfileList.erase(mSortedBWProfileList.begin(),mSortedBWProfileList.end());
  }	
  initializeSortedBWProfileList();
  int profileCount = getProfileCount();
  int desiredProfileIndex = INVALID_PROFILE;
  if (chooseMediumProfile && profileCount > 1) {
    // get the mid profile from the sorted list
    SortedBWProfileListIter iter = mSortedBWProfileList.begin();
    std::advance(iter, static_cast<int>(mSortedBWProfileList.size() / 2));
    desiredProfileIndex = iter->second;
  } else {
    SortedBWProfileListIter iter;
    desiredProfileIndex = mSortedBWProfileList.begin()->second;
    for (iter = mSortedBWProfileList.begin(); iter != mSortedBWProfileList.end(); ++iter) {
      if (iter->first >= mDefaultInitBitrate) {
        break;
      }
      // Choose the profile whose bitrate < default bitrate
      desiredProfileIndex = iter->second;
    }
  }
  if (INVALID_PROFILE == desiredProfileIndex) {
    desiredProfileIndex = mSortedBWProfileList.begin()->second;
    sLogger("%s:%d Got invalid profile index, choose the first index = %d and profileCount = %d and defaultBitrate = %ld\n",
      __FUNCTION__, __LINE__, desiredProfileIndex, profileCount, mDefaultInitBitrate);
  } else {
    sLogger("%s:%d Get initial profile index = %d, bitrate = %ld and defaultBitrate = %ld\n",
      __FUNCTION__, __LINE__, desiredProfileIndex, mProfiles[desiredProfileIndex].bandwidthBitsPerSecond, mDefaultInitBitrate);
  }
  return desiredProfileIndex;
}

/**
 * @brief Update the lowest / desired profile index
 *    by the profile info. 
 */
void ABRManager::updateProfile() {
  /**
   * @brief A temporary structure of iframe track info
   */
  struct IframeTrackInfo {
    long bandwidth;
    int idx;
  };

  int profileCount = getProfileCount();
  
  struct IframeTrackInfo *iframeTrackInfo = new struct IframeTrackInfo[profileCount];
  bool is4K = false;

  int iframeTrackIdx = -1;
  // Construct iframe track info
  for (int i = 0; i < profileCount; i++) {
    if (mProfiles[i].isIframeTrack) {
      iframeTrackIdx++;
      iframeTrackInfo[iframeTrackIdx].bandwidth = mProfiles[i].bandwidthBitsPerSecond;
      iframeTrackInfo[iframeTrackIdx].idx = i;
    }
  }

  // Exists iframe track
  if(iframeTrackIdx >= 0) {
    // Sort the iframe track array by bandwidth ascendingly
    for (int i = 0; i < iframeTrackIdx; i++) {
      for (int j = 0; j < iframeTrackIdx - i; j++) {
        if (iframeTrackInfo[j].bandwidth > iframeTrackInfo[j+1].bandwidth) {
          struct IframeTrackInfo temp = iframeTrackInfo[j];
          iframeTrackInfo[j] = iframeTrackInfo[j+1];
          iframeTrackInfo[j+1] = temp;
        }
      }
    }

    // Exist 4K video?
    int highestProfileIdx = iframeTrackInfo[iframeTrackIdx].idx;
    if(mProfiles[highestProfileIdx].height > HEIGHT_4K
      || mProfiles[highestProfileIdx].width > WIDTH_4K) {
      is4K = true;
    }

    if (mDefaultIframeBitrate > 0) {
      mLowestIframeProfile = mDesiredIframeProfile = iframeTrackInfo[0].idx;
      for (int cnt = 0; cnt <= iframeTrackIdx; cnt++) {
        // find the track less than default bw set, apply to both desired and lower ( for all speed of trick)
        if(iframeTrackInfo[cnt].bandwidth >= mDefaultIframeBitrate) {
          break;
        }
        mDesiredIframeProfile = iframeTrackInfo[cnt].idx;
      }
    } else {
      if(is4K) {
        // Get the default profile of 4k video , apply same bandwidth of video to iframe also
        int desiredProfileIndexNonIframe = getProfileCount() / 2;
        int desiredProfileNonIframeBW = mProfiles[desiredProfileIndexNonIframe].bandwidthBitsPerSecond ;
        mDesiredIframeProfile = mLowestIframeProfile = 0;
        for (int cnt = 0; cnt <= iframeTrackIdx; cnt++) {
          // if bandwidth matches , apply to both desired and lower ( for all speed of trick)
          if(iframeTrackInfo[cnt].bandwidth == desiredProfileNonIframeBW) {
            mDesiredIframeProfile = mLowestIframeProfile = iframeTrackInfo[cnt].idx;
            break;
          }
        }
        // if matching bandwidth not found with video , then pick the middle profile for iframe
        if((!mDesiredIframeProfile) && (iframeTrackIdx >= 1)) {
          int desiredTrackIdx = (int) (iframeTrackIdx / 2) + (iframeTrackIdx % 2);
          mDesiredIframeProfile = mLowestIframeProfile = iframeTrackInfo[desiredTrackIdx].idx;
        }
      } else {
        //Keeping old logic for non 4K streams
        for (int cnt = 0; cnt <= iframeTrackIdx; cnt++) {
            if (mLowestIframeProfile == INVALID_PROFILE) {
              // first pick the lowest profile available
              mLowestIframeProfile = mDesiredIframeProfile = iframeTrackInfo[cnt].idx;
              continue;
            }
            // if more profiles available , stored second best to desired profile
            mDesiredIframeProfile = iframeTrackInfo[cnt].idx;
            break; // select first-advertised
        }
      }
    }
  }
  delete[] iframeTrackInfo;

#if defined(DEBUG_ENABLED)
  sLogger("%s:%d Update profile info, mDesiredIframeProfile = %d, mLowestIframeProfile = %d\n",
    __FUNCTION__, __LINE__, mDesiredIframeProfile, mLowestIframeProfile);
#endif
}

/**
 * @brief According to the given bandwidth, return the best matched
 * profile index.
 * @param bandWidth  The given bandwidth
 * @return the best matched profile index
 */
int ABRManager::getBestMatchedProfileIndexByBandWidth(int bandwidth) {
  // a) Check if network bandwidth changed from starting bandwidth
  // b) Check if netwwork bandwidth is different from persisted bandwidth( needed for first time reporting)
  // find the profile for the newbandwidth
  int desiredProfileIndex = 0;
  int profileCount = getProfileCount();
  for (int i = 0; i < profileCount; i++) {
    const ProfileInfo& profile = mProfiles[i];
    if (!profile.isIframeTrack) {
        if (profile.bandwidthBitsPerSecond == bandwidth) {
            // Good case ,most manifest url will have same bandwidth in fragment file with configured profile bandwidth
            desiredProfileIndex = i;
            break;
        } else if (profile.bandwidthBitsPerSecond < bandwidth) {
            // fragment file name bandwidth doesnt match the profile bandwidth, will be always less
            if((i+1) == profileCount) {
                desiredProfileIndex = i;
                break;
            }
            else
                desiredProfileIndex = (i + 1);
        }
    }
  }
#if defined(DEBUG_ENABLED)
  sLogger("%s:%d Get best matched profile index = %d bitrate = %ld\n",
    __FUNCTION__, __LINE__, desiredProfileIndex,
    (profileCount > desiredProfileIndex && desiredProfileIndex != INVALID_PROFILE) ? mProfiles[desiredProfileIndex].bandwidthBitsPerSecond : 0);
#endif
  return desiredProfileIndex;
}

/**
 * @brief Ramp down the profile one step to get the profile index of a lower bitrate.
 *
 * @param currentProfileIndex The current profile index
 * @return the profile index of a lower bitrate (one step)
 */
int ABRManager::getRampedDownProfileIndex(int currentProfileIndex) {
  // Clamp the param to avoid overflow
  int profileCount = getProfileCount();
  if (currentProfileIndex >= profileCount) {
    sLogger("%s:%d Invalid currentProfileIndex %d exceeds the current profile count %d\n",
      __FUNCTION__, __LINE__, currentProfileIndex, profileCount);
    currentProfileIndex = profileCount - 1;
  }
  
  int desiredProfileIndex = currentProfileIndex;
  if (profileCount == 0) {
    sLogger("%s:%d No profiles found\n",
       __FUNCTION__, __LINE__);
    return desiredProfileIndex;
  }
  long currentBandwidth = mProfiles[currentProfileIndex].bandwidthBitsPerSecond;
  SortedBWProfileListIter iter = mSortedBWProfileList.find(currentBandwidth);
  if (iter == mSortedBWProfileList.end()) {
    sLogger("%s:%d The current bitrate %ld is not in the profile list\n",
       __FUNCTION__, __LINE__, currentBandwidth);
    return desiredProfileIndex;
  }
  if (iter == mSortedBWProfileList.begin()) {
    desiredProfileIndex = iter->second;
  } else {
    // get the prev profile . This is sorted list , so no worry of getting wrong profile 
    std::advance(iter, -1);
    desiredProfileIndex = iter->second;
  }

#if defined(DEBUG_ENABLED)
  sLogger("%s:%d Ramped down profile index = %d bitrate = %ld\n",
    __FUNCTION__, __LINE__, desiredProfileIndex, mProfiles[desiredProfileIndex].bandwidthBitsPerSecond);
#endif
  return desiredProfileIndex;
}

/**
 * @brief Check if the bitrate of currentProfileIndex reaches to the lowest.
 *
 * @param currentProfileIndex The current profile index
 * @return True means it reaches to the lowest, otherwise, it doesn't.
 */
bool ABRManager::isProfileIndexBitrateLowest(int currentProfileIndex) {
  // Clamp the param to avoid overflow
  int profileCount = getProfileCount();
  if (currentProfileIndex >= profileCount) {
    sLogger("%s:%d Invalid currentProfileIndex %d exceeds the current profile count %d\n",
      __FUNCTION__, __LINE__, currentProfileIndex, profileCount);
    currentProfileIndex = profileCount - 1;
  }
  
  // If there is no profiles list, then it means `currentProfileIndex` always reaches to
  // the lowest.
  if (profileCount == 0) {
    sLogger("%s:%d No profiles found\n",
       __FUNCTION__, __LINE__);
    return true;
  }

  long currentBandwidth = mProfiles[currentProfileIndex].bandwidthBitsPerSecond;
  SortedBWProfileListIter iter = mSortedBWProfileList.find(currentBandwidth);
  return iter == mSortedBWProfileList.begin();
}

/**
 * @brief Do ABR by ramping bitrate up/down according to the current
 * network status. Returns the profile index with the bitrate matched with
 * the current bitrate.
 * 
 * @param currentProfileIndex The current profile index
 * @param currentBandwidth The current band width
 * @param networkBandwidth The current available bandwidth (network bandwidth)
 * @param nwConsistencyCnt Network consistency count, used for bitrate ramping up/down
 * @return int Profile index
 */
int ABRManager::getProfileIndexByBitrateRampUpOrDown(int currentProfileIndex, long currentBandwidth, long networkBandwidth, int nwConsistencyCnt) {
  // Clamp the param to avoid overflow
  int profileCount = getProfileCount();
  if (currentProfileIndex >= profileCount) {
    sLogger("%s:%d Invalid currentProfileIndex %d exceeds the current profile count %d\n",
      __FUNCTION__, __LINE__, currentProfileIndex, profileCount);
    currentProfileIndex = profileCount - 1;
  }
  int desiredProfileIndex = currentProfileIndex;
  if (networkBandwidth == -1) {
    // If the network bandwidth is not available, just reset the profile change up/down count.
#if defined(DEBUG_ENABLED)
    sLogger("%s:%d No network bandwidth info available , not changing profile[%d]\n",
      __FUNCTION__, __LINE__, currentProfileIndex);
#endif
    mAbrProfileChangeUpCount = 0;
    mAbrProfileChangeDownCount = 0;
    return desiredProfileIndex;
  }
  // Ensure the mSortedBWProfileList is populated
  initializeSortedBWProfileList();
  // regular scenario where cache is not empty
  if(networkBandwidth > currentBandwidth) {
    // if networkBandwidth > is more than current bandwidth
    SortedBWProfileListIter iter;
    SortedBWProfileListIter currIter = mSortedBWProfileList.find(currentBandwidth);
    SortedBWProfileListIter storedIter = mSortedBWProfileList.end();
    for (iter = currIter; iter != mSortedBWProfileList.end(); ++iter) {          
      // This is sort List 
      if (networkBandwidth >= iter->first) {
        desiredProfileIndex = iter->second;
        storedIter = iter;
      } else {
        break;
      }
    }

    // No need to jump one profile for one network bw increase
    if (storedIter != mSortedBWProfileList.end() && (currIter->first < storedIter->first) && std::distance(currIter, storedIter) == 1) {
      mAbrProfileChangeUpCount++;
      // if same profile holds good for next 3*2 fragments
      if (mAbrProfileChangeUpCount < nwConsistencyCnt) {
        desiredProfileIndex = currentProfileIndex;
      } else {
        mAbrProfileChangeUpCount = 0;
      }
    } else {
      mAbrProfileChangeUpCount = 0;
    }
    mAbrProfileChangeDownCount = 0;
#if defined(DEBUG_ENABLED)
    sLogger("%s:%d Ramp up profile index = %d, bitrate = %ld networkBandwidth = %ld\n",
      __FUNCTION__, __LINE__, desiredProfileIndex,
        (profileCount > desiredProfileIndex && desiredProfileIndex != INVALID_PROFILE) ? mProfiles[desiredProfileIndex].bandwidthBitsPerSecond : 0, networkBandwidth);
#endif
  } else {
    // if networkBandwidth < than current bandwidth
    SortedBWProfileListRevIter reviter;
    SortedBWProfileListIter currIter = mSortedBWProfileList.find(currentBandwidth);
    SortedBWProfileListIter storedIter = mSortedBWProfileList.end();
    for (reviter = mSortedBWProfileList.rbegin(); reviter != mSortedBWProfileList.rend(); ++reviter) {
      // This is sorted List
      if (networkBandwidth >= reviter->first) {
        desiredProfileIndex = reviter->second;
        // convert from reverse iter to forward iter
        storedIter = reviter.base();
        storedIter--;
        break;
      }
    }

    // we didn't find a profile which can be supported in this bandwidth
    if (reviter == mSortedBWProfileList.rend()) {
	desiredProfileIndex = mSortedBWProfileList.begin()->second;
        sLogger("%s:%d Didn't find a profile which supports bandwidth[%ld], min bandwidth available [%ld]. Set profile to lowest!\n", __FUNCTION__, __LINE__, networkBandwidth, mSortedBWProfileList.begin()->first);
    }

    // No need to jump one profile for small  network change
    if (storedIter != mSortedBWProfileList.end() && (currIter->first > storedIter->first) && std::distance(storedIter, currIter) == 1) {
      mAbrProfileChangeDownCount++;
      // if same profile holds good for next 3*2 fragments
      if(mAbrProfileChangeDownCount < nwConsistencyCnt) {
        desiredProfileIndex = currentProfileIndex;
      } else {
        mAbrProfileChangeDownCount = 0;
      }
    } else {
      mAbrProfileChangeDownCount = 0;
    }
    mAbrProfileChangeUpCount = 0;
#if defined(DEBUG_ENABLED)
    sLogger("%s:%d Ramp down profile index = %d, bitrate = %ld networkBandwidth = %ld\n",
      __FUNCTION__, __LINE__, desiredProfileIndex,
      (profileCount > desiredProfileIndex && desiredProfileIndex != INVALID_PROFILE) ? mProfiles[desiredProfileIndex].bandwidthBitsPerSecond : 0, networkBandwidth);
#endif
  }

  if (currentProfileIndex != desiredProfileIndex) {
    sLogger("%s:%d Current bandwidth[%ld] Network bandwidth[%ld] Current profile index[%d] Desired profile index[%d]\n",
      __FUNCTION__, __LINE__, currentBandwidth, networkBandwidth,
      currentProfileIndex, desiredProfileIndex);
  }

  return desiredProfileIndex;
}

/**
 * @brief Get bandwidth of profile
 * 
 * @param profileIndex The profile index
 * @return long bandwidth of the profile
 */
long ABRManager::getBandwidthOfProfile(int profileIndex) {
  // Clamp the param to avoid overflow
  int profileCount = getProfileCount();
  if (profileCount == 0) {
    sLogger("%s:%d No profiles\n",
      __FUNCTION__, __LINE__);
    return 0;
  }
  if (profileIndex >= profileCount) {
    sLogger("%s:%d Invalid currentProfileIndex %d exceeds the current profile count %d\n",
      __FUNCTION__, __LINE__, profileIndex, profileCount);
    profileIndex = profileCount - 1;
  }

  return mProfiles[profileIndex].bandwidthBitsPerSecond;
}

/**
 * @brief Get the index of max bandwidth
 * 
 * @return int index of the max bandwidth
 */
int ABRManager::getMaxBandwidthProfile()
{
  int profileCount = getProfileCount();
  if (profileCount == 0) {
    sLogger("%s:%d No profiles\n",
      __FUNCTION__, __LINE__);
    return 0;
  }
  // Ensure the mSortedBWProfileList is populated
  initializeSortedBWProfileList();

  return mSortedBWProfileList.size()?mSortedBWProfileList.rbegin()->second:0;
}

// Getters/Setters
/**
 * @brief Get the number of profiles
 * 
 * @return The number of profiles 
 */
int ABRManager::getProfileCount() const {
  return static_cast<int>(mProfiles.size());
}

/**
 * @brief Set the default init bitrate
 * 
 * @param defaultInitBitrate Default init bitrate
 */
void ABRManager::setDefaultInitBitrate(long defaultInitBitrate) {
  mDefaultInitBitrate = defaultInitBitrate;
}

/**
 * @brief Get the lowest iframe profile index.
 * 
 * @return the lowest iframe profile index. 
 */
int ABRManager::getLowestIframeProfile() const {
  return mLowestIframeProfile;
}

/**
 * @brief Get the desired iframe profile index.
 * 
 * @return the desired iframe profile index. 
 */
int ABRManager::getDesiredIframeProfile() const {
  return mDesiredIframeProfile;
}

/**
 * @brief Add new profile info into the manager
 * @param profile The profile info
 */
void ABRManager::addProfile(ABRManager::ProfileInfo profile) {
  mProfiles.push_back(profile);
}

/**
 * @brief Clear profiles
 */
void ABRManager::clearProfiles() {
  mProfiles.clear();
  if (mSortedBWProfileList.size()) {
    mSortedBWProfileList.erase(mSortedBWProfileList.begin(),mSortedBWProfileList.end());
  }	
}

/**
 * @brief Set logger function
 * 
 * The logger function must be in the signature int (const char*, ...)
 * 
 * @param logger The logger function
 */
void ABRManager::setLogger(LoggerFuncType logger) {
  sLogger = logger;
}

/**
 * @brief Disable logger, then no logger output.
 */
void ABRManager::disableLogger() {
  // Set the empty logger
  sLogger = emptyLogger;
}

/**
 * @brief Set the simulator log file directory index.
 */
void ABRManager::setLogDirectory(char driveName) {
  gsLogDirectory[0] = driveName;
}

/**
 * @brief Set the default iframe bitrate
 * 
 * @param defaultIframeBitrate Default iframe bitrate
 */
void ABRManager::setDefaultIframeBitrate(long defaultIframeBitrate)
{
  mDefaultIframeBitrate = defaultIframeBitrate;
}
