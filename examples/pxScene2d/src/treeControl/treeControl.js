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

px.import({scene:'px:scene.1.js',keys: 'px:tools.keys'}).then( function ready(imports) {


var root = imports.scene.root;
var scene = imports.scene;
var keys = imports.keys;

function TreeElement(treeroot, parent, parentelem, x, y, height, indent, w, doBorder, contents, openAsCollapsed, index, expandAdjustcb, collapseAdjustcb ) 
{
    /* private variables */
    let children = [];
    let myIndex = (index === undefined || index == null) ? 0:index;
    let collapsed = (openAsCollapsed == undefined || openAsCollapsed == null) ? false: openAsCollapsed;
    let myParent = parentelem;
    let activeItem = -1;
    var treeHeight = height;
    let selectedItem = null;
    var myTreeRoot = treeroot;
    var _this = this;

    /* UI elements */
    let elem = scene.create({t:'textBox',parent:parent,h:height,w:w,x:x, y:y, wordWrap:false,truncate:scene.truncation.NONE,text:contents.name});
    let border = scene.create({t:'rect',parent:elem,lineWidth:0, a:0, id:"border"+contents.name});
    let plus = scene.create({t:'text',parent:border, text:'', a:1, x:5});
    let elemdata = contents.name;
    
    /* public methods */
    this.getTreeRoot = function() {
      return myTreeRoot;
    }
   
    this.getData = function() {
      return elemdata;
    }

    this.setTextColor = function(val) {
      elem.textColor = val;
    }

    this.setFocus = function(val) {
      border.focus = val;
      prevIndex = -1;
      activeItem = -1;
    }

    this.setSelectedItem = function(item) {
      selectedItem = item;
    }

    /* called to move down from element at index */
    this.moveDown = function(index) {
      if (children.length > 0)
      {
        var prevIndex = index;
        if (index+1 < children.length)
        {
          index = index+1;
        }
        else
        {
          index = -1;
          activeItem = -1;
        }
        activeItem = index;
        if (-1 != activeItem)
        {
          children[activeItem].setTextColor(0xff00ccff);
          children[activeItem].setFocus(true);
        }
        else
        {
          border.focus = true;
          border.parent.textColor=0xff00ccff;
        }
      }
    };

    /* called to move up from element at index */
    this.moveUp = function(index) {
          if (children.length > 0)
          {
            var prevIndex = index;
            if (index-1 >= 0)
            {
              index = index-1;
            }
            else
            {
              index = children.length;
              activeItem = children.length;
            }
            activeItem = index;
            if ((children.length != activeItem)  && (-1 != activeItem))
            {
              children[activeItem].setTextColor(0xff00ccff);
              children[activeItem].setFocus(true);
            }
            else
            {
              border.focus = true;
              border.parent.textColor=0xff00ccff;
            }
          }
     };

    this.collapse = function () {
      collapse(collapseAdjustcb);
    };

    Promise.all([elem, border, plus])
      .catch( function (err)
      {
        console.log("Exception while creating the elements of tree item");
      })
      .then( function (success, failure)
      {
            let measure = elem.measureText();
            border.h = 0;
            border.w = 0;
            border.x = -30;
            border.y = -1;
            border.a = doBorder === true?1.0:0.0;
      });

    border.moveToBack();

    /* Function to move subsequent children's position after previous
       child has been collapsed */
    var collapseAdjust = function(childIndex) {
        var len = children.length;
        var y = children[childIndex].getY() + children[childIndex].getHeight();
        for( var c = childIndex+1; c < len; c++) {
            // Move all children after this one up to fill in collapsed space
            children[c].setY(y);
            y+= children[c].getHeight();

        }
        if(collapseAdjustcb) collapseAdjustcb(myIndex);
    }

    /* Function to expand subsequent children's position after previous
       child has been expanded */
    var expandAdjust = function(childIndex) {
        var len = children.length;
        var y = children[childIndex].getY() + children[childIndex].getHeight();
        for( var c = childIndex+1; c < len; c++) {
            // Move all children after this one up to fill in collapsed space
            children[c].setY(y);
            y+= children[c].getHeight();
        }
        if( expandAdjustcb) expandAdjustcb(myIndex);
    }

    /* Function to collapse all children */
    var collapse = function(parentCollapse) {
        collapsed = true;
            var len = children.length;
            for( var c = 0; c < len; c++) {
                //children[c].collapse();
                children[c].setAlpha(0);

            } 
            if(children.length != 0)
            {
                //plus.a = 1;
                plus.text = "+";
            }
            // Pass myIndex to parent callback as starting point to 
            // adjust remaining children's locations
            if( parentCollapse !== undefined && parentCollapse != null)
                parentCollapse(myIndex);
    }.bind(this);

    /** Function to expand all children */
    var expand = function(parentExpand) {
        collapsed = false;
            var len = children.length;
            for( var c = 0; c < len; c++) {
                //children[c].expand();
                children[c].setAlpha(1);

            } 
            if (len > 0)
              plus.text = "-";
            //plus.a = 0;
            // Pass myIndex to parent callback as starting point to 
            // adjust remaining children's locations
            if( parentExpand !== undefined && parentExpand != null)
                parentExpand(myIndex);
    }.bind(this);

    let getTreeHeight = ()=>{
        if( children.length > 0) {
            var sumHeight =
            children.reduce( (sum, child)=>{
                return sum+child.getHeight();
            },0);
            return sumHeight+height;
        }
        else
            return height;
    }


    border.on("onBlur", function (e)
    {
        border.fillColor = 0x3733FF;
        border.parent.textColor=0xffffffff;
    });

    plus.on("onMouseDown", function (e)
    {
            if( collapsed === false) {
                collapse(collapseAdjustcb);
            }
            else {
                expand(expandAdjustcb);
            }
    });

    border.on("onMouseDown", function (e)
    {
        if(!border.focus) {
            border.focus = true;
        }
    });

    border.on("onFocus", function (e)
    {
        border.fillColor = 0x3733FF;
        border.parent.textColor=0xff00ccff;
    });

    border.on("onKeyDown", function(e) {
      var code = e.keyCode;
      if (code == keys.ENTER)
      {
        var treeroot = this.getTreeRoot();
        if (null != treeroot)
        {
          treeroot.setSelectedItem(this.getData());
        }
      }
      else if (code == keys.DOWN)
      {
          if ((children.length > 0) && false == collapsed)
          {
            this.moveDown(activeItem);
          }
          else
          {
            if (null != myParent)
            {
              myParent.moveDown(myIndex);
            }
            else if(false == collapsed)
            {
              this.moveDown(activeItem);
            }
          }
      }
      else if (code == keys.UP)
      {
        if ((children.length > 0) && false == collapsed)
        {
          this.moveUp(activeItem);
        }
        else
        {
          if (null != myParent)
          {
            myParent.moveUp(myIndex);
          }
          else
            this.moveUp(activeItem);
        }
      }
      else if (code == keys.RIGHT)
      {
        if (true == collapsed)
        {
          expand(expandAdjustcb);
        }
      }
      else if (code == keys.LEFT)
      {
        if ((null != myParent) && (true == collapsed))
        {
          elem.textColor = 0xffffffff;
          border.focus = false;
          myParent.setTextColor(0xff00ccff);
          myParent.setFocus(true);
        }
        activeItem = -1;
        if ((children.length > 0) && (false == collapsed))
        {
          collapse(collapseAdjustcb);
        }
        else if(null != myParent)
        {
          myParent.collapse();
        }
      }
    }.bind(this));
    var y = elem.y+height;
    var x = indent;

    if( contents.children !== undefined) {
        var len = contents.children.length;
        for( var l1 = 0; l1 < len; l1++) {
            children[l1] = new TreeElement( (null == myTreeRoot)?this:myTreeRoot, elem, _this, x, treeHeight, height, indent, w, doBorder, contents.children[l1], openAsCollapsed, l1, expandAdjust, collapseAdjust);
            treeHeight+=children[l1].getHeight();
            y += treeHeight;
        }
        if(collapsed == true) {
            collapse(null);
        } 
        if (len > 0)
        {
          elem.text = elem.text + " (" + len + ") ";
        }
    }

    return {
        getHeight: () => collapsed === true ? height : getTreeHeight(), // cumulative height of tree and its children
        collapsed: collapsed, 
        setFocus: function(val) {  
            border.focus = val;
        },
        getFocus: function() {
            return border.focus;
        },
        setAlpha: function(val) {
            elem.a = val;
        },
        setY: function(val) {
            elem.y = val;
        },
        setIndex: function(val) {
          myIndex = val;
        },
        getY: function() {
            return elem.y;
        },
        setTextColor: function(color) {
          elem.textColor = color;
        },
        getSelectedItem: function() {
          return selectedItem;
        },
        getData: function() {
          return elemdata;
        },
        collapse: function() {
          collapse(collapseAdjustcb);
        },
        expand: function() {
          expand(expandAdjustcb);
        },

        /* add the particular item from tree */
        addItem: function(item) {
            var items = item.split(":");
            //the element need to be added to my tree
            if ((items.length > 0) && (items[0] == elemdata))
            {
              // need to add children within me
              if (items.length == 2)
              {
                var isDuplicate = false;
                for (var childindex=0; childindex<children.length; childindex++)
                {
                  if (children[childindex].getData() == items[1])
                  {
                    isDuplicate = true;
                    break;
                  }
                }
                if (false == isDuplicate)
                {
                  var content = {};
                  content["name"] = items[1];
                  var newelem = new TreeElement( (null == myTreeRoot)?this:myTreeRoot, elem, _this, x, treeHeight, height, indent, w, doBorder,content, openAsCollapsed, l1, expandAdjust, collapseAdjust);
                  children.push(newelem);
                  elem.text = elemdata + " (" + children.length + ")";
                  treeHeight+=newelem.getHeight();
                  y += treeHeight;
                  collapse();
                }
              }
              else
              {
                var index = item.indexOf(":");
                if (index != -1)
                {
                  var result = item.slice(index+1);
                  for (var i=0; i<children.length; i++)
                  {
                      children[i].addItem(result);
                  }
                }
              }
            }
        },

        /* remove the particular item from tree */
        removeItem: function(item) {
            var items = item.split(":");
            //the element need to be added to my tree
            if ((items.length > 0) && (items[0] == elemdata))
            {
              //my children need to get removed
              if (items.length == 2)
              {
                var itemIndex = -1;
                var ydiff = 0;
                for (var i=0; i<children.length; i++)
                {
                  if (children[i].getData() == items[1])
                  {
                    itemIndex = i;
                    ydiff = children[i].getHeight();
                    treeHeight -= children[i].getHeight();
                    if (activeItem == i) {
                    activeItem = -1;
                    }
                    else if (activeItem != -1)
                    {
                      activeItem = activeItem-1;
                    }
                    break;
                  }
                }
                if (itemIndex > -1)
                {
                  for (var j=itemIndex+1; j<children.length; j++)
                  {
                    children[j].setY(children[j].getY()-ydiff);
                    children[j].setIndex(j-1);
                  }
                  if ((null != treeroot) && (treeroot.selectedItem() == children[itemIndex].getData()))
                  {
                    treeroot.setSelectedItem(null);
                  }
                  children[itemIndex].removeAll();
                  children[itemIndex].remove();
                  children.splice(itemIndex,1);
                  elem.text = elemdata + " (" + children.length + ")";
                }
              }
              else if (items.length == 1)
              {
                for (var i=0; i<children.length; i++)
                {   
                  var child = children.pop();
                  child.remove();
                  elem.text = elemdata + " (" + children.length + ")";
                }
                if ((null != treeroot) && treeroot.selectedItem() == elemdata)
                {
                  treeroot.setSelectedItem(null);
                }
                elem.remove();
              }
              else
              {
                var index = item.indexOf(":");
                var result = item.slice(index+1);
                for (var i=0; i<children.length; i++)
                {
                    children[i].removeItem(result);
                }
              }
            }
        },

        /* remove all the tree elements */
        removeAll: function() {
          for (var i=0; i<children.length; i++)
          {
            var child = children.pop();
            child.remove();
          }
          treeroot.setSelectedItem(null);
        },
  
        /* remove the tree element from its parent */
        remove: function() {
          elem.remove();
        },

       /* get the tree element object matching the name */
       getElement: function(item) {
         var index = -1;
         var element = null;
         for (var i=0; i<children.length; i++)
         {
           if (children[i].getData() == item)
           {
             element = children[i];
             return element;
           }
           else
           {
             element = children[i].getElement(item);
           }
         }
         return element;
       },
       
       /* get the chilren details of a tree element */
       getChildDetails: function() {
         var result = {};
         result["name"] = elemdata;
         result["childrens"] = [];
         for (var i=0; i<children.length; i++)
         {
           result["childrens"].push(children[i].getChildDetails());
           //result.push(children[i].getChildDetails());
         }
         return result;
       }
    }
}

function Tree(parent, x, y, height, indent, w, doBorder, contents, collapse)
{

    var myTreeRootElem = new TreeElement( null, parent, null, x, y, height, indent, w, doBorder, contents, collapse);
    // associate selected item with only root element

    if(collapse == true) {
        myTreeRootElem.setAlpha(1);
    }
    
    return {

        /* set (enable/disable) the focus on the tree */ 
        setFocus: function(val) {  
            myTreeRootElem.setFocus(val);
        },
 
        /* return whether the tree element got focused or not */
        getFocus: function() {
            return myTreeRootElem.getFocus();
        },

        /* gets the selected item from the tree */
        /* returns the display string of selected element */
        /* press ENTER to select the item */
        getSelectedItem: function() {
            return myTreeRootElem.getSelectedItem();
        },

        /* adds the element to tree */
        /* input need to be provided as root:child:child:element */
        /* add elem to tree root->child->child->child->elem */
        /* example : addItem("Settings:Language:German"); adds German as child of Language */
        addItem: function(item) {
          myTreeRootElem.addItem(item);
        },

       /* removes the partiular element and its childrens from tree */
        /* input need to be provided as root:child:child:element */
        /* removes the elem from tree root->child->child->child->element */
        /* example : removeItem("Settings:Language:German"); */
        removeItem: function(item) {
          myTreeRootElem.removeItem(item);
        },
 
       /* removes all the element from tree */
       removeAll: function() {
         myTreeRootElem.removeAll();
         myTreeRootElem.remove();
         delete myTreeRootElem;
       },

       /* get the particular element object from the tree matching the name */
       /* pass the exact string of element */
       /* returns JSON data in the format {"name": "", "children" : []}} */
       getElement: function(item) {
         var element = null;
         if (myTreeRootElem.getData() == item)
         {
           element = myTreeRootElem;
         }
         else
           element = myTreeRootElem.getElement(item);
         var result = null;
         if (null != element) {
            result = element.getChildDetails();
         }
         return result;
       }
    }
}

module.exports.Tree = Tree;


}).catch( err=>{
  console.error("Import failed for treeControl.js: " + err)
});
