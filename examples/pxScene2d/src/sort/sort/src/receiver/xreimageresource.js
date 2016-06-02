 /*
  * File Name - imageresource.js
  *
  *  This file defines class for image resource
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

 //used to set the quality of image
 var ImageQuality = {
     "ImageQualityNone": 0,
     "ImageQualityLow": 1,
     "ImageQualityHigh": 2
 };

 /*
  *  Class CImageResource
  *  Creates a image object and set properties
  **/
 CImageResource = function(id, app, params, scene) {
     //Calling parent constructor
     XREResource.call(this, id, app);

     var imageItems = [];
     //Setting default values 
     var resamblingEnabled = ALLOW_JPEG_RESAMPLING;
     var resamplingDimensions = [];
     var imgHeight = 0;
     var imageType = IMAGE_TYPE_DEFAULT;
     var imageQuality = ImageQuality.ImageQualityNone;
     var imgUrl = "";
     this.isURLSet = false;

     /**
      * setProperties is a method of class CImageResource, used for setting properties of image resource
      */
     this.setProperties = function(params) {

         if (resamblingEnabled && params.hasOwnProperty('resamplingDimensions') && params.resamplingDimensions !== null) {
             resamplingDimensions = params.resamplingDimensions;
         }
         if (params.hasOwnProperty('imageType') && params.imageType !== null) {
             imageType = params.imageType.toUpperCase();
         }
         if (params.hasOwnProperty('quality') && params.quality !== null) {
             imageQuality = this.imageQualityFromString(params.quality);
         }
         if (params.hasOwnProperty('url') && params.url !== null) {
             imgUrl = params.url;
             this.isURLSet = true;
         }
         //Updating all resource items
         if (imageItems.length > 0) {
             for (var idx = 0; idx < imageItems.length; idx++) {
                 imageItems[idx].updateResource();
             }
         }
     };

     /**
      * imageQualityFromString is a method of class CImageResource, used for getting quality value corresponding to the argument ‘quality’
      */
     this.imageQualityFromString = function(quality) {
         if (quality.toLocaleLowerCase() == "low") {
             return ImageQuality.ImageQualityLow;
         }
         if (quality.toLocaleLowerCase() == "high") {
             return ImageQuality.ImageQualityHigh;
         }
         return ImageQuality.ImageQualityNone;
     };

     /* Function to create pxTextItem object */
     this.createPXItem = function(view) {
         var item = {};
         clog("createPXItem");
         if (imageItems.length === 0 || imageItems[imageItems.length - 1].getParentView()) {
             clog("Creating new imageItem");
             item = new PXImageItem(view, id, app, scene, this);
             imageItems.push(item);
         } else {
             clog("Image item already present");
             item = imageItems[imageItems.length - 1];
             imageItems[imageItems.length - 1].setParentView(view);
             imageItems[imageItems.length - 1].updateResource();
         }
         return item;
     };

     /* Method to get url */
     this.getUrl = function() {
         return imgUrl;
     };

     /* Method to check whether resampling is enabled */
     this.isResamplingEnabled = function() {
         return resamblingEnabled;
     };

     /* Method to get resampling dimensions */
     this.getResampleDimensions = function() {
         return resamplingDimensions;
     };

     //Setting parameters to image object
     this.setProperties(params);
     var item = new PXImageItem(null, id, app, scene, this);
     imageItems.push(item);
 };
 /** 
  *   Class to handle image scene object and asign properties to the object 
  **/
 var PXImageItem = function(view, id, app, scene, res) {
     var _this = this;
     var parentView = null;
     var resource = res;
     var isImageReady = false;

     var imgObject = scene.createImage({
         url: resource.getUrl()
     });

     /**
      *   Method to set resource object with required properties  
      **/
     this.updateResource = function() {
         defaultLogger.log("debug", "PXImageItem : updateResource");
         if (imgObject) {
             if (resource.isResamplingEnabled) {
                 var resamplingDimensions = resource.getResampleDimensions();
                 if (resamplingDimensions.length > 0) {
                     imgObject.w = parseInt(resamplingDimensions[0]);
                     imgObject.h = parseInt(resamplingDimensions[1]);
                 }
             }
             if (resource.isURLSet) {

                 imgObject.url = resource.getUrl();
                 resource.isURLSet = false;

                 imgObject.ready.then(function() {
                     defaultLogger.log("info", " Image ready from PXImageItem");
                     defaultLogger.log("debug", "onready event ======================= " + id);
                     var readyEvent = new OnReady();
                     resource.getEmitter().emit('Event', readyEvent, resource, false);
                     defaultLogger.log("debug", "sending onready event");
                     var metadataEvent = new onMetaData(imgObject.w, imgObject.h);
                     resource.getEmitter().emit('Event', metadataEvent, resource);
                     defaultLogger.log("debug", "sending onMetaData event ");
                     if (!_this.isImageReady) {
                         _this.isImageReady = true;
                         if (parentView) {
                             _this.setImageAlignment();
                         }
                     }
                 });
             }
             //Sending onready event since image ready not obtained when setting url 
             //defaultLogger.log("debug", "Sending onready directly on setProperties");
             //var readyEvent = new OnReady();
             //resource.getEmitter().emit('Event', readyEvent, resource, false);

             if (imgObject.h === 0 || imgObject.w === 0) {
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
      *    Function to get rectangle object measurements
      **/
     this.setImageAlignment = function() {
         clog("setImageAlignment");
         var imgRect = parentView.getResourceRect(imgObject.w, imgObject.h);

         imgObject.sx = imgRect.W / imgObject.w;
         imgObject.sy = imgRect.H / imgObject.h;
         imgObject.x = imgRect.X;
         imgObject.y = imgRect.Y;
         //imgObject.useMatrix = true;
     };

     /**
      *    Function to get image object measurements
      **/
     this.getObjectRect = function() {
         var objRect = {};
         objRect.X = imgObject.x;
         objRect.Y = imgObject.y;
         objRect.W = imgObject.w;
         objRect.H = imgObject.h;
         return objRect;
     };

     /**
      *    Function to get scene rectangle object
      **/
     this.getSceneObject = function() {
         return imgObject;
     };
     this.getWidth = function() {
         return imgObject.w;
     };
     this.getHeight = function() {
         return imgObject.h;
     };
     this.getParentView = function() {
         return parentView;
     };
     this.setParentView = function(view) {
         parentView = view;
         imgObject.parent = parentView.getViewObj();
         resource.registerMouseEvent(resource, imgObject, view);
     };
     if (view) {
         this.setParentView(view);
     }

     this.updateResource();
 };

 //Inherting XREResource
 CImageResource.prototype = Object.create(XREResource.prototype);
 CImageResource.prototype.constructor = CImageResource;

 module.exports = CImageResource;
