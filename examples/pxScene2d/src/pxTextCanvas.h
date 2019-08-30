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
    uint32_t x;
    uint32_t y;
    // current style settings
    rtObjectRef font;
    uint32_t pixelSize;
    uint32_t color;
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
    int32_t h() const { return mh; }
    void setW(int32_t v) { mw = v;}
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

    rtProperty(alignHorizontal, alignHorizontal, setAlignHorizontal, uint32_t);
    rtProperty(fillStyle, fillStyle, setFillStyle, rtValue);
    rtProperty(textBaseline, textBaseline, setTextBaseline, rtString);
    rtProperty(globalAlpha, globalAlpha, setGlobalAlpha, float);
    rtProperty(shadowColor, shadowColor, setShadowColor, uint32_t);
    rtProperty(shadowBlur, shadowBlur, setShadowBlur, uint32_t);
    rtProperty(shadowOffsetX, shadowOffsetX, setShadowOffsetX, float);
    rtProperty(shadowOffsetY, shadowOffsetY, setShadowOffsetY, float);
    rtProperty(width, w, setW, float);
    rtProperty(height, h, setH, float);
    // specific to pxTextCanvas
    rtProperty(label, label, setLabel, rtString);   // mainly for debug logging when multiple canvases are created (default behaviour)
                                                    // Lightning++ assigns the timestamp, initially
    rtProperty(colorMode, colorMode, setColorMode, rtString); // Lightning++ assigns 'ARGB' for color compatibility


    uint32_t alignHorizontal() const;
    rtError alignHorizontal(uint32_t& v) const;
    rtError setAlignHorizontal(uint32_t v);
    rtError fillStyle(rtValue &c) const;
    rtError setFillStyle(rtValue c);
    rtError textBaseline(rtString &b) const;
    rtError setTextBaseline(const rtString& c);
    rtError globalAlpha(float& a) const;
    rtError setGlobalAlpha(const float a);
    rtError shadowColor(uint32_t& c) const;
    rtError setShadowColor(const uint32_t c);
    rtError shadowBlur(uint32_t& b) const;
    rtError setShadowBlur(const uint32_t b);
    rtError shadowOffsetX(float& o) const;
    rtError setShadowOffsetX(const float o);
    rtError shadowOffsetY(float& o) const;
    rtError setShadowOffsetY(const float o);

    rtError label(rtString &c) const;
    rtError setLabel(const rtString &c);
    rtError colorMode(rtString &c) const;
    rtError setColorMode(const rtString &c);

    /* override virtuals from pxObject that must affect the readiness of pxTextCanvas due to text measurements */
    rtError setW(float v) override      { setNeedsRecalc(true); return pxObject::setW(v);}
    rtError setH(float v) override      { setNeedsRecalc(true); return pxObject::setH(v);}
    rtError setClip(bool v) override    { setNeedsRecalc(true); return pxObject::setClip(v);}
    rtError setSX(float v) override     { setNeedsRecalc(true); return pxObject::setSX(v);}
    rtError setSY(float v) override     { setNeedsRecalc(true); return pxObject::setSY(v);}

    void renderText(bool render);
    void setNeedsRecalc(bool recalc);
    virtual float getFBOWidth();
    virtual float getFBOHeight();

    virtual void resourceReady(rtString readyResolution);
    virtual void sendPromise();
    virtual void draw();
    virtual void onInit();
    virtual void update(double t, bool updateChildren = true);

    virtual float getOnscreenWidth();
    virtual float getOnscreenHeight();

    rtMethod1ArgAndReturn("measureText", measureText, rtString, rtObjectRef);
    rtError measureText(rtString text, rtObjectRef &o);

    rtMethod3ArgAndNoReturn("fillText", fillText, rtString, uint32_t, uint32_t);
    rtError fillText(rtString text, uint32_t x, uint32_t y);

    rtMethodNoArgAndNoReturn("clear", clear);
    rtError clear();

    rtMethod4ArgAndNoReturn("fillRect", fillRect, uint32_t, uint32_t, uint32_t, uint32_t);
    rtError fillRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

    rtMethod2ArgAndNoReturn("translate", translate, uint32_t, uint32_t);
    rtError translate(uint32_t x, uint32_t y);

protected:
    pxTextCanvasMeasurements* getMeasurements() { return (pxTextCanvasMeasurements*)measurements.getPtr();}
    void recalc();
    void clearMeasurements();
    static uint32_t argb2rgba(uint32_t val);

    bool mInitialized;
    bool mNeedsRecalc;
    uint32_t mAlignHorizontal;
    rtString mTextBaseline;
    float mGlobalAlpha;
    uint32_t mShadowColor;
    uint32_t  mShadowBlur;
    float mShadowOffsetX;
    float mShadowOffsetY;

    rtString mLabel;
    rtString mColorMode;
    uint32_t mTranslateX;
    uint32_t mTranslateY;

    std::vector<pxTextLine> mTextLines;

#ifdef PXSCENE_FONT_ATLAS
    std::vector<pxTexturedQuads> mQuadsVector;
#endif
    rtObjectRef measurements;

    void renderTextLine(const pxTextLine& textLine);
};

#endif