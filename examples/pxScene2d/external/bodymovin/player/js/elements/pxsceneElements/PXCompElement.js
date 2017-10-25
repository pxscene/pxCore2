function PXCompElement(data, comp,globalData){
    this._parent.constructor.call(this,data, comp,globalData);
    var compGlobalData = {};
    for(var s in globalData){
        if(globalData.hasOwnProperty(s)){
            compGlobalData[s] = globalData[s];
        }
    }
    compGlobalData.renderer = this;
    compGlobalData.compHeight = this.data.h;
    compGlobalData.compWidth = this.data.w;
    this.renderConfig = {
        clearCanvas: true
    };
    this.contextData = {
        saved : Array.apply(null,{length:15}),
        savedOp: Array.apply(null,{length:15}),
        cArrPos : 0,
        cTr : new Matrix(),
        cO : 1
    };
    this.completeLayers = false;
    var i, len = 15;
    for(i=0;i<len;i+=1){
        this.contextData.saved[i] = Array.apply(null,{length:16});
    }
    this.transformMat = new Matrix();
    this.parentGlobalData = this.globalData;
    var cv = document.createElement('pxscene');
    //document.body.appendChild(cv);
    compGlobalData.canvasContext = cv.getContext('px2d');
    this.canvasContext = compGlobalData.canvasContext;
    cv.width = this.data.w;
    cv.height = this.data.h;
    this.canvas = cv;
    this.globalData = compGlobalData;
    this.layers = data.layers;
    this.pendingElements = [];
    this.elements = Array.apply(null,{length:this.layers.length});
    if(this.data.tm){
        this.tm = PropertyFactory.getProp(this,this.data.tm,0,globalData.frameRate,this.dynamicProperties);
    }
    if(this.data.xt || !globalData.progressiveLoad){
        this.buildAllItems();
    }
}
createElement(PXBaseElement, PXCompElement);

PXCompElement.prototype.ctxTransform = PxRenderer.prototype.ctxTransform;
PXCompElement.prototype.ctxOpacity = PxRenderer.prototype.ctxOpacity;
PXCompElement.prototype.save = PxRenderer.prototype.save;
PXCompElement.prototype.restore = PxRenderer.prototype.restore;
PXCompElement.prototype.reset =  function(){
    this.contextData.cArrPos = 0;
    this.contextData.cTr.reset();
    this.contextData.cO = 1;
};
PXCompElement.prototype.resize = function(transformCanvas){
    var maxScale = Math.max(transformCanvas.sx,transformCanvas.sy);
    this.canvas.width = this.data.w*maxScale;
    this.canvas.height = this.data.h*maxScale;
    this.transformCanvas = {
        sc:maxScale,
        w:this.data.w*maxScale,
        h:this.data.h*maxScale,
        props:[maxScale,0,0,0,0,maxScale,0,0,0,0,1,0,0,0,0,1]
    }
    var i,len = this.elements.length;
    for( i = 0; i < len; i+=1 ){
        if(this.elements[i] && this.elements[i].data.ty === 0){
            this.elements[i].resize(transformCanvas);
        }
    }
};

// PXCompElement.prototype.prepareFrame = function(num){
//     this.globalData.frameId = this.parentGlobalData.frameId;
//     this.globalData.mdf = false;
//     this._parent.prepareFrame.call(this,num);
//     if(this.isVisible===false && !this.data.xt){
//         return;
//     }
//     var timeRemapped = num;
//     if(this.tm){
//         timeRemapped = this.tm.v;
//         if(timeRemapped === this.data.op){
//             timeRemapped = this.data.op - 1;
//         }
//     }
//     this.renderedFrame = timeRemapped/this.data.sr;
//     var i,len = this.elements.length;
//
//     if(!this.completeLayers){
//         this.checkLayers(num);
//     }
//
//     for( i = 0; i < len; i+=1 ){
//         if(this.completeLayers || this.elements[i]){
//             this.elements[i].prepareFrame(timeRemapped/this.data.sr - this.layers[i].st);
//             if(this.elements[i].data.ty === 0 && this.elements[i].globalData.mdf){
//                 this.globalData.mdf = true;
//             }
//         }
//     }
//     if(this.globalData.mdf && !this.data.xt){
//         this.canvasContext.clearRect(0, 0, this.data.w, this.data.h);
//         this.ctxTransform(this.transformCanvas.props);
//     }
// };

PXCompElement.prototype.renderFrame = function(parentMatrix){
    if(this._parent.renderFrame.call(this,parentMatrix)===false){
        return;
    }
    if(this.globalData.mdf){
        var i,len = this.layers.length;
        for( i = len - 1; i >= 0; i -= 1 ){
            if(this.completeLayers || this.elements[i]){
                this.elements[i].renderFrame();
            }
        }
    }
    if(this.data.hasMask){
        this.globalData.renderer.restore(true);
    }
    if(this.firstFrame){
        this.firstFrame = false;
    }
    this.parentGlobalData.renderer.save();
    this.parentGlobalData.renderer.ctxTransform(this.finalTransform.mat.props);
    this.parentGlobalData.renderer.ctxOpacity(this.finalTransform.opacity);
    this.parentGlobalData.renderer.canvasContext.drawImage(this.canvas,0,0,this.data.w,this.data.h);
    this.parentGlobalData.renderer.restore();

    if(this.globalData.mdf){
        this.reset();
    }
};

PXCompElement.prototype.setElements = function(elems){
    this.elements = elems;
};

PXCompElement.prototype.getElements = function(){
    return this.elements;
};

PXCompElement.prototype.destroy = function(){
    var i,len = this.layers.length;
    for( i = len - 1; i >= 0; i -= 1 ){
        this.elements[i].destroy();
    }
    this.layers = null;
    this.elements = null;
    this._parent.destroy.call(this._parent);
};
PXCompElement.prototype.checkLayers = PxRenderer.prototype.checkLayers;
PXCompElement.prototype.buildItem = PxRenderer.prototype.buildItem;
PXCompElement.prototype.checkPendingElements = PxRenderer.prototype.checkPendingElements;
PXCompElement.prototype.addPendingElement = PxRenderer.prototype.addPendingElement;
PXCompElement.prototype.buildAllItems = PxRenderer.prototype.buildAllItems;
PXCompElement.prototype.createItem = PxRenderer.prototype.createItem;
PXCompElement.prototype.createImage = PxRenderer.prototype.createImage;
PXCompElement.prototype.createComp = PxRenderer.prototype.createComp;
PXCompElement.prototype.createSolid = PxRenderer.prototype.createSolid;
PXCompElement.prototype.createShape = PxRenderer.prototype.createShape;
PXCompElement.prototype.createText = PxRenderer.prototype.createText;
PXCompElement.prototype.createBase = PxRenderer.prototype.createBase;
PXCompElement.prototype.buildElementParenting = PxRenderer.prototype.buildElementParenting;