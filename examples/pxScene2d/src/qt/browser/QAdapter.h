#ifndef PXSCENE_QADAPTER_H
#define PXSCENE_QADAPTER_H

#include <QApplication>
#include "pxIView.h"

class QAdapter
{

public:

  QAdapter();

  /**
   * init adapter
   * @param root the window handler
   * @param w the width
   * @param h the height
   */
  void init(void *root, int w, int h);

  /**
   * qt update
   */
  void update();

  /**
   * get qt root widget
   */
  void *getRootWidget() const;

  /**
   * resize qt root
   * @param w the width
   * @param h the height
   */
  void resize(int w, int h);

  /**
   * set view
   * @param v the pxView
   */
  void setView(pxIView* v);

private:
  pxIView* mView;
  void *mRootWidget;
  QApplication *mQtApp;
  int mQtArgc;
  char** mQtArgv;
};


#endif