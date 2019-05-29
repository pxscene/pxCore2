#ifndef __pxBrowser_H__
#define __pxBrowser_H__


/**
 * pxBrowser View class
 * this class use QT web engine render web page
 */

#include "pxScene2d.h"

#include "pxOffscreen.h"
#include "pxTexture.h"

#include <string>
#include <map>

#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineCookieStore>
#include <QNetworkCookieJar>
#include <QWebEngineUrlRequestInterceptor>
#include <QNetworkProxy>
#include <QImage>
#include <QKeyEvent>
#include <QEvent>



/**
 * pxBrowser class, used to save console log item from engine
 */
class pxBrowserConsoleLog : public rtObject{
public:
  rtDeclareObject(pxBrowserConsoleLog, rtObject);
  rtReadOnlyProperty(level, level, int32_t);
  rtReadOnlyProperty(logMessage, message, rtString);
  rtReadOnlyProperty(lineNumber, lineNumber, int32_t);
  rtReadOnlyProperty(resourceId, resourceId, rtString);

  rtError level(int32_t& v) const { v = mLevel; return RT_OK; }
  rtError lineNumber(int32_t& v) const { v = mLineNumber; return RT_OK; }
  rtError message(rtString& v) const { v = mMessage; return RT_OK; }
  rtError resourceId(rtString& v) const { v = mResourceId; return RT_OK; }

  void setLevel(int32_t const& v) { mLevel = v; }
  void setMessage(rtString const& v) { mMessage = v; }
  void setLineNumber(int32_t const& v) { mLineNumber = v; }
  void setResourceId(rtString const& v) { mResourceId = v; }
private:
  int32_t mLevel;
  rtString mMessage;
  rtString mResourceId;
  int32_t mLineNumber;
};


/**
 * pxBrowserLink event class, it used to passed url value when link clicked
 */
class pxBrowserLink : public rtObject {
public:
  rtDeclareObject(pxBrowserLink, rtObject);
  rtReadOnlyProperty(value, value, rtString);

  rtError value(rtString& v) const { v = mValue; return RT_OK; }
  void setValue(rtString const& v) { mValue = v; }
private:
  rtString mValue;
};



/**
 * pxBrowser paged loaded event class, it used to passed event when page loaded
 */
class pxBrowserPageLoaded : public rtObject {
public:
  rtDeclareObject(pxBrowserPageLoaded, rtObject);
  rtReadOnlyProperty(success, success, bool);
  rtReadOnlyProperty(httpStatus, httpStatus, int32_t);

  rtError httpStatus(int32_t& v) const { v = mHttpStatus; return RT_OK; }
  rtError success(bool& v) const { v = mSuccess; return RT_OK; }

  void setSuccess(bool const& v) { mSuccess = v; }
  void setHttpStatus(int32_t const& v) { mHttpStatus = v; }
private:
  int32_t mHttpStatus;
  bool mSuccess;
};


/**
 * pxBrowser Proxy class, it used to store the proxy object
 */
class pxBrowserProxy : public rtObject {
public:
  rtDeclareObject(pxBrowserProxy, rtObject);
  rtProperty(type, getType, setType, int);
  rtProperty(hostname, getHostname, setHostname, rtString);
  rtProperty(port, getPort, setPort, int);

  rtError getType(int& v) const { v = mType; return RT_OK; }
  rtError getHostname(rtString& v) const { v = mHostname; return RT_OK; }
  rtError getPort(int& v) const { v = mPort; return RT_OK; }

  rtError setType(int const& v)  { mType = v; return RT_OK; }
  rtError setPort(int const& v)  { mPort = v; return RT_OK; }
  rtError setHostname(rtString const& v)  { mHostname = v; return RT_OK; }
private:
  int mType;
  int mPort;
  rtString mHostname;
};




/**
 * px web error class
 */
struct pxQTWebError
{
  int errorType;
  std::string description;
};


/**
 * events callback defination
 */
typedef std::function<void(int level, char const* message, int lineNumber, char const* resourceId)> ConsoleLogFunc;
typedef std::function<void(char const*url)> HTMLLinkClickedFunc;
typedef std::function<void(bool ok)> HTMLDocumentLoadedFunc;
typedef std::function<void(pxQTWebError const& error)> ErrorFunc;
typedef std::function<void(QNetworkCookie const&)> CookieJarChangedFunc;

/**
 * Request Interceptor, used to add custom headers for page request
 */
class pxWebUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
  pxWebUrlRequestInterceptor(QObject *parent = Q_NULLPTR) : QWebEngineUrlRequestInterceptor(parent)
  {

  }

  void interceptRequest(QWebEngineUrlRequestInfo &info) {
    // add headers
    std::map<std::string, std::string>::iterator headerIt;
    for (headerIt = mHeaders.begin(); headerIt != mHeaders.end(); ++headerIt) {
      info.setHttpHeader(headerIt->first.c_str(), headerIt->second.c_str());
    }
  }

  void setHeaders(std::map<std::string, std::string> const& headers) {
    mHeaders = headers;
  }
private:
  std::map<std::string, std::string> mHeaders;
};

/**
 * qt web engine page class
 */
class pxQTWebPage : public QWebEnginePage
{
public:
  pxQTWebPage(QObject* parent = 0) :
      QWebEnginePage(parent)
      , mIsFirstLoad(true)
      , mConsoleLogEnabled(true)
      , mConsoleFunc(nullptr)
      , mLoadedFunc(nullptr)
      , mLinkClickedFunc(nullptr)
      , mCookieChangedFunc(nullptr)
  {

    /**
     * receive loadFinished event
     */
    QObject::connect(this, QOverload<bool>::of(&pxQTWebPage::loadFinished), this, [this](bool ok) {
      if (mLoadedFunc) {
        mLoadedFunc(ok);
      }
    });

    /**
    * receive link clicked event
    */
    QObject::connect(this, QOverload<QUrl const &>::of(&pxQTWebPage::urlChanged), this, [this](QUrl const & url) {
      if (mIsFirstLoad) {
        mIsFirstLoad = false;
        return;
      }
      QByteArray arr = url.toString().toLocal8Bit();
      if (mLinkClickedFunc) {
        mLinkClickedFunc(arr.constData());
      }
    });

    mCookies.clear();


    /**
    * receive cookies changed event
    */
    QObject::connect(this->profile()->cookieStore(), QOverload<const QNetworkCookie &>::of(&QWebEngineCookieStore::cookieAdded),
                     this, [this](const QNetworkCookie & cookie) {
          if (mCookieChangedFunc) {
            mCookieChangedFunc(cookie);
          }
          mCookies.append(cookie);
        });

    /**
    * receive  cookies changed event
    */
    QObject::connect(this->profile()->cookieStore(), QOverload<const QNetworkCookie &>::of(&QWebEngineCookieStore::cookieRemoved),
                     this, [this](const QNetworkCookie & cookie) {
          if (mCookieChangedFunc) {
            mCookieChangedFunc(cookie);
          }
          mCookies.removeOne(cookie);
        });
  }

  /**
   * overite console function for page
   * @param level  the log level
   * @param message the log message
   * @param lineNumber  the line number
   * @param sourceID  the console file
   */
  virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
  {
    if (!mConsoleLogEnabled) {
      return;
    }
    if (mConsoleFunc) {
      mConsoleFunc(level, message.toLocal8Bit().constData(), lineNumber, sourceID.toLocal8Bit().constData());
    }
  }
  virtual void load(const QUrl &url) {
    mIsFirstLoad = true;
    QWebEnginePage::load(url);
  }

  virtual void load(const QWebEngineHttpRequest &request) {
    mIsFirstLoad = true;
    QWebEnginePage::load(request);
  }

  void setConsoleLogEnabled(bool enabled) {
    mConsoleLogEnabled = enabled;
  }

  bool isConsoleLogEnabled() {
    return mConsoleLogEnabled;
  }

  void setHTMLDocumentLoadedFunc(HTMLDocumentLoadedFunc loadedFunc) {
    this->mLoadedFunc = loadedFunc;
  }

  void setConsoleLogFunc(ConsoleLogFunc consoleFunc) {
    this->mConsoleFunc = consoleFunc;
  }

  void setHTMLLinkClickedFunc(HTMLLinkClickedFunc linkClickedFunc) {
    this->mLinkClickedFunc = linkClickedFunc;
  }

  void setCookieJarChangedFunc(CookieJarChangedFunc cookieChangedFunc) {
    this->mCookieChangedFunc = cookieChangedFunc;
  }

  QList<QNetworkCookie> getCookies() {
    return mCookies;
  }

private:
  bool mIsFirstLoad;
  bool mConsoleLogEnabled;
  ConsoleLogFunc mConsoleFunc;
  HTMLDocumentLoadedFunc mLoadedFunc;
  HTMLLinkClickedFunc mLinkClickedFunc;
  CookieJarChangedFunc mCookieChangedFunc;
  QList<QNetworkCookie> mCookies;
};

/**
 * px qt web view
 */
class pxQTWebView : public QWebEngineView
{
public:
  pxQTWebView(QWidget* parent) : QWebEngineView(parent)
  {
    installEventFilter(this);
  }

  bool eventFilter(QObject* pObject, QEvent* pEvent) override
  {
    if (pEvent->type() == QEvent::Type::UpdateRequest || pEvent->type() == QEvent::Type::LayoutRequest)
    {
    }

    if (pEvent->type() == QEvent::Type::MouseButtonPress
        || pEvent->type() == QEvent::Type::MouseButtonRelease
        || pEvent->type() == QEvent::Type::MouseMove
        )
    {
      const QMouseEvent* const me = static_cast<const QMouseEvent*>(pEvent);
      if(me->modifiers() != Qt::KeyboardModifierMask){
        return false;
      }else{
        rtLogInfo("got event %d %d", me->pos().x(), me->pos().y());
        return QObject::eventFilter(pObject, pEvent);
      }
    }
    
    return QObject::eventFilter(pObject, pEvent);
  }
  
  bool eventFilter2(QObject* pObject, QEvent* pEvent) {
    return QObject::eventFilter(pObject, pEvent);
  }

  void mousePressEvent(QMouseEvent *e) override {
    rtLogInfo("mousePressEvent ??");
    QWidget::mousePressEvent(e);
  }
  void mouseReleaseEvent(QMouseEvent *e) override {
    QWidget::mouseReleaseEvent(e);
  }
  void mouseMoveEvent(QMouseEvent *e) override {
    QWidget::mouseMoveEvent(e);
  }
};

/**
 * pxBrowser class
 */
class pxBrowser : public pxObject
{
public:

  rtDeclareObject(pxBrowser, pxObject);
  rtProperty(url, getUrl, updateUrl, rtString);
  rtProperty(cookieJar, getCookieJar, setCookieJar, rtObjectRef);
  rtProperty(proxy, getProxy, setProxy, rtObjectRef);
  rtProperty(userAgent, getUserAgent, setUserAgent, rtString);

  rtProperty(transparentBackground, getTransparentBackground, setTransparent, bool);
  rtProperty(visible, getVisible, setVisible, bool);
  rtProperty(localStorageEnabled, getLocalStorageEnabled, setLocalStorageEnabled, bool);
  rtProperty(consoleLogEnabled, getConsoleLogEnabled, setConsoleLogEnabled, bool);
  rtProperty(headers, getHeaders, setHeaders, rtObjectRef);


  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }

  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }


  pxBrowser(pxScene2d* scene);

  void initQT();

  rtError getUrl(rtString& url) const { url = rtString(mUrl.c_str()); return RT_OK; };
  rtError updateUrl(rtString const& newUrl);
  rtError getUserAgent(rtString& v) const { v = rtString(mUserAgent.c_str()); return RT_OK; };
  rtError setUserAgent(rtString const& userAgent);

  rtError getTransparentBackground(bool& v) const;
  rtError setTransparent(bool const& transparent);

  bool isVisible() const;
  rtError getVisible(bool& v) const { v = isVisible(); return RT_OK; };
  rtError setVisible(bool const& v);

  bool isLocalStorageEnabled() const;
  rtError getLocalStorageEnabled(bool& v) const { v = isLocalStorageEnabled(); return RT_OK; };
  rtError setLocalStorageEnabled(bool const& v);

  bool isConsoleLogEnabled() const;
  rtError getConsoleLogEnabled(bool& v) const { v = isConsoleLogEnabled(); return RT_OK; };
  rtError setConsoleLogEnabled(bool const& v);



  rtError getHeaders(rtObjectRef& v) const;
  rtError setHeaders(rtObjectRef const& v);
  void setHeaders(std::map<std::string, std::string> const& headers);
  std::map<std::string, std::string> const& getHeaders();


  void dumpProperties();


  rtError getCookieJar(rtObjectRef& v) const;
  rtError setCookieJar(rtObjectRef const& v);

  rtError setProxy(rtObjectRef const& v);
  rtError getProxy(rtObjectRef &v) const { v = mProxy; return RT_OK; };

  /**
   * add on event listener for event
   * @param eventName  the event name
   * @param f the listener function
   */
  rtError addListener(rtString eventName, const rtFunctionRef& f);

  /**
   * convert QNetworkCookie to rtMapObject
   * @param cookie the cookie
   * @return rtObject
   */
  static rtMapObject* convertCookieToMap(QNetworkCookie const& cookie);

  /**
   * update
   * @param t the dela time
   */
  virtual void update(double t);

  /**
   * draw page
   */
  virtual void draw();

  virtual rtError setW(float v) {
    rtError ret = pxObject::setW(v);
    updateSize();
    return ret;
  }

  virtual rtError setH(float v)
  {
    rtError ret = pxObject::setH(v);
    updateSize();
    return ret;
  }

  virtual void onInit(){
    updateSize();
  }

  void updateSize();
  
  pxQTWebView * getWebView(){
    return mWebView;
  }

  static rtError onMouseDown(int argc, rtValue const* argv, rtValue* result, void* context)
  {
    rtObjectRef o = argv[0].toObject();
    pxBrowser* that = static_cast<pxBrowser*>(context);
    
   

    int x = o.get<int>("x");
    int y = o.get<int>("y");
    uint32_t flags = o.get<uint32_t>("flags");

    QPoint point(x,y);
    QMouseEvent e(QEvent::Type::MouseButtonPress, point, Qt::MouseButton::LeftButton, 0, Qt::KeyboardModifierMask);
    that->getWebView()->mousePressEvent(&e);
    rtLogInfo("onMouseDown %d %d",x,y);
    return RT_OK;
  }

  static rtError onMouseMove(int argc, rtValue const* argv, rtValue* result, void* context)
  {
    rtObjectRef o = argv[0].toObject();
    pxBrowser* that = static_cast<pxBrowser*>(context);
    int x = o.get<int>("x");
    int y = o.get<int>("y");

    QPoint point(x,y);
    QMouseEvent e(QEvent::Type::MouseMove, point, Qt::MouseButton::LeftButton, 0, Qt::KeyboardModifierMask);
    that->getWebView()->mouseMoveEvent(&e);
    return RT_OK;
  }

  static rtError onMouseUp(int argc, rtValue const* argv, rtValue* result, void* context)
  {
    rtObjectRef o = argv[0].toObject();
    pxBrowser* that = static_cast<pxBrowser*>(context);


    int x = o.get<int>("x");
    int y = o.get<int>("y");
    uint32_t flags = o.get<uint32_t>("flags");
    
    QPoint point(x,y);
    QMouseEvent e(QEvent::Type::MouseButtonRelease, point, Qt::MouseButton::LeftButton, 0, Qt::KeyboardModifierMask);
    that->getWebView()->mouseReleaseEvent(&e);
    return RT_OK;
  }


protected:
  std::string mUrl;
  std::map<std::string, std::string> mHeaders;

  std::string mUserAgent;

  pxQTWebView *mWebView;
  QNetworkAccessManager* mNetworkManager;
  pxWebUrlRequestInterceptor* webUrlRequestInterceptor;
  rtObjectRef mProxy;
  void* mRootWidget;

  QImage * mRenderQImage;
  pxOffscreen * mOffscreen;
  pxTextureRef mTextureRef;
};

#endif
