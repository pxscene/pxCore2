

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


