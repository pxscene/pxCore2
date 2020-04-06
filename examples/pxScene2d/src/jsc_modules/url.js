/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

'use strict';

var toASCII = require('punycode').toASCII;

exports.parse = urlParse;
exports.resolve = urlResolve;
exports.resolveObject = urlResolveObject;
exports.format = urlFormat;

exports.Url = Url;

function Url() {
  this.protocol = null;
  this.slashes = null;
  this.auth = null;
  this.host = null;
  this.port = null;
  this.hostname = null;
  this.hash = null;
  this.search = null;
  this.query = null;
  this.pathname = null;
  this.path = null;
  this.href = null;
}

// Reference: RFC 3986, RFC 1808, RFC 2396

// define these here so at least they only have to be
// compiled once on the first module load.
const protocolPattern = /^([a-z0-9.+-]+:)/i;
const portPattern = /:[0-9]*$/;

// Special case for a simple path URL
const simplePathPattern = /^(\/\/?(?!\/)[^\?\s]*)(\?[^\s]*)?$/;

const hostnameMaxLen = 255;
// protocols that can allow "unsafe" and "unwise" chars.
const unsafeProtocol = {
  'javascript': true,
  'javascript:': true
};
// protocols that never have a hostname.
const hostlessProtocol = {
  'javascript': true,
  'javascript:': true
};
// protocols that always contain a // bit.
const slashedProtocol = {
  'http': true,
  'http:': true,
  'https': true,
  'https:': true,
  'ftp': true,
  'ftp:': true,
  'gopher': true,
  'gopher:': true,
  'file': true,
  'file:': true
};
const querystring = require('querystring.js');

// This constructor is used to store parsed query string values. Instantiating
// this is faster than explicitly calling `Object.create(null)` to get a
// "clean" empty object (tested with v8 v4.9).
function ParsedQueryString() {}
ParsedQueryString.prototype = Object.create(null);

function urlParse(url, parseQueryString, slashesDenoteHost) {
  if (url instanceof Url) return url;

  var u = new Url();
  u.parse(url, parseQueryString, slashesDenoteHost);
  return u;
}

Url.prototype.parse = function(url, parseQueryString, slashesDenoteHost) {
  if (typeof url !== 'string') {
    throw new TypeError('Parameter "url" must be a string, not ' + typeof url);
  }

  // Copy chrome, IE, opera backslash-handling behavior.
  // Back slashes before the query string get converted to forward slashes
  // See: https://code.google.com/p/chromium/issues/detail?id=25916
  var hasHash = false;
  var start = -1;
  var end = -1;
  var rest = '';
  var lastPos = 0;
  var i = 0;
  for (var inWs = false, split = false; i < url.length; ++i) {
    const code = url.charCodeAt(i);

    // Find first and last non-whitespace characters for trimming
    const isWs = code === 32/* */ ||
                 code === 9/*\t*/ ||
                 code === 13/*\r*/ ||
                 code === 10/*\n*/ ||
                 code === 12/*\f*/ ||
                 code === 160/*\u00A0*/ ||
                 code === 65279/*\uFEFF*/;
    if (start === -1) {
      if (isWs)
        continue;
      lastPos = start = i;
    } else {
      if (inWs) {
        if (!isWs) {
          end = -1;
          inWs = false;
        }
      } else if (isWs) {
        end = i;
        inWs = true;
      }
    }

    // Only convert backslashes while we haven't seen a split character
    if (!split) {
      switch (code) {
        case 35: // '#'
          hasHash = true;
        // Fall through
        case 63: // '?'
          split = true;
          break;
        case 92: // '\\'
          if (i - lastPos > 0)
            rest += url.slice(lastPos, i);
          rest += '/';
          lastPos = i + 1;
          break;
      }
    } else if (!hasHash && code === 35/*#*/) {
      hasHash = true;
    }
  }

  // Check if string was non-empty (including strings with only whitespace)
  if (start !== -1) {
    if (lastPos === start) {
      // We didn't convert any backslashes

      if (end === -1) {
        if (start === 0)
          rest = url;
        else
          rest = url.slice(start);
      } else {
        rest = url.slice(start, end);
      }
    } else if (end === -1 && lastPos < url.length) {
      // We converted some backslashes and have only part of the entire string
      rest += url.slice(lastPos);
    } else if (end !== -1 && lastPos < end) {
      // We converted some backslashes and have only part of the entire string
      rest += url.slice(lastPos, end);
    }
  }

  if (!slashesDenoteHost && !hasHash) {
    // Try fast path regexp
    const simplePath = simplePathPattern.exec(rest);
    if (simplePath) {
      this.path = rest;
      this.href = rest;
      this.pathname = simplePath[1];
      if (simplePath[2]) {
        this.search = simplePath[2];
        if (parseQueryString) {
          this.query = querystring.parse(this.search.slice(1));
        } else {
          this.query = this.search.slice(1);
        }
      } else if (parseQueryString) {
        this.search = '';
        this.query = new ParsedQueryString();
      }
      return this;
    }
  }

  var proto = protocolPattern.exec(rest);
  if (proto) {
    proto = proto[0];
    var lowerProto = proto.toLowerCase();
    this.protocol = lowerProto;
    rest = rest.slice(proto.length);
  }

  // figure out if it's got a host
  // user@server is *always* interpreted as a hostname, and url
  // resolution will treat //foo/bar as host=foo,path=bar because that's
  // how the browser resolves relative URLs.
  if (slashesDenoteHost || proto || /^\/\/[^@\/]+@[^@\/]+/.test(rest)) {
    var slashes = rest.charCodeAt(0) === 47/*/*/ &&
                  rest.charCodeAt(1) === 47/*/*/;
    if (slashes && !(proto && hostlessProtocol[proto])) {
      rest = rest.slice(2);
      this.slashes = true;
    }
  }

  if (!hostlessProtocol[proto] &&
      (slashes || (proto && !slashedProtocol[proto]))) {

    // there's a hostname.
    // the first instance of /, ?, ;, or # ends the host.
    //
    // If there is an @ in the hostname, then non-host chars *are* allowed
    // to the left of the last @ sign, unless some host-ending character
    // comes *before* the @-sign.
    // URLs are obnoxious.
    //
    // ex:
    // http://a@b@c/ => user:a@b host:c
    // http://a@b?@c => user:a host:b path:/?@c

    // v0.12 TODO(isaacs): This is not quite how Chrome does things.
    // Review our test case against browsers more comprehensively.

    var hostEnd = -1;
    var atSign = -1;
    var nonHost = -1;
    for (i = 0; i < rest.length; ++i) {
      switch (rest.charCodeAt(i)) {
        case 9:   // '\t'
        case 10:  // '\n'
        case 13:  // '\r'
        case 32:  // ' '
        case 34:  // '"'
        case 37:  // '%'
        case 39:  // '\''
        case 59:  // ';'
        case 60:  // '<'
        case 62:  // '>'
        case 92:  // '\\'
        case 94:  // '^'
        case 96:  // '`'
        case 123: // '{'
        case 124: // '|'
        case 125: // '}'
          // Characters that are never ever allowed in a hostname from RFC 2396
          if (nonHost === -1)
            nonHost = i;
          break;
        case 35: // '#'
        case 47: // '/'
        case 63: // '?'
          // Find the first instance of any host-ending characters
          if (nonHost === -1)
            nonHost = i;
          hostEnd = i;
          break;
        case 64: // '@'
          // At this point, either we have an explicit point where the
          // auth portion cannot go past, or the last @ char is the decider.
          atSign = i;
          nonHost = -1;
          break;
      }
      if (hostEnd !== -1)
        break;
    }
    start = 0;
    if (atSign !== -1) {
      this.auth = decodeURIComponent(rest.slice(0, atSign));
      start = atSign + 1;
    }
    if (nonHost === -1) {
      this.host = rest.slice(start);
      rest = '';
    } else {
      this.host = rest.slice(start, nonHost);
      rest = rest.slice(nonHost);
    }

    // pull out port.
    this.parseHost();

    // we've indicated that there is a hostname,
    // so even if it's empty, it has to be present.
    if (typeof this.hostname !== 'string')
      this.hostname = '';

    var hostname = this.hostname;

    // if hostname begins with [ and ends with ]
    // assume that it's an IPv6 address.
    var ipv6Hostname = hostname.charCodeAt(0) === 91/*[*/ &&
                       hostname.charCodeAt(hostname.length - 1) === 93/*]*/;

    // validate a little.
    if (!ipv6Hostname) {
      const result = validateHostname(this, rest, hostname);
      if (result !== undefined)
        rest = result;
    }

    if (this.hostname.length > hostnameMaxLen) {
      this.hostname = '';
    } else {
      // hostnames are always lower case.
      this.hostname = this.hostname.toLowerCase();
    }

    if (!ipv6Hostname) {
      // IDNA Support: Returns a punycoded representation of "domain".
      // It only converts parts of the domain name that
      // have non-ASCII characters, i.e. it doesn't matter if
      // you call it with a domain that already is ASCII-only.
      this.hostname = toASCII(this.hostname);
    }

    var p = this.port ? ':' + this.port : '';
    var h = this.hostname || '';
    this.host = h + p;

    // strip [ and ] from the hostname
    // the host field still retains them, though
    if (ipv6Hostname) {
      this.hostname = this.hostname.slice(1, -1);
      if (rest[0] !== '/') {
        rest = '/' + rest;
      }
    }
  }

  // now rest is set to the post-host stuff.
  // chop off any delim chars.
  if (!unsafeProtocol[lowerProto]) {
    // First, make 100% sure that any "autoEscape" chars get
    // escaped, even if encodeURIComponent doesn't think they
    // need to be.
    const result = autoEscapeStr(rest);
    if (result !== undefined)
      rest = result;
  }

  var questionIdx = -1;
  var hashIdx = -1;
  for (i = 0; i < rest.length; ++i) {
    const code = rest.charCodeAt(i);
    if (code === 35/*#*/) {
      this.hash = rest.slice(i);
      hashIdx = i;
      break;
    } else if (code === 63/*?*/ && questionIdx === -1) {
      questionIdx = i;
    }
  }

  if (questionIdx !== -1) {
    if (hashIdx === -1) {
      this.search = rest.slice(questionIdx);
      this.query = rest.slice(questionIdx + 1);
    } else {
      this.search = rest.slice(questionIdx, hashIdx);
      this.query = rest.slice(questionIdx + 1, hashIdx);
    }
    if (parseQueryString) {
      this.query = querystring.parse(this.query);
    }
  } else if (parseQueryString) {
    // no query string, but parseQueryString still requested
    this.search = '';
    this.query = new ParsedQueryString();
  }

  var firstIdx = (questionIdx !== -1 &&
                  (hashIdx === -1 || questionIdx < hashIdx)
                  ? questionIdx
                  : hashIdx);
  if (firstIdx === -1) {
    if (rest.length > 0)
      this.pathname = rest;
  } else if (firstIdx > 0) {
    this.pathname = rest.slice(0, firstIdx);
  }
  if (slashedProtocol[lowerProto] &&
      this.hostname && !this.pathname) {
    this.pathname = '/';
  }

  // to support http.request
  if (this.pathname || this.search) {
    const p = this.pathname || '';
    const s = this.search || '';
    this.path = p + s;
  }

  // finally, reconstruct the href based on what has been validated.
  this.href = this.format();
  return this;
};

function validateHostname(self, rest, hostname) {
  for (var i = 0, lastPos; i <= hostname.length; ++i) {
    var code;
    if (i < hostname.length)
      code = hostname.charCodeAt(i);
    if (code === 46/*.*/ || i === hostname.length) {
      if (i - lastPos > 0) {
        if (i - lastPos > 63) {
          self.hostname = hostname.slice(0, lastPos + 63);
          return '/' + hostname.slice(lastPos + 63) + rest;
        }
      }
      lastPos = i + 1;
      continue;
    } else if ((code >= 48/*0*/ && code <= 57/*9*/) ||
               (code >= 97/*a*/ && code <= 122/*z*/) ||
               code === 45/*-*/ ||
               (code >= 65/*A*/ && code <= 90/*Z*/) ||
               code === 43/*+*/ ||
               code === 95/*_*/ ||
               code > 127) {
      continue;
    }
    // Invalid host character
    self.hostname = hostname.slice(0, i);
    if (i < hostname.length)
      return '/' + hostname.slice(i) + rest;
    break;
  }
}

function autoEscapeStr(rest) {
  var newRest = '';
  var lastPos = 0;
  for (var i = 0; i < rest.length; ++i) {
    // Automatically escape all delimiters and unwise characters from RFC 2396
    // Also escape single quotes in case of an XSS attack
    switch (rest.charCodeAt(i)) {
      case 9:   // '\t'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%09';
        lastPos = i + 1;
        break;
      case 10:  // '\n'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%0A';
        lastPos = i + 1;
        break;
      case 13:  // '\r'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%0D';
        lastPos = i + 1;
        break;
      case 32:  // ' '
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%20';
        lastPos = i + 1;
        break;
      case 34:  // '"'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%22';
        lastPos = i + 1;
        break;
      case 39:  // '\''
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%27';
        lastPos = i + 1;
        break;
      case 60:  // '<'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%3C';
        lastPos = i + 1;
        break;
      case 62:  // '>'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%3E';
        lastPos = i + 1;
        break;
      case 92:  // '\\'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%5C';
        lastPos = i + 1;
        break;
      case 94:  // '^'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%5E';
        lastPos = i + 1;
        break;
      case 96:  // '`'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%60';
        lastPos = i + 1;
        break;
      case 123: // '{'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%7B';
        lastPos = i + 1;
        break;
      case 124: // '|'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%7C';
        lastPos = i + 1;
        break;
      case 125: // '}'
        if (i - lastPos > 0)
          newRest += rest.slice(lastPos, i);
        newRest += '%7D';
        lastPos = i + 1;
        break;
    }
  }
  if (lastPos === 0)
    return;
  if (lastPos < rest.length)
    return newRest + rest.slice(lastPos);
  else
    return newRest;
}

// format a parsed object into a url string
function urlFormat(obj) {
  // ensure it's an object, and not a string url.
  // If it's an obj, this is a no-op.
  // this way, you can call url_format() on strings
  // to clean up potentially wonky urls.
  if (typeof obj === 'string') obj = urlParse(obj);

  else if (typeof obj !== 'object' || obj === null)
    throw new TypeError('Parameter "urlObj" must be an object, not ' +
                        obj === null ? 'null' : typeof obj);

  else if (!(obj instanceof Url)) return Url.prototype.format.call(obj);

  return obj.format();
}

Url.prototype.format = function() {
  var auth = this.auth || '';
  if (auth) {
    auth = encodeAuth(auth);
    auth += '@';
  }

  var protocol = this.protocol || '';
  var pathname = this.pathname || '';
  var hash = this.hash || '';
  var host = false;
  var query = '';

  if (this.host) {
    host = auth + this.host;
  } else if (this.hostname) {
    host = auth + (this.hostname.indexOf(':') === -1 ?
        this.hostname :
        '[' + this.hostname + ']');
    if (this.port) {
      host += ':' + this.port;
    }
  }

  if (this.query !== null && typeof this.query === 'object')
    query = querystring.stringify(this.query);

  var search = this.search || (query && ('?' + query)) || '';

  if (protocol && protocol.charCodeAt(protocol.length - 1) !== 58/*:*/)
    protocol += ':';

  var newPathname = '';
  var lastPos = 0;
  for (var i = 0; i < pathname.length; ++i) {
    switch (pathname.charCodeAt(i)) {
      case 35: // '#'
        if (i - lastPos > 0)
          newPathname += pathname.slice(lastPos, i);
        newPathname += '%23';
        lastPos = i + 1;
        break;
      case 63: // '?'
        if (i - lastPos > 0)
          newPathname += pathname.slice(lastPos, i);
        newPathname += '%3F';
        lastPos = i + 1;
        break;
    }
  }
  if (lastPos > 0) {
    if (lastPos !== pathname.length)
      pathname = newPathname + pathname.slice(lastPos);
    else
      pathname = newPathname;
  }

  // only the slashedProtocols get the //.  Not mailto:, xmpp:, etc.
  // unless they had them to begin with.
  if (this.slashes ||
      (!protocol || slashedProtocol[protocol]) && host !== false) {
    host = '//' + (host || '');
    if (pathname && pathname.charCodeAt(0) !== 47/*/*/)
      pathname = '/' + pathname;
  } else if (!host) {
    host = '';
  }

  search = search.replace(/#/g, '%23');

  if (hash && hash.charCodeAt(0) !== 35/*#*/) hash = '#' + hash;
  if (search && search.charCodeAt(0) !== 63/*?*/) search = '?' + search;

  return protocol + host + pathname + search + hash;
};

function urlResolve(source, relative) {
  return urlParse(source, false, true).resolve(relative);
}

Url.prototype.resolve = function(relative) {
  return this.resolveObject(urlParse(relative, false, true)).format();
};

function urlResolveObject(source, relative) {
  if (!source) return relative;
  return urlParse(source, false, true).resolveObject(relative);
}

Url.prototype.resolveObject = function(relative) {
  if (typeof relative === 'string') {
    var rel = new Url();
    rel.parse(relative, false, true);
    relative = rel;
  }

  var result = new Url();
  var tkeys = Object.keys(this);
  for (var tk = 0; tk < tkeys.length; tk++) {
    var tkey = tkeys[tk];
    result[tkey] = this[tkey];
  }

  // hash is always overridden, no matter what.
  // even href="" will remove it.
  result.hash = relative.hash;

  // if the relative url is empty, then there's nothing left to do here.
  if (relative.href === '') {
    result.href = result.format();
    return result;
  }

  // hrefs like //foo/bar always cut to the protocol.
  if (relative.slashes && !relative.protocol) {
    // take everything except the protocol from relative
    var rkeys = Object.keys(relative);
    for (var rk = 0; rk < rkeys.length; rk++) {
      var rkey = rkeys[rk];
      if (rkey !== 'protocol')
        result[rkey] = relative[rkey];
    }

    //urlParse appends trailing / to urls like http://www.example.com
    if (slashedProtocol[result.protocol] &&
        result.hostname && !result.pathname) {
      result.path = result.pathname = '/';
    }

    result.href = result.format();
    return result;
  }

  if (relative.protocol && relative.protocol !== result.protocol) {
    // if it's a known url protocol, then changing
    // the protocol does weird things
    // first, if it's not file:, then we MUST have a host,
    // and if there was a path
    // to begin with, then we MUST have a path.
    // if it is file:, then the host is dropped,
    // because that's known to be hostless.
    // anything else is assumed to be absolute.
    if (!slashedProtocol[relative.protocol]) {
      var keys = Object.keys(relative);
      for (var v = 0; v < keys.length; v++) {
        var k = keys[v];
        result[k] = relative[k];
      }
      result.href = result.format();
      return result;
    }

    result.protocol = relative.protocol;
    if (!relative.host &&
        !/^file:?$/.test(relative.protocol) &&
        !hostlessProtocol[relative.protocol]) {
      const relPath = (relative.pathname || '').split('/');
      while (relPath.length && !(relative.host = relPath.shift()));
      if (!relative.host) relative.host = '';
      if (!relative.hostname) relative.hostname = '';
      if (relPath[0] !== '') relPath.unshift('');
      if (relPath.length < 2) relPath.unshift('');
      result.pathname = relPath.join('/');
    } else {
      result.pathname = relative.pathname;
    }
    result.search = relative.search;
    result.query = relative.query;
    result.host = relative.host || '';
    result.auth = relative.auth;
    result.hostname = relative.hostname || relative.host;
    result.port = relative.port;
    // to support http.request
    if (result.pathname || result.search) {
      var p = result.pathname || '';
      var s = result.search || '';
      result.path = p + s;
    }
    result.slashes = result.slashes || relative.slashes;
    result.href = result.format();
    return result;
  }

  var isSourceAbs = (result.pathname && result.pathname.charAt(0) === '/');
  var isRelAbs = (
      relative.host ||
      relative.pathname && relative.pathname.charAt(0) === '/'
  );
  var mustEndAbs = (isRelAbs || isSourceAbs ||
                    (result.host && relative.pathname));
  var removeAllDots = mustEndAbs;
  var srcPath = result.pathname && result.pathname.split('/') || [];
  var relPath = relative.pathname && relative.pathname.split('/') || [];
  var psychotic = result.protocol && !slashedProtocol[result.protocol];

  // if the url is a non-slashed url, then relative
  // links like ../.. should be able
  // to crawl up to the hostname, as well.  This is strange.
  // result.protocol has already been set by now.
  // Later on, put the first path part into the host field.
  if (psychotic) {
    result.hostname = '';
    result.port = null;
    if (result.host) {
      if (srcPath[0] === '') srcPath[0] = result.host;
      else srcPath.unshift(result.host);
    }
    result.host = '';
    if (relative.protocol) {
      relative.hostname = null;
      relative.port = null;
      result.auth = null;
      if (relative.host) {
        if (relPath[0] === '') relPath[0] = relative.host;
        else relPath.unshift(relative.host);
      }
      relative.host = null;
    }
    mustEndAbs = mustEndAbs && (relPath[0] === '' || srcPath[0] === '');
  }

  if (isRelAbs) {
    // it's absolute.
    if (relative.host || relative.host === '') {
      if (result.host !== relative.host) result.auth = null;
      result.host = relative.host;
      result.port = relative.port;
    }
    if (relative.hostname || relative.hostname === '') {
      if (result.hostname !== relative.hostname) result.auth = null;
      result.hostname = relative.hostname;
    }
    result.search = relative.search;
    result.query = relative.query;
    srcPath = relPath;
    // fall through to the dot-handling below.
  } else if (relPath.length) {
    // it's relative
    // throw away the existing file, and take the new path instead.
    if (!srcPath) srcPath = [];
    srcPath.pop();
    srcPath = srcPath.concat(relPath);
    result.search = relative.search;
    result.query = relative.query;
  } else if (relative.search !== null && relative.search !== undefined) {
    // just pull out the search.
    // like href='?foo'.
    // Put this after the other two cases because it simplifies the booleans
    if (psychotic) {
      result.hostname = result.host = srcPath.shift();
      //occasionally the auth can get stuck only in host
      //this especially happens in cases like
      //url.resolveObject('mailto:local1@domain1', 'local2@domain2')
      const authInHost = result.host && result.host.indexOf('@') > 0 ?
                       result.host.split('@') : false;
      if (authInHost) {
        result.auth = authInHost.shift();
        result.host = result.hostname = authInHost.shift();
      }
    }
    result.search = relative.search;
    result.query = relative.query;
    //to support http.request
    if (result.pathname !== null || result.search !== null) {
      result.path = (result.pathname ? result.pathname : '') +
                    (result.search ? result.search : '');
    }
    result.href = result.format();
    return result;
  }

  if (!srcPath.length) {
    // no path at all.  easy.
    // we've already handled the other stuff above.
    result.pathname = null;
    //to support http.request
    if (result.search) {
      result.path = '/' + result.search;
    } else {
      result.path = null;
    }
    result.href = result.format();
    return result;
  }

  // if a url ENDs in . or .., then it must get a trailing slash.
  // however, if it ends in anything else non-slashy,
  // then it must NOT get a trailing slash.
  var last = srcPath.slice(-1)[0];
  var hasTrailingSlash = (
      (result.host || relative.host || srcPath.length > 1) &&
      (last === '.' || last === '..') || last === '');

  // strip single dots, resolve double dots to parent dir
  // if the path tries to go above the root, `up` ends up > 0
  var up = 0;
  for (var i = srcPath.length - 1; i >= 0; i--) {
    last = srcPath[i];
    if (last === '.') {
      spliceOne(srcPath, i);
    } else if (last === '..') {
      spliceOne(srcPath, i);
      up++;
    } else if (up) {
      spliceOne(srcPath, i);
      up--;
    }
  }

  // if the path is allowed to go above the root, restore leading ..s
  if (!mustEndAbs && !removeAllDots) {
    for (; up--; up) {
      srcPath.unshift('..');
    }
  }

  if (mustEndAbs && srcPath[0] !== '' &&
      (!srcPath[0] || srcPath[0].charAt(0) !== '/')) {
    srcPath.unshift('');
  }

  if (hasTrailingSlash && (srcPath.join('/').substr(-1) !== '/')) {
    srcPath.push('');
  }

  var isAbsolute = srcPath[0] === '' ||
      (srcPath[0] && srcPath[0].charAt(0) === '/');

  // put the host back
  if (psychotic) {
    result.hostname = result.host = isAbsolute ? '' :
                                    srcPath.length ? srcPath.shift() : '';
    //occasionally the auth can get stuck only in host
    //this especially happens in cases like
    //url.resolveObject('mailto:local1@domain1', 'local2@domain2')
    const authInHost = result.host && result.host.indexOf('@') > 0 ?
                     result.host.split('@') : false;
    if (authInHost) {
      result.auth = authInHost.shift();
      result.host = result.hostname = authInHost.shift();
    }
  }

  mustEndAbs = mustEndAbs || (result.host && srcPath.length);

  if (mustEndAbs && !isAbsolute) {
    srcPath.unshift('');
  }

  if (!srcPath.length) {
    result.pathname = null;
    result.path = null;
  } else {
    result.pathname = srcPath.join('/');
  }

  //to support request.http
  if (result.pathname !== null || result.search !== null) {
    result.path = (result.pathname ? result.pathname : '') +
                  (result.search ? result.search : '');
  }
  result.auth = relative.auth || result.auth;
  result.slashes = result.slashes || relative.slashes;
  result.href = result.format();
  return result;
};

Url.prototype.parseHost = function() {
  var host = this.host;
  var port = portPattern.exec(host);
  if (port) {
    port = port[0];
    if (port !== ':') {
      this.port = port.slice(1);
    }
    host = host.slice(0, host.length - port.length);
  }
  if (host) this.hostname = host;
};

// About 1.5x faster than the two-arg version of Array#splice().
function spliceOne(list, index) {
  for (var i = index, k = i + 1, n = list.length; k < n; i += 1, k += 1)
    list[i] = list[k];
  list.pop();
}

var hexTable = new Array(256);
for (var i = 0; i < 256; ++i)
  hexTable[i] = '%' + ((i < 16 ? '0' : '') + i.toString(16)).toUpperCase();
function encodeAuth(str) {
  // faster encodeURIComponent alternative for encoding auth uri components
  var out = '';
  var lastPos = 0;
  for (var i = 0; i < str.length; ++i) {
    var c = str.charCodeAt(i);

    // These characters do not need escaping:
    // ! - . _ ~
    // ' ( ) * :
    // digits
    // alpha (uppercase)
    // alpha (lowercase)
    if (c === 0x21 || c === 0x2D || c === 0x2E || c === 0x5F || c === 0x7E ||
        (c >= 0x27 && c <= 0x2A) ||
        (c >= 0x30 && c <= 0x3A) ||
        (c >= 0x41 && c <= 0x5A) ||
        (c >= 0x61 && c <= 0x7A)) {
      continue;
    }

    if (i - lastPos > 0)
      out += str.slice(lastPos, i);

    lastPos = i + 1;

    // Other ASCII characters
    if (c < 0x80) {
      out += hexTable[c];
      continue;
    }

    // Multi-byte characters ...
    if (c < 0x800) {
      out += hexTable[0xC0 | (c >> 6)] + hexTable[0x80 | (c & 0x3F)];
      continue;
    }
    if (c < 0xD800 || c >= 0xE000) {
      out += hexTable[0xE0 | (c >> 12)] +
             hexTable[0x80 | ((c >> 6) & 0x3F)] +
             hexTable[0x80 | (c & 0x3F)];
      continue;
    }
    // Surrogate pair
    ++i;
    var c2;
    if (i < str.length)
      c2 = str.charCodeAt(i) & 0x3FF;
    else
      c2 = 0;
    c = 0x10000 + (((c & 0x3FF) << 10) | c2);
    out += hexTable[0xF0 | (c >> 18)] +
           hexTable[0x80 | ((c >> 12) & 0x3F)] +
           hexTable[0x80 | ((c >> 6) & 0x3F)] +
           hexTable[0x80 | (c & 0x3F)];
  }
  if (lastPos === 0)
    return str;
  if (lastPos < str.length)
    return out + str.slice(lastPos);
  return out;
}

/**
 * Polyfill URLSearchParams
 *
 * Inspired from : https://github.com/WebReflection/url-search-params/blob/master/src/url-search-params.js
 */

function URLSearchParams(query) {
  var
    index, key, value,
    pairs, i, length,
    dict = Object.create(null)
  ;
  this[secret] = dict;
  if (!query) return;
  if (typeof query === 'string') {
    if (query.charAt(0) === '?') {
      query = query.slice(1);
    }
    for (
      pairs = query.split('&'),
        i = 0,
        length = pairs.length; i < length; i++
    ) {
      value = pairs[i];
      index = value.indexOf('=');
      if (-1 < index) {
        appendTo(
          dict,
          decode(value.slice(0, index)),
          decode(value.slice(index + 1))
        );
      } else if (value.length){
        appendTo(
          dict,
          decode(value),
          ''
        );
      }
    }
  } else {
    if (isArray(query)) {
      for (
        i = 0,
          length = query.length; i < length; i++
      ) {
        value = query[i];
        appendTo(dict, value[0], value[1]);
      }
    } else if (query.forEach) {
      query.forEach(addEach, dict);
    } else {
      for (key in query) {
        appendTo(dict, key, query[key]);
      }
    }
  }
}

var
  isArray = Array.isArray,
  URLSearchParamsProto = URLSearchParams.prototype,
  find = /[!'\(\)~]|%20|%00/g,
  plus = /\+/g,
  replace = {
    '!': '%21',
    "'": '%27',
    '(': '%28',
    ')': '%29',
    '~': '%7E',
    '%20': '+',
    '%00': '\x00'
  },
  replacer = function (match) {
    return replace[match];
  },
  secret = '__URLSearchParams__:' + Math.random()
;

function addEach(value, key) {
  /* jshint validthis:true */
  appendTo(this, key, value);
}

function appendTo(dict, name, value) {
  var res = isArray(value) ? value.join(',') : value;
  if (name in dict)
    dict[name].push(res);
  else
    dict[name] = [res];
}

function decode(str) {
  return decodeURIComponent(str.replace(plus, ' '));
}

function encode(str) {
  return encodeURIComponent(str).replace(find, replacer);
}

URLSearchParamsProto.append = function append(name, value) {
  appendTo(this[secret], name, value);
};

URLSearchParamsProto.delete = function del(name) {
  delete this[secret][name];
};

URLSearchParamsProto.get = function get(name) {
  var dict = this[secret];
  return name in dict ? dict[name][0] : null;
};

URLSearchParamsProto.getAll = function getAll(name) {
  var dict = this[secret];
  return name in dict ? dict[name].slice(0) : [];
};

URLSearchParamsProto.has = function has(name) {
  return name in this[secret];
};

URLSearchParamsProto.set = function set(name, value) {
  this[secret][name] = ['' + value];
};

URLSearchParamsProto.forEach = function forEach(callback, thisArg) {
  var dict = this[secret];
  Object.getOwnPropertyNames(dict).forEach(function(name) {
    dict[name].forEach(function(value) {
      callback.call(thisArg, value, name, this);
    }, this);
  }, this);
};

/*
URLSearchParamsProto.toBody = function() {
  return new Blob(
    [this.toString()],
    {type: 'application/x-www-form-urlencoded'}
  );
};
*/

URLSearchParamsProto.toJSON = function toJSON() {
  return {};
};

URLSearchParamsProto.toString = function toString() {
  var dict = this[secret], query = [], i, key, name, value;
  for (key in dict) {
    name = encode(key);
    for (
      i = 0,
        value = dict[key];
      i < value.length; i++
    ) {
      query.push(name + '=' + encode(value[i]));
    }
  }
  return query.join('&');
};

module.exports.URLSearchParams = URLSearchParams;

/**
 * Polyfill URL
 *
 * Inspired from : https://github.com/NVIDIA/url-polyfill/blob/master/url.js
 */

var relative = Object.create(null);
relative['ftp'] = 21;
relative['file'] = 0;
relative['gopher'] = 70;
relative['http'] = 80;
relative['https'] = 443;
relative['ws'] = 80;
relative['wss'] = 443;

var relativePathDotMapping = Object.create(null);
relativePathDotMapping['%2e'] = '.';
relativePathDotMapping['.%2e'] = '..';
relativePathDotMapping['%2e.'] = '..';
relativePathDotMapping['%2e%2e'] = '..';

function isRelativeScheme(scheme) {
  return relative[scheme] !== undefined;
}

function invalid() {
  clear.call(this);
  this._isInvalid = true;
}

function IDNAToASCII(h) {
  if ('' == h) {
    invalid.call(this)
  }
  // XXX
  return h.toLowerCase()
}

function percentEscape(c) {
  var unicode = c.charCodeAt(0);
  if (unicode > 0x20 &&
    unicode < 0x7F &&
    // " # < > ? `
    [0x22, 0x23, 0x3C, 0x3E, 0x3F, 0x60].indexOf(unicode) == -1
  ) {
    return c;
  }
  return encodeURIComponent(c);
}

function percentEscapeQuery(c) {
  // XXX This actually needs to encode c using encoding and then
  // convert the bytes one-by-one.

  var unicode = c.charCodeAt(0);
  if (unicode > 0x20 &&
    unicode < 0x7F &&
    // " # < > ` (do not escape '?')
    [0x22, 0x23, 0x3C, 0x3E, 0x60].indexOf(unicode) == -1
  ) {
    return c;
  }
  return encodeURIComponent(c);
}

var EOF = undefined,
  ALPHA = /[a-zA-Z]/,
  ALPHANUMERIC = /[a-zA-Z0-9\+\-\.]/;

function parse(input, stateOverride, base) {
  function err(message) {
    errors.push(message)
  }

  var state = stateOverride || 'scheme start',
    cursor = 0,
    buffer = '',
    seenAt = false,
    seenBracket = false,
    errors = [];

  loop: while ((input[cursor - 1] != EOF || cursor == 0) && !this._isInvalid) {
    var c = input[cursor];
    switch (state) {
      case 'scheme start':
        if (c && ALPHA.test(c)) {
          buffer += c.toLowerCase(); // ASCII-safe
          state = 'scheme';
        } else if (!stateOverride) {
          buffer = '';
          state = 'no scheme';
          continue;
        } else {
          err('Invalid scheme.');
          break loop;
        }
        break;

      case 'scheme':
        if (c && ALPHANUMERIC.test(c)) {
          buffer += c.toLowerCase(); // ASCII-safe
        } else if (':' == c) {
          this._scheme = buffer;
          buffer = '';
          if (stateOverride) {
            break loop;
          }
          if (isRelativeScheme(this._scheme)) {
            this._isRelative = true;
          }
          if ('file' == this._scheme) {
            state = 'relative';
          } else if (this._isRelative && base && base._scheme == this._scheme) {
            state = 'relative or authority';
          } else if (this._isRelative) {
            state = 'authority first slash';
          } else {
            state = 'scheme data';
          }
        } else if (!stateOverride) {
          buffer = '';
          cursor = 0;
          state = 'no scheme';
          continue;
        } else if (EOF == c) {
          break loop;
        } else {
          err('Code point not allowed in scheme: ' + c)
          break loop;
        }
        break;

      case 'scheme data':
        if ('?' == c) {
          query = '?';
          state = 'query';
        } else if ('#' == c) {
          this._fragment = '#';
          state = 'fragment';
        } else {
          // XXX error handling
          if (EOF != c && '\t' != c && '\n' != c && '\r' != c) {
            this._schemeData += percentEscape(c);
          }
        }
        break;

      case 'no scheme':
        if (!base || !(isRelativeScheme(base._scheme))) {
          err('Missing scheme.');
          invalid.call(this);
        } else {
          state = 'relative';
          continue;
        }
        break;

      case 'relative or authority':
        if ('/' == c && '/' == input[cursor+1]) {
          state = 'authority ignore slashes';
        } else {
          err('Expected /, got: ' + c);
          state = 'relative';
          continue
        }
        break;

      case 'relative':
        this._isRelative = true;
        if ('file' != this._scheme)
          this._scheme = base._scheme;
        if (EOF == c) {
          this._host = base._host;
          this._port = base._port;
          this._path = base._path.slice();
          this._query = base._query;
          this._username = base._username;
          this._password = base._password;
          break loop;
        } else if ('/' == c || '\\' == c) {
          if ('\\' == c)
            err('\\ is an invalid code point.');
          state = 'relative slash';
        } else if ('?' == c) {
          this._host = base._host;
          this._port = base._port;
          this._path = base._path.slice();
          this._query = '?';
          this._username = base._username;
          this._password = base._password;
          state = 'query';
        } else if ('#' == c) {
          this._host = base._host;
          this._port = base._port;
          this._path = base._path.slice();
          this._query = base._query;
          this._fragment = '#';
          this._username = base._username;
          this._password = base._password;
          state = 'fragment';
        } else {
          var nextC = input[cursor+1]
          var nextNextC = input[cursor+2]
          if (
            'file' != this._scheme || !ALPHA.test(c) ||
            (nextC != ':' && nextC != '|') ||
            (EOF != nextNextC && '/' != nextNextC && '\\' != nextNextC && '?' != nextNextC && '#' != nextNextC)) {
            this._host = base._host;
            this._port = base._port;
            this._username = base._username;
            this._password = base._password;
            this._path = base._path.slice();
            this._path.pop();
          }
          state = 'relative path';
          continue;
        }
        break;

      case 'relative slash':
        if ('/' == c || '\\' == c) {
          if ('\\' == c) {
            err('\\ is an invalid code point.');
          }
          if ('file' == this._scheme) {
            state = 'file host';
          } else {
            state = 'authority ignore slashes';
          }
        } else {
          if ('file' != this._scheme) {
            this._host = base._host;
            this._port = base._port;
            this._username = base._username;
            this._password = base._password;
          }
          state = 'relative path';
          continue;
        }
        break;

      case 'authority first slash':
        if ('/' == c) {
          state = 'authority second slash';
        } else {
          err("Expected '/', got: " + c);
          state = 'authority ignore slashes';
          continue;
        }
        break;

      case 'authority second slash':
        state = 'authority ignore slashes';
        if ('/' != c) {
          err("Expected '/', got: " + c);
          continue;
        }
        break;

      case 'authority ignore slashes':
        if ('/' != c && '\\' != c) {
          state = 'authority';
          continue;
        } else {
          err('Expected authority, got: ' + c);
        }
        break;

      case 'authority':
        if ('@' == c) {
          if (seenAt) {
            err('@ already seen.');
            buffer += '%40';
          }
          seenAt = true;
          for (var i = 0; i < buffer.length; i++) {
            var cp = buffer[i];
            if ('\t' == cp || '\n' == cp || '\r' == cp) {
              err('Invalid whitespace in authority.');
              continue;
            }
            // XXX check URL code points
            if (':' == cp && null === this._password) {
              this._password = '';
              continue;
            }
            var tempC = percentEscape(cp);
            (null !== this._password) ? this._password += tempC : this._username += tempC;
          }
          buffer = '';
        } else if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c) {
          cursor -= buffer.length;
          buffer = '';
          state = 'host';
          continue;
        } else {
          buffer += c;
        }
        break;

      case 'file host':
        if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c) {
          if (buffer.length == 2 && ALPHA.test(buffer[0]) && (buffer[1] == ':' || buffer[1] == '|')) {
            state = 'relative path';
          } else if (buffer.length == 0) {
            state = 'relative path start';
          } else {
            this._host = IDNAToASCII.call(this, buffer);
            buffer = '';
            state = 'relative path start';
          }
          continue;
        } else if ('\t' == c || '\n' == c || '\r' == c) {
          err('Invalid whitespace in file host.');
        } else {
          buffer += c;
        }
        break;

      case 'host':
      case 'hostname':
        if (':' == c && !seenBracket) {
          // XXX host parsing
          this._host = IDNAToASCII.call(this, buffer);
          buffer = '';
          state = 'port';
          if ('hostname' == stateOverride) {
            break loop;
          }
        } else if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c) {
          this._host = IDNAToASCII.call(this, buffer);
          buffer = '';
          state = 'relative path start';
          if (stateOverride) {
            break loop;
          }
          continue;
        } else if ('\t' != c && '\n' != c && '\r' != c) {
          if ('[' == c) {
            seenBracket = true;
          } else if (']' == c) {
            seenBracket = false;
          }
          buffer += c;
        } else {
          err('Invalid code point in host/hostname: ' + c);
        }
        break;

      case 'port':
        if (/[0-9]/.test(c)) {
          buffer += c;
        } else if (EOF == c || '/' == c || '\\' == c || '?' == c || '#' == c || stateOverride) {
          if ('' != buffer) {
            var temp = parseInt(buffer, 10);
            if (temp != relative[this._scheme]) {
              this._port = temp + '';
            }
            buffer = '';
          }
          if (stateOverride) {
            break loop;
          }
          state = 'relative path start';
          continue;
        } else if ('\t' == c || '\n' == c || '\r' == c) {
          err('Invalid code point in port: ' + c);
        } else {
          invalid.call(this);
        }
        break;

      case 'relative path start':
        if ('\\' == c)
          err("'\\' not allowed in path.");
        state = 'relative path';
        if ('/' != c && '\\' != c) {
          continue;
        }
        break;

      case 'relative path':
        if (EOF == c || '/' == c || '\\' == c || (!stateOverride && ('?' == c || '#' == c))) {
          if ('\\' == c) {
            err('\\ not allowed in relative path.');
          }
          var tmp;
          if (tmp = relativePathDotMapping[buffer.toLowerCase()]) {
            buffer = tmp;
          }
          if ('..' == buffer) {
            this._path.pop();
            if ('/' != c && '\\' != c) {
              this._path.push('');
            }
          } else if ('.' == buffer && '/' != c && '\\' != c) {
            this._path.push('');
          } else if ('.' != buffer) {
            if ('file' == this._scheme && this._path.length == 0 && buffer.length == 2 && ALPHA.test(buffer[0]) && buffer[1] == '|') {
              buffer = buffer[0] + ':';
            }
            this._path.push(buffer);
          }
          buffer = '';
          if ('?' == c) {
            this._query = '?';
            state = 'query';
          } else if ('#' == c) {
            this._fragment = '#';
            state = 'fragment';
          }
        } else if ('\t' != c && '\n' != c && '\r' != c) {
          buffer += percentEscape(c);
        }
        break;

      case 'query':
        if (!stateOverride && '#' == c) {
          this._fragment = '#';
          state = 'fragment';
        } else if (EOF != c && '\t' != c && '\n' != c && '\r' != c) {
          this._query += percentEscapeQuery(c);
        }
        break;

      case 'fragment':
        if (EOF != c && '\t' != c && '\n' != c && '\r' != c) {
          this._fragment += c;
        }
        break;
    }

    cursor++;
  }
}

function clear() {
  this._scheme = '';
  this._schemeData = '';
  this._username = '';
  this._password = null;
  this._host = '';
  this._port = '';
  this._path = [];
  this._query = '';
  this._fragment = '';
  this._isInvalid = false;
  this._isRelative = false;
}

// Does not process domain names or IP addresses.
// Does not handle encoding for the query parameter.
function jURL(url, base /* , encoding */) {
  if (base !== undefined && !(base instanceof jURL))
    base = new jURL(String(base));

  url = String(url)

  this._url = url
  clear.call(this);

  var input = url.replace(/^[ \t\r\n\f]+|[ \t\r\n\f]+$/g, '');
  // encoding = encoding || 'utf-8'

  parse.call(this, input, null, base);
}

jURL.prototype = {
  toString: function() {
    return this.href;
  },
  get href() {
    if (this._isInvalid)
      return this._url;

    var authority = '';
    if ('' != this._username || null != this._password) {
      authority = this._username +
        (null != this._password ? ':' + this._password : '') + '@';
    }

    return this.protocol +
      (this._isRelative ? '//' + authority + this.host : '') +
      this.pathname + this._query + this._fragment;
  },
  set href(href) {
    clear.call(this);
    parse.call(this, href);
  },

  get protocol() {
    return this._scheme + ':';
  },
  set protocol(protocol) {
    if (this._isInvalid)
      return;
    parse.call(this, protocol + ':', 'scheme start');
  },

  get host() {
    return this._isInvalid ? '' : this._port ?
      this._host + ':' + this._port : this._host;
  },
  set host(host) {
    if (this._isInvalid || !this._isRelative)
      return;
    parse.call(this, host, 'host');
  },

  get hostname() {
    return this._host;
  },
  set hostname(hostname) {
    if (this._isInvalid || !this._isRelative)
      return;
    parse.call(this, hostname, 'hostname');
  },

  get port() {
    return this._port;
  },
  set port(port) {
    if (this._isInvalid || !this._isRelative)
      return;
    parse.call(this, port, 'port');
  },

  get pathname() {
    return this._isInvalid ? '' : this._isRelative ?
      '/' + this._path.join('/') : this._schemeData;
  },
  set pathname(pathname) {
    if (this._isInvalid || !this._isRelative)
      return;
    this._path = [];
    parse.call(this, pathname, 'relative path start');
  },

  get search() {
    return this._isInvalid || !this._query || '?' == this._query ?
      '' : this._query;
  },
  set search(search) {
    if (this._isInvalid || !this._isRelative)
      return;
    this._query = '?';
    if ('?' == search[0])
      search = search.slice(1);
    parse.call(this, search, 'query');
  },

  // Gets the URLSearchParams object representing the query parameters of the URL.
  // This property is read-only; to replace the entirety of query parameters of the URL, use the url.search setter.
  get searchParams() {
    return new URLSearchParams(this._query);
  },

  get hash() {
    return this._isInvalid || !this._fragment || '#' == this._fragment ?
      '' : this._fragment;
  },
  set hash(hash) {
    if (this._isInvalid)
      return;
    this._fragment = '#';
    if ('#' == hash[0])
      hash = hash.slice(1);
    parse.call(this, hash, 'fragment');
  },

  get origin() {
    var host;
    if (this._isInvalid || !this._scheme) {
      return '';
    }
    // javascript: Gecko returns String(""), WebKit/Blink String("null")
    // Gecko throws error for "data://"
    // data: Gecko returns "", Blink returns "data://", WebKit returns "null"
    // Gecko returns String("") for file: mailto:
    // WebKit/Blink returns String("SCHEME://") for file: mailto:
    switch (this._scheme) {
      case 'data':
      case 'file':
      case 'javascript':
      case 'mailto':
        return 'null';
    }
    host = this.host;
    if (!host) {
      return '';
    }
    return this._scheme + '://' + host;
  }
};

module.exports.URL = jURL;
