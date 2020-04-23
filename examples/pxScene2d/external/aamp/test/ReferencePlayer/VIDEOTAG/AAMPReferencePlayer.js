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

window.onload = function() {

    // Video
    var video = document.getElementById("video");
    
    // Buttons
    var playButton = document.getElementById("playOrPauseButton");
    var muteButton = document.getElementById("muteVideoButton");
    var closedCaptionButton = document.getElementById("ccButton");
    var rwdButton = document.getElementById("rewindButton");
    var skipBwdButton = document.getElementById("skipBackwardButton");
    var skipFwdButton = document.getElementById("skipForwardButton");
    var fwdButton = document.getElementById("fastForwardButton");
    var homeContentButton = document.getElementById('homeButton');

    // Sliders
    var seekBar = document.getElementById("seekBar");

    var ccStatus = false;
    var firstPlay = true;
    var bitrateList = [];
    video.playbackRate = 1;

	if(urls) {
        // Iteratively adding all the options to videoURLs
        for (var iter = 0; iter < urls.length; iter++) {
            var option = document.createElement("option");
            // Replce http/https with aamp at runtime
            if(urls[iter].url.startsWith("https")) {
                option.value = urls[iter].url.replace(/^https:\/\//i, 'aamps://');
            } else if(urls[iter].url.startsWith("http")) {
                option.value = urls[iter].url.replace(/^http:\/\//i, 'aamp://');
            } else {
                option.value = urls[iter].url;
            }
            option.text = urls[iter].name;
            videoURLs.add(option);
        }
    }

    //to show the navBar initially
    var navBar = document.getElementById('controlDiv');
    navBar.style.display = "block";
    var navBarNext = document.getElementById('controlDivNext');
    navBarNext.style.display = "block";

    function playPause() {
        console.log("playPause");
        if(firstPlay) {
            var firstURL;
            if(urls[0].url.startsWith("https")) {
                firstURL = urls[0].url.replace(/^https:\/\//i, 'aamps://');
            } else if(urls[0].url.startsWith("http")) {
                firstURL = urls[0].url.replace(/^http:\/\//i, 'aamp://');
            } else {
                firstURL = urls[0].url;
            }
            document.getElementById("contentURL").innerHTML = "URL: " + firstURL;
            video.src = firstURL;
            firstPlay = false;
        }
        // If it is a trick play operation
        if(video.playbackRate!=1) {
            // Start playback with normal speed
            video.playbackRate = 1;
            video.play();
        } else {
            if (video.paused == true) {
                // Play the video
                video.playbackRate = 1;
                video.play();
                document.getElementById("playOrPauseIcon").src = "../icons/pause.png"; 
            } else { // Pause the video
                video.pause();
                document.getElementById("playOrPauseIcon").src = "../icons/play.png";
            }
        }
    };

    video.muted = false;
    function mutePlayer() {
        if (video.muted == false) {
            // Mute the video
            video.muted = true;
            document.getElementById("muteIcon").src = "../icons/mute.png";
        } else {
            // Unmute the video
            video.muted = false;
            document.getElementById("muteIcon").src = "../icons/unMute.png";
        }
    };

    function closedCaptionPlayer() {
        if (ccStatus === false) {
            // CC ON
            XREReceiver.onEvent("onClosedCaptions", { enable: true });
            ccStatus = true;
            document.getElementById("ccIcon").src = "../icons/closedCaptioning.png";
            document.getElementById('ccContent').innerHTML = "CC Enabled";      
        } else {
            // CC OFF
            XREReceiver.onEvent("onClosedCaptions", { enable: false });
            ccStatus = false;
            document.getElementById("ccIcon").src = "../icons/closedCaptioningDisabled.png";
            document.getElementById('ccContent').innerHTML = "CC Disabled";
        }
        document.getElementById('ccModal').style.display = "block";
        setTimeout(function(){  document.getElementById('ccModal').style.display = "none"; }, 2000);
    };

    function goToHome() {
        window.location.href = "../index.html";
    }


    function setTime(tValue) {
        //  if no video is loaded, this throws an exception
        try {
            if (tValue == 0) {
                video.currentTime = tValue;
            } else {
                    video.currentTime += tValue;
            }

        } catch (err) {
            // errMessage(err) // show exception
            errMessage("Video content might not be loaded");
        }
    }

    // Event listener for the play/pause button
    playButton.addEventListener("click", function() {
        playPause();
    });

    // Event listener for the home button
    homeContentButton.addEventListener("click", function() {
        goToHome();
    });

    // Event listener for the mute button
    muteButton.addEventListener("click", function() {
        mutePlayer();
    });

    // Event listener for the cc button
    closedCaptionButton.addEventListener("click", function() {
        closedCaptionPlayer();
    });



    function skipBackward() {
        setTime(-300);
    };

    function skipForward() {
        setTime(300);
    };

    function fastrwd() {
        console.log("Rewind not supported");
    };

    function fastfwd() {
        console.log("Fast Forward not supported");
    };

    function overlayController() {
        // Get the modal
        if(navBar.style.display == "block") {
            navBar.style.display = "none";
        } else {
            navBar.style.display = "block";
        }
        if(navBarNext.style.display == "block") {
            navBarNext.style.display = "none";
        } else {
            navBarNext.style.display = "block";
        }        
    };

    // Convert seconds to hours
    function convertSStoHr(videoTime) {
        var hhTime = Math.floor(videoTime / 3600);
        var mmTime = Math.floor((videoTime - (hhTime * 3600)) / 60);
        var ssTime = videoTime - (hhTime * 3600) - (mmTime * 60);
        ssTime = Math.round(ssTime);

        var timeFormat = (hhTime < 10 ? "0" + hhTime : hhTime);
            timeFormat += ":" + (mmTime < 10 ? "0" + mmTime : mmTime);
            timeFormat += ":" + (ssTime  < 10 ? "0" + ssTime : ssTime);

        return timeFormat;
    };

    // Event listener for the rewind button
    rwdButton.addEventListener("click", function() {
        fastrwd();
    });

    // Event listener for the skip Backward button
    skipBwdButton.addEventListener("click", function() {
        skipBackward();
    });

    // Event listener for the skip Forward button
    skipFwdButton.addEventListener("click", function() {
        skipForward();
    });

    // Event listener for the fast Forward button
    fwdButton.addEventListener("click", function() {
        fastfwd();
    });

    // Event listener for the seek bar
    seekBar.addEventListener("change", function() {
        // Calculate the new time
        var time = video.duration * (seekBar.value / 100);
        console.log("seek cursor time");
        // Update the video time
        video.currentTime = time;
    });

    // Update the seek bar as the video plays
    video.addEventListener("timeupdate", function() {
        // Calculate the slider value
        var value = (100 / video.duration) * video.currentTime;
		console.log("in timeupdate: duration - " + video.duration + ", position: " + video.currentTime);
        if(!Number.isNaN(video.duration)) {
            document.getElementById("totalDuration").innerHTML = convertSStoHr(video.duration);
        } else {
            document.getElementById("totalDuration").innerHTML = "00:00:00";
        }
        document.getElementById("currentDuration").innerHTML = convertSStoHr(video.currentTime);
        // Update the slider value
        if(isFinite(value)) {
            seekBar.value = value;
            seekBar.style.width = value+"%";
            seekBar.style.backgroundColor = "red";
        }
    });

    // Pause the video when the seek handle is being dragged
    seekBar.addEventListener("keydown", function() {
        video.pause();
    });

    // Play the video when the seek handle is dropped
    seekBar.addEventListener("keyup", function() {
        video.play();
    });





    //document.getElementById("loadButton").addEventListener("click", getVideo, false);


    //  load video file from select field
    function getVideo() {
        firstPlay = false;
        var fileURLContent = document.getElementById("videoURLs").value; // get select field
        if (fileURLContent != "") {
            var newFileURLContent = fileURLContent;
            video.src = newFileURLContent;
            document.getElementById("contentURL").innerHTML = "URL: " + newFileURLContent;
            //get the selected index of the URL List
            var selectedURL = document.getElementById("videoURLs");
            var optionIndex = selectedURL.selectedIndex;
            //set the index to the selected field
            document.getElementById("videoURLs").selectedIndex = optionIndex;
            console.log("Loading:" + newFileURLContent);
            video.load(); // if HTML source element is used
            console.log("Playing:" + newFileURLContent);
            video.play();
            document.getElementById("playOrPauseIcon").src = "../icons/pause.png";
        } else {
            errMessage("Enter a valid video URL"); // fail silently
        }
    }

    var HTML5Player = function() {
        var that = this;
        this.init = function() {
            this.video = document.getElementById("video");

            // Buttons
            this.playButton = document.getElementById("playOrPauseButton");
            this.rwdButton = document.getElementById("rewindButton");
            this.skipBwdButton = document.getElementById("skipBackwardButton"); 
            this.skipFwdButton = document.getElementById("skipForwardButton");
            this.fwdButton = document.getElementById("fastForwardButton");
            this.muteButton = document.getElementById("muteVideoButton");
            this.closedCaptionButton = document.getElementById("ccButton");
            this.autoVideoLogButton = document.getElementById("autoLogButton");
            this.homeContentButton = document.getElementById('homeButton');

            // Sliders
            this.seekBar = document.getElementById("seekBar");
            this.videoFileList = document.getElementById("videoURLs");

            this.currentObj = this.playButton;
            this.components = [this.playButton, this.rwdButton, this.skipBwdButton, this.skipFwdButton, this.fwdButton, this.muteButton, this.closedCaptionButton, this.videoFileList, this.autoVideoLogButton, this.homeContentButton ];
            this.currentPos = 0;
            this.dropDownListVisible = false;
            this.selectListIndex = 0;
            this.prevObj = null;
            this.addFocus();
        };

        this.reset = function() {
        };

        this.keyLeft = function() {
            this.gotoPrevious();
        };

        this.keyRight = function() {
            this.gotoNext();
        };

        this.keyUp = function() {
            if (this.dropDownListVisible) {
                this.prevVideoSelect();
            } else if ((this.components[this.currentPos] == this.playButton) || (this.components[this.currentPos] == this.rwdButton) || (this.components[this.currentPos] == this.skipBwdButton) || (this.components[this.currentPos] == this.skipFwdButton) || (this.components[this.currentPos] == this.fwdButton) || (this.components[this.currentPos] == this.muteButton) || (this.components[this.currentPos] == this.closedCaptionButton)) {
                //when a keyUp is received from the buttons in the bottom navigation bar
                this.removeFocus();
                this.currentObj = this.videoFileList;
                //move focus to the first element in the top navigation bar
                this.currentPos = this.components.indexOf(this.videoFileList);
                this.addFocus();
            }
        };

        this.keyDown = function() {
            if (this.dropDownListVisible) {
                this.nextVideoSelect();
            } else if ((this.components[this.currentPos] == this.videoFileList) || (this.components[this.currentPos] == this.autoVideoLogButton) || (this.components[this.currentPos] == this.homeContentButton)) {
                //when a keyDown is received from the buttons in the top navigation bar
                this.removeFocus();
                this.currentObj = this.playButton;
                //move focus to the first element in the bottom navigation bar
                this.currentPos = 0;
                this.addFocus();
            }
        };

        this.prevVideoSelect = function() {
            if (this.selectListIndex > 0) {
                this.selectListIndex--;
            } else {
                this.selectListIndex = this.videoFileList.options.length - 1;
            }
            this.videoFileList.options[this.selectListIndex].selected = true;
        };

        this.nextVideoSelect = function() {
            if (this.selectListIndex < this.videoFileList.options.length - 1) {
                this.selectListIndex++;
            } else {
                this.selectListIndex = 0;
            }
            this.videoFileList.options[this.selectListIndex].selected = true;
        };

        this.showDropDown = function() {
            this.dropDownListVisible = true;
            var n = this.videoFileList.options.length;
            this.videoFileList.size = n;
        };

        this.hideDropDown = function() {
            this.dropDownListVisible = false;
            this.videoFileList.size = 1;
        };

        this.ok = function() {
            switch (this.currentPos) {
                case 0:
                        playPause();
                        break;
                case 1:
                        fastrwd();
                        break;
                case 2:
                        skipBackward();
                        break;
                case 3:
                        skipForward();
                        break;
                case 4:
                        fastfwd();
                        break;
                case 5:
                        mutePlayer();
                        break;
                case 6:
                        closedCaptionPlayer();
                        break;
                case 7:
                        if (this.dropDownListVisible == false) {
                            this.showDropDown();
                        } else {
                            this.hideDropDown();
                            getVideo();
                        }
                        break;
                case 8:   
                        toggleOverlay();
                        break;
                case 9: 
                        goToHome();
            };
        };

        this.gotoNext = function() {
            this.removeFocus();
            if (this.currentPos < this.components.length - 1) {
                this.currentPos++;
            } else {
                this.currentPos = 0;
            }
            this.currentObj = this.components[this.currentPos];
            this.addFocus();
        };

        this.gotoPrevious = function() {
            this.removeFocus();
            if (this.currentPos > 0) {
                this.currentPos--;
            } else {
                this.currentPos = this.components.length - 1;
            }
            this.currentObj = this.components[this.currentPos];
            this.addFocus();
        };

        this.addFocus = function() {
                this.currentObj.classList.add("focus");
                //this.currentObj.focus();
        };

        this.removeFocus = function() {
                this.currentObj.classList.remove("focus");
        };

        this.keyEventHandler = function(e, type) {
            var keyCode = e.which || e.keyCode;
            console.log("VIDEOTAG Pressed Key: " + keyCode);
            var keyCodeList = [37, 38, 39, 40, 13, 32, 112, 113, 114, 115, 116, 117, 118, 119,120];

            e.preventDefault();
            if (type == "keydown") {
                switch (keyCode) {
                    case 37: // Left Arrow
                            this.keyLeft();
                            break;
                    case 38: // Up Arrow
                            this.keyUp();
                            break;
                    case 39: // Right Arrow
                            this.keyRight();
                            break;
                    case 40: // Down Arrow
                            this.keyDown();
                            break;
                    case 13: // Enter
                            this.ok();
                            break;
                    case 34: // Page Down
                            skipBackward();
                            break;
                    case 33: // Page Up
                            skipForward();
                            break;
                    case 32:
                            this.ok();
                            break;
                    case 112: // F1
		            case 179:
                            playPause();
                            break;
                    case 113: // F2
                            mutePlayer();
                            break;
                    case 115: // F4
		            case 227:
                            fastrwd();
                            break;
                    case 116: // F5
                    case 228:
                            fastfwd();
                            break;
                    case 117: // F6
                            closedCaptionPlayer();
                            break;
                    case 118: // F7
                            overlayController();
                    default:
                            break;
                }
            }
            return false;
        }
    };

    //function to toggle Overlay widget
    function toggleOverlay() {
        var overlay = document.getElementById('overlayModal');
        var urlMod = document.getElementById('urlModal');
        document.getElementById("logCheck").checked = !document.getElementById("logCheck").checked;
        if(document.getElementById("logCheck").checked) {
            overlay.style.display = "block";
            urlMod.style.display = "block";
        } else {
            overlay.style.display = "none";
            urlMod.style.display = "none";
        }
    }

    var playerObj = new HTML5Player();
    playerObj.init();
    if (document.addEventListener) {
        document.addEventListener("keydown", function(e) {
            return playerObj.keyEventHandler(e, "keydown");
        });
    }

    function onTunedEvent() {
        console.log("onTuned event received");
    }
    
    function onTuneFailedEvent(event) {
        console.log("onTuneFailed event received: errorCode: " + event.code + " description: " + event.description);
    }
    
    function onStatusChangedEvent(event) {
        console.log("statusChanged event received: " + event.state);
    }

    function decoderHandleAvailable(event) {
        console.log("decoderHandleAvailable " + event.decoderHandle);
        XREReceiver.onEvent("onDecoderAvailable", { decoderHandle: event.decoderHandle });
    }
    
    function mediaMetadataParsed(event) {
        console.log("mediaMetadataParsed event received: " + JSON.stringify(event));
        bitrateList = [];
        for (var iter = 0; iter < event.bitrates.length; iter++) {
            bitrate = (event.bitrates[iter] / 1000000).toFixed(1);
            bitrateList.push(bitrate);
        }
        document.getElementById("availableBitratesList").innerHTML = bitrateList;
    }

    if (typeof(window.AAMP_JSController) !== "undefined") {
        console.log("Registering event listeners to AAMP_JSController");
        window.AAMP_JSController.addEventListener("playbackStarted", onTunedEvent);
        window.AAMP_JSController.addEventListener("playbackFailed", onTuneFailedEvent);
        window.AAMP_JSController.addEventListener("playbackStateChanged", onStatusChangedEvent);
        window.AAMP_JSController.addEventListener("decoderAvailable", decoderHandleAvailable);
        window.AAMP_JSController.addEventListener("mediaMetadata", mediaMetadataParsed);
    }
}
