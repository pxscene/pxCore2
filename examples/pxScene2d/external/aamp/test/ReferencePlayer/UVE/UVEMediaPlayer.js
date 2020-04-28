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

//Dummy implementation for running in Chrome/Firefox/Safari
/*
class AAMPMediaPlayer {

    load(uri) {
        console.log("Invoked load");
    }

    initConfig(IConfig) {
        console.log("Invoked initConfig");
    }

    play() {
        console.log("Invoked play");
    }

    pause() {
        console.log("Invoked pause");
    }

    stop() {
        console.log("Invoked stop");
    }

    release() {
        console.log("Invoked release");
    }

    seek(timeSec) {
        console.log("Invoked seek");
    }

    getCurrentState() {
        console.log("Invoked getCurrentState");
    }

    getDurationSec() {
        console.log("Invoked getDurationSec");
    }

    getCurrentPosition() {
        console.log("Invoked getCurrentPosition");
    }

    getVideoBitrates() {
        console.log("Invoked getVideoBitrates");
    }

    getAudioBitrates() {
        console.log("Invoked getAudioBitrates");
    }

    getCurrentVideoBitrate() {
        console.log("Invoked getCurrentVideoBitrate");
    }

    setVideoBitrate(bitrate) {
        console.log("Invoked setVideoBitrate");
    }

    getCurrentAudioBitrate() {
        console.log("Invoked getCurrentAudioBitrate");
    }

    setAudioBitrate(bitrate) {
        console.log("Invoked setAudioBitrate");
    }

    getAudioTrack() {
        console.log("Invoked getAudioTrack");
    }

    setAudioTrack(track) {
        console.log("Invoked setAudioTrack");
    }

    getTextTrack() {
        console.log("Invoked getTextTrack");
    }

    setTextTrack(track) {
        console.log("Invoked setTextTrack");
    }

    getVolume() {
        console.log("Invoked getVolume");
    }

    setVolume(volume) {
        console.log("Invoked setVolume");
    }

    getPlaybackRate() {
        console.log("Invoked getPlaybackRate");
    }

    setPlaybackRate(rate, overshoot) {
        console.log("Invoked setPlaybackRate");
    }

    getSupportedKeySystems() {
        console.log("Invoked getSupportedKeySystems");
    }

    setVideoMute(enabled) {
        console.log("Invoked setVideoMute");
    }

    setSubscribedTags(tagNames) {
        console.log("Invoked setSubscribedTags");
    }

    addEventListener(name, handler) {
        console.log("Invoked addEventListener - " + name);
    }

    removeEventListener(name, handler) {
        console.log("Invoked removeEventListener");
    }

    setDRMConfig(config) {
        console.log("Invoked setDRMConfig");
    }

    addCustomHTTPHeader(headerName, headerValue, isLicenseRequest = false) {
        console.log("Invoked addCustomHTTPHeader");
    }

    removeCustomHTTPHeader(headerName) {
        console.log("Invoked removeCustomHTTPHeader");
    }

    setVideoRect(x, y, w, h) {
        console.log("Invoked setVideoRect");
    }

    setVideoZoom(videoZoom) {
        console.log("Invoked setVideoZoom");
    }

    getAvailableAudioTracks() {
       console.log("Invoked getAvailableAudioTracks");
    }

    getAvailableTextTracks() {
       console.log("Invoked getAvailableTextTracks");
    }

    getVideoRectangle() {
       console.log("Invoked getVideoRectangle");
    }
};
*/
// AAMP Player impl
// AAMPMediaPlayer is defined and available in STB
var playerStatesEnum = { "idle":0, "initializing":1, "initialized":2, "playing":8, "paused":6, "seeking":7 };
Object.freeze(playerStatesEnum);

var anomalySeverityEnum = { "error":0, "warning":1, "trace":2 };
Object.freeze(anomalySeverityEnum);

class AAMPPlayer {

    constructor(appName) {
        //If you run into an error AAMPMediaPlayer is undefined with Chrome/Firefox/Safari please uncomment the above dummy impl
        this.player = new AAMPMediaPlayer(appName);
        this.url = "";
    }

    destroy() {

        if (this.player.getCurrentState() !== playerStatesEnum.idle) {
            this.player.stop();
        }

        this.player.release();
        this.player = null;
    }

    /**
     * URI of the Media being played by the Video Engine
     */
    load(url, foreground) {
        console.log("Url received: " + url);
        this.url = url;
        this.player.load(url, foreground);
    }

    /**
     * IConfig Object with key value pair of launch configuration
     */
    initConfig(configObj) {
        console.log("Invoked initConfig with config: " + JSON.stringify(configObj));
        this.player.initConfig(configObj);
    }

    /**
     * Starts playback when enough data is buffered at playhead
     */
    play() {
        this.player.play();
    }

    /**
     * Soft stop the player instance
     */
    detach() {
        this.player.detach();
    }
    
    /**
     * Pauses playback
     */
    pause() {
        this.player.pause();
    }

    /**
     * Stop playback and free resources
     */
    stop() {
        // Turn OFF CC
        XREReceiver.onEvent("onDecoderAvailable", { decoderHandle: null });
        this.player.stop();
    }

    /**
     * Performs a seek
     * @param timeSec the time in seconds to seek
     */
    seek(timeSec) {
        this.player.seek(timeSec);
    }

    /**
     * Gets the current state
     * This defaults to PlayState.PAUSED
     */
    getCurrentState() {
        return this.player.getCurrentState();
    }

    /**
     * Gets the duration of the content in seconds.
     */
    getDurationSec() {
        return this.player.getDurationSec();
    }

    /**
     * Gets the current user time in seconds.
     */
    getCurrentPosition() {
        return this.player.getCurrentPosition();
    }

    /**
     * Gets available video bitrates
     */
    getVideoBitrates() {
        return this.player.getVideoBitrates();
    }

    /**
     * Gets available audio bitrates
     */
    getAudioBitrates() {
        return this.player.getAudioBitrates();
    }

    /**
     * Gets the current video bitrate.
     */
    getCurrentVideoBitrate() {
        return this.player.getCurrentVideoBitrate();
    }

    /**
     * Sets the current video bitrate.
     */
    setVideoBitrate(bitrate) {
        this.player.setVideoBitrate(bitrate);
    }

    /**
     * Gets the current audio bitrate
     */
    getCurrentAudioBitrate() {
        return this.player.getCurrentAudioBitrate();
    }

    /**
     * Sets the current audio bitrate
     */
    setAudioBitrate(bitrate) {
        this.player.setAudioBitrate(bitrate);
    }

    /**
     * Gets the current audio track
     */
    getAudioTrack() {
        return this.player.getAudioTrack();
    }

    /**
     * Sets the current audio track
     */
    setAudioTrack(track) {
        this.player.setAudioTrack(track);
    }

    /**
     * Gets the current text track
     */
    getTextTrack() {
        return this.player.getTextTrack();
    }

    /**
     * Sets the current text track
     */
    setTextTrack(track) {
        this.player.setTextTrack(track);
    }

    /**
     * Gets the current volume (value between 0 and 1)
     */
    getVolume() {
        return this.player.getVolume();
    }

    /**
     * Sets the current volume (value between 0 and 1)
     */
    setVolume(volume) {
        this.player.setVolume(volume);
    }

    /**
     * Sets the Audio Language
     */
    setAudioLanguage(language) {
        this.player.setAudioLanguage(language);
    }

    /**
     * Gets the current playback rate
     */
    getPlaybackRate() {
        return this.player.getPlaybackRate();
    }

    /**
     * Sets the current playback rate and overshoot if necessary
     */
    setPlaybackRate(rate, overshoot = 0) {
        this.player.setPlaybackRate(rate, overshoot);
    }

    // DRM
    /**
     * Returns a list of key system the video engine supports -- redundant?
     */
    getSupportedKeySystems() {
        return this.player.getSupportedKeySystems();
    }

    /**
     * Black out the video for parental controls
     */
    setVideoMute(enabled) {
        this.player.setVideoMute(enabled);
    }

    // DAI
    /**
     * Subscribe to specific tags / DOM Element
     */
    setSubscribedTags(tagNames) {             // Used to identify Content Break period
         this.player.setSubscribedTags(tagNames);
    }

    addEventListener(eventName, eventHandler) {
        return this.player.addEventListener(eventName, eventHandler, null);
    }

    removeEventListener(eventName, eventHandler) {
        return this.player.removeEventListener(eventName, eventHandler, null);
    }

    /**
     * Set the DRM config params
     */
    setDRMConfig(config) {
        this.player.setDRMConfig(config);
    }

    /**
     * Add custom headers to HTTP requests
     * isLicenseRequest indicates if headers are for license requests
     */
    addCustomHTTPHeader(headerName, headerValue, isLicenseRequest = false) {
        this.player.addCustomHTTPHeader(headerName, headerValue, isLicenseRequest);
    }

    /**
     * Remove a custom headers set previously, with empty args will delete all
     */
    removeCustomHTTPHeader(headerName) {
        this.player.removeCustomHTTPHeader(headerName);
    }

    /**
     * Set display video rectangle co-ordinates
     */
    setVideoRect(x, y, w, h) {
        this.player.setVideoRect(x, y, w, h);
    }

    /**
     * Set video zoom
     */
    setVideoZoom(videoZoom) {
        this.player.setVideoZoom(videoZoom);
    }
    
    /**
     * Get UVE API version
     */
    get version() {
        return this.player.version;
    }

    /**
     * Set Alternate Content
     */
    setAlternateContent(reservationObject, callback) {
	this.player.setAlternateContent(reservationObject, callback);
    }

    /**
     * Get available audio track info
     */
    getAvailableAudioTracks() {
       return this.player.getAvailableAudioTracks();
    }

    /**
     * Get available text track info
     */
    getAvailableTextTracks() {
       return this.player.getAvailableTextTracks();
    }

    /**
     * Get current video rectangle co-ordinates
     */
    getVideoRectangle() {
       return this.player.getVideoRectangle();
    }

};
