var months =
[
    "Jan","Feb","Mar",
    "Apr","May","Jun",
    "Jul","Aug","Sep",
    "Oct","Nov","Dec"
];
function ParseReceiverLogTimestamp( line )
{
    var marker = "[AAMP-PLAYER]";
    var markerOffset = line.indexOf(marker);
    if( markerOffset<0 ) return null; // ignore - not AAMP-related
    
    line = line.substr(0,markerOffset);
    var part = line.split(":");
    if( part.length==3 )
    { // "<sec>:<ms> : " format used in simulator
        var utc = parseInt(part[0])*1000+parseInt(part[1]);
        return utc;
    }
    
    // format used on settop (GMT time)
    part = line.split(" ");
    var idx = 0;
    var year = parseInt(part[idx++]);
    if( isNaN(year) )
    { // year missing (fog logs)
        year = new Date().getFullYear();
        idx--;
    }
    var month = months.indexOf(part[idx++]);
    var day = parseInt(part[idx++]);
    var time = part[idx++].split(":");
    var hour = parseInt(time[0]);
    var minute = parseInt(time[1]);
    var seconds = parseFloat(time[2]);
    var s = Math.floor(seconds);
    var ms = Math.floor((seconds - s)*1000);
    return Date.UTC(year, month, day, hour, minute, s, ms );
}

// format includes %% for numbers and strings to be extracted, in-between other literal text
// returns array of extracted values, or null if literal delimiters don't match
function sscanf( string, format )
{
    var rc = [];
    for(;;)
    {
        var nextParam = format.indexOf("%%");
        if( nextParam<0 ) return rc; // done
        if( string.substr(0,nextParam)!=format.substr(0,nextParam) )
        {
            return null;
        }
        string = string.substr(nextParam); // strip leading characters
        format = format.substr(nextParam+2); // skip past %% in format string
        var charAfterParam = format.substr(0,1);
        var idx = string.indexOf(charAfterParam);
        if( idx<0 || format=="" )
        { // no final delimiter
            rc.push( string );
        }
        else
        { // extract value between delimiters
            rc.push( string.substr(0,idx) );
            string = string.substr(idx);
        }
    }
}

function Pad( num )
{ // convert to fixed width by padding from left with spaces
    num = Math.round(num);
    if( num<10 ) return "      "+num;
    if( num<100 ) return "     "+num;
    if( num<1000 ) return "    "+num;
    if( num<10000 ) return "   "+num;
    if( num<100000 ) return "  "+num;
    if( num<1000000 ) return " "+num;
    return num;
}

var httpCurlErrMap =
{
    0:"OK",
    7:"Couldn't Connect",
    18:"Partial File",
    23:"Interrupted Download",
    28:"Operation Timed Out",
    42:"Aborted by callback",
    56:"Failure with receiving network data",
    200:"OK",
    302:"Temporary Redirect",
    304:"Not Modified",
    204:"No Content",
    400:"Bad Request",
    401:"Unauthorized",
    403:"Forbidden",
    404:"Not Found",
    500:"Internal Server Error",
    502:"Bad Gateway",
    503:"Service Unavailable"
};

function mapError( code )
{
    var desc = httpCurlErrMap[code];
    if( desc!=null )
    {
        desc = "("+desc+")";
    }
    else
    {
        desc = "";
    }
    return ((code<100)?"CURL":"HTTP") + code + desc;
}
