var root = scene.root;


var myText = scene.create({t:"text"});
myText.text = "Select an image to display:\n\n1: Banana\n2: Ball";
myText.x = 100;
myText.y = 100;
myText.parent = root;

console.log(myText.description());


for(var p in myText) 
  console.log(p, myText[p]);


