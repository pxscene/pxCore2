#include "qmacwidget.h"
#include <Cocoa/Cocoa.h>

QMacWidget::QMacWidget(void* window) : QMacNativeWidget(nullptr)
{
  this->mWindow = window;

}

void QMacWidget::init()
{
  NSView* contentView = [((NSWindow*) this->mWindow) contentView];
  NSView* nativeWidgetView = reinterpret_cast<NSView*>(this->winId());
  [contentView addSubview:nativeWidgetView];
}

QMacWidget::~QMacWidget()
{
  // no need do anything here
}