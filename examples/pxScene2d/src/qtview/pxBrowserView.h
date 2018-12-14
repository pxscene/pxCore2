#ifndef __pxBrowserView_H__
#define __pxBrowserView_H__


/**
 * pxBrowser View class
 * this class use QT web engine render web page
 */


#include <string>
#include <map>

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QWebengineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineCookieStore>
#include <QNetworkCookieJar>
#include <QWebEngineUrlRequestInterceptor>
#include <QNetworkProxy>

#include "rtObject.h"

#include "pxIView.h"

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
		, mConsoleLogEnabled(true)
		, mLoadedFunc(nullptr)
		, mLinkClickedFunc(nullptr)
		, mConsoleFunc(nullptr)
		, mIsFirstLoad(true)
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
	HTMLDocumentLoadedFunc mLoadedFunc;
	ConsoleLogFunc mConsoleFunc;
	HTMLLinkClickedFunc mLinkClickedFunc;
	CookieJarChangedFunc mCookieChangedFunc;
	QList<QNetworkCookie> mCookies;
};

/**
 * pxBrowserView class
 */
class pxBrowserView : public pxIView, public rtObject
{
public:

	rtDeclareObject(pxBrowserView, rtObject);
	rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
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
  virtual void RT_STDCALL setViewContainer(pxIViewContainer* l) {}

	pxBrowserView(void *rootWidget, int w = 0, int h = 0);

	void init();

	virtual void  onSize(int32_t w, int32_t h);

	// events return true if the event was consumed by the view
	virtual bool  onMouseDown(int32_t x, int32_t y, uint32_t flags);
	virtual bool  onMouseUp(int32_t x, int32_t y, uint32_t flags);
	virtual bool  onMouseMove(int32_t x, int32_t y);

	virtual bool  onScrollWheel(float dx, float dy);

	virtual bool  onMouseEnter();
	virtual bool  onMouseLeave();

	virtual bool  onFocus();
	virtual bool  onBlur();

	virtual bool  onKeyDown(uint32_t keycode, uint32_t flags);
	virtual bool  onKeyUp(uint32_t keycode, uint32_t flags);
	virtual bool  onChar(uint32_t codepoint);

	virtual void  onUpdate(double t);
	virtual void  onDraw();

	virtual void  onCloseRequest();

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
protected:
	int mWidth;
	int mHeight;

	rtEmitRef mEmit;

	std::string mUrl;
	std::map<std::string, std::string> mHeaders;

	std::string mUserAgent;

	QWebEngineView *mWebView;
	QNetworkAccessManager* mNetworkManager;
	pxWebUrlRequestInterceptor* webUrlRequestInterceptor;
	rtObjectRef mProxy;
	void* mRootWidget;
};

#endif
