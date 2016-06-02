/**
 * Classname: XREObject 
 * Version information: 
 * Date : 
 * Author: 
 * Copyright (C) 2015 - 2016 by TATA Elxsi. All rights reserved.
 */
var XREApplication = require ( '../protocol/xreapplication.js' );
var XREEventHandler = require ( './xreEventHandler.js' );
var EventEmitter = require ( 'events' ).EventEmitter;
var XRElogger = require('../utils/xrelogger.js');

function XREObject ( id, app ) {
   
    var defaultLogger = XRElogger.getLogger("default");
    this.id = id;
    this.app = app;
    this.eventHandlers = new XREEventHandler(); 
    this.eventEmitter = new EventEmitter ();
    if ( app != null ) {
    	this.eventEmitter.on ( 'Event', app.getEventDispatcher().SendEvent);
	
    }
    
    this.getEmitter = function () { 
        return this.eventEmitter;
    }	
 
    this.GetHandlerConfig = function ( xreEvent) {
        return this.eventHandlers.getConfig(xreEvent);
    }

    this.AddEvents = function () {
        eventHandlers.add ( this.eventHandlers );
    }
	
    this.GetKlass = function ()
    {
        var klassName = this.constructor.name;
	
        if ( klassName.substring(0,4) == 'CXRE' ) {
            klassName = klassName.substring(1, klassName.length);
            defaultLogger.log("info",klassName);
        }
        if ( klassName.substring(0,1) == 'C' ) {
            klassName = 'XRE' + klassName.substring(1, klassName.length);
        }

        if ( klassName.substring( klassName.length - 8) == 'Resource' ) {
            klassName = klassName.substring(0, klassName.length-8);
            defaultLogger.log("info",klassName);
        }
        return klassName;
    }

    this.GetProperty = function ( name )
    {
        var return_val = false;
        
        if ( "id" == name.toLowerCase () )
        {
            return_val = this.id;
        }
        if ( "klass" == name.toLowerCase() )
        {
            return_val = GetKlass ();
        }
        /*else if (eventHandlers.IsInstalled ( name ) )
        {
            return_val = m_eventHandlers.GetConfigs(name);
        }*/
		
        if ( return_val ) {
            return return_val;
        }
        else {
            return return_val;
        }
    }

    this.GetProperties = function ( param_names, params ) {
        for (x in param_names) {
            if ( typeof x == 'string') {
                var property_name = x;
                if ( property_val = GetProperty ( property_name ) )
                {
                    params [ property_name ] = property_val;
                }
                else
                {
                    defaultLogger.log("info",'GetProperties got unsupported property : ' + property_name );
                }
            }
            else {
                defaultLogger.log("info",'GetProperties got param name of unsupported type');
            }
		
        }
    }
    this.GetId = function ( ) {
            
        return this.id;
    }
    this.GetApp = function ( ) {
        return this.app;
    }
}

module.exports = XREObject;
