
px.configImport({"jar_bomb:" : px.getPackageBaseFilePath() + "/ordinance/"});

px.import({ scene: 'px:scene.1.js'
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = scene.root;

  var basePackageUri = px.getPackageBaseFilePath();

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  function testFonts(filename) // TODO add woff
  {
    var furl = basePackageUri + "/ordinance/";
    var goodFontRes = scene.create({ t: "fontResource", url:        "FreeSans.ttf" });

    goodFontRes.ready.then( function() { console.log("### CREATED: FreeSans.ttf    <<<< Good" ); },
                            function() { console.log("### FAILED:  FreeSans.ttf    <<<< BAD"  ); }   );

    var  badFontRes = scene.create({ t: "fontResource", url: furl + "DeadSans.ttf" });

    badFontRes.ready.then(  function() { console.log("### CREATED: DeadSans.ttf    <<<< BAD  ... 'DeadSans.ttf' does not exist !" ); },
                            function() { console.log("### FAILED:  DeadSans.ttf    <<<< Good ... 'DeadSans.ttf' does not exist !" ); }   );

    // var  woffRes = scene.create({ t: "fontResource", url: furl + "OpenSans.woff" });
    // var     textWoff = scene.create({t:"textBox", text: "OpenSans.woff", parent: root, pixelSize: 50, w: 800, h: 80, x: 200, y: 200,
    //                 alignHorizontal: scene.alignHorizontal.CENTER, interactive: false, font: woffRes,
    //                 alignVertical:     scene.alignVertical.CENTER, textColor: 0x00ff00FF, a: 1.0});

    var  ttfRes = scene.create({ t: "fontResource", url: furl + "OpenSans.ttf" });
    var     textWoff = scene.create({t:"textBox", text: "OpenSans.woff (ttf)" , parent: root, pixelSize: 50, w: 800, h: 80, x: 200, y: 270,
                    alignHorizontal: scene.alignHorizontal.CENTER, interactive: false, font: ttfRes,
                    alignVertical:     scene.alignVertical.CENTER, textColor: 0x000000FF, a: 1.0});
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //
  function testPNGs(filename)
    {
    var badPNGs = [ "xc1n0g08.png", "xcrn0g04.png", "xd0n2c08.png", "xd9n2c08.png", "xhdn0g08.png", "xs1n0g01.png", "xs4n0g01.png",
                    "xc9n2c08.png", "xcsn0g01.png", "xd3n2c08.png", "xdtn0g01.png", "xlfn0g04.png", "xs2n0g01.png", "xs7n0g01.png"];

    // Test PngSuite ...  http://www.schaik.com/pngsuite/  ... corrupted file examples. NONE should load
    //
    var img_ready = badPNGs.map( name => {

        var url = basePackageUri + "/ordinance/PngSuite/" + name;
        var img = scene.create({t:"image",url:url,parent:root});

        img.ready.then( function() { console.log("### CREATED: " + name + " <<<< BAD   ... file is deliberately damaged."); },
                        function() { console.log("### FAILED:  " + name + " <<<< Good  ... file is deliberately damaged."); }
        );
    });
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  function testJPGs(filename)
  {
    var badJPGs = [ "Bad_Components.jpg", "Bad_Depth.jpg", "Bad_Length.jpg", "Bad_WxH.jpg", "Good.jpg", "Missing.jpg"];

    // Test JPG's corrupted file examples.
    //
    var img_ready = badJPGs.map( name => {

          var url = basePackageUri + "/ordinance/jpg_test/" + name;
          var img = scene.create({t:"image",url:url,parent:root});

          img.ready.then
          ( 
            function()
            { 
              if(name == 'Good.jpg') 
                console.log("### CREATED:  " + name + " <<<< Good  ... file is NOT damaged.");
              else
                console.log("### CREATED: " + name + " <<<< BAD   ... file is deliberately damaged."); 
            },
            function()
            {
              console.log("### FAILED:  " + name + " <<<< Good  ... file is deliberately damaged.");
            }
          );
    });
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  function testLoadZIPs()
  {
    // Test ZIP's corrupted file examples.
    //
    // NOTE:  42.zip == 42.jar  <<< Is a 'zipbomb'   https://en.wikipedia.org/wiki/Zip_bomb
    //
    // 42.zip, 42.jar and Good.zip should open and provide fileNames
    //
    var badZIPs = [ "42.zip", "42.jar", "just_a_txt_file_renamed.zip", "just_a_txt_file_renamed.jar", "Good.zip", "Missing.zip"];

    var zips_ready = badZIPs.map( name =>
    {
      var url          = basePackageUri + "/ordinance/zip_test/" + name;
      var zip_loadFile = scene.loadArchive(url);

      if(zip_loadFile === null || zip_loadFile === undefined)
      {
        console.log("\n###\n### FATAL: loadArchive() ... Failed to open - 'undefined' > "      + name );
      }
      else
      {
        zip_loadFile.ready.then(function(data) // SUCCESS
        {
          console.log("\n");

          if(name == 'Good.zip') 
            console.log("### LOADED: " + name + " <<<< Good  ... file is NOT damaged.");
          else
            console.log("### LOADED: " + name + " <<<< BAD   ... file is deliberately damaged."); 

//                var foo = zip_loadArchive.getFileAsString("foo/test space.zip");

//                console.log("### LOADED:   data    object: [ "+ JSON.stringify(data) +" ]");
              console.log("### LOADED:   data    fileNames:  [ "+ data.fileNames +" ]");
              console.log("### LOADED:   data    loadStatus: [ "+ JSON.stringify(data.loadStatus) +" ]");
        },
        function() // REJECT
        {
          console.log("### LOAD FAILED:  " + name + " <<<< Good  ... file is deliberately damaged.");
        });
      }
    }); // MAP
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  function testGetZIPs(filename)
  {
    // Test ZIP's corrupted file examples.
    //
    // NOTE:  42.zip == 42.jar  <<< Is a 'zipbomb'   https://en.wikipedia.org/wiki/Zip_bomb
    //
    var badZIPs = [ "42.zip", "42.jar", "just_a_txt_file_renamed.zip", "just_a_txt_file_renamed.jar", "Good.zip", "Missing.zip"];

    var zips_ready = badZIPs.map( name =>
    {
      var url         = basePackageUri + "/ordinance/zip_test/" + name;
      var zip_getFile = px.getFile(url);

      if(zip_getFile === null || zip_getFile === undefined)
      {
        console.log("\n###\n### FATAL: getFile() ... Failed to open - 'undefined' > "      + name );
      }
      else
      {
        zip_getFile.then(function(data) // SUCCESS
        {
          console.log("\n");

          if(name == 'Good.zip') 
            console.log("### GOT FILE: " + name + " <<<< Good  ... file is NOT damaged.");
          else
            console.log("### GOT FILE: " + name + " <<<< BAD   ... file is deliberately damaged."); 

            // console.log("### GOT FILE:   data    object: [ "+ JSON.stringify(data) +" ]");
        },
        function() // REJECT
        {
          console.log("### GET FAILED:  " + name + " <<<< Good  ... file is deliberately damaged.");
        });
      }
    }); // MAP
  }

   ////////////////////////////////////////////////////////////////////////////////////////////////////

  testFonts();
  testPNGs();
  testJPGs();
  testGetZIPs();
  testLoadZIPs();

 ////////////////////////////////////////////////////////////////////////////////////////////////////

 function testArchiveFile(filename)
 {
    var url = basePackageUri + "/" + filename;
 
    // - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - - 
    //
    // Using `getFile()' - byte access to zipped archive data.  Not contents
    //

    console.log("\n\n############################################################");
    console.log(    "###");
    console.log(    "###  Trying  'getFile()' ");
    console.log(    "###\n");

    var zip_getFile = px.getFile(url);

    if(zip_getFile === null || zip_getFile === undefined)
    {
      console.log("\n###\n### FATAL: getFile() ... Failed to open - 'undefined' > "      + url + "\n###\n###");
    }
    else
    {
      zip_getFile.then(function(data)
      {
        console.log(" #   zip_getFile   length: [ "+data.length +" ]\n");
        console.log(" #   zip_getFile  data[0]: [ "+data[0] +" ]");
        console.log(" #   zip_getFile  data[1]: [ "+data[1] +" ]\n");
      });
    }

    // - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - - 
    //
    // Using `loadArchive()' - filenames are visible... but unsure how to unzip / access files
    //
    console.log("\n\n############################################################");
    console.log(    "###");
    console.log(    "###  Trying  'loadArchive()' ");
    console.log(    "###\n");

    var zip_loadArchive = scene.loadArchive(url);

    if(zip_loadArchive === null || zip_loadArchive === undefined)
    {
      console.log("\n###\n### FATAL: loadArchive() ... Failed to open - 'undefined' > "      + url + "\n###\n###");
    //  return;
    }
    else
    {
      zip_loadArchive.ready.then(function(data)
      {
        console.log(" #   zip_loadArchive       object: [ "+ JSON.stringify(data) +" ]\n");
        console.log(" #   zip_loadArchive   loadStatus: [ "+ JSON.stringify(zip_loadArchive.loadStatus) +" ]");
        console.log(" #   zip_loadArchive    fileNames: [ "+ zip_loadArchive.fileNames +" ]");

     //   var foo = zip_loadArchive.getFileContents("lib 0.zip");
        var foo = zip_loadArchive.getFileAsString("foo/test space.zip");
        console.log(" #   zip_loadArchive       foo: [ "+ JSON.stringify(foo) +" ]\n");
      });
    }
    // - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - -  - - 

    return zip_getModule;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////

}).catch( function importFailed(err){
  console.error("FATAL: importFailed() >> Import failed for test_bomb.js: " + err);
});

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////}
