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

#define CLAMP(_x, _min, _max) ( (_x) < (_min) ? (_min) : (_x) > (_max) ? (_max) : (_x) )
extern pxContext context;

pxTextLine::pxTextLine(const char* text, uint32_t x, uint32_t y)
        : pixelSize(10)
        , color(0xFFFFFFFF)
{
    this->text     = text;
    this->x        = x;
    this->y        = y;
    this->styleSet = false;
};

void pxTextLine::setStyle(const rtObjectRef& f, uint32_t ps, uint32_t c) {
    // TODO: validate pxFont object
    this->font      = f;
    this->pixelSize = ps;
    this->color     = c;
    this->styleSet  = true;
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

        , mTextW(0.0f)
        , mTextH(0.0f)

        , mLabel("")
        , mColorMode(ColorMode::ARGB)

        , mTranslateX(0)
        , mTranslateY(0)
{
    measurements = new pxTextCanvasMeasurements;

    mFontLoaded = false;
    mFontFailed = false;

    mTextBaseline = pxConstantsTextBaseline::ALPHABETIC;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // Shadow stuff..
    //
    mShadowCfg.shadow = false;
  
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // Highlight stuff..
    //
    mHighlightCfg.highlight = false;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    setW(pxTextCanvas::DEFAULT_WIDTH);
    setH(pxTextCanvas::DEFAULT_HEIGHT);

    setClip(true);
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

    if (mColorMode == ColorMode::ARGB)
    {
        clr = argb2rgba(c.toUInt32());
        rtLogDebug("pxTextCanvas::setFillStyle. ARGB param %#08x converted to RGBA: %#08x", c.toUInt32(), clr.toUInt32());
    }
    else
    {
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
   (mColorMode == ColorMode::RGBA) ? c = "RGBA" : "ARGB";
   return RT_OK;
}

rtError pxTextCanvas::setColorMode(const rtString &c)
{
    rtError res = RT_OK;

    rtString clr;
    this->colorMode(clr); // current color mode
  
    if (c != clr)
    {
        if ((c == "ARGB") || (c == "RGBA"))
        {
            (c == "RGBA") ? mColorMode = ColorMode::RGBA : ColorMode::ARGB;
          
            rtLogDebug("Setting color mode: '%s;", clr.cString());
        }
        else
        {
            rtLogError("Unknown color mode %s. Supported modes: 'ARGB', 'RGBA'", clr.cString());
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

rtError pxTextCanvas::setShadowColor(rtValue c)
{
    rtValue clr = c;
    if (mColorMode == ColorMode::ARGB)
    {
      clr = argb2rgba(c.toUInt32());
    }
  
    return setColor(mShadowCfg.shadowColor, clr);
}

rtError pxTextCanvas::shadowColor(rtValue &c) const
{
    uint32_t cc = 0;
    rtError err = colorFloat4_to_UInt32(&mShadowCfg.shadowColor[0], cc); // RGBA
  
    if (mColorMode == ColorMode::ARGB)
    {
      uint32_t argb = argb2rgba(cc);
      cc = argb;
    }

    c = cc;

    return err;
}

rtError pxTextCanvas::setHighlightColor(rtValue c)
{
    rtValue clr = c;
    if (mColorMode == ColorMode::ARGB)
    {
      clr = argb2rgba(c.toUInt32());
    }

    return setColor(mHighlightCfg.highlightColor, clr);
}

rtError pxTextCanvas::highlightColor(rtValue &c) const
{
    uint32_t cc = 0;
    rtError err = colorFloat4_to_UInt32(&mHighlightCfg.highlightColor[0], cc); // RGBA

    if (mColorMode == ColorMode::ARGB)
    {
      uint32_t argb = argb2rgba(cc);
      cc = argb;
    }

    c = cc;

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

    uint32_t size     = textLine.pixelSize;
    uint32_t alignH   = textLine.alignHorizontal;
    uint32_t baseline = textLine.textBaseline;

    if (mFont != textLine.font)
    {
        setFont(textLine.font);
    }

    rtLogDebug("pxTextCanvas::renderTextLine; textline: '%s', current canvas: '%s' (w x h): %04.0f x %04.0f"
            , cStr
            , mLabel.cString()
            , mw
            , mh
            );

    if (getFontResource() != nullptr)
    {
        getFontResource()->measureTextInternal(cStr, size, sx, sy, mTextW, mTextH);

        switch (alignH)
        {
            case pxConstantsAlignHorizontal::CENTER:   xPos -= float(mTextW / 2);     break;
            case pxConstantsAlignHorizontal::RIGHT:    xPos -= mTextW;                break;
        }

        switch (baseline)
        {
            case pxConstantsTextBaseline::ALPHABETIC:  yPos -= float(size);           break;
            case pxConstantsTextBaseline::TOP:         yPos -= float(0.200 * mTextH); break;
            case pxConstantsTextBaseline::HANGING:     yPos -= float(0.325 * mTextH); break;
            case pxConstantsTextBaseline::MIDDLE:      yPos -= float(0.575 * mTextH); break;
            case pxConstantsTextBaseline::IDEOGRAPHIC: yPos -= float(1.100 * size);   break;
            case pxConstantsTextBaseline::BOTTOM:      yPos -= mTextH;                break;
        }
    }

    // Now, render the text
    if( getFontResource() != nullptr)
    {
#ifdef PXSCENE_FONT_ATLAS
        pxTexturedQuads quads;
        getFontResource()->renderTextToQuads(cStr, size, sx, sy, quads, roundf(xPos) , roundf(yPos));

        quads.setColor(textLine.color);
        mQuadsVector.push_back(quads);
#else
        //getFontResource()->renderText(cStr, size, xPos, tempY, sx, sy, mTextColor,lineWidth);
        rtLogError("pxTextCanvas::drawing without FONT ATLAS is not supported yet.");
#endif
    }
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

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // Shadow stuff...
    //
    static textFx_t textFx;

    textFx.shadow.shadow       = false;  // default
    textFx.highlight.highlight = false;  // default

    if(mShadowCfg.shadow)
    {
        mShadowCfg.width  = getOnscreenWidth()  + mShadowCfg.shadowOffsetX;
        mShadowCfg.height = getOnscreenHeight() + mShadowCfg.shadowOffsetY;

        memcpy(&textFx.shadow, &mShadowCfg, sizeof(shadowFx_t) );
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // Highlight stuff...
    //

    if(mHighlightCfg.highlight)
    {
//        rtLogDebug("Draw Highlight stuff.... mQuadsVector = %d", (int) mQuadsVector.size() );
	
        if(mQuadsVector.size() > 0)
        {
          mHighlightCfg.highlightHeight = mPixelSize * 1.45;   // TODO: '1.45' is an Egregious MAGIC NUMBER
          mHighlightCfg.width           = mTextW; // getOnscreenWidth();
          mHighlightCfg.height          = mTextH; // getOnscreenHeight();
          
          memcpy(&textFx.highlight, &mHighlightCfg, sizeof(highlightFx_t) );
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //
    // DRAW Text / Glyphs via 'mQuadsVector'
    //
  
    mShadowCfg.shadow = true;//JUNK
    textFx_t *pFx = (mShadowCfg.shadow || mHighlightCfg.highlight) ? &textFx : NULL;

    // y = 0 ... for LIGHTNING

    float x = 0, y = 0;// mTextH/2;  // TODO: 'mTextH/2' is an Egregious MAGIC NUMBER
  
    for (std::vector<pxTexturedQuads>::iterator it  = mQuadsVector.begin();
                                                it != mQuadsVector.end();   ++it)
    {
        (*it).draw(x, y, pFx);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
    textLine.textBaseline    = mTextBaseline;
    textLine.translateX      = mTranslateX;
    textLine.translateY      = mTranslateY;

    mTextLines.push_back(textLine);
    setNeedsRecalc(true);

    return RT_OK;
}

rtError pxTextCanvas::setText(const char* s)
{
  if( !mText.compare(s) ) // did NOT change
  {
    return RT_OK;
  }
  
  pxText::setText(s);

  fillText(mText, 0,0);
  
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
    UNUSED_PARAM(x);
    UNUSED_PARAM(y);
    UNUSED_PARAM(width);
    UNUSED_PARAM(height);

    rtLogDebug("pxTextCanvas::fillRect called with params: x %d, y %d, width %d, height %d", x, y, width, height);
    rtLogDebug("pxTextCanvas::fillRect. NOT IMPLEMENTED. Call ignored.");
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

// Shadow stuff
rtDefineProperty(pxTextCanvas, shadow);
rtDefineProperty(pxTextCanvas, shadowColor);
rtDefineProperty(pxTextCanvas, shadowOffsetX);
rtDefineProperty(pxTextCanvas, shadowOffsetY);
rtDefineProperty(pxTextCanvas, shadowBlur);

// Highlight stuff
rtDefineProperty(pxTextCanvas, highlight);
rtDefineProperty(pxTextCanvas, highlightColor);
rtDefineProperty(pxTextCanvas, highlightOffset);
rtDefineProperty(pxTextCanvas, highlightPaddingLeft);
rtDefineProperty(pxTextCanvas, highlightPaddingRight);
