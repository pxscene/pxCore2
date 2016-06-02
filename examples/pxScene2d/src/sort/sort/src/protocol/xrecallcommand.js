var CallCommand = function(){
    var targetId = undefined;
    var targetPath = undefined;
    var method = undefined;
    var params = undefined;
    var callGUID = undefined;
    var commandIdx = 0;
    this.setProperties = function(data){
		targetId  = data.targetId;
		targetPath   = data.targetPath;
		method	= data.method;
		params = data.params;
		commandIdx = data.commandIndex;
		if(data.callGUID){
		    callGUID = data.callGUID;
		}
		return;
    };

    this.execute = function(app)
    {
		//Need to do id from target path
		app.call(targetId, method, params, callGUID);
		clog("call executed");
    };

    this.getCommandIndex = function(){
		return commandIdx;
    };
}


module.exports = CallCommand;
