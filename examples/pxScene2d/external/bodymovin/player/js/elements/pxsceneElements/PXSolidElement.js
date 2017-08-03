function PXSolidElement(data, comp,globalData){
    this._parent.constructor.call(this,data, comp,globalData);
}
createElement(PXBaseElement, PXSolidElement);

PXSolidElement.prototype.createElements = function(){
    this._parent.createElements.call(this);

    // Convert from CSS RGB color format to RGBA hex string
    var fillClr = this.data.sc.replace("#", "0x0");
    fillClr = fillClr + "FF"; //alpha

    var scene = imports.scene;

    this.rect  = scene.create( {t:"rect", parent: this.canvasContext, fillColor: parseInt(fillClr),
                                x: 0, y: 0, w: this.data.sw, h: this.data.sh} );
};

PXSolidElement.prototype.renderFrame = function(parentMatrix){
    if(this._parent.renderFrame.call(this, parentMatrix)===false){
        return;
    }
    var ctx = this.canvasContext;
    this.globalData.renderer.save();

    this.globalData.renderer.ctxTransform(this.finalTransform.mat.props, this.rect);
    this.globalData.renderer.ctxOpacity(this.finalTransform.opacity);

    this.globalData.renderer.restore(this.data.hasMask);
    if(this.firstFrame){
        this.firstFrame = false;
    }
};