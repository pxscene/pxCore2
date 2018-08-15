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

        // visible property
        this._visible = false;

        // PUBLIC methods
        //
        this.addItem     = addItem;
        this.removeItem      = removeItem;
        this.selectedItem = selectedItem;
        this.selectedItemList = selectedItemList;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        var self       = this;
        var x = 0;
        var y = 0;
        var w = 0;
        var h = 0;
        // number of visible items
        var numItems = 0;
        // total number of items in list
        var totalItems = 0;

        // list to store all the items added to list
        var items = [];
        var itemdetails = {};

        // index pointing to current cursor marking position from the list of visible items
        var activeItemIndex = -1;
        // index to indicate start and end position of visible items from main list
        var visibleStartIndex = 0;
        var visibleEndIndex = -1;

        // textbox array holding only the visible items
        var text = [];
        // array holding the small rect's which act as seperator lines between textbox elements
        var seperators = [];

        var yindex = 0;
        var iscontrolpressed = false;
        var mouseActive = false;

        x =               "x" in params ? params.x               : 0;
        y =               "y" in params ? params.y               : 0;
        w =               "w" in params ? params.w               : 200;
        h =               "h" in params ? params.h               : 200;
        numItems =               "numItems" in params ? params.numItems               : 3;
        this._visible =               "visible" in params ? params.visible               : false;

        // spark objects
        var          parent =          "parent" in params ? params.parent          : scene.root;
        var             pts =             "pts" in params ? params.pts             : 18;
        var            font =            "font" in params ? params.font            : "FreeSans.ttf";
        var      text_color =      "text_color" in params ? params.text_color      : 0x303030ff;
        var selection_color = "selection_color" in params ? params.selection_color : 0xCC000088;

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


          text[i].on("onMouseUp", function (e) {
              mouseActive = true;
              if (false == iscontrolpressed)
              {
                var alreadySelected = false;
                for (var item in itemdetails)
                {
                  if (itemdetails[this.text].selected == true)
                  {
                    alreadySelected = true;
                  }
                  activeItemIndex = -1;
                  itemdetails[item].selected = false;
                  if ((undefined != itemdetails[item].text) && (null != itemdetails[item].text))
                    itemdetails[item].text.textColor = text_color;
                }

                if (false == alreadySelected)
                {
                  itemdetails[this.text].selected = true;
                  //itemdetails[this.text].text = this;
                  this.textColor = selection_color;

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
                if (itemdetails[this.text].selected == true)
                {
                  itemdetails[this.text].selected = false;
                  this.textColor = text_color;
                }
                else
                {
                  this.textColor = selection_color;
                  itemdetails[this.text].selected = true;
                 // itemdetails[this.text].text = this;
                }
              }
          }.bind(text[i]));
        }

        /* rearrange the list on scroll down */
        function updateListBottom()
        {
          if (visibleEndIndex != totalItems-1)
          {
            var data = "";
            itemdetails[text[0].text].text = null;
            for (var index=1; index<numItems; index++)
            {
              data = text[index].text;
              text[index-1].text = data;
              if (itemdetails[data].selected == true)
              {
                text[index-1].textColor = selection_color;
              }
              else
              {
                text[index-1].textColor = text_color;
              }
              itemdetails[data].text = text[index-1];
            }
            visibleEndIndex++;
            visibleStartIndex++;
            data = items[visibleEndIndex];
            text[numItems-1].text = data;
            if (itemdetails[data].selected == true)
            {
              text[numItems-1].textColor = selection_color;
            }
            else
            {
              text[numItems-1].textColor = text_color;
            }
            itemdetails[data].text = text[numItems-1];
          }
        }

        /* rearrange the list on scroll up */
        function updateListTop()
        {
          if (visibleStartIndex != 0)
          {
            var data = "";
            itemdetails[text[numItems-1].text].text = null;
            for (var index=numItems-1; index>0; index--)
            {
              data = text[index-1].text;
              text[index].text = data;
              if (itemdetails[data].selected == true)
              {
                text[index].textColor = selection_color;
              }
              else
              {
                text[index].textColor = text_color;
              }
              itemdetails[data].text = text[index];
            }
            visibleEndIndex--;
            visibleStartIndex--;
            data = items[visibleStartIndex];
            text[0].text = data;
            if (itemdetails[data].selected == true)
            {
              text[0].textColor = selection_color;
            }
            else
            {
              text[0].textColor = text_color;
            }
            itemdetails[data].text = text[0];
          }
        }

        /* add item to the list */
        function addItem(data)
        {
          var index = items.indexOf(data);
          if (-1 == index)
          {
            itemdetails[data] = {};
            itemdetails[data].text = null;
            itemdetails[data].selected = false;
            items.push(data);
            if (totalItems < numItems)
            {
              text[totalItems].text = data;
              text[totalItems].draw = true;
              itemdetails[data].text = text[totalItems];
              visibleEndIndex = totalItems;
            }
            totalItems++;
          }
        }

        /* remove item from the list */
        function removeItem(data)
        {
          //remove from the total items list
          var index = items.indexOf(data);
	  if (index > -1) {
	    items.splice(index, 1);
            totalItems--;
          }

          //remove from the visible items list
          index = -1;
          for (var i=0; i<numItems; i++)
          {
            if (text[i].text == data)
            {
              text[i].textColor = text_color;
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

          if (data in itemdetails)
          {
            itemdetails[data].text = null;
            delete itemdetails[data].selected;
            delete itemdetails[data].text;
            delete itemdetails[data];
          }
        }

        /* return the selected item when user selected only one item */
        function selectedItem()
        {
          var result = "UNAVAILABLE";
          for (var item in itemdetails)
          {
            if (itemdetails[item].selected == true)
            {
              result = item;
              break;
            }
          }
          return result;
        }

        /* return the list of selected items when user selected more items */
        function selectedItemList()
        {
          var list = [];
          for (var item in itemdetails)
          {
            if (itemdetails[item].selected == true)
              list.push(item);
          }
          return list;
        }

        Promise.all(assets)
            .catch(function (err) {
                console.log(">>> Loading Assets ... err = " + err);
            })
            .then(function (success, failure)  {


  	        scene.root.on("onPreKeyDown", function(e) {
                  var code = e.keyCode;
                  if (code == keys.ENTER)
                  {
                    if ((false == mouseActive) && (-1 != activeItemIndex))
                    {
                      var data = text[activeItemIndex].text;
                      iscontrolpressed = false;
                      var alreadySelected = false;
                      if (itemdetails[data].selected == true)
                      {
                        alreadySelected = true;
                      }
                      for (var item in itemdetails)
                      {
                        itemdetails[item].selected = false;
                        if ((undefined != itemdetails[item].text) && (null != itemdetails[item].text))
                        {
                          itemdetails[item].text.textColor = text_color;
                        }
                      }
                      if (false == alreadySelected && (-1 != activeItemIndex)) /* handle user pressing ENTER without selection */
                      {
                        text[activeItemIndex].sx = 1;
                        text[activeItemIndex].sy = 1;
                        itemdetails[data].text.textColor = selection_color;
                        itemdetails[data].selected = true;
                      }
                    }
                    mouseActive = false;
                  }
                });

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

        scene.on('onClose', function(e) {
          for (var item in itemdetails)
          {
            for(var key in itemdetails[item]) {
              delete itemdetails[item][key];
            }
            delete itemdetails[item];
          }
        });
    }
    module.exports = ListBox;

}).catch(function importFailed(err) {
    console.error("Import failed for listBox.js: " + err);
});
