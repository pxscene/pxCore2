
var root = scene.root;

var o = scene.create({t:"rect",fillColor:0x000000ff,w:300,h:300,parent:root,cx:150,cy:150,c:[
  {t:"rect",fillColor:0xff0000ff,w:100,h:100},
  {t:"rect",fillColor:0x00ff00ff,w:100,h:100,x:100,y:100,cx:50,cy:50,rx:1,rz:0},
  {t:"rect",fillColor:0x0000ffff,lineColor:0xffff00ff,lineWidth:2,w:100,h:100,x:200,y:200},
]})

var a1 = o.animateTo({r:360},1,scene.PX_LINEAR,scene.PX_END)
  .then(function(z) {
   //   console.log('Im finally here:'+z);
    z.animateTo({sx:2.0,sy:2.0},1,scene.PX_LINEAR,scene.PX_END);
    return z;
    })
  .then(function(z) {
    //console.log("chained promise",z);
    return "test";
  })
.then(function(y) {
  console.log("foo: ", y);
});

var a2 = o.children[1].animateTo({r:-360},6,scene.PX_LINEAR,scene.PX_END)
  .then(function() {
    console.log("child animation done");
});

Promise.all([a1,a2]).then(function() {console.log("all animations complete");});

