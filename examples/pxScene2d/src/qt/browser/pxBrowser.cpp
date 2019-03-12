#include "pxBrowser.h"


#include <QImage>
#include <QPainter>
#include <QRgb>
#include "pxContext.h"


#ifdef WIN32
#include "qt/browser/win32/qtwinmigrate/qwinwidget.h"
#elif __APPLE__
#include "qt/browser/mac/qmacwidget.h"
#endif

#include "QAdapter.h"

extern QAdapter qAdapter;
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

//pxBrowser properties
rtDefineObject(pxBrowser, pxObject);
rtDefineProperty(pxBrowser, url);
rtDefineProperty(pxBrowser, cookieJar);
rtDefineProperty(pxBrowser, proxy);
rtDefineProperty(pxBrowser, userAgent);
rtDefineProperty(pxBrowser, transparentBackground);
rtDefineProperty(pxBrowser, visible);
rtDefineProperty(pxBrowser, localStorageEnabled);
rtDefineProperty(pxBrowser, consoleLogEnabled);
rtDefineProperty(pxBrowser, headers);
// pxBrowser function



pxBrowser::pxBrowser(pxScene2d* scene) : pxObject(scene), mNetworkManager(nullptr),
    webUrlRequestInterceptor(nullptr), mProxy(nullptr), mRenderQImage(nullptr)
    , mOffscreen(nullptr), mTextureRef(nullptr)
{

#ifdef WIN32
  HWND* hwnd = (HWND*)root;
  this->mRootWidget = new QWinWidget(*hwnd);
#elif __APPLE__
  this->mRootWidget = qAdapter.getRootWidget();
#endif

  mHeaders.clear();
  mUrl.empty();
  mUserAgent.empty();

  rtLogInfo("new pxBrowser created ..");
  initQT();
  rtLogInfo("initQT qt done");
  
  mEmit->addListener("onMouseDown", new rtFunctionCallback(pxBrowser::onMouseDown,this));
  mEmit->addListener("onMouseMove", new rtFunctionCallback(pxBrowser::onMouseMove,this));
  mEmit->addListener("onMouseUp", new rtFunctionCallback(pxBrowser::onMouseUp,this));
}

void pxBrowser::updateSize()
{
  this->mWebView->setGeometry(0, 0, mw, mh);
//  this->mWebView->setEnabled(false);
  if (mRenderQImage) delete mRenderQImage;
  mRenderQImage = new QImage((int)mw, (int)mh, QImage::Format_ARGB32);
  if (mOffscreen) delete mOffscreen;
  mOffscreen = new pxOffscreen();
  mOffscreen->init((int)mw, (int)mh);
  mRenderQImage->fill(qRgba(255, 0, 0, 0));
  rtLogInfo("resize RenderQImage and Offscreen w = %f, h = %f", mw, mh);
}

void pxBrowser::initQT()
{

  mNetworkManager = new QNetworkAccessManager();
  webUrlRequestInterceptor = new pxWebUrlRequestInterceptor();


#ifdef WIN32
  QWinWidget* win = (QWinWidget*) mRootWidget;
  ::SetFocus((HWND) win->winId());
#elif __APPLE__
  QMacWidget* win = (QMacWidget*) mRootWidget;
#endif

  // create new QWebEngineView and attach to window
  this->mWebView = new pxQTWebView(win);
  pxQTWebPage* wPage = new pxQTWebPage();

  this->mWebView->setPage(wPage);
  this->mWebView->page()->profile()->setRequestInterceptor(webUrlRequestInterceptor);
  this->mWebView->show();

  this->mWebView->setGeometry(0, 0, mw, mh);

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
                                   rtObjectRef ref = pxBrowser::convertCookieToMap(cookie);
                                   this->mEmit.send("CookieJarChanged", ref);
                                 });

  setVisible(true);
}

rtMapObject* pxBrowser::convertCookieToMap(QNetworkCookie const& cookie)
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

void pxBrowser::update(double /*t*/)
{
  if (this->mWebView && mw > 0 && mh > 0)
  {
    QPainter painter;
    painter.begin(mRenderQImage);
    mWebView->page()->view()->render(&painter, QPoint(), QRegion(0, 0, mw, mh));
    painter.end();
    mScene->mDirty = true;
    markDirty();
  }
}
void pxBrowser::draw()
{

  // here is test background color
  float bgColor[4] = {0.7, 0.7, 0.7, 1.0};
  context.drawRect(mw, mh, 0, bgColor, bgColor);

  // render mRenderQImage in spark gl
  // TODO need looking for a more efficient way to convert QImage bits to offscreen

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
  context.drawImage(0, 0, mw, mh, mTextureRef, nullptr);
}

rtError pxBrowser::updateUrl(rtString const& newUrl)
{
  rtLogInfo("updateUrl url = %s", newUrl.cString());
  this->mWebView->stop();

  this->mUrl = std::string(newUrl.cString());

  QWebEngineHttpRequest request;
  request.setUrl(QUrl(this->mUrl.c_str()));
  webUrlRequestInterceptor->setHeaders(mHeaders);

  this->mWebView->load(request);
  return RT_OK;
}

rtError pxBrowser::setVisible(bool const& visible)
{
#ifdef WIN32
  QWinWidget* win = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* win = (QMacWidget*) mRootWidget;
#endif

  win->setVisible(visible);
  return RT_OK;
}

bool pxBrowser::isVisible() const
{
#ifdef WIN32
  QWinWidget* win = (QWinWidget*) mRootWidget;
#elif __APPLE__
  QMacWidget* win = (QMacWidget*) mRootWidget;
#endif

  return win->isVisible();
}

rtError pxBrowser::setTransparent(bool const& transparent)
{
  this->mWebView->page()->setBackgroundColor(transparent ? Qt::transparent : Qt::white);
  return RT_OK;
}

rtError pxBrowser::getTransparentBackground(bool& v) const
{
  v = this->mWebView->page()->backgroundColor() == Qt::transparent;
  return RT_OK;
}

rtError pxBrowser::setUserAgent(rtString const& userAgent)
{
  this->mUserAgent = std::string(userAgent.cString());
  mWebView->page()->profile()->setHttpUserAgent(QString(userAgent.cString()));
  return RT_OK;
}

rtError pxBrowser::setLocalStorageEnabled(bool const& enabled)
{
  mWebView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, enabled);
  return RT_OK;
}

bool pxBrowser::isLocalStorageEnabled() const
{
  return mWebView->settings()->testAttribute(QWebEngineSettings::LocalStorageEnabled);
}

rtError pxBrowser::setConsoleLogEnabled(bool const& enabled)
{
  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  page->setConsoleLogEnabled(enabled);
  return RT_OK;
}

bool pxBrowser::isConsoleLogEnabled() const
{
  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  return page->isConsoleLogEnabled();
}

void pxBrowser::setHeaders(std::map<std::string, std::string> const& headers)
{
  mHeaders = headers;
}

rtError pxBrowser::getHeaders(rtObjectRef& v) const
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

rtError pxBrowser::setHeaders(rtObjectRef const& headers)
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
      rtString s = key.toString();
      rtString val = headers.get<rtString>(s);
      mapHeaders[std::string(s.cString())] = std::string(val.cString());
    }
  }
  setHeaders(mapHeaders);
  return RT_OK;
}

std::map<std::string, std::string> const& pxBrowser::getHeaders()
{
  return mHeaders;
}

void pxBrowser::dumpProperties()
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

rtError pxBrowser::getCookieJar(rtObjectRef& v) const
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
      rtValue c(pxBrowser::convertCookieToMap(cookie));
      cookArr->Set(index++, &c);
    }

  }
  v = cookArr;
  return RT_OK;
}

rtError pxBrowser::setCookieJar(rtObjectRef const& v)
{
  rtValue rtLength;
  int32_t length;
  v->Get("length", &rtLength);
  rtLength.getInt32(length);
 
  pxQTWebPage* page = (pxQTWebPage*) mWebView->page();
  QWebEngineCookieStore* store = page->profile()->cookieStore();
  
  for (uint32_t i = 0, l = length; i < l; ++i)
  {
    rtValue cookObj;
    v->Get(i, &cookObj);

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

rtError pxBrowser::setProxy(rtObjectRef const& proxy)
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
