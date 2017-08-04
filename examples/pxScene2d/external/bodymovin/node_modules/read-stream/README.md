# read-stream

Base class for readable streams

## Example array

```
var fromArray = require("read-stream").fromArray
    , stream = fromArray(["one", "two"])

stream.pipe(process.stdout)
```

## Example function

```
var ReadStream = require("read-stream")
    // state is a shared object among all reads whose initial
    // value is set to be  { count: 0 }
    , stream = ReadStream(function read(bytes, queue) {
        var count = ++queue.count

        if (count < 5) {
            return count.toString()
        }

        queue.end()
    }, { count: 0 }).stream

stream.pipe(process.stdout)
```

## Example queue

```
var ReadStream = require("read-stream")
    , queue = ReadStream()
    , count = 0

var timer = setInterval(function () {
    count = ++count

    if (count < 5) {
        queue.push(count.toString())
    } else {
        clearInterval(timer)
        queue.end()
    }
}, 500)

queue.stream.pipe(process.stdout)
```

## Installation

`npm install read-stream`

## Contributors

 - Raynos

## MIT Licenced
