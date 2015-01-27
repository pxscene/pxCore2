// pxCore CopyRight 2007-2015 John Robinson
// testScene.cpp

#include "testScene.h"

#include "rtPathUtils.h"
#include "pxScene2d.h"
#include "pxText.h"
#include "pxImage.h"

pxScene2dRef scene = new pxScene2d;

#if 0

// Using C++ API... 


void testScene() {

  rtString d;
  rtGetCurrentDirectory(d);
  d.append("/../images/banana.png");

  scene->init();

  pxObjectRef root = scene->getRoot();
  
  int n = 3;
  int nx = 100;
  int ny = 200;
  for (int i = 0; i < n; i++) {
    pxObjectRef p;

    if (i == 0) {
      rtRefT<rectangle9> r = new rectangle9();
      float c1[4] = {1,0,0,1};
      r->setFillColor(c1);
      float c2[4] = {1,1,1,0.5};
      r->setLineColor(c2);
      r->setLineWidth(10);
      r->setW(300);
      r->setH(30);
      p = r;
      p->animateTo("h", 600, 0.2, pxInterpLinear, seesaw);
    }
    else if (i == 1) {
      rtRefT<pxText> t = new pxText();
      t->setText("Hello");
      p = t;
    }
    else {
      pxImage* i = new pxImage();
      p = i;
      i->setURL(d.cString());
      p->setCX(i->w()/2);
      p->setCY(i->h()/2);
    }

    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }
    p->setX(nx);
    p->setY(ny);

    p->setRX(1);
    p->setRY(0);
    p->setRZ(0);

    p->setParent(root);
    
    p->animateTo("r", 360, 2.0, pxInterpLinear, loop);
    p->animateTo("x", 800, 2.0, pxInterpLinear, seesaw);
    p->animateTo("a", 0.5, 2.0, pxInterpLinear, seesaw);
  }
}
#else

// Using Dynamic rtObject API
void testScene() {
  
  rtString d;
  rtGetCurrentDirectory(d);
  d.append("/../images/banana.png");

  scene->init();

  rtObjectRef root = scene.get<rtObjectRef>("root");  
  
  int n = 3;
  int nx = 100;
  int ny = 200;
  for (int i = 0; i < n; i++) {
    
    rtObjectRef p;

    if (i == 0) {
      scene.sendReturns<rtObjectRef>("createRectangle", p);
      p.set("w", 300);
      p.set("h", 30);
      p.set("fillColor",0x00ff00ff);
      p.set("lineColor",0xffffff80);
      p.set("lineWidth", 10);  
      p.send("animateTo", "h", 600, 0.2, 0, 0);
  }
    else if (i == 1) {
      scene.sendReturns<rtObjectRef>("createText", p);
      p.set("text", "pxCore!");
    }
    else {
      scene.sendReturns<rtObjectRef>("createImage", p);
      p.set("url", d);
      p.set("cx", p.get<float>("w")/2);
      p.set("cy", p.get<float>("h")/2);
    }

    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }

    p.set("parent", root);
    p.set("x", nx);
    p.set("y", ny);
    p.set("rx", 0);
    p.set("ry", 1.0);
    p.set("rz", 0);
    p.send("animateTo", "r", 360, 2.0, 0, 0);
    p.send("animateTo", "x", 800, 2.0, 0, 0);

    // Demonstrate how to invoke same function with a rtFunctionRef
    rtFunctionRef f = p.get<rtFunctionRef>("animateTo");
    f.send("a", 0.1, 2.0, 0, 0);

  }
}
#endif

