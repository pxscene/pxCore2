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

    pxTextLine(const char* text, uint32_t x, uint32_t y)
            : pixelSize(10)
            , color(0xFFFFFFFF)
    {
        this->text = text;
        this->x = x;
        this->y = y;
        this->styleSet = false;
    };

    void setStyle(const rtObjectRef& f, uint32_t ps, uint32_t c) {
        // TODO: validate pxFont object
        this->font = f;
        this->pixelSize = ps;
        this->color = c;
        this->styleSet = true;
    }

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
    pxTextCanvasMeasurements(rtObjectRef sm)
    {
        fromSimpleMeasurements(sm);
    }
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

    void fromSimpleMeasurements(rtObjectRef sm) {
        pxTextSimpleMeasurements* pSm = ((pxTextSimpleMeasurements*)sm.getPtr());
        mw = pSm->w(); mh = pSm->h();
    }

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
    rtProperty(shadowBlur, shadowBlur, setShadowBlur, uint8_t);
    rtProperty(shadowOffsetX, shadowOffsetX, setShadowOffsetX, float);
    rtProperty(shadowOffsetY, shadowOffsetY, setShadowOffsetY, float);

    uint32_t alignHorizontal()              const { return mAlignHorizontal;}
    rtError alignHorizontal(uint32_t& v)    const { v = mAlignHorizontal; return RT_OK;}
    rtError setAlignHorizontal(uint32_t v)        { mAlignHorizontal = v;  setNeedsRecalc(true); return RT_OK;}

    rtError fillStyle(rtValue &c) const
    {
        return textColor(c);
    }

    rtError setFillStyle(rtValue c)
    {
        rtLogInfo("pxTextCanvas::setFillStyle. Called with param: %#08x", c.toUInt32());
        setTextColor(c);
        return RT_OK;
    }
    rtError textBaseline(rtString &b) const
    {
        b = mTextBaseline;
        return RT_OK;
    }

    rtError setTextBaseline(const rtString& c)
    {
        rtLogInfo("pxTextCanvas::setTextBaseline called with param: %s", c.cString());
        rtLogError("pxTextCanvas::setTextBaseline. NOT IMPLEMENTED. Call ignored.");
        mTextBaseline = c;
        return RT_OK;
    }

    rtError globalAlpha(float& a) const
    {
        a = mGlobalAlpha;
        return RT_OK;
    }

    rtError setGlobalAlpha(const float a)
    {
        rtLogInfo("pxTextCanvas::setGlobalAlpha called with param: %f", a);
        rtLogError("pxTextCanvas::setGlobalAlpha. NOT IMPLEMENTED. Call ignored.");
        mGlobalAlpha = a;
        return RT_OK;
    }

    rtError shadowColor(uint32_t& c) const
    {
        c = mShadowColor;
        return RT_OK;
    }

    rtError setShadowColor(const uint32_t c)
    {
        rtLogInfo("pxTextCanvas::setShadowColor. Called with param: %#08x", c);
        rtLogError("pxTextCanvas::setShadowColor. NOT IMPLEMENTED. Call ignored.");
        mShadowColor = c;
        return RT_OK;
    }

    rtError shadowBlur(uint8_t& b) const
    {
        b = mShadowColor;
        return RT_OK;
    }

    rtError setShadowBlur(const uint8_t b)
    {
        rtLogInfo("pxTextCanvas::setShadowBlur. Called with param: %d", b);
        rtLogError("pxTextCanvas::setShadowBlur. NOT IMPLEMENTED. Call ignored.");
        mShadowBlur = b;
        return RT_OK;
    }

    rtError shadowOffsetX(float& o) const
    {
        o = mShadowOffsetX;
        return RT_OK;
    }

    rtError setShadowOffsetX(const float o)
    {
        rtLogInfo("pxTextCanvas::setShadowOffsetX called with param: %f", o);
        rtLogError("pxTextCanvas::setShadowOffsetX. NOT IMPLEMENTED. Call ignored.");
        mShadowOffsetX = o;
        return RT_OK;
    }

    rtError shadowOffsetY(float& o) const
    {
        o = mShadowOffsetY;
        return RT_OK;
    }

    rtError setShadowOffsetY(const float o)
    {
        rtLogInfo("pxTextCanvas::setShadowOffsetY called with param: %f", o);
        rtLogError("pxTextCanvas::setShadowOffsetY. NOT IMPLEMENTED. Call ignored.");
        mShadowOffsetY = o;
        return RT_OK;
    }

    /* override virtuals from pxObject that must affect the readiness of pxTextCanvas due to text measurements */
    rtError setW(float v) override      { rtLogInfo("pxTextCanvas::setW: %f", v); setNeedsRecalc(true); return pxObject::setW(v);}
    rtError setH(float v) override      { rtLogInfo("pxTextCanvas::setH: %f", v); setNeedsRecalc(true); return pxObject::setH(v);}
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

    bool mInitialized;
    bool mNeedsRecalc;
    uint32_t mAlignHorizontal;
    rtString mTextBaseline;
    float mGlobalAlpha;
    uint32_t mShadowColor;
    uint8_t  mShadowBlur;
    float mShadowOffsetX;
    float mShadowOffsetY;

    std::vector<pxTextLine> mTextLines;

#ifdef PXSCENE_FONT_ATLAS
    std::vector<pxTexturedQuads> mQuadsVector;
#endif
    rtObjectRef measurements;

    void renderTextLine(const pxTextLine& textLine);
};

#endif