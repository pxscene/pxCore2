// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxColor.h

#ifndef PX_COLOR_H
#define PX_COLOR_H

#include "pxPixel.h"

const pxColor pxClear (0, 0, 0, 0); // RGBA

#if 0

const pxColor pxWhite (255, 255, 255);
const pxColor pxBlack (  0,   0,   0);
const pxColor pxRed   (255,   0,   0);
const pxColor pxGreen (  0, 255,   0);
const pxColor pxBlue  (  0,   0, 255);
const pxColor pxGray  (128, 128, 128);

#else

const pxColor pxPink	                (255, 192, 203);
const pxColor pxLightPink	            (255, 182, 193);
const pxColor pxHotPink	                (255, 105, 180);
const pxColor pxDeepPink	            (255,  20, 147);
const pxColor pxPaleVioletRed	        (219, 112, 147);
const pxColor pxMediumVioletRed	        (199,  21, 133);
const pxColor pxLightSalmon	            (255, 160, 122);
const pxColor pxSalmon	                (250, 128, 114);
const pxColor pxDarkSalmon	            (233, 150, 122);
const pxColor pxLightCoral	            (240, 128, 128);
const pxColor pxIndianRed	            (205,  92,  92);
const pxColor pxCrimson                 (220,  20,  60);
const pxColor pxFireBrick	            (178,  34,  34);
const pxColor pxDarkRed	                (139,   0,   0);
const pxColor pxRed	                    (255,   0,   0);
const pxColor pxOrangeRed	            (255,  69,   0);
const pxColor pxTomato      	        (255,  99,  71);
const pxColor pxCoral	                (255, 127,  80);
const pxColor pxDarkOrange	            (255, 140,   0);
const pxColor pxOrange	                (255, 165,   0);
const pxColor pxYellow	                (255, 255,   0);
const pxColor pxLightYellow	            (255, 255, 224);
const pxColor pxLemonChiffon	        (255, 250, 205);
const pxColor pxLightGoldenrodYellow	(250, 250, 210);
const pxColor pxPapayaWhip	            (255, 239, 213);
const pxColor pxMoccasin	            (255, 228, 181);
const pxColor pxPeachPuff	            (255, 218, 185);
const pxColor pxPaleGoldenrod	        (238, 232, 170);
const pxColor pxKhaki	                (240, 230, 140);
const pxColor pxDarkKhaki	            (189, 183, 107);
const pxColor pxGold	                (255, 215,   0);
const pxColor pxCornsilk	            (255, 248, 220);
const pxColor pxBlanchedAlmond	        (255, 235, 205);
const pxColor pxBisque	                (255, 228, 196);
const pxColor pxNavajoWhite	            (255, 222, 173);
const pxColor pxWheat	                (245, 222, 179);
const pxColor pxBurlyWood	            (222, 184, 135);
const pxColor pxTan	                    (210, 180, 140);
const pxColor pxRosyBrown	            (188, 143, 143);
const pxColor pxSandyBrown	            (244, 164,  96);
const pxColor pxGoldenrod	            (218, 165,  32);
const pxColor pxDarkGoldenrod	        (184, 134,  11);
const pxColor pxPeru	                (205, 133,  63);
const pxColor pxChocolate	            (210, 105,  30);
const pxColor pxSaddleBrown         	(139,  69,  19);
const pxColor pxSienna	                (160,  82,  45);
const pxColor pxBrown	                (165,  42,  42);
const pxColor pxMaroon	                (128,   0,   0);
const pxColor pxDarkOliveGreen	        ( 85, 107,  47);
const pxColor pxOlive	                (128, 128,   0);
const pxColor pxOliveDrab	            (107, 142,  35);
const pxColor pxYellowGreen	            (154, 205,  50);
const pxColor pxLimeGreen	            ( 50, 205,  50);
const pxColor pxLime	                (  0, 255,   0);
const pxColor pxLawnGreen	            (124, 252,   0);
const pxColor pxChartreuse	            (127, 255,   0);
const pxColor pxGreenYellow	            (173, 255,  47);
const pxColor pxSpringGreen	            (  0, 255, 127);
const pxColor pxMediumSpringGreen	    (  0, 250, 154);
const pxColor pxLightGreen	            (144, 238, 144);
const pxColor pxPaleGreen	            (152, 251, 152);
const pxColor pxDarkSeaGreen	        (143, 188, 143);
const pxColor pxMediumSeaGreen	        ( 60, 179, 113);
const pxColor pxSeaGreen	            ( 46, 139,  87);
const pxColor pxForestGreen	            ( 34, 139,  34);
const pxColor pxGreen	                (  0, 128,   0);
const pxColor pxDarkGreen	            (  0, 100,   0);
const pxColor pxMediumAquamarine	    (102, 205, 170);
const pxColor pxAqua	                (  0, 255, 255);
const pxColor pxCyan	                (  0, 255, 255);
const pxColor pxLightCyan	            (224, 255, 255);
const pxColor pxPale_turquoise	        (175, 238, 238);
const pxColor pxAquamarine	            (127, 255, 212);
const pxColor pxTurquoise	            ( 64, 224, 208);
const pxColor pxMediumTurquoise	        ( 72, 209, 204);
const pxColor pxDark_turquoise	        (  0, 206, 209);
const pxColor pxLightSeaGreen	        ( 32, 178, 170);
const pxColor pxCadetBlue	            ( 95, 158, 160);
const pxColor pxDarkCyan	            (  0, 139, 139);
const pxColor pxTeal	                (  0, 128, 128);
const pxColor pxLightSteelBlue	        (176, 196, 222);
const pxColor pxPowderBlue	            (176, 224, 230);
const pxColor pxLightBlue	            (173, 216, 230);
const pxColor pxSkyBlue	                (135, 206, 235);
const pxColor pxLightSkyBlue	        (135, 206, 250);
const pxColor pxDeepSkyBlue         	(  0, 191, 255);
const pxColor pxDodgerBlue	            ( 30, 144, 255);
const pxColor pxCornflowerBlue	        (100, 149, 237);
const pxColor pxSteelBlue	            ( 70, 130, 180);
const pxColor pxRoyalBlue	            ( 65, 105, 225);
const pxColor pxBlue	                (  0,   0, 255);
const pxColor pxMediumBlue	            (  0,   0, 205);
const pxColor pxDarkBlue	            (  0,   0, 139);
const pxColor pxNavy	                (  0,   0, 128);
const pxColor pxMidnightBlue	        ( 25,  25, 112);
const pxColor pxLavender	            (230, 230, 250);
const pxColor pxThistle	                (216, 191, 216);
const pxColor pxPlum	                (221, 160, 221);
const pxColor pxViolet	                (238, 130, 238);
const pxColor pxOrchid	                (218, 112, 214);
const pxColor pxFuchsia	                (255,   0, 255);
const pxColor pxMagenta	                (255,   0, 255);
const pxColor pxMediumOrchid	        (186,  85, 211);
const pxColor pxMediumPurple	        (147, 112, 219);
const pxColor pxBlueViolet	            (138,  43, 226);
const pxColor pxDarkViolet	            (148,   0, 211);
const pxColor pxDarkOrchid	            (153,  50, 204);
const pxColor pxDark_magenta	        (139,   0, 139);
const pxColor pxPurple	                (128,   0, 128);
const pxColor pxIndigo	                ( 75,   0, 130);
const pxColor pxDarkSlateBlue	        ( 72,  61, 139);
const pxColor pxSlateBlue	            (106,  90, 205);
const pxColor pxMediumSlateBlue	        (123, 104, 238);
const pxColor pxWhite	                (255, 255, 255);
const pxColor pxSnow	                (255, 250, 250);
const pxColor pxHoneydew	            (240, 255, 240);
const pxColor pxMintCream	            (245, 255, 250);
const pxColor pxAzure	                (240, 255, 255);
const pxColor pxAliceBlue	            (240, 248, 255);
const pxColor pxGhostWhite	            (248, 248, 255);
const pxColor pxWhiteSmoke	            (245, 245, 245);
const pxColor pxSeashell	            (255, 245, 238);
const pxColor pxBeige	                (245, 245, 220);
const pxColor pxOld_lace	            (253, 245, 230);
const pxColor pxFloralWhite	            (255, 250, 240);
const pxColor pxIvory	                (255, 255, 240);
const pxColor pxAntiqueWhite	        (250, 235, 215);
const pxColor pxLinen	                (250, 240, 230);
const pxColor pxLavenderBlush	        (255, 240, 245);
const pxColor pxMisty_rose	            (255, 228, 225);
const pxColor pxGainsboro	            (220, 220, 220);
const pxColor pxLightGrey	            (211, 211, 211);
const pxColor pxSilver	                (192, 192, 192);
const pxColor pxDarkGray	            (169, 169, 169);
const pxColor pxGray	                (128, 128, 128);
const pxColor pxDimGray	                (105, 105, 105);
const pxColor pxLightSlateGray	        (119, 136, 153);
const pxColor pxSlateGray	            (112, 128, 144);
const pxColor pxDarkSlateGray	        ( 47,  79,  79);
const pxColor pxBlack	                (  0,   0,   0);

#endif // 0

#endif // PX_COLOR_H