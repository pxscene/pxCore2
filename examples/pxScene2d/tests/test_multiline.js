px.configImport({"browser:" : /*px.getPackageBaseFilePath() + */ "browser/"});

px.import({scene : 'px:scene.1.js',
           keys: 'px:tools.keys.js',
           TextInput:'browser:mleditbox.js'
}).then( function importsAreReady(imports) {

  var scene = imports.scene;
  var keys = imports.keys;
  var TextInput = imports.TextInput;
  var root = scene.root;
  var inputColor = 0x303030ff;

  var homePage = scene.create({t:"object",parent:root,draw:true});
  var browserInputBox = TextInput({pixelSize:20, value:"", w:1000,h:135,x:60, y:60,root:homePage, inputColor:inputColor,draw:true});

  function performApiCalls()
  {
    browserInputBox.setFocus();

    setTimeout(function() { console.log("Value in box is " + browserInputBox.getValue()); }, 10000);
    setTimeout(function() { browserInputBox.moveToEnd(); }, 15000);
    setTimeout(function() { browserInputBox.moveToHome(); }, 20000);
    setTimeout(function() { browserInputBox.selectAll(); console.log("Selected text is " + browserInputBox.getSelectedText()); }, 25000);
    setTimeout(function() { browserInputBox.clearSelection(); console.log("Selected text after clear is " + browserInputBox.getSelectedText()); }, 30000);
    setTimeout(function() { browserInputBox.moveToEnd(); browserInputBox.selectToHome(); console.log("Selected text to home is " + browserInputBox.getSelectedText()); }, 32000);
    setTimeout(function() { browserInputBox.clearSelection(); console.log("Selected text after clear is " + browserInputBox.getSelectedText()); }, 35000);
    setTimeout(function() { browserInputBox.moveToHome(); browserInputBox.selectToEnd(); console.log("Selected text to end is " + browserInputBox.getSelectedText()); }, 40000);
    setTimeout(function() { browserInputBox.clearSelection(); console.log("Selected text after clear is " + browserInputBox.getSelectedText()); }, 43000);

  }

  browserInputBox.ready.then(function() { console.log("input textbox is ready !!!"); performApiCalls(); }); 

}).catch( function importFailed(err){
  console.error("Import failed for multiline.js: " + err)
});

