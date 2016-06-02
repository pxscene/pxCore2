function WrappedText(obj) {
	this._container = null;
	this._textColor = null;
	this._pixelSize = null;
	this._leading = 0;
	this._kearning = 0;
	this.setProps(obj);
	this._text = null;
}
 
WrappedText.prototype.setProps = new function(obj) {
	var temp = new Object();
//	if (obj.x) temp.x = obj.x;
//	if (obj.) temp. = obj.x;

//	this._text = text;
//	this.update();
}

WrappedText.prototype.update = new function() {
	var length = this._text.length;
	var px = 0, py = 0;
	for (i=0; i<length; i++) {
		var temp = scene.create({t:"text", parent:this._container, textColor:this._textColor, pixelSize:this._pixelSize, x:px, y:py});
		temp.text = this._text.charAt(i);
		//temp.cx = temp.w/2;
		//temp.cy = temp.h/2;
		if (px + temp.w > container.w) {
			py += temp.h + leading;
			px = 0;
			temp.x=px;
			temp.y=py;
		}
		px += temp.w + kearning;
	}
}
 
