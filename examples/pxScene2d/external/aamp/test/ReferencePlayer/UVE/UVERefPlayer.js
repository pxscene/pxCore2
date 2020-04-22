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

var playbackSpeeds = [-64, -32, -16, -4, 1, 4, 16, 32, 64];

class VTTCue {
    constructor(start, duration, text, line = 0, align = "", position = 0, size = 0) {
        this.start = start;       //: number,
        this.duration = duration; //: number,
        this.text = text;         //: string,
        this.line = line;         //: number,
        this.align = align;       //: string,
        this.size = size;         //: number,
        this.position = position  //: number
	}
};

let subtitleTimer = undefined;
let displayTimer = undefined;
let vttCueBuffer = [];

//Comcast DRM config for AAMP
var DrmConfig = {'com.microsoft.playready':'mds.ccp.xcal.tv', 'com.widevine.alpha':'mds.ccp.xcal.tv', 'preferredKeysystem':'com.widevine.alpha'};

//DRM config for Sintel asset
var SintelDrmConfig = {'com.microsoft.playready':'https://amssamples.keydelivery.mediaservices.windows.net/PlayReady/', 'com.widevine.alpha':'https://amssamples.keydelivery.mediaservices.windows.net/Widevine/?KID=f9dbca11-a1e2-45c8-891f-fb71063cbfdb', 'preferredKeysystem':'com.microsoft.playready'};
//AAMP initConfig is used to pass certain predefined config params to AAMP
//Commented out values are not implemented for now
//Values assigned are default values of each config param
//All properties are optional.
var defaultInitConfig = {
    /**
     * max initial bitrate (bps)
     */
    initialBitrate: 2500000,

    /**
     * max initial bitrate for 4K assets (bps)
     */
    //initialBitrate4K: number,

    /**
     * min amount of buffer needed before playback (seconds)
     */
    //initialBuffer: number;

    /**
     * max amount of buffer during playback (seconds)
     */
    //playbackBuffer: number;

    /**
     * start position for playback (seconds)
     */
    offset: 0,

    /**
     * network request timeout (seconds)
     */
    networkTimeout: 10,

    /**
     * manifest request timeout (seconds)
     */
    //manifestTimeout: 0.5,
    /**
     * manifest request timeout (seconds)
     */
    playlistTimeout: 10,

    /**
     * max number of fragments to keep as playback buffer (number)
     * e.x:
     *   with a downloadBuffer of 3, there will be 3 fragments of
     *   video or audio cached as buffers during playback.
     *   If each fragment is 2 second long, total download buffer
     *   size of this playback is 3 * 2 = 6 secs
     */
    //downloadBuffer: number;

    /**
     * min amount of bitrate (bps)
     */
    //minBitrate: number;

    /**
     * max amount of bitrate (bps)
     */
    //maxBitrate: number;

    /**
     * preferred audio language
     */
    preferredAudioLanguage: "en",

    /**
     * TSB length in seconds, value of 0 means it is disabled
     */
    //timeShiftBufferLength: number;

    /**
     * offset from live point for live assets (seconds)
     */
    liveOffset: 15,

    /**
     * preferred subtitle language
     */
    preferredSubtitleLanguage: "en",

    /**
     *  network proxy to use (Format <SCHEME>://<PROXY IP:PROXY PORT>)
     */
    //networkProxy: string;

    /**
     *  network proxy to use for license requests (Format same as network proxy)
     */
    //licenseProxy: string;

    /**
     *  time to deem a download with partial bytes as stalled prematurely. (seconds)
     *  optimization to avoid long waits when networkTimeout configured is a high value
     */
    //downloadStallTimeout: number;

    /**
     *  time to deem a download with no bytes received as stalled. (seconds)
     *  optimization to avoid long waits when networkTimeout configured is a high value
     */
    //downloadStartTimeout: number;

    /**
     * drmConfig for the playback
     */
    drmConfig: DrmConfig, //For sample structure DrmConfig

    /**
     * stereo-only for the playback
     */
    //stereoOnly: true,
    /**
     * bulk TimedMetadata reporting
     */
    //bulkTimedMetadata: true,

    /**
     * enable audio video playlist parallel download optimization (only for HLS)
     */
    //parallelPlaylistDownload: true,
	/**
	 * disable audio video playlist parallel download for linear (only for HLS)
	 */
	//parallelPlaylistRefresh: false,

    /**
     * enable async tune method for player
     */
    //asyncTune: true,

    /**
     * use westeros sink based video decoding
     */
    //useWesterosSink: true,

    /**
     * use Average bandwidth for ABR switching
     */
    //useAverageBandwidth: true,

    /**
     * Enable PreCaching of Playlist and TimeWindow for Cache(minutes)
     */
    //preCachePlaylistTime: 5

    /**
     * Progress Report Interval (in seconds)
     */
    //progressReportingInterval: 2,

    /**
     * enable unpaired discontinuity retune config
     */
    //useRetuneForUnpairedDiscontinuity: true,

    /**
     * max attempts for init frag curl timeout failures
     */
    //initFragmentRetryCount: 3
};

var playerState = playerStatesEnum.idle;
var playbackRateIndex = playbackSpeeds.indexOf(1);
var urlIndex = 0;
var mutedStatus = false;
var playerObj = null;
var bgPlayerObj = null;

window.onload = function() {
    initPlayerControls();
    //resetPlayer();
    resetUIOnNewAsset();

    //loadUrl(urls[urlIndex]);
}

function resetSubtitles(emptyBuffers) {
    console.log("Inside resetSubtitles");
    if (displayTimer !== undefined) {
        clearTimeout(displayTimer);
        displayTimer = undefined;
    }
    if (subtitleTimer !== undefined) {
        clearTimeout(subtitleTimer);
        subtitleTimer = undefined;
    }
    document.getElementById("subtitleText").innerHTML = "";
    //Empty all cues
    if (emptyBuffers === true) {
        vttCueBuffer = [];
    }
}

function displaySubtitle(cue, positionMS) {
    var timeOffset = cue.start - positionMS;
    if (timeOffset <= 0) {
        //no need of timer
        console.log("webvtt timeOffset: " + timeOffset + " cue: " + JSON.stringify(cue));
        vttCueBuffer.shift();
        document.getElementById("subtitleText").innerHTML = cue.text;
        subtitleTimer = setTimeout(function() {
            document.getElementById("subtitleText").innerHTML = "";
            if (vttCueBuffer.length > 0) {
                displaySubtitle(vttCueBuffer[0], positionMS + cue.duration);
            } else {
                displayTimer = undefined;
                subtitleTimer = undefined;
            }
        }, cue.duration);
    } else {
        displayTimer = setTimeout(function() {
            displaySubtitle(cue, positionMS + timeOffset);
        }, timeOffset);
    }
}

function webvttDataHandler(event) {
    console.log("webvtt data listener event: " + JSON.stringify(event));
    var subText = event.text.replace(/\n/g, "<br />");
    vttCueBuffer.push(new VTTCue(event.start, event.duration, subText));
}

function playbackStateChanged(event) {
    console.log("Playback state changed event: " + JSON.stringify(event));
    switch (event.state) {
        case playerStatesEnum.idle:
            playerState = playerStatesEnum.idle;
            break;
        case playerStatesEnum.initializing:
            playerState = playerStatesEnum.initializing;
            break;
        case playerStatesEnum.initialized:
            playerState = playerStatesEnum.initialized;
            console.log("Available audio tracks: " + playerObj.getAvailableAudioTracks());
            console.log("Available text tracks: " + playerObj.getAvailableTextTracks());

            if(playerObj.getAvailableTextTracks() != undefined)
            {
                // Remove exsisting options in list
                if(ccTracks.options.length) {
                    for(itemIndex = ccTracks.options.length; itemIndex >= 0; itemIndex--) {
                        ccTracks.remove(itemIndex);
                    }
                }

                var textTrackList = JSON.parse((playerObj.getAvailableTextTracks()));
                // Parse only the closed captioning tracks
                var closedCaptioningList = [];
                for(track=0; track<textTrackList.length;track++) {
                    if(textTrackList[track].type === "CLOSED-CAPTIONS") {
                        closedCaptioningList.push(textTrackList[track].name);
                    }
                }

                // Iteratively adding all the options to ccTracks
                for (var trackNo = 1; trackNo <= closedCaptioningList.length; trackNo++) {
                    var option = document.createElement("option");
                    option.value = trackNo;
                    option.text = closedCaptioningList[trackNo-1];
                    ccTracks.add(option);
                }
            }

            break;
        case playerStatesEnum.playing:
            //Stop vtt rendering
            if (playerState === playerStatesEnum.seeking) {
                resetSubtitles(true);
            }
            playerState = playerStatesEnum.playing;
            break;
        case playerStatesEnum.paused:
            playerState = playerStatesEnum.paused;
            break;
        case playerStatesEnum.seeking:
            //Stop vtt rendering
            if (playerState === playerStatesEnum.paused || playerState === playerStatesEnum.playing) {
                resetSubtitles(true);
            }
            playerState = playerStatesEnum.seeking;
            break;
        default:
            console.log("State not expected");
            break;
    }
    console.log("Player state is: " + playerState);
}

function mediaEndReached() {
    console.log("Media end reached event!");
//  loadNextAsset();
    if (toggleVideo() == false) {
        playerObj.stop();
        resetSubtitles(true);
        resetUIOnNewAsset();
    }
}

function mediaSpeedChanged(event) {
    console.log("Media speed changed event: " + JSON.stringify(event));
    var currentRate = event.speed;
    console.log("Speed Changed [" + playbackSpeeds[playbackRateIndex] + "] -> [" + currentRate + "]");

    if (currentRate != undefined) {
        //Stop vtt rendering
        if (currentRate !== 1) {
            resetSubtitles(currentRate !== 0);
        }

        if (currentRate != 0) {
            playbackRateIndex = playbackSpeeds.indexOf(currentRate);
        } else {
            playbackRateIndex = playbackSpeeds.indexOf(1);
        }
        if (currentRate != 0 && currentRate != 1){
            showTrickmodeOverlay(currentRate);
        }

        if (currentRate === 1) {
            document.getElementById("playOrPauseIcon").src = "../icons/pause.png";
        } else {
            document.getElementById("playOrPauseIcon").src = "../icons/play.png";
        }
    }
}

function bitrateChanged(event) {
    console.log("bitrate changed event: " + JSON.stringify(event));
}

function mediaPlaybackFailed(event) {
    console.log("Media failed event: " + JSON.stringify(event));

    playerObj.stop();
    document.getElementById('errorContent').innerHTML = event.description;
    document.getElementById('errorModal').style.display = "block";

    //Uncomment below line to auto play next asset on playback failure
    //loadNextAsset();
}

function mediaMetadataParsed(event) {
    console.log("Media metadata event: " + JSON.stringify(event));
}

function adResolvedCallback(event) {
    console.log("DAI Callback received: " + JSON.stringify(event));
}

function subscribedTagNotifier(event) {
    const metadata = event.timedMetadata;
    console.log("Subscribed tag notifier event: " + JSON.stringify(metadata));
    /* Sample format
        "timedMetadata": {
        "time":62062,
        "duration":0,
        "name":"#EXT-X-CUE",
        "content":"-X-CUE:ID=eae90713-db8e,DURATION=30.063",
        "type":0,
        "metadata": {
            "ID":"eae90713-db8e",
            "DURATION":"30.063"
            },
        "id":"eae90713-db8e"
    }
    */

    if (!metadata.name.localeCompare("SCTE35")) {
        // When SCTE35 tag is received create a reservation Object and send it to AAMP via setAlternateContent
        // Mock object given below. Here we have bypassed communication with ad server.
        const reservationObject = {
            reservationId: metadata.reservationId,
            reservationBehavior: 0,
            placementRequest: {
                id: "ad1",
                pts: 0,
                url: "http://cdn-ec-pan-02.ip-ads.xcr.comcast.net/omg05/UNI_Packaging_-_Production/453393990093/5a60d158-b057-4636-92ac-0d2ec324dd87/263/209/CSNR8803918400100001_mezz_LVLH07.mpd"
            }
        };
        // According to scte-35 duration, multiple reservation Objects with different/same ad urls and unique ad IDs can be passed to setAlternateContent.
        playerObj.setAlternateContent(reservationObject, adResolvedCallback);
    }
}

function reservationStart(event) {
    // Event marks the start of a reservation/ad break
    console.log("ReservationStart event: " + JSON.stringify(event));
}

function placementStart(event) {
    // Event marks the start of a placement/ad within an ad break
    console.log("PlacementStart event: " + JSON.stringify(event));
}

function placementProgress(event) {
    // Event marks playback progress within a placement/ad
    console.log("PlacementProgress event: " + JSON.stringify(event));
}

function placementError(event) {
    // Event marks any error within a placement/ad
    console.log("PlacementError event: " + JSON.stringify(event));
}

function placementEnd(event) {
    // Event marks the end of a placement/ad within an ad break
    console.log("PlacementEnd event: " + JSON.stringify(event));
}

function reservationEnd(event) {
    // Event marks the end of a reservation/ad break
    console.log("ReservationEnd event: " + JSON.stringify(event));
}

function id3Metadata(event) {
    // Event when ID3 metadata received
    console.log("id3Metadata event: " + JSON.stringify(event));
}

function mediaProgressUpdate(event) {
    //console.log("Media progress update event: " + JSON.stringify(event));
    //TSB length for live assets = (event.endMiliseconds - event.startMiliseconds)
    var duration = event.endMiliseconds;
    var position = event.positionMiliseconds;
    var value = ( position / duration ) * 100;
    var seekBar = document.getElementById("seekBar");

    if(displayTimer === undefined && subtitleTimer === undefined && vttCueBuffer.length !== 0 && event.playbackSpeed === 1) {
        vttCueBuffer = vttCueBuffer.filter((cue) => {
            return cue.start > position;
        });
        //console.log("Media progress: positionMiliseconds=" + position + " cueBufferLength: " + vttCueBuffer.length);
        if (vttCueBuffer.length !== 0) {
            displaySubtitle(vttCueBuffer[0], position);
        }

    }

    document.getElementById("totalDuration").innerHTML = convertSStoHr(duration / 1000.0);
    document.getElementById("currentDuration").innerHTML = convertSStoHr(position / 1000.0);
    //console.log("Media progress update event: value=" + value);
    // Update the slider value
    if(isFinite(value)) {
        seekBar.value = value;
        seekBar.style.width = value+"%";
        seekBar.style.backgroundColor = "red";
    }
}

function mediaPlaybackStarted() {
    document.getElementById("playOrPauseIcon").src = "../icons/pause.png";

    var availableVBitrates = playerObj.getVideoBitrates();
    if (availableVBitrates !== undefined) {
        createBitrateList(availableVBitrates);
    }
}

function mediaDurationChanged(event) {
    console.log("Duration changed!");
}

function decoderHandleAvailable(event) {
    console.log("decoderHandleAvailable " + event.decoderHandle);
    XREReceiver.onEvent("onDecoderAvailable", { decoderHandle: event.decoderHandle });
}

function anomalyEventHandler(event) {
    if (event.severity === anomalySeverityEnum.error || event.severity === anomalySeverityEnum.warning) {
        console.log("anomalyEventHandler got msg: " + event.description);
    }
}


function bufferingChangedHandler(event) {
    //Show buffering animation here, fired when buffers run dry mid-playback
    console.log("Buffers running empty - " + event.buffering);
    if(event.buffering === false) {
        document.getElementById('buffModal').style.display = "block";
    } else {
        document.getElementById('buffModal').style.display = "none";
    }
}

function playbackSeeked(event) {
    console.log("Play Seeked " + JSON.stringify(event));
}

function tuneProfiling(event) {
    console.log("Tune Profiling Data: " + event.microData);
}

function bulkMetadataHandler(event) {
    console.log("Bulk TimedMetadata : " + JSON.stringify(event));
}


function createAAMPPlayer(){
    var newPlayer = new AAMPPlayer("UVE-Ref-Player");
    newPlayer.addEventListener("playbackStateChanged", playbackStateChanged);
    newPlayer.addEventListener("playbackCompleted", mediaEndReached);
    newPlayer.addEventListener("playbackSpeedChanged", mediaSpeedChanged);
    newPlayer.addEventListener("bitrateChanged", bitrateChanged);
    newPlayer.addEventListener("playbackFailed", mediaPlaybackFailed);
    newPlayer.addEventListener("mediaMetadata", mediaMetadataParsed);
    newPlayer.addEventListener("timedMetadata", subscribedTagNotifier);
    newPlayer.addEventListener("playbackProgressUpdate", mediaProgressUpdate);
    newPlayer.addEventListener("playbackStarted", mediaPlaybackStarted);
    newPlayer.addEventListener("bufferingChanged", bufferingChangedHandler);
    newPlayer.addEventListener("durationChanged", mediaDurationChanged);
    newPlayer.addEventListener("decoderAvailable", decoderHandleAvailable);
    newPlayer.addEventListener("vttCueDataListener", webvttDataHandler);
    newPlayer.addEventListener("anomalyReport", anomalyEventHandler);
    newPlayer.addEventListener("reservationStart", reservationStart);
    newPlayer.addEventListener("placementStart", placementStart);
    newPlayer.addEventListener("placementProgress", placementProgress);
    newPlayer.addEventListener("placementError", placementError);
    newPlayer.addEventListener("placementEnd", placementEnd);
    newPlayer.addEventListener("reservationEnd", reservationEnd);
    newPlayer.addEventListener("seeked", playbackSeeked);
    newPlayer.addEventListener("tuneProfiling", tuneProfiling);
    //newPlayer.addEventListener("bulkTimedMetadata", bulkMetadataHandler);
    newPlayer.addEventListener("id3Metadata", id3Metadata);
    //Can add generic callback for ad resolved event or assign unique through setAlternateContent
    //newPlayer.addEventListener("adResolved", adResolvedCallback);
    return newPlayer;
}

// helper functions
function resetPlayer() {
    resetSubtitles(true);

    if (playerState !== playerStatesEnum.idle) {
        playerObj.stop();
    }
    if (playerObj !== null) {
        playerObj.destroy();
        playerObj = null;
    }

    playerObj = createAAMPPlayer();

    playerState = playerStatesEnum.idle;

    //Subscribe to interested tags from playlist
    //let tags = ["#EXT-X-SCTE35","#EXT-X-MARKER"];
    //playerObj.setSubscribedTags(tags);

    mutedStatus = false;
}

function generateInitConfigObject (urlObject) {
    console.log("UrlObject received: " + urlObject.name);
    let initConfigObject = Object.assign({}, defaultInitConfig);

    if (urlObject.name.includes("Sintel") == true) {
        initConfigObject.drmConfig = SintelDrmConfig;
    } else if (urlObject.useDefaultDrmConfig === false) {
        initConfigObject.drmConfig = null;
    }
    return initConfigObject;
}

function loadUrl(urlObject, isLive) {
    console.log("loadUrl: UrlObject received: " + urlObject);
    //set custom HTTP headers for HTTP manifest/fragment/license requests. Example provided below
    //For manifest/fragment request - playerObj.addCustomHTTPHeader("Authentication-Token:", "12345");
    //For license request - playerObj.addCustomHTTPHeader("Content-Type:", "application/octet-stream", true);

    let initConfiguration = generateInitConfigObject(urlObject);
    if(isLive)
        initConfiguration.offset = 15;
    playerObj.initConfig(initConfiguration);
    playerObj.load(urlObject.url, true);
}

function StopCachedChannel() {
    if(bgPlayerObj != null)
    {
        bgPlayerObj.stop();
        bgPlayerObj.destroy();
        bgPlayerObj = null;
    }
}

function cacheStream(urlObject, isLive) {
    console.log("cacheStream: UrlObject received: " + urlObject);
    StopCachedChannel();
	
    bgPlayerObj = createAAMPPlayer();
    let initConfiguration = generateInitConfigObject(urlObject);
    if(isLive)
        initConfiguration.offset = 15;
    bgPlayerObj.initConfig(initConfiguration);
    bgPlayerObj.load(urlObject.url, false);
}

function toggleVideo() {
    let ret = false;
    if(bgPlayerObj != null && playerObj != null)
    {
        playerObj.detach();
        bgPlayerObj.play();
        let tmpPlayer = playerObj;
        playerObj = bgPlayerObj;
        bgPlayerObj = tmpPlayer;
        cacheNextAsset();
        ret = true;
    }
    return ret;
}
