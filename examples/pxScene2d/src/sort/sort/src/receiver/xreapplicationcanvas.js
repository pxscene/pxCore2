/***************************************
 *Name: xreapplication.js
 *Date 9-jun-2015
 ***************************************/
var Constants = require("../utils/config.js");
var XREView = require("../receiver/xreView.js");
var RESULT = require('../utils/enum').RESULT;


/**
 *Class: XREApplicationCanvas
 */
function XREApplicationCanvas(scene, app) {
    _this = this;
    this.sceneRect = {
        "X": scene.x,
        "Y": scene.y,
        "W": scene.w,
        "H": scene.h
    };
    this.xreApp = app;
    this.graphicsScene = scene;

    var views = {};

    //overloaded method for get view
    this.getView = function(arg) {
        if (typeof arg === "number") {
            return getViewWithId(arg);
        } else {
            return getViewWithName(arg);
        }
    };
    //get view using id
    var getViewWithId = function(index) {
        return views[index];
    };

    //get view using name
    var getViewWithName = function(name) {
        var view = undefined;
        for (var currView of views) {
            if (currView.getName() == name) {
                view = curr_view;
                break;
            }
        }
        return view;
    };

    //create new view
    this.createView = function(id, params) {
        clog("create view");
        var view = new XREView(this, id, this.xreApp, this.graphicsScene, params);
        clog("create view created");
        if (!view) {
            return RESULT.XRE_E_COULD_NOT_CREATE_OBJECT;
        }
        views[id] = view;
        return RESULT.XRE_S_OK;
    };

    //delete view
    this.deleteView = function(id) {
        //TODO need to check
        clog("delete view");
        if (views[id]) {
            var view = views[id];
            view.destroyView();
            delete views[id];
        }
    };

    //set  view properties
    this.setViewProperties = function(id, params) {
        var view = views[id];
        if (typeof view == 'undefined') {
            return RESULT.XRE_E_COULD_NOT_CREATE_OBJECT;
        }

        view.setProperties(params);
        return RESULT.XRE_S_OK;
    };

    //call view method
    this.callViewMethod = function(id, method, params) {
        
        var view = views[id];
        if (!view) {
            return RESULT.XRE_E_COULD_NOT_CREATE_OBJECT;
        }
        view.callMethod(method, params);
        return RESULT.XRE_S_OK;
    };


    //remove view
    this.removeView = function(id) {
        //TODO need to check
        views[id].getViewObj().remove();
        delete views[id];
        if (id == Constants.XRE_ID_ROOT_VIEW) rootView = null;
    };

    //get for scene size
    this.getSceneRect = function() {
        return this.sceneRect;
    };

    /*
    void CXREApplicationCanvas::SetParentItem(QGraphicsItem * parent)
    {
    //TODO
    }
     */

    //get parent view
    this.getParentItem = function() {
        if (typeof rootView == 'undefined') {
            return RESULT.XRE_E_COULD_NOT_CREATE_OBJECT;
        }
        return rootView.parentItem();
    };
    /**
     *   Function to return application
     **/
    this.getApp = function() {
        return this.xreApp;
    };

    this.rootView = new XREView(this, Constants.XRE_ID_ROOT_VIEW, this.xreApp, scene); //TODO add param empty
    views[Constants.XRE_ID_ROOT_VIEW] = this.rootView;

}


module.exports = XREApplicationCanvas;
