/*

 pxCore Copyright 2005-2019 John Robinson

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

#ifndef PX_TEXTCANVAS_H
#define PX_TEXTCANVAS_H

#include "pxScene2d.h"
#include "pxText.h"

namespace pxCalc {
    template<typename T>
    inline T min(T x, T y) { return (x < y ? x : y); }

    template<typename T>
    inline T max(T x, T y) { return (x > y ? x : y); }

    template<typename T>
    inline T clamp(T x, T lo, T hi) { return max(min(x, hi), lo); }
}

struct pxTextLine
{
    pxTextLine(): styleSet(false)
            , x(0), y(0)
            , pixelSize(10)
            , color(0xFFFFFFFF)
    {};
    pxTextLine(const char* text, uint32_t x, uint32_t y);
    void setStyle(const rtObjectRef& f, uint32_t ps, uint32_t c);

    bool styleSet;
    rtString text;
    int32_t x;
    int32_t y;

    // current style settings
    rtObjectRef font;
    uint32_t pixelSize;
    uint32_t color;
    uint32_t alignHorizontal;
    uint32_t textBaseline;
};

/**********************************************************************
 *
 * pxTextCanvasMeasurements (used in javascript measureText() call)
 *
 **********************************************************************/
class pxTextCanvasMeasurements: public rtObject
{
public:
    pxTextCanvasMeasurements():mw(0), mh(0) {}
    pxTextCanvasMeasurements(rtObjectRef sm);
    virtual ~pxTextCanvasMeasurements() {}

    rtDeclareObject(pxTextCanvasMeasurements, rtObject);
    rtReadOnlyProperty(width, width, int32_t);

    int32_t width() const { return mw;  }
    rtError width(int32_t& v) const { v = mw;  return RT_OK; }
  
    int32_t w() const { return mw;  }
    void setW(int32_t v) { mw = v;}
  
    int32_t h() const { return mh; }
    void setH(int32_t v) { mh = v; }
  
    void clear() { mw = 0; mh = 0;}
    void fromSimpleMeasurements(rtObjectRef sm);

protected:
    int32_t mw;
    int32_t mh;
};

/**********************************************************************
 *
 * pxTextCanvas
 *
 **********************************************************************/
class pxTextCanvas : public pxText {
public:
    rtDeclareObject(pxTextCanvas, pxText);

    pxTextCanvas(pxScene2d *s);
    virtual ~pxTextCanvas() {}
  
    rtProperty(alignHorizontal, alignHorizontal, setAlignHorizontal, rtString);
    rtProperty(fillStyle,       fillStyle,       setFillStyle,       rtValue);
    rtProperty(textBaseline,    textBaseline,    setTextBaseline,    rtString);
    rtProperty(globalAlpha,     globalAlpha,     setGlobalAlpha,     float);

    rtProperty(shadow,          shadow,          setShadow,          bool);
    rtProperty(shadowColor,     shadowColor,     setShadowColor,     rtValue);
    rtProperty(shadowOffsetX,   shadowOffsetX,   setShadowOffsetX,   float);
    rtProperty(shadowOffsetY,   shadowOffsetY,   setShadowOffsetY,   float);
    rtProperty(shadowBlur,      shadowBlur,      setShadowBlur,      float);

    rtProperty(highlight,             highlight,             setHighlight,              bool);
    rtProperty(highlightColor,        highlightColor,        setHighlightColor,         rtValue);
    rtProperty(highlightOffset,       highlightOffset,       setHighlightOffset,        float);
    rtProperty(highlightPaddingLeft,  highlightPaddingLeft,  setHighlightPadddingLeft,  float);
    rtProperty(highlightPaddingRight, highlightPaddingRight, setHighlightPadddingRight, float);

    rtProperty(width,  w, setW, float);
    rtProperty(height, h, setH, float);
    // specific to pxTextCanvas
    rtProperty(label, label, setLabel, rtString);   // mainly for debug logging when multiple canvases are created (default behaviour)
    rtProperty(colorMode, colorMode, setColorMode, rtString); // Lightning++ assigns 'ARGB' for color compatibility

    uint32_t alignHorizontal() const;
    rtError alignHorizontal(uint32_t& v) const;
    rtError alignHorizontal(rtString& v) const;
    rtError setAlignHorizontal(const rtString &v);
  
    rtError fillStyle(rtValue &c) const;
    rtError setFillStyle(rtValue c);
  
    rtError textBaseline(uint32_t& v) const;
    rtError textBaseline(rtString& v) const;
    rtError setTextBaseline(const rtString &v);
  
    rtError globalAlpha(float& a) const;
    rtError setGlobalAlpha(const float a);

    rtError label(rtString &c) const;
    rtError setLabel(const rtString &c);

    rtError colorMode(rtString &c) const;
    rtError setColorMode(const rtString &c);

    /* override virtuals from pxObject that must affect the readiness of pxTextCanvas due to text measurements */
    rtError setW(float v)   override    { setNeedsRecalc(true); return pxObject::setW(v);    }
    rtError setH(float v)   override    { setNeedsRecalc(true); return pxObject::setH(v);    }
    rtError setClip(bool v) override    { setNeedsRecalc(true); return pxObject::setClip(v); }
    rtError setSX(float v)  override    { setNeedsRecalc(true); return pxObject::setSX(v);   }
    rtError setSY(float v)  override    { setNeedsRecalc(true); return pxObject::setSY(v);   }

    rtError setPixelSize(uint32_t v) override ;

// - - - - - - - - - - - - - - - - - - Shadow - - - - - - - - - - - - - - - - - -

    rtError shadow(bool &v) const                      { v = mShadow;  return RT_OK; }
    virtual rtError setShadow(bool v)                  { mShadow = v;  return RT_OK; }

    rtError shadowColor(rtValue &c) const;
    rtError setShadowColor(rtValue c);

    rtError shadowOffsetX(float &v) const              { v = mShadowOffsetX; return RT_OK; }
    virtual rtError setShadowOffsetX(float v)          { mShadowOffsetX = v; return RT_OK; }

    rtError shadowOffsetY(float &v) const              { v = mShadowOffsetY; return RT_OK; }
    virtual rtError setShadowOffsetY(float v)          { mShadowOffsetY = v; return RT_OK; }

    rtError shadowBlur(float &v) const                 { v = mShadowBlur;    return RT_OK; }
    virtual rtError setShadowBlur(float v)             { mShadowBlur = v;    return RT_OK; }

    // - - - - - - - - - - - - - - - - - - Highlight - - - - - - - - - - - - - - - - - -

    rtError highlight(bool &v) const                   { v = mHighlight;  return RT_OK; }
    virtual rtError setHighlight(bool v)               { mHighlight = v;  return RT_OK; }

    rtError highlightColor(rtValue &c) const;
    rtError setHighlightColor(rtValue c);

    rtError highlightOffset(float &v) const            { v = mHighlightOffset;       return RT_OK; }
    virtual rtError setHighlightOffset(float v)        { mHighlightOffset = v;       return RT_OK; }

    rtError highlightPaddingLeft(float &v) const       { v = mHighlightPaddingLeft;  return RT_OK; }
    virtual rtError setHighlightPadddingLeft(float v)  { mHighlightPaddingLeft = v;  return RT_OK; }

    rtError highlightPaddingRight(float &v) const      { v = mHighlightPaddingRight; return RT_OK; }
    virtual rtError setHighlightPadddingRight(float v) { mHighlightPaddingRight = v; return RT_OK; }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void renderText(bool render);
    void setNeedsRecalc(bool recalc);
    virtual float getFBOWidth();
    virtual float getFBOHeight();

    virtual rtError setText(const char* text);
  
    virtual void resourceReady(rtString readyResolution);
    virtual void sendPromise();
    virtual void draw();
    virtual void onInit();
    virtual void update(double t, bool updateChildren = true);

    virtual float getOnscreenWidth();
    virtual float getOnscreenHeight();

    rtMethod1ArgAndReturn("measureText", measureText, rtString, rtObjectRef);
    rtError measureText(rtString text, rtObjectRef &o);

    rtMethod3ArgAndNoReturn("fillText", fillText, rtString, int32_t, int32_t);
    rtError fillText(rtString text, int32_t x, int32_t y);

    rtMethodNoArgAndNoReturn("clear", clear);
    rtError clear();

    rtMethod4ArgAndNoReturn("fillRect", fillRect, int32_t, int32_t, uint32_t, uint32_t);
    rtError fillRect(int32_t x, int32_t y, uint32_t width, uint32_t height);

    rtMethod2ArgAndNoReturn("translate", translate, int32_t, int32_t);
    rtError translate(int32_t x, int32_t y);

protected:
    pxTextCanvasMeasurements* getMeasurements() { return (pxTextCanvasMeasurements*)measurements.getPtr();}
    void recalc();
    void clearMeasurements();
    static uint32_t argb2rgba(uint32_t val);

    bool mInitialized;
    bool mNeedsRecalc;

    rtString mAlignHorizontalStr;
    uint32_t mAlignHorizontal;
    rtString mTextBaselineStr;
    uint32_t mTextBaseline;
    float    mGlobalAlpha;

    bool     mShadow;
    float    mShadowColor[4];
    float    mShadowOffsetX;
    float    mShadowOffsetY;
    float    mShadowBlur;

    bool     mHighlight;
    float    mHighlightColor[4];
    float    mHighlightOffset;
    float    mHighlightPaddingLeft;
    float    mHighlightPaddingRight;
  
    float    mTextW;
    float    mTextH;

    rtString mLabel;
    rtString mColorMode;
    int32_t  mTranslateX;
    int32_t  mTranslateY;

    std::vector<pxTextLine> mTextLines;

#ifdef PXSCENE_FONT_ATLAS
    std::vector<pxTexturedQuads> mQuadsVector;
#endif
    rtObjectRef measurements;

    void renderTextLine(const pxTextLine& textLine);
};

#endif
