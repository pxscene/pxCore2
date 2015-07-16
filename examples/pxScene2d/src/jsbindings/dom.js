var root = scene.root;

var o = scene.create({t:"rect",fillColor:0xffffffff,w:300,h:300,parent:root,cx:150,cy:150,c:[
  {t:"rect",fillColor:0xff0000ff,w:100,h:100},
  {t:"rect",fillColor:0x00ff00ff,w:100,h:100,x:100,y:100,cx:50,cy:50,rx:0,rz:1},
  {t:"rect",fillColor:0x0000ffff,w:100,h:100,x:200,y:200},
]})

o.animateTo({r:360},2,scene.PX_LINEAR,scene.PX_LOOP);
o.animateTo({sx:0.5,sy:0.5,a:0},2,scene.PX_LINEAR,scene.PX_SEESAW);
o.children[1].animateTo({r:-360},1,scene.PX_LINEAR,scene.PX_LOOP);
