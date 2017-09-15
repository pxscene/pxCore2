#include <sstream>
#include <vector>
#include <set>
#include <iostream>

#include "pxWindowUtil.h"
#include "pxCore.h"
#include "pxKeycodes.h"
#include "test_includes.h" // Needs to be included last

class pxWindowUtilTest : public testing::Test
{
public:
  virtual void SetUp()
  {
    initNativeKeycodes();
    initPxKeycodes();
    initPrintableAscii();
  }

  virtual void TearDown()
  {
  }

  void testNativeKeycodesHaveNoDuplicates()
  {
    std::set<uint32_t> allNativeKeycodes(mNativeKeycodes.begin(), mNativeKeycodes.end());
    EXPECT_TRUE (allNativeKeycodes.size() == mNativeKeycodes.size());
  }

  void testPxKeycodesHaveNoDuplicates()
  {
    std::set<uint32_t> allPxKeycodes(mPxKeycodes.begin(), mPxKeycodes.end());
    EXPECT_TRUE (allPxKeycodes.size() == mPxKeycodes.size());
  }

  void testNativeKeycodeToValidPxKeycode()
  {
    std::set<uint32_t> coverageNoDups;
    std::set<uint32_t> allNativeKeycodes(mNativeKeycodes.begin(), mNativeKeycodes.end());
    std::set<uint32_t>::const_iterator it;
    for (it = allNativeKeycodes.begin(); it != allNativeKeycodes.end(); ++it)
    {
      uint32_t nativeKeycode = *it;
      uint32_t pxKeycode = keycodeFromNative(nativeKeycode);
      if (pxKeycode > 0)
      {
        coverageNoDups.insert(pxKeycode);
      }
    }

    std::set<uint32_t> allPxKeycodes(mPxKeycodes.begin(), mPxKeycodes.end());
    for (it = allPxKeycodes.begin(); it != allPxKeycodes.end(); ++it)
    {
      coverageNoDups.erase(*it);
    }
    coverageNoDups.erase(PX_KEY_NATIVE_CLEAR); // FIXME HACK HACK in pxWindowUtil marked as TODO
    coverageNoDups.erase(PX_KEY_NATIVE_SEPARATOR); // FIXME HACK HACK in pxWindowUtil marked as TODO
    EXPECT_TRUE (coverageNoDups.size() == 0);
  }

  void testPxKeycodeToValidPrintableAscii()
  {
    std::set<uint32_t> coverageNoDups;
    std::set<uint32_t> allPxKeycodes(mPxKeycodes.begin(), mPxKeycodes.end());
    std::set<uint32_t>::const_iterator it;
    for (it = allPxKeycodes.begin(); it != allPxKeycodes.end(); ++it)
    {
      uint32_t pxKeycode = *it;
      uint32_t ascii = keycodeToAscii(pxKeycode, 0);
      if (ascii > 0)
      {
        coverageNoDups.insert(ascii);
      }
      ascii = keycodeToAscii(pxKeycode, PX_MOD_SHIFT);
      if (ascii > 0)
      {
        coverageNoDups.insert(ascii);
      }
    }

    for (it = mPrintableAscii.begin(); it != mPrintableAscii.end(); ++it)
    {
      coverageNoDups.erase(*it);
    }
    EXPECT_TRUE (coverageNoDups.size() == 0);
  }

  void testNativeKeycodeToPxKeycodeCoverage()
  {
    std::set<uint32_t> coverageNoDups;
    std::set<uint32_t> allNativeKeycodes(mNativeKeycodes.begin(), mNativeKeycodes.end());
    std::set<uint32_t>::const_iterator it;
    for (it = allNativeKeycodes.begin(); it != allNativeKeycodes.end(); ++it)
    {
      uint32_t nativeKeycode = *it;
      uint32_t pxKeycode = keycodeFromNative(nativeKeycode);
      EXPECT_TRUE (pxKeycode > 0);
      if (pxKeycode > 0)
      {
        coverageNoDups.insert(pxKeycode);
      }
    }

    std::set<uint32_t> allPxKeycodes(mPxKeycodes.begin(), mPxKeycodes.end());
    for (it = coverageNoDups.begin(); it != coverageNoDups.end(); ++it)
    {
      allPxKeycodes.erase(*it);
    }
    allPxKeycodes.erase(PX_KEY_WINDOWKEY_LEFT); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_WINDOWKEY_RIGHT); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_SELECT); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEYDOWN_REPEAT); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_NUMLOCK); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_DASH); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_FASTFORWARD); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_REWIND); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_PLAY); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_PLAYPAUSE); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_YELLOW); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_BLUE); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_RED); // FIXME HACK HACK no such in pxWindowUtil
    allPxKeycodes.erase(PX_KEY_GREEN); // FIXME HACK HACK no such in pxWindowUtil
    EXPECT_TRUE (allPxKeycodes.size() == 0);
  }

  void testPxKeycodeToPrintableAsciiCoverage()
  {
    std::set<uint32_t> coverageNoDups;
    std::set<uint32_t> allPxKeycodes(mPxKeycodes.begin(), mPxKeycodes.end());
    std::set<uint32_t>::const_iterator it;
    for (it = allPxKeycodes.begin(); it != allPxKeycodes.end(); ++it)
    {
      uint32_t pxKeycode = *it;
      uint32_t ascii = keycodeToAscii(pxKeycode, 0);
      if (ascii > 0)
      {
        coverageNoDups.insert(ascii);
      }
      ascii = keycodeToAscii(pxKeycode, PX_MOD_SHIFT);
      if (ascii > 0)
      {
        coverageNoDups.insert(ascii);
      }
    }

    std::set<uint32_t> allPrintableAscii(mPrintableAscii.begin(), mPrintableAscii.end());
    for (it = coverageNoDups.begin(); it != coverageNoDups.end(); ++it)
    {
      allPrintableAscii.erase(*it);
    }
    EXPECT_TRUE (allPrintableAscii.size() == 0);
  }

  void testUnknownNativeKeycodeToZero()
  {
    uint32_t nativeKeycode = ((uint32_t)-1);
    EXPECT_TRUE (keycodeFromNative(nativeKeycode) == 0);
  }

  void testUnknownPxKeycodeToZero()
  {
    uint32_t pxKeycode = ((uint32_t)-1);
    EXPECT_TRUE (keycodeToAscii(pxKeycode, 0) == 0);
    EXPECT_TRUE (keycodeToAscii(pxKeycode, PX_MOD_SHIFT) == 0);
  }

private:
  void initNativeKeycodes()
  {
    mNativeKeycodes.push_back(PX_KEY_NATIVE_ENTER);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_BACKSPACE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_TAB);
    //mNativeKeycodes.push_back(PX_KEY_NATIVE_CANCEL); // FIXME HACK HACK in pxWindowUtil marked as TODO
    mNativeKeycodes.push_back(PX_KEY_NATIVE_CLEAR);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SHIFT);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_CONTROL);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_ALT);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_PAUSE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_CAPSLOCK);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_ESCAPE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SPACE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_PAGEUP);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_PAGEDOWN);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_END);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_HOME);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_LEFT);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_UP);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_RIGHT);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_DOWN);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_COMMA);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_PERIOD);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SLASH);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_ZERO);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_ONE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_TWO);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_THREE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_FOUR);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_FIVE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SIX);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SEVEN);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_EIGHT);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NINE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SEMICOLON);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_EQUALS);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_A);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_B);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_C);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_D);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_E);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_G);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_H);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_I);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_J);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_K);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_L);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_M);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_N);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_O);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_P);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_Q);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_R);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_S);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_T);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_U);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_V);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_W);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_X);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_Y);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_Z);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_OPENBRACKET);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_BACKSLASH);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_CLOSEBRACKET);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD0);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD1);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD2);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD3);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD4);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD5);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD6);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD7);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD8);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMPAD9);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_MULTIPLY);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_ADD);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SEPARATOR);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SUBTRACT);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_DECIMAL);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_DIVIDE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F1);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F2);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F3);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F4);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F5);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F6);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F7);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F8);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F9);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F10);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F11);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_F12);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_DELETE);
    //mNativeKeycodes.push_back(PX_KEY_NATIVE_NUMLOCK); // FIXME HACK HACK in pxWindowUtil marked as TODO
    mNativeKeycodes.push_back(PX_KEY_NATIVE_SCROLLLOCK);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_PRINTSCREEN);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_INSERT);
    //mNativeKeycodes.push_back(PX_KEY_NATIVE_HELP); // FIXME HACK HACK in pxWindowUtil marked as TODO
    mNativeKeycodes.push_back(PX_KEY_NATIVE_BACKQUOTE);
    mNativeKeycodes.push_back(PX_KEY_NATIVE_QUOTE);
    // Append here any new PX_KEY_NATIVE_... used by pxWindowUtil
  }

  void initPxKeycodes()
  {
    mPxKeycodes.push_back(PX_KEY_BACKSPACE);
    mPxKeycodes.push_back(PX_KEY_TAB);
    mPxKeycodes.push_back(PX_KEY_ENTER);
    mPxKeycodes.push_back(PX_KEY_SHIFT);
    mPxKeycodes.push_back(PX_KEY_CTRL);
    mPxKeycodes.push_back(PX_KEY_ALT);
    mPxKeycodes.push_back(PX_KEY_PAUSE);
    mPxKeycodes.push_back(PX_KEY_CAPSLOCK);
    mPxKeycodes.push_back(PX_KEY_ESCAPE);
    mPxKeycodes.push_back(PX_KEY_SPACE);
    mPxKeycodes.push_back(PX_KEY_PAGEUP);
    mPxKeycodes.push_back(PX_KEY_PAGEDOWN);
    mPxKeycodes.push_back(PX_KEY_END);
    mPxKeycodes.push_back(PX_KEY_HOME);
    mPxKeycodes.push_back(PX_KEY_LEFT);
    mPxKeycodes.push_back(PX_KEY_UP);
    mPxKeycodes.push_back(PX_KEY_RIGHT);
    mPxKeycodes.push_back(PX_KEY_DOWN);
    mPxKeycodes.push_back(PX_KEY_INSERT);
    mPxKeycodes.push_back(PX_KEY_DELETE);
    mPxKeycodes.push_back(PX_KEY_ZERO);
    mPxKeycodes.push_back(PX_KEY_ONE);
    mPxKeycodes.push_back(PX_KEY_TWO);
    mPxKeycodes.push_back(PX_KEY_THREE);
    mPxKeycodes.push_back(PX_KEY_FOUR);
    mPxKeycodes.push_back(PX_KEY_FIVE);
    mPxKeycodes.push_back(PX_KEY_SIX);
    mPxKeycodes.push_back(PX_KEY_SEVEN);
    mPxKeycodes.push_back(PX_KEY_EIGHT);
    mPxKeycodes.push_back(PX_KEY_NINE);
    mPxKeycodes.push_back(PX_KEY_A);
    mPxKeycodes.push_back(PX_KEY_B);
    mPxKeycodes.push_back(PX_KEY_C);
    mPxKeycodes.push_back(PX_KEY_D);
    mPxKeycodes.push_back(PX_KEY_E);
    mPxKeycodes.push_back(PX_KEY_F);
    mPxKeycodes.push_back(PX_KEY_G);
    mPxKeycodes.push_back(PX_KEY_H);
    mPxKeycodes.push_back(PX_KEY_I);
    mPxKeycodes.push_back(PX_KEY_J);
    mPxKeycodes.push_back(PX_KEY_K);
    mPxKeycodes.push_back(PX_KEY_L);
    mPxKeycodes.push_back(PX_KEY_M);
    mPxKeycodes.push_back(PX_KEY_N);
    mPxKeycodes.push_back(PX_KEY_O);
    mPxKeycodes.push_back(PX_KEY_P);
    mPxKeycodes.push_back(PX_KEY_Q);
    mPxKeycodes.push_back(PX_KEY_R);
    mPxKeycodes.push_back(PX_KEY_S);
    mPxKeycodes.push_back(PX_KEY_T);
    mPxKeycodes.push_back(PX_KEY_U);
    mPxKeycodes.push_back(PX_KEY_V);
    mPxKeycodes.push_back(PX_KEY_W);
    mPxKeycodes.push_back(PX_KEY_X);
    mPxKeycodes.push_back(PX_KEY_Y);
    mPxKeycodes.push_back(PX_KEY_Z);
    mPxKeycodes.push_back(PX_KEY_WINDOWKEY_LEFT);
    mPxKeycodes.push_back(PX_KEY_WINDOWKEY_RIGHT);
    mPxKeycodes.push_back(PX_KEY_SELECT);
    mPxKeycodes.push_back(PX_KEY_NUMPAD0);
    mPxKeycodes.push_back(PX_KEY_NUMPAD1);
    mPxKeycodes.push_back(PX_KEY_NUMPAD2);
    mPxKeycodes.push_back(PX_KEY_NUMPAD3);
    mPxKeycodes.push_back(PX_KEY_NUMPAD4);
    mPxKeycodes.push_back(PX_KEY_NUMPAD5);
    mPxKeycodes.push_back(PX_KEY_NUMPAD6);
    mPxKeycodes.push_back(PX_KEY_NUMPAD7);
    mPxKeycodes.push_back(PX_KEY_NUMPAD8);
    mPxKeycodes.push_back(PX_KEY_NUMPAD9);
    mPxKeycodes.push_back(PX_KEY_MULTIPLY);
    mPxKeycodes.push_back(PX_KEY_ADD);
    mPxKeycodes.push_back(PX_KEY_SUBTRACT);
    mPxKeycodes.push_back(PX_KEY_DECIMAL);
    mPxKeycodes.push_back(PX_KEY_DIVIDE);
    mPxKeycodes.push_back(PX_KEY_F1);
    mPxKeycodes.push_back(PX_KEY_F2);
    mPxKeycodes.push_back(PX_KEY_F3);
    mPxKeycodes.push_back(PX_KEY_F4);
    mPxKeycodes.push_back(PX_KEY_F5);
    mPxKeycodes.push_back(PX_KEY_F6);
    mPxKeycodes.push_back(PX_KEY_F7);
    mPxKeycodes.push_back(PX_KEY_F8);
    mPxKeycodes.push_back(PX_KEY_F9);
    mPxKeycodes.push_back(PX_KEY_F10);
    mPxKeycodes.push_back(PX_KEY_F11);
    mPxKeycodes.push_back(PX_KEY_F12);
    mPxKeycodes.push_back(PX_KEY_NUMLOCK);
    mPxKeycodes.push_back(PX_KEY_SCROLLLOCK);
    mPxKeycodes.push_back(PX_KEY_SEMICOLON);
    mPxKeycodes.push_back(PX_KEY_EQUALS);
    mPxKeycodes.push_back(PX_KEY_COMMA);
    mPxKeycodes.push_back(PX_KEY_DASH);
    mPxKeycodes.push_back(PX_KEY_PERIOD);
    mPxKeycodes.push_back(PX_KEY_FORWARDSLASH);
    mPxKeycodes.push_back(PX_KEY_GRAVEACCENT);
    mPxKeycodes.push_back(PX_KEY_OPENBRACKET);
    mPxKeycodes.push_back(PX_KEY_BACKSLASH);
    mPxKeycodes.push_back(PX_KEY_CLOSEBRACKET);
    mPxKeycodes.push_back(PX_KEY_SINGLEQUOTE);
    mPxKeycodes.push_back(PX_KEY_PRINTSCREEN);
    mPxKeycodes.push_back(PX_KEY_FASTFORWARD);
    mPxKeycodes.push_back(PX_KEY_REWIND);
    mPxKeycodes.push_back(PX_KEY_PLAY);
    mPxKeycodes.push_back(PX_KEY_PLAYPAUSE);
    mPxKeycodes.push_back(PX_KEYDOWN_REPEAT);
    mPxKeycodes.push_back(PX_KEY_YELLOW);
    mPxKeycodes.push_back(PX_KEY_BLUE);
    mPxKeycodes.push_back(PX_KEY_RED);
    mPxKeycodes.push_back(PX_KEY_GREEN);
    // Append here any new PX_KEY_... defined in pxKeycodes.h
  }

  void initPrintableAscii()
  {
    for (uint32_t i = 0; i < 256; i++)
    {
      if (isprint(i) != 0)
      {
        mPrintableAscii.insert(i);
      }
    }
  }

  std::vector<uint32_t> mPxKeycodes;
  std::vector<uint32_t> mNativeKeycodes;
  std::set<uint32_t> mPrintableAscii;
};

TEST_F(pxWindowUtilTest, pxWindowUtilTests)
{
  testNativeKeycodesHaveNoDuplicates();
  testPxKeycodesHaveNoDuplicates();
  testNativeKeycodeToValidPxKeycode();
  testPxKeycodeToValidPrintableAscii();
  testNativeKeycodeToPxKeycodeCoverage();
  testPxKeycodeToPrintableAsciiCoverage();
  testUnknownNativeKeycodeToZero();
  testUnknownPxKeycodeToZero();
}
