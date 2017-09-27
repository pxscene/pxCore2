#include <list>
#include <sstream>

#define private public
#define protected public

#include "pxWindow.h"
#include "pxScene2d.h"
#include <string.h>
#include <sstream>

#include "test_includes.h" // Needs to be included last

// Mock for imageResource so that we don't have to wait for real download
class MockWindow : public pxWindow {
  
  protected:
    virtual void onCloseRequest() {}
    virtual void onClose() {}
    virtual void onAnimationTimer() { pxWindow::onAnimationTimer();}
    
    virtual void onSize(int32_t w, int32_t h) {pxWindow::onSize(w, h);}
    
    // See constants used for flags below
    virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags) {
        pxWindow::onMouseDown(x, y, flags);
      }
    virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags) {pxWindow::onMouseUp(x, y, flags);}
    virtual void onMouseEnter() {pxWindow::onMouseEnter();}
    virtual void onMouseLeave() {pxWindow::onMouseLeave();}
    
    virtual void onFocus() {pxWindow::onFocus();}
    virtual void onBlur() {pxWindow::onBlur();}
  
    virtual void onMouseMove(int32_t x, int32_t y) {pxWindow::onMouseMove(x, y);}
    
    // See pxWindowNative.h for keycode constants
    // See constants used for flags below
    virtual void onKeyDown(uint32_t keycode, uint32_t flags) {pxWindow::onKeyDown(keycode, flags);}
    virtual void onKeyUp(uint32_t keycode, uint32_t flags) {pxWindow::onKeyUp(keycode, flags);}
    virtual void onChar(uint32_t codepoint) {pxWindow::onChar(codepoint);}
    
    // pxSurfaceNative abstracts a platform specific drawing surface
    // to perform platform specific drawing please see pxWindowNative.h
    // for the definition of this type
    virtual void onDraw(pxSurfaceNative s) {pxWindow::onDraw(s);}    

  };

TEST(pxWindowTest, pxWindowTest)
{
  MockWindow window;
  
  window.init(0, 0, 100, 100);
  
  // To enable this event call setAnimationFPS defined above
  window.onAnimationTimer();
  
  window.onSize(0,0);
  
  // See constants used for flags below
  window.onMouseDown(0,0,0);
  window.onMouseUp(0,0,0);
  window.onMouseEnter();
  window.onMouseLeave();
  
  window.onFocus();
  window.onBlur();

  window.onMouseMove(0,0);
  
  // See pxWindowNative.h for keycode constants
  // See constants used for flags below
  window.onKeyDown(0,0);
  window.onKeyUp(0,0);
  window.onChar(0);
  
  // pxSurfaceNative abstracts a platform specific drawing surface
  // to perform platform specific drawing please see pxWindowNative.h
  // for the definition of this type
  window.onDraw(NULL);

  window.onCloseRequest();
  window.onClose();
  
  //window.close();
  
  EXPECT_EQ(0,0);
  

}