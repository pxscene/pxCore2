var root = scene.root;

var fontSize = 24;

function HorizontalMenu(scene, items) {
  this._scene = scene;
  this._textItems = new Array();
  this._items = items;
  this._selectedText = 0;
  this._selectedColor = 0x598ee3ff;
  this._wordSpacing = 12;
  this._animationSpeed = 0.75;
  this._leftPadding = 10;

  var length = 0;

  // position items
  var xCoord = this._leftPadding;
  var yCoord = 10;

  this._topLine = this._scene.createRectangle({fillColor:0xffffffff, y:this._leftPadding, w:this._scene.w,
    h:2, parent:this._scene.root});
  this._topLine.visible = true;

  this._bottomLine = this._scene.createRectangle({fillColor:0xffffffff, y:fontSize +
      yCoord + 12, w:this._scene.w, h:2, parent:this._scene.root});
  this._bottomLine.visible = true;

  this._topSelectionLine = this._scene.createRectangle({fillColor:this._selectedColor,
      parent:this._scene.root, y:this._topLine.y, x:this._leftPadding, h:2});
  this._topSelectionLine.visible = true;
    
  this._bottomSelectionLine = this._scene.createRectangle({fillColor:this._selectedColor,
    parent:this._scene.root, y:this._bottomLine.y, x:this._leftPadding, h:2});
  this._bottomSelectionLine.visible = true;

  for (t in this._items) {
    var txtItem = this._scene.createText({x:xCoord,y:yCoord, parent: this._scene.root,
      clip: true, textColor: 0xffffffff, pixelSize: fontSize, text: items[t]});
    xCoord += txtItem.w + this._wordSpacing;
    txtItem.visible = true;
    this._textItems[this._textItems.length] = txtItem;
    length += xCoord;
  }

  this._selectedText = 0;
  this._items[this._selectedText].fillColor = this._selectedColor;
  this._topSelectionLine.w = this.widthOfItem(0);
  this._bottomSelectionLine.w = this.widthOfItem(0);
}

HorizontalMenu.prototype.widthOfItem = function(i) {
  return this._textItems[i].w;
}

HorizontalMenu.prototype.moveSelection = function(menu, src, dest, xpos, width)
{
  menu._selectedText = dest;
  this._bottomSelectionLine.animateTo({w:width}, this._animationSpeed, scene.PX_STOP, 0);
  this._topSelectionLine.animateTo({w:width}, this._animationSpeed, scene.PX_STOP, 0);

  this._topSelectionLine.animateTo({x:xpos}, this._animationSpeed, scene.PX_STOP, 0);
  this._bottomSelectionLine.animateTo({x:xpos},  this._animationSpeed, scene.PX_STOP, 0, function(e) {
    // Is this a BUG? "this" doesn't appear to be HorizontalMenu

    // changing text color doesn't appear to work
    menu._textItems[dest].fillColor = menu._selectedColor;
    menu._textItems[src].fillColor = 0xffffffff;
  });
}

HorizontalMenu.prototype.next = function() {
  var idx = this._selectedText + 1;
  if (idx > this._items.lenght)
    idx = 0;

  this.moveSelection(this, this._selectedText, idx, this._textItems[idx].x,
      this.widthOfItem(idx));
}

HorizontalMenu.prototype.prev = function() {
  var idx = this._selectedText - 1;
  if (idx < 0)
    idx = this._items.length - 1;

  this.moveSelection(this, this._selectedText, idx, this._textItems[idx].x,
      this.widthOfItem(idx));
}

var menuItems =  ["Movies", "TV", "Kids", "Networks", "Music", "Latino",
  "StreamPix", "Sports & Fitness", "Multicultural", "Local", "Searchlight",
  "XFINITY Services"];

var menu = new HorizontalMenu(scene, menuItems);

scene.on("onKeyDown", function(e) {
  if (e.keyCode == 39) {
    menu.next();
  }
  else if (e.keyCode == 37) {
    menu.prev();
  }
});

