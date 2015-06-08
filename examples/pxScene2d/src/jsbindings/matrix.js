var root = scene.root;

var o = scene.create({t:"object",parent:root,useMatrix:true,parent:root,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:20,m42:20,m43:0,m44:1});

scene.createRectangle({useMatrix:true,fillColor:0xff0000ff,w:300,h:300,parent:o,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:20,m42:20,m43:0,m44:1});
