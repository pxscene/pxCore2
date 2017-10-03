#include <list>
#include <sstream>

#define private public
#define protected public

#include "pxWindow.h"
#include "pxScene2d.h"
#include <string.h>
#include <sstream>

#include "test_includes.h" // Needs to be included last


TEST(pxWindowTest, pxWindowTest)
{
  pxWindow window;
  
  window.init(0, 0, 100, 100);
  
  window.onAnimationTimer();
  
  window.onSize(0,0);

  window.onMouseDown(0,0,0);
  window.onMouseUp(0,0,0);
  window.onMouseEnter();
  window.onMouseLeave();
  
  window.onFocus();
  window.onBlur();

  window.onMouseMove(0,0);

  window.onKeyDown(0,0);
  window.onKeyUp(0,0);
  window.onChar(0);
  window.onDraw(NULL);

  window.onCloseRequest();
  window.onClose();
  
  EXPECT_EQ(0,0);
  

}