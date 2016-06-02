 /*
  * File Name - nineSliceImageResource.js
  *
  *  This file defines class for nine slice image resource
  *
  */
 var Constants = require("../utils/config.js");
 var XREResource = require("./xreresource.js");
 var OnReady = require('../events/resourceEvent.js').COnReadyEvent;
 var onMetaData = require('../events/resourceEvent.js').COnImageMetadataEvent;
 var XRElogger = require('../utils/xrelogger.js');
 var ALLOW_JPEG_RESAMPLING = true; // need to check where this parameter is initialized
 var IMAGE_TYPE_DEFAULT = 0;
 var defaultLogger = XRElogger.getLogger("default");

 /**
  *class XRENineSliceImageResource
  */
 var XRENineSliceImageResource = function(id, app, params, scene) {
     var _this = this;
     var nineImageItems = [];
     var lInset = 0;
     var rInset = 0;
     var bInset = 0;
     var tInset = 0;
     var url = 0;

     /**
      *method to set the properties
      */
     this.setProperties = function(params) {

         if (params.hasOwnProperty('left') && params.left !== null) {
             lInset = params.left;
         }
         if (params.hasOwnProperty('right') && params.right !== null) {
             rInset = params.right;
         }
         if (params.hasOwnProperty('bottom') && params.bottom !== null) {
             bInset = params.bottom;
         }
         if (params.hasOwnProperty('top') && params.top !== null) {
             tInset = params.top;
         }

         if (params.hasOwnProperty('url') && params.url !== null) {
             url = params.url;

         }
         if (nineImageItems.length > 0) {
             clog(nineImageItems.length);
             clog(nineImageItems);
             for (var idx = 0; idx < nineImageItems.length; idx++) {
                 nineImageItems[idx].updateResource();
             }
         }
     };
     /**
      *method to get the url
      */
     this.getUrl = function() {
         return url;
     };
     /**
      *method to get the lInset
      */
     this.getLInset = function() {
         return lInset;
     };
     /**
      *method to get the rInset
      */
     this.getRInset = function() {
         return rInset;
     };
     /**
      *method to get the bInset
      */
     this.getBInset = function() {
         return bInset;
     };
     /**
      *method to get the tInset
      */
     this.getTInset = function() {
         return tInset;
     };
     /**
      *method to create PXNineImageItem object
      */
     this.createPXItem = function(view) {
         clog("Inside createPXItem PXNineImageItem");
         var item = {};
         clog("createPXItem");
         if (nineImageItems.length === 0 || nineImageItems[nineImageItems.length - 1].getParentView()) {
             clog("Creating new nine slice imageItem");
             item = new PXNineImageItem(view, id, app, scene, this);
             nineImageItems.push(item);
         } else {
             clog("nine slice Image item already present");
             item = nineImageItems[nineImageItems.length - 1];
             nineImageItems[nineImageItems.length - 1].setParentView(view);
             nineImageItems[nineImageItems.length - 1].updateResource();
         }
         return item;
     };
     XREResource.call(this, id, app);

     this.setProperties(params);
     var item = new PXNineImageItem(null, id, app, scene, this);
     nineImageItems.push(item);

 };
 var PXNineImageItem = function(view, id, app, scene, res) {
     clog("PXNineImageItem");
     var _this = this;
     var parentView = null;
     var resource = res;
     var isImageReady = false;
     var nineImgObject = scene.createImage9({
         url: resource.getUrl()
     });

     /**
      * Function to return image object after setting alignment
      */
     this.updateResource = function() {
         defaultLogger.log("debug", "PXNineImageItem : updateResource");
         if (nineImgObject) {
             nineImgObject.url = resource.getUrl();
             nineImgObject.ready.then(function() {
                 defaultLogger.log("info", " Image ready from PXNineImageItem");
                 defaultLogger.log("debug", "onready event PXNineImageItem======================= " + id);
                 var readyEvent = new OnReady();
                 resource.getEmitter().emit('Event', readyEvent, resource, false);
                 defaultLogger.log("debug", "onready event sent");
                 
                 var metadataEvent = new onMetaData(nineImgObject.w, nineImgObject.h);
                 resource.getEmitter().emit('Event', metadataEvent, resource);
                 defaultLogger.log("debug", "onMetaData event sent");
                 if (!_this.isImageReady) {
                     _this.isImageReady = true;
                     if (parentView) {
                         _this.setImageAlignment();
                     }
                 }
             });
             //Sending onready event since image ready not obtained when setting url 
/*             var readyEvent = new OnReady();
             resource.getEmitter().emit('Event', readyEvent, resource, false);*/

             if (nineImgObject.h === 0 || nineImgObject.w === 0) {
                 defaultLogger.log("debug", "Width,height zero>>>>>>>>>>>>>>>.");
                 this.isImageReady = false;
             } else {
                 defaultLogger.log("debug", "has width >>>>>>>>>>>>>>calling setParentView");
                 this.isImageReady = true;
                 if (parentView) {
                     this.setImageAlignment();
                 }
             }

         } else {
             defaultLogger.log("debug", "updateResource : ERROR : Image object not defined");
         }

     };
     /**
      *  Method to set parent view to image object
      **/
     this.setImageAlignment = function() {
         clog("nineImgObject w,h");
         clog(nineImgObject.w);
         clog(nineImgObject.h);
         var imgRect = parentView.getResourceRect(nineImgObject.w, nineImgObject.h);

         nineImgObject.sx = imgRect.W / nineImgObject.w;
         nineImgObject.sy = imgRect.H / nineImgObject.h;
         //nineImgObject.x = imgRect.X;
         //nineImgObject.y = imgRect.Y;
         clog(nineImgObject.sx);
         clog(nineImgObject.sy);
         //clog(nineImgObject.x);
         //clog(nineImgObject.y);

     };
     /**
      *    Function to get image object measurements
      **/
     this.getObjectRect = function() {
         var objRect = {};
         objRect.X = this.nineImgObject.x;
         objRect.Y = this.nineImgObject.y;
         objRect.W = this.nineImgObject.w;
         objRect.H = this.nineImgObject.h;
         return objRect;
     };
     this.getSceneObject = function() {
         return nineImgObject;
     };
     this.getParentView = function() {
         return parentView;
     };
     this.setParentView = function(view) {
         parentView = view;
         nineImgObject.parent = parentView.getViewObj();
         resource.registerMouseEvent(resource, nineImgObject, view);
     };
     if (view) {
         this.setParentView(view);
     }
     this.updateResource();
 };

 XRENineSliceImageResource.prototype = Object.create(XREResource.prototype);
 XRENineSliceImageResource.prototype.constructor = XRENineSliceImageResource;
 module.exports = XRENineSliceImageResource;
