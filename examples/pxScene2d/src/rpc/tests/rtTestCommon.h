#include <limits.h>
#include "../../rtObject.h"
#include "../../rtError.h"
#include "../../rtValue.h"
#include "../../rtString.h"
#include "../../rtObjectMacros.h"
#include <map>
//#include <rapidjson/stringbuffer.h>
#include <functional>

class rtLcd : public rtObject
{
public:
  rtDeclareObject(rtLcd, rtObject);
  rtProperty(text, text, setText, rtString);
  rtProperty(height, height, setHeight, uint32_t);
  rtProperty(width, width, setWidth, uint32_t);

  rtString text() const { return m_text; }
  rtError  text(rtString& s) const { s = m_text; return RT_OK; }
  rtError  setText(rtString const& s) { m_text = s; return RT_OK; }

  uint32_t height() const { return m_height; }
  rtError  height(uint32_t& s) const { s = m_height; return RT_OK; }
  rtError  setHeight(uint32_t s) { m_height = s; return RT_OK; }

  uint32_t width() const { return m_width; }
  rtError  width(uint32_t& s) const { s = m_width; return RT_OK; }
  rtError  setWidth(uint32_t s) { m_width = s; return RT_OK; }

private:
  rtString m_text;
  uint32_t m_width;
  uint32_t m_height;
};

class rtThermostat : public rtObject
{
public:
  rtDeclareObject(rtThermostat, rtObject);
  rtProperty(lcd, lcd, setLcd, rtObjectRef);
  rtProperty(onTempChanged, onTempChanged, setOnTempChanged, rtFunctionRef);

  rtObjectRef lcd() const { return m_lcd; }
  rtError     lcd(rtObjectRef& ref) const { ref = m_lcd; return RT_OK; }
  rtError     setLcd(rtObjectRef lcd) { m_lcd = lcd; return RT_OK; }

  rtFunctionRef onTempChanged() const { return m_onTempChanged; } 
  rtError onTempChanged(rtFunctionRef& ref) const { ref = m_onTempChanged; return RT_OK; } 
  rtError setOnTempChanged(rtFunctionRef ref) { m_onTempChanged = ref; return RT_OK; } 

  rtError hello()
  {
    printf("hello!\n");
    return RT_OK;
  }

private:
  rtObjectRef 		m_lcd;
  rtFunctionRef 	m_onTempChanged;
};


rtDefineObject(rtLcd,rtObject);
rtDefineProperty(rtLcd,text);
rtDefineProperty(rtLcd,width);
rtDefineProperty(rtLcd,height);

rtDefineObject(rtThermostat,rtObject);
rtDefineProperty(rtThermostat,lcd);
rtDefineProperty(rtThermostat,onTempChanged);
