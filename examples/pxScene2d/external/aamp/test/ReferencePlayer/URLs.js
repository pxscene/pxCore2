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

var urls = [
     { name:"AD1 (DASH/CLEAR)", url: "http://cdn-ec-pan-02.ip-ads.xcr.comcast.net/omg05/UNI_Packaging_-_Production/453393990093/5a60d158-b057-4636-92ac-0d2ec324dd87/263/209/CSNR8803918400100001_mezz_LVLH07.mpd", useDefaultDrmConfig: false },
     { name:"AD2 (DASH/CLEAR)", url: "http://ccr.ip-ads.xcr.comcast.net/omg04/UNI_Packaging_-_Production/470766150395/735f1f50-39a0-41f5-bb2a-a0dc5f3b3552/446/601/CSNE8736698000090001_mezz_LVLH07.mpd", useDefaultDrmConfig: false },
     { name:"AD3 (DASH/CLEAR)", url: "http://ccr.ip-ads.xcr.comcast.net/omg08/UNI_Packaging_-_Production/316269638415/6563a411-908b-4abd-b3ae-23dc910fd136/563/237/CSNF8700103700100001_mezz_LVLH07.mpd", useDefaultDrmConfig: false },
     { name:"AD4 (DASH/CLEAR)", url: "http://ccr.ip-ads.xcr.comcast.net/omg05/UNI_Packaging_-_Production/450921542127/e5c6fac0-74a4-4301-807b-4ecdca384d86/977/301/CSAF8000010270110001_mezz_LVLH07.mpd", useDefaultDrmConfig: false },
     { name:"AD5 (DASH/CLEAR)", url: "http://ccr.ip-ads.xcr.comcast.net/omg04/UNI_Packaging_-_Production/436790854077/6a7baede-0129-4998-b7a2-807f725bb837/813/225/CSNE8674797000090001_mezz_LVLH07.mpd", useDefaultDrmConfig: false },
     { name:"AD6 (DASH/CLEAR)", url: "http://ccr.ip-ads.xcr.comcast.net/omg07/UNI_Packaging_-_Production/444469830341/b6613718-4eeb-4f60-a9f0-bb13996ae4a6/968/473/CSNE8692156000090001_mezz_LVLH07.mpd", useDefaultDrmConfig: false },
     { name:"Big Buck Bunny (CLEAR)", url: "http://amssamples.streaming.mediaservices.windows.net/683f7e47-bd83-4427-b0a3-26a6c4547782/BigBuckBunny.ism/manifest(format=mpd-time-csf)", useDefaultDrmConfig: false },
     { name:"Sintel MPD", url: "http://amssamples.streaming.mediaservices.windows.net/d444b1c9-c64a-4d65-b0f4-433810cf5e92/SintelMultiAudio.ism/manifest(format=mpd-time-csf)", useDefaultDrmConfig: false },
     { name:"AD (DASH)", url: "http://ccr.ip-ads.xcr.comcast.net/omg07/346241094255/nbcuni.comNBCU2019010200010506/HD_VOD_DAI_QAOA5052100H_0102_LVLH06.mpd", useComcastDrmConfig: true },
     { name:"HLN_MIMIC (SCTE35)", url: "http://172.27.223.96:8880/HLN_MIMIC.mpd", useDefaultDrmConfig: true },
     { name:"TRUTV_MIMIC (SCTE35)", url: "http://172.27.223.96:8880/TRUTV_MIMIC.mpd", useDefaultDrmConfig: true },
     { name:"CNN (LIVE)", url: "http://cdn-ec-pan-03.linear-nat-pil.xcr.comcast.net/CNNHD_HD_NAT_16141_0_5646493630829879163.mpd", useDefaultDrmConfig: true },
     { name:"Transformers (DASH)", url: "http://ccr.ipvod-t6.xcr.comcast.net/ipvod11/TVNC0000000003007630/movie/1558999503868/manifest.mpd", useDefaultDrmConfig: true },
     { name:"Deadpool (HLS)", url: "http://ccr.ipvod-t6.xcr.comcast.net/ipvod4/PFXM3033039820190601/movie/1558895446379/manifest.m3u8", useDefaultDrmConfig: true }
];
