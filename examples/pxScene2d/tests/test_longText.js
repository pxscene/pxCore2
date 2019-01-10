px.import({scene:"px:scene.1.js",
assert:"../test-run/assert.js",
manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

  var scene = imports.scene;
  var root = imports.scene.root;
  var assert = imports.assert.assert;
  var manual = imports.manual;

  var manualTest = manual.getManualTestValue();

  var basePackageUri = px.getPackageBaseFilePath();
  
  var array_objects = [];
  var test = [];
  var fonts = [
               "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-BoldItalic.ttf",
              ];
  
  var x_axis=0;
  var y_axis=0;
  var txt = "Welcome To Pxscene World!";
  txt_obj = scene.create({t:"text",x:x_axis,y:50,textColor:0xff0880ff,text:txt.repeat(10000),fontUrl:fonts[0],parent:root,pixelSize:20});


  var textBox_obj = scene.create({t:"textBox",x:20,w:1200,h:600,y:y_axis+200,wordWrap:true,textColor:0xff0880ff,text:txt.repeat(10000),fontUrl:fonts[0],parent:root,pixelSize:20});


  var tests = {

    test_text: function() {
  
    
      return new Promise(function(resolve, reject) {
        
        var results = [];
        txt_obj.font.ready.then(function(){
          metrics = txt_obj.font.getFontMetrics(20);
          metrics1 = txt_obj.font.measureText(20,txt.repeat(10000));
          var string = "Font Metrics\n" + "Height:" + metrics.height + "\nAscent:" + metrics.ascent + "\nDescent:" + metrics.descent + "\nNaturalLeading:" + metrics.naturalLeading + "\nBaseline:" + metrics.baseline;
          scene.create({t:"text",x:20,y:40,text:string,parent:root})
          var string1 = "Text Measurement\n" + "Height:" + metrics1.h + "\nWidth:" + metrics1.w;
          scene.create({t:"text",x:180,y:40,text:string1,parent:root});

          results.push(assert((metrics1.w == 2690000), 'text width does not match expected value. actual: '+metrics1.w+'; expected: 2690000'));          
        },function(){
          results.push(assert(false, 'promise for text was rejected!'));
          root.removeAll();
          var Error = "Could not Load Text Object \"" + fonts[0] + "\"";
          scene.create({t:"text",text:Error,parent:root})
        }).then(function() {
          resolve(results);
        });
      });
  },
  test_textBox: function() {
  
    
    return new Promise(function(resolve, reject) {
      
      var results = [];
      textBox_obj.font.ready.then(function(){
        console.log("Got promise for textBox");
        results.push(assert(true, 'promise for textBox was received!'));
      },function(){
        results.push(assert(false, 'promise for textBox was rejected!'));
        root.removeAll();
        var Error = "Could not Load Text Object \"" + fonts[0] + "\"";
        scene.create({t:"text",text:Error,parent:root})
      }).then(function() {
        resolve(results);
      });
    });
  }
}
  module.exports.tests = tests;

  if(manualTest === true) {

    manual.runTestsManually(tests);

  }

  }).catch( function importFailed(err){
    console.error("Import failed for test_longText.js: " + err)
  });