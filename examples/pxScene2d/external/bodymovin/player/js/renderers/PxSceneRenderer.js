function PxRenderer(animationItem, config){
    this.animationItem = animationItem;
    this.renderConfig = {
        clearCanvas: (config && config.clearCanvas !== undefined) ? config.clearCanvas : true,
        context: (config && config.context) || null,
        progressiveLoad: (config && config.progressiveLoad) || false,
        preserveAspectRatio: (config && config.preserveAspectRatio) || 'xMidYMid meet'
    };
    this.renderConfig.dpr = (config && config.dpr) || 1;
    if (this.animationItem.wrapper) {
        this.renderConfig.dpr = (config && config.dpr) || window.devicePixelRatio || 1;
    }
    this.renderedFrame = -1;
    this.globalData = {
        frameNum: -1
    };
    this.contextData = {
        saved : Array.apply(null,{length:15}),
        savedOp: Array.apply(null,{length:15}),
        cArrPos : 0,
        cTr : new Matrix(),
        cO : 1
    };
    var i, len = 15;
    for(i=0;i<len;i+=1){
        this.contextData.saved[i] = Array.apply(null,{length:16});
    }
    this.elements = [];
    this.pendingElements = [];
    this.transformMat = new Matrix();
    this.completeLayers = false;
}
extendPrototype(BaseRenderer,PxRenderer);

PxRenderer.prototype.createBase = function (data) {

    console.log(" GOT >>> PxRenderer.prototype.createBase() ");

    return new PXBaseElement(data, this, this.globalData);
};

PxRenderer.prototype.createShape = function (data) {

    console.log(" GOT >>> PxRenderer.prototype.createShape() ");

    return new PXShapeElement(data, this, this.globalData);
};

PxRenderer.prototype.createText = function (data) {

    console.log(" GOT >>> PxRenderer.prototype.createText() ");
    
        return new PXTextElement(data, this, this.globalData);
};

PxRenderer.prototype.createImage = function (data) {

    console.log(" GOT >>> PxRenderer.prototype.createImage() ");
    
        return new PXImageElement(data, this, this.globalData);
};

PxRenderer.prototype.createComp = function (data) {

    console.log(" GOT >>> PxRenderer.prototype.createComp() ");

    return new PXCompElement(data, this, this.globalData);
};

PxRenderer.prototype.createSolid = function (data) {

    console.log(" GOT >>> PxRenderer.prototype.createSolid() ");

    return new PXSolidElement(data, this, this.globalData);
};

PxRenderer.prototype.ctxTransform = function(props, element){

//    console.log(" GOT >>> PxRenderer.prototype.ctxTransform() ");

    if(props[0] === 1 && props[1] === 0 && props[4] === 0 && props[5] === 1 && props[12] === 0 && props[13] === 0){
        return;
    }
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.transform(props[0],props[1],props[4],props[5],props[12],props[13]);
        return;
    }
    this.transformMat.cloneFromProps(props);
    this.transformMat.transform(this.contextData.cTr.props[0],this.contextData.cTr.props[1],this.contextData.cTr.props[2],this.contextData.cTr.props[3],this.contextData.cTr.props[4],this.contextData.cTr.props[5],this.contextData.cTr.props[6],this.contextData.cTr.props[7],this.contextData.cTr.props[8],this.contextData.cTr.props[9],this.contextData.cTr.props[10],this.contextData.cTr.props[11],this.contextData.cTr.props[12],this.contextData.cTr.props[13],this.contextData.cTr.props[14],this.contextData.cTr.props[15])

    this.contextData.cTr.cloneFromProps(this.transformMat.props);
    var trProps = this.contextData.cTr.props;

    if(element)
    {
        // console.log(" GOT >>> PxRenderer.prototype.ctxTransform(element)   ##");

        // ROW MAJOR 
        element.m11 = trProps[0];
        element.m12 = trProps[1];
        element.m13 = trProps[2];
        element.m14 = trProps[3];  // DX

        element.m21 = trProps[4];
        element.m22 = trProps[5];
        element.m23 = trProps[6];
        element.m24 = trProps[7];  // DY

        element.m31 = trProps[8];
        element.m32 = trProps[9];
        element.m33 = trProps[10];
        element.m34 = trProps[11]; // DZ

        element.m41 = trProps[12];
        element.m42 = trProps[13];
        element.m43 = trProps[14];
        element.m44 = trProps[15];

        element.useMatrix = true;
    }
};


PxRenderer.prototype.ctxOpacity = function(op){

//    console.log(" GOT >>> PxRenderer.prototype.ctxOpacity() ");

    if(op === 1){
        return;
    }
    if(!this.renderConfig.clearCanvas){
//        this.canvasContext.globalAlpha *= op < 0 ? 0 : op;
        this.canvasContext.a *= op < 0 ? 0 : op;
        return;
    }
    this.contextData.cO *= op < 0 ? 0 : op;
//    this.canvasContext.globalAlpha = this.contextData.cO;
  this.canvasContext.a = this.contextData.cO;
};

PxRenderer.prototype.reset = function(){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.restore();
        return;
    }
    this.contextData.cArrPos = 0;
    this.contextData.cTr.reset();
    this.contextData.cO = 1;
};

PxRenderer.prototype.save = function(actionFlag){
    if(!this.renderConfig.clearCanvas){
        this.canvasContext.save();
        return;
    }
    if(actionFlag){
        this.canvasContext.save();
    }
    var props = this.contextData.cTr.props;
    if(this.contextData.saved[this.contextData.cArrPos] === null || this.contextData.saved[this.contextData.cArrPos] === undefined){
        this.contextData.saved[this.contextData.cArrPos] = new Array(16);
    }
    var i,arr = this.contextData.saved[this.contextData.cArrPos];
    for(i=0;i<16;i+=1){
        arr[i] = props[i];
    }
    this.contextData.savedOp[this.contextData.cArrPos] = this.contextData.cO;
    this.contextData.cArrPos += 1;
};

PxRenderer.prototype.restore = function(actionFlag){
    if(!this.renderConfig.clearCanvas){
        return;
    }
    this.contextData.cArrPos -= 1;
    var popped = this.contextData.saved[this.contextData.cArrPos];
    var i,arr = this.contextData.cTr.props;
    for(i=0;i<16;i+=1){
        arr[i] = popped[i];
    }
    popped = this.contextData.savedOp[this.contextData.cArrPos];
    this.contextData.cO = popped;
};


PxRenderer.prototype.configAnimation = function(animData){

    if(this.animationItem.wrapper){
        this.animationItem.container = document.createElement('pxscene');
        this.animationItem.wrapper.appendChild(this.animationItem.container);
          this.canvasContext = this.animationItem.wrapper.getContext();
    }else{
        this.canvasContext = this.renderConfig.context;
    }
    this.data = animData;
    this.globalData.canvasContext = this.canvasContext;
    this.globalData.renderer = this;
    this.globalData.isDashed = false;
    this.globalData.totalFrames = Math.floor(animData.tf);
    this.globalData.compWidth = animData.w;
    this.globalData.compHeight = animData.h;
    this.globalData.frameRate = animData.fr;
    this.globalData.frameId = 0;
    this.globalData.compSize = {
        w: animData.w,
        h: animData.h
    };
    this.globalData.progressiveLoad = this.renderConfig.progressiveLoad;
    this.layers = animData.layers;
    this.transformCanvas = {};
    this.transformCanvas.w = animData.w;
    this.transformCanvas.h = animData.h;
    this.globalData.fontManager = new FontManager();
    this.globalData.fontManager.addChars(animData.chars);
    this.globalData.fontManager.addFonts(animData.fonts,document.body);
    this.globalData.getAssetData = this.animationItem.getAssetData.bind(this.animationItem);
    this.globalData.getAssetsPath = this.animationItem.getAssetsPath.bind(this.animationItem);
    this.globalData.elementLoaded = this.animationItem.elementLoaded.bind(this.animationItem);
    this.globalData.addPendingElement = this.animationItem.addPendingElement.bind(this.animationItem);
    this.globalData.transformCanvas = this.transformCanvas;
    this.elements = Array.apply(null,{length:animData.layers.length});

    this.updateContainerSize();
};

PxRenderer.prototype.updateContainerSize = function () {

    console.log(" GOT >>> PxRenderer.prototype.updateContainerSize() ");
    
    var elementWidth,elementHeight;
    if(this.animationItem.wrapper && this.animationItem.container){
        elementWidth  = this.animationItem.wrapper.offsetWidth;
        elementHeight = this.animationItem.wrapper.offsetHeight;
    }else{
        elementWidth  = this.canvasContext.w * this.renderConfig.dpr;
        elementHeight = this.canvasContext.h * this.renderConfig.dpr;
    }
    var elementRel,animationRel;
    if(this.renderConfig.preserveAspectRatio.indexOf('meet') !== -1 || this.renderConfig.preserveAspectRatio.indexOf('slice') !== -1){
        var par = this.renderConfig.preserveAspectRatio.split(' ');
        var fillType = par[1] || 'meet';
        var pos = par[0] || 'xMidYMid';
        var xPos = pos.substr(0,4);
        var yPos = pos.substr(4);
        elementRel = elementWidth/elementHeight;
        animationRel = this.transformCanvas.w/this.transformCanvas.h;
        if(animationRel>elementRel && fillType === 'meet' || animationRel<elementRel && fillType === 'slice'){
            this.transformCanvas.sx = elementWidth/(this.transformCanvas.w/this.renderConfig.dpr);
            this.transformCanvas.sy = elementWidth/(this.transformCanvas.w/this.renderConfig.dpr);
        }else{
            this.transformCanvas.sx = elementHeight/(this.transformCanvas.h / this.renderConfig.dpr);
            this.transformCanvas.sy = elementHeight/(this.transformCanvas.h / this.renderConfig.dpr);
        }

        if(xPos === 'xMid' && ((animationRel<elementRel && fillType==='meet') || (animationRel>elementRel && fillType === 'slice'))){
            this.transformCanvas.tx = (elementWidth-this.transformCanvas.w*(elementHeight/this.transformCanvas.h))/2*this.renderConfig.dpr;
        } else if(xPos === 'xMax' && ((animationRel<elementRel && fillType==='meet') || (animationRel>elementRel && fillType === 'slice'))){
            this.transformCanvas.tx = (elementWidth-this.transformCanvas.w*(elementHeight/this.transformCanvas.h))*this.renderConfig.dpr;
        } else {
            this.transformCanvas.tx = 0;
        }
        if(yPos === 'YMid' && ((animationRel>elementRel && fillType==='meet') || (animationRel<elementRel && fillType === 'slice'))){
            this.transformCanvas.ty = ((elementHeight-this.transformCanvas.h*(elementWidth/this.transformCanvas.w))/2)*this.renderConfig.dpr;
        } else if(yPos === 'YMax' && ((animationRel>elementRel && fillType==='meet') || (animationRel<elementRel && fillType === 'slice'))){
            this.transformCanvas.ty = ((elementHeight-this.transformCanvas.h*(elementWidth/this.transformCanvas.w)))*this.renderConfig.dpr;
        } else {
            this.transformCanvas.ty = 0;
        }

    }else if(this.renderConfig.preserveAspectRatio == 'none'){
        this.transformCanvas.sx = elementWidth/(this.transformCanvas.w/this.renderConfig.dpr);
        this.transformCanvas.sy = elementHeight/(this.transformCanvas.h/this.renderConfig.dpr);
        this.transformCanvas.tx = 0;
        this.transformCanvas.ty = 0;
    }else{
        this.transformCanvas.sx = this.renderConfig.dpr;
        this.transformCanvas.sy = this.renderConfig.dpr;
        this.transformCanvas.tx = 0;
        this.transformCanvas.ty = 0;
    }
    this.transformCanvas.props = [this.transformCanvas.sx,0,0,0,0,this.transformCanvas.sy,0,0,0,0,1,0,this.transformCanvas.tx,this.transformCanvas.ty,0,1];
    var i, len = this.elements.length;
    for(i=0;i<len;i+=1){
        if(this.elements[i] && this.elements[i].data.ty === 0){
            this.elements[i].resize(this.globalData.transformCanvas);
        }
    }
};

PxRenderer.prototype.destroy = function () {
    if(this.renderConfig.clearCanvas) {
        this.animationItem.wrapper.innerHTML = '';
    }
    var i, len = this.layers ? this.layers.length : 0;
    for (i = len - 1; i >= 0; i-=1) {
        this.elements[i].destroy();
    }
    this.elements.length = 0;
    this.globalData.canvasContext = null;
    this.animationItem.container = null;
    this.destroyed = true;
};

PxRenderer.prototype.renderFrame = function(num){

//    console.log(" >>>>>>>>>>>>>>>> PxRenderer.prototype.renderFrame("+num+") ");
    
    if(this.renderedFrame == num || this.destroyed){
        return;
    }

    this.renderedFrame = num;

    this.globalData.frameNum = num;
    this.globalData.frameId += 1;
    this.globalData.projectInterface.currentFrame = num;

    //console.log('--------');
    //console.log('NEW: ',num);
    var i, len = this.layers.length;
    if(!this.completeLayers){
        this.checkLayers(num);
    }
    for (i = 0; i < len; i++) {
        if(this.completeLayers || this.elements[i]){
            this.elements[i].prepareFrame(num - this.layers[i].st);
        }
    }
    for (i = len - 1; i >= 0; i-=1) {
        if(this.completeLayers || this.elements[i]){
            this.elements[i].renderFrame();
        }
    }
};

PxRenderer.prototype.buildItem = function(pos){
    var elements = this.elements;
    if(elements[pos] || this.layers[pos].ty == 99){
        return;
    }
    var element = this.createItem(this.layers[pos], this,this.globalData);
    elements[pos] = element;
    element.initExpressions();
    if(this.layers[pos].ty === 0){
        element.resize(this.globalData.transformCanvas);
    }
};

PxRenderer.prototype.checkPendingElements  = function(){

    console.log(" GOT >>> PxRenderer.prototype.checkPendingElements() ");
    
        while(this.pendingElements.length){
        var element = this.pendingElements.pop();
        element.checkParenting();
    }
};

PxRenderer.prototype.hide = function(){
    this.animationItem.container.style.display = 'none';
};

PxRenderer.prototype.show = function(){
    this.animationItem.container.style.display = 'block';
};

PxRenderer.prototype.searchExtraCompositions = function(assets){
    var i, len = assets.length;
    var floatingContainer = document.createElementNS(svgNS,'g');
    for(i=0;i<len;i+=1){
        if(assets[i].xt){
            var comp = this.createComp(assets[i],this.globalData.comp,this.globalData);
            comp.initExpressions();
            //comp.compInterface = CompExpressionInterface(comp);
            //Expressions.addLayersInterface(comp.elements, this.globalData.projectInterface);
            this.globalData.projectInterface.registerComposition(comp);
        }
    }
};