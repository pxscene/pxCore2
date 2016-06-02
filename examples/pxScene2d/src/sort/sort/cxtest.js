var text = scene.createText();
text.text = "This should be spinning";
text.cx = text.w/2;
console.log("text width: ", text.w);
text.parent = scene.root;
text.animateTo({"r":360}, 1.0, 0, 2);

var ball = scene.createImage();
ball.url = process.cwd() + "/../../images/ball.png";
ball.cx = ball.w/2;
ball.cy = ball.h/2;
ball.parent = scene.root;
ball.animateTo({r: 360}, 1.0, 0, 2);
//ball.animateTo("r", 360, 1.0, 0, 2);

scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
    text.x = (width-text.w)/2;
    text.y = height/2;
});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});




