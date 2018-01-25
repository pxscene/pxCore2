// A simple "toy" WS implementatin for DukLuv (Duktape 1.x)

(function() {

var classes = require('classes.js');
var _url = require('url.js');

// TODO update to TextDecoder
// TODO update buffer handling
//var decode = new TextDecoder().decode;





function concatTypedArrays(a, b) { // a, b TypedArray of same type
  var c = new (a.constructor)(a.length + b.length);
  c.set(a, 0);
  c.set(b, a.length);
  return c;
}

// Designed to used within a coroutine will yield if there is insufficient data
// to fulfill a read* request.
// resume after additional data has been appended via the .appendData method.
function DataStream() {
  // TODO continually growing
  this.u8Arr = new Uint8Array(0);
  this.dv = new DataView(this.u8Arr);
  this.offset = 0;
  this.yield = Duktape.Thread.yield;
  this.closed = false;
}

DataStream.prototype.ensureLen = function(len) {
  //print("ensure length")
  //print("check yield", this.offset, this.dv.length, len)
  while (this.offset+len-1 >= this.dv.length) {
    //print("yielding")
    this.yield(!this.closed);
  }
}

DataStream.prototype.close = function() {
  this.closed = true;
}

DataStream.prototype.appendData = function(d) {
  // TODO performance... growth
  //print("appending")
  if (this.offset >= this.dv.length) {
    //print("no need to concat")
    this.u8Arr = d;
  }
  else {
    //print("concat arrays")
    this.u8Arr = concatTypedArrays(this.u8Arr.subarray(this.offset), d);
  }
  this.offset = 0;
  //print("total buffer: ", this.u8Arr.length)
  this.dv = new DataView(this.u8Arr);
}

// Warning assume ascii encoding in buffer
DataStream.prototype.readStringTilUint8 = function(u8, consume) {
  var s = "";
  for (var i = this.offset; i < this.dv.length; i++) {
    var b = this.readUint8(consume)
    if (b==u8) {
      break;
    }
    else
      s += String.fromCharCode(b);
  }
  return s;
}

DataStream.prototype.read = function(len, consume) {
  if (consume == undefined)
    consume = true;
  var result = new Uint8Array(len);
  for (var i = 0; i < len; i++)
    result[i] = this.readUint8();
  return result;
}

DataStream.prototype.readUint8 = function(consume) {
  if (consume == undefined)
    consume = true;
  this.ensureLen(1);
  //print("offset: ", this.offset);
  //print("len: ", this.dv.length);
  var result = this.dv.getUint8(this.offset);
  if (consume)
    this.offset += 1;
  return result;
}

DataStream.prototype.readUint16 = function(consume) {
  if (consume==undefined)
    consume = true;
  this.ensureLen(2);
  var result = this.dv.getUint16(this.offset);
  if (consume)
    this.offset += 2;
  return result;
}

DataStream.prototype.readUint32 = function(consume) {
  if (consume==undefined)
    consume = true;
  this.ensureLen(4);
  var result = this.dv.getUint32(this.offset);
  if (consume)
    this.offset += 4;
  return result;
}


//uv.tcp_getpeername("www.google.com")

function WebSocket(url) {
  this.url = url;
  this.secure = false;

  this.parsedUrl = _url.parse(this.url);
  if (!this.parsedUrl.path)
    this.parsedUrl.path = "/";

  print(this.parsedUrl.hostname);
  print(this.parsedUrl.port);
  print(this.parsedUrl.path);

  //this.send2 = this.send2.bind(this);
  //this.process2 = this.process.bind(this);

  this.responseHdr = [];
  this.state = this.WS_STATE_HDR_VERB;
  this.tcp = new classes.Tcp();  

  this.t = new Duktape.Thread(this.process);
  //print("after starting thread")
  this.dv = new DataStream();

  var that = this;

  uv.dns_getaddrinfo({
    //node: "luvit.io",
    node: this.parsedUrl.hostname,
    socktype: "stream", // Only show TCP results
    // family: "inet",  // Only show IPv4 results
  }, function (err, results) {
    print("getaddr");
    //assert(!err, err);
    if (!err && results.length > 0) {
      print(JSON.stringify(results))
      that.tcp.connect(results[0].addr, parseInt(that.parsedUrl.port), function(e){
        //print("connected", that.url)    
        that.tcp.readStart(function(a,b,c) {
          //print("in readStart", a,b,c)
          if (b != undefined) {
            that.dv.appendData(new Uint8Array(b));
            // We have more data let's process it with a coroutine until it yields
            Duktape.Thread.resume(that.t,that);
          }
          else {
            that.state = that.WS_STATE_CLOSED;
            print("read callback with undefined... assume remote socket closed")
            that.emit("close");
          }
        });
        print("path", that.parsedUrl.path);
        print("port", that.parsedUrl.port);
        that.tcp.write("GET "+that.parsedUrl.path+" HTTP/1.1\r\n"+
//        that.tcp.write("GET / HTTP/1.1\r\n"+
        //"Host: "+that.parsedUrl.host+"\r\n"+
        "Connection: Upgrade\r\n"+
        "Pragma: no-cache\r\n"+
        "Cache-Control: no-cache\r\n"+
        "Upgrade: websocket\r\n"+
//        "Origin: file://\r\n"+
        "Sec-WebSocket-Version: 13\r\n"+
//        "User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.116 Safari/537.36\r\n"+
//        "Accept-Encoding: gzip, deflate, sdch\r\n"+
//        "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"+
        "Sec-WebSocket-Key: bKdPyn3u98cTfZJSh4TNeQ==\r\n"+
//        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n"+
        "\r\n",function(){print("header written");});
      });
    }
    else
      print("Error resolving host: ", this.parsedUrl.hostname);
  });
}

WebSocket.prototype.__proto__ = classes.Emitter.prototype;

WebSocket.prototype.WS_STATE_ERROR    = -1;
WebSocket.prototype.WS_STATE_CLOSED   = -2;
WebSocket.prototype.WS_STATE_HDR_VERB = 0;
WebSocket.prototype.WS_STATE_HDR_LINE = 1;
WebSocket.prototype.WS_STATE_HDR_COMPLETE = 2;
WebSocket.prototype.WS_STATE_DATA     = 3;

WebSocket.prototype.WS_OP_CONTINUATION = 0x0;
WebSocket.prototype.WS_OP_TEXTFRAME    = 0x1;
WebSocket.prototype.WS_OP_BINFRAME     = 0x2;
WebSocket.prototype.WS_OP_CLOSE        = 0x8;
WebSocket.prototype.WS_OP_PING         = 0x9;
WebSocket.prototype.WS_OP_PONG         = 0xA;

WebSocket.prototype.emit = function(t, args) {
  if (this.handlers[t]) {
    for(var i = 0; i < this.handlers[t].length; i++) {
      this.handlers[t][i].apply(null, args);
    }
  }
}

WebSocket.prototype.send2 = function(op, data) {
  var hdrSize;
  var dataSize = (data)?data.length:0;
  if (dataSize <= 125)
    hdrSize = 2;
  else if (dataSize <= 32768)
    hdrSize = 4;
  var hdr = new Uint8Array(hdrSize)
  var dv = new DataView(hdr);
  dv.setUint8(0, 0x80 | (op & 0x0f)); // write fin and op
  if (dataSize <= 125) {
    dv.setUint8(1, dataSize);
  }
  else if (datasize <= 32768) {
    dv.setUint8(1,126);
    ds.setUint16(2,dataSize);
  }
  else {
    print("Don't support message larger than 32768 bytes currently.")
  }
  this.tcp.write(Duktape.Buffer(hdr));
  if (data)
    this.tcp.write(Duktape.Buffer(data));
  //print("sending message")
}

WebSocket.prototype.ping = function(cb) {
  this.send2(this.WS_OP_PING, null);
  if (cb)
    cb();
}

WebSocket.prototype.send = function(data, cb) {
  // TODO if string utf8 encode
  this.send2(WS_OP_BINFRAME, data);
  if (cb)
    cb();
}

// Duktape coroutines doesn't seem to like the this to be bound to another object
WebSocket.prototype.process = function(_that) {
  //print("processWS")
  while (_that.state != _that.WS_STATE_CLOSED || _that.state != _that.WS_STATE_ERROR) {
    if (_that.state == _that.WS_STATE_HDR_VERB) {
      var l = _that.dv.readStringTilUint8("\r".charCodeAt(0));
      //dv.readUint8(); // eat \r
      _that.dv.readUint8(); // eat \n
      //print(l)
      //print(l.length)
      if (l == "HTTP/1.1 101 Switching Protocols") {
        _that.state = _that.WS_STATE_HDR_LINE;
      }
      else
        _that.state = _that.WS_STATE_ERROR;
    }
    else if (_that.state == _that.WS_STATE_HDR_LINE) {
      //print("here")
      var l = _that.dv.readStringTilUint8("\r".charCodeAt(0));
      //dv.readUint8(); // eat \r
      _that.dv.readUint8(); // eat \n
      //print(l)
      if (l != "") {
        var f = l.split(": ");
        _that.responseHdr.push({key:f[0],val:f[1]});
      }
      else
        _that.state = _that.WS_STATE_HDR_COMPLETE;
    }
    else if (_that.state == _that.WS_STATE_HDR_COMPLETE) {
      print("header complete")
      //print(_that.responseHdr);
      for (var x =0; x < _that.responseHdr.length; x++)
        print("hdr")
      // TODO sanity check headers
      _that.state = _that.WS_STATE_DATA;
      _that.emit("connected");

      /*
      var pingMessage = new Uint8Array(1);
      pingMessage[0] = 'a'.charCodeAt(0);
      _that.send2(_that.WS_OP_PING, pingMessage);  
      */    
    }
    else if (_that.state == _that.WS_STATE_DATA) {
      //print("processing frame")
      var b = _that.dv.readUint8();
      var fin = b & 0x80;
      var op  = b & 0x0f;
      //print("fin: ", fin)
      //print("op: ", op)
      var b = _that.dv.readUint8();
      var mask = b & 0x80;
      var len = b & 0x7f;
      if (len == 126) {
        len = _that.dv.readUint16();
      }
      else if (len == 127) {
        print("Error 64 bit length not supported")
      }
      //print("mask: ", mask)
      //print("len: ", len)
      var message = _that.dv.read(len);
      //print(String.fromCharCode.apply(null, message));

      if (op==_that.WS_OP_CONTINUATION || op==_that.WS_OP_TEXTFRAME || op==_that.WS_OP_BINFRAME){
        /*
        if (op==_that.WS_OP_CONTINUATION)
          print("WS_OP_CONTINUATION")
        else if (op==_that.WS_OP_TEXTFRAME)
          print("WS_OP_TEXTFRAME")
        else if (op==_that.WS_OP_BINFRAME)
          print("WS_OP_BINFRAME")
          */
/*
        for(var i = 0; i < _that.handlers['message'].length; i++) {
          _that.handlers['message'][i](message);
        }
        */
        // TODO handle continuation frames
        // TODO handle fin bit
        // TODO if text frame decode utf8 string
        _that.emit("message",[message]);
      }
      else if (op==_that.WS_OP_CLOSE) {
        state = _that.WS_STATE_CLOSED;
        print("WS_OP_CLOSE")
      }
      else if (op==_that.WS_OP_PING) {
        // Send Pong back with same payload
        //print("WS_OP_PING")
        _that.send2(_that.WS_OP_PONG, message);   
      }
      else if (op==_that.WS_OP_PONG) {
        // Eat Pongs per spec
        //print("WS_OP_PONG")
        _that.emit("pong")         
      }
    }
  }
  print("exited loop")
  _that.emit("close")

  that.tcp.shutdown(function() {
    print("socket closed")
  });
}

module.exports=WebSocket;

}());
