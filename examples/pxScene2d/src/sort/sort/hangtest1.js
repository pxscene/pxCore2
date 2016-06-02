var text = scene.createText();
text.text = "This should be spinning";
text.x = 50;
text.rx = 1;
text.rz = 0;
text.parent = scene.root;
text.animateTo("r", 360, 1.0, 0, 2);

scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
    text.y = height/2;

// Make sure the render loop still keeps going
while(true);


});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});




