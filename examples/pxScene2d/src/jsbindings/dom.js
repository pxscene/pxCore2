var root = scene.root;

scene.create({t:"rect",fillColor:0xffffffff,w:164,h:164,parent:root,cx:82,cy:82,c:[
  {t:"rect",fillColor:0xff0000ff,w:64,h:64},
  {t:"rect",fillColor:0x00ff00ff,w:64,h:64,x:32,y:32},
  {t:"rect",fillColor:0x0000ffff,w:64,h:64,x:64,y:64},
  {t:"text",text:"Hello World!",textColor:0x000000ff,pixelSize:32,x:0,y:32},
]}).animateTo({r:360},1,scene.PX_LINEAR,scene.PX_LOOP);
