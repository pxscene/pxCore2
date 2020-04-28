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
                alert(mediaType[data[i].type] + "\n" + data[i].url);
                break;
            }
        }
    }

    var canvas = document.getElementById("myCanvas");
    canvas.onclick = myclickhandler;

    var timestamp_min;
    var timestamp_max;
    var bitrate_max;
    var allbitrates;
    var currentBitrate;

    function time2x(t) {
        return BITRATE_MARGIN + 16 + (t - timestamp_min) * 0.1;
    }

    function bitrate2y(bitrate) {
        for( var i=0; i<allbitrates.length; i++ )
        {
            if( allbitrates[i] == bitrate ) return i*ROWHEIGHT*2 + 64;
        }
        return 0;
    }
                         
    function AddBitrate( newBitrate )
    {
        currentBitrate = newBitrate;
        for( var i=0; i<allbitrates.length; i++ )
        {
            if( allbitrates[i]==currentBitrate ) return;
        }
        allbitrates.push( currentBitrate );
    }

    var eMEDIATYPE_VIDEO = 0;
    var eMEDIATYPE_AUDIO = 1;
    var eMEDIATYPE_SUBTITLE = 2;
    var eMEDIATYPE_MANIFEST = 3;
    var eMEDIATYPE_LICENSE = 4;
    var eMEDIATYPE_IFRAME = 5;
    var eMEDIATYPE_INIT_VIDEO = 6;
    var eMEDIATYPE_INIT_AUDIO = 7;
    var eMEDIATYPE_INIT_SUBTITLE = 8;
    var eMEDIATYPE_PLAYLIST_VIDEO = 9;
    var eMEDIATYPE_PLAYLIST_AUDIO = 10;
    var eMEDIATYPE_PLAYLIST_SUBTITLE = 11;
    var eMEDIATYPE_PLAYLIST_IFRAME = 12;
    var eMEDIATYPE_INIT_IFRAME = 13;
    
    var mediaType = [ // enum MediaType
        "VIDEO",
        "AUDIO",
        "SUBTITLE",
        "MANIFEST",
        "LICENCE",
        "IFRAME",
        "INIT_VIDEO",
        "INIT_AUDIO",
        "INIT_SUBTITLE",
        "PLAYLIST_VIDEO",
        "PLAYLIST_AUDIO",
        "PLAYLIST_SUBTITLE",
        "PLAYLIST_IFRAME",
        "INIT_IFRAME"];
    
    var topMargin = 24;

    function myLoadHandler(e) {
        var iframe_seqstart = null;
        var av_seqstart = null;
        var bandwidth_samples = [];

        // initialize state
        timestamp_min = null;
        timestamp_max = null;
        bitrate_max = null;
        allbitrates = [0];
        var marker = [];

        if (!sessionClicked) {
            // parse data
            var raw = e.target.result;

            // cut out single sessions from a bigger log
            var sessions = raw.split("[AAMP-PLAYER]aamp_tune:");
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
        currentBitrate = 0;
                         
        for (var i = 0; i < samples.length; i++) {
            var line = samples[i];
            var aampLogOffset = line.indexOf( "[AAMP-PLAYER]" );
            var offs;
            if( aampLogOffset >=0 )
            { // aamp-related logging
                var payload = line.substr(aampLogOffset+13); // skip "[AAMP-PLAYER]" prefix
                var param;
                if( payload.startsWith("HttpRequestEnd:") )
                {
                    var json = payload.substr(15);
                    var httpRequestEnd = JSON.parse(json);
                    var obj = {};
                    var type = httpRequestEnd.type;
                    obj.error = mapError(httpRequestEnd.responseCode);
                    obj.durationms = 1000*httpRequestEnd.curlTime;
                    obj.type = type;
                    obj.bytes = httpRequestEnd.times.dlSz;
                    obj.url = httpRequestEnd.url;
                    var doneUtc = ParseReceiverLogTimestamp(line);
                    obj.utcstart = doneUtc-obj.durationms;
                    if (timestamp_min == null || obj.utcstart < timestamp_min) timestamp_min = obj.utcstart;
                    if (timestamp_max == null || doneUtc > timestamp_max) timestamp_max = doneUtc;
                    if( type==eMEDIATYPE_PLAYLIST_VIDEO || type==eMEDIATYPE_VIDEO )
                    {
                         obj.bitrate = currentBitrate;
//                         console.log( currentBitrate + "=" + obj.url );
                    }
                    else
                    {
                         obj.bitrate = 0;
                    }
                    data.push(obj);
                }
                else if( line.indexOf("GST_MESSAGE_EOS")>=0 )
                {
                    marker.push( [ParseReceiverLogTimestamp(line), "GST_EOS"] );
                }
                else if( line.indexOf("IP_AAMP_TUNETIME")>=0 )
                {
                    marker.push( [ParseReceiverLogTimestamp(line), "Tuned"] );
                }
                else if( param = sscanf(payload,'ABRMonitor-Reset::{"Reason":"%%","Bandwidth":%%,"Profile":%%}') )
                {
                         var reason = param[0];
                         AddBitrate(param[1]);
                         marker.push( [ParseReceiverLogTimestamp(line), reason+":"+Math.round(currentBitrate/1000)+"kbps"] );
                }
                else if( param = sscanf(payload,"AAMPLogABRInfo : switching to '%%' profile '%% -> %%' currentBandwidth[%%]->desiredBandwidth[%%]") )
                {
                        var reason = param[0];
                        AddBitrate(param[4]);
                        marker.push( [ParseReceiverLogTimestamp(line), reason+":"+Math.round(currentBitrate/1000)+"kbps"] );
                }
            }
        } // next line
        allbitrates.sort( function(a,b){return b-a;} );
                         
        var canvas = document.getElementById("myCanvas");
        canvas.width = Math.min(10000, time2x(timestamp_max) + 16); // cap max width to avoid canvas limitation
        canvas.height = allbitrates.length*ROWHEIGHT*2 + 480;

        var ctx = canvas.getContext("2d");
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.font = "18px Arial";

        // timeline backdrop
        ctx.textAlign = "center";
        var y0 = bitrate2y(0)+ROWHEIGHT;
        var y1 = bitrate2y(bitrate_max)-ROWHEIGHT;
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
            ctx.strokeRect(x0, topMargin, 1, y1-topMargin);

            ctx.fillStyle = '#000000';
            ctx.fillText((t0 - timestamp_min) / 1000 + "s", x0, topMargin );
        }

        // draw y-axis labels
        ctx.textAlign = "right";
        ctx.strokeStyle = '#dddddd';
        for (var i = 0; i < allbitrates.length; i++) {
            var bitrate = allbitrates[i];
            var y = bitrate2y(bitrate);
            ctx.strokeRect(BITRATE_MARGIN + 2, y, canvas.width, 1);

            ctx.fillStyle = '#000000';
            var label = (bitrate==0)?"audio/other":bitrate;
            ctx.fillText(label, BITRATE_MARGIN, y + ROWHEIGHT / 2 - 3);
        }
        
        ctx.textAlign = "center";
        for( var i=0; i<marker.length; i++ )
        {
            var x = time2x(marker[i][0]);
            var label = marker[i][1];
            var y = (i%8)*24 + topMargin+y0+64;

            ctx.fillStyle = '#cc0000';
            ctx.fillRect(x, topMargin, 1, y-topMargin );
                         
            ctx.fillStyle = '#000000';
            ctx.fillText(label, x, y+16 );
        }

        ctx.textAlign = "left";

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
            var pad = 0; // +16 used to include labels poking out past bars as consideration for overlap
            for (var i = 0; i < data.length; i++) {
                for (var j = i + 1; j < data.length; j++) {
                    if (
                        data[i].x + data[i].w + pad > data[j].x &&
                        data[j].x + data[j].w + pad > data[i].x &&
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
            if (data[i].error != "HTTP200(OK)" ) {
                ctx.fillStyle = '#ff0000';
            }
            else
            {
                switch( data[i].type )
                {
                case eMEDIATYPE_INIT_VIDEO:
                case eMEDIATYPE_VIDEO:
                    ctx.fillStyle = '#ccffcc'; // light green
                    break;
                         
                case eMEDIATYPE_AUDIO:
                case eMEDIATYPE_INIT_AUDIO:
                    ctx.fillStyle = '#ccccff'; // light blue
                    break;

                case eMEDIATYPE_MANIFEST:
                case eMEDIATYPE_PLAYLIST_VIDEO:
                    ctx.fillStyle = '#00cc00'; // dark green
                    break;
                         
                case eMEDIATYPE_PLAYLIST_AUDIO:
                    ctx.fillStyle = '#0000cc'; // dark blue
                    break;

                case eMEDIATYPE_PLAYLIST_SUBTITLE:
                case eMEDIATYPE_PLAYLIST_IFRAME:
                    ctx.fillStyle = '#cccc00'; // dark yellow
                    break;

                case eMEDIATYPE_INIT_IFRAME:
                case eMEDIATYPE_IFRAME:
                    ctx.fillStyle = '#ffffcc'; // light yellow
                    break;
                         
                case eMEDIATYPE_LICENSE:
                case eMEDIATYPE_INIT_SUBTITLE:
                case eMEDIATYPE_SUBTITLE:
                default:
                    ctx.fillStyle = '#aaaaaa';
                    ctx.strokeStyle = '#000000';
                    break;
                }
            }
                         
            ctx.fillRect(data[i].x, data[i].y - ROWHEIGHT / 2, data[i].w, ROWHEIGHT - 1);
      

            ctx.strokeStyle = '#999999';
            ctx.strokeRect(data[i].x, data[i].y - ROWHEIGHT / 2, data[i].w, ROWHEIGHT - 1);
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
}
