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

var promise1 = Promise.resolve(3);
var promise2 = _testPromiseResolvedReturnFunc();
var promise3 = _testPromiseReturnFunc();

Promise.all([promise1, promise2, promise3]).then(function (values) {
    print("OK");
});

var promise4 = _testPromiseRejectedReturnFunc();

promise4.then(function (val) {
    print("resolved2");
}).catch(function (val) {
    print("rejected2");
});

var promise5 = _testPromiseReturnRejectFunc();

promise5.then(function (val) {
    print("resolved3");
}).catch(function (val) {
    print("rejected3");
});

var promise6 = _testPromiseReturnFunc();

new Promise((resolve, reject) => {
    promise6.then(function (val) {
        resolve(val);
    }).catch(function (val) {
        reject(val);
    });
}).then(function (val) {
    print("resolved4");
},
function (val) {
    print("rejected4");
}
);


