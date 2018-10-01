function assert(condition, message) {
  if (!condition) {
    throw new Error(`Assertion failed. ${message}`);
  }
}

console.log('TEST events');
const events = require('events');
const myEmitter = new events();
let eventArrived = false;
myEmitter.on('event', () => {
  eventArrived = true;
});
const hadListeners = myEmitter.emit('event');
assert(hadListeners === true, 'events test #1 failed');
setTimeout(() => {
  assert(eventArrived === true, 'events test #2 failed');
});

console.log('TEST buffer');
const buffer = require('buffer');
const buffer1 = buffer.Buffer;
const buf1 = buffer1.from('hello world', 'ascii');
assert(buf1.toString() === 'hello world', 'buffer test #1 failed');
assert(buf1.toString('hex') === '68656c6c6f20776f726c64', 'buffer test #2 failed');
assert(buf1.toString('base64') === 'aGVsbG8gd29ybGQ=', 'buffer test #3 failed');

console.log('TEST stream');
const stream = require('stream');
const Readable = stream.Readable;
const inStream = new Readable({
  read() {}
});
inStream.push('ABCDEFGHIJKLM');
inStream.push('NOPQRSTUVWXYZ');
inStream.push(null); // No more data
const buf = inStream.read(10);
assert(buf.toString() === 'ABCDEFGHIJ', 'stream test #1 failed');

console.log('TEST oauth');
const oauth = require('oauth');
const OAuth = oauth.OAuth;
const oauth_var = new OAuth("https://api.twitter.com/oauth/request_token",
  "https://api.twitter.com/oauth/access_token",
  "key",
  "secret",
  "1.0",
  null,
  "HMAC-SHA1");
assert(typeof oauth_var.getOAuthRequestToken === 'function', 'oauth test #1 failed');

console.log('TEST crypto');
const crypto = require('crypto');
const secret = 'abcdefg';
const hash = crypto.createHmac('sha256', secret)
.update('I love cupcakes')
.digest('hex');
assert(hash === 'c0fa1bc00531bd78ef38c628449c5102aeabd49b5dc3a2a516ea6ea959d6658e', 'crypto test #1 failed');

console.log('TEST htmlparser');
const htmlparser = require('htmlparser');
const rawHtml = "<script language= javascript></script>";
let parsed;
const handler = new htmlparser.DefaultHandler(function (error) {
  parsed = error === null;
});
const parser = new htmlparser.Parser(handler);
parser.parseComplete(rawHtml);
setTimeout(() => {
  assert(parsed === true, 'htmlparser test #1 failed');
});

console.log('TEST jszip');
const jszip = require('jszip');
const zip = new jszip();
zip.file("hello.txt", "Hello, World!").file("tempfile", "nothing");
zip.folder("images").file("smile.gif", "R0lGODlhAQABAIAAAAUEBAAAACwAAAAAAQABAAACAkQBADs=", {base64: true});
zip.file("Xmas.txt", "Ho ho ho !", {date : new Date("December 25, 2007 00:00:01")});
zip.remove("tempfile");
const base64zip = zip.generate();
const first10 = base64zip.toString().substr(0, 10);
assert(first10 === 'UEsDBAoAAA', 'jszip test #1 failed');

console.log('TEST url');
const Url = require('url');
const obj = Url.parse('https://example.com');
assert(obj.hostname === 'example.com', 'url test #1 failed');
const r1 = Url.resolve('/one/two/three', 'four');
const r2 = Url.resolve('http://example.com/', '/one');
const r3 = Url.resolve('http://example.com/one', '/two');
assert(r1 === '/one/two/four', 'url test #2 failed');
assert(r2 === 'http://example.com/one', 'url test #3 failed');
assert(r3 === 'http://example.com/two', 'url test #4 failed');

console.log('TEST querystring');
const querystring = require('querystring');
var q1 = querystring.stringify({ foo: 'bar', baz: ['qux', 'quux'], corge: '' });
var q2 = querystring.stringify({ foo: 'bar', baz: 'qux' }, ';', ':');
assert(q1.toString() === 'foo=bar&baz=qux&baz=quux&corge=', 'querystring test #1 failed');
assert(q2.toString() === 'foo:bar;baz:qux', 'querystring test #2 failed');

// rm /tmp/cache/*
console.log('TEST http');
const http = require('http');
let read = 0;
const onResponse = function (res) {
  assert(res.statusCode === 0, 'http test #1 failed');
  res.on('data', function (c) {
    read += c.length;
  });
  res.on('end', function () {
    assert(read > 100, 'http test #2 failed');
  });
};
http.get('https://example.com', onResponse);
setTimeout(() => {
  assert(read > 100, 'http test #3 failed');
}, 5000);
