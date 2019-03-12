#include "qmacwidget.h"
#include "rtLog.h"
#include "pxKeycodes.h"
#include "pxWindowUtil.h"
#include <Cocoa/Cocoa.h>
#include <QKeyEvent>
#include <QApplication>


QMacWidget::QMacWidget(void* window) : QMacNativeWidget(nullptr)
{
  this->mWindow = window;
  mWebViewList.clear();
}

void QMacWidget::init()
{
  NSView* contentView = [((NSWindow*) this->mWindow) contentView];
  NSView* nativeWidgetView = reinterpret_cast<NSView*>(this->winId());
  [contentView addSubview:nativeWidgetView];
  nativeWidgetView.alphaValue = 0;
  installEventFilter(this);
}

QMacWidget::~QMacWidget()
{
  // no need do anything here
}

bool QMacWidget::eventFilter(QObject* pObject, QEvent* pEvent)
{

  // why child webview cannot receive UpdateRequest event ??
  // so here dispatch event to child view
  if (pEvent->type() == QEvent::Type::UpdateRequest || pEvent->type() == QEvent::Type::LayoutRequest)
  {
    for (QObject* object: mWebViewList)
    {
      object->eventFilter(object, pEvent);
    }
  }
  
  if(pEvent->type() ==  QEvent::Type::MouseButtonPress
     || pEvent->type() ==  QEvent::Type::MouseButtonRelease
     || pEvent->type() ==  QEvent::Type::MouseMove
     ){
    const QMouseEvent* const me = static_cast<const QMouseEvent*>( pEvent );
    if(!mView){
      return false;
    }
    uint32_t flags = 0;
    QPoint p = me->pos();
    
    if(me->button() == Qt::MouseButton::LeftButton){
      flags = PX_LEFTBUTTON;
    }
    if(me->button() == Qt::MouseButton::RightButton){
      flags = PX_RIGHTBUTTON;
    }
    if(me->button() == Qt::MouseButton::MidButton){
      flags = PX_MIDDLEBUTTON;
    }
    
    if(Qt::MetaModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_CONTROL;
    if(Qt::ShiftModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_SHIFT;
    if(Qt::AltModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_ALT;
    if(Qt::ControlModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_COMMAND;
    
    if(pEvent->type() ==  QEvent::Type::MouseButtonPress){
      mView->onMouseDown(p.x(), p.y(), flags);
    }else if (pEvent->type() ==  QEvent::Type::MouseButtonRelease){
      mView->onMouseUp(p.x(), p.y(), flags);
    }else{
      mView->onMouseMove(p.x(), p.y());
    }
    return false;
  }
  
  
  if(pEvent->type() ==  QEvent::Type::KeyPress
     || pEvent->type() == QEvent::Type::KeyRelease
     ){
    const QKeyEvent* const ke = static_cast<const QKeyEvent*>( pEvent );
    if(!mView){
      return false;
    }
    uint32_t flags = 0;
    
    if(Qt::MetaModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_CONTROL;
    if(Qt::ShiftModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_SHIFT;
    if(Qt::AltModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_ALT;
    if(Qt::ControlModifier == QApplication::keyboardModifiers()) flags |= PX_MOD_COMMAND;
    
    if(ke->isAutoRepeat())
      flags |= PX_KEYDOWN_REPEAT;
   
    rtLogInfo("key = %d", ke->key() );
    if(pEvent->type() ==  QEvent::Type::KeyPress){
      mView->onKeyDown(ke->key(), flags);
    }else{
      mView->onKeyUp(ke->key(), flags);
    }
  }
  return QObject::eventFilter(pObject, pEvent);
}

void QMacWidget::addWebView(QObject* wb)
{
  mWebViewList.push_back(wb);
}

void QMacWidget::removeWebview(QObject* wb)
{
  std::remove(mWebViewList.begin(), mWebViewList.end(), wb);
}
