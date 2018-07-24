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


px.import({ scene: 'px:scene.1.js',
             keys: 'px:tools.keys'
}).then(function importsAreReady(imports)
{
    var scene = imports.scene;
    var keys  = imports.keys;

    var laterTimer = null;
    var wideAnim   = null;

    var didNarrow = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EXAMPLE:  
//
//   var listBox  = new ListBox({  x: 10, y: 10, w: 800, h: 35, numItems: 3, visible:false, text_color:0x303030ff, selection_color:0xCC000088, pts:18 , font:font file, left:5px });
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    function ListBox(params) {

        var left = 5;

        
        this._visible = false;

        // PUBLIC methods
        //
        this.addItem     = addItem;
        this.removeItem      = removeItem;        
        this.selectedItem = selectedItem;
        this.selectedItemList = selectedItemList;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        function notify(msg) {
            console.log("layout( " + msg + " ) >>> WxH: " + w + " x " + h + " at (" + x + "," + y + ")");
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        var self       = this;
        var x = 0;
        var y = 0;
        var w = 0;
        var h = 0;
        var numItems = 0;
        // list to store all the items added to list
        var items = [];
        // index pointing to current cursor marking position
        var activeItemIndex = -1;
        var selectedItems = [];
        // index to indicate start and end position of visible items from main list
        var visibleStartIndex = 0;
        var visibleEndIndex = -1;
        // array holding only the visible items
        var text = [];
        // array holding all the items
        var items = [];
        var totalItems = 0;
        var startVisibleIndex = 0;
        var endVisibleIndex = numItems - 1;
        var seperators = [];
        var yindex = 0;
        //array holding the selected text items
        var selectitems = [];
        var iscontrolpressed = false;
                                                 // set -OR- (default)   
        x =               "x" in params ? params.x               : 0;
        y =               "y" in params ? params.y               : 0;
        w =               "w" in params ? params.w               : 200;
        h =               "h" in params ? params.h               : 200;
        numItems =               "numItems" in params ? params.numItems               : 3;
        this._visible =               "visible" in params ? params.visible               : false;

        var          parent =          "parent" in params ? params.parent          : scene.root;  // critical
        var             pts =             "pts" in params ? params.pts             : 18;
        var            font =            "font" in params ? params.font            : "FreeSans.ttf";
        var      text_color =      "text_color" in params ? params.text_color      : 0x303030ff;  // black
        var selection_color = "selection_color" in params ? params.selection_color : 0xCC000088;  // yellow

        var container = scene.create({ t: "object", parent: parent,    x: x, y: y, w: w, h: h, draw:false, focus:true });
        var clipRect  = scene.create({ t: "rect", parent: container, x: 0,       y: 0,       w: w, h: h, clip: true, fillColor: 0xFFFFFFF0, focus:true} );
        var scrollBar  = scene.create({ t: "rect", parent: clipRect, x: w-20,       y: 0,       w: 20, h: h, fillColor: 0xFFFFFFF0
} );

        var fontRes   = scene.create({ t: "fontResource",  url: font       });
        var arrowUpImage = scene.create({t:"image", url:"browser/images/arrow.png", parent:scrollBar, x: 0, y:0, w:20, h:20});
        var arrowDownImage = scene.create({t:"image", url:"browser/images/arrow_invert.png", parent:scrollBar, x: 0, y:h-20, w:20, h:20});

        var assets = [fontRes, clipRect, scrollBar, arrowUpImage, arrowDownImage];

        var itemHeight = h/numItems;

        Object.defineProperty(this, "visible",
        {
            set: function (val) { this._visible = val; if(this._visible) { container.draw = true; } else { container.draw=false;} },
            get: function () { return this._visible; },
        });

        for (var i=0; i<numItems; i++)
        {
          text[i]      = scene.create({ t: "textBox", font: fontRes, parent: clipRect, textColor: 0x000000F0, pixelSize: pts, x: 0, y:yindex, w:w-20, h:itemHeight-2, alignVertical : scene.alignVertical.CENTER, draw:false, ellipsis:true, focus:true, xStartPos : left   });
          seperators[i]      = scene.create({ t: "rect", w: w-20, h: 2, parent: clipRect, x: 0, y: yindex+itemHeight });
          assets.push(text[i]);
          assets.push(seperators[i]);
          yindex = yindex + itemHeight;
          text[i].selected = false;


          text[i].on("onMouseUp", function (e) {
              if (false == iscontrolpressed)
              {
                var alreadySelected = false;
                for (var i=0; i<selectitems.length; i++)
                {
                  if (selectitems[i] == this)
                  {
                    alreadySelected = true;
                  }
                  activeItemIndex = -1;
                  selectitems[i].selected = false;
                  selectitems[i].textColor = text_color;
                  selectitems.pop();
                }  

                if (false == alreadySelected)
                {
                  this.textColor = selection_color;
                  this.selected = true;
                  selectitems.push(this);
                  for (var index=0; index<text.length; index++)
                  {
                    if (text[index] == this)
                    {
                      activeItemIndex = index;
                      break;
                    }
                  }
                }
              }
              else
              {
                if (this.selected == true)
                {
                  this.selected = false;
                  var index = selectitems.indexOf(this);
	          if (index > -1) {
	            selectitems.splice(index, 1);
	          }
                  this.textColor = text_color;
                }
                else
                {
                  this.textColor = selection_color;
                  this.selected = true;
                  selectitems.push(this);
                }
              }
          }.bind(text[i]));
        }

        /* rearrange the list on scroll down */
        function updateListBottom()
        {
          if (visibleEndIndex != totalItems-1)
          {
            for (var index=1; index<numItems; index++)
            {
              text[index-1].text = text[index].text;
            }
            visibleEndIndex++;
            visibleStartIndex++;
            text[numItems-1].text = items[visibleEndIndex];
          }
        }

        /* rearrange the list on scroll up */
        function updateListTop()
        {
          if (visibleStartIndex != 0)
          {
            for (var index=numItems-1; index>0; index--)
            {
              text[index].text = text[index-1].text;
            }
            visibleEndIndex--;
            visibleStartIndex--;
            text[0].text = items[visibleStartIndex];
          }
        }

        /* add item to the list */
        function addItem(data)
        {
          var index = items.indexOf(data);
          if (-1 == index)                                                                                        
          {                       
            items.push(data);            
            if (totalItems < numItems)                                                                                     
            {                 
              text[totalItems].text = data;
              text[totalItems].draw = true;                                                                       
              visibleEndIndex = totalItems;
            }                 
            totalItems++;
          }     
        }

        /* remove item from the list */
        function removeItem(data)
        {
          var itemsindex = -1;
          //remove from the total items list
          var index = items.indexOf(data);
	  if (index > -1) {
	    items.splice(index, 1);
            itemsindex = index;
	  }
          totalItems--;          

          //remove from the visible items list
          index = -1;
          for (var i=0; i<numItems; i++)
          {
            if (text[i].text == data)
            {
              index = i; 
              break;
            }
          }
          if (index != -1)
          {
            for (var i=index; i<numItems-1; i++)
            {
              text[i].text = text[i+1].text;
            }
            if (items.length >= numItems)
            {
              text[numItems-1].text = items[visibleEndIndex];
            }
            else
            {
              text[index].text = "";
              visibleEndIndex--;
            }
          }

          //remove from the select items list
          index = -1;
          for (var i=0; i<selectitems.length; i++)
          {
            if (selectitems[i].text == data)
            {
              index = i;
              break;
            }
          }
          //if selected item is removed rearrange the list
          // [1 2 3 4] 5 -> if 3 is selected and removed -> [1 2 4 5]
	  if (index > -1) {
	    selectitems.splice(index, 1);
            }
        }

        /* return the selected item when user selected only one item */
        function selectedItem()
        {
          if (-1 != activeItemIndex)
          {
            return text[activeItemIndex].text;
          }
          else
            return "UNAVAILABLE"
        }

        /* return the list of selected items when user selected more items */
        function selectedItemList()
        {
          var list = [];
          for (var i=0; i<selectitems.length; i++)
          {
            list.push(selectitems[i].text);
          }
          return list;
        }

        Promise.all(assets)
            .catch(function (err) {
                console.log(">>> Loading Assets ... err = " + err);
            })
            .then(function (success, failure)  {


  	        scene.root.on("onKeyDown", function(e) {
                  var code = e.keyCode; var flags = e.flags;
                  if(keys.is_CTRL(e.flags))
                  {
                    iscontrolpressed = true;
                  }
                  else if (code == keys.DOWN)
                  {
                      iscontrolpressed = false;
                      if (activeItemIndex == numItems-1)
                      {
                        updateListBottom.bind(this)();
                      }
                      if (-1 != activeItemIndex)
                      {
                        text[activeItemIndex].sx = 1;
                        text[activeItemIndex].sy = 1;
                      }
                      if (activeItemIndex < numItems-1)
                      {
                        activeItemIndex++;
                      }
                      text[activeItemIndex].sx = 1.2;
                      text[activeItemIndex].sy = 1.2;
                  }
                  else if (code == keys.UP)
                  {
                      iscontrolpressed = false;
                      if (activeItemIndex == 0)
                      {
                        updateListTop.bind(this)();
                      }
                      if (-1 != activeItemIndex)
                      {
                        text[activeItemIndex].sx = 1;
                        text[activeItemIndex].sy = 1;
                      }
                      if (activeItemIndex > 0)
                      {
                        activeItemIndex--;
                      }
                      text[activeItemIndex].sx = 1.2;
                      text[activeItemIndex].sy = 1.2;
                  }
                  else if (code == keys.ENTER)
                  {
                    iscontrolpressed = false;
                    var alreadySelected = false;
                    for (var i=0; i<selectitems.length; i++)
                    {
                      if (selectitems[i] == text[activeItemIndex])
                      {
                        alreadySelected = true;
                      }
                      selectitems[i].selected = false;
                      selectitems[i].textColor = text_color;
                      selectitems.pop();
                    }
                    if (false == alreadySelected && (-1 != activeItemIndex)) /* handle user pressing ENTER without selection */
                    {
                      text[activeItemIndex].sx = 1;
                      text[activeItemIndex].sy = 1;
                      text[activeItemIndex].textColor = selection_color;
                      text[activeItemIndex].selected = true;
                      selectitems.push(text[activeItemIndex]);
                    }
                  }
                  else
                  {
                    iscontrolpressed = false;
                  }
                }.bind(self));



  	        scene.root.on("onKeyUp", function(e) {
                  var code = e.keyCode; var flags = e.flags;
                  if (code == keys.CTRL)
                  {
                    iscontrolpressed = false;
                  }
                }.bind(self));

                arrowDownImage.on("onMouseUp", function (e) {
                  updateListBottom.bind(this)();
                }.bind(self));

                arrowUpImage.on("onMouseUp", function (e) {
                  updateListTop.bind(this)();
                }.bind(self));
        });
    }
    module.exports = ListBox;

}).catch(function importFailed(err) {
    console.error("Import failed for listBox.js: " + err);
});
