var u1 = "http://farm4.static.flickr.com/3307/5767175230_b5d2bf2312_z.jpg";
var u2 = "http://farm6.static.flickr.com/5263/5793867021_3e1d5d3aae_z.jpg";

var i1 = scene.create({t:"image", url:u1, parent:scene.root});
var i2 = scene.create({t:"image", url:u2, parent:scene.root, x:400});


if (false) {
//This crashes
i1.ready.then(function() {console.log("image 1 ready"); });
i2.ready.then(function() {console.log("image 2 ready"); });

Promise.all([i1.ready, i2.ready]).then(function(){console.log("Both ready");});
}
else {
//This crashes
var p1 = new Promise(function(fulfill,reject){
i1.ready.then(function() {console.log("image 1 ready"); fulfill(i1); });
});

var p2 = new Promise(function(fulfill,reject){
i2.ready.then(function() {console.log("image 2 ready"); fulfill(i2); });
});



Promise.all([p1, p2]).then(function(){console.log("Both ready");});
}
