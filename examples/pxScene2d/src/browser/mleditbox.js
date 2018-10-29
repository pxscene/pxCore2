/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

px.import({scene:"px:scene.1.js",keys:'px:tools.keys.js'})
.then( function ready(imports)
{
  var scene = imports.scene;
  var root = scene.root;
  var keys = imports.keys;

  var nolines = 1;
  var totlines = 20;
  /* point to current active line where text operations are performed */
  var curline = 0;

  var linemap = {};


  /* initialize line specific data */
  function initializeLine(index)
  {
    if (linemap[index] == undefined)
    {
      linemap[index] = {};
      linemap[index].x = 0;
      linemap[index].y = 0;
      linemap[index].width = 0;
      linemap[index].h = 0;
      linemap[index].length = 0;
      linemap[index].text = "";
    }
  }

  /* detect last active line and cursor pos before selection */
  var lastactiveline = -1;
  var lastactivepos = -1;

  var TextInput = function(config)
  {
    /* populate input configuration data */
    var inputConfig = config;
    var pts         = config.pixelSize;
    var fontRes     = config.font;

    if( config.font === undefined) {
      fontRes  = scene.create({t:"fontResource",url:"FreeSans.ttf"});
    }
    var inputWidth = config.w;
    var inputHeight = config.h;
    if( config.h === undefined) {
      inputHeight = 40;
    }
    var x = config.x;
    var y = config.y;
    var value = config.value;
    
    var inputColor = config.inputColor;
    if( inputColor === undefined) {
      inputColor = 0x303030ff;
    }

    var buttonDown = false;
    var selectionColor = config.selectionColor;
    if( selectionColor === undefined) {
      selectionColor = 0xFCF2A488;
    }

    var firstShiftDown = true;
    var firstShiftUp = true;
    var firstShiftRight = true;
    var firstShiftLeft = true;
    var movingRight = false;
    var movingLeft = false;

    var updateStartPos = false;

    var underSelection = false;
    var selection   = [];
    var selectedText = "";

    var cursor_pos = 0;

    /* elements */
    var inputRes = scene.create({t:"imageResource",url:"browser/images/input2.png"});
    var inputBg  = scene.create({t:"image9",resource:inputRes,a:0,x:x,y:y,insetLeft:10,insetRight:10,insetTop:10,insetBottom:10,parent:config.root,h:inputHeight*3});
    var textinput = scene.create({t:"textBox", parent: inputBg, textColor: 0x202020ff,
                                      x: 10, y: 0,  w: 1280, h: inputBg.h, a: 1.0,
                                      font: fontRes, pixelSize: pts, wordWrap: true,
                                      interactive: true,
                                      alignHorizontal: scene.alignHorizontal.LEFT,
                                      alignVertical: scene.alignVertical.TOP
                                      });

    var cursor = scene.create({t:"rect", w:2, h:inputHeight-(inputHeight/2), fillColor:0xFF0000FF, parent:textinput,x:0,y:8,a:0,h:0});
    /* selection rect for all the lines */
    for (var i=0; i<totlines; i++)
    {
      selection[i]   = scene.create({ t: "rect", w: 0, parent: textinput, fillColor: selectionColor, x: 0, y: 0, a: 0 });
    }

    var objsReady = [];
    objsReady.push(inputRes.ready);
    objsReady.push(fontRes.ready);
    objsReady.push(inputBg.ready);
    objsReady.push(textinput.ready);
    objsReady.push(cursor.ready);
    for (var i=0; i<totlines; i++)
    {
      objsReady.push(selection[i].ready);
    }

    var getSelectedText = function() {
      selectedText = "";
      for (var i=0; i<nolines; i++)
      {
        if (selection[i].a == 1)
        {
          selectedText += linemap[i].text.substring(selection[i].startPos, selection[i].endPos);
        }
      }
      return selectedText;
    }

    var removeSelectedText = function() {
        for (var i=0; i<nolines; i++)
        {
          if (selection[i].a == 1)
          {
            var modifiedText = linemap[i].text.substring(0, selection[i].startPos) + linemap[i].text.substring(selection[i].endPos);  
            linemap[i].text = modifiedText;
            linemap[i].length = modifiedText.length;
            var measure = fontRes.measureText(pts, modifiedText);
            linemap[i].width = measure.w;
            linemap[i].h = measure.h;
          }
        }
     }

     var reArrangeText = function() {
         for (var i=0; i<nolines-1; i++)
         {
           if (selection[i].a == 1)
           {
             if (linemap[i].width < (inputWidth-30))
             {
               var moveUpLength = selection[i].endPos - selection[i].startPos;
               var nextLineData = linemap[i+1].text;
               var moveUpText = nextLineData.substring(0, moveUpLength);
               linemap[i].text += moveUpText;
               linemap[i+1].text = nextLineData.substring(moveUpLength);
               var measure = fontRes.measureText(pts, linemap[i].text);
               linemap[i].length = linemap[i].text.length;
               linemap[i].width = measure.w;
               linemap[i].h = measure.h;
               measure = fontRes.measureText(pts, linemap[i+1].text);
               linemap[i+1].length = linemap[i+1].text.length;
               linemap[i+1].width = measure.w;
               linemap[i+1].h = measure.h;
             }
           }
         }
         textinput.text = ""; 
         for (var i=0; i<nolines; i++)
         {
           if(linemap[i].text.length == 0)
             nolines--;
         }
         for (var i=0; i<nolines-1; i++)
         {
           textinput.text += linemap[i].text;
           if (linemap[i].text.substring(linemap[i].text.length-1) != "\n")
             textinput.text += "\n";
         }
         textinput.text += linemap[nolines-1].text;
         cursor.y = 0;
         curline = 0;
         cursor_pos = 0;
         cursor.x = 0;
         cursor.a = 1;
      }
        

    var clearSelection = function() {
      for (var i=0; i<selection.length; i++)
      {
        selection[i].w = 0;
        selection[i].a = 0;
        selection[i].x = 0;
        selection[i].h = 0;
        selection[i].y = 0;
        selection[i].startPos = -1;
        selection[i].endPos = -1;
      }
      underSelection = false;
      selectedText = "";
      buttonDown = false;
      firstShiftDown = true;
      firstShiftUp = true;
      firstShiftLeft = true;
      firstShiftRight = true;
      movingRight = false;
      movingLeft = false;
      updateStartPos = false;
      lastactiveline = -1;
      lastactivepos = -1;
    }
         
    /* function to select all the lines */
    var selectAll = function () {
        underSelection = true;
        lastactiveline = curline;
        lastactivepos = cursor_pos;
        for (var i=0; i<nolines; i++)
        {
          var curtext = linemap[i].text;
          var curlen = curtext.length;
          if (curlen > 0)
          {
            selection[i].x = linemap[i].x;
            selection[i].w = linemap[i].width;
            selection[i].h = linemap[i].h;
            selection[i].y = linemap[i].y;
            selection[i].startPos = 0;
            selection[i].endPos = curlen;
            selection[i].a = 1;
          }
        }
        cursor.a = 0;
        return;
    };

    /* function to deselect any selected content */
    var unSelectAll = function() {
      if ((-1 != lastactiveline) && (-1 != lastactivepos))
      {
        curline = lastactiveline;
        cursor.y = linemap[curline].y;
        cursor_pos = lastactivepos;
        var currentTextBoxMetrics = fontRes.measureText(pts, linemap[curline].text.substring(0, cursor_pos));
        cursor.x = currentTextBoxMetrics.w;
      }
      clearSelection();
      cursor.a = 1;
    };

    /* function to move cursor to first line left */
    var moveToHome = function() {
        curline = 0;
        cursor_pos = 0;
        cursor.y = 8;
        cursor.x = 0;
    }

    /* function to move cursor to last line right */
    var moveToEnd = function() {
        curline = nolines-1;
        cursor_pos = linemap[nolines-1].text.length;
        cursor.y = linemap[curline].y;
        cursor.x = linemap[curline].width;
    }
    
    /* function to select till the home */
    var selectToHome = function () {
        underSelection = true;
        lastactiveline = curline;
        lastactivepos = cursor_pos;
        var stringToSelect = "";
        for (var i=nolines-1; i>=0; i--)
        {
          var currentString = linemap[i].text;
          if (currentString.length > 0)
          {
            if (i == curline)
            {
              stringToSelect = currentString.substring(0, cursor_pos);
              selection[i].startPos = 0;
              selection[i].endPos = cursor_pos;
            }
            else
            {
              stringToSelect = currentString;
              selection[i].startPos = 0;
              selection[i].endPos = stringToSelect.length;
            }
            var metrics = fontRes.measureText(pts, stringToSelect);
            selection[i].w = metrics.w + 1;
            selection[i].h = linemap[i].h;
            selection[i].a = 1;
            selection[i].x = linemap[i].x;
            selection[i].y = linemap[i].y;
          }
        }
    }

    /* function to select till the end */
    var selectToEnd = function () {
        underSelection = true;
        lastactiveline = curline;
        lastactivepos = cursor_pos;
        var stringToSelect = "";
        for (var i=curline; i<nolines; i++)
        {
          var currentString = linemap[i].text;
          if (currentString.length > 0)
          {
            if (i == curline)
            {
              stringToSelect = currentString.substring(cursor_pos);
              selection[i].startPos = cursor_pos;
              selection[i].endPos = stringToSelect.length;
            }
            else
            {
              stringToSelect = currentString;
              selection[i].startPos = 0;
              selection[i].endPos = stringToSelect.length;
            }
            var metrics = fontRes.measureText(pts, stringToSelect);
            selection[i].w = metrics.w;
            selection[i].h = linemap[i].h;
            selection[i].x = linemap[i].width - metrics.w;
            selection[i].y = linemap[i].y;
            selection[i].a = 1;
            if (i == 0)
            {
              var metricsComplete = fontRes.measureText(pts, currentString);
              selection[i].x = metricsComplete.w - metrics.w;
            }
          }
        }
    } 

    var getValue = function() {
        var result = "";
        for (var i=0; i<nolines; i++)
        {
          result += linemap[i].text;
        }
        return result;
    }

    function binarySearch(array, x)
    {
        var snip;
        var metrics;
        var pos_x = 0;

        var lo = -1, hi = array.length;
        while (1 + lo !== hi)
        {
            var mi = lo + ((hi - lo) >> 1);

            snip    = array.slice(0, mi );
            metrics = fontRes.measureText(pts, snip);

            if (metrics.w > x)
            {
                hi = mi;
            }
            else
            {
                pos_x = metrics.w;
                lo = mi;
            }
        }

        return [lo, pos_x];
    }

    /* function to find the line where the y value falls */
    function findRow(y)
    {
      var topy = -1, bottomy = -1;
      var lo = -1, hi = nolines;
      while (1 + lo !== hi)
      {
          var mi = lo + ((hi - lo) >> 1);

          var val = linemap[mi].y;

          if (val > y)
          {
              hi = mi;
          }
          else
          {
              lo = mi;
          }
      }
      if (lo == -1)
        lo = 0;
      else if (lo == nolines)
        lo = nolines-1;
      return lo;
    }

    // returns nearest text insertion point / cursor position to (x,y)
    function pointToCursor(x, y)
    {
        var snip = linemap[0].text;
        var temp = snip;
        var metrics  = fontRes.measureText(pts, snip);
        var full_w   = metrics.w;
        var return_x = metrics.w;
        var arr = [];
        arr[0] = snip.length;
        arr[1] = return_x;
        if (x <= full_w)
        {
           if(snip.length > 0)
            { 
                var ans   = binarySearch(snip, x);

                var index = ans[0];
                var lhs   = ans[1];
                var rhs   = full_w;

                if(index + 1 < snip.length)
                {
                    index++;
                    snip = temp.slice(0,  index);
                    rhs  =  fontRes.measureText(pts, snip).w;
                }
                else
                {
                    index = snip.length;
                }

                var lhs_w = (x   - lhs);  // Click is 'within Glyph' - closer to LHS
                var rhs_w = (rhs -   x);  // Click is 'within Glyph' - closer to RHS

                if(lhs_w <= rhs_w) // Closer to start..
                {
                    index--;
                    return_x = lhs_w;
                }            
                else
                {
                    return_x = rhs_w;                    
                }
                arr[0] = index;
                arr[1] = return_x;
            }
        }
    
        var yindex = findRow(y);
        var bottomindex;
        var top_h = y-linemap[yindex].y;
        var bottom_h;

        if (yindex < nolines-1)
        {
          bottomindex = yindex+1;
          bottom_h = linemap[yindex+1].y - y;   
        }
        else
        {
          bottomindex = nolines-1;
          bottom_h = linemap[nolines-1].y - y;   
        }
          
        if (top_h <= bottom_h)
          arr[2] = yindex;
        else
          arr[2] = bottomindex;
        
        return arr;
    }

    Promise.all(objsReady).then(function(o) {
      inputBg.w = inputWidth;
      inputBg.a = 0.9;
      textinput.w = inputWidth;
      inputBg.ready.then(function(o) {

        inputBg.on("onBlur", function (e)
        {
          cursor.a = 0;
        });

        inputBg.on("onFocus", function (e)
        {
          cursor.ready.then(function(o) {
            console.log("CURSOR READY");
            var measure  = fontRes.measureText(pts,"sample");
            cursor.a = 1;
            cursor.h = measure.h;
            cursor.animateTo({a:0},0.5,   scene.animation.TWEEN_LINEAR,scene.animation.OPTION_OSCILLATE,scene.animation.COUNT_FOREVER);
          });
        });

        inputBg.on("onMouseDown", function (e) {
            if (false == buttonDown)
            {
              var ptc = pointToCursor(e.x, e.y);

              var rowindex = ptc[2];
              if (linemap[rowindex].text.length < ptc[0])
              {
                cursor_pos = linemap[rowindex].text.length;
              }
              else
              {
                cursor_pos = ptc[0];
              }
              cursor.y = linemap[rowindex].y;
              updateCursor(cursor_pos);
           
              curline = rowindex;
              var text = linemap[rowindex].text;
              if (text.length > 0)
              {
                var metrics = fontRes.measureText(pts, text.substring(0, ptc[0]));
                selection[curline].x =  linemap[curline].x + metrics.w;
                selection[curline].startPos =  cursor_pos;
                selection[curline].endPos =  cursor_pos;
                buttonDown = true;
              }
            }
            else
            {
              cursor.a = 1;
              buttonDown = false;
              movingRight = false;
              movingLeft = false;
              unSelectAll();
            }
        });

        inputBg.on("onMouseUp", function (e) {
            cursor.a = 1;
            buttonDown = false;
            movingRight = false;
            movingLeft = false;
        });

        inputBg.on("onMouseMove", function(e) {
            if (buttonDown) {
                // Selecting by mouse drag
                var ptc = pointToCursor(e.x, e.y); // returns a tuple
                //change of line
                var index = ptc[2];
                if (curline > index)
                {
                   var prevTextBoxVal = linemap[curline-1].text;
                   var currentTextBoxVal = linemap[curline].text;
                   var prevBoxTextLength = prevTextBoxVal.length;
                   var currentTextBoxLength = currentTextBoxVal.length;

                   if (true == firstShiftUp)
                   {
                     var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(0, cursor_pos));
                     selection[curline].w = currentTextBoxMetrics.w;
                     selection[curline].x = linemap[curline].x;
                     selection[curline].h = currentTextBoxMetrics.h;
                     selection[curline].startPos = 0;
                     selection[curline].endPos = cursor_pos;
                   }
                   else
                   {
                     var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal);
                     selection[curline].w = currentTextBoxMetrics.w;
                     selection[curline].h = currentTextBoxMetrics.h;
                     selection[curline].x = linemap[curline].x;
                     selection[curline].startPos = 0;
                     selection[curline].endPos = currentTextBoxVal.length;
                   }
                   selection[curline].h = linemap[curline].h;
                   selection[curline].y = linemap[curline].y;
                   selection[curline].a = 1;
                   var prevTextBoxMetrics = fontRes.measureText(pts, prevTextBoxVal.substring(cursor_pos));
                   selection[curline-1].w = prevTextBoxMetrics.w;
                   prevTextBoxMetrics = fontRes.measureText(pts, prevTextBoxVal.substring(0, cursor_pos));
                   selection[curline-1].x = linemap[curline-1].x + prevTextBoxMetrics.w;
                   selection[curline-1].y = linemap[curline-1].y;
                   selection[curline-1].h = linemap[curline-1].h;
                   selection[curline-1].startPos = cursor_pos;
                   selection[curline-1].endPos = prevTextBoxVal.length;
                   selection[curline-1].a = 1;
                   if (true == firstShiftUp)
                   {
                     firstShiftUp = false;
                   }
                   cursor.y = linemap[curline-1].y;
                   curline--;
                }
                else if (curline < index)
                {
                    var nextTextBoxVal = linemap[curline+1].text;
                    var currentTextBoxVal = linemap[curline].text;
                    var nextBoxTextLength = nextTextBoxVal.length;
                    var currentTextBoxLength = currentTextBoxVal.length;
                    if (true == firstShiftDown)
                    {
                      var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(cursor_pos));
                      selection[curline].w = currentTextBoxMetrics.w;
                      currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(0,cursor_pos));
                      selection[curline].x = linemap[curline].x + currentTextBoxMetrics.w;
                      selection[curline].y = linemap[curline].y;
                      selection[curline].startPos = cursor_pos;
                      selection[curline].endPos = currentTextBoxVal.length;
                    }
                    else
                    {
                      var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal);
                      selection[curline].w = currentTextBoxMetrics.w;
                      currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(0,cursor_pos));
                      selection[curline].x = linemap[curline].x;
                      selection[curline].startPos = 0;
                      selection[curline].endPos = cursor_pos;
                    }
                    selection[curline].h = linemap[curline].h;
                    selection[curline].a = 1;
                    selection[curline+1].y = linemap[curline+1].y;
                    selection[curline+1].a = 1;
                    if (nextBoxTextLength < cursor_pos)
                    {
                      selection[curline+1].startPos = 0;
                      selection[curline+1].endPos = nextBoxTextLength;
                      var nextTextBoxMetrics = fontRes.measureText(pts, nextTextBoxVal.substring(0,nextBoxTextLength));
                      selection[curline+1].w = nextTextBoxMetrics.w;
                      selection[curline+1].h = nextTextBoxMetrics.h;
                    }
                    else
                    {
                      selection[curline+1].startPos = 0;
                      selection[curline+1].endPos = cursor_pos;
                      var nextTextBoxMetrics = fontRes.measureText(pts, nextTextBoxVal.substring(0,cursor_pos));
                      selection[curline+1].w = nextTextBoxMetrics.w;
                      selection[curline+1].h = nextTextBoxMetrics.h;
                    }
                    if (true == firstShiftDown)
                    {
                      firstShiftDown = false;
                    }
                    cursor.y = linemap[curline+1].y;
                    curline++;
                }
                else
                {
                  if (updateStartPos)
                    selection[curline].startPos = ptc[0];
                  else
                    selection[curline].endPos = ptc[0];

                  if (selection[curline].startPos > selection[curline].endPos)
                  {
                    updateStartPos = !updateStartPos;
                    var temp = selection[curline].startPos;
                    selection[curline].startPos = selection[curline].endPos;
                    selection[curline].endPos = temp;
                  }
                  var metrics = fontRes.measureText(pts, linemap[curline].text.substring(selection[curline].startPos, selection[curline].endPos));
                  selection[curline].w = metrics.w;
                  selection[curline].h = linemap[curline].h;
                  selection[curline].a = 1;
                  selection[curline].y = linemap[curline].y;
                  metrics = fontRes.measureText(pts, linemap[curline].text.substring(0, selection[curline].startPos));
                  selection[curline].x = linemap[curline].x + metrics.w;
                  //selection[curline].endPos = ptc[0];
                }
                curline = index;
                cursor.y = linemap[curline].y;
                updateCursor(ptc[0]); 
            }
        });

        inputBg.on("onChar", function(e) {
            var code  = e.charCode;
            var data = String.fromCharCode(code);
            var linedata = linemap[curline].text;
            var prestring = linedata.substring(0, cursor_pos);
            var poststring = linedata.substring(cursor_pos);
            var finalstring = prestring + data + poststring;
            var newmeasure  = fontRes.measureText(pts, finalstring);
            var newwidth = newmeasure.w;
            if (newwidth > (inputWidth-30))
            {
              var chartomove = poststring.substring(poststring.length-1);
              linedata = prestring + data + poststring.substring(0, poststring.length - 1);
              linemap[curline].text = linedata;
              var measure  = fontRes.measureText(pts,linedata);
              linemap[curline].width  = measure.w;
              linemap[curline].length  = linedata.length;
              if (linemap[curline+1] == undefined)
              {
                initializeLine(curline+1);
                nolines++;
              }
              var i = 0;
              for (i = curline+1; i<nolines-1; i++)
              {
                linedata = linemap[i].text;
                linemap[i].text = chartomove + linedata.substring(0, linedata.length-1);
                var measure  = fontRes.measureText(pts,linemap[i].text);
                linemap[i].width = measure.w;
                chartomove = linedata.substring(linedata.length-1);
              }
              if (i == nolines-1)
              {
                linedata = linemap[i].text;
                var lastmeasure  = fontRes.measureText(pts,linedata+chartomove);
                var newwidth = lastmeasure.w;
                if (newwidth > (inputWidth-30))
                {
                  var measure  = fontRes.measureText(pts, chartomove);
                  initializeLine(i+1);
                  linemap[i+1].length = 1;
                  linemap[i+1].width = measure.w;
                  linemap[i+1].y = linemap[i].y + linemap[i].h;
                  linemap[i+1].text = chartomove;
                  nolines++;
                }
                else
                {
                  linemap[i].text = chartomove + linedata.substring(0, linedata.length); 
                  linemap[i].length++;
                  var measure  = fontRes.measureText(pts,linemap[i].text);
                  linemap[i].width = measure.w;
                }
              }
            }
            else
            {
              linemap[curline].text = finalstring;
              linemap[curline].length++;
              linemap[curline].width = newwidth;
            }
            var finaltext = "";
            //textinput.text = "";
            for (var i=0; i<nolines-1; i++)
            {
              //console.log("i=" + i + "text=" + linemap[i].text);
              //textinput.text += linemap[i].text;
              finaltext += linemap[i].text;
              //console.log(textinput.text);
              if (linemap[i].text.substring(linemap[i].text.length-1) != "\n")
              {
                //textinput.text += "\n";
                finaltext += "\n";
              }
            }
            textinput.text = finaltext;
            textinput.text += linemap[nolines-1].text;
            cursor_pos++;

            var measure;

            measure  = fontRes.measureText(pts,linemap[curline].text.substring(0, linemap[curline].text.length));
            linemap[curline].width = measure.w;
            if (cursor_pos != linemap[curline].text.length)
              measure  = fontRes.measureText(pts,linemap[curline].text.substring(0, cursor_pos));
            cursor.x = measure.w;
            linemap[curline].h = measure.h;
            linemap[curline].y = cursor.y;
            if ((cursor_pos == linemap[curline].text.length) && (linemap[curline].width > (inputWidth-30)))
            {
              cursor_pos = 0;
              cursor.x = 0;
              curline++;
              cursor.y += measure.h;
             }
        });

        inputBg.on("onKeyDown", function(e)
        {
          var code  = e.keyCode;
          var flags = e.flags;

          switch(code)
          {
            case keys.ENTER:
              if (curline < totlines-1)
              {
                linemap[curline].text += "\n";
                linemap[curline].length++;
                var measure = fontRes.measureText(pts, linemap[curline].text);
                linemap[curline].width = measure.w;
                //linemap[curline].h = measure.h;
                linemap[curline].length++;
                initializeLine(curline+1);
                curline++;
                cursor.y = linemap[curline-1].y + linemap[curline-1].h;
                cursor.x = 0;
                cursor_pos = 0;
                nolines++;
              }
              break;
            case keys.HOME:
              moveToHome();
              break;
            case keys.END:
              moveToEnd();
              break;
            case keys.A:   // << CTRL + A 
              if (keys.is_CTRL(e.flags))  // ctrl Pressed also
              {
                selectAll();
              }
              break;
            case keys.C:   // << CTRL + C
                if ((keys.is_CTRL(e.flags) || keys.is_CMD(e.flags)))  // ctrl Pressed also
                {
                    console.log("Copying text ...... " );
                    scene.clipboardSet('PX_CLIP_STRING', getSelectedText());
                }
                break;
            case keys.X:   // << CTRL + X
                if ((keys.is_CTRL(e.flags) || keys.is_CMD(e.flags)))  // ctrl Pressed also
                {
                    scene.clipboardSet('PX_CLIP_STRING', getSelectedText());
                    console.log("Cutting text ...... " );
                    removeSelectedText();
                    reArrangeText();
                    clearSelection();
                }
                break;
            case keys.V:   // << CTRL + V
                if ((keys.is_CTRL(e.flags) || keys.is_CMD(e.flags)) )  // ctrl Pressed also
                {
                    var fromClip = scene.clipboardGet('PX_CLIP_STRING');

                    if (curline <= nolines-1)
                    {
                      // insert character
                      var currentString = linemap[curline].text;
                      var currentLength = currentString.length;

                      //var finalstring = prestring + data + poststring;
                      var newmeasure  = fontRes.measureText(pts, currentString + fromClip);
                      var newwidth = newmeasure.w;
                      if (newwidth <= (inputWidth-30))
                      {
                        //no need of moving between text boxes
                        //linemap[curline].text = currentString.slice(0, cursor_pos) + fromClip + currentString.slice(cursor_pos);
                        linemap[curline].text = currentString.substring(0, cursor_pos) + fromClip + currentString.substring(cursor_pos);
                        cursor_pos += fromClip.length;
                        linemap[curline].length += fromClip.length;
                        linemap[curline].width = newwidth;
                        linemap[curline].h = newmeasure.h;
                        updateCursor(cursor_pos);
                      }
                      else
                      {
                        var stringBefCursor = currentString.substring(0, cursor_pos);
                        var stringAfterCursor = currentString.substring(cursor_pos);
                        var lenAfterAppend = stringBefCursor.length + fromClip.length; 
                        var widthWithFirstPortion = getWidth(stringBefCursor+fromClip);
                        if (widthWithFirstPortion <=  (inputWidth-30))
                        {
                         // var totalvalidlen = validHoldableIndex(stringBefCursor+fromClip+stringAfterCursor);
                          var availableWidth = newwidth - (inputWidth-30);
                          var availableStringLength = getAvailableLength(stringAfterCursor, availableWidth);
                          var lenRemainingAfterAppend = stringAfterCursor.length - availableStringLength;
                          var remainingInString = stringAfterCursor.substring(0,availableStringLength);
                          var stringToMove = stringAfterCursor.substring(availableStringLength);
                          linemap[curline].text = stringBefCursor + fromClip + remainingInString;
                          linemap[curline].length = linemap[curline].text.length;
                          var newmeasuredata  = fontRes.measureText(pts, linemap[curline].text);
                          linemap[curline].width = newmeasuredata.w;
                          linemap[curline].h = newmeasuredata.h;
                          cursor_pos += fromClip.length;
                          updateCursor(cursor_pos);
                          for (var i=curline+1; i<nolines; i++)
                          {
                            newmeasure  = fontRes.measureText(pts, stringToMove + currentString);
                            currentString = linemap[i].text;
                            lenAfterAppend = stringToMove.length + currentString.length;
                            if (newmeasure.w <=  (inputWidth-30))
                            {
                              linemap[i].text = stringToMove + currentString;
                              linemap[i].length = linemap[i].text.length;
                              var newmeasure  = fontRes.measureText(pts, linemap[i].text);
                              linemap[i].width = newmeasure.w;
                              linemap[i].h = newmeasure.h;
                              break;
                            }
                            else
                            {
                              availableWidth = newmeasure.w - (inputWidth-30);
                              availableStringLength = getAvailableLength(currentString, availableWidth);
                              remainingInString = currentString.substring(0, availableStringLength);
                              linemap[i].text = stringToMove + remainingInString;
                              var newmeasure  = fontRes.measureText(pts, linemap[i].text);
                              linemap[i].length = linemap[i].text.length;
                              linemap[i].width = newmeasure.w;
                              linemap[i].h = newmeasure.h;
                              stringToMove = currentString.substring(availableStringLength);
                            }
                          }
                        }
                        else
                        {
                          var lastIndex = -1;
                          var newmeasure  = fontRes.measureText(pts, stringBefCursor);
                          var newwidth = newmeasure.w;
                          var availableWidth = (inputWidth-30) - newwidth;
                          var availableStringLength = getAvailableLength(fromClip, availableWidth);
                          var remainingInString = fromClip.substring(0, availableStringLength);
                          var stringToMove = fromClip.substring(availableStringLength) + stringAfterCursor;
                          var remClipMovedLength = fromClip.length - availableStringLength;
                          linemap[curline].text = stringBefCursor + remainingInString;
                          newmeasure  = fontRes.measureText(pts, linemap[curline].text);
                          linemap[curline].h = newmeasure.h;
                          linemap[curline].length = linemap[curline].text.length;
                          linemap[curline].width = newmeasure.w;
                          if (curline == nolines-1) // this is last line
                          {
                            for (var j=curline+1; j<totlines; j++)
                            {
                              if (linemap[j] == undefined)
                              {
                                initializeLine(j);
                                nolines++;
                              }
                              newmeasure  = fontRes.measureText(pts, stringToMove);
                              if (newmeasure.w <=  (inputWidth-30))
                              {
                                linemap[j].text = stringToMove;
                                linemap[j].length = linemap[j].text.length;
                                linemap[j].width = newmeasure.w;
                                linemap[j].y = linemap[j-1].y + linemap[j-1].h;
                                linemap[j].h = newmeasure.h;
                                lastIndex = j;
                                break;
                              }
                              else
                              {
                                availableWidth = newmeasure.w - (inputWidth-30);
                                availableStringLength = getAvailableLength(stringToMove, (inputWidth-30));
                                remainingInString = stringToMove.substring(0, availableStringLength);
                                linemap[j].text = remainingInString;
                                linemap[j].length = remainingInString.length;
                                linemap[j].y = linemap[j-1].y + linemap[j-1].h;
                                newmeasure  = fontRes.measureText(pts, remainingInString);
                                linemap[j].h = newmeasure.h;
                                linemap[j].width = newmeasure.w;
                                stringToMove = stringToMove.substring(availableStringLength);
                              }
                            }
                            if (linemap[lastIndex].text.length <= stringAfterCursor.length)
                            {
                              curline = lastIndex - 1;
                              cursor_pos = linemap[lastIndex-1].text.length - (stringAfterCursor.length - linemap[lastIndex].length);
                              cursor.y = linemap[lastIndex-2].y + linemap[lastIndex-2].h;
                            }
                            else
                            {
                              curline = lastIndex;
                              cursor_pos = linemap[lastIndex].text.length - (stringAfterCursor.length);
                              cursor.y = linemap[lastIndex-1].y + linemap[lastIndex-1].h;
                            }
                            updateCursor(cursor_pos); 
                          }
 
                          else
                          {
                            for (var i=curline+1; i<nolines; i++)
                            {
                              currentString = linemap[i].text;
                              newmeasure  = fontRes.measureText(pts, stringToMove + currentString);
                              lenAfterAppend = stringToMove.length + currentString.length;
                              if (newmeasure.w <=  (inputWidth-30))
                              {
                                linemap[i].text = stringToMove + currentString;
                                linemap[i].length = linemap[i].text.length;
                                linemap[i].width = newmeasure.w;
                                linemap[i].h = newmeasure.h;
                                lastIndex = i;
                                break;
                              }
                              else
                              {
                                availableWidth = newmeasure.w - (inputWidth-30);
                                availableStringLength = getAvailableLength(currentString, availableWidth);
                                remainingInString = currentString.substring(0, availableStringLength);
                                linemap[i].text = stringToMove + remainingInString;
                                linemap[i].length = linemap[i].text.length;
                                var newmeasured  = fontRes.measureText(pts, linemap[i].text);
                                linemap[i].width = newmeasured.w;
                                linemap[i].h = newmeasured.h;
                                textinput.text += linemap[i].text;
                                if (linemap[i].text.substring(linemap[i].text.length-1) != "\n")
                                  textinput.text += "\n";
                                stringToMove = currentString.substring(availableStringLength);
                              }
                            }
                            if (linemap[lastIndex].text.length <= stringAfterCursor.length)
                            {
                              curline = lastIndex - 1;
                              cursor_pos = linemap[lastIndex-1].text.length - (stringAfterCursor.length - linemap[lastIndex].length);
                              cursor.y = linemap[lastIndex-2].y + linemap[lastIndex-2].h;
                            }
                            else
                            {
                              curline = lastIndex;
                              cursor_pos = linemap[lastIndex].text.length - (stringAfterCursor.length);
                              cursor.y = linemap[lastIndex-1].y + linemap[lastIndex-1].h;
                            }
                            updateCursor(cursor_pos);
                          }
                        }
                      }
                      textinput.text = "";
                      for (var i=0; i<nolines-1; i++)
                      { 
                        textinput.text += linemap[i].text;
                        if (linemap[i].text.substring(linemap[i].text.length-1) != "\n")
                          textinput.text += "\n";
                      }
                      textinput.text += linemap[nolines-1].text;
                    }
                }
                break;
            case keys.ESCAPE:
              unSelectAll();
              break;
            case keys.BACKSPACE:
              {
                if (true == underSelection)
                {
                  removeSelectedText();
                  reArrangeText();
                  clearSelection();
                }
                else
                {
                  if ((curline >= 0) /*&& (cursor_pos > 0)*/)
                  {
                    if ((curline == 0) && (cursor_pos == 0))
                    {
			/* Avoid doing backspace operation on first line and zero'th position */
                    }
                    else
                    {
                      var text = linemap[curline].text;
                      if( text.length > 0) {
                        var preString;
                        var postString;
                        var m;
                        if (cursor_pos == text.length)
                        {
                          preString = text.substring(0,cursor_pos-1);
                          postString = "";
                          m = fontRes.measureText(pts, text.substring(cursor_pos-1));
                        }
                        else
                        {
                          preString = text.substring(0,cursor_pos-1);
                          postString = text.substring(cursor_pos);
                          m = fontRes.measureText(pts, text.substring(cursor_pos, cursor_pos+1));
                        }

                        linemap[curline].text = preString + postString;
                        //linemap[curline].width -= m.w;
                        //linemap[curline].length--;
                        cursor_pos -= 1; // removed 1 character
                        if ((cursor_pos <= 0) && (curline != 0))
                        {
                          curline--;
                          cursor_pos = linemap[curline].text.length;
                          cursor.y -= linemap[curline+1].h;
                        }
                        for (var i=curline+1; i<nolines; i++)
                        {
                          var myText = linemap[i].text;
                          if (myText.length > 0)
                          {
                            var moveUpTextFromCurrent = myText.substring(0,1);
                            linemap[i-1].text += moveUpTextFromCurrent;
                            linemap[i].text = myText.substring(1);
                          }
                        }
                        var nodeletedlines = 0;
                        for (var i=0; i<nolines; i++)
                        {
                          var myText = linemap[i].text;
                          if (myText.length > 0)
                          {
                            var curmetrics = fontRes.measureText(pts, myText);
                            linemap[i].length = myText.length;
                            linemap[i].width = curmetrics.w;
                            linemap[i].h = curmetrics.h;
                          }
                          else
                          {
                            linemap[i].length = 0;
                            linemap[i].width = 0;
                            linemap[i].h = 0;
                            nodeletedlines++;
                          }
                        }
                        //nolines -= nodeletedlines;
                        if (nolines == 0)
                          nolines = 1;
                        textinput.text = "";
                        for (var i=0; i<nolines-1; i++)
                        {
                          textinput.text += linemap[i].text;
                          if (linemap[i].text.substring(linemap[i].text.length-1) != "\n")
                            textinput.text += "\n";
                        }
                        if (nolines >= 1)
                        {
                          textinput.text += linemap[nolines-1].text;
                          var data = linemap[curline].text;
                          var curmetrics = fontRes.measureText(pts, data.substring(0, cursor_pos));
                          cursor.x = curmetrics.w;
                          cursor.y = linemap[curline].y;
                        }
                      }
                    }
                  }
                }
              }
              break;
            case keys.RIGHT:
              {
                if (keys.is_CTRL(e.flags) || keys.is_CMD(e.flags))
                {
                    moveToEnd();
                }
                else if ((keys.is_CTRL_SHIFT(e.flags) || keys.is_CMD_SHIFT(e.flags)) )
                {
                    selectToEnd();
                }
                else if (keys.is_SHIFT(e.flags))
                {
                  underSelection = true;
                  var validcase = false;

                  if (firstShiftLeft == false)
                  {
                    if ((cursor_pos == linemap[curline].text.length) && (curline < nolines-1) && (selection[curline].a == 1))
                    {
                      selection[curline].endPos = linemap[curline].text.length;
                      selection[curline].startPos = linemap[curline].text.length-1;
                      cursor_pos = 0;
                      cursor.y = linemap[curline+1].h;
                      curline++;
                      selection[curline].startPos = 0;
                      validcase = true;
                    }
                    else
                    {
                      if (cursor_pos < linemap[curline].text.length)
                      {
                        cursor_pos++;
                        selection[curline].startPos++;
                        validcase = true;
                      }
                    }
                  }

                  if (cursor_pos == linemap[curline].text.length)
                  {
                    if (curline < nolines-1)
                    {
                      curline++;
                      cursor_pos = 1;
                      cursor.y += linemap[curline-1].h;
                      selection[curline].y = linemap[curline].y;
                      selection[curline].a = 1;
                      selection[curline].h = linemap[curline].h;
                      selection[curline].startPos = 0;
                      selection[curline].endPos = cursor_pos;
                      selection[curline].x = 0;
                      validcase = true;
                    }
                  }
                  else
                  {
                    var curtext = linemap[curline].text;
                    var curlength = curtext.length;
                    if ((cursor_pos >= 0) && (curlength > 0) & (cursor_pos < curlength))
                    {
                      cursor_pos++;
                      var curtext = linemap[curline].text;
                      validcase = true;
                    }
                  }
                  if (true == validcase) 
                  {
                    var curtext = linemap[curline].text;
                    var curmetrics;
                    if (cursor_pos == curtext.length)
                    {
                      curmetrics = fontRes.measureText(pts, curtext.substring(cursor_pos-1));
                    }
                    else
                    {
                      curmetrics = fontRes.measureText(pts, curtext.substring(cursor_pos,cursor_pos+1));
                    }
                    if (true == firstShiftRight)
                    {
                      firstShiftRight = false;
                      lastactiveline = -1;
                      lastactivepos = -1;
                      selection[curline].y = linemap[curline].y;
                      selection[curline].h = linemap[curline].h;
                      selection[curline].a = 1;
                      selection[curline].startPos = cursor_pos-1;
                      var data = curtext.substring(0, cursor_pos);
                      var measure  = fontRes.measureText(pts,data);
                      selection[curline].x = measure.w;
/*
                      if (cursor_pos != curtext.length-1)
                        selection[curline].endPos = cursor_pos+1;
                      else
                        selection[curline].endPos = cursor_pos;
*/
                      selection[curline].x = cursor.x;
                      cursor.a = 0;
                    }
                    selection[curline].endPos = cursor_pos;
                    selection[curline].w += curmetrics.w;
                    updateCursor(cursor_pos);
                  }
                }
                else
                {
                  if (cursor_pos == linemap[curline].length)
                  {
                    if (curline+1 < nolines)
                    {
                      cursor.y += linemap[curline].h;
                      cursor_pos=0;
                      var data = linemap[curline].text;
                      var measure  = fontRes.measureText(pts,data.substring(0, cursor_pos));
                      cursor.x = measure.w;
                      curline++;
                    }
                  }
                  else
                  {
                    var curlength = linemap[curline].length;
                    if ((cursor_pos < curlength) && (curlength > 0))
                    {
                      var data = linemap[curline].text;
                      var measure  = fontRes.measureText(pts,data.substring(cursor_pos,cursor_pos+1));
                      cursor.x += measure.w;
                      cursor_pos++;
                    }
                  }
                }
                break;
              }
            case keys.LEFT:
              {
                if (keys.is_CTRL(e.flags) || keys.is_CMD(e.flags))
                {
                    moveToHome();
                }
                else
                if ((keys.is_CTRL_SHIFT(e.flags) || keys.is_CMD_SHIFT(e.flags)) )
                {
                    selectToHome();
                }
                else
                {
                  if (keys.is_SHIFT(e.flags))
                  {
                    underSelection = true;
                    var validcase = false;
                    if (firstShiftRight == false)
                    {
                      if ((cursor_pos == 0) && (curline > 0) && (selection[curline-1].a == 1))
                      {
                        selection[curline].endPos = 0;
                        cursor_pos = linemap[curline-1].text.length;
                        cursor.y = linemap[curline-1].h;
                        cursor.x = linemap[curline-1].width;
                        curline--;
                        selection[curline].endPos = cursor_pos;
                        selection[curline].startPos = cursor_pos-1;
                        validcase = true;
                      }
                      else
                      {
                        if (cursor_pos > 0)
                        {
                          var curtext = linemap[curline].text;
                          cursor_pos--;
                          var data = curtext.substring(cursor_pos);
                          var measure  = fontRes.measureText(pts,data);
                          cursor.x = measure.w;
                          selection[curline].endPos--;
                          validcase = true;
                        }
                      }
                    }
                    if (cursor_pos == 0)
                    {
                      if (curline > 0)
                      {
                        curline--;
                        cursor_pos = linemap[curline].text.length;
                        cursor.y -= linemap[curline+1].h;
                        selection[curline].y = linemap[curline].y;
                        selection[curline].a = 1;
                        selection[curline].h = linemap[curline].h;
                        selection[curline].startPos = linemap[curline].text.length;
                        selection[curline].endPos = linemap[curline].text.length;
                        cursor.x = linemap[curline].width;
                        validcase = true;
                      }
                    }
                    else
                    {
                      var curtext = linemap[curline].text;
                      var curlength = curtext.length;
                      if ((cursor_pos > 0) && (curlength > 0))
                      {
                        cursor_pos--;
                        var data = curtext.substring(cursor_pos);
                        var measure  = fontRes.measureText(pts,data);
                        cursor.x = linemap[curline].width - measure.w;
                        validcase = true;
                      }
                    }

                    if (true == validcase) 
                    {
                      var curtext = linemap[curline].text;

                      var curmetrics = fontRes.measureText(pts, curtext.substring(cursor_pos,cursor_pos+1));

                      if (true == firstShiftLeft)
                      {
                        firstShiftLeft = false;
                        lastactiveline = -1;
                        lastactivepos = -1;
                        selection[curline].y = linemap[curline].y;
                        selection[curline].h = linemap[curline].h;
                        selection[curline].a = 1;
                        selection[curline].endPos = cursor_pos+1;
                        cursor.a = 0;
                      }
                      selection[curline].startPos = cursor_pos;
                      selection[curline].x = cursor.x;
                      selection[curline].w += curmetrics.w;
                    }
                  }
                  else
                  {
                    if (cursor_pos == 0)
                    {
                      if (curline > 0)
                      {
                        cursor.y -= linemap[curline].h;
                        cursor_pos=linemap[curline-1].text.length;
                        cursor.x = linemap[curline-1].width;
                        curline--;
                      }
                    }
                    else
                    {
                      var curlength = linemap[curline].length;
                      if ((cursor_pos > 0) && (curlength > 0))
                      {
                        var data = linemap[curline].text;
                        var measure  = fontRes.measureText(pts,data.substring(cursor_pos-1,cursor_pos));
                        if ((cursor.x-measure.w) < 0)
                          cursor.x = 0;
                        else
                          cursor.x -= /*linemap[curline].x +*/ measure.w;
                        console.log(cursor.x);
                        cursor_pos--;
                        //updateCursor(cursor_pos);
                      }
                    }
                  }
                }
                break;
              }
            case keys.UP:
              {
                if (curline > 0)
                {
                  if ((keys.is_CTRL_SHIFT(e.flags) || keys.is_CMD_SHIFT(e.flags)) )
                  {
                      underSelection = true;
                      var prevTextBoxVal = linemap[curline-1].text;
                      var currentTextBoxVal = linemap[curline].text;
                      var prevBoxTextLength = prevTextBoxVal.length;
                      var currentTextBoxLength = currentTextBoxVal.length;

                      if (true == firstShiftUp)
                      {
                        var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(0, cursor_pos));
                        selection[curline].w = currentTextBoxMetrics.w;
                        selection[curline].x = linemap[curline].x;
                        selection[curline].startPos = 0;
                        selection[curline].endPos = cursor_pos;
                      }
                      else
                      {
                        var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal);
                        selection[curline].w = currentTextBoxMetrics.w;
                        selection[curline].x = linemap[curline].x;
                        selection[curline].startPos = 0;
                        //selection[curline].endPos = currentTextBoxVal.length-1;
                        selection[curline].endPos = currentTextBoxVal.length;
                      }
                      selection[curline].h = linemap[curline].h;
                      selection[curline].y = linemap[curline].y;
                      selection[curline].a = 1;
                      var prevTextBoxMetrics = fontRes.measureText(pts, prevTextBoxVal.substring(cursor_pos));
                      selection[curline-1].w = prevTextBoxMetrics.w;
                      prevTextBoxMetrics = fontRes.measureText(pts, prevTextBoxVal.substring(0, cursor_pos));
                      selection[curline-1].x = linemap[curline-1].x + prevTextBoxMetrics.w;
                      selection[curline-1].y = linemap[curline-1].y;
                      selection[curline-1].h = linemap[curline-1].h;
                      selection[curline-1].startPos = cursor_pos;
                      //selection[curline-1].endPos = prevTextBoxVal.length-1;
                      selection[curline-1].endPos = prevTextBoxVal.length;
                      selection[curline-1].a = 1;
                      if (true == firstShiftUp)
                      {
                        firstShiftUp = false;
                        lastactiveline = curline;
                        lastactivepos = cursor_pos;
                      }
                      cursor.y = linemap[curline-1].y;
                      curline--;
                  }
                  else
                  {
                      cursor.y = linemap[curline-1].y;
                      curline--;
                  }
                }
                break;
              }
            case keys.DOWN:
              {
                if (curline < nolines-1)
                {
                  if ((keys.is_CTRL_SHIFT(e.flags) || keys.is_CMD_SHIFT(e.flags)) )
                  {
                    underSelection = true;
                    var nextTextBoxVal = linemap[curline+1].text;
                    var currentTextBoxVal = linemap[curline].text;
                    var nextBoxTextLength = nextTextBoxVal.length;
                    var currentTextBoxLength = currentTextBoxVal.length;
                    if (true == firstShiftDown)
                    {
                      var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(cursor_pos));
                      selection[curline].w = currentTextBoxMetrics.w;
                      currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(0,cursor_pos));
                      selection[curline].x = linemap[curline].x + currentTextBoxMetrics.w;
                      selection[curline].y = linemap[curline].y;
                      selection[curline].startPos = cursor_pos;
                      selection[curline].endPos = currentTextBoxVal.length;
                    }
                    else
                    {
                      var currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal);
                      selection[curline].w = currentTextBoxMetrics.w;
                      currentTextBoxMetrics = fontRes.measureText(pts, currentTextBoxVal.substring(0,cursor_pos));
                      selection[curline].x = linemap[curline].x;
                      selection[curline].startPos = 0;
                      selection[curline].endPos = cursor_pos;
                    }
                    selection[curline].h = linemap[curline].h;
                    selection[curline].a = 1;
                    var nextTextBoxMetrics = fontRes.measureText(pts, nextTextBoxVal.substring(0,cursor_pos));
                    selection[curline+1].w = nextTextBoxMetrics.w;
                    selection[curline+1].h = linemap[curline+1].h;
                    selection[curline+1].y = linemap[curline+1].y;
                    selection[curline+1].a = 1;
                    if (nextBoxTextLength < cursor_pos)
                    {
                      selection[curline+1].startPos = 0;
                      //selection[curline+1].endPos = nextBoxTextLength-1;
                      selection[curline+1].endPos = nextBoxTextLength;
                    }
                    else
                    {
                      selection[curline+1].startPos = 0;
                      //selection[curline+1].endPos = cursor_pos-1;
                      selection[curline+1].endPos = cursor_pos;
                    }
                    if (true == firstShiftDown)
                    {
                      firstShiftDown = false;
                      lastactiveline = curline;
                      lastactivepos = cursor_pos;
                    }
                    cursor.y = linemap[curline+1].y;
                    curline++;
                  }
                  else
                  {
                    if ((curline+1) < nolines)
                    {
                      cursor.y = linemap[curline+1].y;
                      if (linemap[curline+1].width < cursor.x)
                        cursor.x = linemap[curline+1].width;
                      if (curline != nolines-1)
                        curline++;
                    }
                  }
                }
                break;
              }
          }
         });
    });

    function updateCursor(pos)
    {
      var s = linemap[curline].text;
      var metrics = fontRes.measureText(pts, s.substring(0, pos));
      cursor.x = /*linemap[curline].x + */metrics.w;
    }

    function getWidth(text)
    {
      var measure  = fontRes.measureText(pts, text);
      return measure.w;
    }

    function getAvailableLength(text, width)
    {
      var index = -1;
      for (var i=text.length; i>0; i--)
      {
        var curdata = text.substring(0,i);
        var measure  = fontRes.measureText(pts, curdata);
        if (measure.w <= width)
        {
          index = i;
          break;
        }
      }
      return index;
    }
   });

   return {
     setFocus: function() {
       inputBg.focus = true;
     },
     getFocus: function() {
       return inputBg.focus;
     },
     ready: new Promise(function(resolve, reject) {
       Promise.all(objsReady)
       .then(function() {
         console.log("PROMISE ALL IS READY");
         resolve();
       });
     }),
     getValue: function() {
       return getValue();
     },
     moveToHome: function() {
       moveToHome();
     },
     moveToEnd: function() {
       moveToEnd();
     },
     selectToHome: function() {
       selectToHome();
     },
     selectToEnd: function() {
       selectToEnd();
     },
     selectAll: function() {
       selectAll();
     },
     clearSelection: function() {
       unSelectAll();
     },
     getSelectedText: function() {
       return getSelectedText();
     }
   }
  }

  initializeLine(0);
  module.exports = TextInput;

}).catch(function importFailed(err){
    console.error("Import failed for TextInput.js: " + err);
});
