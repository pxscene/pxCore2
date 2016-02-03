// pxCore CopyRight 2007-2015 John Robinson
// pxTextBox.cpp

#include "rtConstants.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "pxFileDownloader.h"
#include "pxTimer.h"
#include "pxContext.h"
extern pxContext context;
#include <math.h>
#include <map>

// TODO can we eliminate direct utf8.h usage
extern "C" {
#include "utf8.h"
}


pxTextBox::pxTextBox(pxScene2d* s):pxText(s)
{
  measurements= new pxTextMeasurements();
  
  mFontLoaded = false;
  mInitialized = false;
  mWordWrap = false;
  mEllipsis = false;
  lineNumber = 0;
  lastLineNumber = 0;
  mTruncation = rtConstantsTruncation::NONE;  
  mXStartPos = 0;
  mXStopPos = 0;
  mAlignVertical = rtConstantsAlignVertical::TOP;
  mAlignHorizontal = rtConstantsAlignHorizontal::LEFT;
  mLeading = 0;  
  mNeedsRecalc = true;

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
      pxObject::onTextureReady();
      mReady.send("reject",this);
  }    
  
}

float pxTextBox::getFBOWidth() 
{ 
  if( !clip() && mTruncation == rtConstantsTruncation::NONE && !mWordWrap) 
     return noClipW;
  else 
    return mw; 
}
float pxTextBox::getFBOHeight() 
{ 
  if( !clip() && mTruncation == rtConstantsTruncation::NONE) 
     return noClipH;
  else 
    return mh;
}

void pxTextBox::onInit()
{
  //printf("pxTextBox::onInit. mFontLoaded=%d\n",mFontLoaded);
  mInitialized = true;
  // If this is using the default font, we would not get a callback
  if(mFontLoaded || getFontResource()->isFontLoaded())
  {
    mFontLoaded = true;
    setNeedsRecalc(true);
  }
}

void pxTextBox::recalc() 
{
  if( mNeedsRecalc) {
    if (!mText || !strcmp(mText.cString(),"")) {
         clearMeasurements();
         setMeasurementBounds(mx, 0, my, 0);
       return;
    }  
    
    clearMeasurements();
    renderText(false);
    
    setNeedsRecalc(false);

  }
}
void pxTextBox::setNeedsRecalc(bool recalc) 
{ 
  //printf("Setting mNeedsRecalc=%d\n",recalc); 
  mNeedsRecalc = recalc; 

  if(recalc)
  {
    //printf("TextBox CREATE NEW PROMISE\n");
    createNewPromise();
  }

}

void pxTextBox::sendPromise() 
{ 
  //printf("pxTextBox::sendPromise mInitialized=%d mFontLoaded=%d mNeedsRecalc=%d\n",mInitialized,mFontLoaded,mNeedsRecalc);
  if(mInitialized && mFontLoaded && !mNeedsRecalc && !((rtPromise*)mReady.getPtr())->status()) 
  {
    //printf("pxTextBox SENDPROMISE\n");
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
  //printf("pxTextBox::setText %s\n",s);
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
  //printf("pxTextBox::setPixelSize %s\n",mText.cString());
  mPixelSize = v; 
  setNeedsRecalc(true); 
  return RT_OK; 
}
rtError pxTextBox::setFontUrl(const char* s)
{
  //printf("pxTextBox::setFontUrl \"%s\" mInitialized=%d\n",s,mInitialized);
  mFontLoaded = false;
  setNeedsRecalc(true);
  return pxText::setFontUrl(s);
}


rtError pxTextBox::setFont(rtObjectRef o) 
{ 
  mFontLoaded = false;
  setNeedsRecalc(true);
   
  return pxText::setFont(o);
}


void pxTextBox::draw() {
  static pxTextureRef nullMaskRef;
	if (mCached.getPtr() && mCached->getTexture().getPtr()) 
  {
    if(!clip() && mTruncation == rtConstantsTruncation::NONE)
    {
      //printf("!CLF: pxTextBox::draw() with cachedPtr && noClip values x=%f y=%f w=%f h=%f\n",noClipX,noClipY,noClipW,noClipH);
      context.drawImage(noClipX,noClipY,noClipW,noClipH,mCached->getTexture(),nullMaskRef,rtConstantsStretch::NONE,rtConstantsStretch::NONE);
    }
    else 
    {
      context.drawImage(0,0,mw,mh,mCached->getTexture(),nullMaskRef,rtConstantsStretch::NONE,rtConstantsStretch::NONE);
    }
	}
	else 
    {
	  renderText(true);
	}
  
  if (!mFontLoaded && getFontResource()->isDownloadInProgress())
    getFontResource()->raiseDownloadPriority();  
}
void pxTextBox::update(double t)
{
  //printf("pxTextBox::update: mNeedsRecalc=%d\n",mNeedsRecalc);
  if( mNeedsRecalc ) {
    //printf("pxTextBox::update: mNeedsRecalc=%d\n",mNeedsRecalc);
    //printf("pxTextBox::update: mInitialized=%d && mFontLoaded=%d\n",mInitialized, mFontLoaded);
    
    recalc();

    setNeedsRecalc(false);
    mDirty = true;
    mScene->mDirty = true;
  }  
    
  pxText::update(t);
}
/** This function needs to measure the text, taking into consideration
 *  wrapping, truncation and dimensions; but it should not render the 
 *  text yet. 
 * */
void pxTextBox::determineMeasurementBounds() 
{
  
}
void pxTextBox::clearMeasurements()
{
    lastLineNumber = 0;
    lineNumber = 0;
    noClipX = 0;
    noClipY = 0;
    noClipW = mw;
    noClipH = mh;
 //   startX = mx;
    startY = my;
    getMeasurements()->clear(); 
}

void pxTextBox::renderText(bool render)
{
  //printf("pxTextBox::renderText render=%d initialized=%d fontLoaded=%d\n",render,mInitialized,mFontLoaded);

  if( !mInitialized || !mFontLoaded) {
    return;
  }

  // These mimic the values used by pxText when it calls pxText::renderText
  float sx = 1.0; 
  float sy = 1.0;
  float tempX = mx;
  lineNumber = 0;
                  
	if (!mText || !strcmp(mText.cString(),"")) 
  {
     clearMeasurements();
     setMeasurementBounds(mx, 0, my, 0);
	   return;
	}


	if( !mWordWrap) 
  {
    //printf("calling renderTextNoWordWrap\n");
    renderTextNoWordWrap(sx, sy, tempX, render);

	} 
	else 
	{	
    renderTextWithWordWrap(mText, sx, sy, tempX, mPixelSize, mTextColor, render);

	}

}

void pxTextBox::renderTextWithWordWrap(const char *text, float sx, float sy, float tempX, uint32_t size, float* color, bool render)
{
	float tempY = 0;

  if( mAlignHorizontal == rtConstantsAlignHorizontal::LEFT && mTruncation != rtConstantsTruncation::NONE ) 
  {
    if( mXStopPos != 0) 
    {
      // TODO: if this single line won't fit when it accounts for mXStopPos, 
      // need to wrap to a new line
    }
  }


   measureTextWithWrapOrNewLine( text, sx, sy, tempX, tempY, size, color, render);

}


void pxTextBox::measureTextWithWrapOrNewLine(const char *text, float sx, float sy, float tempX, float &tempY, 
                                           uint32_t size, float* color, bool render) 
{
  //printf(">>>>>>>>>>>>>>>>>>>> pxTextBox::measureTextWithWrapOrNewLine\n");
  
	int i = 0;
	u_int32_t charToMeasure;
	float charW=0, charH=0;

  rtString accString = "";
  bool lastLine = false;
  float lineWidth = mw;

  // If really rendering, startY should reflect value for any verticalAlignment adjustments
  if( render) {
    tempY = startY;
  }
  if(lineNumber == 0) {
    if( mAlignHorizontal == rtConstantsAlignHorizontal::LEFT) 
    {
 //     setLineMeasurements(true, mXStartPos, tempY);
      tempX = mXStartPos;
    }
    else {
//      setLineMeasurements(true, tempX, tempY);
    }
  }
    // Read char by char to determine full line of text before rendering
		while((charToMeasure = u8_nextchar((char*)text, &i)) != 0) 
		{
			char tempChar[2];
			tempChar[0] = charToMeasure;
			tempChar[1] = 0;
			getFontResource()->measureTextChar(charToMeasure, size,  sx, sy, charW, charH);
			if( isNewline(charToMeasure))
      {
        // Render what we had so far in accString; since we are here, it will fit.
        renderOneLine(accString.cString(), mx, tempY, sx, sy, size, color, lineWidth, render);

        accString = "";			
				tempY += (mLeading*sy) + charH;

				lineNumber++;
        //if( tempX + charW
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
				if( lastLine) 
        {
					renderTextRowWithTruncation(accString, lineWidth, mx, tempY, sx, sy, size, color, render);
					// Clear accString because we've rendered it 
					accString = "";	
					break; // break out of reading mText

				} 
				else 
        {  // If not lastLine
          
          // Account for the case where wordWrap is off, but newline was found previously
          if( !mWordWrap && lineNumber != 0 ) {
            lastLineNumber = lineNumber;
            //printf("!!!!CLF: calling renderTextRowWithTruncation! %s\n",accString.cString());
            if( mTruncation != rtConstantsTruncation::NONE) {
              renderTextRowWithTruncation(accString, mw, mx, tempY, sx, sy, size, color, render);
              accString = ""; 
              break; 
            } else {
              if( clip()) {
                renderOneLine(accString, mx, tempY, sx, sy, size, color, mw, render);
                accString = ""; 
                break; 
              } else {
                accString.append(tempChar);
                tempX += charW;
                continue;
              }
            }
          }
          // End special case when !wordWrap but newline found
          
					// Out of space on the current line; find and wrap at word boundary
					char * tempStr = strdup(accString.cString()); // Should give a copy
					// Need to free the duplicate string?
					int length = accString.length();
					int n = length-1;
					
					while(!isWordBoundary(tempStr[n]) && n >= 0) 
          {
						n--;
					}
					if( isWordBoundary(tempStr[n])) 
          {

						tempStr[n+1] = '\0';
						// write out entire string that will fit
						// Use horizonal positioning
            //printf("Calling renderOneLine with lineNumber=%d\n",lineNumber);
            renderOneLine(tempStr, mx, tempY, sx, sy, size, color, lineWidth, render);

						// Now reset accString to hold remaining text
						tempStr = strdup(accString.cString());
						n++;
						if( strlen(tempStr+n) > 0) 
            {
							if( isSpaceChar(tempStr[n])) 
              {
								accString = tempStr+n+1;
							}
							else 
              {
								accString = tempStr+n;
							}
						} 
						else 
            { 
							accString = "";
						}
						if( !isSpaceChar(tempChar[0]) || (isSpaceChar(tempChar[0]) && accString.length() != 0)) 
            {
							accString.append(tempChar);
						}

					}
					// Now skip to next line
					tempY += (mLeading*sy) + charH;
					tempX = 0;
					lineNumber++;

					getFontResource()->measureTextInternal(accString.cString(), size, sx, sy, charW, charH);

          tempX += charW;			
          
          // If Truncation is NONE, we never want to set as last line; 
          // just keep rendering...
          if( mTruncation != rtConstantsTruncation::NONE && tempY + ((mLeading*sy) + (charH*2)) > this->h() && !lastLine) 
          {
            lastLine = true;
            if(mXStopPos != 0 && mAlignHorizontal == rtConstantsAlignHorizontal::LEFT) 
            {
                lineWidth = mXStopPos - mx;
            } 				
          } 

				}

			}

		}
		
		if(accString.length() > 0) {
      lastLineNumber = lineNumber;
      if( mTruncation == rtConstantsTruncation::NONE && !mWordWrap ) {
        //printf("CLF! Sending tempX instead of this->w(): %f\n", tempX);
        renderOneLine(accString.cString(), mx, tempY, sx, sy, size, color, tempX, render);
      } else {
        // check if we need to truncate this last line
        if( !lastLine && mXStopPos != 0 && mAlignHorizontal == rtConstantsAlignHorizontal::LEFT && mTruncation != rtConstantsTruncation::NONE && mXStopPos > mXStartPos
            && tempX > mw) {
          renderTextRowWithTruncation(accString, mXStopPos - mx, mx, tempY, sx, sy, size, color, render);
        } 
        else
        {
          renderOneLine(accString.cString(), mx, tempY, sx, sy, size, color, this->w(), render);
        }
      }
		  
		}	

 
  if( !render) {
      float metricHeight=0;
      getFontResource()->getHeight(mPixelSize,metricHeight);
      //printf("pxTextBox::renderTextWithWordWrap metricHeight is %f\n",metricHeight);
      //printf("pxTextBox::renderTextWithWordWrap lineNumer=%d\n",lineNumber);
      //printf("pxTextBox::renderTextWithWordWrap mLeading=%f\n",mLeading);
      float textHeight = tempY+metricHeight+(((charH/metricHeight)-1)*(mLeading*sy));//charH + ((charH/metricHeight)*mLeading);//((lastLineNumber+1)*metricHeight) + ((lastLineNumber-1)* mLeading); 
      //textHeight -= (mLeading*sy); // no leading after last row of text
      //printf("pxTextBox::renderTextWithWordWrap textHeight is %f\n",textHeight);
      //printf("pxTextBox::renderTextWithWordWrap tempY is %f\n",tempY);

      // NOTE that the only time mWordWrap should be false in this function
      // is when there are newline char(s) in the text being rendered.
      if( mAlignVertical == rtConstantsAlignVertical::BOTTOM ) 
      {
        if(!mWordWrap ) 
        {
          startY = my + (mh - textHeight); // could be negative
          if(!clip() && mTruncation == rtConstantsTruncation::NONE) 
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
            if(mTruncation == rtConstantsTruncation::NONE) {
              noClipH = textHeight;
              startY = my;
            }
          }
        }
      } 
      else if( mAlignVertical == rtConstantsAlignVertical::CENTER)
      {
        if(!mWordWrap ) 
        {
          startY = my+ (mh/2) - textHeight/2;
          if(!clip() && mTruncation == rtConstantsTruncation::NONE) 
          {
            noClipY = my;
            noClipH = mh;
          }
        } 
        else 
        {
          startY = my + (mh-textHeight)/2;
          if(!clip())  
          {
            noClipY = (my + (mh/2)) - textHeight/2;
            if(mTruncation == rtConstantsTruncation::NONE)
            {
              startY = my;
              noClipH = textHeight;
            }
          }
        }
      }
      else if( mAlignVertical == rtConstantsAlignVertical::TOP)
      {
        startY = my;
        if( mWordWrap && !clip() && mTruncation == rtConstantsTruncation::NONE)
        {
          noClipY = my;
          noClipH = textHeight;
          
        }
        
      }
      
      // Now set the top and bottom Y bounds
      if( !clip() && mTruncation == rtConstantsTruncation::NONE) 
      {
        setMeasurementBoundsY(true, noClipY);
        setMeasurementBoundsY(false, textHeight);        
      }
      else 
      {
        //printf("!CLF: Setting bounds: startY=%f, my=%f, textHeight=%f\n",startY, my, textHeight);
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


void pxTextBox::renderOneLine(const char * tempStr, float tempX, float tempY, float sx, float sy, uint32_t size, float* color, float lineWidth, bool render )
{
  float charW =0, charH=0;

  //printf("pxTextBox::renderOneLine tempY=%f noClipY=%f\n",tempY,noClipY);
  float xPos = tempX; 
  getFontResource()->measureTextInternal(tempStr, size, sx, sy, charW, charH);
  
  if( !clip() && mTruncation == rtConstantsTruncation::NONE) 
  {
    //printf("!CLF: Setting NoClip values in renderOneLine to noClipW=%f\n",noClipW);
    noClipW = (noClipW < charW) ? charW:noClipW;
    if( !mWordWrap)  
    {
      if( mAlignHorizontal == rtConstantsAlignHorizontal::CENTER ) 
      { 
        xPos = tempX;
        noClipX = (lineWidth/2) - charW/2;
      }
      else if( mAlignHorizontal == rtConstantsAlignHorizontal::RIGHT) 
      {
        xPos = tempX;
        noClipX = mw-charW;
      }
      else 
      {
        if( lineNumber == 0)
        {
          printf("LineNumber is 0\n");
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
      // mWordWrap is on - No Clip and No Truncation
      if( mAlignHorizontal == rtConstantsAlignHorizontal::CENTER)  
      { 
        xPos = (lineWidth/2) - charW/2;
        noClipX = tempX;
      }
      else if( mAlignHorizontal == rtConstantsAlignHorizontal::RIGHT) 
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
    // If we're here, clip could be on or off
    if( mAlignHorizontal == rtConstantsAlignHorizontal::CENTER)  
    { 
      xPos = (lineWidth/2) - charW/2;
    }
    else if( mAlignHorizontal == rtConstantsAlignHorizontal::RIGHT) 
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
    if( !clip() && mTruncation == rtConstantsTruncation::NONE) 
    {
      setMeasurementBoundsX(true, xPos);
      if( lineNumber == lastLineNumber || mTruncation == rtConstantsTruncation::NONE) {
        //printf("!CLF: calculating lineMeasurement! pixelSize=%d noClipH=%f noClipY=%f lineNumber=%d\n",mPixelSize,noClipH, noClipY, lineNumber);
        setLineMeasurements(false,xPos+charW, noClipY+(noClipH-(noClipH/(lineNumber+1))));
        setMeasurementBoundsX(false, charW );
      } 

    }
    else
    {
      setMeasurementBoundsX(true, xPos<mx?mx:xPos);
      if( mWordWrap) {
        //printf("!CLF: wordWrap true: tempY=%f, mh=%f, charH=%f\n",tempY, mh, charH);
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
      if(!clip() && mTruncation == rtConstantsTruncation::NONE) {
        //printf("!CLF lineNumber == 0 !mWordWrap !clip() && mTruncation == NONE tempX=%f xPos=%f xStartPos=%f noClipX=%f noClipW=%f charW=%f\n", tempX, xPos,mXStartPos, noClipX, noClipW, charW);
        if( noClipX != tempX) { 
          setMeasurementBounds(false, noClipX+charW, charH);
        }
        else {
          setMeasurementBounds(false, noClipX+(charW+xPos), charH);
        }
        setLineMeasurements(false, noClipX+(charW+xPos), tempY);
        setLineMeasurements(true, noClipX, tempY);
        setMeasurementBoundsX(true, noClipX);
      }
      else {
       // If we're here, clip could be on or off
       //printf("!CLF lineNumber == 0 !mWordWrap clip=%d mTruncation=%d tempX=%f xPos=%f xStartPos=%f noClipX=%f noClipW=%f charW=%f\n", clip(),mTruncation, tempX, xPos,mXStartPos, noClipX, noClipW, charW);
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
            //printf("xPos+charW >mw lineWidth=%f\n",lineWidth);
            setMeasurementBounds(false, mw-xPos, charH);
          } 
          else {
            //printf("else not xPos+charW >mw lineWidth=%f\n",lineWidth);
            setMeasurementBounds(false, (xPos+width) > mw? mw:xPos+width, charH);
          }
        }
        setLineMeasurements(false, (xPos+width) > mw? mw:xPos+width, tempY);

      }
    }
    else 
    {
      // mWordWrap is true and lineNumber==0
      if( !clip() && mTruncation == rtConstantsTruncation::NONE)
      {
        //printf("!CLF: No clip here we go: noClipY=%f my=%f, tempY=%f, noClipH=%f\n",noClipY,my, tempY,noClipH);
        //printf("!CLF: No clip here we go: noClipX=%f mx=%f, tempX=%f, noClipW=%f\n",noClipX,mx, tempX,noClipW);
          
          // Set last line and bounds measurements in case there's only one line
          setLineMeasurements(false,xPos+charW,noClipY);//tempY);
            
          //setMeasurementBoundsX(true, noClipX);//xPos);
          //setLineMeasurements(true, noClipX, noClipY);//xPos, tempY);

          setLineMeasurements(true, xPos, noClipY);
          setMeasurementBoundsX(true, xPos);
          setMeasurementBoundsX(false, noClipW);//charW );  
     

      }
      else         
      {
        //printf("!CLF: Here we go: xPos=%f mx=%f, tempX=%f, lineWidth=%f, charW=%f mw=%f\n",xPos,mx, tempX,lineWidth, charW, mw);
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
  if( render) {getFontResource()->renderText(tempStr, size, xPos, tempY, sx, sy, color,lineWidth);} 
 

}

void pxTextBox::setMeasurementBoundsY(bool start, float yVal) {

  rtRefT<pxTextBounds> bounds = getMeasurements()->getBounds();  
  //printf("pxTextBox::setMeasurementBoundsY: start=%d yVal=%f and current vals y1=%f y2=%f\n",start, yVal,bounds->y1(),bounds->y2());  
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
  //printf("pxTextBox::setMeasurementBoundsX: start=%d xVal=%f already set to %f\n",start, xVal,bounds->x2());
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
  //printf("pxTextBox::setMeasurementBounds: start=%d xVal=%f yVal%f\n",start, xVal,yVal);
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
  //printf("pxTextBox::setMeasurementBounds\n");
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
  //printf("pxTextBox::setLineMeasurements firstLine=%d xPos=%f yPos=%f\n", firstLine, xPos, yPos);
  float height=0, ascent=0, descent=0, naturalLeading=0;
  getFontResource()->getMetrics(mPixelSize, height, ascent, descent, naturalLeading);
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
  //printf("pxTextBox::renderTextNoWordWrap render=%d\n",render);
  
  float charW=0, charH=0;
  float lineWidth = this->w();
  float tempXStartPos = tempX;
  float tempY = my;
  
  if( mAlignHorizontal == rtConstantsAlignHorizontal::LEFT) {
    tempXStartPos = mXStartPos;

  }
  
  // Measure as single line since there's no word wrapping
  getFontResource()->measureTextInternal(mText, mPixelSize, sx, sy, charW, charH);
  //printf(">>>>>>>>>>>> pxTextBox::renderTextNoWordWrap charH=%f charW=%f\n", charH, charW);

  float metricHeight=0;
  getFontResource()->getHeight(mPixelSize, metricHeight);
  //printf(">>>>>>>>>>>>>> metric height is %f and charH is %f\n", metricHeight, charH);
  if( charH > metricHeight) // There's a newline in the text somewhere
  {
    lineNumber = 0;
    measureTextWithWrapOrNewLine(mText, sx, sy, tempX, render?startY:tempY, mPixelSize, mTextColor, render);
  }
  else 
  {
    // Calculate vertical alignment values
    if( mAlignVertical == rtConstantsAlignVertical::BOTTOM || mAlignVertical == rtConstantsAlignVertical::CENTER) 
    {
      if( mAlignVertical == rtConstantsAlignVertical::BOTTOM ) 
      {
        tempY = my + (mh - charH); // could be negative
      } 
      else 
      {
        tempY = my+ (mh/2) - charH/2;
      }
    }
    if( mTruncation == rtConstantsTruncation::NONE && !clip() && charH > mh) {
      noClipH = charH;
      noClipW = (noClipW < charW) ? charW:noClipW;
    }
    // Will it fit on one line OR is there no truncation, so we don't care...
    if( (charW + tempXStartPos) <= lineWidth || mTruncation == rtConstantsTruncation::NONE) 
    {
      lastLineNumber = lineNumber = 0;
      //printf("pxTextBox::renderTextNoWordWrap setLineMeasurements tempXStartPos=%f tempY=%f before renderOneLine\n",tempXStartPos, tempY);
      setLineMeasurements(true, tempXStartPos, tempY);
      setMeasurementBounds(true, tempXStartPos, tempY);

      renderOneLine(mText, tempX, tempY, sx, sy, mPixelSize, mTextColor, lineWidth, render); 
    
    }      
    else 
    { 
      // Do we really need to check for truncation? It has to be one of these
      // since we checked for NONE above.
      if(mTruncation == rtConstantsTruncation::TRUNCATE || mTruncation == rtConstantsTruncation::TRUNCATE_AT_WORD) 
      {
          renderTextRowWithTruncation(mText, lineWidth, tempX, tempY, sx, sy, mPixelSize, mTextColor, render);
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
                                          float sx, float sy, uint32_t pixelSize, float* color, bool render) 
{
  //printf(">>>>>>>>>>>>>>>>>>pxTextBox::renderTextRowWithTruncation lineNumber = %d render = %d\n",lineNumber, render);
  //printf(">>>>>>>>>>>>>>>>>>pxTextBox::renderTextRowWithTruncation tempY = %f tempX = %f\n",tempY, tempX);
  //printf(">>>>>>>>>>>>>>>>>>pxTextBox::renderTextRowWithTruncation lineWidth = %f \n",lineWidth);
  
	float charW=0, charH=0;
	char * tempStr = strdup(accString.cString()); // Should give a copy
	int length = accString.length();
	float ellipsisW = 0;
	if( mEllipsis) 
  {
		length -= ELLIPSIS_LEN;
		getFontResource()->measureTextInternal(ELLIPSIS_STR, pixelSize, sx, sy, ellipsisW, charH);
		//printf("ellipsisW is %f\n",ellipsisW);
	}
  
  // Make adjustments for H_LEFT xStartPos and xStopPos, as applicable
  if( mAlignHorizontal == rtConstantsAlignHorizontal::LEFT)
  {
    if( lineNumber == 0) {
      tempX = mXStartPos;
    }
    // Adjust line width when stop pos is authored (and using H_LEFT)
    if( mTruncation != rtConstantsTruncation::NONE && mXStopPos != 0 && mXStopPos > tempX) 
    {
      lineWidth = mXStopPos - tempX;
    }    
  }

  
	for(int i = length; i > 0; i--) 
  { 
		tempStr[i] = '\0';
		charW = 0;
		charH = 0;
		getFontResource()->measureTextInternal(tempStr, pixelSize, sx, sy, charW, charH);
		if( (tempX + charW + ellipsisW) <= lineWidth) 
    {
			float xPos = tempX;  
			if( mTruncation == rtConstantsTruncation::TRUNCATE) 
      {
				// we're done; just render
				// Ignore xStartPos and xStopPos if H align is not LEFT
				if( mAlignHorizontal == rtConstantsAlignHorizontal::CENTER )
        { 
					xPos = (lineWidth/2) - ((charW+ellipsisW)/2);
				}	
        else if( mAlignHorizontal == rtConstantsAlignHorizontal::RIGHT)
        {
          xPos =  lineWidth - charW - ellipsisW;
        }	
        
        if(!mWordWrap) {setMeasurementBounds(xPos, charW, tempY, charH); }
        setLineMeasurements(false, xPos+charW, tempY);
        if( lineNumber==0) {setLineMeasurements(true, xPos, tempY);}

        if( render) {
          getFontResource()->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, color,lineWidth);
        } 
				if( mEllipsis) 
        {
					//printf("rendering truncated text with ellipsis\n");
					if( render) {
            getFontResource()->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, color,lineWidth);
          } 
          if(!mWordWrap) { setMeasurementBounds(xPos, charW+ellipsisW, tempY, charH); }
          setLineMeasurements(false, xPos+charW+ellipsisW, tempY);
				}
				break;
			} 
			else if( mTruncation == rtConstantsTruncation::TRUNCATE_AT_WORD)
      {
				// Look for word boundary on which to break
				int n = 1;
				while(!isWordBoundary(tempStr[i-n]) && n <= i) 
        {
					n++;
				}
										
				if( isWordBoundary(tempStr[i-n])) 
        {
					tempStr[i-n+1] = '\0';
					getFontResource()->measureTextInternal(tempStr, pixelSize, sx, sy, charW, charH);

          // Ignore xStartPos and xStopPos if H align is not LEFT
					if( mAlignHorizontal == rtConstantsAlignHorizontal::CENTER) 
          { 
						xPos = (lineWidth/2) - ((charW+ellipsisW)/2);
					}	
					else if( mAlignHorizontal == rtConstantsAlignHorizontal::RIGHT) 
          {
						xPos = lineWidth - charW - ellipsisW;
					}					
          if(!mWordWrap) { setMeasurementBounds(xPos, charW, tempY, charH); }
          setLineMeasurements(false, xPos+charW, tempY); 		
          if( lineNumber==0) {setLineMeasurements(true, xPos, tempY);	}							
					if( render){
            getFontResource()->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, color,lineWidth);
          }
				}
				if( mEllipsis) 
        {
					//printf("rendering  text on word boundary with ellipsis\n");
					if( render) {
            getFontResource()->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, color,lineWidth);
          }
          if(!mWordWrap) { setMeasurementBounds(xPos, charW+ellipsisW, tempY, charH); }
          setLineMeasurements(false, xPos+charW+ellipsisW, tempY);
				}
				break;
			}

		}
		
	}	
}

bool pxTextBox::isWordBoundary( char ch )
{
    return (strchr(word_boundary_chars, ch) != NULL);
}
bool pxTextBox::isSpaceChar( char ch ) 
{
	return (strchr(space_chars, ch) != NULL);
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
    return (strchr(isnew_line_chars, ch) != 0);
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
  ////printf("pxTextBox::getFontMetrics\n");  
  
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
  //printf("pxTextBox::measureText() mNeedsRecalc=%d text=%s\n",mNeedsRecalc,mText.cString());
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
  //printf("measurement is %f, %f\n",measurements->getCharLast()->x(), measurements->getCharLast()->y());
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
