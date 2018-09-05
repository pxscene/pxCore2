
var events = require('events');
var util = require('util');
var EventEmitter = events.EventEmitter;

function Sender() {
    EventEmitter.call(this);
}

util.inherits(Sender, events.EventEmitter);

px.import({ scene: 'px:scene.1' }).then( function importsAreReady(imports)
{
    var sender = new Sender();
    sender.emit('open');
    sender.on('open', function () {
            print('onOpen');
    });

}).catch( function importFailed(err){
    console.error("Import failed for events_test.js: " + err);
});
