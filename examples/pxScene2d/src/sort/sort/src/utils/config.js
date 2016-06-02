//For testing XRE application
function define(name, value) {
    Object.defineProperty(exports, name, {
        value:      value,
        enumerable: true
    });
}

define("ks_XRERectangle", "XRERectangle");
define("ks_XREImage", "XREImage");
define("ks_XRENineSliceImage", "XRENineSliceImage");
define("ks_XREText", "XREText");
define("ks_XREHTMLText", "XREHTMLText");
define("ks_XREHTMLText2", "XREHTMLText2");
define("ks_XREFont", "XREFont");
define("ks_XREStylesheet", "XREStylesheet");
define("ks_XRETransformAnimation", "XRETransformAnimation");
define("ks_XREAlphaAnimation", "XREAlphaAnimation");
define("ks_XREDimensionsAnimation", "XREDimensionsAnimation");
define("ks_XREAbsoluteTranslationAnimation", "XREAbsoluteTranslationAnimation");
define("ks_XREAbsoluteScaleAnimation", "XREAbsoluteScaleAnimation");
define("ks_XREColorAnimation", "XREColorAnimation");
define("ks_XREApplication", "XREApplication");
define("ks_XREVideo", "XREVideo");
define("ks_XREYoutubeVideo", "XREYoutubeVideo");
define("ks_XREFlash", "XREFlash");
define("ks_XRESound", "XRESound");
define("ks_XREHTMLView", "XREHTMLView");
define("ks_XREMTheoryHTMLView", "XREMTheoryHTMLView");
define("ks_XRETextInput", "XRETextInput");
define("ks_XREScript", "XREScript");
define("ks_XREPlugin", "XREPlugin");
define("ks_XREView", "XREView");
define("ks_XRECommandSequence", "XRECommandSequence");
define("ks_XRECachedCommandSequence", "XRECachedCommandSequence");
define("ks_XREServiceProxy", "XREServiceProxy");


define("XRE_ID_NULL", 0);
define("XRE_ID_ROOT_APPLICATION", 1);
define("XRE_ID_ROOT_VIEW", 2);
define("XRE_DISCONNECTED_MSG_IMG", 3);
define("XRE_DISCONNECTED_MSG_VIEW", 4);
define("XRE_ID_CCS_FOR_APP_OVER_HTTP", 5);
define("XRE_ID_CLIENT", 2048);
define("XRE_PROTOCOL_HEADER","XRE\r\n");
define("XRE_PROTOCOL_HEADER_SIZE", 5);
define("XRE_LEN_HEADER_SIZE", 4);

define("XRE_FONT_PATH", "../fonts");

define("DEFAULT_OBJECT_FILE_PATH", "/opt");
