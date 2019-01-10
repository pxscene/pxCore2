/**
 * the markdown unit test util
 */
px.import({scene: "px:scene.1.js",
          assert: "../../test-run/assert.js"
}).then( function ready(imports)
{
  var assert = imports.assert.assert;

  module.exports = {

    /**
     * local markdown from file and return the renderer root
     * 
     * @param childScene the child scene 
     * @param mdUrl the markdown file url
     * @param testFunc the unit test function
     */
    loadMarkdown: function(childScene, mdUrl, testFunc) {
      return new Promise(function(resolve, reject) {
        childScene.url = 'mimeScene.js?url=' + mdUrl;
        childScene.ready.then(function() {
          childScene.api.mimeRenderer.ready.then(function(md) {
            setTimeout(function() {
              var root = md.getParentRoot().children[0];
              try {
                var results = testFunc({markdown:md, root, options: md.options});
                resolve(results);
              } catch(err) {
                console.log(err);
                resolve([assert(false,"Error happened, " + err)]);
              }
            },1000);
          })
        }).catch(function(err) {
          console.log(err);
          resolve([assert(false,"Error happened, " + err)]);
        });
      });
    },

    /**
     * dump rt object, console.log rtObject struct
     * 
     * @param rtObject the rt object
     * @param depth the struct depth
     */
    dumpRtObject: function(rtObject, depth = 0) {
      if(!rtObject) {
        return;
      }
      console.log(' '.repeat(depth) + rtObject.description() + (rtObject.text ? (': ' + rtObject.text) : ''));
      for(var i = 0; i < rtObject.numChildren; i ++){
        this.dumpRtObject(rtObject.children[i], depth+1);
      }
    },
  };

}).catch( function importFailed(err){
  console.error("Import failed for test_md_util.js: " + err)
});