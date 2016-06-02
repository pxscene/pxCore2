
var gooey = function() {
  return {
    class: function(d) {
      return d;
    }
    render: function(o,p) {
      var m = o.t.render(o)
      m.p = p?p:null;
    }
  }
}

var hflow = gooey.class({
  render: function(o,c) {
    for(let v of o.c) {
      console.log(v.t);
    }
  }
});

var app = gooey.class({
  render: function(o,c) {
    return gooey.create(hflow, {x:100,y:100,w:200,h:100},[
      gooey.create("rect", {w:200,h:200,fillColor:0xff0000ff})
      gooey.create("rect", {w:200,h:200,fillColor:0x00ff00ff}),
      gooey.create("rect", {w:200,h:200,fillColor:0x0000ffff}),       
      ]);
  }
});

//gooey.registerClass("foo", f);
// assume string classes are terminals now... 

// should t be a string... would be good for total serialized form... but then what defines the
// factory... would have to have some factory registry for that namespace...
// what about namespacing... different toolkits etc... 

// observation that construction time layout and general layout should have some way to avoid redundancy

var w = 400;
var h = 400;
gooey.render({t:app,w:w,h:h});
