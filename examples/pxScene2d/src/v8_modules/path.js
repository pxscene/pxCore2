/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

"use strict";

function normalizeStringWin32(path, allowAboveRoot) {
    var res = '';
    var lastSlash = -1;
    var dots = 0;
    var code;
    for (var i = 0; i <= path.length; ++i) {
        if (i < path.length)
            code = path.charCodeAt(i);
        else if (code === 47/*/*/ || code === 92/*\*/)
            break;
        else
            code = 47/*/*/;
        if (code === 47/*/*/ || code === 92/*\*/) {
            if (lastSlash === i - 1 || dots === 1) {
                // NOOP
            } else if (lastSlash !== i - 1 && dots === 2) {
                if (res.length < 2 ||
                    res.charCodeAt(res.length - 1) !== 46/*.*/ ||
                    res.charCodeAt(res.length - 2) !== 46/*.*/) {
                    if (res.length > 2) {
                        const start = res.length - 1;
                        var j = start;
                        for (; j >= 0; --j) {
                            if (res.charCodeAt(j) === 92/*\*/)
                                break;
                        }
                        if (j !== start) {
                            if (j === -1)
                                res = '';
                            else
                                res = res.slice(0, j);
                            lastSlash = i;
                            dots = 0;
                            continue;
                        }
                    } else if (res.length === 2 || res.length === 1) {
                        res = '';
                        lastSlash = i;
                        dots = 0;
                        continue;
                    }
                }
                if (allowAboveRoot) {
                    if (res.length > 0)
                        res += '\\..';
                    else
                        res = '..';
                }
            } else {
                if (res.length > 0)
                    res += '\\' + path.slice(lastSlash + 1, i);
                else
                    res = path.slice(lastSlash + 1, i);
            }
            lastSlash = i;
            dots = 0;
        } else if (code === 46/*.*/ && dots !== -1) {
            ++dots;
        } else {
            dots = -1;
        }
    }
    return res;
}

function normalizeStringPosix(path, allowAboveRoot) {
    var res = '';
    var lastSlash = -1;
    var dots = 0;
    var code;
    for (var i = 0; i <= path.length; ++i) {
        if (i < path.length)
            code = path.charCodeAt(i);
        else if (code === 47/*/*/)
            break;
        else
            code = 47/*/*/;
        if (code === 47/*/*/) {
            if (lastSlash === i - 1 || dots === 1) {
                // NOOP
            } else if (lastSlash !== i - 1 && dots === 2) {
                if (res.length < 2 ||
                    res.charCodeAt(res.length - 1) !== 46/*.*/ ||
                    res.charCodeAt(res.length - 2) !== 46/*.*/) {
                    if (res.length > 2) {
                        const start = res.length - 1;
                        var j = start;
                        for (; j >= 0; --j) {
                            if (res.charCodeAt(j) === 47/*/*/)
                                break;
                        }
                        if (j !== start) {
                            if (j === -1)
                                res = '';
                            else
                                res = res.slice(0, j);
                            lastSlash = i;
                            dots = 0;
                            continue;
                        }
                    } else if (res.length === 2 || res.length === 1) {
                        res = '';
                        lastSlash = i;
                        dots = 0;
                        continue;
                    }
                }
                if (allowAboveRoot) {
                    if (res.length > 0)
                        res += '/..';
                    else
                        res = '..';
                }
            } else {
                if (res.length > 0)
                    res += '/' + path.slice(lastSlash + 1, i);
                else
                    res = path.slice(lastSlash + 1, i);
            }
            lastSlash = i;
            dots = 0;
        } else if (code === 46/*.*/ && dots !== -1) {
            ++dots;
        } else {
            dots = -1;
        }
    }
    return res;
}


win32 = {
    normalize: function normalize(path) {
        const len = path.length;
        if (len === 0)
            return '.';
        var rootEnd = 0;
        var code = path.charCodeAt(0);
        var device;
        var isAbsolute = false;

        // Try to match a root
        if (len > 1) {
            if (code === 47/*/*/ || code === 92/*\*/) {
                // Possible UNC root

                // If we started with a separator, we know we at least have an absolute
                // path of some kind (UNC or otherwise)
                isAbsolute = true;

                code = path.charCodeAt(1);
                if (code === 47/*/*/ || code === 92/*\*/) {
                    // Matched double path separator at beginning
                    var j = 2;
                    var last = j;
                    // Match 1 or more non-path separators
                    for (; j < len; ++j) {
                        code = path.charCodeAt(j);
                        if (code === 47/*/*/ || code === 92/*\*/)
                            break;
                    }
                    if (j < len && j !== last) {
                        const firstPart = path.slice(last, j);
                        // Matched!
                        last = j;
                        // Match 1 or more path separators
                        for (; j < len; ++j) {
                            code = path.charCodeAt(j);
                            if (code !== 47/*/*/ && code !== 92/*\*/)
                                break;
                        }
                        if (j < len && j !== last) {
                            // Matched!
                            last = j;
                            // Match 1 or more non-path separators
                            for (; j < len; ++j) {
                                code = path.charCodeAt(j);
                                if (code === 47/*/*/ || code === 92/*\*/)
                                    break;
                            }
                            if (j === len) {
                                // We matched a UNC root only
                                // Return the normalized version of the UNC root since there
                                // is nothing left to process

                                return '\\\\' + firstPart + '\\' + path.slice(last) + '\\';
                            } else if (j !== last) {
                                // We matched a UNC root with leftovers

                                device = '\\\\' + firstPart + '\\' + path.slice(last, j);
                                rootEnd = j;
                            }
                        }
                    }
                } else {
                    rootEnd = 1;
                }
            } else if ((code >= 65/*A*/ && code <= 90/*Z*/) ||
                       (code >= 97/*a*/ && code <= 122/*z*/)) {
                // Possible device root

                code = path.charCodeAt(1);
                if (path.charCodeAt(1) === 58/*:*/) {
                    device = path.slice(0, 2);
                    rootEnd = 2;
                    if (len > 2) {
                        code = path.charCodeAt(2);
                        if (code === 47/*/*/ || code === 92/*\*/) {
                            // Treat separator following drive name as an absolute path
                            // indicator
                            isAbsolute = true;
                            rootEnd = 3;
                        }
                    }
                }
            }
        } else if (code === 47/*/*/ || code === 92/*\*/) {
            // `path` contains just a path separator, exit early to avoid unnecessary
            // work
            return '\\';
        }

        code = path.charCodeAt(len - 1);
        var trailingSeparator = (code === 47/*/*/ || code === 92/*\*/);
        var tail;
        if (rootEnd < len)
            tail = normalizeStringWin32(path.slice(rootEnd), !isAbsolute);
        else
            tail = '';
        if (tail.length === 0 && !isAbsolute)
            tail = '.';
        if (tail.length > 0 && trailingSeparator)
            tail += '\\';
        if (device === undefined) {
            if (isAbsolute) {
                if (tail.length > 0)
                    return '\\' + tail;
                else
                    return '\\';
            } else if (tail.length > 0) {
                return tail;
            } else {
                return '';
            }
        } else {
            if (isAbsolute) {
                if (tail.length > 0)
                    return device + '\\' + tail;
                else
                    return device + '\\';
            } else if (tail.length > 0) {
                return device + tail;
            } else {
                return device;
            }
        }
    },
    extname: function extname(path) {
        var parts = path.split('.');
        return parts.length > 1 ? ('.' + parts[parts.length-1]) : '';
    },
}

posix = {
    sep: '/',
    delimiter: ':',
    normalize: function normalize(path) {
        if (path.length === 0)
            return '.';

        const isAbsolute = path.charCodeAt(0) === 47/*/*/;
        const trailingSeparator = path.charCodeAt(path.length - 1) === 47/*/*/;

        // Normalize the path
        path = normalizeStringPosix(path, !isAbsolute);

        if (path.length === 0 && !isAbsolute)
            path = '.';
        if (path.length > 0 && trailingSeparator)
            path += '/';

        if (isAbsolute)
            return '/' + path;
        return path;
    },
    extname: function extname(path) {
        var parts = path.split('.');
        return parts.length > 1 ? ('.' + parts[parts.length-1]) : '';
    },
}

if (uv_platform() === 'win32')
    module.exports = win32;
else
    module.exports = posix;
