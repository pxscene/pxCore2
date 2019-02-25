#include <string>
#include <map>
#include "rtLog.h"

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QRgb>

#include "pxBrowserView.h"
#include "pxContext.h"

#ifdef WIN32
#include "qt/browser/win32/qtwinmigrate/qwinwidget.h"
#elif __APPLE__
#include "qt/browser/mac/qmacwidget.h"
#endif

int argc = 0;
static QApplication *globalQTApp = new QApplication(argc, 0);
extern pxContext context;

//pxBrowserConsoleLog properties
rtDefineObject(pxBrowserConsoleLog, rtObject);
rtDefineProperty(pxBrowserConsoleLog, level);
rtDefineProperty(pxBrowserConsoleLog, logMessage);
rtDefineProperty(pxBrowserConsoleLog, lineNumber);
rtDefineProperty(pxBrowserConsoleLog, resourceId);

//pxBrowserLink properties
rtDefineObject(pxBrowserLink, rtObject);
rtDefineProperty(pxBrowserLink, value);

//pxBrowserPageLoaded properties
rtDefineObject(pxBrowserPageLoaded, rtObject);
rtDefineProperty(pxBrowserPageLoaded, success);
rtDefineProperty(pxBrowserPageLoaded, httpStatus);

//pxBrowserProxy properties
rtDefineObject(pxBrowserProxy, rtObject);
rtDefineProperty(pxBrowserProxy, type);
rtDefineProperty(pxBrowserProxy, hostname);
rtDefineProperty(pxBrowserProxy, port);

//pxBrowserView properties
rtDefineObject(pxBrowserView, rtObject);
rtDefineProperty(pxBrowserView, url);
rtDefineProperty(pxBrowserView, cookieJar);
rtDefineProperty(pxBrowserView, proxy);
rtDefineProperty(pxBrowserView, userAgent);
rtDefineProperty(pxBrowserView, transparentBackground);
rtDefineProperty(pxBrowserView, visible);
rtDefineProperty(pxBrowserView, localStorageEnabled);
rtDefineProperty(pxBrowserView, consoleLogEnabled);
rtDefineProperty(pxBrowserView, headers);
// pxBrowserView function
rtDefineMethod(pxBrowserView, addListener);

pxBrowserView::pxBrowserView(void* root, int w, int h) :
    mWidth(0), mHeight(0), mEmit(new rtEmit()), mNetworkManager(nullptr),
    webUrlRequestInterceptor(nullptr), mProxy(nullptr), mDirty(false), mRenderQImage(nullptr)
    , mOffscreen(nullptr), mTextureRef(nullptr)
{

#ifdef WIN32
  HWND* hwnd = (HWND*)root;
  this->mRootWidget = new QWinWidget(*hwnd);
#elif __APPLE__
  QMacWidget* r = new QMacWidget(root);
  r->init();
  this->mRootWidget = (void*)r;
#endif

  mHeaders.clear();
  mUrl.empty();
  mUserAgent.empty();

  this->mWidth = w;
  this->mHeight = h;
  mNetworkManager = new QNetworkAccessManager();
  webUrlRequestInterceptor = new pxWebUrlRequestInterceptor();
  mRenderQImage = new QImage(mWidth, mHeight, QImage::Format_ARGB32);
  mOffscreen = new pxOffscreen();
  mOffscreen->init(mWidth,mHeight);
  mRenderQImage->fill(qRgba(255, 0, 0, 0));
  mTextureRef = context.createTexture(*mOffscreen);
}


void pxBrowserView::initQT()
{

#ifdef WIN32
  QWinWidget* win = (QWinWidget*) mRootWidget;
  ::SetFocus((HWND) win->winId());
#elif __APPLE__
  QMacWidget* win = (QMacWidget*) mRootWidget;
#endif

  rtLogInfo("pxBrowserView init w= %d, h=%d", mWidth, mHeight);
  win->setGeometry(0, 0, mWidth, mHeight);

  // create new QWebEngineView and attach to window
  this->mWebView = new pxQTWebView(win);
  pxQTWebPage* wPage = new pxQTWebPage();

  this->mWebView->setPage(wPage);
  this->mWebView->page()->profile()->setRequestInterceptor(webUrlRequestInterceptor);
  this->mWebView->show();

  this->mWebView->setGeometry(0, 0, mWidth, mHeight);

  win->move(0, 0);
  win->show();


  // binding ConsoleLogFunc
  wPage->setConsoleLogFunc([&](int level, char const* messgae, int lineNumber, char const* resourceId)
                           {

                             pxBrowserConsoleLog * item = new pxBrowserConsoleLog();
                             item->setLevel(level);
                             item->setMessage(messgae);
                             item->setLineNumber(lineNumber);
                             item->setResourceId(resourceId);
                             this->mEmit.send("ConsoleLog", item);

                             if (level == QWebEnginePage::JavaScriptConsoleMessageLevel::ErrorMessageLevel)
                             {
                               rtMapObject* error = new rtMapObject();

                               rtValue errorType("console");
                               error->Set("errorType", &errorType);

                               rtValue description(messgae);
                               error->Set("description", &description);
                               this->mEmit.send("Error", error);
                             }
                           });
  // binding HTMLDocumentLoadedFunc
  wPage->setHTMLDocumentLoadedFunc([&](bool succeed)
                                   {

                                     pxBrowserPageLoaded * item = new pxBrowserPageLoaded();
                                     item->setSuccess(succeed);
                                     item->setHttpStatus(0);
                                     this->mEmit.send("HTMLDocumentLoaded", item);

                                     if (!succeed)
                                     {
                                       rtMapObject* error = new rtMapObject();

                                       rtValue errorType("load");
                                       error->Set("errorType", &errorType);

                                       rtValue description("load failed");
                                       error->Set("description", &description);
                                       this->mEmit.send("Error", error);
                                     }
                                   });

  // binding HTMLLinkClickedFunc
  wPage->setHTMLLinkClickedFunc([&](char const* url)
                                {
                                  pxBrowserLink * link = new pxBrowserLink();
                                  link->setValue(url);
                                  this->mEmit.send("HTMLLinkClicked", link);
                                });

  // binding CookieJarChangedFunc
  wPage->setCookieJarChangedFunc([&](QNetworkCookie const& cookie)
                                 {
                                   rtMapObject* cook = new rtMapObject();
                                   rtObjectRef
                                   ref = pxBrowserView::convertCookieToMap(cookie);
                                   this->mEmit.send("CookieJarChanged", ref);
                                 });

  setVisible(true);
}

rtMapObject* pxBrowserView::convertCookieToMap(QNetworkCookie const& cookie)
{
  rtMapObject* cook = new rtMapObject();

  rtValue name(cookie.name().constData());
  cook->Set("name", &name);

  rtValue value(cookie.value().constData());
  cook->Set("value", &value);

  rtValue domain(cookie.domain().toLocal8Bit().constData());
  cook->Set("domain", &domain);

  return cook;
}

void pxBrowserView::onSize(int32_t w, int32_t h)
{
  mWidth = w;
  mHeight = h;
}

// events return true if the event was consumed by the view
bool pxBrowserView::onMouseDown(int32_t x, int32_t y, uint32_t flags)
{ return true; }

bool pxBrowserView::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{ return true; }

bool pxBrowserView::onMouseMove(int32_t x, int32_t y)
{ return true; }

bool pxBrowserView::onScrollWheel(float dx, float dy)
{ return false; };

bool pxBrowserView::onMouseEnter()
{ return true; }

bool pxBrowserView::onMouseLeave()
{ return true; }

bool pxBrowserView::onFocus()
{ return true; }

bool pxBrowserView::onBlur()
{ return true; }

bool pxBrowserView::onKeyDown(uint32_t keycode, uint32_t flags)
{
  return true;
}

bool pxBrowserView::onKeyUp(uint32_t keycode, uint32_t flags)
{ return true; }

bool pxBrowserView::onChar(uint32_t codepoint)
{ return true; }

void pxBrowserView::onUpdate(double t)
{
  if (globalQTApp)
  {
    globalQTApp->sendPostedEvents();
  }

  if (this->mWebView)
  {
    QPainter painter;
    painter.begin(mRenderQImage);
    mWebView->page()->view()->render(&painter, QPoint(), QRegion(0, 0, mWidth, mHeight));
    painter.end();
    mDirty = true;
  }
}

void pxBrowserView::onDraw()
{
  // here is test background color
  float bgColor[4] = {0.7, 0.7, 0.7, 1.0};
  context.drawRect(mWidth, mHeight, 0, bgColor, bgColor);

  // render mRenderQImage in spark gl
  // TODO need looking for a more efficient way to convert QImage bits to offscreen
  if(mDirty){
    const unsigned char* bits = mRenderQImage->bits();
    mOffscreen->fill(qRgba(0, 0, 0, 0));

    for (int y = 0; y < mOffscreen->height(); y++)
    {
      pxPixel* d = mOffscreen->scanline(y);
      for (int x = 0; x < mOffscreen->width(); x++)
      {
        int index = y * mOffscreen->width() + x;
        *d = qRgba(bits[index * 4], bits[index * 4 + 1], bits[index * 4 + 2], bits[index * 4 + 3]);
        d++;
      }
    }

    // TODO maybe need update texture, there is always has create and delete texture on draw method, this may cause lower fps.
    mTextureRef = context.createTexture(*mOffscreen);
    context.drawImage(0, 0, mWidth, mHeight, mTextureRef, nullptr);
    mDirty = false;
  }
}

void pxBrowserView::onCloseRequest()
{

};

rtError pxBrowserView::updateUrl(rtString const& newUrl)
{
  this->mWebView->stop();

  this->mUrl = std::string(newUrl.cString());

  QWebEngineHttpRequest request;
  request.setUrl(QUrl(this->mUrl.c_str()));
  webUrlRequestInterceptor->setHeaders(mHeaders);

  this->mWebView->load(request);
  return RT_OK;
}

rtError pxBrowserView::setVisible(bool const& visible)
{
#ifdef WIN32
  QWinWidget* win = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* win = (QMacWidget*) mRootWidget;
#endif

  win->setVisible(visible);
  return RT_OK;
}

bool pxBrowserView::isVisible() const
{
#ifdef WIN32
  QWinWidget* win = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* win = (QMacWidget*) mRootWidget;
#endif

  return win->isVisible();
}

rtError pxBrowserView::setTransparent(bool const& transparent)
{
  this->mWebView->page()->setBackgroundColor(transparent ? Qt::transparent : Qt::white);
  return RT_OK;
}

rtError pxBrowserView::getTransparentBackground(bool& v) const
{
  v = this->mWebView->page()->backgroundColor() == Qt::transparent;
  return RT_OK;
}

rtError pxBrowserView::setUserAgent(rtString const& userAgent)
{
  this->mUserAgent = std::string(userAgent.cString());
  mWebView->page()->profile()->setHttpUserAgent(QString(userAgent.cString()));
  return RT_OK;
}

rtError pxBrowserView::setLocalStorageEnabled(bool const& enabled)
{
  mWebView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, enabled);
  return RT_OK;
}

bool pxBrowserView::isLocalStorageEnabled() const
{
  return mWebView->settings()->testAttribute(QWebEngineSettings::LocalStorageEnabled);
}

rtError pxBrowserView::setConsoleLogEnabled(bool const& enabled)
{
  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  page->setConsoleLogEnabled(enabled);
  return RT_OK;
}

bool pxBrowserView::isConsoleLogEnabled() const
{
  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  return page->isConsoleLogEnabled();
}

void pxBrowserView::setHeaders(std::map<std::string, std::string> const& headers)
{
  mHeaders = headers;
}

rtError pxBrowserView::getHeaders(rtObjectRef& v) const
{

  rtMapObject* headers = new rtMapObject();
  std::map<std::string, std::string>::const_iterator headerIt;
  for (headerIt = this->mHeaders.begin(); headerIt != mHeaders.end(); ++headerIt)
  {
    rtValue v(headerIt->second.c_str());
    headers->Set(headerIt->first.c_str(), &v);
  }
  v = headers;
  return RT_OK;
}

rtError pxBrowserView::setHeaders(rtObjectRef const& headers)
{
  std::map<std::string, std::string> mapHeaders;

  rtValue allKeys;
  headers->Get("allKeys", &allKeys);
  rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();

  for (uint32_t i = 0, l = arr->length(); i < l; ++i)
  {
    rtValue key;
    if (arr->Get(i, &key) == RT_OK && !key.isEmpty())
    {
      rtString
      s = key.toString();
      rtString
      val = headers.get<rtString>(s);
      mapHeaders[std::string(s.cString())] = std::string(val.cString());
    }
  }
  setHeaders(mapHeaders);
  return RT_OK;
}

std::map<std::string, std::string> const& pxBrowserView::getHeaders()
{
  return mHeaders;
}

void pxBrowserView::dumpProperties()
{
  bool transparent;
  this->getTransparentBackground(transparent);
  char proxy_str[512] = "no proxy";

  if (this->mProxy)
  {
    rtValue type;
    rtValue hostname;
    rtValue port;

    mProxy->Get("type", &type);
    mProxy->Get("hostname", &hostname);
    mProxy->Get("port", &port);

    sprintf(proxy_str, "type = %d, %s:%d", type.toInt32(), hostname.toString().cString(), port.toInt32());
  }


  printf("pxBrowser %-25s= %s\n", "url", mUrl.c_str());

  rtObjectRef
  cookies;
  getCookieJar(cookies);
  rtValue length;
  cookies->Get("length", &length);
  printf("pxBrowser %-25s= %d\n", "cookieJar.length", length.toInt32());
  for (int i = 0; i < length.toInt32(); i++)
  {
    rtValue obj;
    cookies->Get(i, &obj);
    rtObjectRef
    c = obj.toObject();

    rtValue v;
    c->Get("name", &v);
    printf("     >cookie %d : name=%s\n", i, v.toString().cString());
    c->Get("value", &v);
    printf("     >cookie %d : value=%s\n", i, v.toString().cString());
    c->Get("domain", &v);
    printf("     >cookie %d : domain=%s\n", i, v.toString().cString());
  }
  printf("pxBrowser %-25s= %s\n", "proxy", proxy_str);
  printf("pxBrowser %-25s= %s\n", "userAgent", mUserAgent.c_str());
  printf("pxBrowser %-25s= %s\n", "transparentBackground", transparent ? "true" : "false");
  printf("pxBrowser %-25s= %s\n", "visible", isVisible() ? "true" : "false");
  printf("pxBrowser %-25s= %s\n", "localStorageEnabled", isLocalStorageEnabled() ? "true" : "false");
  printf("pxBrowser %-25s= %s\n", "consoleLogEnabled", isConsoleLogEnabled() ? "true" : "false");
  printf("pxBrowser %-25s= %lu\n", "headers.length", mHeaders.size());


  std::map<std::string, std::string>::iterator headerIt;
  for (headerIt = mHeaders.begin(); headerIt != mHeaders.end(); ++headerIt)
  {
    printf("    >pxBrowser headers: %s = %s\n", headerIt->first.c_str(), headerIt->second.c_str());
  }
}

rtError pxBrowserView::getCookieJar(rtObjectRef& v) const
{
  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  QList<QNetworkCookie> cookies = page->getCookies();

  rtArrayObject* cookArr = new rtArrayObject();
  QUrl url = QUrl(mUrl.c_str());
  QString domain = url.host();

  int index = 0;
  for (int i = 0; i < cookies.size(); ++i)
  {
    QNetworkCookie cookie = cookies.at(i);
    if (cookie.domain() == domain)
    {
      rtValue c(pxBrowserView::convertCookieToMap(cookie));
      cookArr->Set(index++, &c);
    }

  }
  v = cookArr;
  return RT_OK;
}

rtError pxBrowserView::setCookieJar(rtObjectRef const& v)
{

  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  rtArrayObject* arr = static_cast<rtArrayObject*>(v.getPtr());
  QWebEngineCookieStore* store = page->profile()->cookieStore();

  for (uint32_t i = 0, l = arr->length(); i < l; ++i)
  {
    rtValue cookObj;
    arr->Get(i, &cookObj);

    rtValue v;
    QNetworkCookie cook;
    cookObj.toObject()->Get("name", &v);
    cook.setName(v.toString().cString());

    cookObj.toObject()->Get("value", &v);
    cook.setValue(v.toString().cString());

    cookObj.toObject()->Get("domain", &v);
    cook.setDomain(v.toString().cString());

    store->setCookie(cook);
  }

  return RT_OK;
}

rtError pxBrowserView::setProxy(rtObjectRef const& proxy)
{
  mProxy = proxy;

  QNetworkProxy qp;
  rtString
  hostname;
  int port;
  int type;

  pxBrowserProxy * p = dynamic_cast<pxBrowserProxy*>(proxy.getPtr());
  p->getHostname(hostname);
  p->getType(type);
  p->getPort(port);

  qp.setType(QNetworkProxy::ProxyType(type));
  qp.setHostName(hostname.cString());
  qp.setPort(port);

  QNetworkProxy::setApplicationProxy(qp);

  return RT_OK;
}

rtError pxBrowserView::addListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->addListener(eventName, f);
  return RT_OK;
}
