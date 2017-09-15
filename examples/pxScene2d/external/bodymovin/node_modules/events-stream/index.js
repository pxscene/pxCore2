var ReadStream = require("read-stream")

module.exports = events

function events(elem, eventName, capture) {
    if (elem.addEventListener) {
        return DOMEvents(elem, eventName, capture)
    } else {
        return EEEvents(elem, eventName)
    }
}

function DOMEvents(elem, eventName, capture) {
    if (capture === undefined) {
        capture = false
    }

    var queue = ReadStream()
        , stream = queue.stream

    elem.addEventListener(eventName, queue.push, capture)

    stream.close = _close

    return stream

    function _close() {
        elem.removeEventListener(eventName, queue.push, capture)
    }
}

function EEEvents(ee, eventName) {
    var queue = ReadStream()
        , stream = queue.stream

    ee.on(eventName, queue.push)

    stream.close = _close

    return stream

    function _close() {
        ee.removeListener(eventName, queue.push)
    }
}
