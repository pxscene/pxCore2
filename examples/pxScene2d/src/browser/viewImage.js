px.import({scene:"px:scene.1.js",url:'url'/*,path:'path'*/}).then(function(imports) {

  scene = imports.scene
  var i
  var resolved = false
  var vector = false
  var url

  module.exports._ready = new Promise(function(resolve, reject){
    url = px.appQueryParams.url;
    
    var Url = imports.url//require('url')
    //var Path = imports.path//require('path')

    var ext
    var urlParts = Url.parse(url,true)

    ext = urlParts.query['_ext']
    if (!ext) {
      //  ext = Path.extname(urlParts.pathname)
      ext = urlParts.pathname.split('.').pop()
    }
    console.log('ext', ext)
    
    vector = ext=='svg'
    
    var ir = scene.create({ t: "imageResource", url: url, w: scene.w, h: scene.h });

    i = scene.create({t:'image',resource:ir,parent:scene.root,
      stretchX:vector?scene.stretch.NONE:scene.stretch.STRETCH,
      stretchY:vector?scene.stretch.NONE:scene.stretch.STRETCH,
      a:0})

    i.ready.then(function(o){
      module.exports._preferredW=o.resource.w
      module.exports._preferredH=o.resource.h

      resolved = true
      updateSize(scene.w,scene.h)
      i.animate({a:1},0.3,scene.animation.TWEEN_STOP,scene.animation.OPTION_LOOP,1)

      resolve(o)
    })
  })

  function updateSize(w,h) {
    if (resolved) {
      var sx = i.resource.w>0?w/i.resource.w:1
      var sy = i.resource.h>0?h/i.resource.h:1
      var s = (sx<sy)?sx:sy
      
      if (!vector) {
        i.sx = i.sy = s>1?1:s
      }
      else {
        var nir = scene.create({ t: "imageResource", url: url, w: scene.w, h: scene.h });
        
        var ni = scene.create({t:'image',resource:nir,parent:scene.root,
          stretchX:vector?scene.stretch.NONE:scene.stretch.STRETCH,
          stretchY:vector?scene.stretch.NONE:scene.stretch.STRETCH,
          a:0})

        ni.ready.then(function(){
          var di = i
          di.animateTo({a:0},0.2,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then(function(){
           di.remove()
          })

          i = ni
          ni.animateTo({a:1},0.2,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1)
        })
          
      }
    }
  }

  var timer
  function updateSizeLater(w,h) {
    if (timer)
      clearTimeout(timer)
    timer = setTimeout(function(){updateSize(w,h)},50)
  }
  

  scene.on("onResize", function(e) { 
    if (vector)
      updateSizeLater(e.w,e.h) 
    else
      updateSize(e.w,e.h)
  })

  updateSize(scene.w,scene.h)

})