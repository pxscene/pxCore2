#include <QMacNativeWidget>

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

protected:
  void* mWindow;
};