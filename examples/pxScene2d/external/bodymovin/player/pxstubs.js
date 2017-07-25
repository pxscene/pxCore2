
// - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function requestAnimationFrame(num)
  {
      if(window !== undefined)
      {
          window.requestAnimationFrame(num);
      }
      else
      {
          console.log("ERROR:  'window' is undefined !!");
      }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function getFnName(fn)
  {
    var f = typeof fn == 'function';
    var s = f && ((fn.name && ['', fn.name]) || fn.toString().match(/function ([^\(]+)/));
    return (!f && 'not a function') || (s && s[1] || 'anonymous') + "()";
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - -

function dummyWindow() 
{
    var vv = false;

    this.style = {};

    this.pxRequestAnimationFrame = function(s) { if(vv) console.log("WINDOW >> pxRequestAnimationFrame( "+s+" );  ");  return setTimeout(s, 1000/60); };
    this.pxCancelAnimationFrame  = function(s) { if(vv) console.log("WINDOW >> pxCancelAnimationFrame( "+s+" );   "); };

    this.requestAnimationFrame = this.pxRequestAnimationFrame;

    module.exports = dummyWindow;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function dummyElement() 
  {
      var vv = true;

      this.style = {};

      this.setAttribute          = function(s, v)  { if(vv) console.log("ELEMENT >> setAttribute( "+s+" )    VALUE: " + JSON.stringify(v) );  this[s] = v; };
      this.appendChild           = function(c)     { if(vv) console.log("ELEMENT >> appendChild()              CHILD: " + JSON.stringify(c) ); };
      this.addEventListener      = function(s,e,b) { if(vv) console.log("ELEMENT >> addEventListener( \"" + s + "\" >> " + getFnName(e) + " )"); };
      this.insertBefore          = function(p,n)   { if(vv) console.log("ELEMENT >> insertBefore( )          "); };
      this.requestAnimationFrame = function(n)     { if(vv) console.log("ELEMENT >> requestAnimationFrame( ) "); }; 
      this.getContext            = function(e)     { if(vv) console.log("ELEMENT >> getContext( "+e+" )   "); return this; }; // rendering context

      module.exports = dummyElement;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  function dummyDocument() 
  {
      var vv = true;

      this.style = {};

      this.setAttribute          = function(s, v)   { if(vv) console.log("DOCUMENT >> setAttribute( "+s+" )    VALUE: " + JSON.stringify(v) );  this[s]  = v; };
      this.appendChild           = function(c)      { if(vv) console.log("DOCUMENT >> appendChild()              CHILD: " + JSON.stringify(c) ); };
      this.createElement         = function(e)      { if(vv) console.log("DOCUMENT >> createElement( "+e+" )   ");  return new dummyElement(); };
      this.createElementNS       = function(s,e)    { if(vv) console.log("DOCUMENT >> createElementNS( "+e+" ) ");  return new dummyDocument(); };  
      this.setAttributeNS        = function(ns,n,v) { if(vv) console.log("DOCUMENT >> setAttributeNS( "+n+" )  "); this[s]  = e; }; 
      this.addEventListener      = function(s,e,b)  { if(vv) console.log("DOCUMENT >> addEventListener( "+ JSON.stringify(e) +" )"); };
      this.insertBefore          = function(p,n)    { if(vv) console.log("DOCUMENT >> insertBefore( )          "); }; 
      this.requestAnimationFrame = function(n)      { if(vv) console.log("DOCUMENT >> requestAnimationFrame( ) "); };
      this.getContext            = function(e)      { if(vv) console.log("DOCUMENT >> getContext( "+e+" )   "); return {}; }; // rendering context
      this.getElementsByTagName = function(n)       { if(vv) console.log("DOCUMENT >> getElementsByTagName( "+n+" ) "); return {}; }; // rendering context

      module.exports = this;
  }

var dummy_window   = new dummyWindow();
var dummy_document = new dummyDocument(); 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
