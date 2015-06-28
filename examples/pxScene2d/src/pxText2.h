// pxCore CopyRight 2007-2015 John Robinson
// pxText.h

#ifndef PX_TEXT2_H
#define PX_TEXT2_H

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"
#include "pxText.h"


/*#### truncation - one of the following values
* NONE - no trunctaction
* TRUNCATE - text is truncated at the bottom of the text object.  The last word may be partially truncated. 
* TRUNCATE_AT_WORD_BOUNDARY - text is truncated at the bottom of the text object.  Truncation occurs at the word boundary.
* ELLIPSES - text is truncated at the bottom of the text object with ellipses applied.  The last word may be broken off by the ellipses.
* ELLIPSES_AT_WORD_BOUNDARY - text is truncated at the bottom of the text object with ellipses applied to the last possible full word.
###### Note: When truncation is applied at the word boundary, in a situation where the last line of text to be rendered contains a
*/
enum PX_TRUNCATION {
	NONE, 
	TRUNCATE,
	TRUNCATE_AT_WORD
};
enum PX_VERTICAL_ALIGN {
	V_TOP, 
	V_CENTER,
	V_BOTTOM
};
enum PX_HORIZONTAL_ALIGN {
	H_LEFT, 
	H_CENTER,
	H_RIGHT
};
#define ELLIPSIS_STR "..."
#define ELLIPSIS_LEN (sizeof(ELLIPSIS_STR)-1)	
static const char isnew_line_chars[] = "\n\v\f\r";
static const char word_boundary_chars[] = " \t/:&,;.";
static const char space_chars[] = " \t";

//class pxTextMetrics;
//typedef rtRefT<pxTextMetrics> pxTextMetricsRef;

class pxTextMetrics: public pxObject {

public:
	pxTextMetrics():mRefCount(0) {  }
	virtual ~pxTextMetrics() {}

	virtual unsigned long AddRef() 
	{
		return rtAtomicInc(&mRefCount);
	}

	virtual unsigned long Release() 
	{
		long l = rtAtomicDec(&mRefCount);
		if (l == 0) delete this;
			return l;
	}
	rtDeclareObject(pxTextMetrics, pxObject);
	rtProperty(height, height, setHeight, float); 
	rtProperty(ascent, ascent, setAscent, float);
	rtProperty(descent, descent, setDescent, float);
 
	float height()             const { return mHeight; }
	rtError height(float& v)   const { v = mHeight; return RT_OK;   }
	rtError setHeight(float v)       { mHeight = v; return RT_OK;   }

	float ascent()             const { return mAscent; }
	rtError ascent(float& v)   const { v = mAscent; return RT_OK;   }
	rtError setAscent(float v)       { mAscent = v; return RT_OK;   } 

	float descent()             const { return mDescent; }
	rtError descent(float& v)   const { v = mDescent; return RT_OK;   }
	rtError setDescent(float v)       { mDescent = v; return RT_OK;   } 
  
  private:
    rtAtomic mRefCount;	
   	float mHeight;
    float mAscent;
    float mDescent;
};

class pxText2: public pxText {
public:
  rtDeclareObject(pxText2, pxText);

  pxText2();
  ~pxText2();
  
  rtProperty(wordWrap, wordWrap, setWordWrap, bool);
  rtProperty(ellipsis, ellipsis, setEllipsis, bool);
  rtProperty(xStartPos, xStartPos, setXStartPos, float); 
  rtProperty(xStopPos, xStopPos, setXStopPos, float);
  rtProperty(truncation, truncation, setTruncation, uint32_t);
  rtProperty(verticalAlign, verticalAlign, setVerticalAlign, uint8_t);
  rtProperty(horizontalAlign, horizontalAlign, setHorizontalAlign, uint8_t);
  rtProperty(leading, leading, setLeading, float); 	
  
  bool wordWrap()            const { return mWordWrap;}
  rtError wordWrap(bool& v)  const { v = mWordWrap; return RT_OK;  }
  rtError setWordWrap(bool v) { mWordWrap = v; return RT_OK; }
  
  bool ellipsis()            const { return mEllipsis;}
  rtError ellipsis(bool& v)  const { v = mEllipsis; return RT_OK;  }
  rtError setEllipsis(bool v) { mEllipsis = v; return RT_OK; }
  
  float xStartPos()             const { return mXStartPos; }
  rtError xStartPos(float& v)   const { v = mXStartPos; return RT_OK;   }
  rtError setXStartPos(float v)       { mXStartPos = v; return RT_OK;   }
  
  float xStopPos()             const { return mXStopPos; }
  rtError xStopPos(float& v)   const { v = mXStopPos; return RT_OK;   }
  rtError setXStopPos(float v)       { mXStopPos = v; return RT_OK;   }
    
  uint32_t truncation()             const { return mTruncation; }
  rtError truncation(uint32_t& v)   const { v = mTruncation; return RT_OK;   }
  rtError setTruncation(uint32_t v)       { mTruncation = v; return RT_OK;   }

  uint8_t verticalAlign()             const { return mVerticalAlign; }
  rtError verticalAlign(uint8_t& v)   const { v = mVerticalAlign; return RT_OK;   }
  rtError setVerticalAlign(uint8_t v)       { mVerticalAlign = v; return RT_OK;   }
  
  uint8_t horizontalAlign()             const { return mHorizontalAlign; }
  rtError horizontalAlign(uint8_t& v)   const { v = mHorizontalAlign; return RT_OK;   }
  rtError setHorizontalAlign(uint8_t v)       { mHorizontalAlign = v; return RT_OK;   }
  
  float leading()             const { return mLeading; }
  rtError leading(float& v)   const { v = mLeading; return RT_OK;   }
  rtError setLeading(float v)       { mLeading = v; return RT_OK;   }  
  
  virtual rtError setText(const char* s); 
  virtual rtError setPixelSize(uint32_t v);
  void renderText(const char *text, uint32_t size, float x, float y, 
                  float sx, float sy, 
                  float* color, float mw);
  virtual void draw();

  void renderTextRowWithTruncation(rtString & accString, float lineWidth, float tempY, float sx, float sy, uint32_t pixelSize,float* color);
  void renderTextNoWordWrap(float sx, float sy, float tempX, uint32_t pixelSize,float* color);
  bool isNewline( char ch );
  bool isWordBoundary( char ch );
  bool isSpaceChar( char ch );
  rtMethodNoArgAndReturn("getFontMetrics", getFontMetrics, rtObjectRef);
  rtError getFontMetrics(rtObjectRef& o);

  virtual rtError Set(const char* name, const rtValue* value)
  {
	  printf("pxText2 Set for %s\n", name );

    mDirty = mDirty || (!strcmp(name,"wordWrap") ||
              !strcmp(name,"ellipsis") ||
              !strcmp(name,"xStartPos") ||
              !strcmp(name,"xStopPos") ||
              !strcmp(name,"mTruncation") ||
              !strcmp(name,"verticalAlign")||
              !strcmp(name,"horizontalAlign") ||
              !strcmp(name,"leading"));

    rtError e = pxText::Set(name, value);

    return e;
  }


 protected:
 
	uint32_t mTruncation;  
	float mXStartPos;
	float mXStopPos;
	float mVerticalAlign;
	float mHorizontalAlign;
	float mLeading;
	bool mWordWrap;
	bool mEllipsis;
};

#endif
