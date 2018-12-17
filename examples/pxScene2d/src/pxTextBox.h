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

// pxTextBox.h

#ifndef PX_TEXTBOX_H
#define PX_TEXTBOX_H

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rtString.h"
#include "rtRef.h"
#include "pxScene2d.h"
#include "pxText.h"



/**********************************************************************
 * 
 * pxCharPosition
 * 
 **********************************************************************/
class pxCharPosition: public rtObject {

public:
	pxCharPosition():mX(0),mY(0) {  }
	virtual ~pxCharPosition() {}

	rtDeclareObject(pxCharPosition, rtObject);
	rtReadOnlyProperty(x, x, float);
  rtReadOnlyProperty(y, y, float);

	float x()             const { return mX; }
	rtError x(float& v)   const { v = mX; return RT_OK; }
	rtError setX(float v)       { mX = v; return RT_OK; }

	float y()             const { return mY; }
	rtError y(float& v)   const { v = mY; return RT_OK; }
	rtError setY(float v)       { mY = v; return RT_OK; }

  void clear() {
    mX = 0;
    mY = 0;
  }
      
  private:
    float mX;
    float mY;
    
};

/**********************************************************************
 * 
 * pxTextBounds
 * 
 **********************************************************************/
class pxTextBounds: public rtObject {

public:
	pxTextBounds() { clear(); }
	virtual ~pxTextBounds() {}
  
	rtDeclareObject(pxTextBounds, rtObject);
	rtReadOnlyProperty(x1, x1, float);
  rtReadOnlyProperty(y1, y1, float);
  rtReadOnlyProperty(x2, x2, float);
  rtReadOnlyProperty(y2, y2, float);

	float x1()             const { return mX1; }
	rtError x1(float& v)   const { v = mX1; return RT_OK; }
	rtError setX1(float v)       { mX1 = v; return RT_OK; }

	float y1()             const { return mY1; }
	rtError y1(float& v)   const { v = mY1; return RT_OK; }
	rtError setY1(float v)       { mY1 = v; return RT_OK; }

	float x2()             const { return mX2; }
	rtError x2(float& v)   const { v = mX2; return RT_OK; }
	rtError setX2(float v)       { mX2 = v; return RT_OK; }

	float y2()             const { return mY2; }
	rtError y2(float& v)   const { v = mY2; return RT_OK; }
	rtError setY2(float v)       { mY2 = v; return RT_OK; }
      
  void clear() {
    mX1 = 0;
    mY1 = 0;
    mX2 = 0;
    mY2 = 0;
  }    
  private:
    float mX1;
    float mY1;
    float mX2;
    float mY2;  
    
};
/**********************************************************************
 * 
 * pxTextMeasurements
 * 
 **********************************************************************/
class pxTextMeasurements: public rtObject
{
  
public:
	pxTextMeasurements()
  {
    mBounds    = new pxTextBounds();
    mCharFirst = new pxCharPosition();
    mCharLast  = new pxCharPosition();
  }
	virtual ~pxTextMeasurements() {}

	rtDeclareObject(pxTextMeasurements, rtObject);
  rtReadOnlyProperty(bounds, bounds, rtObjectRef);
  rtReadOnlyProperty(charFirst, charFirst, rtObjectRef);
  rtReadOnlyProperty(charLast,  charLast,  rtObjectRef);
  
  rtError bounds(rtObjectRef& v) const
  {
    v = mBounds;
    return RT_OK;
  }
  rtError charFirst(rtObjectRef& v) const
  {
    v = mCharFirst;
    return RT_OK;
  }
  rtError charLast(rtObjectRef& v) const
  {
    v = mCharLast;
    return RT_OK;
  } 
  
  rtRefT<pxTextBounds>   getBounds()    { return mBounds;    }
  rtRefT<pxCharPosition> getCharFirst() { return mCharFirst; }
  rtRefT<pxCharPosition> getCharLast()  { return mCharLast;  }
  
  void clear() {
    mBounds->clear();
    mCharFirst->clear();
    mCharLast->clear();
  }    
      
  private:
   
    rtRefT<pxTextBounds>   mBounds;
    rtRefT<pxCharPosition> mCharFirst;
    rtRefT<pxCharPosition> mCharLast;
    
};

/**********************************************************************
 * 
 * pxTextBox
 * 
 **********************************************************************/
class pxTextBox: public pxText
{
public:
  rtDeclareObject(pxTextBox, pxText);

  pxTextBox(pxScene2d* s);
  virtual ~pxTextBox(){}
  
  rtProperty(wordWrap, wordWrap, setWordWrap, bool);
  rtProperty(ellipsis, ellipsis, setEllipsis, bool);
  rtProperty(xStartPos, xStartPos, setXStartPos, float); 
  rtProperty(xStopPos,  xStopPos,  setXStopPos,  float);
  rtProperty(truncation, truncation, setTruncation, uint32_t);
  rtProperty(alignVertical, alignVertical, setAlignVertical, uint32_t);
  rtProperty(alignHorizontal, alignHorizontal, setAlignHorizontal, uint32_t);
  rtProperty(leading, leading, setLeading, float); 	
  
  bool wordWrap()                        const { return mWordWrap;             }
  rtError wordWrap(bool& v)              const { v = mWordWrap; return RT_OK;  }
  rtError setWordWrap(bool v)                  { mWordWrap = v; setNeedsRecalc(true); return RT_OK; }
  
  bool ellipsis()                        const { return mEllipsis;             }
  rtError ellipsis(bool& v)              const { v = mEllipsis; return RT_OK;  }
  rtError setEllipsis(bool v)                  { mEllipsis = v; setNeedsRecalc(true); return RT_OK; }
  
  float xStartPos()                      const { return mXStartPos;              }
  rtError xStartPos(float& v)            const { v = mXStartPos; return RT_OK;   }
  rtError setXStartPos(float v)                { mXStartPos = v; setNeedsRecalc(true); return RT_OK; }
  
  float xStopPos()                       const { return mXStopPos;              }
  rtError xStopPos(float& v)             const { v = mXStopPos; return RT_OK;   }
  rtError setXStopPos(float v)                 { mXStopPos = v; setNeedsRecalc(true); return RT_OK; }
    
  uint32_t truncation()                  const { return mTruncation;              }
  rtError truncation(uint32_t& v)        const { v = mTruncation; return RT_OK;   }
  rtError setTruncation(uint32_t v)            { mTruncation = v; setNeedsRecalc(true); return RT_OK;   }

  uint8_t alignVertical()                const { return mAlignVertical;              }
  rtError alignVertical(uint32_t& v)     const { v = mAlignVertical; return RT_OK;   }
  rtError setAlignVertical(uint32_t v)         { mAlignVertical = v;  setNeedsRecalc(true); return RT_OK;   }
  
  uint8_t alignHorizontal()              const { return mAlignHorizontal;              }
  rtError alignHorizontal(uint32_t& v)   const { v = mAlignHorizontal; return RT_OK;   }
  rtError setAlignHorizontal(uint32_t v)       { mAlignHorizontal = v;  setNeedsRecalc(true); return RT_OK;   }
  
  float leading()                        const { return mLeading;              }
  rtError leading(float& v)              const { v = mLeading; return RT_OK;   }
  rtError setLeading(float v)                  { mLeading = v;  setNeedsRecalc(true); return RT_OK; }
  
  virtual rtError setW(float v)                { setNeedsRecalc(true); return pxObject::setW(v);    }
  virtual rtError setH(float v)                { setNeedsRecalc(true); return pxObject::setH(v);    }
  virtual rtError setClip(bool v)              { mClip = v; setNeedsRecalc(true); return RT_OK;     }
  virtual rtError setText(const char* s);
  virtual rtError setPixelSize(uint32_t v);
  virtual rtError setFontUrl(const char* s);
  virtual rtError setFont(rtObjectRef o);
  
  void renderText(bool render);

  virtual void resourceReady(rtString readyResolution);
  virtual void sendPromise();
  virtual void draw();
  virtual void onInit();
  virtual void update(double t);

 
  //rtMethodNoArgAndReturn("getFontMetrics", getFontMetrics, rtObjectRef);
  //rtError getFontMetrics(rtObjectRef& o);
  rtMethodNoArgAndReturn("measureText", measureText, rtObjectRef);
  rtError measureText(rtObjectRef& o); 

  virtual rtError Set(uint32_t i, const rtValue* value) override
  {
    (void)i;
    (void)value;
    rtLogError("pxTextBox::Set(uint32_t, const rtValue*) - not implemented");
    return RT_ERROR_NOT_IMPLEMENTED;
  }

  virtual rtError Set(const char* name, const rtValue* value) override
  {
	  //rtLogDebug("pxTextBox Set for %s\n", name );

    mDirty = mDirty || (!strcmp(name,"clip")            ||
                        !strcmp(name,"w")               ||
                        !strcmp(name,"h")               ||
                        !strcmp(name,"wordWrap")        ||
                        !strcmp(name,"ellipsis")        ||
                        !strcmp(name,"xStartPos")       ||
                        !strcmp(name,"xStopPos")        ||
                        !strcmp(name,"truncation")      ||
                        !strcmp(name,"alignVertical")   ||
                        !strcmp(name,"alignHorizontal") ||
                        !strcmp(name,"leading"));

    rtError e = pxText::Set(name, value);

    return e;
  }


 protected:
 
  pxTextMeasurements* getMeasurements() { return (pxTextMeasurements*)measurements.getPtr();}
    
	uint32_t mTruncation;  
  uint32_t mAlignVertical;
  uint32_t mAlignHorizontal;

  float mXStartPos;
	float mXStopPos;
	float mLeading;

  bool mWordWrap;
	bool mEllipsis;
  
  bool mInitialized;
  bool mNeedsRecalc;

  #ifdef PXSCENE_FONT_ATLAS
  std::vector<pxTexturedQuads> mQuadsVector;
  #endif

  rtObjectRef measurements;
  uint32_t lineNumber;
  uint32_t lastLineNumber;
  float noClipX, noClipY, noClipW, noClipH;
//  float startX;
  float startY;
//  bool clippedLastLine;
  
  void setNeedsRecalc(bool recalc);
  virtual float getFBOWidth();
  virtual float getFBOHeight(); 
  bool isNewline( char ch );
  bool isWordBoundary( char ch );
  bool isSpaceChar( char ch );  
  
  void renderTextRowWithTruncation(rtString & accString, float lineWidth, float tempX, float tempY, float sx, float sy, uint32_t pixelSize, bool render);
  void renderTextNoWordWrap(float sx, float sy, float tempX, bool render);
  void renderTextWithWordWrap(const char *text, float sx, float sy, float tempX, uint32_t pixelSize, bool render);
  void measureTextWithWrapOrNewLine(const char *text, float sx, float sy, float tempX, float &tempY, uint32_t size, bool render);
  void renderOneLine(const char * tempStr, float tempX, float tempY, float sx, float sy,  uint32_t size, float lineWidth, bool render, bool isNewLineCase = false);
  
  void recalc();
  void clearMeasurements();
  void setMeasurementBoundsY(bool start, float yVal);
  void setMeasurementBoundsX(bool start, float xVal);  
  void setMeasurementBounds( bool start, float xVal, float yVal);
  void setMeasurementBounds(float xPos, float width, float yPos, float height);
  void setLineMeasurements(bool firstLine, float xPos, float y);
};

#endif
