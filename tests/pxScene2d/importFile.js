px.import({scene:"px:scene.1.js"}).then( function ready(imports) {
var scene = imports.scene;
var inputRes = scene.create({t:"imageResource", id:"imageRes", url:px.getBaseFilePath()+"supportfiles/input2.png"});
function TextInput()
{

}

module.exports = TextInput;

}).catch(function importFailed(err){
    console.error("Import failed for importFile.js: " + err);
});
