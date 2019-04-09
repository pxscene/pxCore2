/**
 * emit class
 */
function WBEmit() {}
WBEmit.prototype.$emit = function(name) {
    var args = Array.prototype.slice.call(arguments, 1);
    if (this._events && this._events[name])
        this._events[name].forEach(function(cb) {
            cb.apply(this, args)
        }.bind(this));
    return this;
};
WBEmit.prototype.$off = function(name, cb) {
    if (!this._events) return this;
    if (!cb) delete this._events[name];
    if (this._events[name]) this._events[name] = this._events[name].filter(function(i) {
        return i != cb
    });
    return this;
};
WBEmit.prototype.$on = function(name, cb) {
    if (!this._events) this._events = {};
    if (!this._events[name]) this._events[name] = [];
    this._events[name].push(cb);
    return cb;
};

module.exports = WBEmit;