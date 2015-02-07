scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});

function testScene() 
{
  var root = scene.root;

  var n = 10;
  var nx = 100;
  var ny = 100;

  for (i = 0; i < n; ++i) {
    if (i < 1) {
        p = scene.createRectangle({w:300,h:30,fillColor:0x00ff00ff,lineColor:0xffffff80,lineWidth:10});
        p.animateTo({h:600}, 0.5, 0, 1);
    }
    else if (i < 2) {
      var url = process.cwd() + "/../../images/curve_rectangle.png";
        p = scene.createImage9({url:url});
        p.cx = p.w/2;
        p.cy = p.h/2;
        ny = 100;
        p.animateTo({h:600}, 0.5, 0, 0);
        p.animateTo({w:600}, 0.5, 0, 1);
    }
    else if (i < n-3) {
        var url = process.cwd() + "/../../images/banana.png";
        p = scene.createImage({url:url});
        p.cx = p.w/2;
        p.cy = p.h/2;
        ny = 100;
    }
    else {
      p = scene.createText();
        p.animateTo({sx:2,sy:2}, 1.0, 0, 0);
        nx = 200;
        if (i == n-3) {
            p.text = "Iñtërnâtiônàližætiøn";
            p.textColor = 0xffff00ff;
            ny = 200;
        }
        else if (i == n-2) {
            p.text = "pxCore!";
            p.textColor = 0xff0000ff;
            ny = 300;
        }
        else if (i == n-1) {
            p.text = "Ādam";
            p.textColor = 0x00ffffff;
            ny = 400;
        }
        p.cx = p.w/2;
    }
    
    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }

    p.parent = root;
    p.x = nx;
    p.y = ny;
    
    p.rx = 0;
    p.ry = 1.0;
    p.rz = 0.0;

      p.animateTo({r:360}, 1.0+(i*0.3), 0, 2);
    if (i < n-1) {
        p.animateTo({x:600}, 1.0+(i*0.3), 0, 1);
    }

      p.animateTo({a:0.1}, 2.0, 0, 1);
  }
}

testScene();

