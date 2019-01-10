/**
 * An interactive test for Optimus.
 */

px.import({scene:'px:scene.1.js',keys:'px:tools.keys.js',optimus:'optimus'})
.then(imports => {

  const scene = imports.scene;
  const optimus = imports.optimus;
  let app;
  let logOnScreen;
  let log = (...args) => {
    console.log.apply(console, args);
    if (logOnScreen) {
      logOnScreen.apply(null, args);
    }
  };

  const samples = {
    "optimus.createApplication [westeros_test]": () => {
      optimus.setScene(scene);
      app = optimus.createApplication({
        id: "1", priority: 1, x: 320, y: 180, w: 640, h: 360,
        cx: 0, cy: 0, sx: 1.0, sy: 1.0, r: 0, a: 1, interactive: true,
        painting: true, clip: false, mask: false, draw: true,
        launchParams: {"cmd":"westeros_test"}
      });
      log(`optimus.createApplication called`);
    },
    "optimus.createApplication [netflix]": () => {
      optimus.setScene(scene);
      app = optimus.createApplication({
        id: "1", priority: 1, x: 320, y: 180, w: 640, h: 360,
        cx: 0, cy: 0, sx: 1.0, sy: 1.0, r: 0, a: 1, interactive: true,
        painting: true, clip: false, mask: false, draw: true,
        launchParams: {"cmd":"netflix"}
      });
      log(`optimus.createApplication called`);
    },
    "optimus.createApplication [receiver]": () => {
      optimus.setScene(scene);
      app = optimus.createApplication({
        id: "2", priority: 1, x: 320, y: 180, w: 640, h: 360,
        cx: 0, cy: 0, sx: 1.0, sy: 1.0, r: 0, a: 1, interactive: true,
        painting: true, clip: false, mask: false, draw: true,
        launchParams: {"cmd":"receiver"}
      });
      log(`optimus.createApplication called`);
    },
    "optimus.createApplication [spark]": () => {
      optimus.setScene(scene);
      app = optimus.createApplication({
        id: "3", priority: 1, x: 320, y: 180, w: 640, h: 360,
        cx: 0, cy: 0, sx: 1.0, sy: 1.0, r: 0, a: 1, interactive: true,
        painting: true, clip: false, mask: false, draw: true,
        launchParams: {"cmd":"spark","uri":"browser.js"}
      });
      log(`optimus.createApplication called`);
    },
    "optimus.createApplication [WebApp]": () => {
      optimus.setScene(scene);
      app = optimus.createApplication({
        id: "4", priority: 1, x: 320, y: 180, w: 640, h: 360,
        cx: 0, cy: 0, sx: 1.0, sy: 1.0, r: 0, a: 1, interactive: true,
        painting: true, clip: false, mask: false, draw: true,
        launchParams: {"cmd":"WebApp","uri":"https://www.google.com"}
      });
      log(`optimus.createApplication called`);
    },
    "optimus.on [create/ready/suspend/resume/destroy]": () => {
      optimus.on("create", (app) => { log("<optimus event>",`created ${JSON.stringify(app)}`) });
      optimus.on("ready", (app) => { log("<optimus event>",`ready ${JSON.stringify(app)}`) });
      optimus.on("suspend", (app) => { log("<optimus event>",`suspended ${JSON.stringify(app)}`) });
      optimus.on("resume", (app) => { log("<optimus event>",`resumed ${JSON.stringify(app)}`) });
      optimus.on("destroy", (app) => { log("<optimus event>",`destroyed ${JSON.stringify(app)}`) });
      log(`optimus.on called`);
    },
    "optimus.removeListener [create/ready/suspend/resume/destroy]": () => {
      optimus.removeListener("create");
      optimus.removeListener("ready");
      optimus.removeListener("suspend");
      optimus.removeListener("resume");
      optimus.removeListener("destroy");
      log(`optimus.removeListener called`);
    },
    "app.ready": () => {
      app.ready.then(() => { log("ready promise resolved") });
      log(`app.ready called`);
    },
    "app.suspend": () => {
      let ret = app.suspend();
      log(`app.suspend returned: ${ret}`);
    },
    "app.resume": () => {
      let ret = app.resume();
      log(`app.resume returned: ${ret}`);
    },
    "app.destroy": () => {
      let ret = app.destroy();
      log(`app.destroy returned: ${ret}`);
    },
    "app.state": () => {
      let ret = app.state();
      log(`app.state returned: ${ret}`);
    },
    "app.moveToFront": () => {
      app.moveToFront();
      log(`app.moveToFront called`);
    },
    "app.moveToBack": () => {
      app.moveToBack();
      log(`app.moveToBack called`);
    },
    "app.moveForward": () => {
      app.moveForward();
      log(`app.moveForward called`);
    },
    "app.moveBackward": () => {
      app.moveBackward();
      log(`app.moveBackward called`);
    },
    "app.setFocus": () => {
      app.setFocus();
      log(`app.setFocus called`);
    },
    "app.isFocused": () => {
      let ret = app.isFocused();
      log(`app.isFocused returned: ${ret}`);
    },
    "app.animateTo": () => {
      const x = app.x;
      app.animateTo({ x: x+100 },0.5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_FASTFORWARD,1)
      .then(() => {
        log(`app.animateTo first animation done`);
        return app.animateTo({ x: x },0.5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_FASTFORWARD,1);
      })
      .then(() => {
        log(`app.animateTo second animation done`);
      });
    },
    "app.animate": () => {
      const y = app.y;
      app.animate({ y: y+100 },0.5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_FASTFORWARD,1);
      log(`app.animate first animation started`);
      setTimeout(() => {
        app.animate({ y: y },0.5,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_FASTFORWARD,1);
        log(`app.animate second animation started`);
      }, 1000);
    },
    "app.setProperties": () => {
      const y = app.y;
      app.y = y+100;
      log(`app.y set`);
      setTimeout(() => {
        app.y = y;
        log(`app.y set to the initial value`);
      }, 500);
    },
    "app[...]": () => {
      let props = {};
      let keys = [
        'id','name','priority','type',
        'x','y','w','h','cx','cy','sx','sy','r','a','interactive','painting','clip','mask','draw',
        'hasApi','pid','externalApp'
      ];
      for (const key of keys) {
        props[key] = app[key];
      }
      log(`app properties are: ${JSON.stringify(props)}`);
    },
    "app = null": () => {
      app = null;
      log(`app set to null`);
    }
  };

  /**
   * UI
   */
  (() => {
    logOnScreen = (...args) => {
      const lines = _logTextBox.text.split(/\n/).filter(i => i);
      lines.push(args.join(' '));
      _logTextBox.text = lines.join("\n");
      const m = _logTextBox.measureText().bounds.y2;
      _logTextBox.y = _logY + Math.min(0, _logHeight - m);
    };
    let _y = 0
      , _sel = 0
      , _buttonsView
      , _logView
      , _ptrTextBox
      , _logTextBox
      , _lineHeight
      , _logHeight
      , _logY
      , _allKeys = Object.keys(samples)
    ;
    _buttonsView = scene.create({t:"object",parent:scene.root,x:0,y:0,w:scene.w/2,h:scene.h});
    _logView = scene.create({t:"rect",parent:scene.root,x:scene.w/2,y:0,w:scene.w/2,h:scene.h,fillColor:0x00000088});
    _ptrTextBox = scene.create({t:"textBox",parent:_buttonsView,x:0,y:0,w:50,h:20,text:"->",pixelSize:18,textColor:0xffffffff});
    for (const k of _allKeys) {
      const txt = scene.create({t:"textBox",parent:_buttonsView,x:50,y:_y,w:50,h:20,text:k,pixelSize:18,textColor:0xffffffff});
      _y += txt.h;
    }
    _lineHeight = _y / _allKeys.length;
    _logHeight = scene.h;
    _logY = 0;
    _logTextBox = scene.create({t:"textBox",parent:_logView,x:0,y:_logY,w:_logView.w,h:_logHeight,text:"",wordWrap:true,pixelSize:18,textColor:0xffffffff});
    imports.scene.root.on("onKeyDown", (e) => {
      switch (e.keyCode) {
        case imports.keys.DOWN:
          if (_sel < _allKeys.length - 1) {
            _ptrTextBox.y = ++_sel * _lineHeight;
          }
          break;
        case imports.keys.UP:
          if (_sel > 0) {
            _ptrTextBox.y = --_sel * _lineHeight;
          }
          break;
        case imports.keys.ENTER:
          try {
            samples[_allKeys[_sel]]();
          } catch (e) {
            log(`OOPS, this didn't work: ${e}`);
          }
          break;
      }
    });
  })();
});
