# Introduction

ABR (Automatic BitRate) Library is an independent library for automatic bitrate switching. ABR will start at a reasonable bitrate and will ramp up or down, depending on network conditions.  Only one bitrate will be recorded at any one time, but to the player, it will look like a single, consistent video playback.

Here is a brief introduction to the workflow of ABR library:

(1) ABR library requires a list of profiles (tracks) as input, each profile has a bandwidth.

(2) ABR library will choose a profile whose bandwidth best matches with the current network bandwidth. That is, the library will output the profile index with the desired bitrate base on the network condition.

A HLS/DASH stream may contain several profiles in different bandwidths, for adapting to different network conditions.

The ABR library supports both HLS and DASH.

# Library design principle

The input of the ABR library includes

- A list of profile (track) info
- The currently available network bandwidth
- The currently chosen profile index.

The output of the ABR library is
- The chosen (desired) profile index.

ABR library is encapsulated as a class `ABRManager`. The `ABRManager` provides the following functions

## Constructor

It provides a void-parameter constructor `ABRManager::ABRManager()`, so it's very easy to initialize it.

## Input

`struct ABRManager::ProfileInfo` is the data structure of the required profile info, it contains the following fields.

- `isIframeTrack`. A flag to denote if it is the iframe track.
- `bandwidthBitsPerSecond`. Bandwidth per second, i.e, bitrate.
- `width`. The width of resolution
- `height`. The height of resolution.

ABR library provides the following function to add profile info into the manager

- `void ABRManager::addProfile(ABRManager::ProfileInfo profile)`

  This method is used to add a profile into the manager.

## Output

ABR library provides several functions for chosing profiles in different ways, so it provides flexible way for user to use in different scenarios. 

- `int ABRManager::getInitialProfileIndex(bool chooseMediumProfile)`

  Get a reasonable bitrate at startup - get initial profile index, choose the medium profile or the profile whose bitrate >= the default bitrate.

- `int ABRManager::getBestMatchedProfileIndexByBandWidth(int bandwidth)`

  According to the given bandwidth, return the best matched profile index.

- `int ABRManager::getDesiredIframeProfile()`

  Get the current desired iframe profile index.

- `int ABRManager::getLowestIframeProfile()`

  Get the current available lowest iframe profile index.

- `int ABRManager::getRampedDownProfileIndex(int currentProfileIndex)`

  Ramps down the profile one step to get the profile index of a lower bitrate.

- `int ABRManager::getProfileIndexByBitrateRampUpOrDown(int currentProfileIndex, long currentBandwidth, long networkBandwidth)`

  According to the current bandwidth, current avaialbe network bandwidth and current chosen profile index, do ABR by ramping bitrate up/down. Returns the profile index with the bitrate matched with the current bitrate.

## Update

ABR library provides the following functions to update the internal state of the manager.

- `void ABRManager::updateProfile()`

  If the profiles are changed, ABR library provides this function to update the profiles. Concretely, it will update the lowest / desired profile index according to the profile info, the lowest / desired profile index will used in the output functions.

- `void ABRManager::clearProfiles()`

  Remove all profiles.

## Auxiliary functions

ABR library provides the following auxiliary functions to make the library easier to use.
- `int ABRManager::getProfileCount()`

  Get the number of profiles.

- `long ABRManager::getBandwidthOfProfile(int profileIndex)`

  Get the bandwidth of a profile

- `void ABRManager::setDefaultInitBitrate(long defaultInitBitrate)`

  Change the default initialize bitrate for `ABRManager::getInitialProfileIndex`, if you don't change it, the default value is 1000000

- `bool isProfileIndexBitrateLowest(int currentProfileIndex)`
  Check if the bitrate of currentProfileIndex reaches to the lowest.

- `static void ABRManager::setLogger(LoggerFuncType logger)`
  Customize the logger function for this library. The logger function must be in the signature `int (const char*, ...)`

- `static void disableLogger()`

  Disable the logger function for this library.

# Detailed Documentation

For the detailed documentation for each member function, please see

[ABRManager](html/class_a_b_r_manager.html)

[ABRManager::ProfileInfo](html/struct_a_b_r_manager_1_1_profile_info.html)

# Build steps

## Prerequisite

- cmake 2.6+ https://cmake.org/download/
- g++ 5.4+ https://gcc.gnu.org/

## Build

Assume `abr/` is the root directory of this library. The build steps are

```sh
cd abr/
mkdir build
cd build
cmake ..
make
sudo make install
```

Then a static library `libabr.a` will be generated in `abr/build`, and it will be installed
```
-- Installing: /usr/local/lib/libabr.a
-- Installing: /usr/local/include/abr/ABRManager.h
```

# Sample Usage

```cpp
#include <abr/ABRManager.h>

// Construct the manager object
ABRManager abrManager; 

// Add profiles
abrManager.addProfile({
  .isIframeTrack = ...,
  .bandwidthBitsPerSecond = ...,
  .width = ...,
  .height = ...,
});
...
abrManager.addProfile({
  .isIframeTrack = true,
  .bandwidthBitsPerSecond = 36000,
  .width = 1024,
  .height = 768,
});

// After adding all profiles, update it
abrManager.updateProfile();

// Get the initial profile file index (starts at a reasonable bitrate)
currentProfileIndex = abrManager.getInitialProfileIndex(true);

// Then choose one of the following function to switch the bitrate (get the profile index).
// abrManager.getBestMatchedProfileIndexByBandWidth
// abrManager.getDesiredIframeProfile
// abrManager.getLowestIframeProfile
// abrManager.getRampedDownProfileIndex
// abrManager.getProfileIndexByBitrateRampUpOrDown
```