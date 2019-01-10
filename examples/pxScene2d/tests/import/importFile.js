px.import({scene:"px:scene.1.js"}).then( function ready(imports) {
var scene = imports.scene;
var inputRes = scene.create({t:"imageResource", id:"imageRes", url:px.getBaseFilePath()+"images/status_bg.png"});

function TextInput()
{

}

TextInput.image = inputRes;
module.exports = TextInput;

}).catch(function importFailed(err){
    console.error("Import failed for importFile.js: " + err);
});
