function PXImageElement(data,parentContainer,globalData,comp,placeholder){
    this.assetData = globalData.getAssetData(data.refId);
    this._parent.constructor.call(this,data,parentContainer,globalData,comp,placeholder);
}
createElement(PXBaseElement, PXImageElement);

PXImageElement.prototype.createElements = function(){

    this._parent.createElements.call(this);

    var assetData = this.assetData;
    var assetPath = px.getPackageBaseFilePath() + "/"  + this.globalData.getAssetsPath(assetData);

    var scene = imports.scene;

    this.img  = scene.create( {t:"image", url: assetPath, parent: this.canvasContext,
                                        w: parseInt(assetData.w), h: parseInt(assetData.h), id: assetData.id,
                                stretchX:scene.stretch.STRETCH, stretchY:scene.stretch.STRETCH} );
    var self = this;

    this.img.ready.then(
        function(d)
        {
            console.log("IMAGE LOADED");
            self.globalData.elementLoaded();
        },
        function(o)
        {
            console.log("IMAGE FAILED");
        }
    );
};


PXImageElement.prototype.renderFrame = function(parentMatrix){

    // console.log("PXImageElement >> renderFrame() - ENTER"); // HACK

    if(this.failed){
        return;
    }
    if(this._parent.renderFrame.call(this,parentMatrix)===false){
        return;
    }

    this.globalData.renderer.save();
    
    var finalMat = this.finalTransform.mat.props;
    var finalOp  = this.finalTransform.opacity;

    var element = this.img;

    this.globalData.renderer.ctxTransform(finalMat, element);
    this.globalData.renderer.ctxOpacity(this.finalTransform.opacity);

//    console.log("PXImageElement >> renderFrame() - finalMat = " + JSON.stringify(finalMat ) ); // HACK

    this.globalData.renderer.restore(this.data.hasMask);

    if(this.firstFrame){
        this.firstFrame = false;
    }    
};

PXImageElement.prototype.destroy = function(){
    this._parent.destroy.call(this._parent);
    this.img = null;
};