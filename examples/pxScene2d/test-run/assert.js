/** Assert functions for testing pxscene js applications */


/** assert will return the standard success/fail prefix based on 
 *  the passed in condition **/
module.exports.assert = function(condition, message) {
  //console.log("Inside px assert with condition: " +condition);
  if( condition === false) 
  {
    //console.log("FAILURE : "+message);
    return "FAILURE: "+message;
  } else {
    return "SUCCESS";
  }
}
