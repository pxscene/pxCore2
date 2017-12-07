"use strict";

function runInNewContext(code, sandbox, options, px, xModule, filename, dirname) {
    return uv.run_in_new_context(code, sandbox, options, px, xModule, filename, dirname);
}

module.exports = {
    runInNewContext: runInNewContext,
}
