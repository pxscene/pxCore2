"use strict";


// These override the built-in resolve and load with
// the exact same semantics, but with logging added.

// Set up our app-specefic require hooks.
//var modResolve = Duktape.modResolve;
//Duktape.modResolve = function (id) {
//  print("modResolve", this.id, id);
//  return modResolve.call(this, id);
//};

//var modLoad = Duktape.modLoad;
//Duktape.modLoad = function () {
//  print("modLoad", this.id);
//  return modLoad.call(this);
//};

//var modCompile = Duktape.modCompile;
//Duktape.modCompile = function (code) {
//  print("modCompile", this.id, code.length);
//  return modCompile.call(this, code);
//};

//// Test require
//var p = require('./modules/utils.js').prettyPrint;

//p("B", require('./b.js'));

//p("THIS", this);

global.Promise = require('bluebird.js');

global.constructPromise = function (obj) {
    return new Promise(function (resolve, reject) {
        obj.setResolve(reject);
        //obj.setReject(reject);
    });
}

global.constructPromise2 = function (obj) {
    return Promise.fromCallback(function (callback) {
        obj.setResolve(callback);
    })
}

Promise.setScheduler(function (fn) { // fn is what to execute
    var timer = uv.new_timer.call({});
    uv.timer_start(timer, 0, 0, fn); // add the function as a callback to the timer
});

function sleep(milliseconds) {
    var start = new Date().getTime();
    for (var i = 0; i < 1e7; i++) {
        if ((new Date().getTime() - start) > milliseconds) {
            return milliseconds;
        }
    }
}

global.mysleep = sleep;

global.printres = function (res) {
    print(res)
    return res;
}

global.printerror = function (res) {
    print("error")
}


var zz = require('zz.js');
var vz1 = new zz({});
print(vz1);
print(vz1.x);

//function func1(params) {
//    this._x = 5;
//    Object.defineProperty(this, "x",
//    {
//        set: function (val) { this._x = val; },
//        get: function () { return this._x; },
//    });
//    return this;
//}

//var v1 = func1({});
//print(v1.x);


//function func() {
//    var c = myreturn_promise2();
//    print(c);

//    c.then(function (s) {
//        print("<promise result>")
//        //c.setResolve(function (obj) {});
//    })

//    //.catch(function (err) {
//    //    print("<promise error>")
//    //})

//    return c;
//}

//var c = func();

//Promise.all([c]).then(function (obj) {
//    print("Promise.all")
//    print(obj)
//})

//print(myreturn_array());

//var msleep1 = sleep;

//myin_object(msleep1);

//var c = myreturn_promise(1500);
//c.then(function (res) {
//    print(res*2)
//});

//var path = require('path.js');
//print(path.normalize('C:\\temp\\\\foo\\bar\\..\\'));

//var qs = require('querystring.js');
////var c = qs.parse('foo=bar&abc=xyz&abc=123');
//var c = qs.parse('w=JavaScript_%D1%88%D0%B5%D0%BB%D0%BB%D1%8B&foo=bar');
//print(c.foo);
//print(c.w);

//var url = require('url.js');
//print(url.resolve('/one/two/three', 'four'));
//print(url.resolve('http://example.com/', '/one'));
//print(url.resolve('http://example.com/one', '/two'));

//var p = url.parse('https://nodejs.org:9090/api/url.html?x=2&z=3');
//print(p.protocol);
//print(p.host);
//print(p.port);
//print(p.query);
//print(p.path);

//var mysleep_var = Promise.method(mysleep);
//var d = mysleep_var(1000);
//d.then(printres);

//var p = require('modules/utils.js').prettyPrint;

//var vm = require('vm.js');
//var funccode = '(function (px, module, __filename, __dirname) {\n'
//+ 'print(px);\n'
//+ 'print(module);\n'
//+ 'module.z = 2;\n'
//+ 'print(__filename);\n'
//+ 'print(__dirname);\n'
//+ '});';
//var sandbox = { x: 123, y: 256  };
//var options = { z: 123, w: 256 };
//var px = {};
//var xModule = {};
//var filename = 'filename';
//var dirname = 'dirname';
//vm.runInNewContext(funccode, sandbox, options, px, xModule, filename, dirname);
//print("OK")
//print(moduleFunc);

//var vvv = { x: 123, y: 256 };
//print(vvv);
//Object.keys(vvv).map(function (key) {
//    global.key = vvv[key];
//});

//var fs = require('fs.js');
//var data = fs.readFile('fs.js', function (err, data) {
//    //print(err);
//    //p(data);
//    print(data);
//});


//mysleep_var(1000).then(printres).catch(printerror);

//Promise.all([mysleep_var(1000)]).then(printres).catch(printerror);

//print(uv.fs_stat("test-argv.js").mtime);
//p(global);
//p(Duktape);
//print(Duktape.modLoaded);



