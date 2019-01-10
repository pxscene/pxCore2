/**
 * Interactive test for the Receiver's Optimus API.
 */

px.import({scene:'px:scene.1.js',keys:'px:tools.keys.js',optimus:'optimus'})
    .then(imports => {

        const scene = imports.scene;
        const optimus = imports.optimus;

        const pixelSize = 24;

        let app;
        let logOnScreen;
        let clearLog;

        let log = (...args) => {
            console.log.apply(console, args);
            if (logOnScreen) {
                logOnScreen.apply(null, args);
            }
        };

        const samples = {
            "Create Receiver": () => {
                optimus.setScene(scene);
                app = optimus.createApplication({
                    id: "2", priority: 1, x: 0, y: 0, w: scene.w, h: scene.h,
                    cx: 0, cy: 0, sx: 1.0, sy: 1.0, r: 0, a: 0.8, interactive: true,
                    painting: true, clip: false, mask: false, draw: true,
                    launchParams: {"cmd":"receiver"}
                });
                log(`application created`);
                log(`wait for the window to appear`);
                log(`then you can set focus and control the Receiver`);
                log(`yet this menu remains accessible`);

            },
            "Receiver.setFocus": () => {
                app.setFocus();
                log(`setFocus called`);
            },
            "Receiver.suspend": () => {
                let ret = app.suspend();
                log(`suspend returned: ${ret}`);
            },
            "Receiver.resume": () => {
                let ret = app.resume();
                log(`resume returned: ${ret}`);
            },
            "Receiver.ready?": () => {
                app.ready.then(() => { log("ready promise resolved") });
                log(`ready called`);
            },
            "Receiver.state": () => {
                let ret = app.state();
                log(`state returned: ${ret}`);
            },
            "Receiver.getDeviceCaps": () => {
                let api = app.api();
                let ret = api.getDeviceCaps();
                log(`getDeviceCaps returned: ${ret}`);
            },
            "Receiver.getQuirks": () => {
                let api = app.api();
                let ret = api.getQuirks();
                log(`getQuirks returned: ${ret}`);
            },
            "Receiver.isFocused?": () => {
                let ret = app.isFocused();
                log(`isFocused returned: ${ret}`);
            },
            "Receiver info": () => {
                let props = {};
                let keys = [
                    'id','name','priority','type',
                    'x','y','w','h','cx','cy','sx','sy','r','a','interactive','painting','clip','mask','draw',
                    'hasApi','pid','externalApp'
                ];
                for (const key of keys) {
                    props[key] = app[key];
                }
                log(`Receiver properties are: ${JSON.stringify(props)}`);
            },
            "Clear log": () => {
                clearLog();

            },
            "Say hello": () => {
                log ("Hello!");
            },
        };

        /**
         * UI
         */
        (() => {

            function handleKeyPress(e)
            {
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
                    case imports.keys.R:
                        try {
                            samples[_allKeys[_sel]]();
                        } catch (e) {
                            log(`Failed. ${e}`);
                        }
                        break;
                }
            }

            logOnScreen = (...args) => {
                const lines = _logTextBox.text.split(/\n/).filter(i => i);
                lines.push(args.join(' '));
                _logTextBox.text = lines.join("\n");
                const m = _logTextBox.measureText().bounds.y2;
                _logTextBox.y = _logY + Math.min(0, _logHeight - m);
            };
            clearLog = () => {
                _logTextBox.text = "";
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
            _logView = scene.create({t:"rect",parent:scene.root,x:scene.w/2,y:0,w:scene.w/2,h:scene.h});
            _ptrTextBox = scene.create({t:"textBox",parent:_buttonsView,x:0,y:0,w:50,h:20,text:"->",pixelSize:pixelSize,textColor:0xffffffff});
            for (const k of _allKeys) {
                const txt = scene.create({t:"textBox",parent:_buttonsView,x:50,y:_y,w:50,h:20,text:k,pixelSize:pixelSize,textColor:0xffffffff});
                _y += txt.h;
            }
            _lineHeight = _y / _allKeys.length;
            _logHeight = scene.h;
            _logY = 0;
            _logTextBox = scene.create({t:"textBox",parent:_logView,x:0,y:_logY,w:_logView.w,h:_logHeight,text:"",wordWrap:true,pixelSize:pixelSize,textColor:0xffffffff});
            log("use Rec to select menu item");
            imports.scene.root.on("onKeyDown", e =>{handleKeyPress(e);});

            if(0) // automatic scenario
            {
                let _timer = setInterval( function sendKeyDown() {
                    if(true) {
                        handleKeyPress({keyCode: imports.keys.DOWN});
                    } else {
                        clearInterval(_timer);
                    }

                }, 1000);
            }

        })();


    });
