/**
 * List of supported file extensions and associated mime types (renderers)
 *
 * There are two types of mime renderers:
 * - scene - are rendered as a scene
 * - object - are rendered as a object
 */

'use strict';

module.exports =
{
  "rdkItems" : [
    {
      "title"       : "Picture Pile",
      "description" : "A Screen Saver App",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/picpile.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/picturepile.js" }
    },
    {
      "title"       : "Polaroid Picture Pile",
      "description" : "Used to launch PxScene application",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/ppp.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/polaroid/pp_polaroid.js" }
    },
    {
      "title"       : "Fish Tank",
      "description" : "A virtual fish tank showing some graphics capabilities of Spark",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/fishtank.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/fishtank/fishtank.js" }
    },
    {
      "title"       : "Coverflow",
      "description" : "UI Design Concept",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/coverflow.png",
      "attributes"  : 
        { "url":"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/gallery/coverflowtest_v2.js" }
    },
    {
      "title"       : "Animated PNGs",
      "description" : "A sample of some animated PNG images",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/apng.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/apng1.js" }
    },
    {
      "title"       : "Play Mask",
      "description" : "Rotating Images",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/playmask.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/playmask.js" }
    },
    {
      "title"       : "Gallery 3",
      "description" : "A gallery of nested scenes",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/gallery3.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/gallery3.js" }
    },
    {
      "title"       : "Horoscopes",
      "description" : "Your source for astrology",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/horoscopes.png",
      "attributes"  :
        { "url":"https://px-apps.sys.comcast.net/px_apps/horoscopes/horoscopes1.js" }
    },

    // {
    //   "title"       : "Showcase",
    //   "description" : "Guide-Like app inteneded to show Spark off-the-shelf capabilities.",
    //   "type"        : "Spark",
    //   "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/jrshowcase.png",
    //   "attributes"  :
    //     { "url":"http://sparkscapp.stb.r53.xcal.tv:3000/index.js" }
    // },

    {
      "title"       : "Gallery SVG",
      "description" : "SVG Gallery app - highlighting Spark's new SVG capabilities",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/gallerySVG.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/gallerySVG.js" }
    },
    

    {
      "title"       : "Bluetooth Streaming",
      "description" : "Play music from your Bluetooth enabled device",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/bluetoothTile.png",
      "attributes"  :
        { "url":"https://px-apps.sys.comcast.net/px_apps/bluetoothAudio_wp/dist/index.js" }
    },
  
    // {
    //   "title"       : "Bluetooth Audio In",
    //   "description" : "Play music from your Bluetooth enabled device",
    //   "type"        : "Spark",
    //   "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/btaudioin.png",
    //   "attributes"  :
    //     { "url":"https://px-apps.sys.comcast.net/px_apps/bluetooth-audio-in-app/app.js" }
    // },
    {
      "title"       : "Sleep Timer",
      "description" : "Set a timer for your TV to go to sleep when you do",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/sleeptimer.png",
      "attributes"  :
        { "url":"https://px-apps.sys.comcast.net/px_apps/sleep-timer/sleep-timer.js" }
    },
    {
      "title"       : "Fandango",
      "description" : "Get your movie tickets here",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/fandango.png",
      "attributes"  :
        { "url":"http://d243sour89mi5g.cloudfront.net/xre/pxscene/fandango/0.0.2/fandango.js?zipCode=19103" }
    },

    {
      "title"       : "Browser",
      "description" : "Browser app - navigate to additional Spark apps.",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/browser.png",
      "attributes"  :
        { "url":"browser.js" }
    },

    {
      "title"       : "Events",
      "description" : "Events app - demonstating mouse and keyboard events.",
      "type"        : "Spark",
      "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/events.png",
      "attributes"  :
        { "url":"http://www.pxscene.org/examples/px-reference/gallery/events.js" }
    },
    // {
    //   "title"       : "TopCoder Showcase",
    //   "description" : "A Video Listing App",
    //   "type"        : "Spark",
    //   "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/showcase.png",
    //   "attributes"  :
    //     { "url":"http://showcase-prototype.herokuapp.com/" }
    // },
    // {
    //   "title"       : "Web App Demo",
    //   "description" : "Used to launch web application",
    //   "type"        : "WebApp",
    //   "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/dromaeo.png",
    //   "attributes"  :
    //     { "url":"http://dromaeo.com/" }
    // },
    // {
    //   "title"       : "XRE Test Suite",
    //   "description" : "Used to perform various XRE related test cases",
    //   "type"        : "XRE",
    //   "image"       : "https://px-apps.sys.comcast.net/px_apps/showcase/images/tiles/suite.png",
    //   "attributes"  :
    //     { "url":"xre://tvxxre-ci-h2zjpp.x1.xcal.tv:10004/suite" }
    // }
  ]
};
