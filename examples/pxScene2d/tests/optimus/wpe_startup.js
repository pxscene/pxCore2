
px.import({ scene: 'px:scene.1.js', keys:      'px:tools.keys.js',
             Optimus: 'optimus.js' }).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var optimus = imports.Optimus;
  var root  = imports.scene.root;
  var receiverApp;
  var browserApp;
  var launchUrl = "http://www.example.com";
  var browserBg;

  //this is needed to prevent black background
  module.exports.wantsClearscreen = function()
  {
    return false;
  };

  scene.addServiceProvider(function(serviceName, serviceCtx){                   
    if (serviceName === ".optimus")
    {
      return optimus;
    }
    if (serviceName === ".startupTest")
    {
      return {testMethod:function(){console.log("test function");}};
    }
    return "deny";          
  });

  var RECEIVER_APP_ID = "1001";
  var nextAppId = 1002;
  var BROWSER_APP_ID;

  optimus.setScene(scene);

  browserBg = scene.create({ t: "rect", parent: root, fillColor: 0x000000ff, x: 0, y: 0, w:scene.getWidth(), h:scene.getHeight() });
  browserBg.a = 0;

  var receiverAppProps = { id:RECEIVER_APP_ID, priority:1, x:0, y:0, w:scene.getWidth(), h:scene.getHeight(), cx:0, cy:0, sx:1.0, sy:1.0, r:0, a:1, interactive:true, painting:true, clip:false, mask:false, draw:true, launchParams:{ "cmd":"receiver" } };

  receiverApp = optimus.createApplication(receiverAppProps);
  optimus.getApplicationById(RECEIVER_APP_ID).setFocus();

  function showReceiver() {
    browserBg.a = 0;
    browserApp.a = 0;
    receiverApp.a = 1;
  }

  function showBrowser() {
    receiverApp.a = 0;
    browserBg.a = 1;
    browserApp.a = 1;
  }

  function destroyBrowser() {
    optimus.getApplicationById(BROWSER_APP_ID).destroy();
    browserApp = undefined;
    browserBg.a = 0;
  }

  function launchWebPage(url) {
    console.log("launching url" + url);
    if (browserApp == undefined)
    { 
      launchUrl = url;
      BROWSER_APP_ID = nextAppId;
        var browserAppProps = { id:BROWSER_APP_ID, priority:1, x:0, y:0, w:scene.getWidth(), h:scene.getHeight(), cx:0, cy:0, sx:1, sy:1, r:0, a:1, interactive:true, painting:true, clip:false, mask:false, draw:true, launchParams:{ "cmd":"WebApp", "uri": launchUrl } };
      browserApp = optimus.createApplication(browserAppProps);
      nextAppId++;
    }
    else 
    {
      browserApp.api().url = url;
    }
    optimus.getApplicationById(BROWSER_APP_ID).setFocus();
  }

  scene.root.on("onPreKeyDown", function(e) {
      if (e.keyCode == keys.ONE)  
      {
        launchWebPage("https://slashdot.org");
        showBrowser();
      }
      else if (e.keyCode == keys.TWO)
      {
        launchWebPage("https://www.wired.com");
        showBrowser();
      }
      else if (e.keyCode == keys.THREE)
      {
        launchWebPage("https://www.cnn.com");
        showBrowser();
      }
      else if (e.keyCode == keys.FOUR)
      {
        launchWebPage("http://www.ebizmba.com");
        showBrowser();
      }
      else if (e.keyCode == keys.FIVE)
      {
        launchWebPage("https://www.google.com");
        showBrowser();
      }
      else if (e.keyCode == keys.EIGHT)
      {
        optimus.getApplicationById(RECEIVER_APP_ID).setFocus(true);
        showReceiver();
      }
      else if (e.keyCode == keys.NINE)
      {
        optimus.getApplicationById(BROWSER_APP_ID).setFocus(true);
        showBrowser();
      }
      else if (e.keyCode == keys.ZERO)
      {
        optimus.getApplicationById(RECEIVER_APP_ID).setFocus(true);
        showReceiver();  
        destroyBrowser();
      }
    });
}).catch( function importFailed(err){
  console.error("Import failed for startup.js: " + err);
});
