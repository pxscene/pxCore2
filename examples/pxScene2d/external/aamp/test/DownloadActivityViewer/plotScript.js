window.onload = function() {
    var ROWHEIGHT = 24;
    var BITRATE_MARGIN = 112;
    var TIMELINE_MARGIN = 80;
    var BITRATE_SCALE = (600 / 10000000) * 3;
    var data;
    var sessionData = [];
    var sessionClicked = false;
    var sessionClickedID = 0;

    function myclickhandler(event) {
        var x = event.pageX;
        var y = event.pageY - ROWHEIGHT;
        for (var i = 0; i < data.length; i++) {
            if (x >= data[i].x && x < data[i].x + data[i].w &&
                y >= data[i].y && y < data[i].y + ROWHEIGHT) {
                alert(data[i].url);
                break;
            }
        }
    }

    var canvas = document.getElementById("myCanvas");
    canvas.onclick = myclickhandler;

    var timestamp_min;
    var timestamp_max;
    var bitrate_max;

    function time2x(t) {
        return BITRATE_MARGIN + 16 + (t - timestamp_min) * 0.1;
    }

    function bitrate2y(bitrate) {
        return (bitrate_max - bitrate) * BITRATE_SCALE + ROWHEIGHT;
    }

    function GetDownloadType(line) {
        // receiver.log compatible check for AAMP-managed ABR (no FOG); lines start with timestamp
        if (line.indexOf("aampabr#T:MANIFEST") >= 0) {
            return "MV";
        }
        if (line.indexOf("aampabr#T:VIDEO") >= 0) {
            return "V";
        }

        if (line.indexOf("abrs_manifest#t:MANIFEST-VIDEO") >= 0) {
            return "MV";
        }
        if (line.indexOf("abrs_manifest#t:DASH-MANIFEST") >= 0) {
            return "MV";
        }
        if (line.indexOf("abrs_manifest#t:MAIN-MANIFEST") >= 0) {
            return "MV";
        }
        if (line.indexOf("abrs_manifest#t:MANIFEST-AUDIO") >= 0) {
            return "MA";
        }
        if (line.indexOf("abrs_manifest#t:MANIFEST-I-FRAME") >= 0) {
            return "MI";
        }
        if (line.indexOf("abrs_fragment#t:VIDEO") >= 0) {
            return "V";
        }
        if (line.indexOf("abrs_fragment#t:AUDIO") >= 0) {
            return "A";
        }
        if (line.indexOf("abrs_fragment#t:I-FRAME") >= 0) {
            return "I";
        }
        return undefined;
    }

    function myLoadHandler(e) {
        var showbitrate = document.getElementById("showbandwidth").checked;
        var iframe_seqstart = null;
        var av_seqstart = null;
        var bandwidth_samples = [];
        var allbitrates = [];

        // initialize state
        timestamp_min = null;
        timestamp_max = null;
        bitrate_max = null;

        if (!sessionClicked) {
            // parse data
            var raw = e.target.result;

            // test code to cut out single sessions from a bigger log
            var sessions = raw.split("[processTSBRequest][INFO]new recording:");
            sessions.shift(); // to remove first null match

            var currentSession = document.getElementById("session");
            while (currentSession.firstChild) {
                currentSession.removeChild(currentSession.firstChild);
            }
            for (iter = 1; iter <= sessions.length; iter++) {
                var option = document.createElement("option");
                option.text = iter;
                option.value = iter;
                var sessionDataItem = sessions[iter - 1];
                sessionData.push(sessionDataItem);
                currentSession.add(option);
            }
            var samples = sessions[0].split("\n");
        } else {
            var samples = sessionData[sessionClickedID - 1].split("\n");
        }


        data = [];

        for (var i = 0; i < samples.length; i++) {
            var line = samples[i];
            var type = GetDownloadType(line);

            if (type != undefined) {
                var obj = {};
                obj.error = undefined;
                obj.type = type;

                fields = line.split(",");
                for (var j = 0; j < fields.length; j++) {
                    var part = fields[j].split(":");
                    var attr = part[0];
                    var value = parseInt(part[1], 10);
                    if (attr == 's') {
                        obj.utcstart = value;
                        if (timestamp_min == null || value < timestamp_min) timestamp_min = value;

                    } else if (attr == 'd') {
                        obj.durationms = value;
                        value += obj.utcstart;
                        if (timestamp_max == null || value > timestamp_max) timestamp_max = value;
                    } else if (attr == 'r') {
                        obj.bitrate = value;
                        if (bitrate_max == null || value > bitrate_max) bitrate_max = value;
                        for (var ib = 0; ib < allbitrates.length; ib++) {
                            if (allbitrates[ib] == value) break;
                        }
                        if (ib == allbitrates.length) {
                            allbitrates.push(value);
                        }
                    } else if (attr == 'estr') {
                        obj.estr = value; ///8;
                    } else if (attr == "sz") {
                        obj.bytes = value;
                    } else if (attr == 'n') {
                        obj.seqno = value;
                        if (value >= 0) { // ignore negative seqno - used for DASH 'init' data
                            if (type == 'I') {
                                if (iframe_seqstart == null || value < iframe_seqstart) iframe_seqstart = value;
                            } else {
                                if (av_seqstart == null || value < av_seqstart) av_seqstart = value;
                            }
                        }
                    } else if (attr == 'hcode' && value != 200) {
                        if (obj.error == undefined) obj.error = "";
                        obj.error += "(" + value + ")";
                    } else if (attr == "cerr" && value != 0) {
                        if (obj.error == undefined) obj.error = "";
                        obj.error += "(" + value + ")";
                    } else if (attr == "url") {
                        obj.url = fields[j];
                    }
                }
                data.push(obj);
            }
        }

        if (showbitrate) {
            for (var i = 0; i < data.length; i++) {
                //if( data[i].type != 'V' ) continue; // only consider consider bitrate while processing video fragments
                var bw_sample = {};

                bw_sample["t"] = data[i].utcstart + data[i].durationms / 2;
                t = data[i].estr; // units?
                if (t > bitrate_max) bitrate_max = t;
                bw_sample["estr"] = t;

                t = data[i].bytes * 8000 / data[i].durationms; // bits per second
                if (t > bitrate_max) bitrate_max = t;
                bw_sample["calcr"] = t;

                bandwidth_samples.push(bw_sample);
            }
            bandwidth_samples.sort(function(a, b) {
                return a.t - b.t;
            });
        }

        var canvas = document.getElementById("myCanvas");
        canvas.width = Math.min(10000, time2x(timestamp_max) + 16); // cap max width to avoid canvas limitation
        canvas.height = bitrate_max * BITRATE_SCALE + TIMELINE_MARGIN + ROWHEIGHT;
        //alert( canvas.width + "x" + canvas.height );

        var ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.font = "18px Arial";

        // timeline backdrop
        ctx.textAlign = "center";
        var y0 = bitrate2y(0);
        var y1 = bitrate2y(bitrate_max);
        var shade = true;
        for (var t0 = timestamp_min; t0 < timestamp_max; t0 += 1000) {
            var x0 = time2x(t0);
            var x1 = time2x(t0 + 1000);
            if (shade) { // light grey band for every other second
                ctx.fillStyle = '#f3f3f3';
                ctx.fillRect(x0, y0, x1 - x0, y1 - y0);
            }
            shade = !shade;

            ctx.strokeStyle = '#dddddd';
            ctx.strokeRect(x0, y0, 1, 44);

            ctx.fillStyle = '#000000';
            ctx.fillText((t0 - timestamp_min) / 1000 + "s", x0, y0 + 64);
        }

        // draw y-axis labels
        ctx.textAlign = "right";
        ctx.strokeStyle = '#dddddd';
        for (var i = 0; i < allbitrates.length; i++) {
            var bitrate = allbitrates[i];
            var y = bitrate2y(bitrate);
            ctx.strokeRect(BITRATE_MARGIN + 2, y, canvas.width, 1);

            ctx.fillStyle = '#000000';
            ctx.fillText(bitrate + "bps", BITRATE_MARGIN, y + ROWHEIGHT / 2 - 3);
        }

        ctx.textAlign = "left";
        if (showbitrate) { // render line graphs for bitrate fluctuations
            // aamp/fog-estimated available bitrate
            ctx.strokeStyle = '#ff00ff'; // magenta
            ctx.beginPath();
            for (var i = 0; i < bandwidth_samples.length; i++) {
                var t0 = bandwidth_samples[i].t;
                var x0 = time2x(t0);
                var bitrate = bandwidth_samples[i].estr;
                var y0 = bitrate2y(bitrate);
                if (i == 0) {
                    ctx.moveTo(x0, y0);
                } else {
                    ctx.lineTo(x0, y0);
                }
            }
            ctx.stroke();

            // computed per-download bitrate
            ctx.strokeStyle = '#ff7700'; // orange
            ctx.beginPath();
            for (var i = 0; i < bandwidth_samples.length; i++) {
                var t0 = bandwidth_samples[i].t;
                var x0 = time2x(t0);
                var bitrate = bandwidth_samples[i].calcr;
                var y0 = bitrate2y(bitrate);
                if (i == 0) {
                    ctx.moveTo(x0, y0);
                } else {
                    ctx.lineTo(x0, y0);
                }
            }
            ctx.stroke();
        }

        // compute default positions of bars
        for (var i = 0; i < data.length; i++) {
            var t0 = data[i].utcstart;
            var t1 = data[i].durationms + t0;
            data[i].x = time2x(t0);
            data[i].w = time2x(t1) - data[i].x;
            var bitrate = data[i].bitrate;
            data[i].y = bitrate2y(bitrate);
        }

        // adjust bar placement to avoid overlap w/ parallel downloads
        for (;;) {
            var bump = false;
            for (var i = 0; i < data.length; i++) {
                for (var j = i + 1; j < data.length; j++) {
                    if (
                        // +16 used to include labels poking out past bars as consideration for overlap
                        data[i].x + data[i].w + 16 > data[j].x &&
                        data[j].x + data[j].w + 16 > data[i].x &&
                        data[i].y + ROWHEIGHT > data[j].y &&
                        data[j].y + ROWHEIGHT > data[i].y) {
                        data[j].y += ROWHEIGHT;
                        bump = true;
                    }
                }
            }
            if (!bump) {
                break;
            }
        }

        for (var i = 0; i < data.length; i++) {
            // map colors based on download type and success/failure
            if (data[i].error != undefined) {
                ctx.fillStyle = '#ff0000';
            } else if (data[i].type == 'MV') { // video manifest
                ctx.fillStyle = '#00cc00'; // dark green
            } else if (data[i].type == 'V') { // video fragment
                ctx.fillStyle = '#ccffcc'; // light green
            } else if (data[i].type == 'MA') { // audio manifest
                ctx.fillStyle = '#0000cc'; // dark blue
            } else if (data[i].type == 'A') { // audio fragment
                ctx.fillStyle = '#ccccff'; // light blue
            } else if (data[i].type == 'MI') { // iframe manifest
                ctx.fillStyle = '#cccc00'; // dark yellow
            } else if (data[i].type == 'I') { // iframe fragment
                ctx.fillStyle = '#ffffcc'; // light yellow
            } else {
                ctx.fillStyle = '#000000';
                ctx.strokeStyle = '#000000';
            }

            ctx.fillRect(data[i].x, data[i].y - ROWHEIGHT / 2, data[i].w, ROWHEIGHT - 1);

            var seqno = data[i].seqno;
            if (seqno != null) //data[i].type == 'V' )
            {
                if (seqno < 0) {
                    seqno = "*";
                } else {
                    if (data[i].type == 'I') {
                        seqno -= iframe_seqstart;
                    } else {
                        // If wants to do normalization to the seqno 
                        if(document.getElementById("applyNorm").checked) {
                            seqno -= av_seqstart;
                        }
                    }
                }
                if (data[i].error != null) {
                    seqno += data[i].error;
                }
                ctx.fillStyle = '#000000';
                ctx.fillText(seqno, data[i].x + 2, data[i].y + ROWHEIGHT / 2 - 6);
            }
        }
    }

    function handleFileSelect(evt) {
        sessionData = [];
        sessionClickedID = 0;
        sessionClicked = false;
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

    // For a new Tune Session
    function newSession(evt) {
        var currentSession = document.getElementById("session");
        sessionClickedID = currentSession.options[currentSession.selectedIndex].value;
        sessionClicked = true;
        myLoadHandler();
    }

    // For a new checkbox change request
    function newCheckBoxChange(evt) {
        if(sessionClicked) {
            myLoadHandler();
        } else {
            var event = new Event('change');
            // Dispatch the change event
            document.getElementById('files').dispatchEvent(event);
        }
        
    }

    document.getElementById('files').addEventListener('change', handleFileSelect, false);
    document.getElementById('session').addEventListener('change', newSession, false);
    document.getElementById('applyNorm').addEventListener('change', newCheckBoxChange, false);
    document.getElementById('showbandwidth').addEventListener('change', newCheckBoxChange, false);

}
