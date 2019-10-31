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
    var params = (new URL(document.location)).searchParams;
    var viewType = params.get('viewType'); 
    // for the Micro Event Viewer without Nav bar
    if(viewType == "normal") {
        //Hide Nav elements
        document.getElementById('fileUpload').style.display = "none";
        document.getElementById('files').style.display = "none";
        document.getElementById('enterText').style.display = "none";
        document.getElementById('submitButton').style.display = "none";
        var blobInfo = params.get('blobInfo'); 
        document.getElementById('enterText').value = blobInfo;
        myLoadHandler(document.getElementById('enterText').value);
    }

    function time2x(t) { // map time in milliseconds (relative to start of tune) to x-axis
        return t * 0.1 + 32;
    }

    /*
    Common:
    ct = Content Type
    it = Initiation Time of Playback in epoch format (on Receiver side)
    tt = Total Tune Time/latency
    pi = Playback Index
    ts = Tune Status
    va = Vector of tune Attempts

    Individual Tune Attempts:
    s = Start Time in epoch format
    td = Tune Duration
    st = Stream Type
    u = URL
    r = Result (1:Success, 0:Failure)
    v = Vector of Events happened

    Events:
    i = Id
    	0: Main Manifest Download
    	1: Video Playlist download
    	2: Audio Playlist download
    	3: Video Init fragment download
    	4: Audio Init fragment download
    	5: Video fragment download
    	6: Audio fragment download
    	7: Video framgment decryption
    	8: Audio framgment decryption
    	9: License Acquisition overall
    	10: License Acquisition pre-processing - Not included
    	11: License Acquisition Network
    	12: License Acquisition post-processing - Not included
    b = Beginning time of the event, relative to 's'
    d = Duration till the completion of event
    o = Output of Event (200:Success, Non 200:Error Code)
    */

    function myLoadHandler(e) {
        // Hide bucket details initially
        hideBucketDetails();
        var obj;
        if(document.getElementById('enterText').value) {
            obj = JSON.parse(e);
            document.getElementById('enterText').value = "";
        } else {
            obj = JSON.parse(e.target.result);
        }
        var contentType = obj.ct;
        var initialization_time_utc = obj.it;
        var total_tune_time = obj.tt;
        var playback_index = obj.pi;
        var tune_status = obj.ts;
        var va = obj.va;

        function myclickhandler(event) {
            var x = event.pageX;
            var y = event.pageY-31;

            for (var i = 0; i < va.length; i++) {
                var tuneInfo = va[i];
                var startTimeUtc = tuneInfo.s;
                var events = tuneInfo.v;
                var uri = tuneInfo.u;
                var sz = 16;
                var y0 = canvas.height - sz;
                var attemptStartTime = startTimeUtc - initialization_time_utc;

                for (var j = 0; j < events.length; j++) {
                    var event = events[j];
                    var eventId = event.i;
                    var eventBeginRelative = event.b;
                    var eventDuration = event.d;
                    var eventOutput = event.o;
                    var start = attemptStartTime + eventBeginRelative;
                    var finish = start + eventDuration;
                    var x0 = time2x(start);
                    var x1 = time2x(finish);
                    var bucketType;

                    switch (eventId) {
                        case 0:
                            bucketType = "Main Manifest Download";
                            break;
                        case 1:
                            bucketType = "Video Playlist Download";
                            break;
                        case 2:
                            bucketType = "Audio Playlist Download";
                            break;
                        case 3:
                            bucketType = "Video Init Fragment Download";
                            break;
                        case 4:
                            bucketType = "Audio Init Fragment Download";
                            break;
                        case 5:
                            bucketType = "Video Fragment Download";
                            break;
                        case 6:
                            bucketType = "Audio Fragment Download";
                            break;
                        case 7:
                            bucketType = "Video Fragment Decryption";
                            break;
                        case 8:
                            bucketType = "Audio Fragment Decryption";
                            break;
                        case 9:
                            bucketType = "License Acquisition Overall";
                            break;
                        case 10:
                            bucketType = "License Acquisition Pre-processing";
                            break;
                        case 11:
                            bucketType = "License Acquisition Network";
                            break;
                        case 12:
                            bucketType = "License Acquisition Post-processing";
                            break;
                    }
                    // The cordinates of the bucket
                    var x1Box = x0;
                    var y1Box = y0;
                    var x2Box = x1;
                    var y2Box = y0 + sz;
                    // Check if mouse click is inside the bucket
                    if(((x >= x1Box) && (x <= x2Box)) && ((y >= y1Box && y <= y2Box))) {
                        document.getElementById('typeID').innerHTML = bucketType;
                        document.getElementById('codeID').innerHTML = eventOutput;
                        document.getElementById('durationID').innerHTML = eventDuration + "ms";
                        document.getElementById('uriID').innerHTML = uri;
                        document.getElementById("bucketModalContent").style.top = y-240;
                        document.getElementById("bucketModalContent").style.left = x;
                        document.getElementById('bucketModal').style.display = "block";
                        break;
                    }
                    y0 -= sz;
                }
            }
        }

        var canvas = document.getElementById("myCanvas");
        canvas.onclick = myclickhandler;
        var ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.font = "18px Arial";

        // timeline backdrop
        ctx.textAlign = "center";
        var y0 = 0;
        var y1 = canvas.height - 2; // avoid overlap with canvas outline
        var shade = true;
        for (var t0 = 0; t0 <= 40000; t0 += 1000) { // render backdrop for timeline, with alternating grey bands
            var x0 = time2x(t0);
            var x1 = time2x(t0 + 1000);
            if (shade) {
                ctx.fillStyle = '#f3f3f3'; // light grey
                ctx.fillRect(x0, y0, x1 - x0, y1 - y0);
            }
            shade = !shade; // alternate rendering the light gray backdrop

            ctx.strokeStyle = '#dddddd';
            ctx.strokeRect(x0, y0, 1, 44); // tick mark

            ctx.fillStyle = '#000000';
            ctx.fillText((t0 / 1000) + "s", x0, y0 + 64); // tick text
        }

        for (var i = 0; i < va.length; i++) { // walk through the tune attempts
            var tuneInfo = va[i];
            var startTimeUtc = tuneInfo.s;
            var tuneDurationMs = tuneInfo.td;
            var streamType = tuneInfo.st;
            var uri = tuneInfo.u;
            var result = tuneInfo.r; // 1 for success
            var events = tuneInfo.v;
            var sz = 16;
            var y0 = canvas.height - sz;

            var attemptStartTime = startTimeUtc - initialization_time_utc;

            // draw translucent backdrop behind each tune attempt - allows us to see delay before first tune, and time between tunes
            ctx.globalAlpha = 0.25;
            if (tuneInfo.r == 1) { // tune attempt success
                ctx.fillStyle = '#7fff7f';
            } else { // tune attempt fail
                ctx.fillStyle = '#ff7f00';
            }
            var x0 = time2x(attemptStartTime);
            var x1 = time2x(attemptStartTime + tuneDurationMs);
            ctx.fillRect(x0, 96, x1 - x0, canvas.height - 96);

            ctx.globalAlpha = 1.0;

            for (var j = 0; j < events.length; j++) {
                var event = events[j];
                var eventId = event.i;
                var eventBeginRelative = event.b;
                var eventDuration = event.d;
                var eventOutput = event.o;

                var start = attemptStartTime + eventBeginRelative;
                var finish = start + eventDuration;
                var x0 = time2x(start);
                var x1 = time2x(finish);
                switch (eventId) {
                    case 0:
                        ctx.fillStyle = '#7f7f7f';
                        break; // Main Manifest Download

                        // video-related profiling: green
                    case 1:
                        ctx.fillStyle = '#00ff00';
                        break; // Video Playlist download
                    case 3:
                        ctx.fillStyle = '#00dd00';
                        break; // Video Init fragment download
                    case 5:
                        ctx.fillStyle = '#00bb00';
                        break; // Video fragment download
                    case 7:
                        ctx.fillStyle = '#009900';
                        break; // Video fragment decryption

                        // audio-related profiling: blue
                    case 2:
                        ctx.fillStyle = '#0000ff';
                        break; // Audio Playlist download
                    case 4:
                        ctx.fillStyle = '#0000dd';
                        break; // Audio Init fragment download
                    case 6:
                        ctx.fillStyle = '#0000bb';
                        break; // Audio fragment download
                    case 8:
                        ctx.fillStyle = '#000099';
                        break; // Audio framgment decryption

                        // drm-related profiling: yellow
                    case 9:
                        ctx.fillStyle = '#ffff00';
                        break; // License Acquisition overall
                    case 10:
                        ctx.fillStyle = '#dddd00';
                        break; // License Acquisition pre-processing - Not included
                    case 11:
                        ctx.fillStyle = '#bbbb00';
                        break; // License Acquisition Network
                    case 12:
                        ctx.fillStyle = '#999900';
                        break; // License Acquisition post-processing - Not included
                }
                ctx.fillRect(x0, y0, x1 - x0, sz);
                if (eventOutput != 200) { // draw red line through middle if error
                    ctx.fillStyle = "#ff0000";
                    ctx.fillRect(x0, y0 + sz / 2, x1 - x0, 1);
                }
                y0 -= sz;
            }
        }

        /*
        	ctx.strokeStyle = '#ff00ff'; // magenta
            ctx.beginPath();
            ctx.moveTo(x0,y0);
            ctx.lineTo(x0,y0);
            ctx.stroke();
        */

    }

    function handleFileSelect(evt) {
        //hide overlay on file upload
        document.getElementById('bucketModal').style.display = "none";
        var files = evt.target.files;
        for (var fileIndex = 0; fileIndex < files.length; fileIndex++) {
            var f = files[fileIndex];
            if (f.type = "text/plain") {
                var reader = new FileReader();
                reader.onload = myLoadHandler;
                reader.readAsText(f);
            }
        }
    }
    
    function dataEntered() {
        //when user clicks to submit the entered text
        myLoadHandler(document.getElementById('enterText').value);
    }

    document.getElementById('files').addEventListener('change', handleFileSelect, false);
    document.getElementById('submitButton').addEventListener('click', dataEntered, false);

}

// Hide bucket details when close button is clicked
function hideBucketDetails() {
    document.getElementById('bucketModal').style.display = "none";
}
