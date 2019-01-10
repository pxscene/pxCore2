px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;

var textVal = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
var font = scene.create({t:"fontResource",url:"http://xre2-apps.cvs-a.ula.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Med.ttf"});
var pixelSize = 22;  
var bg = scene.create({t:"rect", parent:root, w:root.w, h:root.h, fillColor:0xcccccc80});
var title = scene.create({t:"text",text:"Text Alignment Test:",x:30,y:50,parent:root,font:font,pixelSize:18});
var instructions = scene.create({t:"textBox",
          text:"Displays text with all possible horizontal and vertical alignment settings.  Press T to toggle truncation on/off. Press W to toggle wrapping on/off",
          parent:root,font:font,pixelSize:18,y:70,x:30,wordWrap:true,h:30,w:root.w});

var textBoxes = [];
/* row 0, column 0 */
var testContainer = scene.create({t:"object", parent:root,x:0, y:100,w:1030,h:500});
var r0c0Parent = scene.create({t:"object", parent:testContainer, x:30, y:30, w:300,h:125});
var r0c0Rect = scene.create({t:"rect", parent:r0c0Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r0c0TextBox = scene.create({t:"textBox", parent:r0c0Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.LEFT, alignVertical:scene.alignVertical.TOP,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[0] =  r0c0TextBox;             
/* row 0, column 1 */
var r0c1Parent = scene.create({t:"object", parent:testContainer, x:380, y:30, w:300,h:125});
var r0c1Rect = scene.create({t:"rect", parent:r0c1Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r0c1TextBox = scene.create({t:"textBox",parent:r0c1Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.CENTER, alignVertical:scene.alignVertical.TOP,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[1] =  r0c1TextBox;  

/* row 0, column 2 */
var r0c2Parent = scene.create({t:"object", parent:testContainer, x:730, y:30, w:300,h:125});
var r0c2Rect = scene.create({t:"rect", parent:r0c2Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r0c2TextBox = scene.create({t:"textBox", parent:r0c2Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.RIGHT, alignVertical:scene.alignVertical.TOP,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[2] =  r0c2TextBox;  


/* row 1, column 0 */
var r1c0Parent = scene.create({t:"object", parent:testContainer, x:30, y:200, w:300,h:125});
var r1c0Rect = scene.create({t:"rect", parent:r1c0Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r1c0TextBox = scene.create({t:"textBox", parent:r1c0Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.LEFT, 
              alignVertical:scene.alignVertical.CENTER,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[3] =  r1c0TextBox;  
              
/* row 1, column 1 */
var r1c1Parent = scene.create({t:"object", parent:testContainer, x:380, y:200, w:300,h:125});
var r1c1Rect = scene.create({t:"rect", parent:r1c1Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r1c1TextBox = scene.create({t:"textBox",parent:r1c1Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.CENTER, 
              alignVertical:scene.alignVertical.CENTER,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[4] =  r1c1TextBox;  

/* row 1, column 2 */
var r1c2Parent = scene.create({t:"object", parent:testContainer, x:730, y:200, w:300,h:125});
var r1c2Rect = scene.create({t:"rect", parent:r1c2Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r1c2TextBox = scene.create({t:"textBox", parent:r1c2Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.RIGHT, 
              alignVertical:scene.alignVertical.CENTER,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[5] =  r1c2TextBox; 

/* row 2, column 0 */
var r2c0Parent = scene.create({t:"object", parent:testContainer, x:30, y:370, w:300,h:125});
var r2c0Rect = scene.create({t:"rect", parent:r2c0Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r2c0TextBox = scene.create({t:"textBox", parent:r2c0Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.LEFT, 
              alignVertical:scene.alignVertical.BOTTOM,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[6] =  r2c0TextBox;
              
/* row 2, column 1 */
var r2c1Parent = scene.create({t:"object", parent:testContainer, x:380, y:370, w:300,h:125});
var r2c1Rect = scene.create({t:"rect", parent:r2c1Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r2c1TextBox = scene.create({t:"textBox",parent:r2c1Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.CENTER, 
              alignVertical:scene.alignVertical.BOTTOM,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[7] =  r2c1TextBox;

/* row 2, column 2 */
var r2c2Parent = scene.create({t:"object", parent:testContainer, x:730, y:370, w:300,h:125});
var r2c2Rect = scene.create({t:"rect", parent:r2c2Parent, fillColor:0x000000ff,lineColor:0xFF0000FF,lineWidth:1,w:300,h:125});
var r2c2TextBox = scene.create({t:"textBox", parent:r2c2Parent, text:textVal,w:300,h:125,
              truncation:scene.truncation.NONE,
              alignHorizontal:scene.alignHorizontal.RIGHT, 
              alignVertical:scene.alignVertical.BOTTOM,
              wordWrap:false,clip:false,pixelSize:pixelSize,ellipsis:true,
              font:font});
textBoxes[8] =  r2c2TextBox;

var truncation = 0; // none
root.on("onChar", function(e) {
  console.log("got char "+e.charCode);
  if (e.charCode == 119) // w for wordWrap
  { 
    console.log("textBoxes.length= "+textBoxes.length);
    for( var i = 0; i < textBoxes.length; i++)
      textBoxes[i].wordWrap = !textBoxes[i].wordWrap;
  } 
  else if(e.charCode == 116) // t for truncation
  { 
    if( !truncation) 
      truncation = 2;
    else 
      truncation = 0;
    for( var i = 0; i < textBoxes.length; i++)
      textBoxes[i].truncation = truncation;    
  }

});

              

}).catch( function importFailed(err){
  console.error("Import failed for dynamics.js: " + err)
}); 
