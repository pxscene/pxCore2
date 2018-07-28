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

// pxTextBox.cpp

#include "pxConstants.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "rtFileDownloader.h"
#include "pxTimer.h"
#include "pxContext.h"

extern pxContext context;
#include <math.h>
#include <map>
#include <stdlib.h>

static const char      isNewline_chars[] = "\n\v\f\r";
static const char isWordBoundary_chars[] = " \t/:&,;.";
static const char    isSpaceChar_chars[] = " \t";

#define ELLIPSIS_STR u8"\u2026"

#if 1
// TODO can we eliminate direct utf8.h usage
extern "C" {
#include "../../../src/utf8.h"
}
#endif


pxTextBox::pxTextBox(pxScene2d* s): pxText(s),
                                    mTruncation(pxConstantsTruncation::NONE),
                                    mAlignVertical(pxConstantsAlignVertical::TOP),
                                    mAlignHorizontal(pxConstantsAlignHorizontal::LEFT),
                                    mXStartPos(0),  mXStopPos(0), mLeading(0), 
                                    mWordWrap(false), mEllipsis(false), mInitialized(false), mNeedsRecalc(true),
                                    lineNumber(0), lastLineNumber(0),
                                    noClipX(0), noClipY(0), noClipW(0), noClipH(0), startY(0)
{
  measurements= new pxTextMeasurements();

  mFontLoaded      = false;
  mFontFailed      = false;

}

/** This signals that the font file loaded successfully; now we need to
 * send the ready promise once we have the text, too
 */
void pxTextBox::resourceReady(rtString readyResolution)
{
  if( !readyResolution.compare("resolve"))
  {
    mFontLoaded = true;

    if( mInitialized) {
      setNeedsRecalc(true);
      pxObject::onTextureReady();
    }
  }
  else
  {
      mFontFailed = true;
      pxObject::onTextureReady();
      mReady.send("reject",this);
  }
}

float pxTextBox::getFBOWidth()
{
  if( !clip() && mTruncation == pxConstantsTruncation::NONE && !mWordWrap) {
     if( noClipW > MAX_TEXTURE_WIDTH) rtLogWarn("Text width is larger than maximum texture allowed: %lf.  Maximum texture size of %d will be used.",noClipW, MAX_TEXTURE_WIDTH);
     return noClipW > MAX_TEXTURE_WIDTH?MAX_TEXTURE_WIDTH:noClipW;
  }
  else 
  {
    return pxText::getFBOWidth();
  }
}
float pxTextBox::getFBOHeight()
{
  if( !clip() && mTruncation == pxConstantsTruncation::NONE) {
    if( noClipH > MAX_TEXTURE_HEIGHT) rtLogWarn("Text height is larger than maximum texture allowed: %lf.  Maximum texture size of %d will be used.",noClipH, MAX_TEXTURE_HEIGHT);
    return noClipH > MAX_TEXTURE_HEIGHT?MAX_TEXTURE_HEIGHT:noClipH;
  }
  else
  {
    return pxText::getFBOHeight();
  }
}

void pxTextBox::onInit()
{
  //rtLogDebug("pxTextBox::onInit. mFontLoaded=%d\n",mFontLoaded);
  mInitialized = true;
  // If this is using the default font, we would not get a callback
  if(mFontLoaded || (getFontResource() != NULL && getFontResource()->isFontLoaded()))
  {
    mFontLoaded = true;
    setNeedsRecalc(true);
  }
}

void pxTextBox::recalc()
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


    // always do renderText(true) here rather than 
    // waiting for draw() call so that even when the 
    // textBox or its parent has draw=false, the measurements
    // get calculated and the promise gets resolved.
#ifdef PXSCENE_FONT_ATLAS
      mQuadsVector.clear();
#endif
      renderText(true);
      mDirty = false;

  }
}
void pxTextBox::setNeedsRecalc(bool recalc)
{
  //rtLogDebug("Setting mNeedsRecalc=%d\n",recalc);
  mNeedsRecalc = recalc;

  if(recalc)
  {
    //rtLogDebug("TextBox CREATE NEW PROMISE\n");
    createNewPromise();
    //mDirty = true;
  }

}

void pxTextBox::sendPromise()
{
  //rtLogDebug("pxTextBox::sendPromise mInitialized=%d mFontLoaded=%d mNeedsRecalc=%d\n",mInitialized,mFontLoaded,mNeedsRecalc);
if(mInitialized && mFontLoaded && !mNeedsRecalc && !mDirty && !((rtPromise*)mReady.getPtr())->status())
  {
    //rtLogDebug("pxTextBox SENDPROMISE\n");
    mReady.send("resolve",this);
  }
}

/**
 * setText: for pxTextBox, setText sets the text value, but does not
 * affect the dimensions of the object.  Dimensions are respected
 * and text is wrapped/truncated within those dimensions according
 * to other properties.
 **/
rtError pxTextBox::setText(const char* s) {
  //rtLogDebug("pxTextBox::setText %s\n",s);
  if( !mText.compare(s)){
    rtLogDebug("pxTextBox.setText setting to same value %s and %s\n", mText.cString(), s);
    return RT_OK;
  }
  mText = s;
  setNeedsRecalc(true);
  return RT_OK;
}

rtError pxTextBox::setPixelSize(uint32_t v)
{
  //rtLogDebug("pxTextBox::setPixelSize %s\n",mText.cString());
  mPixelSize = v;
  setNeedsRecalc(true);
  return RT_OK;
}
rtError pxTextBox::setFontUrl(const char* s)
{
  //rtLogDebug("pxTextBox::setFontUrl \"%s\" mInitialized=%d\n",s,mInitialized);
  mFontFailed = false;
  mFontLoaded = false;
  setNeedsRecalc(true);
  return pxText::setFontUrl(s);
}


rtError pxTextBox::setFont(rtObjectRef o)
{
  mFontFailed = false;
  mFontLoaded = false;
  setNeedsRecalc(true);

  return pxText::setFont(o);
}


void pxTextBox::draw() 
{
#ifdef PXSCENE_FONT_ATLAS
  if (mDirty)
  {
    mQuadsVector.clear();
    renderText(true);
    mDirty = false;
  
  }
  float x = 0, y = 0;
  if(!clip() && mTruncation == pxConstantsTruncation::NONE) {
    x = roundf(noClipX); 
    y = roundf(noClipY); 
  }

  for (std::vector<pxTexturedQuads>::iterator it = mQuadsVector.begin() ; it != mQuadsVector.end(); ++it)
    (*it).draw(x, y, mTextColor);


#else
  static pxTextureRef nullMaskRef;
  if (mCached.getPtr() && mCached->getTexture().getPtr())
  {
    if(!clip() && mTruncation == pxConstantsTruncation::NONE)
    {
      //rtLogDebug("!CLF: pxTextBox::draw() with cachedPtr && noClip values x=%f y=%f w=%f h=%f\n",noClipX,noClipY,noClipW,noClipH);
      context.drawImage(noClipX,noClipY,noClipW,noClipH,mCached->getTexture(),nullMaskRef,false);
    }
    else
    {
      context.drawImage(0,0,getFBOWidth(),getFBOHeight(),mCached->getTexture(),nullMaskRef, true);
    }
  }
  else
  {
    renderText(true);
    mDirty = false;
  }

  //if (!mFontLoaded && getFontResource()->isDownloadInProgress())
    //getFontResource()->raiseDownloadPriority();
#endif
}
void pxTextBox::update(double t)
{
  pxText::update(t);	 

  if( mNeedsRecalc ) {

     recalc();

   }

}
/** This function needs to measure the text, taking into consideration
 *  wrapping, truncation and dimensions; but it should not render the
 *  text yet.
 * */
void pxTextBox::clearMeasurements()
{
    lastLineNumber = 0;
    lineNumber = 0;
    noClipX = 0;
    noClipY = 0;
    noClipW = mw;
    noClipH = mh;
 //   startX = mx;
    startY = 0;
    getMeasurements()->clear();
}

void pxTextBox::renderText(bool render)
{
  //rtLogDebug("pxTextBox::renderText render=%d initialized=%d fontLoaded=%d\n",render,mInitialized,mFontLoaded);

  if( !mInitialized || !mFontLoaded) 
  {
    return;
  }

  // These mimic the values used by pxText when it calls pxText::renderText
  float sx = 1.0;
  float sy = 1.0;
  float tempX = 0;//mx;
  lineNumber = 0;
  
  if (!mText || !strcmp(mText.cString(),""))
  {
     clearMeasurements();
     setMeasurementBounds(mx, 0, my, 0);
     return;
  }


  if( !mWordWrap)
  {
    rtLogDebug("calling renderTextNoWordWrap\n");
    //startY = 0;
    renderTextNoWordWrap(sx, sy, tempX, render);

  }
  else
  {
    renderTextWithWordWrap(mText, sx, sy, tempX, mPixelSize, render);
  }
}

void pxTextBox::renderTextWithWordWrap(const char *text, float sx, float sy, float tempX, uint32_t size, bool render)
{
  // TODO ignoring sx and sy now
  sx = 1.0;
  sy = 1.0;
  float tempY = 0;

  if( mAlignHorizontal == pxConstantsAlignHorizontal::LEFT && mTruncation != pxConstantsTruncation::NONE )
  {
    if( mXStopPos != 0)
    {
      // TODO: if this single line won't fit when it accounts for mXStopPos,
      // need to wrap to a new line
    }
  }
   measureTextWithWrapOrNewLine( text, sx, sy, tempX, tempY, size, render);
}


void pxTextBox::measureTextWithWrapOrNewLine(const char *text, float sx, float sy, float tempX, float &tempY,
                                           uint32_t size, bool render)
{
    // TODO ignoring sx and sy now
    sx = 1.0;
    sy = 1.0;
    //rtLogDebug(">>>>>>>>>>>>>>>>>>>> pxTextBox::measureTextWithWrapOrNewLine\n");

    u_int32_t charToMeasure;
    float charW=0, charH=0;

    rtString accString = "";
    bool lastLine = false;
    float lineWidth = mw;

    // If really rendering, startY should reflect value for any verticalAlignment adjustments
    if( render) 
    {
      tempY = startY;
    }
    if(lineNumber == 0) 
    {
      if( mAlignHorizontal == pxConstantsAlignHorizontal::LEFT)
      {
   //     setLineMeasurements(true, mXStartPos, tempY);
        tempX = mXStartPos;
      }
      else {
  //      setLineMeasurements(true, tempX, tempY);
      }
    }
    
    // Read char by char to determine full line of text before rendering
    int i = 0;
    int lasti = 0;
    int numbytes = 1;
    char* tempChar = NULL;
    while((charToMeasure = u8_nextchar((char*)text, &i)) != 0)
    {
      // Determine if the character is multibyte
      numbytes = i-lasti;
      if (tempChar != NULL)
      {
        delete [] tempChar;
      }
      tempChar = new char[numbytes+1];
      memset(tempChar, '\0', sizeof(char)*(numbytes+1));
      if(numbytes == 1) {
        tempChar[0] = charToMeasure;
      } 
      else {
        for( int pos = 0; pos < numbytes ; pos++) {
          tempChar[pos] = text[lasti+pos];
        }
      }
      lasti = i;

      if (getFontResource() != NULL)
      {
        getFontResource()->measureTextChar(charToMeasure, size, sx, sy, charW, charH);
      }
      if( isNewline(charToMeasure))
      {
        //rtLogDebug("Found NEWLINE; calling renderOneLine\n");
        // Render what we had so far in accString; since we are here, it will fit.
        renderOneLine(accString.cString(), 0, tempY, sx, sy, size, lineWidth, render);

        accString = "";
        tempY += (mLeading*sy) + charH;

        lineNumber++;
        tempX = 0;
        continue;
      }

      // Check if text still fits on this line, or if wrap needs to occur
      if( (tempX + charW) <= lineWidth || (!mWordWrap && lineNumber == 0))
      {
        accString.append(tempChar);
        tempX += charW;
      }
      else
      {
        // The text in hand will not fit on the current line, so prepare
        // to render what we've got and skip to next line.
        // Note: Last line will never be set when truncation is NONE.
        if( lastLine || (mTruncation != pxConstantsTruncation::NONE && (tempY + ((mLeading*sy) + charH) >= this->h())) )
        {
          //rtLogDebug("LastLine: Calling renderTextRowWithTruncation with mx=%f for string \"%s\"\n",mx,accString.cString());
          renderTextRowWithTruncation(accString, lineWidth, 0, tempY, sx, sy, size, render);
          // Clear accString because we've rendered it
          accString = "";
          break; // break out of reading mText

        }
        else  // If NOT lastLine
        {
          // Account for the case where wordWrap is off, but newline was found previously
          if( !mWordWrap && lineNumber != 0 )
          {
            lastLineNumber = lineNumber;
            //rtLogDebug("!!!!CLF: calling renderTextRowWithTruncation! %s\n",accString.cString());
            if( mTruncation != pxConstantsTruncation::NONE) {
              renderTextRowWithTruncation(accString, mw, mx, tempY, sx, sy, size, render);
              accString = "";
              break;
            }
            else
            {
              if( clip() )
              {
                renderOneLine(accString, 0, tempY, sx, sy, size, mw, render);
                accString = "";
                break;
              }
              else
              {
                accString.append(tempChar);
                tempX += charW;
                continue;
              }
            }
          }
          // End special case when !wordWrap but newline found

          // Out of space on the current line; find and wrap at word boundary
          char *tempStr = strdup(accString.cString()); // Should give a copy
          int    length = accString.length();
          int         n = length-1;

          while(!isWordBoundary(tempStr[n]) && n >= 0)
          {
            n--;
          }
          if( isWordBoundary(tempStr[n]))
          {
            tempStr[n+1] = '\0';
            // write out entire string that will fit
            // Use horizonal positioning
            //rtLogDebug("Calling renderOneLine with lineNumber=%d\n",lineNumber);
            renderOneLine(tempStr, 0, tempY, sx, sy, size, lineWidth, render);
            free(tempStr);

            // Now reset accString to hold remaining text
            tempStr = strdup(accString.cString());
            n++;

            if( strlen(tempStr+n) > 0)
            {
              if( isSpaceChar(tempStr[n]))
              {
                //rtLogDebug("Attempting to move past leading space at %d in string \"%s\"\n",n, tempStr);
                accString = tempStr+n+1;
              }
              else
              {
                //rtLogDebug("Is not leading space at %d in string \"%s\"\n",n, tempStr);
                accString = tempStr+n;
                //rtLogDebug("So accString is now \"%s\"\n",accString.cString());
              }
            }
            else
            {
              accString = "";
            }

            if( !isSpaceChar(tempChar[0]) || (isSpaceChar(tempChar[0]) && accString.length() != 0))
            {
              //rtLogDebug("space char check to add to string: \"%s\"\n",accString.cString());
              //rtLogDebug("space char check: \"%s\"\n",tempChar);
              accString.append(tempChar);
            }
            
          }

          delete [] tempChar;
          tempChar = NULL;

          // Free tempStr
          free(tempStr);
          // Now skip to next line
          tempY += (mLeading*sy) + charH;
          tempX = 0;
          lineNumber++;

          if (getFontResource() != NULL)
          {
            getFontResource()->measureTextInternal(accString.cString(), size, sx, sy, charW, charH);
          }

          tempX += charW;

          // If Truncation is NONE, we never want to set as last line;
          // just keep rendering...
          if( mTruncation != pxConstantsTruncation::NONE && tempY + ((mLeading*sy) + (charH*2)) > this->h() && !lastLine)
          {
            lastLine = true;
            if(mXStopPos != 0 && mAlignHorizontal == pxConstantsAlignHorizontal::LEFT)
            {
                lineWidth = mXStopPos - mx;
            }
          }
        }
      }
    }//WHILE
    if (tempChar != NULL)
    {
      delete [] tempChar;
      tempChar = NULL;
    }

    if(accString.length() > 0) {
      lastLineNumber = lineNumber;
      if( mTruncation == pxConstantsTruncation::NONE && !mWordWrap ) {
        //rtLogDebug("CLF! Sending tempX instead of this->w(): %f\n", tempX);
        renderOneLine(accString.cString(), 0, tempY, sx, sy, size, tempX, render);
      } else {
        // check if we need to truncate this last line
        if( !lastLine && mXStopPos != 0 && mAlignHorizontal == pxConstantsAlignHorizontal::LEFT
            && mTruncation != pxConstantsTruncation::NONE && mXStopPos > mXStartPos
            && tempX > mw) {
          renderTextRowWithTruncation(accString, mXStopPos - mx, mx, tempY, sx, sy, size, render);
        }
        else
        {
          renderOneLine(accString.cString(), 0, tempY, sx, sy, size, this->w(), render);
        }
      }

    }


  if( !render) {
      float metricHeight=0;
      if (getFontResource() != NULL)
      {
        getFontResource()->getHeight(mPixelSize, metricHeight);
      }
      //rtLogDebug("pxTextBox::renderTextWithWordWrap metricHeight is %f\n",metricHeight);
      //rtLogDebug("pxTextBox::renderTextWithWordWrap lineNumer=%d\n",lineNumber);
      //rtLogDebug("pxTextBox::renderTextWithWordWrap mLeading=%f\n",mLeading);
      float textHeight = tempY+metricHeight+(((charH/metricHeight)-1)*(mLeading*sy));//charH + ((charH/metricHeight)*mLeading);//((lastLineNumber+1)*metricHeight) + ((lastLineNumber-1)* mLeading);
      //textHeight -= (mLeading*sy); // no leading after last row of text
      //rtLogDebug("pxTextBox::renderTextWithWordWrap textHeight is %f\n",textHeight);
      //rtLogDebug("pxTextBox::renderTextWithWordWrap tempY is %f\n",tempY);

      // NOTE that the only time mWordWrap should be false in this function
      // is when there are newline char(s) in the text being rendered.
      if( mAlignVertical == pxConstantsAlignVertical::BOTTOM )
      {
        if(!mWordWrap )
        {
          startY = my + (mh - textHeight); // could be negative
          if(!clip() && mTruncation == pxConstantsTruncation::NONE)
          {
            noClipY = my;
            noClipH = textHeight;//mh;
          }
        }
        else
        {
          startY = my + (mh - textHeight); // could be negative
          if(!clip())
          {
            noClipY = my-(textHeight-mh);
            if(mTruncation == pxConstantsTruncation::NONE) {
              noClipH = textHeight;
              startY = 0;//my;
            }
          }
        }
      }
      else if( mAlignVertical == pxConstantsAlignVertical::CENTER)
      {
        if(!mWordWrap )
        {
          startY = my + (mh - textHeight)/2;
          if(!clip() && mTruncation == pxConstantsTruncation::NONE)
          {
            noClipY = my;
            noClipH = textHeight;
          }
        }
        else
        {
          startY = my + (mh - textHeight)/2;
          if(!clip())
          {
            noClipY = my + (mh - textHeight)/2;
            if(mTruncation == pxConstantsTruncation::NONE)
            {
              startY = 0;//my;
              noClipH = textHeight;
            }
          }
        }
      }
      else if( mAlignVertical == pxConstantsAlignVertical::TOP)
      {
        startY = 0;//my; // This fixes XRE2-85 clip:true wordWrap:true thin sliver of text shown
        if( //mWordWrap &&
          !clip() && mTruncation == pxConstantsTruncation::NONE)
        {
          noClipY = 0;//my; // This fixes clip:true wordWrap:true y position additive of my
          noClipH = textHeight;

        }
        //else if( !mWordWrap && !clip() && mTruncation == pxConstantsTruncation::NONE
                 //&& lastLineNumber != 0)
        //{
          //noClipH = textHeight;
        //}

      }

      // Now set the top and bottom Y bounds
      if( !clip() && mTruncation == pxConstantsTruncation::NONE)
      {
        setMeasurementBoundsY(true, noClipY);
        setMeasurementBoundsY(false, textHeight);
      }
      else
      {
        //rtLogDebug("!CLF: Setting bounds: startY=%f, my=%f, textHeight=%f\n",startY, my, textHeight);
        if(startY < my) {
          setMeasurementBoundsY(true, my);
          setMeasurementBoundsY(false, textHeight>mh?mh:textHeight);
        }
        else {
          setMeasurementBoundsY(true, startY);
          setMeasurementBoundsY(false, textHeight>mh?mh:textHeight);
        }
      }
  }
}


void pxTextBox::renderOneLine(const char * tempStr, float tempX, float tempY, float sx, float sy, uint32_t size, float lineWidth, bool render )
{
  // TODO ignoring sx and sy now.
  sx = 1.0;
  sy = 1.0;

  float charW =0, charH=0;

  //rtLogDebug("pxTextBox::renderOneLine tempY=%f noClipY=%f tempStr=%s\n",tempY,noClipY, tempStr);
  float xPos = tempX;
  if (getFontResource() != NULL)
  {
    getFontResource()->measureTextInternal(tempStr, size, sx, sy, charW, charH);
  }

  if( !clip() && mTruncation == pxConstantsTruncation::NONE)
  {
    //rtLogDebug("!CLF: Setting NoClip values in renderOneLine to noClipW=%f\n",noClipW);
    noClipW = (noClipW < charW) ? charW:noClipW;
    if( !mWordWrap)
    {
      // If any one line exceeds texture maximums, warn and calculate text that will fit
      if( charW > MAX_TEXTURE_WIDTH) 
      {
        rtLogWarn("Text width is larger than maximum texture allowed: %lf.  Maximum texture size of %d will be used.",charW, MAX_TEXTURE_WIDTH);
        float tempWidthRatio = charW/MAX_TEXTURE_WIDTH;
        uint32_t strLen = strlen(tempStr);
        uint32_t tempNewLen = static_cast<uint32_t> (strLen/tempWidthRatio);

        char* trimmedTempStr = (char *)malloc(tempNewLen+1);
        memset(trimmedTempStr,'\0',tempNewLen+1);

        uint32_t tmpPos = 0;
        if( mAlignHorizontal == pxConstantsAlignHorizontal::CENTER )
          tmpPos = ((uint32_t) strlen(tempStr)/2)-(tempNewLen/2); // Take middle of tempStr
        else if( mAlignHorizontal == pxConstantsAlignHorizontal::RIGHT)
          tmpPos = ((uint32_t) strlen(tempStr)-(tempNewLen)); // Take end of tempStr

        strncpy(trimmedTempStr, tempStr+tmpPos,tempNewLen);
        getFontResource()->measureTextInternal(trimmedTempStr, size, sx, sy, charW, charH);
        noClipW = charW;
        // Render with new, trimmed string
        renderOneLine(trimmedTempStr, tempX, tempY, sx, sy, size, lineWidth, render);

        free(trimmedTempStr);   
        return;
      }   


      if( mAlignHorizontal == pxConstantsAlignHorizontal::CENTER )
      {
        if( charW < noClipW) {
             xPos = (lineWidth - charW)/2;
          noClipX = tempX;
        }
        else {
             xPos = tempX;
          noClipX = (lineWidth - charW)/2;
        }
      }
      else if( mAlignHorizontal == pxConstantsAlignHorizontal::RIGHT)
      {
        if( charW < noClipW)
        {
             xPos = mw-charW;
          noClipX = tempX;
        }
        else
        {
             xPos = tempX;
          noClipX = mw-charW;
        }
      }
      else
      {
        if( lineNumber == 0)
        {
          //rtLogDebug("LineNumber is 0\n");
          xPos = tempX;
          noClipX = mXStartPos;
        }
        else
        {
             xPos = tempX;
          noClipX = tempX;
        }
      }

    }
    else
    {
      // mWordWrap is "ON" >>>  No Clip and No Truncation
      if( mAlignHorizontal == pxConstantsAlignHorizontal::CENTER)
      {
           xPos = (lineWidth - charW)/2;
        noClipX = tempX;
      }
      else if( mAlignHorizontal == pxConstantsAlignHorizontal::RIGHT)
      {
           xPos = lineWidth - charW;
        noClipX = tempX;
      }
      else
      {
        if( lineNumber == 0)
        {
             xPos = mXStartPos;
          noClipX = tempX;
        }
        else
        {
             xPos = tempX;
          noClipX = tempX;
        }
      }
    }
  }
  else
  {
    // If we're here, clip could be "ON" or "OFF"
    if( mAlignHorizontal == pxConstantsAlignHorizontal::CENTER)
    {
       xPos = (lineWidth - charW)/2;
    }
    else if( mAlignHorizontal == pxConstantsAlignHorizontal::RIGHT)
    {
       xPos = lineWidth - charW;
    }
    else
    {
      if(lineNumber==0)
      {
        xPos = mXStartPos;
      }
    }
  }


   //If text is being clipped, then the last line measurements may not be the same as
   //when lineNumber==lastLineNumber.  Check now.
  if( lineNumber != 0)
  {
    if( !clip() && mTruncation == pxConstantsTruncation::NONE)
    {
      setMeasurementBoundsX(true, xPos);
      if( lineNumber == lastLineNumber || mTruncation == pxConstantsTruncation::NONE)
      {
        //rtLogDebug("!CLF: calculating lineMeasurement! charH=%f pixelSize=%d noClipH=%f noClipY=%f lineNumber=%d\n",charH,mPixelSize,noClipH, noClipY, lineNumber);
        setLineMeasurements(false, xPos+charW, noClipY+noClipH-(noClipH/(lineNumber+1)));
        setMeasurementBoundsX(false, charW );
      }
    }
    else
    {
      setMeasurementBoundsX(true, xPos<mx?mx:xPos);
      if( mWordWrap) {
        //rtLogDebug("!CLF: wordWrap true: tempY=%f, mh=%f, charH=%f\n",tempY, mh, charH);
        if( tempY + charH <= mh) {
          setLineMeasurements(false,xPos+charW, tempY);
        }
        setMeasurementBoundsX(false, charW);
      }
      else {
        setLineMeasurements(false, xPos+charW > mw? lineWidth: xPos+charW, tempY);
        setMeasurementBoundsX(false, charW > mw? lineWidth: charW );
      }
    }

  }


  if( lineNumber == 0)
  {
    if(!mWordWrap)
    {
      // Set start/stop line and bounds values because this is the only
      // line of text there is...
      // !CLF:  TODO:  What if there are newlines within the text, and
      // that's how we got here with mWordWrap==false?
      if(!clip() && mTruncation == pxConstantsTruncation::NONE) {
        //rtLogDebug("!CLF lineNumber == 0 !mWordWrap !clip() && mTruncation == NONE tempX=%f xPos=%f xStartPos=%f noClipX=%f noClipW=%f charW=%f\n", tempX, xPos,mXStartPos, noClipX, noClipW, charW);
        if( noClipX != tempX) {
          setMeasurementBounds(false, noClipX+charW, charH);
          setMeasurementBoundsX(true, noClipX);
          setLineMeasurements(true, noClipX, tempY);
        }
        else {
          setMeasurementBounds(false, noClipX+(charW+xPos), charH);
          if( charW < lineWidth) {
            setMeasurementBoundsX(true, xPos);
            setLineMeasurements(true, xPos, tempY);
          } else {
            setMeasurementBoundsX(true, noClipX);
            setLineMeasurements(true, noClipX, tempY);
          }
        }
        setLineMeasurements(false, noClipX+(charW+xPos), tempY);
      }
      else {
       // If we're here, clip could be on or off
       //rtLogDebug("!CLF lineNumber == 0 !mWordWrap clip=%d mTruncation=%d tempX=%f xPos=%f xStartPos=%f noClipX=%f noClipW=%f charW=%f\n", clip(),mTruncation, tempX, xPos,mXStartPos, noClipX, noClipW, charW);
       float width = charW;
       if( clip() && charW > lineWidth) {
         width = lineWidth;// respect max
       }

        if( xPos != tempX) {
          setLineMeasurements(true, xPos<mx?mx:xPos, tempY);
          setMeasurementBoundsX(true, xPos<mx?mx:xPos);
          setMeasurementBounds(false, (xPos+width) > mw? mw:width, charH);
        }
        else {
          if( xPos+width > mw && (xPos+lineWidth) > mw) {
            //rtLogDebug("xPos+charW >mw lineWidth=%f\n",lineWidth);
            setMeasurementBounds(false, mw-xPos, charH);
          }
          else {
            //rtLogDebug("else not xPos+charW >mw lineWidth=%f\n",lineWidth);
            setMeasurementBounds(false, (xPos+width) > mw? mw:xPos+width, charH);
          }
        }
        if( !clip())
          setLineMeasurements(false, width > mw? mw:width, tempY);
        else {
          float tmpX = xPos<mx?mx:xPos;
          setLineMeasurements(false, (tmpX+width) > mw? mw:tmpX+width, tempY);
        }
      }
    }
    else
    {
      // mWordWrap is true and lineNumber==0
      if( !clip() && mTruncation == pxConstantsTruncation::NONE)
      {
        //rtLogDebug("!CLF: No clip here we go: noClipY=%f my=%f, tempY=%f, noClipH=%f\n",noClipY,my, tempY,noClipH);
        //rtLogDebug("!CLF: No clip here we go: noClipX=%f mx=%f, tempX=%f, noClipW=%f\n",noClipX,mx, tempX,noClipW);

          // Set last line and bounds measurements in case there's only one line
          setLineMeasurements(false,xPos+charW,noClipY);//tempY);

          //setMeasurementBoundsX(true, noClipX);//xPos);
          //setLineMeasurements(true, noClipX, noClipY);//xPos, tempY);

          setLineMeasurements(true, xPos, noClipY);
          setMeasurementBoundsX(true, xPos);
          //rtLogDebug("setMeasurementBounds is using noClipW of %f\n",noClipW);
          //rtLogDebug("setMeasurementBounds charW of %f\n",charW);
          setMeasurementBoundsX(false, charW);//noClipW);//charW );  // Fix x2 bounds issue
      }
      else
      {
        //rtLogDebug("!CLF: Here we go: xPos=%f mx=%f, tempX=%f, lineWidth=%f, charW=%f mw=%f\n",xPos,mx, tempX,lineWidth, charW, mw);
        setMeasurementBoundsX(true, xPos<mx?mx:xPos);
        setLineMeasurements(true, xPos<mx?mx:xPos, tempY< my?my:tempY);
        if( charW > mw && (xPos+lineWidth) > mw) {
          setMeasurementBoundsX(false, mw-xPos );
        }
        else {
          setMeasurementBoundsX(false, charW > mw? mw:charW );
        }
        setLineMeasurements(false, xPos+charW, tempY);
      }
    }
  }

  // Now, render the text
  if( render && getFontResource() != NULL)
  {
 #ifdef PXSCENE_FONT_ATLAS
     pxTexturedQuads quads;
     getFontResource()->renderTextToQuads(tempStr, size, sx, sy, quads, roundf(xPos), roundf(tempY));
     mQuadsVector.push_back(quads);
 #else
   getFontResource()->renderText(tempStr, size, xPos, tempY, sx, sy, mTextColor,lineWidth);
#endif
  }
}

void pxTextBox::setMeasurementBoundsY(bool start, float yVal) {

  rtRefT<pxTextBounds> bounds = getMeasurements()->getBounds();
  //rtLogDebug("pxTextBox::setMeasurementBoundsY: start=%d yVal=%f and current vals y1=%f y2=%f\n",start, yVal,bounds->y1(),bounds->y2());
  if( start) {
    if( bounds->y1()== 0 || bounds->y1() > yVal) {
        bounds->setY1(yVal);
    }
  }
  else {
    if( bounds->y2() < (bounds->y1() + yVal)) {
       bounds->setY2(bounds->y1() + yVal);
    }
  }
}
void pxTextBox::setMeasurementBoundsX(bool start, float xVal)
{

  rtRefT<pxTextBounds> bounds = getMeasurements()->getBounds();
  //rtLogDebug("pxTextBox::setMeasurementBoundsX: start=%d xVal=%f already set to %f\n",start, xVal,bounds->x2());
  if( start) {
    if( bounds->x1() == 0 || (bounds->x1() > xVal)) {
      bounds->setX1(xVal);
    }
  }
  else {
    if( bounds->x2() < (bounds->x1() + xVal) ) {
      bounds->setX2(bounds->x1() + xVal);
    }
  }
}

void pxTextBox::setMeasurementBounds(bool start, float xVal, float yVal)
{
  rtRefT<pxTextBounds> bounds = getMeasurements()->getBounds();
  //rtLogDebug("pxTextBox::setMeasurementBounds: start=%d xVal=%f yVal%f\n",start, xVal,yVal);
  if( start) {
    if( bounds->x1() == 0 || (bounds->x1() > xVal)) {
      bounds->setX1(xVal);
    }
    if( bounds->y1()== 0 || bounds->y1() > yVal) {
      bounds->setY1(yVal);
    }
  }
  else {
    if( bounds->x2() < (bounds->x1() + xVal) ) {
      bounds->setX2(bounds->x1() + xVal);
    }
    if( bounds->y2() < (bounds->y1() + yVal)) {
       bounds->setY2(bounds->y1() + yVal);
    }
  }
}

void pxTextBox::setMeasurementBounds(float xPos, float width, float yPos, float height)
{
  //rtLogDebug("pxTextBox::setMeasurementBounds\n");
  // Set the bounds for the text
  rtRefT<pxTextBounds> bounds = getMeasurements()->getBounds();
  if( bounds->x2() < (xPos + width) ) {
    bounds->setX2(xPos + width);
  }
  if( bounds->x1() == 0 || (bounds->x1() > xPos)) {
    bounds->setX1(xPos);
  }
  if( bounds->y2() < (yPos + height)) {
     bounds->setY2(yPos + height);
  }
  if( bounds->y1()== 0 || bounds->y1() > yPos) {
    bounds->setY1(yPos);
  }

}

void pxTextBox::setLineMeasurements(bool firstLine, float xPos, float yPos)
{
  //rtLogDebug("pxTextBox::setLineMeasurements firstLine=%d xPos=%f yPos=%f\n", firstLine, xPos, yPos);
  float height=0, ascent=0, descent=0, naturalLeading=0;
  if (getFontResource() != NULL)
  {
    getFontResource()->getMetrics(mPixelSize, height, ascent, descent, naturalLeading);
  }
  
  if(!firstLine) {
    getMeasurements()->getCharLast()->setX(xPos);
    getMeasurements()->getCharLast()->setY(yPos + ascent);
  } else {
    getMeasurements()->getCharFirst()->setX(xPos);
    getMeasurements()->getCharFirst()->setY(yPos + ascent);
  }
}

/**
 * renderTextNoWordWrap: To be used when wordWrap is false
 * */
void pxTextBox::renderTextNoWordWrap(float sx, float sy, float tempX, bool render)
{
  //TODO ignoring sx and sy now
  sx = 1.0;
  sy = 1.0;
  //rtLogDebug("pxTextBox::renderTextNoWordWrap render=%d\n",render);

  float charW=0, charH=0;
  float lineWidth = this->w();
  float tempXStartPos = tempX;
  float tempY = 0;//my;

  if( mAlignHorizontal == pxConstantsAlignHorizontal::LEFT) {
    tempXStartPos = mXStartPos;

  }

  // Measure as single line since there's no word wrapping
  if (getFontResource() != NULL)
  {
    getFontResource()->measureTextInternal(mText, mPixelSize, sx, sy, charW, charH);
  }
  //rtLogDebug(">>>>>>>>>>>> pxTextBox::renderTextNoWordWrap charH=%f charW=%f\n", charH, charW);

  float metricHeight=0;
  if (getFontResource() != NULL)
  {
    getFontResource()->getHeight(mPixelSize, metricHeight);
  }
  //rtLogDebug(">>>>>>>>>>>>>> metric height is %f and charH is %f\n", metricHeight, charH);
  
  if( charH > metricHeight) // There's a newline in the text somewhere
  {
    lineNumber = 0;
    noClipH = charH;
 //   noClipW = charW;
    float tempY = 0;
    measureTextWithWrapOrNewLine(mText, sx, sy, tempX, tempY, mPixelSize, render);
  }
  else
  {
    // Calculate vertical alignment values
    if( mAlignVertical == pxConstantsAlignVertical::BOTTOM || mAlignVertical == pxConstantsAlignVertical::CENTER)
    {
      if( mAlignVertical == pxConstantsAlignVertical::BOTTOM )
      {
        tempY = my + (mh - charH); // could be negative    // BOTTOM
      }
      else
      {
//         tempY = my+ (mh/2) - charH/2;
        tempY = (mh - charH)/2; // could be negative  // CENTER
      }
    }
    if( mTruncation == pxConstantsTruncation::NONE && !clip() && charH > mh)
    {
      noClipH = charH;
      noClipW = (noClipW < charW) ? charW:noClipW;
    }
    // Will it fit on one line OR is there no truncation, so we don't care...
    if( mTruncation == pxConstantsTruncation::NONE || (charW + tempXStartPos) <= lineWidth)
    {
      lastLineNumber = lineNumber = 0;
      //rtLogDebug("pxTextBox::renderTextNoWordWrap setLineMeasurements tempXStartPos=%f tempY=%f before renderOneLine\n",tempXStartPos, tempY);
      setLineMeasurements(true, tempXStartPos, tempY);
      setMeasurementBounds(true, tempXStartPos, tempY);

      renderOneLine(mText, tempX, tempY, sx, sy, mPixelSize, lineWidth, render);
    }
    else
    {
      // Do we really need to check for truncation? It has to be one of these
      // since we checked for NONE above.
      if(mTruncation == pxConstantsTruncation::TRUNCATE || mTruncation == pxConstantsTruncation::TRUNCATE_AT_WORD)
      {
          renderTextRowWithTruncation(mText, lineWidth, tempX, tempY, sx, sy, mPixelSize, render);
      }
    }
  }
}

/**
 * renderTextRowWithTruncation: Only to be called when truncation != NONE
 *
 * vAlign information is set prior to the call of this function, so we should
 * be able to depend on it for setting bounds and start/stop positions.
 * */
void pxTextBox::renderTextRowWithTruncation(rtString & accString, float lineWidth, float tempX, float tempY,
                                          float sx, float sy, uint32_t pixelSize, bool render)
{
  //rtLogDebug(">>>>>>>>>>>>>>>>>>pxTextBox::renderTextRowWithTruncation lineNumber = %d render = %d\n",lineNumber, render);
  //rtLogDebug(">>>>>>>>>>>>>>>>>>pxTextBox::renderTextRowWithTruncation tempY = %f tempX = %f\n",tempY, tempX);
  //rtLogDebug(">>>>>>>>>>>>>>>>>>pxTextBox::renderTextRowWithTruncation lineWidth = %f \n",lineWidth);
  // TODO ignoring sx and sy now
  sx = 1.0;
  sy = 1.0;

  float charW=0, charH=0;
  char * tempStr = strdup(accString.cString()); // Should give a copy
  int length = accString.length();
  float ellipsisW = 0;
  if( mEllipsis)
  {
    // Determine ellipsis width in pixels
    if (getFontResource() != NULL)
    {
      getFontResource()->measureTextInternal(ELLIPSIS_STR, pixelSize, sx, sy, ellipsisW, charH);
    }
    //rtLogDebug("ellipsisW is %f\n",ellipsisW);
  }

  // Make adjustments for H_LEFT xStartPos and xStopPos, as applicable
  if( mAlignHorizontal == pxConstantsAlignHorizontal::LEFT)
  {
    if( lineNumber == 0) {
      tempX = mXStartPos;
    }
    // Adjust line width when stop pos is authored (and using H_LEFT)
    if( mTruncation != pxConstantsTruncation::NONE && mXStopPos != 0 && mXStopPos > tempX)
    {
      lineWidth = mXStopPos - tempX;
    }
  }

 
    // TODO this looks pretty inefficient... We should revisit...
  for(int i = length; i > 0; i--)
  {
    // eliminate a utf8 character to see if new string width fits
    tempStr[u8_offset(tempStr,i)] = '\0';
    charW = 0;
    charH = 0;
    if (getFontResource() != NULL)
    {
      getFontResource()->measureTextInternal(tempStr, pixelSize, sx, sy, charW, charH);
    }
	
    if( (tempX + charW + ellipsisW) <= lineWidth)
    {
      float xPos = tempX;
      if( mTruncation == pxConstantsTruncation::TRUNCATE)
      {
        // we're done; just render
        // Ignore xStartPos and xStopPos if H align is not LEFT
        if( mAlignHorizontal == pxConstantsAlignHorizontal::CENTER )
        {
          xPos = (lineWidth - (charW + ellipsisW))/2;
        }
        else if( mAlignHorizontal == pxConstantsAlignHorizontal::RIGHT)
        {
          xPos =  lineWidth - charW - ellipsisW;
        }

        if(!mWordWrap) {setMeasurementBounds(xPos, charW, tempY, charH); }
        else { setMeasurementBoundsX(false, charW);}
        setLineMeasurements(false, xPos+charW, tempY);
        if( lineNumber==0) {setLineMeasurements(true, xPos, tempY);}

        if( render && getFontResource() != NULL) {
#ifdef PXSCENE_FONT_ATLAS
          pxTexturedQuads quads;
          getFontResource()->renderTextToQuads(tempStr, pixelSize, sx, sy, quads, roundf(xPos), roundf(tempY));
          mQuadsVector.push_back(quads);
#else
          getFontResource()->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, mTextColor,lineWidth);
#endif       
        }
        if( mEllipsis)
        {
          //rtLogDebug("rendering truncated text with ellipsis\n");
          if( render && getFontResource() != NULL) {
#ifdef PXSCENE_FONT_ATLAS
            pxTexturedQuads quads;  
            getFontResource()->renderTextToQuads(ELLIPSIS_STR, pixelSize, sx, sy, quads, roundf(xPos+charW), roundf(tempY));
            mQuadsVector.push_back(quads);
#else
            getFontResource()->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, mTextColor,lineWidth);
#endif          
          }
          if(!mWordWrap) { setMeasurementBounds(xPos, charW+ellipsisW, tempY, charH); }
          else { setMeasurementBoundsX(false, charW+ellipsisW);}
          setLineMeasurements(false, xPos+charW+ellipsisW, tempY);
        }
        break;
      }
      else if( mTruncation == pxConstantsTruncation::TRUNCATE_AT_WORD)
      {
        // Look for word boundary on which to break
        int n = 1;
        // rtLogDebug("Start looking for wordBoundary at length %d in \"%s\"\n",length,tempStr);
        // check if we're already at a word boundary in the original string
        if( mEllipsis && (accString.find(length-1," ")==length || accString.find(length-1,"\t")==length) ) 
        {
          //rtLogDebug("Start looking for wordBoundary char= %s in \"%s\"\n",accString.substring(length-1,1).cString(),accString.cString());
          n = 0;
        } 
        else 
        {
          while(!isWordBoundary(tempStr[i-n]) && n <= i)
          {
            n++;
          }
        }

        if( isWordBoundary(tempStr[i-n]))
        {
          tempStr[i-n+1] = '\0';
          if (getFontResource() != NULL)
          {
            getFontResource()->measureTextInternal(tempStr, pixelSize, sx, sy, charW, charH);
          }

          // Ignore xStartPos and xStopPos if H align is not LEFT
          if( mAlignHorizontal == pxConstantsAlignHorizontal::CENTER)
          {
            xPos = (lineWidth - (charW+ellipsisW))/2;
          }
          else if( mAlignHorizontal == pxConstantsAlignHorizontal::RIGHT)
          {
            xPos = lineWidth - charW - ellipsisW;
          }
          if(!mWordWrap){ setMeasurementBounds(xPos, charW, tempY, charH); }
          else { setMeasurementBoundsX(false, charW);}
          setLineMeasurements(false, xPos+charW, tempY);
          if( lineNumber==0) {setLineMeasurements(true, xPos, tempY);  }
          if( render && getFontResource() != NULL)
          {
#ifdef PXSCENE_FONT_ATLAS
            pxTexturedQuads quads;
            getFontResource()->renderTextToQuads(tempStr, pixelSize, sx, sy, quads, roundf(xPos), roundf(tempY));
            mQuadsVector.push_back(quads);
#else
            getFontResource()->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, mTextColor,lineWidth);
#endif          
          }
        }
        if( mEllipsis)
        {
          //rtLogDebug("rendering  text on word boundary with ellipsis\n");
          if( render && getFontResource() != NULL) {
#ifdef PXSCENE_FONT_ATLAS
            pxTexturedQuads quads;
            getFontResource()->renderTextToQuads(ELLIPSIS_STR, pixelSize, sx, sy, quads, roundf(xPos+charW), roundf(tempY));
            mQuadsVector.push_back(quads);
#else
            getFontResource()->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, mTextColor,lineWidth);
#endif         
          }
          if(!mWordWrap) { setMeasurementBounds(xPos, charW+ellipsisW, tempY, charH); }
          else { setMeasurementBoundsX(false, charW+ellipsisW);}
          setLineMeasurements(false, xPos+charW+ellipsisW, tempY);
        }
        break;
      }
    }
  }//FOR

  if(tempStr)
    free(tempStr);
  tempStr = NULL;
}

bool pxTextBox::isWordBoundary( char ch )
{
    return (strchr(isWordBoundary_chars, ch) != NULL);
}
bool pxTextBox::isSpaceChar( char ch )
{
  return (strchr(isSpaceChar_chars, ch) != NULL);
}
/*-------------------------------------------------------------------------
 * the following characters are considered as white-space
 * ' '  (0x20)  space (SPC)
 * '\t' (0x09)  horizontal tab (TAB)
 * '\n' (0x0a)  newline (LF)
 * '\v' (0x0b)  vertical tab (VT)
 * '\f' (0x0c)  feed (FF)
 * '\r' (0x0d)  carriage return (CR)
 -------------------------------------------------------------------------*/

bool pxTextBox::isNewline( char ch )
{
    return (strchr(isNewline_chars, ch) != 0);
}
/*
#### getFontMetrics - returns information about the font (font and size).  It does not convey information about the text of the font.
* See section 3.a in http://www.freetype.org/freetype2/docs/tutorial/step2.html .
* The returned object has the following properties:
* height - float - the distance between baselines
* ascent - float - the distance from the baseline to the font ascender (note that this is a hint, not a solid rule)
* descent - float - the distance from the baseline to the font descender  (note that this is a hint, not a solid rule)
*/
//rtError pxTextBox::getFontMetrics(rtObjectRef& o) {

  //if(!mInitialized || !mFontLoaded) {
    //rtLogWarn("getFontMetrics called TOO EARLY -- not initialized or font not loaded!\n");

    //return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
  //}
  ////rtLogDebug("pxTextBox::getFontMetrics\n");

  //getFontResource()->getFontMetrics(mPixelSize, o);
  //// set Baseline relative to my
////  pxTextMetrics* metrics = (pxTextMetrics*)o.getPtr();
////  metrics->setBaseline(metrics->baseline()+my);
////  o = metrics;

  //return RT_OK;
//}

/**
 * #### measureText – returns an object with the following properties (measurements are relative to (x,y) of the text object):
 * bounds - object - {x1:0, y1:0, x2:0, y2:0} - The two points representing the bounding rectangle of the complete text
 * charFirst - {x:0, y:0} - The x position represents the left most rendered pixel of the first character on the first line of text.
 *                          The y position represents the baseline.
 * charLast - {x:0, y:0} -  The x position represents the right most rendered pixel of the last character on the last line of text.
 *                          The y position represents the baseline.
 * */
rtError pxTextBox::measureText(rtObjectRef& o) {
  //rtLogDebug("pxTextBox::measureText() mNeedsRecalc=%d text=%s\n",mNeedsRecalc,mText.cString());
  if( mNeedsRecalc) {
    if(!mInitialized || !mFontLoaded) {
      rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
      o = measurements;
      return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
    }
    recalc();
    mNeedsRecalc = true;  // Hack to leave this set so that promise will be issued, as necessary
  }
  //pxTextMeasurements* measure = new pxTextMeasurements();
  //o = measure;
  //rtLogDebug("measurement is %f, %f\n",measurements->getCharLast()->x(), measurements->getCharLast()->y());
  o = measurements;

  return RT_OK;
}

// pxTextBounds
rtDefineObject(pxTextBounds, rtObject);
rtDefineProperty(pxTextBounds, x1);
rtDefineProperty(pxTextBounds, y1);
rtDefineProperty(pxTextBounds, x2);
rtDefineProperty(pxTextBounds, y2);
// pxCharPosition
rtDefineObject(pxCharPosition, rtObject);
rtDefineProperty(pxCharPosition, x);
rtDefineProperty(pxCharPosition, y);
// pxTextMeasurements
rtDefineObject(pxTextMeasurements, rtObject);
rtDefineProperty(pxTextMeasurements, bounds);
rtDefineProperty(pxTextMeasurements, charFirst);
rtDefineProperty(pxTextMeasurements, charLast);
// pxTextBox
rtDefineObject(pxTextBox, pxText);
rtDefineProperty(pxTextBox, wordWrap);
rtDefineProperty(pxTextBox, ellipsis);
rtDefineProperty(pxTextBox, xStartPos);
rtDefineProperty(pxTextBox, xStopPos);
rtDefineProperty(pxTextBox, truncation);
rtDefineProperty(pxTextBox, alignVertical);
rtDefineProperty(pxTextBox, alignHorizontal);
rtDefineProperty(pxTextBox, leading);
//rtDefineMethod(pxTextBox, getFontMetrics);
rtDefineMethod(pxTextBox, measureText);
