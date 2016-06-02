px.import("px:scene.1.js").then( function ready(scene) {

  var red = 0xff0000ff;
  var green = 0x00ff00ff;
  var blue = 0x0000ffff;
  
  var gooey = function() {
    return {
      class: function(d) {
        return d;
      },
      render: function(o) {
        while (typeof o.t != "string") {
          o = o.t.render(o);
        }
        if (o.c) {
          for (var i = 0; i < o.c.length; i++) {
            o.c[i] = this.render(o.c[i]);
          }
        }
        return o;
      }
    };
  }();
  
  console.log(gooey.class);
  
  var hflow = gooey.class({
    render: function(o) {
      var c = {t:"rect",x:o.x,w:o.w,h:o.h,fillColor:0xffff00ff,c:o.c};
      var x = 0;
      for (var v of c.c) {
        v.x = x;
        v.y = 0;
        v.h = o.h;
        x+=v.w;
      }
      return c;
    }
  });
  
  var app = gooey.class({
    render: function(o) {
      return {t:hflow,w:o.w,h:o.h,c:[
        {t:"rect",w:200,h:200,fillColor:red},
        {t:hflow,w:400,h:400,fillColor:0xffffffff,lineColor:red,lineWidth:2,c:[
          {t:"rect",w:200,h:200,fillColor:0x00ffffff},
          {t:"rect",w:200,h:200,fillColor:blue},
        ]},
        {t:"rect",w:200,h:200,fillColor:blue},
      ]};
    }
  });
  
  console.log("gooey!");
  //var d = gooey.render({t:app,w:400,h:400});
  
  var root = scene.root;
  

  //console.log(JSON.stringify(d));
  
  function clone(o) {
    var c = {};
    for (var v in o) {
      if (v == "c")
        continue;
      if (o.hasOwnProperty(v)) 
        c[v] = o[v];
    }
    return c;
  }
  
  function construct(o,p) {
    if (o.t == "rect") {
      var n = scene.createRectangle(clone(o));
      n.parent = p;
      if (o.c) {
        for(var i = 0; i < o.c.length; i++) {
          construct(o.c[i],n);
        }
      }
    }
    else
      console.log("unhandled type");
  }
  
  //scene.createRectangle({t:"rect",w:200,h:200,fillColor:red,parent:root});
  //construct(d,root);
  
  //root.clear();
  
  function onSize(w,h) {
    root.painting = false;
    root.removeAll();
    var d = gooey.render({t:app,w:w,h:h});
    construct(d,root);
    root.painting = true;
  }
  
  scene.on("onResize", function(e) {onSize(e.w,e.h);});
  onSize(scene.w,scene.h);
});
