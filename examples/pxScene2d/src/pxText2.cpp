// pxCore CopyRight 2007-2015 John Robinson
// pxText2.cpp

#include "pxText.h"
#include "pxText2.h"
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


pxText2::pxText2(pxScene2d* s):pxText(s)
{
  measurements= new pxTextMeasurements(s);
  
  mFontLoaded = false;
  mInitialized = false;
  mWordWrap = false;
  mEllipsis = false;
  lineNumber = 0;
  lastLineNumber = 0;
  mTruncation = false;  
  mXStartPos = 0;
  mXStopPos = 0;
  mVerticalAlign = 0;
  mHorizontalAlign = 0;
  mLeading = 0;  
  mNeedsRecalc = true;

}

pxText2::~pxText2()
{
	 delete measurements;
   measurements = 0;
}

/** This signals that the font file loaded successfully; now we need to 
 * send the ready promise once we have the text, too
 */
void pxText2::fontLoaded() 
{
  printf("pxText2::fontLoaded. Initialized=%d\n",mInitialized);
  mFontLoaded = true;
  if( mInitialized) {
    setNeedsRecalc(true);
    // This repaint logic is necessary for clearing FBO if
    // clipping is on
    repaint();
    pxObject* parent = mParent;
    while (parent)
    {
     parent->repaint();
     parent = parent->parent();
    }    
  }
}

float pxText2::getFBOWidth() 
{ 
  if( !clip() && mTruncation == NONE && !mWordWrap) 
     return noClipW;
  else 
    return mw; 
}
float pxText2::getFBOHeight() 
{ 
  if( !clip() && mTruncation == NONE) 
     return noClipH;
  else 
    return mh;
}

void pxText2::onInit()
{
//  printf("pxText2::onInit. mFontLoaded=%d\n",mFontLoaded);
  mInitialized = true;
  if( mFontLoaded) {
    setNeedsRecalc(true);
 
  }
}

void pxText2::recalc() 
{
  if( mNeedsRecalc) {
    if (!mText || !strcmp(mText.cString(),"")) {
         clearMeasurements();
         setMeasurementBounds(mx, 0, my, 0);
       return;
    }  
    
    clearMeasurements();
    renderText(0);
    renderText(2);
    
    setNeedsRecalc(false);

  }
}
void pxText2::setNeedsRecalc(bool recalc) 
{ 
  //printf("Setting mNeedsRecalc=%d\n",recalc); 
  mNeedsRecalc = recalc; 

  //if(recalc && static_cast<rtPromise *>(mReady.getPtr())->status()) 
  //{
    ////printf("pxText2::setNeedsRecalc deleting old promise\n");
    //static_cast<rtPromise *>(mReady.getPtr())->Release(); 

    //mReady = new rtPromise;
 
  //}
  
}
/** 
 * setText: for pxText2, setText sets the text value, but does not
 * affect the dimensions of the object.  Dimensions are respected 
 * and text is wrapped/truncated within those dimensions according
 * to other properties.
 **/
rtError pxText2::setText(const char* s) { 
  //printf("pxText2::setText %s\n",s);
  rtString str(s);
  setCloneProperty("text",str);
  /*mText = s;*/
  return RT_OK; 
}

rtError pxText2::setPixelSize(uint32_t v) 
{
  setCloneProperty("pixelSize",v);
  /*mPixelSize = v;*/
  return RT_OK; 
}
rtError pxText2::setFaceURL(const char* s)
{
  /*mFontLoaded = false;*/
  rtString str(s);
  setCloneProperty("faceUrl",str);
  return RT_OK;
  //return pxText::setFaceURL(s);
}


void pxText2::draw() {
  if (mDirty)
  {
    // TODO make this configurable
    if (mText.length() >= 10)
    {
      mCached = NULL;
      pxContextFramebufferRef cached = context.createFramebuffer(getFBOWidth(),getFBOHeight());//mw,mh);
      if (cached.getPtr())
      {
        pxContextFramebufferRef previousSurface = context.getCurrentFramebuffer();
        context.setFramebuffer(cached);
        pxMatrix4f m;
        context.setMatrix(m);
        context.setAlpha(1.0);
        context.clear(mw,mh);
        renderText(1);
        context.setFramebuffer(previousSurface);
        mCached = cached;
      }
    }
    else mCached = NULL;

    mDirty = false;
  }

  static pxTextureRef nullMaskRef;
	if (mCached.getPtr() && mCached->getTexture().getPtr()) 
  {
    if(!clip() && mTruncation == NONE)
    {
      //printf("!CLF: pxText2::draw() with cachedPtr and noClip values x=%f y=%f w=%f h=%f\n",noClipX,noClipY,noClipW,noClipH);
      context.drawImage(noClipX,noClipY,noClipW,noClipH,mCached->getTexture(),nullMaskRef,PX_NONE,PX_NONE);
    }
    else 
    {
      context.drawImage(0,0,mw,mh,mCached->getTexture(),nullMaskRef,PX_NONE,PX_NONE);
    }
	}
	else {
	  renderText(1);

	}
}
void pxText2::update(double t)
{
  /*
  //printf("pxText2::update: mNeedsRecalc=%d\n",mNeedsRecalc);
  if( mNeedsRecalc ) {
//    printf("pxText2::update: mNeedsRecalc=%d\n",mNeedsRecalc);
//    printf("pxText2::update: mInitialized=%d && mFontLoaded=%d\n",mInitialized, mFontLoaded);
    recalc();

    setNeedsRecalc(false);
    mDirty = true;
    if(mInitialized && mFontLoaded ) {
      //printf("pxText2::update: FIRING PROMISE\n");
      mReady.send("resolve", this);
    }
  } */
    
  pxObject::update(t);
}
/** This function needs to measure the text, taking into consideration
 *  wrapping, truncation and dimensions; but it should not render the 
 *  text yet. 
 * */
void pxText2::determineMeasurementBounds() 
{
  
}
void pxText2::clearMeasurements()
{
    lastLineNumber = 0;
    lineNumber = 0;
    noClipX = mx;
    noClipY = my;
    noClipW = mw;
    noClipH = mh;
 //   startX = mx;
    startY = my;
    measurements->clear(); 
}

void pxText2::renderText(uint8_t render)
{
  //printf("pxText2::renderText render=%d initialized=%d fontLoaded=%d\n",render,mInitialized,mFontLoaded);

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

void pxText2::renderTextWithWordWrap(const char *text, float sx, float sy, float tempX, uint32_t size, float* color, uint8_t render)
{
	float tempY = 0;

  if( mHorizontalAlign == H_LEFT && mTruncation != NONE ) 
  {
    if( mXStopPos != 0) 
    {
      // TODO: if this single line won't fit when it accounts for mXStopPos, 
      // need to wrap to a new line
    }
  }


   measureTextWithWrapOrNewLine( text, sx, sy, tempX, tempY, size, color, render);

}


void pxText2::measureTextWithWrapOrNewLine(const char *text, float sx, float sy, float tempX, float &tempY, 
                                           uint32_t size, float* color, uint8_t render) 
{
  //printf(">>>>>>>>>>>>>>>>>>>> pxText2::measureTextWithWrapOrNewLine\n");
  
	int i = 0;
	u_int32_t charToMeasure;
	float charW, charH;

  rtString accString = "";
  bool lastLine = false;
  float lineWidth = mw;
  
  bool reallyRendering = (render==1);
  //printf("reallyRendering is %d\n",reallyRendering);

  // If really rendering, startY should reflect value for any verticalAlignment adjustments
  if( render == 1 || render == 2) { // This could be either render==1 || render==2
    tempY = startY;
  }
  if(lineNumber == 0) {
    if( mHorizontalAlign == H_LEFT) 
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
			mFace->measureTextChar(charToMeasure, size,  sx, sy, charW, charH);
			if( isNewline(charToMeasure))
      {
        // Render what we had so far in accString; since we are here, it will fit.
        renderOneLine(accString.cString(), mx, tempY, sx, sy, size, color, lineWidth, reallyRendering);

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
					renderTextRowWithTruncation(accString, lineWidth, mx, tempY, sx, sy, size, color, reallyRendering);
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
            if( mTruncation != NONE) {
              renderTextRowWithTruncation(accString, mw, mx, tempY, sx, sy, size, color, reallyRendering);
              accString = ""; 
              break; 
            } else {
              if( clip()) {
                renderOneLine(accString, mx, tempY, sx, sy, size, color, mw, reallyRendering);
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
            renderOneLine(tempStr, mx, tempY, sx, sy, size, color, lineWidth, reallyRendering);

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

					mFace->measureText(accString.cString(), size, sx, sy, charW, charH);

          tempX += charW;			
          
          // If Truncation is NONE, we never want to set as last line; 
          // just keep rendering...
          if( mTruncation != NONE && tempY + ((mLeading*sy) + (charH*2)) > this->h() && !lastLine) 
          {
            lastLine = true;
            if(mXStopPos != 0 && mHorizontalAlign == H_LEFT) 
            {
                lineWidth = mXStopPos - mx;
            } 				
          } 

				}

			}

		}
		
		if(accString.length() > 0) {
      lastLineNumber = lineNumber;
      if( mTruncation == NONE && !mWordWrap ) {
        //printf("CLF! Sending tempX instead of this->w(): %f\n", tempX);
        renderOneLine(accString.cString(), mx, tempY, sx, sy, size, color, tempX, reallyRendering);
      } else {
        // check if we need to truncate this last line
        if( !lastLine && mXStopPos != 0 && mHorizontalAlign == H_LEFT && mTruncation != NONE && mXStopPos > mXStartPos
            && tempX > mw) {
          renderTextRowWithTruncation(accString, mXStopPos - mx, mx, tempY, sx, sy, size, color, reallyRendering);
        } 
        else
        {
          renderOneLine(accString.cString(), mx, tempY, sx, sy, size, color, this->w(), reallyRendering);
        }
      }
		  
		}	

 
  if( render == 0) {
      float metricHeight;
      mFace->getHeight(mPixelSize,metricHeight);
      //printf("pxText2::renderTextWithWordWrap metricHeight is %f\n",metricHeight);
      //printf("pxText2::renderTextWithWordWrap lineNumer=%d\n",lineNumber);
      //printf("pxText2::renderTextWithWordWrap mLeading=%f\n",mLeading);
      float textHeight = tempY+metricHeight+(((charH/metricHeight)-1)*(mLeading*sy));//charH + ((charH/metricHeight)*mLeading);//((lastLineNumber+1)*metricHeight) + ((lastLineNumber-1)* mLeading); 
      //textHeight -= (mLeading*sy); // no leading after last row of text
      //printf("pxText2::renderTextWithWordWrap textHeight is %f\n",textHeight);
      //printf("pxText2::renderTextWithWordWrap tempY is %f\n",tempY);

      // NOTE that the only time mWordWrap should be false in this function
      // is when there are newline char(s) in the text being rendered.
      if( mVerticalAlign == V_BOTTOM ) 
      {
        if(!mWordWrap ) 
        {
          startY = my + (mh - textHeight); // could be negative
          if(!clip() && mTruncation == NONE) 
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
            if(mTruncation == NONE) {
              noClipH = textHeight;
              startY = my;
            }
          }
        }
      } 
      else if( mVerticalAlign == V_CENTER)
      {
        if(!mWordWrap ) 
        {
          startY = my+ (mh/2) - textHeight/2;
          if(!clip() && mTruncation == NONE) 
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
            if(mTruncation == NONE)
            {
              startY = my;
              noClipH = textHeight;
            }
          }
        }
      }
      else if( mVerticalAlign == V_TOP)
      {
        startY = my;
        if( mWordWrap && !clip() && mTruncation == NONE)
        {
          noClipY = my;
          noClipH = textHeight;
          
        }
        
      }
      
      // Now set the top and bottom Y bounds
      if( !clip() && mTruncation == NONE) 
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


void pxText2::renderOneLine(const char * tempStr, float tempX, float tempY, float sx, float sy, uint32_t size, float* color, float lineWidth, uint8_t render )
{
  float charW; float charH;

  //printf("pxText2::renderOneLine tempY=%f noClipY=%f\n",tempY,noClipY);
  float xPos = tempX; 
  mFace->measureText(tempStr, size, sx, sy, charW, charH);
  
  if( !clip() && mTruncation == NONE) 
  {
    //printf("!CLF: Setting NoClip values in renderOneLine to noClipW=%f\n",noClipW);
    noClipW = charW;
    if( !mWordWrap)  
    {
      if( mHorizontalAlign == H_CENTER ) 
      { 
        xPos = tempX;
        noClipX = (lineWidth/2) - charW/2;
      }
      else if( mHorizontalAlign == H_RIGHT) 
      {
        xPos = tempX;
        noClipX = mw-charW;
      }
      else 
      {
        if( lineNumber == 0)
        {
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
      if( mHorizontalAlign == H_CENTER ) 
      { 
        xPos = (lineWidth/2) - charW/2;
        noClipX = tempX;
      }
      else if( mHorizontalAlign == H_RIGHT) 
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
    if( mHorizontalAlign == H_CENTER ) 
    { 
      xPos = (lineWidth/2) - charW/2;
    }
    else if( mHorizontalAlign == H_RIGHT) 
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
    if( !clip() && mTruncation == NONE) 
    {
      setMeasurementBoundsX(true, xPos);
      if( lineNumber == lastLineNumber || mTruncation == NONE) {
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
      if(!clip() && mTruncation == NONE) {
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
      if( !clip() && mTruncation == NONE)
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
  if( render == 1) {mFace->renderText(tempStr, size, xPos, tempY, sx, sy, color,lineWidth);} 
 

}

void pxText2::setMeasurementBoundsY(bool start, float yVal) {

  rtRefT<pxTextBounds> bounds = measurements->getBounds();  
  //printf("pxText2::setMeasurementBoundsY: start=%d yVal=%f and current vals y1=%f y2=%f\n",start, yVal,bounds->y1(),bounds->y2());  
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
void pxText2::setMeasurementBoundsX(bool start, float xVal)
{

  rtRefT<pxTextBounds> bounds = measurements->getBounds();
  //printf("pxText2::setMeasurementBoundsX: start=%d xVal=%f already set to %f\n",start, xVal,bounds->x2());
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
 
void pxText2::setMeasurementBounds(bool start, float xVal, float yVal)
{
  rtRefT<pxTextBounds> bounds = measurements->getBounds();
  //printf("pxText2::setMeasurementBounds: start=%d xVal=%f yVal%f\n",start, xVal,yVal);
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
void pxText2::setMeasurementBounds(float xPos, float width, float yPos, float height)
{
  //printf("pxText2::setMeasurementBounds\n");
  // Set the bounds for the text
  rtRefT<pxTextBounds> bounds = measurements->getBounds();
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

void pxText2::setLineMeasurements(bool firstLine, float xPos, float yPos) 
{
  //printf("pxText2::setLineMeasurements firstLine=%d xPos=%f yPos=%f\n", firstLine, xPos, yPos);
  float height, ascent, descent, naturalLeading;
  mFace->getMetrics(mPixelSize, height, ascent, descent, naturalLeading);
  if(!firstLine) {
    measurements->getLastChar()->setX(xPos);
    measurements->getLastChar()->setY(yPos + ascent);
  } else {
    measurements->getFirstChar()->setX(xPos);
    measurements->getFirstChar()->setY(yPos + ascent);    
  }
}

/** 
 * renderTextNoWordWrap: To be used when wordWrap is false
 * */
void pxText2::renderTextNoWordWrap(float sx, float sy, float tempX, uint8_t render)
{
  //printf("pxText2::renderTextNoWordWrap render=%d\n",render);
  
  float charW, charH;
  float lineWidth = this->w();
  float tempXStartPos = tempX;
  float tempY = my;
  
  if( mHorizontalAlign == H_LEFT) {
    tempXStartPos = mXStartPos;

  }
  
  // Measure as single line since there's no word wrapping
  mFace->measureText(mText, mPixelSize, sx, sy, charW, charH);
  //printf(">>>>>>>>>>>> pxText2::renderTextNoWordWrap charH=%f charW=%f\n", charH, charW);

  float metricHeight;
  mFace->getHeight(mPixelSize, metricHeight);
  //printf(">>>>>>>>>>>>>> metric height is %f and charH is %f\n", metricHeight, charH);
  if( charH > metricHeight) // There's a newline in the text somewhere
  {
    lineNumber = 0;
    measureTextWithWrapOrNewLine(mText, sx, sy, tempX, render?startY:tempY, mPixelSize, mTextColor, render);
  }
  else 
  {
    // Calculate vertical alignment values
    if( mVerticalAlign == V_BOTTOM || mVerticalAlign == V_CENTER) 
    {
      if( mVerticalAlign == V_BOTTOM ) 
      {
        tempY = my + (mh - charH); // could be negative
      } 
      else 
      {
        tempY = my+ (mh/2) - charH/2;
      }
    }
    if( mTruncation == NONE && !clip() && charH > mh) {
      noClipH = charH;
      noClipW = charW;
    }
    // Will it fit on one line OR is there no truncation, so we don't care...
    if( (charW + tempXStartPos) <= lineWidth || mTruncation == NONE) 
    {
      lastLineNumber = lineNumber = 0;
      //printf("pxText2::renderTextNoWordWrap setLineMeasurements tempXStartPos=%f tempY=%f before renderOneLine\n",tempXStartPos, tempY);
      setLineMeasurements(true, tempXStartPos, tempY);
      setMeasurementBounds(true, tempXStartPos, tempY);

      renderOneLine(mText, tempX, tempY, sx, sy, mPixelSize, mTextColor, lineWidth, render); 
    
    }      
    else 
    { 
      // Do we really need to check for truncation? It has to be one of these
      // since we checked for NONE above.
      if(mTruncation == TRUNCATE || mTruncation == TRUNCATE_AT_WORD) 
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
void pxText2::renderTextRowWithTruncation(rtString & accString, float lineWidth, float tempX, float tempY, 
                                          float sx, float sy, uint32_t pixelSize, float* color, uint8_t render) 
{
  //printf(">>>>>>>>>>>>>>>>>>pxText2::renderTextRowWithTruncation lineNumber = %d render = %d\n",lineNumber, render);
  //printf(">>>>>>>>>>>>>>>>>>pxText2::renderTextRowWithTruncation tempY = %f tempX = %f\n",tempY, tempX);
  //printf(">>>>>>>>>>>>>>>>>>pxText2::renderTextRowWithTruncation lineWidth = %f \n",lineWidth);
  bool reallyRendering = (render==1);
  
	float charW, charH;
	char * tempStr = strdup(accString.cString()); // Should give a copy
	int length = accString.length();
	float ellipsisW = 0;
	if( mEllipsis) 
  {
		length -= ELLIPSIS_LEN;
		mFace->measureText(ELLIPSIS_STR, pixelSize, sx, sy, ellipsisW, charH);
		//printf("ellipsisW is %f\n",ellipsisW);
	}
  
  // Make adjustments for H_LEFT xStartPos and xStopPos, as applicable
  if( mHorizontalAlign == H_LEFT)
  {
    if( lineNumber == 0) {
      tempX = mXStartPos;
    }
    // Adjust line width when stop pos is authored (and using H_LEFT)
    if( mTruncation != NONE && mXStopPos != 0 && mXStopPos > tempX) 
    {
      lineWidth = mXStopPos - tempX;
    }    
  }

  
	for(int i = length; i > 0; i--) 
  { 
		tempStr[i] = '\0';
		charW = 0;
		charH = 0;
		mFace->measureText(tempStr, pixelSize, sx, sy, charW, charH);
		if( (tempX + charW + ellipsisW) <= lineWidth) 
    {
			float xPos = tempX;  
			if( mTruncation == TRUNCATE) 
      {
				// we're done; just render
				// Ignore xStartPos and xStopPos if H align is not LEFT
				if( mHorizontalAlign == H_CENTER )
        { 
					xPos = (lineWidth/2) - ((charW+ellipsisW)/2);
				}	
        else if( mHorizontalAlign == H_RIGHT)
        {
          xPos =  lineWidth - charW - ellipsisW;
        }	
        
        if(!mWordWrap) {setMeasurementBounds(xPos, charW, tempY, charH); }
        setLineMeasurements(false, xPos+charW, tempY);
        if( lineNumber==0) {setLineMeasurements(true, xPos, tempY);}

        if( reallyRendering) {
          mFace->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, color,lineWidth);
        } 
				if( mEllipsis) 
        {
					//printf("rendering truncated text with ellipsis\n");
					if( reallyRendering) {
            mFace->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, color,lineWidth);
          } 
          if(!mWordWrap) { setMeasurementBounds(xPos, charW+ellipsisW, tempY, charH); }
          setLineMeasurements(false, xPos+charW+ellipsisW, tempY);
				}
				break;
			} 
			else if( mTruncation == TRUNCATE_AT_WORD)
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
					mFace->measureText(tempStr, pixelSize, sx, sy, charW, charH);

          // Ignore xStartPos and xStopPos if H align is not LEFT
					if( mHorizontalAlign == H_CENTER )
          { 
						xPos = (lineWidth/2) - ((charW+ellipsisW)/2);
					}	
					else if( mHorizontalAlign == H_RIGHT) 
          {
						xPos = lineWidth - charW - ellipsisW;
					}					
          if(!mWordWrap) { setMeasurementBounds(xPos, charW, tempY, charH); }
          setLineMeasurements(false, xPos+charW, tempY); 		
          if( lineNumber==0) {setLineMeasurements(true, xPos, tempY);	}							
					if( reallyRendering){
            mFace->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, color,lineWidth);
          }
				}
				if( mEllipsis) 
        {
					//printf("rendering  text on word boundary with ellipsis\n");
					if( reallyRendering) {
            mFace->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, color,lineWidth);
          }
          if(!mWordWrap) { setMeasurementBounds(xPos, charW+ellipsisW, tempY, charH); }
          setLineMeasurements(false, xPos+charW+ellipsisW, tempY);
				}
				break;
			}

		}
		
	}	
}

bool pxText2::isWordBoundary( char ch )
{
    return (strchr(word_boundary_chars, ch) != NULL);
}
bool pxText2::isSpaceChar( char ch ) 
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

bool pxText2::isNewline( char ch )
{
    return (strchr(isnew_line_chars, ch) != 0);
}
/*
#### getFontMetrics - returns information about the font face (font and size).  It does not convey information about the text of the font.  
* See section 3.a in http://www.freetype.org/freetype2/docs/tutorial/step2.html .  
* The returned object has the following properties:
* height - float - the distance between baselines
* ascent - float - the distance from the baseline to the font ascender (note that this is a hint, not a solid rule)
* descent - float - the distance from the baseline to the font descender  (note that this is a hint, not a solid rule)
*/
rtError pxText2::getFontMetrics(rtObjectRef& o) {
    //printf("pxText2::getFontMetrics\n");  
	float height, ascent, descent, naturalLeading;
	pxTextMetrics* metrics = new pxTextMetrics(mScene);

  //if( mNeedsRecalc) {
    //if(!mInitialized || !mFontLoaded) {
      //rtLogWarn("getFontMetrics called TOO EARLY -- not initialized or font not loaded!\n");
      //o = metrics;
      //return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
    //}
    //recalc();
    //mNeedsRecalc = true;  // Hack to leave this set so that promise will be issued, as necessary
  //}

  printf("pxText2::getFontMetrics: mPixelSize == %d\n",mPixelSize);
	mFace->getMetrics(mPixelSize, height, ascent, descent, naturalLeading);
	metrics->setHeight(height);
	metrics->setAscent(ascent);
	metrics->setDescent(descent);
  metrics->setNaturalLeading(naturalLeading);
  metrics->setBaseline(my+ascent);
	o = metrics;

	return RT_OK;
}

/**
 * #### measureText – returns an object with the following properties (measurements are relative to (x,y) of the text object):
 * bounds - object - {x1:0, y1:0, x2:0, y2:0} - The two points representing the bounding rectangle of the complete text
 * firstChar - {x:0, y:0} - The x position represents the left most rendered pixel of the first character on the first line of text. 
 *                          The y position represents the baseline.
 * lastChar - {x:0, y:0} -  The x position represents the right most rendered pixel of the last character on the last line of text.  
 *                          The y position represents the baseline.
 * */
rtError pxText2::measureText(rtObjectRef& o) {
  //printf("pxText2::measureText()\n");
  //if( mNeedsRecalc) {
    //if(!mInitialized || !mFontLoaded) {
      //rtLogWarn("measureText called TOO EARLY -- not initialized or font not loaded!\n");
      //o = measurements;
      //return RT_OK; // !CLF: TO DO - COULD RETURN RT_ERROR HERE TO CATCH NOT WAITING ON PROMISE
    //}
    //recalc();
    //mNeedsRecalc = true;  // Hack to leave this set so that promise will be issued, as necessary
  //}
	//pxTextMeasurements* measure = new pxTextMeasurements();
	//o = measure;
  o = measurements;
  
	return RT_OK;
}

void pxText2::commitClone()
{
  bool dirty = false;
  const vector<pxObjectCloneProperty>& properties = mClone->getProperties();
  for(vector<pxObjectCloneProperty>::const_iterator it = properties.begin();
      it != properties.end(); ++it)
  {
    dirty = true;
    if ((it)->propertyName == "text")
    {
      mText = (it)->value.toString();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "pixelSize")
    {
      mPixelSize = (it)->value.toInt32();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "faceUrl")
    {
      mFontLoaded = false;
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "wordWrap")
    {
      mWordWrap = (it)->value.toBool();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "ellipsis")
    {
      mEllipsis = (it)->value.toBool();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "xStartPos")
    {
      mXStartPos = (it)->value.toFloat();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "xStopPos")
    {
      mXStopPos = (it)->value.toFloat();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "truncation")
    {
      mTruncation = (it)->value.toInt32();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "verticalAlign")
    {
      mVerticalAlign = (it)->value.toInt8();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "horizontalAlign")
    {
      mHorizontalAlign = (it)->value.toInt8();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "leading")
    {
      mLeading = (it)->value.toFloat();
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "w")
    {
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "h")
    {
      setNeedsRecalc(true);
    }
    else if ((it)->propertyName == "clip")
    {
      setNeedsRecalc(true);
    }
  }
  pxText::commitClone();
  
  if (dirty)
  {
    mDirty = true;
  }
  //printf("pxText2::commitClone: mNeedsRecalc=%d\n",mNeedsRecalc);
  if( mNeedsRecalc ) {
    //printf("pxText2::commitClone: mNeedsRecalc=%d\n",mNeedsRecalc);
    //printf("pxText2::commitClone: mInitialized=%d && mFontLoaded=%d\n",mInitialized, mFontLoaded);
    recalc();

    setNeedsRecalc(false);
    mDirty = true;
    if(mInitialized && mFontLoaded ) {
      printf("pxText2::commitClone: FIRING PROMISE\n");
      mReady.send("resolve", this);
    }
  }
}
rtError pxText2::setCloneProperty(rtString propertyName, rtValue value)
{
  printf("pxText2::setCloneProperty\n");
  pxObject::setCloneProperty(propertyName, value);
  // reset promise
  if(static_cast<rtPromise *>(mReady.getPtr())->status()) 
  {
    printf("pxText2::setCloneProperty deleting old promise\n");
    //static_cast<rtPromise *>(mReady.getPtr())->Release(); 

    mReady = new rtPromise;
 
  }
  return RT_OK;
  
}
// pxTextMetrics
rtDefineObject(pxTextMetrics, pxObject);
rtDefineProperty(pxTextMetrics, height); 
rtDefineProperty(pxTextMetrics, ascent);
rtDefineProperty(pxTextMetrics, descent);
rtDefineProperty(pxTextMetrics, naturalLeading);
rtDefineProperty(pxTextMetrics, baseline);
// pxTextBounds
rtDefineObject(pxTextBounds, pxObject);
rtDefineProperty(pxTextBounds, x1);
rtDefineProperty(pxTextBounds, y1);
rtDefineProperty(pxTextBounds, x2);
rtDefineProperty(pxTextBounds, y2);
// pxCharPosition
rtDefineObject(pxCharPosition, pxObject);
rtDefineProperty(pxCharPosition, x);
rtDefineProperty(pxCharPosition, y);
// pxTextMeasurements
rtDefineObject(pxTextMeasurements, pxObject);
rtDefineProperty(pxTextMeasurements, bounds); 
rtDefineProperty(pxTextMeasurements, firstChar);
rtDefineProperty(pxTextMeasurements, lastChar);
// pxText2
rtDefineObject(pxText2, pxText);
rtDefineProperty(pxText2, wordWrap);
rtDefineProperty(pxText2, ellipsis);
rtDefineProperty(pxText2, xStartPos); 
rtDefineProperty(pxText2, xStopPos);
rtDefineProperty(pxText2, truncation);
rtDefineProperty(pxText2, verticalAlign);
rtDefineProperty(pxText2, horizontalAlign);
rtDefineProperty(pxText2, leading);
rtDefineMethod(pxText2, getFontMetrics);
rtDefineMethod(pxText2, measureText);
