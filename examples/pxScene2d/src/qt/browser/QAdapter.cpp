#include "QAdapter.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include "rtLog.h"
#include <QCoreApplication>
#ifdef WIN32
#include "qt/browser/win32/qtwinmigrate/qwinwidget.h"
#elif __APPLE__

#include "qt/browser/mac/qmacwidget.h"

#endif

QAdapter::QAdapter(): mView(nullptr), mRootWidget(nullptr), mQtArgv(nullptr), mQtArgc(0), mQtApp(nullptr)
{
}

void QAdapter::init(void *root, int w, int h)
{
  mQtApp = new QApplication(mQtArgc, mQtArgv);
#ifdef WIN32
  HWND* hwnd = (HWND*)root;
  QWinWidget * r = new QWinWidget(*hwnd);
  r->setStyleSheet("background-color:transparent;");
  r->setGeometry(0, 0, w, h);
  this->mRootWidget = (void *)r;
#elif __APPLE__
  QMacWidget *r = new QMacWidget(root);
  r->init();
  r->setStyleSheet("background-color:red;");
  r->setGeometry(0, 0, w, h);
  this->mRootWidget = (void *) r;
  r->setView(mView);
#endif

  rtLogInfo("finished QT init, w= %d, h = %d, root = %p", w, h, root);
}

void QAdapter::update()
{
  if (mQtApp) {
    mQtApp->sendPostedEvents();
  }
}


void QAdapter::resize(int w, int h)
{
#ifdef WIN32
  QWinWidget* r = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* r = (QMacWidget*) mRootWidget;
#endif
  if (r) {
    rtLogInfo("QT resize w = %d, h = %d, root = %p", w, h, r);
    r->setGeometry(0, 0, w, h);
  }
}

void QAdapter::setView(pxIView *v)
{
  mView = v;

#ifdef WIN32
  QWinWidget* r = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* r = (QMacWidget*) mRootWidget;
  if (r)
  {
    r->setView(v);
  }
#endif
}


void *QAdapter::getRootWidget() const
{
  return mRootWidget;
}
