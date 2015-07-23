
var matrixHelp = require("./mat4.js");

var root = scene.root;

var o = scene.create({t:"object",parent:root,useMatrix:true,parent:root,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:50,m42:50,m43:0,m44:1});

var rectRed = scene.createRectangle({useMatrix:true,fillColor:0xff0000ff,w:100,h:100,parent:o,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:100,m42:100,m43:0,m44:1});

var rectGreen = scene.createRectangle({useMatrix:true,fillColor:0x00ff00ff,w:100,h:100,parent:o,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:300,m42:100,m43:0,m44:1});
  
var rectBlue = scene.createRectangle({useMatrix:true,fillColor:0x0000ffff,w:100,h:100,parent:o,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:300,m42:300,m43:0,m44:1});
                       
var rectWhite = scene.createRectangle({useMatrix:true,fillColor:0xffffffff,w:100,h:100,parent:o,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:100,m42:300,m43:0,m44:1});       
                                      
var rectGray = scene.createRectangle({useMatrix:true,fillColor:0x808080ff,w:100,h:100,parent:o,
                       m11:1,m12:0,m13:0,m14:0,
                       m21:0,m22:1,m23:0,m24:0,
                       m31:0,m32:0,m33:1,m34:0,
                       m41:500,m42:300,m43:0,m44:1});                                       
                       
var myStartMatrixRed = matrixHelp.create();
myStartMatrixRed[13]=100;
myStartMatrixRed[14]=100;
var myRotateMatrixRed = matrixHelp.create();
var axis = [1,1,0];
matrixHelp.rotate( myRotateMatrixRed, myStartMatrixRed, 3.14159265, axis ); //6.28318531  3.14159265

console.log(myRotateMatrixRed);

var myStartMatrixGreen = matrixHelp.create();
var myRotateMatrixGreen2 = matrixHelp.create();
var myRotateMatrixGreen2B = matrixHelp.create();
var axis2 = [1,0,0];
myStartMatrixGreen[13]=300;
myStartMatrixGreen[14]=100;
matrixHelp.rotate( myRotateMatrixGreen2, myStartMatrixGreen, 3.14159265, axis2);
matrixHelp.rotate( myRotateMatrixGreen2B, myStartMatrixGreen, 6.28318531, axis2 );


var myStartMatrixBlue = matrixHelp.create();
var myRotateMatrixBlue = matrixHelp.create();
var myRotateMatrixBlueB = matrixHelp.create();
var axis3 = [0,0,1];
myStartMatrixBlue[13]=300;
myStartMatrixBlue[14]=300;
matrixHelp.rotate( myRotateMatrixBlue, myStartMatrixBlue, 3.14159265, axis3);
matrixHelp.rotate( myRotateMatrixBlueB, myStartMatrixBlue, 6.28318531, axis3 );

var myStartMatrixWhite = matrixHelp.create();
var myRotateMatrixWhite = matrixHelp.create();
var myRotateMatrixWhiteB = matrixHelp.create();
var vector = [0.75,0.75,0,0];
var vector2 = [1.25,1.25,0,0];
myStartMatrixWhite[13]=100;
myStartMatrixWhite[14]=300;
matrixHelp.scale( myRotateMatrixWhite, myStartMatrixWhite, vector);
matrixHelp.scale( myRotateMatrixWhiteB, myStartMatrixWhite, vector2 );

var myStartMatrixGray = matrixHelp.create();
var myRotateMatrixGray = matrixHelp.create();
var myRotateMatrixGrayB = matrixHelp.create();
var myRotateMatrixGrayC = matrixHelp.create();
var myRotateMatrixGrayD = matrixHelp.create();
myStartMatrixGray[13]=500;
myStartMatrixGray[14]=300;
matrixHelp.rotateZ(myRotateMatrixGray, myStartMatrixGray, -1.57079633);
matrixHelp.rotateZ(myRotateMatrixGrayB, myRotateMatrixGray, -1.57079633);
matrixHelp.rotateZ(myRotateMatrixGrayC, myRotateMatrixGrayB, -1.57079633);
matrixHelp.rotateZ(myRotateMatrixGrayD, myRotateMatrixGrayC, -1.57079633);

function animate() {


  rectRed.animateTo({ m11:myRotateMatrixRed[0],m12:myRotateMatrixRed[1],m13:myRotateMatrixRed[2],m14:myRotateMatrixRed[3],
                   m21:myRotateMatrixRed[4],m22:myRotateMatrixRed[5],m23:myRotateMatrixRed[6],m24:myRotateMatrixRed[7],
                   m31:myRotateMatrixRed[8],m32:myRotateMatrixRed[9],m33:myRotateMatrixRed[10],m34:myRotateMatrixRed[11],
                   m41:myRotateMatrixRed[13],m42:myRotateMatrixRed[14],m43:0,m44:1
    },2, scene.PX_LINEAR, scene.PX_END)
      .then(function(o)
      {o.animateTo({ m11:myStartMatrixRed[0],m12:myStartMatrixRed[1],m13:myStartMatrixRed[2],m14:myStartMatrixRed[3],
                     m21:myStartMatrixRed[4],m22:myStartMatrixRed[5],m23:myStartMatrixRed[6],m24:myStartMatrixRed[7],
                     m31:myStartMatrixRed[8],m32:myStartMatrixRed[9],m33:myStartMatrixRed[10],m34:myStartMatrixRed[11],
                     m41:myStartMatrixRed[13],m42:myStartMatrixRed[14],m43:0,m44:1
      },2, scene.PX_LINEAR, 0)});
   
 
  rectGreen.animateTo({ m11:myRotateMatrixGreen2[0],m12:myRotateMatrixGreen2[1],m13:myRotateMatrixGreen2[2],m14:myRotateMatrixGreen2[3],
                   m21:myRotateMatrixGreen2[4],m22:myRotateMatrixGreen2[5],m23:myRotateMatrixGreen2[6],m24:myRotateMatrixGreen2[7],
                   m31:myRotateMatrixGreen2[8],m32:myRotateMatrixGreen2[9],m33:myRotateMatrixGreen2[10],m34:myRotateMatrixGreen2[11],
                   m41:myRotateMatrixGreen2[13],m42:myRotateMatrixGreen2[14],m43:0,m44:1
    },2, scene.PX_LINEAR, scene.PX_END)
      .then(function(o)
      {o.animateTo({ m11:myRotateMatrixGreen2B[0],m12:myRotateMatrixGreen2B[1],m13:myRotateMatrixGreen2B[2],m14:myRotateMatrixGreen2B[3],
                     m21:myRotateMatrixGreen2B[4],m22:myRotateMatrixGreen2B[5],m23:myRotateMatrixGreen2B[6],m24:myRotateMatrixGreen2B[7],
                     m31:myRotateMatrixGreen2B[8],m32:myRotateMatrixGreen2B[9],m33:myRotateMatrixGreen2B[10],m34:myRotateMatrixGreen2B[11],
                     m41:myRotateMatrixGreen2B[13],m42:myRotateMatrixGreen2B[14],m43:0,m44:1
      },2, scene.PX_LINEAR, 0)});
    
  
   
  rectBlue.animateTo({ m11:myRotateMatrixBlue[0],m12:myRotateMatrixBlue[1],m13:myRotateMatrixBlue[2],m14:myRotateMatrixBlue[3],
                   m21:myRotateMatrixBlue[4],m22:myRotateMatrixBlue[5],m23:myRotateMatrixBlue[6],m24:myRotateMatrixBlue[7],
                   m31:myRotateMatrixBlue[8],m32:myRotateMatrixBlue[9],m33:myRotateMatrixBlue[10],m34:myRotateMatrixBlue[11],
                   m41:myRotateMatrixBlue[13],m42:myRotateMatrixBlue[14],m43:0,m44:1
    },2, scene.PX_LINEAR, scene.PX_END)
      .then(function(o)
      {o.animateTo({ m11:myRotateMatrixBlueB[0],m12:myRotateMatrixBlueB[1],m13:myRotateMatrixBlueB[2],m14:myRotateMatrixBlueB[3],
                     m21:myRotateMatrixBlueB[4],m22:myRotateMatrixBlueB[5],m23:myRotateMatrixBlueB[6],m24:myRotateMatrixBlueB[7],
                     m31:myRotateMatrixBlueB[8],m32:myRotateMatrixBlueB[9],m33:myRotateMatrixBlueB[10],m34:myRotateMatrixBlueB[11],
                     m41:myRotateMatrixBlueB[13],m42:myRotateMatrixBlueB[14],m43:0,m44:1
      },2, scene.PX_LINEAR, scene.PX_END)});


 

    
  rectWhite.animateTo({ m11:myRotateMatrixWhite[0],m12:myRotateMatrixWhite[1],m13:myRotateMatrixWhite[2],m14:myRotateMatrixWhite[3],
                   m21:myRotateMatrixWhite[4],m22:myRotateMatrixWhite[5],m23:myRotateMatrixWhite[6],m24:myRotateMatrixWhite[7],
                   m31:myRotateMatrixWhite[8],m32:myRotateMatrixWhite[9],m33:myRotateMatrixWhite[10],m34:myRotateMatrixWhite[11],
                   m41:myRotateMatrixWhite[13],m42:myRotateMatrixWhite[14],m43:0,m44:1
    },2, scene.PX_LINEAR, scene.PX_END)
      .then(function(o)
      {o.animateTo({ m11:myRotateMatrixWhiteB[0],m12:myRotateMatrixWhiteB[1],m13:myRotateMatrixWhiteB[2],m14:myRotateMatrixWhiteB[3],
                     m21:myRotateMatrixWhiteB[4],m22:myRotateMatrixWhiteB[5],m23:myRotateMatrixWhiteB[6],m24:myRotateMatrixWhiteB[7],
                     m31:myRotateMatrixWhiteB[8],m32:myRotateMatrixWhiteB[9],m33:myRotateMatrixWhiteB[10],m34:myRotateMatrixWhiteB[11],
                     m41:myRotateMatrixWhiteB[13],m42:myRotateMatrixWhiteB[14],m43:0,m44:1
      },2, scene.PX_LINEAR, 0)
        .then(function(o)
          {o.animateTo({ m11:myStartMatrixWhite[0],m12:myStartMatrixWhite[1],m13:myStartMatrixWhite[2],m14:myStartMatrixWhite[3],
                         m21:myStartMatrixWhite[4],m22:myStartMatrixWhite[5],m23:myStartMatrixWhite[6],m24:myStartMatrixWhite[7],
                         m31:myStartMatrixWhite[8],m32:myStartMatrixWhite[9],m33:myStartMatrixWhite[10],m34:myStartMatrixWhite[11],
                         m41:myStartMatrixWhite[13],m42:myStartMatrixWhite[14],m43:0,m44:1
          },2, scene.PX_LINEAR, scene.PX_END)
          });});
 
 
  rectGray.animateTo({ m11:myRotateMatrixGray[0],m12:myRotateMatrixGray[1],m13:myRotateMatrixGray[2],m14:myRotateMatrixGray[3],
                   m21:myRotateMatrixGray[4],m22:myRotateMatrixGray[5],m23:myRotateMatrixGray[6],m24:myRotateMatrixGray[7],
                   m31:myRotateMatrixGray[8],m32:myRotateMatrixGray[9],m33:myRotateMatrixGray[10],m34:myRotateMatrixGray[11],
                   m41:myRotateMatrixGray[13],m42:myRotateMatrixGray[14],m43:0,m44:1
      },2, scene.PX_LINEAR, scene.PX_END)
        .then(function(o)
        {o.animateTo({ m11:myRotateMatrixGrayB[0],m12:myRotateMatrixGrayB[1],m13:myRotateMatrixGrayB[2],m14:myRotateMatrixGrayB[3],
                       m21:myRotateMatrixGrayB[4],m22:myRotateMatrixGrayB[5],m23:myRotateMatrixGrayB[6],m24:myRotateMatrixGrayB[7],
                       m31:myRotateMatrixGrayB[8],m32:myRotateMatrixGrayB[9],m33:myRotateMatrixGrayB[10],m34:myRotateMatrixGrayB[11],
                       m41:myRotateMatrixGrayB[13],m42:myRotateMatrixGrayB[14],m43:0,m44:1
        },2, scene.PX_LINEAR, scene.PX_END)
          .then(function(o)
          {o.animateTo({ m11:myRotateMatrixGrayC[0],m12:myRotateMatrixGrayC[1],m13:myRotateMatrixGrayC[2],m14:myRotateMatrixGrayC[3],
                         m21:myRotateMatrixGrayC[4],m22:myRotateMatrixGrayC[5],m23:myRotateMatrixGrayC[6],m24:myRotateMatrixGrayC[7],
                         m31:myRotateMatrixGrayC[8],m32:myRotateMatrixGrayC[9],m33:myRotateMatrixGrayC[10],m34:myRotateMatrixGrayC[11],
                         m41:myRotateMatrixGrayC[13],m42:myRotateMatrixGrayC[14],m43:0,m44:1
          },2, scene.PX_LINEAR, scene.PX_END)
            .then(function(o)
            {o.animateTo({ m11:myRotateMatrixGrayD[0],m12:myRotateMatrixGrayD[1],m13:myRotateMatrixGrayD[2],m14:myRotateMatrixGrayD[3],
                           m21:myRotateMatrixGrayD[4],m22:myRotateMatrixGrayD[5],m23:myRotateMatrixGrayD[6],m24:myRotateMatrixGrayD[7],
                           m31:myRotateMatrixGrayD[8],m32:myRotateMatrixGrayD[9],m33:myRotateMatrixGrayD[10],m34:myRotateMatrixGrayD[11],
                           m41:myRotateMatrixGrayD[13],m42:myRotateMatrixGrayD[14],m43:0,m44:1
            },2, scene.PX_LINEAR, scene.PX_END)
            

          .then(function(o) {
            // DO A LOOP based on the longest animation
            animate();});});});}); 
            
                     
}


animate();

