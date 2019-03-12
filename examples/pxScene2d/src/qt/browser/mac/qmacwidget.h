#include <QMacNativeWidget>
#include <list>
#include <QEvent>
#include "pxIView.h"

/**
 * QMacWidget use attach qt view to root content NSView
 * QMacNativeWidget need build in mm file
 */
class QMacWidget : public QMacNativeWidget
{
public:
  QMacWidget(void* window);

  ~QMacWidget();

  /**
   * attach qt view to mac nsview
   */
  void init();

  /**
   * add web view
   */
  void addWebView(QObject*);

  /**
   * remove web view
   */
  void removeWebview(QObject*);

  /**
   * set view
   * @param v the view
   */
  void setView(pxIView* v) {
    mView = v;
  }

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;
  std::list<QObject*> mWebViewList;

protected:
  void* mWindow;
  pxIView* mView;
};