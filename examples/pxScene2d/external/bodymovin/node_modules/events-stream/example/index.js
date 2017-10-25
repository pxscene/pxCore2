var events = require("../index")
    , map = require("lazy-map-stream")
    , filter = require("lazy-filter-stream")
    , to = require("write-stream")

var keys = events(document.body, "keypress")
    , enters = filter(keys, function (event) {
        return event.keyCode === 13
    })
    , texts = map(enters, function (event) {
        return event.target.value
    })
    , elems = map(texts, function (text) {
        var elem = document.createElement("div")
        elem.textContent = text
        return elem
    })

elems.pipe(append(document.body))

function append(elem) {
    return to(function write(node) {
        console.log("appending", elem, node)
        elem.appendChild(node)
    })
}