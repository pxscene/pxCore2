
A. Overview:

    AAMP Reference Player is a bundle of reference players showcasing various AAMP interfaces. The different reference players are -

    1. HTML Reference Player - As the name suggests, this reference player showcases AAMP interfaces that are leveraged using
       standard HTML5 <video> tag. To select AAMP, the URI protocol must be changes to aamp:// or aamps:// instead of http:// or https:// respectively
       This forces webkit's MediaPlayer to pick gstaampsrc and gstaamp gstreamer plugins. The gstaamp gstreamer plugin houses the
       AAMP player.

    2. UVE Reference Player - Unified Video Engine (UVE) is a set of JavaScript binding APIs which allows its user to interact with
       AAMP player. The bindings are made available in JavaScript with the help of injectedbundle once DOM elements are loaded by webkit.
       The specification document for UVE is available at
       https://etwiki.sys.comcast.net/pages/viewpage.action?spaceKey=VIDEOARCH&title=Unified+Video+Engine+API+v0.5

B. Target audience:

    This README is targeted to OTT app vendors and HTML5 developers who are interested in adopting AAMP for their media player
    applications in STB.

C. General setup:

    C.1. Setup in RDK devices(Comcast):

         a. Host ReferencePlayer folder in a web server or in /opt/www/ folder in device
            Folders in /opt/www/ can be accessed using http://localhost:50050/<folder name>

         b. If hosted in /opt/www folder, need the following override config files in /opt folder since
            XRE rejects Fling requests with URLs with domain localhost/127.0.0.1. Contents for the override files are provided in
            Appendix section.

            b.1. /opt/webfilterpatterns.conf
            b.2. /opt/xrehtmlappswhitelist.conf


         c. To launch reference player, use Comcast's Chariot App (https://send2chariot.xreapps.net:8443/send2chariot/#/send/html)
           c.1 Fill in X1 Device ID (from XRAY)
           c.2 Select "Chariot XRE" for Launch Type
           c.3 fill in URL to launch (i.e. http://localhost:50050/ReferencePlayer/index.html)
           c.4 Specify "Default" for Browser Type
           c.5 Select "Mute Player"
           c.6 Select "Use Background"
           c.7 Select "Enable Debug Mode -1"
           c.8 Click "Send" button

D. Features

    1. Clear and encrypted(Access/AES-128) HLS supported
    2. Clear and encrypted(Playready/Widevine) DASH supported
    3. Supports inband closedcaptions
    4. Audio language selection is supported
    5. Trickplay and Audio/Video bitrate selection is supported

E. Folder structure

   -icons                            // UI elements of reference players and homepage
   -UVE
     -index.html                     // Homepage of UVE reference player
     -UVEMediaPlayer.js              // Includes the "AAMPPlayer" JS class which wraps UVE binding object AAMPMediaPlayer.
     -UVEPlayerUI.js                 // JS code for the UI elements and their functionality
     -UVERefPlayer.js                // Main JS file
     -UVERefPlayerStyle.js           // CSS for reference player and its UI
   -VIDEOTAG
     -AAMPReferencePlayer.js         // JS code for HTML reference player
     -AAMPReferencePlayerStyle.js    // CSS for reference player and its UI
     -index.html                     // Homepage of HTML reference player
   -index.html                       // Homepage of reference player
   -ReferencePlayer.js               // JS  code for Homepage and redirection to respective reference players
   -URLs.js                          // JS code to give URLs
   -ReferencePlayerStyle.css         // CSS for Homepage and its UI

F. Appendix

a. /opt/webfilterpatterns.conf
{"webfilter":[
  {"host":"localhost", "block":"0"},
  {"host":"127.0.0.1", "block":"1"},
  {"host":"169.254.*", "block":"1"},
  {"host":"192.168.*", "block":"1"},
  {"host":"[::1]", "block":"1"},
  {"scheme":"file", "block":"1"},
  {"scheme":"ftp", "block":"1"},
  {"scheme":"*", "block":"0"},
  {"block":"1"}
]}

b. /opt/xrehtmlappswhitelist.conf
{"htmlappswhitelist":
  [
    {
      "name" : "About Blank Page",
      "url" : "about:blank"
    },
    {
      "name" : "Viper Player",
      "url" : "ccr.player-platform-stage.xcr.comcast.net/index.html"
    },
    {
      "name" : "Google",
      "url" : "www.google.com"
    },
    {
      "name" : "AAMP Reference Player",
      "url" : "localhost:50050/ReferencePlayer/index.html"
    }
  ]
}
