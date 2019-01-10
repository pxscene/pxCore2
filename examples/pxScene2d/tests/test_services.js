var Services = [
    "systemService/1=requestSystemUptime,reboot",
    "systemService/2=reboot,requestSystemUptime,getPowerState,setPowerState",
    "systemService/3=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested",
    "systemService/4=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions",
    "systemService/5=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled",
    "systemService/6=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled,getFirmwareUpdateInfo,updateFirmware,setMode,getMode",
    "systemService/7=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled,getFirmwareUpdateInfo,updateFirmware,setMode,getMode,setPreferredStandbyMode,getPreferredStandbyMode,getAvailableStandbyModes,setDeepSleepTimer",
    "systemService/8=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled,getFirmwareUpdateInfo,updateFirmware,setMode,getMode,setPreferredStandbyMode,getPreferredStandbyMode,getAvailableStandbyModes,setDeepSleepTimer,getXconfParams,getSerialNumber",
    "systemService/9=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled,getFirmwareUpdateInfo,updateFirmware,setMode,getMode,setPreferredStandbyMode,getPreferredStandbyMode,getAvailableStandbyModes,setDeepSleepTimer,getXconfParams,getSerialNumber,getFirmwareDownloadPercent,getDownloadedFirmwareInfo,getFirmwareUpdateState",
    "systemService/10=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled,getFirmwareUpdateInfo,updateFirmware,setMode,getMode,setPreferredStandbyMode,getPreferredStandbyMode,getAvailableStandbyModes,setDeepSleepTimer,getXconfParams,getSerialNumber,getFirmwareDownloadPercent,getDownloadedFirmwareInfo,getFirmwareUpdateState,getMacAddress",
    "systemService/11=reboot,requestSystemUptime,getPowerState,setPowerState,hasRebootBeenRequested,getSystemVersions,setGzEnabled,isGzEnabled,getFirmwareUpdateInfo,updateFirmware,setMode,getMode,setPreferredStandbyMode,getPreferredStandbyMode,getAvailableStandbyModes,setDeepSleepTimer,getXconfParams,getSerialNumber,getFirmwareDownloadPercent,getDownloadedFirmwareInfo,getFirmwareUpdateState,getMacAddress,getCoreTemperature,getTimeZoneDST,setTimeZoneDST",
    "org.openrdk.DisplaySettings/1=setCurrentResolution,getCurrentResolution,getSupportedResolutions,getSupportedVideoDisplays,getConnectedVideoDisplays,setZoomSetting,getZoomSetting,setSoundMode,getSoundMode",
    "org.openrdk.DisplaySettings/3=setCurrentResolution,getCurrentResolution,getSupportedResolutions,getSupportedVideoDisplays,getConnectedVideoDisplays,setZoomSetting,getZoomSetting,setSoundMode,getSoundMode,getSupportedAudioModes",
    "org.openrdk.DisplaySettings/4=setCurrentResolution,getCurrentResolution,getSupportedResolutions,getSupportedVideoDisplays,getConnectedVideoDisplays,setZoomSetting,getZoomSetting,setSoundMode,getSoundMode,getSupportedAudioModes,readEDID,readHostEDID",
    "org.openrdk.DisplaySettings/5=setCurrentResolution,getCurrentResolution,getSupportedResolutions,getSupportedVideoDisplays,getConnectedVideoDisplays,setZoomSetting,getZoomSetting,setSoundMode,getSoundMode,getSupportedAudioModes,readEDID,readHostEDID,getSupportedAudioPorts,getConnectedAudioPorts,getActiveInput",
    "FrontPanelService/1=powerLedOn,powerLedOff,setBrightness,getBrightness",
    "FrontPanelService/2=powerLedOn,powerLedOff,setBrightness,getBrightness,setClockBrightness,getClockBrightness,getFrontPanelLights",
    "FrontPanelService/3=powerLedOn,powerLedOff,setBrightness,getBrightness,setClockBrightness,getClockBrightness,getFrontPanelLights",
    "FrontPanelService/4=powerLedOn,powerLedOff,setBrightness,getBrightness,setClockBrightness,getClockBrightness,getFrontPanelLights,setLED,setBlink,setPreferences,getPreferences",
    "FrontPanelService/5=powerLedOn,powerLedOff,setBrightness,getBrightness,setClockBrightness,getClockBrightness,getFrontPanelLights,setLED,setBlink,setPreferences,getPreferences,set24HourClock,is24HourClock",
    "homeNetworkingService/1=isVPOPEnabled,setVPOPEnabled,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled",
    "homeNetworkingService/2=isVPOPEnabled,setVPOPEnabled,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled,getDiscoveredDevices",
    "homeNetworkingService/3=isVPOPEnabled,setVPOPEnabled,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled,getDiscoveredDevices,isUpnpEnabled,setUpnpEnabled",
    "homeNetworkingService/4=isVidiPathEnabled,setVidiPathEnabled,isVidiPathCapable,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled,getDiscoveredDevices,isUpnpEnabled,setUpnpEnabled",
    "homeNetworkingService/5=isVidiPathEnabled,setVidiPathEnabled,isVidiPathCapable,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled,getDiscoveredDevices,isUpnpEnabled,setUpnpEnabled,getStreamingDevices",
    "homeNetworkingService/6=isVidiPathEnabled,setVidiPathEnabled,isVidiPathCapable,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled,getDiscoveredDevices,isUpnpEnabled,setUpnpEnabled,getStreamingDevices,getActiveNetworkInterface,getNetworkInterfaces,isInterfaceEnabled,setInterfaceEnabled",
    "homeNetworkingService/7=isVidiPathEnabled,setVidiPathEnabled,isVidiPathCapable,getDeviceName,setDeviceName,isMdvrEnabled,setMdvrEnabled,getDiscoveredDevices,isUpnpEnabled,setUpnpEnabled,getStreamingDevices,getActiveNetworkInterface,getNetworkInterfaces,isInterfaceEnabled,setInterfaceEnabled,getInterfaceControlPersistence,setInterfaceControlPersistence",
    "screenCaptureService=uploadScreenCapture",
    //"MsoPairingAPI=xrPairingStart,xrValidationUpdate,xrValidationComplete",
    "MSOPairingService/2=setInitialTimeout,setBWKeyPressTimeout,setTimeouts,setMaxValidationAttempts,setDontCycle",
    "MSOPairingService/3=setInitialTimeout,setBWKeyPressTimeout,setTimeouts,setMaxValidationAttempts,setDontCycle,getBatteryStatus,getFirmwareVersion",
    "MSOPairingService/4=setInitialTimeout,setBWKeyPressTimeout,setTimeouts,setMaxValidationAttempts,setDontCycle,getBatteryStatus,getFirmwareVersion,lastKeyPressedTimestamp,lastRfKeyPressedTimestamp,lastIrKeyPressedTimestamp",
    "MSOPairingService/5=setInitialTimeout,setBWKeyPressTimeout,setTimeouts,setMaxValidationAttempts,setDontCycle,getBatteryStatus,getFirmwareVersion,lastKeyPressedTimestamp,lastRfKeyPressedTimestamp,lastIrKeyPressedTimestamp,setPairingState",
    "MSOPairingService/6=setInitialTimeout,setBWKeyPressTimeout,setTimeouts,setMaxValidationAttempts,setDontCycle,getBatteryStatus,getFirmwareVersion,lastKeyPressedTimestamp,lastRfKeyPressedTimestamp,lastIrKeyPressedTimestamp,setPairingState,getLastKeypressSource,getLastPairingTimestamp",
    "com.comcast.stateObserver/1=getValues,registerListeners,unregisterListeners",
    "com.comcast.stateObserver/2=getValues,registerListeners,unregisterListeners",
    "org.openrdk.StoragemanagerService/1=getDeviceIds,getDeviceInfo,getTSBStatus,getTSBMaxMinutes,setTSBMaxMinutes,isTSBEnabled,setTSBEnabled",
    "org.openrdk.StoragemanagerService/3=getDeviceIds,getDeviceInfo,getTSBStatus,getTSBMaxMinutes,setTSBMaxMinutes,isTSBEnabled,setTSBEnabled,getTSBCapacityMinutes,getTSBCapacity,getTSBFreeSpace,getDVRCapacity,getDVRFreeSpace,isDVREnabled,setDVREnabled,getHDDHealth,getHDDInfo",
    "com.comcast.TsbSettings=enableTsb,getTsbState,getTsbProperties,setTsbProperties",
    "Warehouse/1=resetDevice,getVersion",
    "Warehouse/2=resetDevice,getDeviceInfo,setFrontPanelState",
    "Warehouse/3=resetDevice,getDeviceInfo,setFrontPanelState,internalReset",
    "WebSocketService=getUrl,getReadyState,connect,send,close",
    //"ShazamAPI=snagAudioNow,stopRecording",
    "BrowserSettings=deleteCookies,deleteCache",
    "AVInput=numberOfInputs,currentVideoMode,contentProtected",
    "MemoryInfo=getInfo",
    //"RFRemoteAPI=startPairing,getBatteryLevel,getBoundRemotes,unbind,announceFirmware,installFirmware",
    //"Skype_1=start,stop,initiateSkypeCall,answerSkypeCall",
    //"Skype_2=start,stop,initiateSkypeCall,answerSkypeCall",
    "com.comcast.hdmiCec_1=setEnabled,getEnabled,setName,getName,getConnectedDevices,sendMessage,getCECAddresses",
    "com.comcast.hdmiInput_1=getHDMIInputDevices",
    //"com.comcast.hdmiInput_2=getHDMIInputDevices,writeEDID,readEDID",
    "org.rdk.wifiManager/1=getCurrentState,getAvailableSSIDs,setEnabled,connect,saveSSID,clearSSID,getPairedSSID,isPaired",
    "org.rdk.wifiManager/2=getCurrentState,getAvailableSSIDs,setEnabled,connect,saveSSID,clearSSID,getPairedSSID,isPaired,isConnectedLNF,switchToPrivate",
    "org.rdk.wifiManager/3=getCurrentState,getAvailableSSIDs,setEnabled,connect,saveSSID,clearSSID,getPairedSSID,isPaired,isConnectedLNF,switchToPrivate,getPairedSSIDInfo, getSwitchToPrivateResults, setAutoSwitchToPrivateEnabled, isAutoSwitchToPrivateEnabled, setStopLNFWhileDisconnected, isStopLNFWhileDisconnected",
    "org.rdk.wifiManager/4=getCurrentState,getAvailableSSIDs,setEnabled,connect,saveSSID,clearSSID,getPairedSSID,isPaired,isConnectedLNF,switchToPrivate,getPairedSSIDInfo, getSwitchToPrivateResults, setAutoSwitchToPrivateEnabled, isAutoSwitchToPrivateEnabled, setStopLNFWhileDisconnected, isStopLNFWhileDisconnected",
    "org.rdk.dataCapture_1=enableAudioCapture,getAudioClip",
    "org.rdk.traceroute_1=trace,traceNamedEndpoint,getNamedEndpoints",
    "org.rdk.bluetoothSettings_1=getStatusSupport,startDeviceDiscovery,stopDeviceDiscovery,getDiscoveredDevices,getPairedDevices,getConnectedDevices,setDeviceConnection,setAudioStream,setDevicePairing,setBluetoothEnabled,setSinkDiscovery,getDeviceInfo",
    "com.comcast.copilot_1=terminate,getState",
    "org.openrdk.ControlService/1=",
    "org.openrdk.ControlService/2=getAllRemoteData,getSingleRemoteData,getLastKeypressSource",
    "org.openrdk.ControlService/3=getAllRemoteData,getSingleRemoteData,getLastKeypressSource",
    "org.openrdk.ControlService/4=getAllRemoteData,getSingleRemoteData,getLastKeypressSource",
    "org.openrdk.RemoteActionMappingService/1=getLastUsedDeviceID,getKeymap,setKeyActionMapping,clearKeyActionMapping,getFullKeyActionMapping,getSingleKeyActionMapping",
    "org.openrdk.RemoteActionMappingService/2=getLastUsedDeviceID,getKeymap,setKeyActionMapping,clearKeyActionMapping,getFullKeyActionMapping,getSingleKeyActionMapping",
    "com.comcast.application_3=getAppInfo,setAppRestart,setConnectionReset,setConnectionSuspend",
    "org.rdk.ping_1=ping,pingNamedEndpoint,getNamedEndpoints",
    "com.comcast.HDCPProfile/1=setHDCPProfile,getHDCPProfile",
    "com.comcast.HDCPProfile/2=setHDCPProfile,getHDCPProfile,getHDCPStatus",
    "com.comcast.HDCPProfile/3=setHDCPProfile,getHDCPProfile,getHDCPStatus",
    "org.openrdk.VREXmanagerService=getValues,registerListeners,unregisterListeners",
    "org.openrdk.VREXmanagerService/2=getValues,registerListeners,unregisterListeners,setValues",
    "org.openrdk.VREXmanagerService/3=getValues,registerListeners,unregisterListeners,setValues",
    "org.openrdk.videoApplicationEvents_1=setApplications,getApplications,setEnabled,isEnabled",
    "com.comcast.XreFrameRateService/2=setCollectionFrequency,startFpsCollection,stopFpsCollection",
    "rdk.linearSegmentedAdvertising_1=enable,isEnabled",
    "org.rdk.hdcpCompliance_1=getHDCPStatus",
    "com.comcast.XREReceiverDiagnostics=getXREAppMetrics,getPerformanceMetrics",
    "com.comcast.webcamera=start,stop",
    "org.rdk.intrusionDetection_1=enable,isEnabled",
    "org.openrdk.ETVIntegratedSignaling_1=setApplicationIDFilters,setMaxRandomDelay,setEnableEISSMessages,isEnableEISSMessages",
    "org.rdk.IPDirectUnicast_1=enable,isEnabled",
    "org.rdk.LoggingPreferences=isKeystrokeMaskEnabled,setKeystrokeMaskEnabled,onReportKeystrokeMaskChanged",
    "com.comcast.XreFrameRateService=setCollectionFrequency,startFpsCollection,stopFpsCollection",
]

var serviceEvents = new Map();
serviceEvents.set("MSOPairingService"                       , "xrPairingStart,xrValidationComplete,xrValidationUpdate,xrConfigurationComplete");
serviceEvents.set("homeNetworkingService"                   , "deviceDiscoveryUpdate");
serviceEvents.set("AVInput"                                 , "onAVInputActive,onAVInputInactive");
serviceEvents.set("com.comcast.stateObserver"               , "propertyChanged");
serviceEvents.set("com.comcast.hdmiInput_1"                 , "onDevicesChanged");
serviceEvents.set("org.rdk.hdcpCompliance_1"                , "onDisplayConnectionChanged");
serviceEvents.set("com.comcast.XreFrameRateService"         , "onFpsEvent");
serviceEvents.set("org.rdk.wifiManager"                     , "onWIFIStateChanged,onError,onSSIDsChanged,onPairingStateChanged,onWPSPairingButtonPressed");
serviceEvents.set("org.rdk.LoggingPreferences"              , "onKeystrokeMaskEnabledChange");
serviceEvents.set("com.comcast.webcamera"                   , "onWebcameraConnected,onWebcameraDisconnected");
serviceEvents.set("org.rdk.traceroute_1"                    , "onTraceComplete");
serviceEvents.set("org.rdk.bluetoothSettings_1"             , "statusChanged,pairingRequest,pinRequest,requestFailed");
serviceEvents.set("com.comcast.copilot_1"                   , "onStateChange");
serviceEvents.set("org.rdk.intrusionDetection_1"            , "onIntrusionDetected");
serviceEvents.set("org.openrdk.videoApplicationEvents_1"    , "onStart,onComplete,onWatched,onOccurence");
serviceEvents.set("org.rdk.ping_1"                          , "onPingComplete");
serviceEvents.set("org.openrdk.DisplaySettings"             , "connectedVideoDisplaysUpdated,resolutionChanged,zoomSettingUpdated,activeInputChanged");
serviceEvents.set("org.openrdk.StoragemanagerService"       , "onDeviceStatusChanged,onTSBError,onHealthWarning,onDeviceFailure");
serviceEvents.set("screenCaptureService"                    , "uploadComplete");
serviceEvents.set("Warehouse"                               , "deviceInfoRetrieved,resetDone");
serviceEvents.set("com.comcast.hdmiCec_1"                   , "onDevicesChanged,onMessage,onHdmiHotPlug,cecAddressesChanged");
serviceEvents.set("WebSocketService"                        , "onOpen,onError,onClose,onMessage");
serviceEvents.set("com.comcast.HDCPProfile"                 , "onDisplayConnectionChanged");


var defaultServiceParams = new Map();
defaultServiceParams.set("org.rdk.ping_1:ping", "['1-2-3-4', 'www.google.com']");

px.import({ scene: 'px:scene.1.js',
             keys: 'px:tools.keys.js'
             }).then( function importsAreReady(imports)
{
    var scene = imports.scene;
    var root  = imports.scene.root;
    var keys  = imports.keys;

    var currentService = "";
    var currentServiceIdx = 0;
    var currentMethod = "";
    var currentMethodIdx = 0;
    var currentParams = "";

    var currentAPIButton = 0;

    var selectedButton = 0;
    var selectedButtons = null;

    var mainButtons = [
        {   name : "API",
            rect: null,
            text: null,
            onClicked: function(obj) {


            },

            onSelected: function(obj) {
                refreshEdit();

            },

            onScroll: function(obj, increment) {
                currentServiceIdx = currentServiceIdx + increment;

                refreshEdit();
            }
        }
        ,
    {   name : "METHOD",
        rect: null,
        text: null,
        onClicked: function(obj) {


        },

        onSelected: function(obj) {
             refreshEdit();
        },

        onScroll: function(obj, increment) {
            currentMethodIdx = currentMethodIdx + increment;

            refreshEdit();
        }

    }
    ,
    {   name : "PARAMS",
        rect: null,
        text: null,
        onClicked: function(obj) {
                  edit();

        },

        onSelected: function(obj) {
             refreshEdit();
        }
    }
    ,
    {   name : "INIT",
        rect: null,
        text: null,
        onClicked: function(obj) {

                  try
                  {
                      var srvParts = currentService.split("/");
                      var serviceName = srvParts[0];
                      var version = srvParts[1];

                      service = scene.getService(serviceName);
                      smlog.text += "Created " + serviceName + "\n";

                      if (version !== undefined)
                          service.setApiVersionNumber(version);
                      else
                          service.setApiVersionNumber(1);

                      var events = serviceEvents.get(serviceName);
                      if (events !== undefined)
                      {
                          var eventsList = events.split(",");
                          var eventsQuoted = "";
                          var first = true;

                          for (var n = 0; n < eventsList.length; n++ )
                          {
                              if (first === false)
                                  eventsQuoted += ",";

                              eventsQuoted += "\"";
                              eventsQuoted += eventsList[n];
                              eventsQuoted += "\"";

                              if (first === true)
                                  first = false;
                          }

                          service.registerForEvents("{\"events\": [" + eventsQuoted + "]}");
                          service.onEvent = serviceEventHandler;
                      }

                  } catch (e)
                  {
                      smlog.text += "Failed to create " + currentService + "\n";
                  }
        }

    }
    ,
    {   name : "CALL",
        rect: null,
        text: null,
        onClicked: function(obj) {

                  try
                  {
                      // Can't enter "" on STB
                      currentParams = currentParams.replace(/'/g, "\"");

                      var result = service.callMethod(currentMethod, "{\"params\":" + currentParams + "}");

                      smlog.text += "Called  " + currentMethod + ", params: '" + currentParams +  "', result: '" + result + "'\n";
                  } catch (e)
                  {
                      smlog.text += "Failed to call " + currentMethod + "\n";
                  }
        }

    }
    ,
    {   name : "Info",
        rect: null,
        text: null,
        onClicked: function(obj) {

                  var name;
                  var versionNumber;
                  var sharedObjectsList;
                  var quirks;

                  try
                  {
                      name = service.getName();
                  }
                  catch (e)
                  {
                      name = "Failed to get Service Name";
                  }

                  try
                  {
                      versionNumber = service.getApiVersionNumber();
                  }
                  catch (e)
                  {
                      versionNumber = "Failed to get Service Name";
                  }

                  try
                  {
                      sharedObjectsList = service.getSharedObjectsList();
                      var parsedObj = JSON.parse(sharedObjectsList);

                      //var parsedObj = JSON.parse("[{\"list\":\"val1\"}]");
                      //var parsedObj = JSON.parse("{\"list\":[\"val2\",\"val3\"]}");
                      //smlog.text += "parsedObj.length = " + parsedObj.length + "\n";
                      //smlog.text += "parsedObj.list = " + parsedObj[0].list + "\n";
                      //smlog.text += "parsedObj.list = " + parsedObj.list + "\n";
                      sharedObjectsList = parsedObj.list;
                  }
                  catch (e)
                  {
                      sharedObjectsList = "Failed to get Shared Objects List";
                  }

                  try
                  {
                      quirks = service.getQuirks();
                      var parsedObj = JSON.parse(quirks);
                      quirks = parsedObj.list;

                  }
                  catch (e)
                  {
                      quirks = "Failed to get Quirks";
                  }

                  infoText.text = "name: " + name + "\n";
                  infoText.text += "versionNumber: " + versionNumber + "\n";
                  infoText.text += "sharedObjectsList: " + sharedObjectsList + "\n";
                  infoText.text += "quirks: " + quirks + "\n";

                  info.a = info.a == 0 ? 1 : 0;
        }
    }
    ,
    {   name : "Clear log",
        rect: null,
        text: null,
        onClicked: function(obj) {
                  smlog.text = "";
        }
    }
    ,
    {   name : "HELP",
        rect: null,
        text: null,
        onClicked: function(obj) {
                  help.a = help.a == 0 ? 1 : 0;
        }

    }
    ]

    var pts = 12;

    var regColor = 0x7f7f7fff;
    var selectedColor = 0xffffffff;

    var fontRes = scene.create({t:"fontResource", url:"FreeSans.ttf"});

    var header = scene.create({t:"text", text:"Service Manager test.", font:fontRes, parent:root, pixelSize:18, textColor:0xffffffff, x:10, y:10});

    var buttonsView = scene.create({t:"object", x:10, y:50, w: 900, h: 30, parent:root});
    var buttonsRect = scene.create({t:"rect", x:0, y:0, w:buttonsView.w, h:buttonsView.h, parent:buttonsView, fillColor:0x5555557f});

    var smButtonsView = scene.create({t:"object", x:10, y:50, w: 800, h: 30, parent:root, a:0});
    var smButtonsRect = scene.create({t:"rect", x:0, y:0, w:smButtonsView.w, h:smButtonsView.h, parent:smButtonsView, fillColor:0x5555557f});

    var editView = scene.create({t:"object", x:buttonsView.x, y:buttonsView.y + buttonsView.h + 10, w: buttonsView.w, h: 30, parent:root});
    var editRect = scene.create({t:"rect", x:0, y:0, w:editView.w, h:editView.h, parent:editView, fillColor:0x5555557f});
    var editText = scene.create({t:"text",text:"", font:fontRes, parent:editRect,pixelSize:pts, textColor:0xffffffff, x:5, y:5});

    var cursor = scene.create({t:"rect", a:0, x:5, y:5, w:2, h:editView.h - 10, parent:editView, fillColor:0xffffffff});

    var smlog = scene.create({t:"text",text:"", font:fontRes, parent:root, pixelSize:pts, textColor:0xffffffff, x:10, y:150});

    var help = scene.create({t:"object", x:buttonsView.x, y:buttonsView.y + buttonsView.h + 10, w: buttonsView.w, h: 100, parent:root, a:0});
    var helpRect = scene.create({t:"rect", x:0, y:0, w:help.w, h:help.h, parent:help, fillColor:0x7f7f7fff});
    var helpText = scene.create({t:"text",text:"", font:fontRes, parent:helpRect, pixelSize:pts, textColor:0xff, x:10, y:10});

    helpText.text = "Press UP or DOWN keys on API or METHOD button to select Service or Method.\n" +
        "Press ENTER on API, METHOD or PARAMS to edit value.\n" +
        "Press INIT to init selected Service.\n" +
        "Press CALL to call selected Method of the Service.\n";

    var info = scene.create({t:"object", x:buttonsView.x, y:buttonsView.y + buttonsView.h + 10, w: buttonsView.w, h: 100, parent:root, a:0});
    var infoRect = scene.create({t:"rect", x:0, y:0, w:help.w, h:help.h, parent:info, fillColor:0x7f7f7fff});
    var infoText = scene.create({t:"text",text:"", font:fontRes, parent:infoRect, pixelSize:pts, textColor:0xff, x:10, y:10});

    var service = null;

    buttonsView.on("onKeyDown", butttonsKeyHandler);
    smButtonsView.on("onKeyDown", butttonsKeyHandler);

    function butttonsKeyHandler(e)
    {
        var code = e.keyCode;
        var flags = e.flags;

        switch (code)
        {
            case keys.LEFT:
                selectedButton--;
                if (selectedButton < 0)
                    selectedButton = selectedButtons.length - 1;

                if (selectedButtons[selectedButton].onSelected !== undefined)
                    selectedButtons[selectedButton].onSelected(selectedButtons[selectedButton]);

                help.a = 0;

                break;
            case keys.RIGHT:
                selectedButton++;
                if (selectedButton >= selectedButtons.length)
                    selectedButton = 0;

                if (selectedButtons[selectedButton].onSelected !== undefined)
                    selectedButtons[selectedButton].onSelected(selectedButtons[selectedButton]);

                help.a = 0;

                break;

            case keys.UP:
                if (selectedButtons[selectedButton].onScroll !== undefined)
                    selectedButtons[selectedButton].onScroll(selectedButtons[selectedButton], -1);

                break;

            case keys.DOWN:
                if (selectedButtons[selectedButton].onScroll !== undefined)
                    selectedButtons[selectedButton].onScroll(selectedButtons[selectedButton], 1);

                break;

            case keys.ENTER:
                selectedButtons[selectedButton].onClicked(selectedButtons[selectedButton]);
                break;

            default:
                break;
        }

        refreshButtons(selectedButtons);
    }


    editView.on("onChar", function (e)
    {
        editText.text = editText.text + String.fromCharCode(e.charCode);
        cursor.x = editText.x + fontRes.measureText(pts, editText.text).w;
    });

    editView.on("onKeyDown", function(e)
    {
        var code = e.keyCode;
        var flags = e.flags;

        switch (code) {
            case keys.BACKSPACE:
                editText.text = editText.text.slice(0, -1);
                cursor.x = editText.x + fontRes.measureText(pts, editText.text).w;
                break;

            case keys.ENTER:

                if (mainButtons[selectedButton].name == "API")
                    currentService = editText.text;

                else if (mainButtons[selectedButton].name == "METHOD")
                    currentMethod = editText.text;

                else if (mainButtons[selectedButton].name == "PARAMS")
                    currentParams = editText.text;

                editView.focus = false;
                buttonsView.focus = true;
                cursor.a = 0;
                break;

            default:
                break;
        }
    });

    function edit()
    {
        buttonsView.focus = false;
        editView.focus = true;

        cursor.x = editText.x + fontRes.measureText(pts, editText.text).w;
        cursor.a = 1;
    }

    function updateService()
    {
        if (currentServiceIdx < 0 )
            currentServiceIdx = Services.length - 1;
        else if (currentServiceIdx >=  Services.length)
            currentServiceIdx = 0;

        var srvLine = Services[currentServiceIdx];
        var parts = srvLine.split("=");

        var srv = parts[0];
        currentService = srv;
        editText.text = currentService;

        currentMethodIdx = 0;
        currentMethod = parts[1].split(",")[currentMethodIdx];
    }

    function updateMethod()
    {
        var srvLine = Services[currentServiceIdx];
        var methods = srvLine.split("=")[1].split(",");

        if (currentMethodIdx < 0)
            currentMethodIdx = methods.length - 1;
        else if (currentMethodIdx >= methods.length)
            currentMethodIdx = 0;

        currentMethod = methods[currentMethodIdx];
        editText.text = currentMethod;
    }

    function refreshEdit()
    {
        if (selectedButtons === mainButtons)
        {
            var oldService = currentService;
            var oldMethod = currentMethod;

            if (mainButtons[selectedButton].name == "API")
            {
                updateService();
            }
            else if (mainButtons[selectedButton].name == "METHOD")
            {
                updateMethod();
            }
            else if (mainButtons[selectedButton].name == "PARAMS")
            {
                editText.text = currentParams;
            }
            else
                editText.text = "";

            if (oldService !== currentService || oldMethod !== currentMethod)
            {
                var defaultParams = defaultServiceParams.get(currentService.split("/")[0] + ":" + currentMethod);

                if (defaultParams !== undefined)
                    currentParams = defaultParams;
                else
                    currentParams = "";
            }
        }

    }

    function createButtons(view, buttons, width)
    {
        currentX = 10;

        for (var n = 0; n < buttons.length; n++ )
        {
            buttons[n].rect  = scene.create({t:"rect", x:currentX, y:5, w:width - 20, h:view.h - 10, parent:view, fillColor:regColor});
            buttons[n].text  = scene.create({t:"text", text:buttons[n].name, font:fontRes, parent:buttons[n].rect, pixelSize:pts, textColor:0x000000ff, x: (buttons[n].rect.w - fontRes.measureText(pts, buttons[n].name).w) / 2, y:0});

            currentX += width;
        }
    }

    function refreshButtons(buttons)
    {
        for (var n = 0; n < buttons.length; n++ )
        {
            if (selectedButton == n)
                buttons[n].rect.fillColor = selectedColor;
            else
                buttons[n].rect.fillColor = regColor;
        }

    }

    buttonsView.focus = true;
    selectedButtons = mainButtons;

    createButtons(buttonsView, mainButtons, 100);

    refreshButtons(mainButtons);
    refreshEdit();

    function serviceEventHandler(event, result)
    {
        smlog.text += "Got " + event + " event from " + currentService + "." + currentMethod + " call, result: '" + result + "'\n";
    }


}).catch( function importFailed(err){
    console.error("Import failed for test_servicemanager.js: " + err)
});

