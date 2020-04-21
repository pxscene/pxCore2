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
    var canvasY = 0;
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
    i = Id - see profilerBuckerNames below
    b = Beginning time of the event, relative to 's'
    d = Duration till the completion of event
    o = Output of Event (200:Success, Non 200:Error Code)
    */

    var profilerBucketNames =
    [
     {"name":"main manifest download", "color":"#cccccc" },// 0
     {"name":"video playlist download", "color":"#aaffaa" }, // 1
     {"name":"audio playlist download", "color":"#aaaaff" }, // 2
     {"name":"subtitle playlist download", "color":"#aaaaaa" }, // 3
     {"name":"video initialization fragment download", "color":"#aaffaa" }, // 4
     {"name":"audio initialization fragment download", "color":"#aaaaff" }, // 5
     {"name":"subtitle initialization fragment download", "color":"#aaaaaa" }, // 6
     {"name":"video fragment download", "color":"#aaffaa" }, // 7
     {"name":"audio fragment download", "color":"#aaaaff" }, // 8
     {"name":"subtitle fragment download", "color":"#aaaaaa" }, // 9
     {"name":"video decryption", "color":"#aaffaa" }, // 10
     {"name":"audio decryption", "color":"#aaaaff" }, // 11
     {"name":"subtitle decryption", "color":"#aaaaaa" }, // 12
     {"name":"license acquisition total", "color":"#ffffaa" }, // 13
     {"name":"license acquisition pre-processing", "color":"#ffffaa" },// 14
     {"name":"license acquisition network", "color":"#ffffaa" }, // 15
     {"name":"license acquisition post-processing", "color":"#ffffaa" }, // 16
     ];

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
        if( va == undefined )
        { // make structure for JSON_FROM_AAMP_LOG.txt consistent with receiver generated JSON
            contentType = "unknown"; // unused
            initialization_time_utc = obj.s; // used!
            total_tune_time = obj.td;// unused
            playback_index = 1; // unused
            tune_status = "false"; // unused
            va = [obj];
        }

        function myclickhandler(event) {
            var x = event.pageX;
            var y = event.pageY-31;

            var sz = 20;
            for (var i = 0; i < va.length; i++) {
                var tuneInfo = va[i];
                var startTimeUtc = tuneInfo.s;
                var events = tuneInfo.v;
                var uri = tuneInfo.u;
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
                    var bucketType = profilerBucketNames[eventId].name;
                    // The coordinates of the bucket
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
                        document.getElementById("bucketModalContent").style.top = y-canvasY;
                        document.getElementById("bucketModalContent").style.left = x;
                        document.getElementById('bucketModal').style.display = "block";
                        break;
                    }
                    y0 -= sz;
                }
            }
        }

        var sz = 20;
        var canvas = document.getElementById("myCanvas");
        var requiredCanvasHeight = 72;
        for (var i = 0; i < va.length; i++) { // walk through the tune attempts
            requiredCanvasHeight += va[i].v.length*sz;
        }
        canvas.height = requiredCanvasHeight;
        canvasY = canvas.height;

        canvas.onclick = myclickhandler;
        var ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.font = "18px Arial";

        // timeline backdrop
        ctx.textAlign = "center";
        var y0 = 0;
        var y1 = canvas.height - 2; // avoid overlap with canvas outline
        var shade = true;
        for (var t0 = 0; t0 <= 20000; t0 += 1000) { // render backdrop for timeline, with alternating grey bands
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
            ctx.fillRect(x0, canvas.height - sz*events.length, x1 - x0, sz*events.length );

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
                ctx.fillStyle = profilerBucketNames[eventId].color;
                ctx.fillRect(x0, y0+1, x1 - x0, sz-2);
                ctx.strokeStyle = '#000000';
                ctx.strokeRect( x0,y0+1,x1-x0,sz-2 );
                if (eventOutput != 200) { // draw red line through middle if error
                    ctx.fillStyle = "#ff0000";
                    ctx.fillRect(x0, y0 + sz / 2, x1 - x0, 1);
                }
                ctx.textAlign = 'left';
                ctx.fillStyle = '#000000';
                ctx.fillText( profilerBucketNames[eventId].name, x1+3, y0+14 );
                y0 -= sz;
            }
        }
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
