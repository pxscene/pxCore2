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

#include "pxConstants.h"
#include "pxText.h"
#include "pxTextCanvas.h"
#include "pxContext.h"
#include "pxWebGL.h"

#define CLAMP(_x, _min, _max) ( (_x) < (_min) ? (_min) : (_x) > (_max) ? (_max) : (_x) )
extern pxContext context;

//pxTextLine
pxTextLine::pxTextLine()
        : styleSet(false)
        , x(0), y(0)
        , pixelSize(10)
        , color(0xFFFFFFFF), alignHorizontal(0), textBaseline(0)
        , translateX(0)
        , translateY(0)
{
    this->effects.shadowEnabled = false;
    this->effects.highlightEnabled = false;
}

pxTextLine::pxTextLine(const char* text, uint32_t x, uint32_t y)
        : pixelSize(10)
        , color(0xFFFFFFFF), alignHorizontal(0), textBaseline(0), translateX(0), translateY(0)
{
    this->text = text;
    this->x = x;
    this->y = y;
    this->styleSet = false;
    this->effects.shadowEnabled = false;
    this->effects.highlightEnabled = false;
};

void pxTextLine::setStyle(const rtObjectRef& f, uint32_t ps, uint32_t c) {
    // TODO: validate pxFont object
    this->font = f;
    this->pixelSize = ps;
    this->color = c;
    this->styleSet = true;
}

bool pxTextLine::hasTextEffects() const
{
    return effects.shadowEnabled || effects.highlightEnabled;
}

//pxTextCanvasMeasurements
pxTextCanvasMeasurements::pxTextCanvasMeasurements(rtObjectRef sm)
{
    fromSimpleMeasurements(sm);
}

void pxTextCanvasMeasurements::fromSimpleMeasurements(rtObjectRef sm) {
    pxTextSimpleMeasurements* pSm = ((pxTextSimpleMeasurements*)sm.getPtr());
    mw = pSm->w(); mh = pSm->h();
}

//pxTextCanvas
pxTextCanvas::pxTextCanvas(pxScene2d* s): pxText(s)
        , mInitialized(false)
        , mNeedsRecalc(false)
        , mAlignHorizontal(pxConstantsAlignHorizontal::LEFT)
        , mGlobalAlpha(0.0f)
        , mShadowOffsetX(0.0f)
        , mShadowOffsetY(0.0f)
        , mShadowBlur(0.0f)
        , mHighlight(false)
        , mLabel("")
        , mColorMode("RGBA") //TODO: make a const class from it?
        , mTranslateX(0)
        , mTranslateY(0)
{
    measurements = new pxTextCanvasMeasurements;
    mFontLoaded = false;
    mFontFailed = false;
    mTextBaseline = pxConstantsTextBaseline::ALPHABETIC;
	setClip(true);
    float c[4] = {1, 1, 1, 0};
    memset(mHighlightRect, 0, sizeof(mHighlightRect));
    memcpy(mShadowColor, c, sizeof(mShadowColor));
    memcpy(mHighlightColor, c, sizeof(mHighlightColor));
}

/** This signals that the font file loaded successfully; now we need to
 * send the ready promise once we have the text, too
 */
void pxTextCanvas::resourceReady(rtString readyResolution)
{
    if( !readyResolution.compare("resolve"))
    {
        mFontLoaded = true;
        if( mInitialized) {
            setNeedsRecalc(true);
            pxObject::onTextureReady();
            if( !mParent)
            {
                // Send the promise here because the canvas will not get an
                // update call until it has parent
                recalc();
                sendPromise();
            }
        }
    }
    else
    {
        mFontFailed = true;
        pxObject::onTextureReady();
        mReady.send("reject",this);
    }
}

uint32_t pxTextCanvas::alignHorizontal() const
{
    return mAlignHorizontal;
}

rtError pxTextCanvas::alignHorizontal(uint32_t& v) const
{
    v = mAlignHorizontal;
    return RT_OK;
}

rtError pxTextCanvas::alignHorizontal(rtString& v) const
{
    v = mAlignHorizontalStr;
    return RT_OK;
}

rtError pxTextCanvas::setAlignHorizontal(const rtString &v)
{
    if (v == "left" || v == "start")
    {
        mAlignHorizontal = pxConstantsAlignHorizontal::LEFT;
    }
    else if (v == "center")
    {
        mAlignHorizontal = pxConstantsAlignHorizontal::CENTER;
    }
    else if (v == "right" || v == "end")
    {
        mAlignHorizontal = pxConstantsAlignHorizontal::RIGHT;
    }
    else
    {
        mAlignHorizontal = pxConstantsAlignHorizontal::LEFT;
    }

    mAlignHorizontalStr = v;

    setNeedsRecalc(true);

    return RT_OK;
}

rtError pxTextCanvas::fillStyle(rtValue &c) const
{
    return textColor(c);
}

rtError pxTextCanvas::setFillStyle(rtValue c)
{
    rtValue clr;
    if (mColorMode == "ARGB")
    {
        clr = argb2rgba(c.toUInt32());
        rtLogDebug("pxTextCanvas::setFillStyle. ARGB param %#08x converted to RGBA: %#08x", c.toUInt32(), clr.toUInt32());
    } else {
        clr = c;
    }
    setTextColor(clr);
    return RT_OK;
}

rtError pxTextCanvas::textBaseline(uint32_t& v) const
{
    v = mTextBaseline;
    return RT_OK;
}

rtError pxTextCanvas::textBaseline(rtString& v) const
{
    v = mTextBaselineStr;
    return RT_OK;
}

rtError pxTextCanvas::setTextBaseline(const rtString &v)
{
    if (v == "alphabetic")
    {
        mTextBaseline = pxConstantsTextBaseline::ALPHABETIC;
    }
    else if (v == "top")
    {
        mTextBaseline = pxConstantsTextBaseline::TOP;
    }
    else if (v == "hanging")
    {
        mTextBaseline = pxConstantsTextBaseline::HANGING;
    }
    else if (v == "middle")
    {
        mTextBaseline = pxConstantsTextBaseline::MIDDLE;
    }
    else if (v == "ideographic")
    {
        mTextBaseline = pxConstantsTextBaseline::IDEOGRAPHIC;
    }
    else if (v == "bottom")
    {
        mTextBaseline = pxConstantsTextBaseline::BOTTOM;
    }
    else
    {
        mTextBaseline = pxConstantsTextBaseline::ALPHABETIC;
    }

    mTextBaselineStr = v;

    setNeedsRecalc(true);

    return RT_OK;
}

rtError pxTextCanvas::globalAlpha(float& a) const
{
    a = mGlobalAlpha;
    return RT_OK;
}

rtError pxTextCanvas::setGlobalAlpha(const float a)
{
    rtLogDebug("pxTextCanvas::setGlobalAlpha called with param: %f", a);
    mGlobalAlpha = CLAMP(a, 0.0f, 1.0f);
    setA(mGlobalAlpha); // temporary solution. Actually alpha must be applied to the rendered objects, not the canvas itself.
    return RT_OK;
}

rtError pxTextCanvas::label(rtString &c) const
{
    c = mLabel;
    return RT_OK;
}
rtError pxTextCanvas::setLabel(const rtString &c)
{
    mLabel = c;
    return RT_OK;
}

rtError pxTextCanvas::colorMode(rtString &c) const
{
    c = mColorMode;
    return RT_OK;
}
rtError pxTextCanvas::setColorMode(const rtString &c)
{
    rtError res = RT_OK;

    if (c != mColorMode)
    {
        if ((c == "ARGB") || (c == "RGBA"))
        {
            mColorMode = c;
            rtLogDebug("Setting color mode: '%s;", c.cString());
        }
        else
        {
            rtLogError("Unknown color mode %s. Supported modes: 'ARGB', 'RGBA'", c.cString());
            res = RT_ERROR;
        }
    }

    return res;
}

float pxTextCanvas::getFBOWidth()
{
    return pxText::getFBOWidth();
}
float pxTextCanvas::getFBOHeight()
{
    return pxText::getFBOHeight();
}

void pxTextCanvas::onInit()
{
    pxText::onInit();
    rtLogDebug("pxTextCanvas::onInit. mFontLoaded=%d\n",mFontLoaded);
    mInitialized = true;

    // If this is using the default font, we would not get a callback
    if(mFontLoaded || (getFontResource() != nullptr && getFontResource()->isFontLoaded()))
    {
        mFontLoaded = true;
        setNeedsRecalc(true);
        if (!mParent)
        {
            resourceReady("resolve");
        }
    }
}

void pxTextCanvas::recalc()
{

    if( mNeedsRecalc && mInitialized && mFontLoaded) {
        clearMeasurements();
#ifdef PXSCENE_FONT_ATLAS
        mQuadsVector.clear();
#endif
        renderText(false);
        setNeedsRecalc(false);
        if(clip()) {
            pxObject::onTextureReady();
        }
#ifdef PXSCENE_FONT_ATLAS
        mQuadsVector.clear();
#endif
        renderText(true);
        mDirty = false;
    }
}

void pxTextCanvas::clearMeasurements()
{
    getMeasurements()->clear();
}

void pxTextCanvas::setNeedsRecalc(bool recalc)
{
    rtLogDebug("Setting mNeedsRecalc=%d\n",recalc);
    mNeedsRecalc = recalc;

    if(recalc)
    {
        rtLogDebug("TextCanvas CREATE NEW PROMISE\n");
        createNewPromise();
//        mDirty = true;
    }
}

void pxTextCanvas::sendPromise()
{
    rtLogDebug("pxTextCanvas::sendPromise mInitialized=%d mFontLoaded=%d mNeedsRecalc=%d\n",mInitialized,mFontLoaded,mNeedsRecalc);
    // TODO: hanlde mNeedsRecalc properly!
    if(mInitialized && mFontLoaded /*&& !mNeedsRecalc*/ && !mDirty && !((rtPromise*)mReady.getPtr())->status()) {
        rtLogDebug("pxTextCanvas SENDPROMISE\n");
        mReady.send("resolve", this);
    } else {
//        rtLogDebug("pxTextCanvas NOT sending promise");
    }
}

rtError pxTextCanvas::setPixelSize(uint32_t v)
{
    mPixelSize = v;
    setNeedsRecalc(true);
    return RT_OK;
}
bool pxTextCanvas::shadow() const
{
    return mShadowColor[3] > 0 && (mShadowOffsetX || mShadowOffsetY);
}

bool pxTextCanvas::highlight() const
{
    return mHighlight && mHighlightColor[3] > 0;
}

rtError pxTextCanvas::setShadowColor(rtValue c)
{
    rtValue clr;
    if (mColorMode == "ARGB")
    {
        clr = argb2rgba(c.toUInt32());
    } else {
        clr = c;
    }

    return setColor(mShadowColor, clr);
}

rtError pxTextCanvas::shadowColor(rtValue &c) const
{
    uint32_t cc = 0;
    rtError err = colorFloat4_to_UInt32(&mShadowColor[0], cc);

    if (mColorMode == "ARGB")
    {
        c = rgba2argb(cc);
    } else {
        c = cc;
    }
    return err;
}

void pxTextCanvas::renderText(bool render)
{
    if (render && !mTextLines.empty())
    {
        for (std::vector<pxTextLine>::iterator it = mTextLines.begin();
                                              it != mTextLines.end();   ++it)
        {
            renderTextLine(*it);
        }
    }
}

void pxTextCanvas::renderTextLine(const pxTextLine& textLine)
{
    const char* cStr = textLine.text.cString();
    float xPos = (float)(textLine.x + textLine.translateX);
    float yPos = (float)(textLine.y + textLine.translateY);
    // TODO ignoring sx and sy now.
    float sx = 1.0;
    float sy = 1.0;

    uint32_t size = textLine.pixelSize;
    uint32_t alignH = textLine.alignHorizontal;
    uint32_t baseline = textLine.textBaseline;

    if (mFont != textLine.font)
    {
        setFont(textLine.font);
    }

    rtLogDebug("pxTextCanva::renderTextLine; textline: '%s', current canvas: '%s' (w x h): %04.0f x %04.0f, shadow: %s, highlight: %s"
            , cStr
            , mLabel.cString()
            , mw
            , mh
            , textLine.effects.shadowEnabled ? "YES" : "NO"
            , textLine.effects.highlightEnabled ? "YES" : "NO"
            );

    if (getFontResource() != nullptr)
    {
        float textW, textH;
        long ascender;
        long descender;
        getFontResource()->measureTextInternal(cStr, size, sx, sy, textW, textH, ascender, descender);

        switch (alignH)
        {
            case pxConstantsAlignHorizontal::CENTER:
                xPos -= textW / 2;
                break;

            case pxConstantsAlignHorizontal::RIGHT:
                xPos -= textW;
                break;
        }

        switch (baseline)
        {
            case pxConstantsTextBaseline::ALPHABETIC:
                yPos -= float(ascender);
                break;

            case pxConstantsTextBaseline::TOP:
                yPos -= 0.2 * textH;
                break;

            case pxConstantsTextBaseline::HANGING:
                yPos -= 0.325 * textH;
                break;

            case pxConstantsTextBaseline::MIDDLE:
                yPos -= 0.575 * textH;
                break;

            case pxConstantsTextBaseline::IDEOGRAPHIC:
                yPos -= float(1.1 * size);
                break;

            case pxConstantsTextBaseline::BOTTOM:
                yPos -= textH;
                break;
        }
    }

    // Now, render the text
    if( getFontResource() != nullptr)
    {
#ifdef PXSCENE_FONT_ATLAS
        pxTexturedQuads quads;
        getFontResource()->renderTextToQuads(cStr, size, sx, sy, quads, roundf(xPos) , roundf(yPos));

        quads.setColor(textLine.color);
        if (textLine.hasTextEffects()) quads.setTextEffects(&textLine.effects);
        mQuadsVector.push_back(quads);
#else
        //getFontResource()->renderText(cStr, size, xPos, tempY, sx, sy, mTextColor,lineWidth);
        rtLogError("pxTextCanvas::drawing without FONT ATLAS is not supported yet.");
#endif
    }
}

rtError pxTextCanvas::paint(float x, float y, uint32_t color, bool translateOnly)
{
#ifdef PXSCENE_FONT_ATLAS
    if (mDirty)
    {
        mQuadsVector.clear();
        renderText(true);
        mDirty = false;
    }

    context.pushState();
    pxMatrix4f m;
    if (translateOnly)
    {
        m.translate(mx+x, my+y);
    }
    else
    {
      float tempX = mx;
      float tempY = my;
      mx += x;
      my += y;
      applyMatrix(m);
      mx = tempX;
      my = tempY;
    }
    context.setMatrix(m);
    context.setAlpha(ma);

    //ensure the viewport and size are correctly set
    int w = 0, h = 0;
    context.getSize(w, h);
    context.setSize(w, h);

    float textColor[4];
    memcpy(textColor, mTextColor, sizeof(textColor));
    if (color != 0xFFFFFFFF)
    {
        if (mColorMode == "ARGB")
        {
            color = argb2rgba(color);
        }
        textColor[PX_RED  ] *= (float)((color>>24) & 0xff) / 255.0f;
        textColor[PX_GREEN] *= (float)((color>>16) & 0xff) / 255.0f;
        textColor[PX_BLUE ] *= (float)((color>> 8) & 0xff) / 255.0f;
        textColor[PX_ALPHA] *= (float)((color>> 0) & 0xff) / 255.0f;
    }

    for (std::vector<pxTexturedQuads>::iterator it = mQuadsVector.begin() ; it != mQuadsVector.end(); ++it)
    {
        (*it).draw(0, 0, textColor);
    }
    context.popState();
#else
    rtLogError("pxTextCanvas::drawing without FONT ATLAS is not supported yet.");
#endif
    return RT_OK;
}

void pxTextCanvas::draw()
{
#ifdef PXSCENE_FONT_ATLAS
    if (mDirty)
    {
        mQuadsVector.clear();
        renderText(true);
        mDirty = false;
    }

    float x = 0;
    float y = 0;
    for (std::vector<pxTexturedQuads>::iterator it  = mQuadsVector.begin();
                                                it != mQuadsVector.end();   ++it)
    {
        (*it).draw(x, y);
    }
#else
    rtLogError("pxTextCanvas::drawing without FONT ATLAS is not supported yet.");
#endif
}

float pxTextCanvas::getOnscreenWidth()
{
    // TODO review max texture handling
    return this->w();
}

float pxTextCanvas::getOnscreenHeight()
{
    // TODO review max texture handling
    return this->h();
}

void pxTextCanvas::update(double t, bool updateChildren)
{
    if( mNeedsRecalc ) {
        recalc();
        markDirty();
    }
    pxText::update(t, updateChildren);
}

rtError pxTextCanvas::measureText(rtString text, rtObjectRef& o)
{
    rtObjectRef tcm = new pxTextCanvasMeasurements(); // TODO: aren't we leaking here with 'new'? Is it efficient?
    o = tcm;

    if(getFontResource() != nullptr)
    {
        rtObjectRef sm; // pxTextSimpleMeasurements
        getFontResource()->measureText(mPixelSize, text, sm);
        ((pxTextCanvasMeasurements*)tcm.getPtr())->fromSimpleMeasurements(sm);

        rtLogDebug("pxTextCanvas::measureText(). Got measurements; width: %d, height: %d "
                , ((pxTextCanvasMeasurements*)tcm.getPtr())->width()
                , ((pxTextCanvasMeasurements*)tcm.getPtr())->h()
                );
    }
    else
    {
        rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
        //return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
    }
    return RT_OK;
}

rtError pxTextCanvas::fillText(rtString text, int32_t x, int32_t y)
{
    rtLogDebug("pxTextCanvas::fillText called with params: text: '%s', x %d, y %d. Canvas: '%s' (w x h): %04.0f x %04.0f"
            , text.cString()
            , x
            , y
            , mLabel.cString()
            , mw
            , mh
            );
    pxTextLine textLine(text, x, y);
    rtValue color;
    textColor(color);
    textLine.setStyle(mFont, mPixelSize, color.toInt32());

    textLine.alignHorizontal = mAlignHorizontal;
    textLine.textBaseline = mTextBaseline;
    textLine.translateX = mTranslateX;
    textLine.translateY = mTranslateY;

    // Shadow
    textLine.effects.shadowEnabled       = false;  // default
    textLine.effects.highlightEnabled = false;  // default

    if(shadow()) // creating the shadow effects config
    {
        textLine.effects.shadowEnabled = true;
        memcpy(textLine.effects.shadowColor, mShadowColor, sizeof(textLine.effects.shadowColor));
        textLine.effects.shadowBlur = mShadowBlur;
        textLine.effects.shadowOffsetX = mShadowOffsetX;
        textLine.effects.shadowOffsetY = mShadowOffsetY;
        textLine.effects.shadowWidth = getOnscreenWidth() + textLine.effects.shadowOffsetX;
        textLine.effects.shadowHeight = getOnscreenHeight() + textLine.effects.shadowOffsetY;
    }

    if(highlight())// creating the hightlight effects config
    {
        textLine.effects.highlightEnabled = true;
        memcpy(textLine.effects.highlightColor, mHighlightColor, sizeof(textLine.effects.highlightColor));
        // now we have to translate the previous fillRect() call to highlight properties
        int32_t rectX = mHighlightRect[0];
        int32_t rectY = mHighlightRect[1];
        uint32_t rectW =  mHighlightRect[2];
        uint32_t rectH =  mHighlightRect[3];
        textLine.effects.highlightOffset = (y - rectY) > 0 ? (y - rectY) : 0;//getOnscreenHeight()/2;
        textLine.effects.highlightWidth = getOnscreenWidth();
        textLine.effects.highlightHeight = rectH;
        textLine.effects.highlightPaddingLeft = x - rectX;
        textLine.effects.highlightPaddingRight = rectW - textLine.effects.highlightWidth - textLine.effects.highlightPaddingLeft;
        textLine.effects.highlightBlockHeight = getOnscreenHeight();
        mHighlight = false;
    }
    mTextLines.push_back(textLine);
    setNeedsRecalc(true);
return RT_OK;
}

rtError pxTextCanvas::clear()
{
    mTextLines.clear();
    mTranslateX = 0;
    mTranslateY = 0;
    setNeedsRecalc(true);

    return RT_OK;
}

rtError pxTextCanvas::fillRect(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    rtLogDebug("pxTextCanvas::fillRect called with params: x %d, y %d, width %d, height %d", x, y, width, height);
    rtLogDebug("pxTextCanvas::fillRect is a hack now. A rect (we call it 'highlight') can be drawn beneath a text line issued by the next fillText() command.");

    // pxTextCanvas does not support drawing of primitives (yet); fillRect is a special case: it is used by Lightning
    // for highlighting a block of text. Current implementation sets the highlight parameters which will be applied
    // to the next fillText() call.
    mHighlight = true;
    memcpy(mHighlightColor, mTextColor, sizeof(mHighlightColor));
    mHighlightRect[0] = x;
    mHighlightRect[1] = y;
    mHighlightRect[2] = width;
    mHighlightRect[3] = height;
    return RT_OK;
}

rtError pxTextCanvas::translate(int32_t x, int32_t y)
{
    mTranslateX += x;
    mTranslateY += y;

    rtLogDebug("pxTextCanvas::translate applied translation params: x: %d, y: %d, current translation (x, y): %d, %d", x, y, mTranslateX, mTranslateY);

    return RT_OK;
}

uint32_t pxTextCanvas::argb2rgba(uint32_t val)
{
    return val << 8 | val >> 24;
}

uint32_t pxTextCanvas::rgba2argb(uint32_t val)
{
    return val << 24 | val >> 8;
}

// pxTextCanvasMeasurements
rtDefineObject(pxTextCanvasMeasurements, rtObject);
rtDefineProperty(pxTextCanvasMeasurements, width);

// pxTextCanvas
rtDefineObject(pxTextCanvas, pxText);
rtDefineProperty(pxTextCanvas, alignHorizontal);
rtDefineProperty(pxTextCanvas, fillStyle);
rtDefineProperty(pxTextCanvas, textBaseline);
rtDefineProperty(pxTextCanvas, globalAlpha);

rtDefineProperty(pxTextCanvas, label);
rtDefineProperty(pxTextCanvas, colorMode);
rtDefineProperty(pxTextCanvas, width);
rtDefineProperty(pxTextCanvas, height);

rtDefineMethod(pxTextCanvas, measureText);
rtDefineMethod(pxTextCanvas, fillText);
rtDefineMethod(pxTextCanvas, clear);
rtDefineMethod(pxTextCanvas, fillRect);
rtDefineMethod(pxTextCanvas, translate);

rtDefineProperty(pxTextCanvas, shadowColor);
rtDefineProperty(pxTextCanvas, shadowOffsetX);
rtDefineProperty(pxTextCanvas, shadowOffsetY);
rtDefineProperty(pxTextCanvas, shadowBlur);
