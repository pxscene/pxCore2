#include "qmacwidget.h"
#include "rtLog.h"
#include <Cocoa/Cocoa.h>

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
