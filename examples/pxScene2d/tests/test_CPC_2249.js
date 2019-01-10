/**
 * An interactive test for the sound service with Receiver m4a Sound File Playback (CPC-2249).
 *
 * How to use the test:
 *   Buttons initService, killService, garbageCollect manage service lifecycle,
 *   url, volume, position, autoPlay - select value for set... functions,
 *   set..., get..., play, pause - call sound functions.
 */

px.import({
  scene: 'px:scene.1.js',
  keys: 'px:tools.keys.js'
})
.then(function(imports) {
  var NS = {
    mtd: {
      setProperties: ["url", "volume", "position", "autoPlay"],
      setUrl: ["url"],
      setVolume: ["volume"],
      setPosition: ["position"],
      setAutoPlay: ["autoPlay"],
      getProperties: [],
      getUrl: [],
      getVolume: [],
      getPosition: [],
      getAutoPlay: [],
      play: [],
      pause: []
    },
    evt:["onStreamPlaying","onStreamPaused","onStreamProgress","onStreamComplete","onReady","onError"],
    sid:"org.rdk.soundPlayer_1",
    val:{
      url:[
        "http://cdn.online-convert.com/example-file/audio/mp3/example.mp3",
        "http://www.sample-videos.com/audio/mp3/wave.mp3",
        "http://custom-hls.iheart.com/ihr-ingestion-pipeline-production-sbmg/encodes/May18/053018/A10301A0001408727X_20180529180357690/193240.20122.m4a?null"
      ],
      volume:[100,90,80,70,60,50,40,30,20,10,0],
      position:[0,5000,10000,15000,20000,25000,30000,60000],
      autoPlay:[true,false]
    },
    txt:{},
    arg:{}
  }, FN = {
    createTextBox: function(x,y,text) {
      return imports.scene.create({
        t: "textBox",
        parent: imports.scene.root,
        x: x,
        y: y,
        w: imports.scene.w,
        h: 20,
        text: text,
        a: 0.4,
        pixelSize: 15,
        textColor: 0xffffffff
      });
    },
    setTextBoxText: function(textBox,text) {
      textBox.text = text;
      textBox.a = 1.0;
      textBox.animateTo(
          {a:0.4},
          4,
          imports.scene.animation.TWEEN_LINEAR,
          imports.scene.animation.OPTION_LOOP,
          1);
    },
    customServiceMethods: {
      initService: function() {
        var _service = imports.scene.getService(NS.sid);
        _service.setApiVersionNumber(1);
        _service.registerForEvents(JSON.stringify({"events":NS.evt}));
        _service.onEvent = function (event, res) {
          FN.setTextBoxText(NS.txt[event], event+" "+res);
        };
        NS.s = _service;
      },
      killService: function () {
        NS.s.unregisterEvents();
        delete NS.s;
      },
      garbageCollect: function () {
        imports.scene.collectGarbage();
      }
    },
    toggle: function(id) {
      if (Object.keys(NS.val).indexOf(id) >= 0) {
        var _values = NS.val[id]
            , _arg = NS.arg[id]
            , _index = _values.indexOf(_arg)
            , _val = _values[_index < _values.length - 1 ? _index + 1 : 0]
        ;
        NS.arg[id] = _val;
        FN.setTextBoxText(NS.txt[id], id+" = "+_val);

      } else if (Object.keys(NS.mtd).indexOf(id) >= 0) {
        var _args = []
            , _keys = NS.mtd[id]
            , _i = 0
            , _r
            , _paramsStr
        ;
        for (; _i < _keys.length; _i++) {
          _args.push(NS.arg[_keys[_i]]);
        }
        _paramsStr = JSON.stringify({"params":_args});
        _r = NS.s.callMethod(id, _paramsStr);
        FN.setTextBoxText(NS.txt[id], id+" "+JSON.stringify(_args)+" >>> "+_r);

      } else if (Object.keys(FN.customServiceMethods).indexOf(id) >= 0) {
        FN.customServiceMethods[id]();
        FN.setTextBoxText(NS.txt[id], id);
      }
    }
  };
  // ==================================================
  (function main() {
    var _y = 0
        , _sel = 0
        , _ptrBox
        , _lineHeight
        , _allKeys = Object.keys(FN.customServiceMethods)
        .concat(Object.keys(NS.val))
        .concat(Object.keys(NS.mtd))
        .concat(NS.evt)
        ;
    _ptrBox = FN.createTextBox(0, _y, "->");
    _allKeys.forEach(function(k) {
      var _textBox = FN.createTextBox(50, _y, k);
      _y += _textBox.h;
      NS.txt[k] = _textBox;
    });
    _lineHeight = _y / _allKeys.length;
    Object.keys(NS.val).forEach(function(k) {
      FN.toggle(k); // set defaults
    });
    imports.scene.root.on("onKeyDown", function (e) {
      switch (e.keyCode) {
        case imports.keys.DOWN:
          if (_sel < _allKeys.length - 1) {
            _ptrBox.y = ++_sel * _lineHeight;
          }
          break;
        case imports.keys.UP:
          if (_sel > 0) {
            _ptrBox.y = --_sel * _lineHeight;
          }
          break;
        case imports.keys.ENTER:
          FN.toggle(_allKeys[_sel]);
          break;
      }
    });
  })();
});
