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

void test(int l, char** args){
  rtLogSetLevel(rtLogLevel::RT_LOG_INFO);
  rtLogInfo("??????? %d", l);
  for (int i = 0; i< l; i++) {
    rtLogInfo("arg %d = %s",i,args[i]);
  }
  
}

QApplication *qtApp = nullptr;

QAdapter::QAdapter(): mView(nullptr), mRootWidget(nullptr)
{
  char *c1 = "-nokeyboard=true";
  char *c2 = "-nomouse=true";
  char *c3 = "-style=windows";
  char* a[3];
  a[0] = c1;
  a[1] = c2;
  a[2] = c3;
  int __argc = 0;
  test(__argc,a);
  
  QStringList strList = QCoreApplication::arguments();
  
  qtApp = new QApplication(__argc,a);
}

void QAdapter::init(void *root, int w, int h)
{
#ifdef WIN32
  HWND* hwnd = (HWND*)root;
  this->mRootWidget = new QWinWidget(*hwnd);
#elif __APPLE__
  QMacWidget *r = new QMacWidget(root);
  r->init();
  r->setStyleSheet("background-color:red;");
  r->setGeometry(0, 0, w, h);
  this->mRootWidget = (void *) r;
#endif

  rtLogInfo("finished QT init, w= %d, h = %d, root = %p", w, h, root);
//  r->show(); // let it full
  r->setView(mView);
}

void QAdapter::update()
{
  qtApp->sendPostedEvents();
}


void QAdapter::resize(int w, int h)
{
#ifdef WIN32
  QWinWidget* r = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* r = (QMacWidget*) mRootWidget;
#endif

  rtLogInfo("QT resize w = %d, h = %d, %p =",w,h,r);
  r->setGeometry(0, 0, w, h);
}

void QAdapter::setView(pxIView *v)
{
  mView = v;

#ifdef WIN32
  QWinWidget* r = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* r = (QMacWidget*) mRootWidget;
#endif
  if (r)
  {
    r->setView(v);
  }
}


void *QAdapter::getRootWidget() const
{
  return mRootWidget;
}
