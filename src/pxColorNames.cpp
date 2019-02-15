/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxColorNames.cpp


#include <map>
#include <string.h> // for strlen()

#include "pxColorNames.h"


//////////////////////////////////////////////////////////////////////
//
//   NOTE:  Using VSCode + "Color Highlight" plugin will allow
//          the #RRGGBB color values below to be visualized.
//
//////////////////////////////////////////////////////////////////////

static std::map<rtString, uint32_t> colorNames =
{
  {"indianred",             0xCD5C5Cff },  // #CD5C5C   // "IndianRed",
  {"lightcoral",            0xF08080ff },  // #F08080   // "LightCoral",
  {"salmon",                0xFA8072ff },  // #FA8072   // "Salmon",
  {"darksalmon",            0xE9967Aff },  // #E9967A   // "DarkSalmon",
  {"lightsalmon",           0xFFA07Aff },  // #FFA07A   // "LightSalmon",
  {"crimson",               0xDC143Cff },  // #DC143C   // "Crimson",
  {"red",                   0xFF0000ff },  // #FF0000   // "Red",
  {"firebrick",             0xB22222ff },  // #B22222   // "FireBrick",
  {"darkred",               0x8B0000ff },  // #8B0000   // "DarkRed",
  {"pink",                  0xFFC0CBff },  // #FFC0CB   // "Pink",
  {"lightpink",             0xFFB6C1ff },  // #FFB6C1   // "LightPink",
  {"hotpink",               0xFF69B4ff },  // #FF69B4   // "HotPink",
  {"deeppink",              0xFF1493ff },  // #FF1493   // "DeepPink",
  {"mediumvioletred",       0xC71585ff },  // #C71585   // "MediumVioletRed",
  {"palevioletred",         0xDB7093ff },  // #DB7093   // "PaleVioletRed",
  {"coral",                 0xFF7F50ff },  // #FF7F50   // "Coral",
  {"tomato",                0xFF6347ff },  // #FF6347   // "Tomato",
  {"orangered",             0xFF4500ff },  // #FF4500   // "OrangeRed",
  {"darkorange",            0xFF8C00ff },  // #FF8C00   // "DarkOrange",
  {"orange",                0xFFA500ff },  // #FFA500   // "Orange",
  {"gold",                  0xFFD700ff },  // #FFD700   // "Gold",
  {"yellow",                0xFFFF00ff },  // #FFFF00   // "Yellow",
  {"lightyellow",           0xFFFFE0ff },  // #FFFFE0   // "LightYellow",
  {"lemonchiffon",          0xFFFACDff },  // #FFFACD   // "LemonChiffon",
  {"lightgoldenrodyellow",  0xFAFAD2ff },  // #FAFAD2   // "LightGoldenrodYellow",
  {"papayawhip",            0xFFEFD5ff },  // #FFEFD5   // "PapayaWhip",
  {"moccasin",              0xFFE4B5ff },  // #FFE4B5   // "Moccasin",
  {"peachpuff",             0xFFDAB9ff },  // #FFDAB9   // "PeachPuff",
  {"palegoldenrod",         0xEEE8AAff },  // #EEE8AA   // "PaleGoldenrod",
  {"khaki",                 0xF0E68Cff },  // #F0E68C   // "Khaki",
  {"darkkhaki",             0xBDB76Bff },  // #BDB76B   // "DarkKhaki",
  {"lavender",              0xE6E6FAff },  // #E6E6FA   // "Lavender",
  {"thistle",               0xD8BFD8ff },  // #D8BFD8   // "Thistle",
  {"plum",                  0xDDA0DDff },  // #DDA0DD   // "Plum",
  {"violet",                0xEE82EEff },  // #EE82EE   // "Violet",
  {"orchid",                0xDA70D6ff },  // #DA70D6   // "Orchid",
  {"fuchsia",               0xFF00FFff },  // #FF00FF   // "Fuchsia",
  {"magenta",               0xFF00FFff },  // #FF00FF   // "Magenta",
  {"mediumorchid",          0xBA55D3ff },  // #BA55D3   // "MediumOrchid",
  {"mediumpurple",          0x9370DBff },  // #9370DB   // "MediumPurple",
  {"blueviolet",            0x8A2BE2ff },  // #8A2BE2   // "BlueViolet",
  {"darkviolet",            0x9400D3ff },  // #9400D3   // "DarkViolet",
  {"darkorchid",            0x9932CCff },  // #9932CC   // "DarkOrchid",
  {"darkmagenta",           0x8B008Bff },  // #8B008B   // "DarkMagenta",
  {"purple",                0x800080ff },  // #800080   // "Purple",
  {"rebeccapurple",         0x663399ff },  // #663399   // "RebeccaPurple",
  {"indigo",                0x4B0082ff },  // #4B0082   // "Indigo",
  {"mediumslateblue",       0x7B68EEff },  // #7B68EE   // "MediumSlateBlue",
  {"slateblue",             0x6A5ACDff },  // #6A5ACD   // "SlateBlue",
  {"darkslateblue",         0x483D8Bff },  // #483D8B   // "DarkSlateBlue",
  {"greenyellow",           0xADFF2Fff },  // #ADFF2F   // "GreenYellow",
  {"chartreuse",            0x7FFF00ff },  // #7FFF00   // "Chartreuse",
  {"lawngreen",             0x7CFC00ff },  // #7CFC00   // "LawnGreen",
  {"lime",                  0x00FF00ff },  // #00FF00   // "Lime",
  {"limegreen",             0x32CD32ff },  // #32CD32   // "LimeGreen",
  {"palegreen",             0x98FB98ff },  // #98FB98   // "PaleGreen",
  {"lightgreen",            0x90EE90ff },  // #90EE90   // "LightGreen",
  {"mediumspringgreen",     0x00FA9Aff },  // #00FA9A   // "MediumSpringGreen",
  {"springgreen",           0x00FF7Fff },  // #00FF7F   // "SpringGreen",
  {"mediumseagreen",        0x3CB371ff },  // #3CB371   // "MediumSeaGreen",
  {"seagreen",              0x2E8B57ff },  // #2E8B57   // "SeaGreen",
  {"forestgreen",           0x228B22ff },  // #228B22   // "ForestGreen",
  {"green",                 0x008000ff },  // #008000   // "Green",
  {"darkgreen",             0x006400ff },  // #006400   // "DarkGreen",
  {"yellowgreen",           0x9ACD32ff },  // #9ACD32   // "YellowGreen",
  {"olivedrab",             0x6B8E23ff },  // #6B8E23   // "OliveDrab",
  {"olive",                 0x808000ff },  // #808000   // "Olive",
  {"darkolivegreen",        0x556B2Fff },  // #556B2F   // "DarkOliveGreen",
  {"mediumaquamarine",      0x66CDAAff },  // #66CDAA   // "MediumAquamarine",
  {"darkseagreen",          0x8FBC8Fff },  // #8FBC8F   // "DarkSeaGreen",
  {"lightseagreen",         0x20B2AAff },  // #20B2AA   // "LightSeaGreen",
  {"darkcyan",              0x008B8Bff },  // #008B8B   // "DarkCyan",
  {"teal",                  0x008080ff },  // #008080   // "Teal",
  {"aqua",                  0x00FFFFff },  // #00FFFF   // "Aqua",
  {"cyan",                  0x00FFFFff },  // #00FFFF   // "Cyan",
  {"lightcyan",             0xE0FFFFff },  // #E0FFFF   // "LightCyan",
  {"paleturquoise",         0xAFEEEEff },  // #AFEEEE   // "PaleTurquoise",
  {"aquamarine",            0x7FFFD4ff },  // #7FFFD4   // "Aquamarine",
  {"turquoise",             0x40E0D0ff },  // #40E0D0   // "Turquoise",
  {"mediumturquoise",       0x48D1CCff },  // #48D1CC   // "MediumTurquoise",
  {"darkturquoise",         0x00CED1ff },  // #00CED1   // "DarkTurquoise",
  {"cadetblue",             0x5F9EA0ff },  // #5F9EA0   // "CadetBlue",
  {"steelblue",             0x4682B4ff },  // #4682B4   // "SteelBlue",
  {"lightsteelblue",        0xB0C4DEff },  // #B0C4DE   // "LightSteelBlue",
  {"powderblue",            0xB0E0E6ff },  // #B0E0E6   // "PowderBlue",
  {"lightblue",             0xADD8E6ff },  // #ADD8E6   // "LightBlue",
  {"skyblue",               0x87CEEBff },  // #87CEEB   // "SkyBlue",
  {"lightskyblue",          0x87CEFAff },  // #87CEFA   // "LightSkyBlue",
  {"deepskyblue",           0x00BFFFff },  // #00BFFF   // "DeepSkyBlue",
  {"dodgerblue",            0x1E90FFff },  // #1E90FF   // "DodgerBlue",
  {"cornflowerblue",        0x6495EDff },  // #6495ED   // "CornflowerBlue",
  {"royalblue",             0x4169E1ff },  // #4169E1   // "RoyalBlue",
  {"blue",                  0x0000FFff },  // #0000FF   // "Blue",
  {"mediumblue",            0x0000CDff },  // #0000CD   // "MediumBlue",
  {"darkblue",              0x00008Bff },  // #00008B   // "DarkBlue",
  {"navy",                  0x000080ff },  // #000080   // "Navy",
  {"midnightblue",          0x191970ff },  // #191970   // "MidnightBlue",
  {"cornsilk",              0xFFF8DCff },  // #FFF8DC   // "Cornsilk",
  {"blanchedalmond",        0xFFEBCDff },  // #FFEBCD   // "BlanchedAlmond",
  {"bisque",                0xFFE4C4ff },  // #FFE4C4   // "Bisque",
  {"navajowhite",           0xFFDEADff },  // #FFDEAD   // "NavajoWhite",
  {"wheat",                 0xF5DEB3ff },  // #F5DEB3   // "Wheat",
  {"burlywood",             0xDEB887ff },  // #DEB887   // "BurlyWood",
  {"tan",                   0xD2B48Cff },  // #D2B48C   // "Tan",
  {"rosybrown",             0xBC8F8Fff },  // #BC8F8F   // "RosyBrown",
  {"sandybrown",            0xF4A460ff },  // #F4A460   // "SandyBrown",
  {"goldenrod",             0xDAA520ff },  // #DAA520   // "Goldenrod",
  {"darkgoldenrod",         0xB8860Bff },  // #B8860B   // "DarkGoldenrod",
  {"peru",                  0xCD853Fff },  // #CD853F   // "Peru",
  {"chocolate",             0xD2691Eff },  // #D2691E   // "Chocolate",
  {"saddlebrown",           0x8B4513ff },  // #8B4513   // "SaddleBrown",
  {"sienna",                0xA0522Dff },  // #A0522D   // "Sienna",
  {"brown",                 0xA52A2Aff },  // #A52A2A   // "Brown",
  {"maroon",                0x800000ff },  // #800000   // "Maroon",
  {"white",                 0xFFFFFFff },  // #FFFFFF   // "White",
  {"snow",                  0xFFFAFAff },  // #FFFAFA   // "Snow",
  {"honeydew",              0xF0FFF0ff },  // #F0FFF0   // "Honeydew",
  {"mintcream",             0xF5FFFAff },  // #F5FFFA   // "MintCream",
  {"azure",                 0xF0FFFFff },  // #F0FFFF   // "Azure",
  {"aliceblue",             0xF0F8FFff },  // #F0F8FF   // "AliceBlue",
  {"ghostwhite",            0xF8F8FFff },  // #F8F8FF   // "GhostWhite",
  {"whitesmoke",            0xF5F5F5ff },  // #F5F5F5   // "WhiteSmoke",
  {"seashell",              0xFFF5EEff },  // #FFF5EE   // "Seashell",
  {"beige",                 0xF5F5DCff },  // #F5F5DC   // "Beige",
  {"oldlace",               0xFDF5E6ff },  // #FDF5E6   // "OldLace",
  {"floralwhite",           0xFFFAF0ff },  // #FFFAF0   // "FloralWhite",
  {"ivory",                 0xFFFFF0ff },  // #FFFFF0   // "Ivory",
  {"antiquewhite",          0xFAEBD7ff },  // #FAEBD7   // "AntiqueWhite",
  {"linen",                 0xFAF0E6ff },  // #FAF0E6   // "Linen",
  {"lavenderblush",         0xFFF0F5ff },  // #FFF0F5   // "LavenderBlush",
  {"mistyrose",             0xFFE4E1ff },  // #FFE4E1   // "MistyRose",
  {"gainsboro",             0xDCDCDCff },  // #DCDCDC   // "Gainsboro",
  {"lightgray",             0xD3D3D3ff },  // #D3D3D3   // "LightGray",
  {"lightgrey",             0xD3D3D3ff },  // #D3D3D3   // "LightGrey",
  {"silver",                0xC0C0C0ff },  // #C0C0C0   // "Silver",
  {"darkgray",              0xA9A9A9ff },  // #A9A9A9   // "DarkGray",
  {"darkgrey",              0xA9A9A9ff },  // #A9A9A9   // "DarkGrey",
  {"gray",                  0x808080ff },  // #808080   // "Gray",
  {"grey",                  0x808080ff },  // #808080   // "Grey",
  {"dimgray",               0x696969ff },  // #696969   // "DimGray",
  {"dimgrey",               0x696969ff },  // #696969   // "DimGrey",
  {"lightslategray",        0x778899ff },  // #778899   // "LightSlateGray",
  {"lightslategrey",        0x778899ff },  // #778899   // "LightSlateGrey",
  {"slategray",             0x708090ff },  // #708090   // "SlateGray",
  {"slategrey",             0x708090ff },  // #708090   // "SlateGrey",
  {"darkslategray",         0x2F4F4Fff },  // #2F4F4F   // "DarkSlateGray",
  {"darkslategrey",         0x2F4F4Fff },  // #2F4F4F   // "DarkSlateGrey",
  {"black",                 0x000000ff }   // #000000   // "Black",
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

rtError web2rgb(rtString &input, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a)
{
  rtError retVal = RT_OK;

  char clr6[9] = {0};
  unsigned int rr, gg, bb, aa;
  
  if (input.length() == 0)
  {
    return RT_FAIL; // FAIL
  }

  char* clr = (char *) input.cString();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Try Named Colors
  //
  if(clr[0] != '#')
  {
    input.toLowerAscii(); // in-place;

    uint32_t c = colorNames[ input ];
    
#ifdef PX_LITTLEENDIAN_PIXELS
    
    bb = (uint8_t)((c >> 16) & 0xff);  // B
    gg = (uint8_t)((c >>  8) & 0xff);  // G
    rr = (uint8_t)((c >>  0) & 0xff);  // R

#else
    
    rr = (uint8_t)((c >> 24) & 0xff);  // R
    gg = (uint8_t)((c >> 16) & 0xff);  // G
    bb = (uint8_t)((c >>  8) & 0xff);  // B
    
#endif
    
    r = rr;   g = gg;  b = bb;  a = 0xFF;

    return RT_OK;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  //  Set by WebColor   "#rgb" or "#RRGGBB"  strings
  //
  if(clr[0] != '#')
  {
    // rtLogDebug("web2rgb: %s  <<< Unknown Input \n", input);
    return RT_FAIL;
  }

  clr++; // skip '#'
  
  size_t len = strlen(clr);
  
  switch(len)
  {
    case 3:   //#RGB  >> #RRGGBBAA
    {
      clr6[0] = clr[0];  clr6[1] = clr[0];  // 01  RR
      clr6[2] = clr[1];  clr6[3] = clr[1];  // 23  GG
      clr6[4] = clr[2];  clr6[5] = clr[2];  // 45  BB
      clr6[6] = 'F';     clr6[7] = 'F';     // 67  AA

      clr = clr6; // point here now !
    }
    break;
      
    case 4:   //#RGBA  >> #RRGGBBAA
    {
      clr6[6] = clr[3];  clr6[7] = clr[3];  // 67  AA (Do First... will overwrite)
      clr6[0] = clr[0];  clr6[1] = clr[0];  // 01  RR
      clr6[2] = clr[1];  clr6[3] = clr[1];  // 23  GG
      clr6[4] = clr[2];  clr6[5] = clr[2];  // 45  BB
      
      clr = clr6; // point here now !
    }
    break;
      
    case 6:   //#RRGGBB  >> #RRGGBBAA
    {
      clr6[0] = clr[0];  clr6[1] = clr[1];  // 01  RR
      clr6[2] = clr[2];  clr6[3] = clr[3];  // 23  GG
      clr6[4] = clr[4];  clr6[5] = clr[5];  // 45  BB
      clr6[6] = 'F';     clr6[7] = 'F';     // 67  AA
      
      clr = clr6; // point here now !
    }
    break;
      
    case 8:   //#RRGGBBAA ... perfect !
    break;
      
    default:
      
      return RT_FAIL; // Unexpected.
  }
  
  // Read "RRGGBB" formatted string
  if( sscanf(clr, "%02x%02x%02x%02x", &rr, &gg, &bb, &aa) == 4)
  {
    r = rr;   g = gg;  b = bb;  a = aa;
  }
  else
  {
    retVal = RT_FAIL;
  }
  
  return retVal;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
