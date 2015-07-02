// pxCore CopyRight 2007-2015 John Robinson
// pxText.cpp

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


pxText2::pxText2()
{
  measurements= new pxTextMeasurements();
}

pxText2::~pxText2()
{
	 
}

/** 
 * setText: for pxText2, setText sets the text value, but does not
 * affect the dimensions of the object.  Dimensions are respected 
 * and text is wrapped/truncated within those dimensions according
 * to other properties.
 * */
rtError pxText2::setText(const char* s) { 

  mText = s; 
  
  return RT_OK; 
}

rtError pxText2::setPixelSize(uint32_t v) 
{   
  mPixelSize = v; 
  //mFace->measureText(mText, mPixelSize, 1.0, 1.0, mw, mh);
  return RT_OK; 
}


void pxText2::draw() {

	if (mCached.getPtr()) {
    context.drawImage(0,0,mw,mh,mCached,pxTextureRef(),PX_NONE,PX_NONE);
	}
	else {
	  renderText(mText, mPixelSize, 0, 0, 1.0, 1.0, mTextColor, mw);

	}
}

void pxText2::renderText(const char *text, uint32_t size, float x, float y, 
                  float sx, float sy, 
                  float* color, float mw)
{

	if (!text) {
    setMeasurementBounds(mx, 0, my, 0);
    lastLineNumber = 0;
    lineNumber = 0;
    measurements->clear();
		return;
	}

  // Prerender the text to determine dimensions and number of lines
	float tempY = 0;
//  uint32_t lineNumber = 0;  
  float tempX;


  if( mHorizontalAlign == H_LEFT) 
  {
    tempX = mXStartPos;
  } 
  else 
  {
    tempX = x;
  }
//	tempY = y;


	if( !mWordWrap) 
  {
    renderTextNoWordWrap(sx, sy, tempX, size, color);

	} 
	else 
	{	
    renderTextWithWordWrap(text, sx, sy, tempX, size, color);

	}


  // Set the bounds for the text
  printf("Measurements: boundsX2=%f, boundsX1=%f, boundsY2=%f, boundsY1=%f\n", 
      measurements->getBounds()->x2(), measurements->getBounds()->x1(), 
      measurements->getBounds()->y2(), measurements->getBounds()->y1());
      
  printf("First and last positions: firstCharX=%f, firstCharY=%f, lastCharX=%f, lastCharY=%f\n", 
      measurements->getFirstChar()->x(), measurements->getFirstChar()->y(), 
      measurements->getLastChar()->x(), measurements->getLastChar()->y());
 
}

void pxText2::renderTextWithWordWrap(const char *text, float sx, float sy, float tempX, uint32_t size, float* color)
{
	float tempY = 0;
//  uint32_t lineNumber = 0;
  
  if( mVerticalAlign == V_BOTTOM || mVerticalAlign == V_CENTER) 
  {
    measureTextWithWrapOrNewLine( text, sx, sy, tempX, tempY, size, color, lineNumber, false);
    float metricHeight, ascent, descent;
    mFace->getMetrics(metricHeight, ascent, descent);
    float textHeight =  ((lineNumber+1)*metricHeight) + ((lineNumber-1)* mLeading);
    if( mVerticalAlign == V_BOTTOM ) 
    {
      tempY = my + (mh - textHeight); // could be negative
    } 
    else 
    {
      tempY = my+ (mh/2) - textHeight/2;
    }
  }
  if( mHorizontalAlign == H_LEFT && mTruncation != NONE && lineNumber == 0) 
  {
    if( mXStopPos != 0) 
    {
      // if this single line won't fit when it accounts for mXStopPos, 
      // need to wrap to a new line
      printf("!CLF: HERE WE ARE!\n");
    }
  }
  lastLineNumber = lineNumber;
  lineNumber = 0;
  measureTextWithWrapOrNewLine( text, sx, sy, tempX, tempY, size, color, lineNumber, true);
}


void pxText2::measureTextWithWrapOrNewLine(const char *text, float sx, float sy, float tempX, float &tempY, uint32_t size, float* color, uint32_t &lineNumber, bool render) {
	int i = 0;
	u_int32_t charToMeasure;
	float charW, charH;
      // Wordwrap is wraap
		rtString accString = "";
		bool lastLine = false;
		float lineWidth = mw;

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
        renderOneLine(lineNumber, accString.cString(), mx, tempY, sx, sy, size, color, lineWidth, render);

        accString = "";			
				tempY += (mLeading*sy) + charH;

				lineNumber++;
        //if( tempX + charW
				tempX = 0;
				continue;
			}
		
      // Check if text still fits on this line, or if wrap needs to occur
			if( (tempX + charW) <= lineWidth || !mWordWrap) 
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
					if( render) { renderTextRowWithTruncation(accString, lineWidth, tempY, sx, sy, size, color);}
					// Clear accString because we've rendered it 
					accString = "";	
					break; // break out of reading mText

				} 
				else 
        {  // If not lastLine
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
						//printf("wordBoundary is at \"%c\"\n",tempStr[n]);
						tempStr[n+1] = '\0';
						//printf("rendering wrappingtext on word boundary %s\n", tempStr);
						// write out entire string that will fit
						// Use horizonal positioning
            renderOneLine(lineNumber, tempStr, mx, tempY, sx, sy, size, color, lineWidth, render);

						// Now reset accString to hold remaining text
						tempStr = strdup(accString.cString());
						n++;
						if( strlen(tempStr+n) > 0) 
            {
							//printf("char at n is now \"%c\"\n",tempStr[n]);
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
						
						//printf("New accString after truncate/wrap is \"%s\"\n",accString.cString());
					}
					// Now skip to next line
					tempY += (mLeading*sy) + charH;
					tempX = 0;
					lineNumber++;

					mFace->measureText(accString.cString(), size, sx, sy, charW, charH);

          tempX += charW;			
          
          // If Truncation is NONE, we never want to set as last line; 
          // just keep rendering...
          if( mTruncation != NONE && tempY + ((mLeading*sy) + charH)*2 > this->h() && !lastLine) 
          {
            lastLine = true;
            if(mXStopPos != 0 && mTruncation != NONE && mHorizontalAlign == H_LEFT) 
            {
                lineWidth = mXStopPos - mx;
            } 				
          }

				}

			}

		}
		
		if(accString.length() > 0) {
      lastLineNumber = lineNumber;
      if( mTruncation == NONE && !mWordWrap && !clip()) {
        printf("CLF! Sending tempX instead of this->w(): %f\n", tempX);
        renderOneLine(lineNumber, accString.cString(), 0, tempY, sx, sy, size, color, tempX, render);
      } else {
        renderOneLine(lineNumber, accString.cString(), 0, tempY, sx, sy, size, color, this->w(), render);
      }
		  //if( render) {mFace->renderText(accString.cString(), size, 0, tempY, sx, sy, color,this->w());}
		}	
}


void pxText2::renderOneLine(uint32_t lineNumber, const char * tempStr, float tempX, float tempY, float sx, float sy, uint32_t size, float* color, float lineWidth, bool render )
{
  float charW; float charH;

  float xPos = mx; 
  mFace->measureText(tempStr, size, sx, sy, charW, charH);
  printf("pxText2::renderOneLine returned charW=%f for %s\n",charW, tempStr);
  if( mHorizontalAlign == H_LEFT && lineNumber == 0) 
  {
    // Only respect xStartPos if H align is LEFT
    xPos = mXStartPos;
  } 
  else {
    if( mHorizontalAlign == H_CENTER ) 
    { 
      xPos = (lineWidth/2) - charW/2;
    }
    else if( mHorizontalAlign == H_RIGHT) 
    {
       xPos = lineWidth - charW;
    }							
  }
  
  setMeasurementBounds(xPos, charW, tempY, charH);
  
  printf("renderOneLine is testing lineNumber=%d and lastLineNumber=%d and render=%d\n",lineNumber, lastLineNumber,render);
  if( lineNumber == lastLineNumber && render) {
    setLastLineMeasurements(xPos+charW, tempY);
  }
  // Now, render the text
  if( render) {mFace->renderText(tempStr, size, xPos, tempY, sx, sy, color,lineWidth);} 

}

void pxText2::setMeasurementBounds(float xPos, float width, float yPos, float height)
{
  // Set the bounds for the text
  rtRefT<pxTextBounds> bounds = measurements->getBounds();
  if( bounds->x2() < (xPos + width) ) {
    bounds->setX2(xPos + width);
  } 
  if( bounds->x1() > xPos) {
    bounds->setX1(xPos);
  }
  if( bounds->y2() < (yPos + height)) {
     bounds->setY2(yPos + height);
  }
  if( bounds->y1() > yPos) {
    bounds->setY1(yPos);
  } 
  
  if( lineNumber == 0) {
    float height, ascent, descent;
    mFace->getMetrics(height, ascent, descent);

    measurements->getFirstChar()->setX(xPos);
    measurements->getFirstChar()->setY(yPos + ascent);
  }
}

void pxText2::setLastLineMeasurements(float x, float y) 
{
  float height, ascent, descent;
  mFace->getMetrics(height, ascent, descent);
  measurements->getLastChar()->setX(x);
  measurements->getLastChar()->setY(y+ascent);
}

/** 
 * renderTextNoWordWrap: To be used when wordWrap is false
 * */
void pxText2::renderTextNoWordWrap(float sx, float sy, float tempX, uint32_t pixelSize, float* color )
{
  float charW, charH;
  float lineWidth = this->w();
  float tempXStartPos = 0;
//  uint32_t lineNumber = 0;
  float tempXStopPos = 0;
  float tempY = my;
  
  if( mHorizontalAlign == H_LEFT) {
    tempXStartPos = mXStartPos;
    tempXStopPos = mXStopPos;
  }
  // Adjust line width when start/stop pos are authored (and using H_LEFT)
  if(mTruncation != NONE && mXStopPos != 0 && mXStopPos > mXStartPos) 
  {
    //tempXStartPos = mXStartPos;
    //lineWidth = mXStartPos - mXStopPos;
    lineWidth = tempXStopPos - tempXStartPos;
    printf("lineWidth is now %d\n", lineWidth);
  }
  
  // Measure as single line since there's no word wrapping
  mFace->measureText(mText, pixelSize, sx, sy, charW, charH);
  printf("pxText2::renderTextNoWordWrap charH=%f charW=%f\n", charH, charW);

      
  // Will it fit on one line OR is there no truncation, so we don't care...
  if( (charW + tempXStartPos) <= lineWidth || mTruncation == NONE) 
  {
    float metricHeight, ascent, descent;
    mFace->getMetrics(metricHeight, ascent, descent);
    printf("metric height is %f and charH is %f\n", metricHeight, charH);
    if( charH > metricHeight) {

      // There are newlines in the text, so we need to adjust for those
      if( mVerticalAlign == V_BOTTOM || mVerticalAlign == V_CENTER) {
        measureTextWithWrapOrNewLine( mText, sx, sy, tempX, tempY, pixelSize, color, lineNumber, false);
        float textHeight =  charH + ((charH/metricHeight)*mLeading);
        if( mVerticalAlign == V_BOTTOM ) {
          tempY = my + (mh - textHeight); // could be negative
        } else {

          tempY = my+ (mh/2) - textHeight/2;
        }
      }
      lastLineNumber = lineNumber;      
      lineNumber = 0;
      measureTextWithWrapOrNewLine(mText, sx, sy, tempX, tempY, pixelSize, color, lineNumber, true);
    } 
    else {
        // it fits and/or there's no truncation, so render it
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
        lastLineNumber = lineNumber = 0;
        renderOneLine(lineNumber, mText, 0, tempY, sx, sy, pixelSize, color, lineWidth, true);        

    }
  } 
  else 
  {
    // Check for truncation
    if(mTruncation == TRUNCATE || mTruncation == TRUNCATE_AT_WORD) 
    {
      renderTextRowWithTruncation(mText, lineWidth, my, sx, sy, pixelSize, color);

    }
  
  }
}

/**
 * renderTextRowWithTruncation: Only to be called when truncation != NONE
 * */
void pxText2::renderTextRowWithTruncation(rtString & accString, float lineWidth, float tempY, 
                                          float sx, float sy, uint32_t pixelSize, float* color) 
{
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
  
	for(int i = length; i > 0; i--) 
  { 
		tempStr[i] = '\0';
		charW = 0;
		charH = 0;
		mFace->measureText(tempStr, pixelSize, sx, sy, charW, charH);
		if( (charW + ellipsisW) <= lineWidth) 
    {
			float xPos = this->x(); 
			if( mTruncation == TRUNCATE) 
      {
				// we're done; just render
				//printf("rendering truncated text %s\n", tempStr);
				// Ignore xStartPos and xStopPos if H align is not LEFT
				if( mHorizontalAlign == H_CENTER )
        { 
					xPos = (lineWidth/2) - (charW/2);
				}	
        else if( mHorizontalAlign == H_RIGHT)
        {
          xPos =  lineWidth - charW - ellipsisW;
        }	
        
        setMeasurementBounds(xPos, charW, tempY, charH);
        setLastLineMeasurements(xPos+charW, tempY);
				mFace->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, color,lineWidth);
				if( mEllipsis) 
        {
					//printf("rendering truncated text with ellipsis\n");
					mFace->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, color,lineWidth);
          setMeasurementBounds(xPos+charW, ellipsisW, tempY, charH);
          setLastLineMeasurements(xPos+charW+ellipsisW, tempY);
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
						xPos = (lineWidth/2) - (charW/2);
					}	
					else if( mHorizontalAlign == H_RIGHT) 
          {
						xPos = lineWidth - charW - ellipsisW;
					}					
          setMeasurementBounds(xPos, charW, tempY, charH);
          setLastLineMeasurements(xPos+charW, tempY); 										
					mFace->renderText(tempStr, pixelSize, xPos, tempY, 1.0, 1.0, color,lineWidth);
				}
				if( mEllipsis) 
        {
					//printf("rendering  text on word boundary with ellipsis\n");
					mFace->renderText(ELLIPSIS_STR, pixelSize, xPos+charW, tempY, 1.0, 1.0, color,lineWidth);
          setMeasurementBounds(xPos+charW, ellipsisW, tempY, charH);
          setLastLineMeasurements(xPos+charW+ellipsisW, tempY);
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

	float height, ascent, descent;
	pxTextMetrics* metrics = new pxTextMetrics();

	mFace->getMetrics(height, ascent, descent);
	metrics->setHeight(height);
	metrics->setAscent(ascent);
	metrics->setDescent(descent);
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
  printf("pxText2::measureText()\n");

	//pxTextMeasurements* measure = new pxTextMeasurements();
	//o = measure;
  o = measurements;
  
	return RT_OK;
}
// pxTextMetrics
rtDefineObject(pxTextMetrics, pxObject);
rtDefineProperty(pxTextMetrics, height); 
rtDefineProperty(pxTextMetrics, ascent);
rtDefineProperty(pxTextMetrics, descent);
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
