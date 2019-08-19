#include "pxConstants.h"
#include "pxText.h"
#include "pxTextCanvas.h"
#include "pxContext.h"

extern pxContext context;

//pxTextLine
pxTextLine::pxTextLine(const char* text, uint32_t x, uint32_t y)
        : pixelSize(10)
        , color(0xFFFFFFFF)
{
    this->text = text;
    this->x = x;
    this->y = y;
    this->styleSet = false;
};

void pxTextLine::setStyle(const rtObjectRef& f, uint32_t ps, uint32_t c) {
    // TODO: validate pxFont object
    this->font = f;
    this->pixelSize = ps;
    this->color = c;
    this->styleSet = true;
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
        , mGlobalAlpha(0.0)
        , mShadowColor(0X00000000) /*fully transparent black*/
        , mShadowBlur(0)
        , mShadowOffsetX(0.0)
        , mShadowOffsetY(0.0)

{
    rtLogDebug("pxTextCanvas::ctor - begin");
    measurements = new pxTextCanvasMeasurements;
    mFontLoaded = false;
    mFontFailed = false;
    mTextBaseline = "alphabetic"; //TODO: make a const class from it?
	mLabel = "";
    rtLogDebug("pxTextCanvas::ctor - end");
}
/** This signals that the font file loaded successfully; now we need to
 * send the ready promise once we have the text, too
 */
void pxTextCanvas::resourceReady(rtString readyResolution)
{
    rtLogDebug("pxTextCanvas::resourceReady - begin");
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
    rtLogDebug("pxTextCanvas::resourceReady - end");
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

rtError pxTextCanvas::setAlignHorizontal(uint32_t v)
{
    mAlignHorizontal = v;
    setNeedsRecalc(true);
    return RT_OK;
}

rtError pxTextCanvas::fillStyle(rtValue &c) const
{
    return textColor(c);
}

rtError pxTextCanvas::setFillStyle(rtValue c)
{
    rtLogInfo("pxTextCanvas::setFillStyle. Called with param: %#08x", c.toUInt32());
    rtValue clr(argb2rgba(c.toUInt32()));
    rtLogInfo("pxTextCanvas::setFillStyle. Converted to: %#08x", clr.toUInt32());
    setTextColor(clr);
    return RT_OK;
}
rtError pxTextCanvas::textBaseline(rtString &b) const
{
    b = mTextBaseline;
    return RT_OK;
}

rtError pxTextCanvas::setTextBaseline(const rtString& c)
{
    rtLogInfo("pxTextCanvas::setTextBaseline called with param: %s", c.cString());
    rtLogError("pxTextCanvas::setTextBaseline. NOT IMPLEMENTED. Call ignored.");
    mTextBaseline = c;
    return RT_OK;
}

rtError pxTextCanvas::globalAlpha(float& a) const
{
    a = mGlobalAlpha;
    return RT_OK;
}

rtError pxTextCanvas::setGlobalAlpha(const float a)
{
    rtLogInfo("pxTextCanvas::setGlobalAlpha called with param: %f", a);
    rtLogError("pxTextCanvas::setGlobalAlpha. NOT IMPLEMENTED. Call ignored.");
    mGlobalAlpha = a;
    return RT_OK;
}

rtError pxTextCanvas::shadowColor(uint32_t& c) const
{
    c = mShadowColor;
    return RT_OK;
}

rtError pxTextCanvas::setShadowColor(const uint32_t c)
{
    rtLogInfo("pxTextCanvas::setShadowColor. Called with param: %#08x", c);
    rtLogError("pxTextCanvas::setShadowColor. NOT IMPLEMENTED. Call ignored.");
    mShadowColor = c;
    return RT_OK;
}

rtError pxTextCanvas::shadowBlur(uint32_t& b) const
{
    b = mShadowBlur;
    return RT_OK;
}

rtError pxTextCanvas::setShadowBlur(const uint32_t b)
{
    rtLogInfo("pxTextCanvas::setShadowBlur. Called with param: %d", b);
    rtLogError("pxTextCanvas::setShadowBlur. NOT IMPLEMENTED. Call ignored.");
    mShadowBlur = b;
    return RT_OK;
}

rtError pxTextCanvas::shadowOffsetX(float& o) const
{
    o = mShadowOffsetX;
    return RT_OK;
}

rtError pxTextCanvas::setShadowOffsetX(const float o)
{
    rtLogInfo("pxTextCanvas::setShadowOffsetX called with param: %f", o);
    rtLogError("pxTextCanvas::setShadowOffsetX. NOT IMPLEMENTED. Call ignored.");
    mShadowOffsetX = o;
    return RT_OK;
}

rtError pxTextCanvas::shadowOffsetY(float& o) const
{
    o = mShadowOffsetY;
    return RT_OK;
}

rtError pxTextCanvas::setShadowOffsetY(const float o)
{
    rtLogInfo("pxTextCanvas::setShadowOffsetY called with param: %f", o);
    rtLogError("pxTextCanvas::setShadowOffsetY. NOT IMPLEMENTED. Call ignored.");
    mShadowOffsetY = o;
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
    rtLogDebug("pxTextCanvas::onInit - begin.");
    pxText::onInit();
    rtLogInfo("pxTextCanvas::onInit. mFontLoaded=%d\n",mFontLoaded);
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
    rtLogDebug("pxTextCanvas::onInit - end.");
}

void pxTextCanvas::recalc()
{
    rtLogDebug("pxTextCanvas::recalc - begin.");

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
    rtLogDebug("pxTextCanvas::recalc - end.");
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
    rtLogDebug("TextCanvas sendPromise - begin");
    rtLogDebug("pxTextCanvas::sendPromise mInitialized=%d mFontLoaded=%d mNeedsRecalc=%d\n",mInitialized,mFontLoaded,mNeedsRecalc);
    // TODO: hanlde mNeedsRecalc properly!
    if(mInitialized && mFontLoaded /*&& !mNeedsRecalc*/ && !mDirty && !((rtPromise*)mReady.getPtr())->status())
    {
        rtLogDebug("pxTextCanvas SENDPROMISE\n");
        mReady.send("resolve",this);

    } else {
        rtLogDebug("pxTextCanvas NOT sending promise");
    }
    rtLogDebug("TextCanvas sendPromise - end");
}

void pxTextCanvas::renderText(bool render)
{
    rtLogDebug("pxTextCanvas::renderText - begin.");
    if (render && !mTextLines.empty()) {
        for (std::vector<pxTextLine>::iterator it = mTextLines.begin() ; it != mTextLines.end(); ++it)
            renderTextLine(*it);
    }
    rtLogDebug("pxTextCanvas::renderText - end.");
}

void pxTextCanvas::renderTextLine(const pxTextLine& textLine)
{
    rtLogDebug("pxTextCanvas::renderTextLine - begin");

    const char* cStr = textLine.text.cString();
    float xPos = textLine.x;
    float yPos = textLine.y;

    // TODO ignoring sx and sy now.
    float sx = 1.0;
    float sy = 1.0;

    float charW = 0, charH = 0;
    uint32_t size = textLine.pixelSize;
    setFont(textLine.font);

    // TODO: calc alignment
    if (getFontResource() != nullptr)
    {
        getFontResource()->measureTextInternal(cStr, size, sx, sy, charW, charH);
        //TODO: add clipping support here
        mw = charW > mw ? charW : mw;
        mh = charH > mh ? charH : mh;
    }

    // Now, render the text
    if( getFontResource() != nullptr)
    {
#ifdef PXSCENE_FONT_ATLAS
        pxTexturedQuads quads;
        getFontResource()->renderTextToQuads(cStr, size, sx, sy, quads, roundf(xPos), roundf(yPos));
        quads.setColor(textLine.color);
        mQuadsVector.push_back(quads);
#else
        //getFontResource()->renderText(cStr, size, xPos, tempY, sx, sy, mTextColor,lineWidth);
        rtLogError("pxTextCanvas::drawing without FONT ATLAS is not supported yet.");
#endif
    }
    rtLogDebug("pxTextCanvas::renderTextLine - end");
}

void pxTextCanvas::draw()
{
    rtLogDebug("pxTextCanvas::draw - begin.");
#ifdef PXSCENE_FONT_ATLAS
    if (mDirty)
    {
        mQuadsVector.clear();
        renderText(true);
        mDirty = false;
    }
    float x = 0, y = 0;
    for (std::vector<pxTexturedQuads>::iterator it = mQuadsVector.begin() ; it != mQuadsVector.end(); ++it)
        (*it).draw(x, y);
#else
    rtLogError("pxTextCanvas::drawing without FONT ATLAS is not supported yet.");
#endif
    rtLogDebug("pxTextCanvas::draw - end.");
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
    rtLogDebug("pxTextCanvas::update - begin.");
    if( mNeedsRecalc ) {
        recalc();
        markDirty();
    }
    pxText::update(t, updateChildren);
    rtLogDebug("pxTextCanvas::update - end.");
}

rtError pxTextCanvas::measureText(rtString text, rtObjectRef& o)
{
    rtLogDebug("pxTextCanvas::measureText - begin.");
    rtLogInfo("pxTextCanvas::measureText() mNeedsRecalc is %d text: %s", mNeedsRecalc, text.cString());
    rtObjectRef tcm = new pxTextCanvasMeasurements(); // TODO: aren't we leaking here with 'new'? Is it efficient?
    o = tcm;
    if(getFontResource() != nullptr) {
        rtObjectRef sm; // pxTextSimpleMeasurements
        getFontResource()->measureText(mPixelSize, text, sm);
        ((pxTextCanvasMeasurements*)tcm.getPtr())->fromSimpleMeasurements(sm);

        rtLogInfo("pxTextCanvas::measureText(). Got measurements; width: %d, height: %d "
                , ((pxTextCanvasMeasurements*)tcm.getPtr())->width()
                , ((pxTextCanvasMeasurements*)tcm.getPtr())->h()
                );

    } else {
        rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
        //return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
    }
    rtLogDebug("pxTextCanvas::measureText - end.");
    return RT_OK;
}

rtError pxTextCanvas::fillText(rtString text, uint32_t x, uint32_t y)
{
    rtLogDebug("pxTextCanvas::fillText - begin.");
    rtLogInfo("pxTextCanvas::fillText called with params: text: %s, x %d, y %d", text.cString(), x, y);
    pxTextLine textLine(text, x, y);
    uint32_t color;
    textColorInternal(color);
    textLine.setStyle(mFont, mPixelSize, color);
    mTextLines.push_back(textLine);
    setNeedsRecalc(true);
    rtLogDebug("pxTextCanvas::fillText - end.");
    return RT_OK;
}

rtError pxTextCanvas::clear()
{
    rtLogDebug("pxTextCanvas::clear - begin.");
    mTextLines.clear();
    mw = 0;
    mh = 0;
    setNeedsRecalc(true);
    rtLogDebug("pxTextCanvas::clear - end.");

    return RT_OK;
}

rtError pxTextCanvas::fillRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    UNUSED_PARAM(x);
    UNUSED_PARAM(y);
    UNUSED_PARAM(width);
    UNUSED_PARAM(height);
    rtLogInfo("pxTextCanvas::fillRect called with params: x %d, y %d, width %d, height %d", x, y, width, height);
    rtLogError("pxTextCanvas::fillRect. NOT IMPLEMENTED. Call ignored.");
    return RT_OK;
}

rtError pxTextCanvas::translate(uint32_t x, uint32_t y)
{
    UNUSED_PARAM(x);
    UNUSED_PARAM(y);
    rtLogInfo("pxTextCanvas::translate called with params: x %d, y %d", x, y);
    rtLogError("pxTextCanvas::translate. NOT IMPLEMENTED. Call ignored.");
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
rtDefineProperty(pxTextCanvas, shadowColor);
rtDefineProperty(pxTextCanvas, shadowBlur);
rtDefineProperty(pxTextCanvas, shadowOffsetX);
rtDefineProperty(pxTextCanvas, shadowOffsetY);
rtDefineProperty(pxTextCanvas, label);

rtDefineMethod(pxTextCanvas, measureText);
rtDefineMethod(pxTextCanvas, fillText);
rtDefineMethod(pxTextCanvas, clear);
rtDefineMethod(pxTextCanvas, fillRect);
rtDefineMethod(pxTextCanvas, translate);