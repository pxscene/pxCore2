px.import({ scene: 'px:scene.1.js',
             keys: 'px:tools.keys.js'
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys = imports.keys;
  var root = imports.scene.root;

  var pts      = 24;
  var bg       = scene.create({t:"image",url:"./images/status_bg.png",parent:root,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH});
  var fontRes  = scene.create({t:"fontResource",url:"FreeSans.ttf"});
  var inputRes = scene.create({t:"imageResource",url:"./images/input2.png"});
  var inputBg  = scene.create({t:"image9",resource:inputRes,a:0.9,x:10,y:10,w:400,insetLeft:10,insetRight:10,insetTop:10,insetBottom:10,parent:bg});
  var spinner  = scene.create({t:"image",url:"./images/spinningball2.png",cx:50,cy:50,y:-30,parent:inputBg,sx:0.3,sy:0.3,a:0});
  var prompt   = scene.create({t:"text",text:"Enter Url to JS File or Package",font:fontRes, parent:inputBg,pixelSize:pts,textColor:0x869CB2ff,x:10,y:2});
  var url      = scene.create({t:"text",text:"",font:fontRes, parent:inputBg,pixelSize:pts,textColor:0x303030ff,x:10,y:2});
  var cursor   = scene.create({t:"rect", w:2, h:inputBg.h-10, parent:inputBg,x:10,y:5});

  spinner.animateTo({r:360},1.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,scene.animation.COUNT_FOREVER);
  cursor.animateTo({a:0},0.5,   scene.animation.TWEEN_LINEAR,scene.animation.OPTION_OSCILLATE,scene.animation.COUNT_FOREVER);

  var contentBG = scene.create({t:"rect", x:10,y:60,parent:bg,fillColor:0xffffffff,a:0.05,draw:false});
  var content   = scene.create({t:"scene",x:10,y:60,parent:bg,clip:true});

  var cursor_pos = 0;

  var selection       = scene.create({t:"rect", w:2, h:inputBg.h-10, parent:inputBg, fillColor:0xFCF2A488, x:10,y:5});
  var selection_x     = 0;
  var selection_start = 0;
  var selection_chars = 0; // number of characters selected  (-)ive is LEFT of cursor start position
  var selection_text  = "";

  url.moveToFront();

  inputBg.on("onChar",function(e)
  {
    if (e.charCode == keys.ENTER)  // <<<  ENTER KEY
      return;

    if(selection_chars != 0)
    {
      removeSelection(); // overwrote selection
    }
    // TODO should we be getting an onChar event for backspace
    if (e.charCode != keys.BACKSPACE)
    {
      if(cursor_pos == 0)
      {
        // append character
        url.text += String.fromCharCode(e.charCode);
      }
      else
      {
        // insert character
        url.text = url.text.slice(0, cursor_pos) + String.fromCharCode(e.charCode) + url.text.slice(cursor_pos);
      }
      prompt.a = (url.text)?0:1; // hide placeholder

      cursor_pos += 1; // inserted 1 character

      updateCursor(cursor_pos);
    }
});


function reload(u) {

  spinner.a = 1;
  if (!u)
    u = url.text;
  else
    url.text = u;

  // TODO Temporary hack
  if (u.indexOf(':') == -1)
    u = 'http://www.pxscene.org/examples/px-reference/gallery/' + u;

  content.url = u;
  //scene.setFocus(content);
  content.focus = true;
  if (true)
  {
    content.ready.then(
      function(o) {
        spinner.a = 0;
        spinner.r = 0;
        console.log(o);
        contentBG.draw = true;
      },
      function() {
        spinner.a = 0;
        spinner.r = 0;
        contentBG.draw = false;
      }
    );
  }
}

inputBg.on("onKeyDown", function(e)
{
  var code  = e.keyCode;
  var flags = e.flags;

  switch(code)
  {
    case keys.BACKSPACE:
      {
        console.log("BACKSPACE " + url.text);

        var s = url.text.slice();

        if(selection_chars == 0)
        {
           var before_cursor = s.slice(0,cursor_pos-1);
           var  after_cursor = s.slice(cursor_pos);

           url.text = before_cursor + after_cursor;

           if(cursor_pos > 0) cursor_pos--;
        }
        else
        {
           removeSelection();  // Delete selection
        }

        updateCursor(cursor_pos);
      }
      break;

    case keys.ENTER:
      reload();
      break;

    case keys.LEFT:
      if(cursor_pos > 0)
      {
        if(flags == 8) // <<  SHIFT KEY
        {
          if(selection_chars == 0) // New selection ?
          {
             selection_start = cursor_pos;
             selection_x     = cursor.x + cursor.w; // Start selection
          }
          selection_chars--;

          makeSelection();
        }

        cursor_pos--;

        updateCursor(cursor_pos);
      }

      if(flags != 8 && selection.w != 0)
      {
        clearSelection();
      }
      break;

    case keys.RIGHT:
      if(cursor_pos < url.text.length)
      {
        if(flags == 8) // <<  SHIFT KEY
        {
          if(selection_chars == 0) // New selection ?
          {
            selection_start = cursor_pos - 1;
            selection_x     = cursor.x + cursor.w; // Start selection
          }
          selection_chars++;

          makeSelection();
        }

        cursor_pos++;

        updateCursor(cursor_pos);
      }

      if(flags != 8 && selection.w != 0)
      {
        clearSelection();
      }
      break;

     case keys.C:   // << CTRL + "c"
      if( ((flags & 16)==16) )  // ctrl Pressed also
      {
         console.log("onKeyDown ....   CTRL-C >>> [" + selection_text + "]");

         scene.clipboardSet('PX_CLIP_STRING', selection_text);
      }
    break;

    case keys.V:   // << CTRL + "v"
      if( ((flags & 16)==16) )  // ctrl Pressed also
      {
        // On PASTE ... access the Native CLIPBOARD and GET the top!   fancy.js
        //
        var fromClip = scene.clipboardGet('PX_CLIP_STRING'); // TODO ... pass TYPE of clip to get.

        console.log("onKeyDown ....   CTRL-V >>> [" + fromClip + "]");

        url.text = url.text.slice(0, cursor_pos) + fromClip + url.text.slice(cursor_pos);

        prompt.a  = (url.text)?0:1;
        cursor.x  = url.x + url.w;

        cursor_pos+= fromClip.length;

        updateCursor(cursor_pos);

        clearSelection();
      }
      break;

    case keys.X:   // << CTRL + "x"
      if( ((flags & 16)==16) )  // ctrl Pressed also
      {
        // On CUT ... access the Native CLIPBOARD and GET the top!   fancy.js
        //
        console.log("onKeyDown ....   CTRL-X >>> [" + selection_text + "]");
        scene.clipboardSet('PX_CLIP_STRING', selection_text);

        removeSelection();
      }
      break;

      case 0: // zero value
       break; // only a modifer key ? Ignore

      default:
        // prompt.a = (url.text)?0:1;
        // cursor.x = url.x + url.w;
        break;
  } // SWITCH
});


function updateCursor(pos)
{
  var       s = url.text.slice();
  var    snip = s.slice(0, pos); // measure characters to the left of cursor
  var metrics = fontRes.measureText(pts, snip);

  cursor.x = url.x + metrics.w; // offset to cursor

  console.log("updateCursor() >>> pos = " + pos);
}

function clearSelection()
{
  selection_text  = "";
  selection_start = 0;
  selection_chars = 0;

  selection.x = 0;
  selection.w = 0;
}

function removeSelection()
{
  url.text = url.text.replace(selection_text,'');

  if(selection_chars > 0)
  {
    cursor_pos -= selection_text.length;
  }

  updateCursor(cursor_pos);
  clearSelection();
}

function makeSelection()  // Selection made: left-to-right
{
  var start = selection_start + 1;
  var end   = selection_chars + start;

  if(selection_chars < 0) // Selection made: right-to-left
  {
    end   = start - 1;  // original start is end .. left-to-right
    start = start + selection_chars - 1;
  }

  var          s = url.text.slice();
  selection_text = s.slice(start, end); // measure characters up to cursor
  var metrics = fontRes.measureText(pts, selection_text);

  console.log("makeSelection() >>>  s: "+start+"  e: "+end+" selection_text = [" + selection_text + "]");

  selection.x = selection_x;
  selection.w = metrics.w;

  if(selection_chars < 0) // selecting towards LEFT
  {
    selection.x -= metrics.w;
  }
}

inputBg.on("onFocus", function(e) {
  cursor.draw = true;
  clearSelection();

  cursor_pos = url.text.length;
  updateCursor(cursor_pos);
});

inputBg.on("onBlur", function(e) {
  cursor.draw = false;
});

inputBg.on("onMouseUp", function(e) {
  inputBg.focus = true;
});

content.on("onMouseUp", function(e) {
  content.focus=true;
});

function updateSize(w,h) {
  bg.w = w;
  bg.h = h;
  inputBg.w = w-20;
  spinner.x = w-100;
  content.w = w-20;
  content.h = h-70;
  contentBG.w = w-20;
  contentBG.h = h-70;
}

scene.root.on("onPreKeyDown", function(e) {
  if (e.keyCode == keys.L && e.flags == 16) { // ctrl-l
    //console.log("api:"+content.api);
//    if (content.api) content.api.test(32);
    //scene.setFocus(inputBg);
    inputBg.focus = true;
    url.text = "";
    prompt.a = (url.text)?0:1;
    cursor.x = 10;
    e.stopPropagation();
  }
});


if (true) {
  scene.root.on("onKeyDown", function(e) {
	  var code = e.keyCode; var flags = e.flags;
    console.log("onKeyDown browser.js:", code, ", ", flags);
    if (code == keys.R && ((flags & 48) == 48)) {  // ctrl-alt-r
      console.log("Browser.js Reloading");
      reload();
      e.stopPropagation();
      console.log("Browser.js reload done");
    }
    else if (code == keys.H && ((flags & 48)==48)) {  // ctrl-alt-h
      var homeURL = "browser.js";
      console.log("browser.js Loading home");
      reload("gallery.js");
      e.stopPropagation();
    }
  });
}



scene.on("onResize", function(e) { updateSize(e.w,e.h); });
updateSize(scene.w,scene.h);

//scene.setFocus(inputBg);
inputBg.focus = true;
}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err)
});

