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

/**
 * @file aampoutputprotection.cpp
 * @brief Comcast Output protection management for Aamp
 */


#include "aampoutputprotection.h"
#include "config.h"
#include "priv_aamp.h"


//#define DEBUG_FUNC_TRACE 1
#ifdef DEBUG_FUNC_TRACE
#define DEBUG_FUNC fprintf(stdout, "%s --> %d\n", __FUNCTION__, __LINE__);
#else
#define DEBUG_FUNC
#endif

/* Static local variables */
AampOutputProtection* s_pAampOP = NULL;


/**
 * @brief AampOutputProtection Constructor
 */
AampOutputProtection::AampOutputProtection()
: m_sourceWidth(0)
, m_sourceHeight(0)
, m_displayWidth(1280)
, m_displayHeight(720)
, m_isHDCPEnabled(false)
, m_gstElement(NULL)
, m_hdcpCurrentProtocol(dsHDCP_VERSION_MAX)
, m_opProtectMutex()
{
    DEBUG_FUNC;
    pthread_mutex_init(&m_opProtectMutex,NULL);

#ifndef USE_OPENCDM
    memset(&m_minOPLevels, 0, sizeof(MinOPLevelsplayReady));
#endif

    // Get initial HDCP status
    SetHDMIStatus();

    // Register IARM callbacks
    logprintf("%s : registering DSMGR events...\n", __FUNCTION__);
    IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, HDMIEventHandler);
    IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS, HDMIEventHandler);
    IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionHandler);
}

/**
 * @brief AampOutputProtection Destructor
 */
AampOutputProtection::~AampOutputProtection()
{
    DEBUG_FUNC;

    // Register IARM callbacks
    logprintf("%s : unregistering DSMGR events...\n", __FUNCTION__);
    IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG, HDMIEventHandler);
    IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS, HDMIEventHandler);
    IARM_Bus_RemoveEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE, ResolutionHandler);

    s_pAampOP = NULL;

    pthread_mutex_destroy(&m_opProtectMutex);
}


/**
 * @brief Find GstElement with target name
 * @param element : GstBin in which search is done
 * @param targetName : Name of element to find
 * @retval GstElement if found, NULL otherwise
 */
GstElement* AampOutputProtection::FindElement(GstElement *element, const char* targetName)
{
    GstElement *re = NULL;

//    logprintf("%s : element = %p\n", __FUNCTION__, element);

    if(element == NULL) {
        logprintf("%s : --> element = %p\n", __FUNCTION__, element);
        return NULL;
    }

    if (GST_IS_BIN(element)) {
//        logprintf("%s : element = %p\n", __FUNCTION__, element);
        GstIterator* it = gst_bin_iterate_elements(GST_BIN(element));
        GValue item = G_VALUE_INIT;
        bool done = false;
        while(!done) {
            switch (gst_iterator_next(it, &item)) {
                case GST_ITERATOR_OK:
                {
                    GstElement *next = GST_ELEMENT(g_value_get_object(&item));
                    done = (re = FindElement(next, targetName)) != NULL;
                    g_value_reset (&item);
                    break;
                }
                case GST_ITERATOR_RESYNC:
                    gst_iterator_resync (it);
                    break;
                case GST_ITERATOR_ERROR:
                case GST_ITERATOR_DONE:
                    done = true;
                    break;
            }
        }
        g_value_unset (&item);
        gst_iterator_free(it);
    } else {
        gchar* elemName = gst_element_get_name(element);
        if(elemName != NULL) {
            if (strstr(elemName, targetName)) {
                re = element;
            }
            g_free(elemName);
        }
        else {
            logprintf("%s : --> gst_element_get_name returned NULL for element = %p\n", __FUNCTION__, element);
        }
    }
//    logprintf("%s : --> resultant element = %p\n", __FUNCTION__, element);
    return re;
}

#define TEMP_BUF_LEN 80


/**
 * @brief Check if source is UHD, by checking dimentions from Video decoder
 * @retval true, if source is UHD, otherwise false
 */
bool AampOutputProtection::IsSourceUHD()
{
    bool retVal = false;

//    DEBUG_FUNC;

#ifdef USE_SAGE_SVP
    static gint     sourceHeight    = 0;
    static gint     sourceWidth     = 0;
    GstElement*     videoDec        = NULL;

    if(m_gstElement == NULL) {
        // Value not set, since there is no
        // decoder yet the output size can not
        // be determined
        return false;
    }

    videoDec = FindElement(m_gstElement, VIDEO_DECODER_NAME);
    if (videoDec) {
        g_object_get(videoDec, "video_height", &sourceHeight, NULL);
        g_object_get(videoDec, "video_width", &sourceWidth, NULL);
    }

    if(sourceWidth != m_sourceWidth || sourceHeight != m_sourceHeight) {
        logprintf("%s viddec (%p) --> says width %d, height %d\n", __FUNCTION__, videoDec, sourceWidth, sourceHeight);
        m_sourceWidth   = sourceWidth;
        m_sourceHeight  = sourceHeight;
    }
    if(sourceWidth != 0 && sourceHeight != 0 &&
       (sourceWidth >= UHD_WITDH || sourceHeight >= UHD_HEIGHT) ) {
        // Source Material is UHD
        retVal = true;
    }
#endif
    return retVal;
}


/**
 * @brief Set the HDCP status using data from DeviceSettings
 */
void AampOutputProtection::SetHDMIStatus()
{
    bool                    isConnected              = false;
    bool                    isHDCPCompliant          = false;
    bool                    isHDCPEnabled            = true;
    dsHdcpProtocolVersion_t hdcpProtocol             = dsHDCP_VERSION_MAX;
    dsHdcpProtocolVersion_t hdcpReceiverProtocol     = dsHDCP_VERSION_MAX;
    dsHdcpProtocolVersion_t hdcpCurrentProtocol      = dsHDCP_VERSION_MAX;

    DEBUG_FUNC;

    //Get the HDMI port
    ::device::VideoOutputPort &vPort = ::device::Host::getInstance().getVideoOutputPort("HDMI0");

    try {
        isConnected        = vPort.isDisplayConnected();
        hdcpProtocol       = (dsHdcpProtocolVersion_t)vPort.getHDCPProtocol();
        if(isConnected) {
            isHDCPCompliant          = (vPort.getHDCPStatus() == dsHDCP_STATUS_AUTHENTICATED);
            isHDCPEnabled            = vPort.isContentProtected();
            hdcpReceiverProtocol     = (dsHdcpProtocolVersion_t)vPort.getHDCPReceiverProtocol();
            hdcpCurrentProtocol      = (dsHdcpProtocolVersion_t)vPort.getHDCPCurrentProtocol();
        }
        else {
            isHDCPCompliant = false;
            isHDCPEnabled = false;
        }
    }
    catch (const std::exception e) {
        logprintf("DeviceSettings exception caught in %s\n", __FUNCTION__);
    }

    m_isHDCPEnabled = isHDCPEnabled;

    if(m_isHDCPEnabled) {
        if(hdcpCurrentProtocol == dsHDCP_VERSION_2X) {
            m_hdcpCurrentProtocol = hdcpCurrentProtocol;
        }
        else {
            m_hdcpCurrentProtocol = dsHDCP_VERSION_1X;
        }
        logprintf("%s : detected HDCP version %s\n", __FUNCTION__, m_hdcpCurrentProtocol == dsHDCP_VERSION_2X ? "2.x" : "1.4");
    }
    else {
        logprintf("%s : DeviceSettings HDCP is not enabled\n", __FUNCTION__);
    }

    if(!isConnected) {
        m_hdcpCurrentProtocol = dsHDCP_VERSION_1X;
        logprintf("%s : GetHDCPVersion: Did not detect HDCP version defaulting to 1.4 (%d)\n", __FUNCTION__, m_hdcpCurrentProtocol);
    }

    return;
}

/**
 * @brief Set values of resolution member variable
 * @param width
 * @param height
 */
void AampOutputProtection::SetResolution(int width, int height)
{
    DEBUG_FUNC;

    m_displayWidth   = width;
    m_displayHeight  = height;
}

#ifndef USE_OPENCDM
// Pleayrady OP Callback
/**
 * @brief Pleayrady OP Callback to ensure HDCP compliance
 * @param f_pvOutputLevelsData : Pointer to licenses output restrictions information
 * @param f_dwCallbackType : Type of callback
 * @param data : Pointer passed from Drm_Reader_Bind, m_minOPLevels
 * @retval DRM_SUCCESS if no errors encountered
 */
DRM_RESULT DRM_CALL AampOutputProtection::PR_OP_Callback(const DRM_VOID *f_pvOutputLevelsData,
                                                                DRM_POLICY_CALLBACK_TYPE f_dwCallbackType,
                                                                const DRM_VOID *data)
{
    DRM_RESULT res = DRM_SUCCESS;

    DEBUG_FUNC;

    logprintf("%s : outputLevelsCallback outputLevels=%p callbackType=%u data=%p\n",
            __FUNCTION__, f_pvOutputLevelsData, static_cast<uint32_t>(f_dwCallbackType), data);

    AampOutputProtection *pInstance = AampOutputProtection::GetAampOutputProcectionInstance();

    // We only care about the play callback.
    if (f_dwCallbackType != DRM_PLAY_OPL_CALLBACK)
        return DRM_SUCCESS;

    // Pull out the protection levels.
    DRM_PLAY_OPL_EX* pr_Levels          = (DRM_PLAY_OPL_EX*)f_pvOutputLevelsData;
    MinOPLevelsplayReady* pm_Levels     = (MinOPLevelsplayReady*)data;

    if(pm_Levels != NULL) {
        pm_Levels->compressedDigitalVideo      = pr_Levels->minOPL.wCompressedDigitalVideo;
        pm_Levels->uncompressedDigitalVideo    = pr_Levels->minOPL.wUncompressedDigitalVideo;
        pm_Levels->analogVideo                 = pr_Levels->minOPL.wAnalogVideo;
        pm_Levels->compressedDigitalAudio      = pr_Levels->minOPL.wCompressedDigitalAudio;
        pm_Levels->uncompressedDigitalAudio    = pr_Levels->minOPL.wUncompressedDigitalAudio;

        // At actual device, enable/disable device output protection will be needed
        // upon receiving this protection information.
        logprintf("%s : compressed digital %d, uncompressed digital %d, analog video %d\n",
                  __FUNCTION__,
                  pm_Levels->compressedDigitalVideo,
                  pm_Levels->uncompressedDigitalVideo,
                  pm_Levels->analogVideo);

        // HDCP needs to be turned on for levels 270 and higher
        if(pm_Levels->uncompressedDigitalVideo >= 270) {
            // Get current HDCP level.
            if(pInstance->m_isHDCPEnabled) {
                // We have an HDCP connection
                if(pInstance->m_hdcpCurrentProtocol == dsHDCP_VERSION_1X ||
                   pInstance->m_hdcpCurrentProtocol == dsHDCP_VERSION_2X) {
                    // We have an active HDCP connection
                    logprintf("%s : HDCP is enabled version --> %d\n", __FUNCTION__, pInstance->m_hdcpCurrentProtocol);
                    res = DRM_SUCCESS;
                }
            }
            else {
                logprintf("%s : HDCP --> is not connected\n", __FUNCTION__, pInstance->m_hdcpCurrentProtocol);
                res = DRM_E_FAIL;
            }
        }
        else {
            logprintf("%s : HDCP --> is not required, current version %d,  uncompressedDigitalVideo = %d\n",
                      __FUNCTION__, pInstance->m_hdcpCurrentProtocol, pm_Levels->uncompressedDigitalVideo);
            res = DRM_SUCCESS;
        }
    }

    pInstance->Release();

    // All done.
    return res;
}
#endif


/**
 * @brief IARM event handler for HDCP and HDMI hot plug events
 * @param owner
 * @param eventId
 * @param data
 * @param len
 */
void AampOutputProtection::HDMIEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    DEBUG_FUNC;

    AampOutputProtection *pInstance = AampOutputProtection::GetAampOutputProcectionInstance();

    switch (eventId) {
        case IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG :
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdmi_hotplug_event = eventData->data.hdmi_hpd.event;

            const char *hdmihotplug = (hdmi_hotplug_event == dsDISPLAY_EVENT_CONNECTED) ? "connected" : "disconnected";
            logprintf("%s : Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d status: %s\n",
                      __FUNCTION__, hdmi_hotplug_event, hdmihotplug);

            pInstance->SetHDMIStatus();

            break;
        }
        case IARM_BUS_DSMGR_EVENT_HDCP_STATUS :
        {
            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            int hdcpStatus = eventData->data.hdmi_hdcp.hdcpStatus;
            const char *hdcpStatusStr = (hdcpStatus == dsHDCP_STATUS_AUTHENTICATED) ? "authenticated" : "authentication failure";
            logprintf("%s : Received IARM_BUS_DSMGR_EVENT_HDCP_STATUS  event data:%d status:%s\n",
                      __FUNCTION__, hdcpStatus, hdcpStatusStr);

            pInstance->SetHDMIStatus();
            break;
        }
        default:
            logprintf("%s : Received unknown IARM bus event:%d\n", __FUNCTION__, eventId);
            break;
    }

    pInstance->Release();
}

/**
 * @brief IARM event handler for resolution changes
 * @param owner
 * @param eventId
 * @param data
 * @param len
 */
void AampOutputProtection::ResolutionHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    DEBUG_FUNC;

    AampOutputProtection *pInstance = AampOutputProtection::GetAampOutputProcectionInstance();

    switch (eventId) {
        case IARM_BUS_DSMGR_EVENT_RES_PRECHANGE:
            logprintf("%s : Received IARM_BUS_DSMGR_EVENT_RES_PRECHANGE \n",__FUNCTION__);
            break;
        case IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE:
        {
            int width = 1280;
            int height = 720;

            IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
            width   = eventData->data.resn.width ;
            height  = eventData->data.resn.height ;

            logprintf("%s : Received IARM_BUS_DSMGR_EVENT_RES_POSTCHANGE event width : %d height : %d\n",
                      __FUNCTION__, width, height);

            pInstance->SetResolution(width, height);
            break;
        }
        default:
            logprintf("%s : Received unknown resolution event %d\n", __FUNCTION__, eventId);
            break;
    }

    pInstance->Release();
}


/**
 * @brief Check if  AampOutputProcectionInstance active
 * @retval true or false
 */
bool AampOutputProtection::IsAampOutputProcectionInstanceActive()
{
    bool retval = false;

    if(s_pAampOP != NULL) {
        retval = true;
    }
    return retval;
}

/**
 * @brief Singleton for object creation
 * @retval AampOutputProtection object
 */
AampOutputProtection * AampOutputProtection::GetAampOutputProcectionInstance()
{
    DEBUG_FUNC;
    if(s_pAampOP == NULL) {
        logprintf("%s --> s_pAampOP = %p\n", __FUNCTION__, s_pAampOP);
        s_pAampOP = new AampOutputProtection();
    }
    s_pAampOP->AddRef();

    return s_pAampOP;
}
