# events-stream

A stream of DOM events

## Example

```
var events = require("events-stream")

// Keys is a readable stream that emits all keypress events dispatched from
// document.body
var keys = events(document.body, "keypress")
```

## Installation

`npm install events-stream`

## Contributors

 - Raynos

## MIT Licenced