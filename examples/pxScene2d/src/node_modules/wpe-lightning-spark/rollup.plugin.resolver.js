const path = require('path');

module.exports = function localResolver(platform) {
    return {
        resolveId(importee, importer) {
            if (importee.indexOf('.' + path.sep) === -1) {
                return null;
            }

            if (!importer) {
                return null;
            }

            const filename = importee.split(path.sep).pop();

            const endsWith = ".dev.mjs";
            if (filename.lastIndexOf(endsWith) === (filename.length - endsWith.length)) {
                const parts = importee.split(path.sep);
                const index = parts.indexOf('src');
                const newPath = __dirname + path.sep + "src" + path.sep + parts.slice(index).join(path.sep);
                const absPath = path.resolve(newPath, importee.substr(0, importee.length - endsWith.length) + "." + platform + ".mjs");
                return absPath
            }
        }
    };
}