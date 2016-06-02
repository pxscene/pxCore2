px.import("px:scene.1.js").then(function(scene) {


  function testLoadArchive(u,f) {
    scene.loadArchive(u)
      .ready.then(function(a) {
        console.log("---------------------------");
        console.log("");
        console.log("loadArchive succeeded for (",u,").");
        console.log("files: ", a.fileNames);
        console.log("file data: >>>>>>");
        console.log(a.getFileAsString(f));
        console.log("<<<<<<");
        console.log("loadStatus:", a.loadStatus);
      }, function(a){
        console.log("XXXXXXXXXXXXXXXXXXXXXXXXXXX");
        console.log("");
        console.log("loadArchive failed for (",u,").");
      });
  }

  // Takes a few seconds to time out
  //testLoadArchive("http://abc.abc.abc","");
  testLoadArchive("fail","");
  testLoadArchive("http://localhost/~johnrobinson/fail","");
  testLoadArchive("start.sh","");
  testLoadArchive("gallery.zip","hello.js");

  testLoadArchive("http://localhost/~johnrobinson/index.html","");
  testLoadArchive("http://localhost/~johnrobinson/gallery.zip","hello.js");

});
