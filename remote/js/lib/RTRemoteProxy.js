const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const logger = require('../lib/common/logger');

/**
 * RTRemoteProxy class 
 * Returns a proxy object of passed rtObject
 * set : allows to set a property 
 * get allows to get property if available 
 * or return value from function call	 
 **/


class RTRemoteProxy {
  
  /*create new proxy object*/
  constructor(rtObject) {
    return new Proxy(rtObject, {
      get : function(rtObject, property, receiver) {
        return (...args) => {
          return rtObject.get(property).then((response) => {
            if(response.type == "f".charCodeAt(0)) {
              //TODO :: need to add condition to use sendReturns or send both.
	            var methodName = property;
              return rtObject.sendReturns(methodName, ...args);
            } 
            else {
              return response;
	          }
          });
	      };
      },
      set : function(rtObject, property, value) {
        return rtObject.get(property).then((response) => {
	        var type = response.type;
       	  return rtObject.set(property, RTValueHelper.create(value, type)).then(() => {
            return rtObject.get(property).then((response) => {
              let result = false;
	            if (type === RTValueType.UINT64 || type === RTValueType.INT64) {
	              result = value.toString() === response.value.toString();
	            } else {
	              result = response.value === value;
	            }
	            logger.debug(`Set succesfull ? ${result}`);
	          }).catch((err) => {
	            logger.error(err);
	          });
	        })
        })
      }	
    });
  }
}

module.exports = RTRemoteProxy;

