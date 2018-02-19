# Spark Graph API

A 2d scene graph API supporting the following:

* 2d hierarchical affine scene graph;
* Small set of atomic UI elements;
* Complex components built through composition;
* Animation primitives;
* Rich masking primitives;
* More to come...

### `Scene` instance

**Properties**

`root` *object* - returns the root `Object` for the scene;  
`w` *num* - returns the width in pixel-sized units of the scene;  
`h` *num* - returns the height in pixel-sized units of the scene;  
`animation`	 *object* - `animation` for animation-related constants;  
`stretch`  *object* - `stretch` for x/y stretch-related constants (for `image`);  
`alignVertical`	*object* - `alignVertical` for vertical alignment-related constants (for `textBox`);  
`alignHorizontal` *object* - `alignHorizontal` for horizontal alignment-related constants (for `textBox`);  
`truncation` *object* - `truncation` for truncation-related constants (for `textBox`);

**Methods**

`create(json)`	*object* - create a subtree of scene graph `Objects` ;

use `json t:` to construct specific object types:
* `text`;
* `textBox`;
* `wayland`;
* `image`;
* `image9`;
* `rect`;
* `object`;

`create(json)` *resource* - create resources that can be shared;  
use `json t:` to construct specific resource types:
* `imageResource`;
* `fontResource`;

`on(string, function)` - registers a function callback for the specified event:
* `onMouseDown` - parameter object has properties: x, y, flags;
* `onMouseUp` - parameter object has properties: x, y, flags;
* `onMouseMove` - parameter object has properties: x, y;
* `onMouseEnter` - parameter object has properties: target, stopPropagation();
* `onMouseLeave` - parameter object has properties: target, stopPropagation();
* `onFocus` - parameter object has properties: target, stopPropagation();
* `onBlur` - parameter object has properties: target, stopPropagation();
* `onKeyDown` - parameter object has properties: target, keyCode, flags, stopPropagation();
* `onKeyUp` - parameter object has properties: target, keyCode, flags, stopPropagation();
* `onChar` - parameter object has properties: target, charCode, stopPropagation();
* `onResize` - parameter object has properties w, h;

`delListener(string, function)` - unregister a callback function for the specified event;
`getFocus(Object)` - will return the object that has keyboard focus;

### `Object` instance

**Properties**

`parent` *object*		- 	returns this object's parent `Object`;  
`x` *num* - x-coordinate used as input into the object's transform function in pixel units;  
`y` *num* - y-coordinate used as input into the object's transform function in pixel units;  
`w` *num* - pixel unit width of the object;  
`h` *num* - pixel unit height of the object;  
`cx` *num* - x offset used as the center of rotation and scale;  
`cy` *num* - y offset used as the center of rotation and scale;  
`sx` *num* - scale factor in the x dimension;  
`sy` *num* - scale factor in the y dimension;  
`a` *num* - opacity of the object `[0-1]`;  
`r` *num* - angle of rotation in degrees;  
`id` *string* - user-assigned associated string identifier for the object, defaults to "";  
`interactive` *bool* - determines whether the object is mouse interactive. defaults to true;  
`painting`  *bool* - when set to false the object and it's children will be snapshot'ted; when set to true the object and it's children will immediately reflect any changes;  
`clip`  *bool* - determines whether the drawing done by the object and it's children will be clipped by the objects w and h properties. defaults to false;  
`mask`  *bool* - determines whether this object will be used to define an alpha layer mask for the siblings of this object. defaults to false;  
`draw`  *bool* - determines whether this object will be drawn;  
`focus`  *bool* - when set to true, this object will be given keyboard focus. the object that had previous focus will be blurred;  
`numChildren` *num* - returns the number of immediate child objects that this object parents;  
`children` *Array* - returns a collection of child objects for this object;

**Methods**

`getChild(i)` *object* - returns the i-th child for this object;  
`remove` - removes this object from the scene;  
`removeAll` - removes all children and their descendants from the scene;  
`moveToFront` - moves this child to the top of the z-order within its siblings;  
`moveToBack` - moves this child to the bottom of the z-order within its siblings;  
`moveForward` - moves this child one forward in the z-order within its siblings;  
`moveBackward` - moves this child one backward in the z-order within its siblings;   

`animateTo(json, duration, tween, type, count)` *promise* - tween the object's properties to the targets specified over the specified duration using the tween function;
* `tween` indicates the interpolator to be used for the animation. See `animation constants` for valid values. Defaults to TWEEN_LINEAR;
* `type` (option) determines whether the animation loops or oscillates. See `animation constants` for valid values. Defaults to OPTION_LOOP;
* `count` is the number of times the animation should repeat. Defaults to 1. Use COUNT_FOREVER to create an animation that won't end until canceled;  
returns a promise that will fulfill when the animation is complete;   

`on(string, function)` - registers a function callback for the specified event:
* `onMouseDown` - parameter object has properties: x, y, flags;
* `onMouseUp` - parameter object has properties: x, y, flags;
* `onMouseMove` - parameter object has properties: x, y;
* `onMouseEnter` - parameter object has properties: target, stopPropagation();
* `onMouseLeave` - parameter object has properties: target, stopPropagation();
* `onFocus` - parameter object has properties: target, stopPropagation();
* `onBlur` - parameter object has properties: target, stopPropagation();
* `onKeyDown` - parameter object has properties: target, keyCode, flags, stopPropagation();
* `onKeyUp` - parameter object has properties: target, keyCode, flags, stopPropagation();
* `onChar` - parameter object has properties: target, charCode, stopPropagation();
* `onSize` - parameter object has properties w, h;

`delListener(string, function)` - unregister a callback function for the specified event;  
`getObjectById(id)` - returns object with associated id. search scope is this object and its children;

### `Rectangle` instance inherits from `Object`

`fillColor` *num* - specifies the color to ﬁll the interior of the rectangle. eg. `0xff0000ff`, opaque red, `0xrrggbbaa`;  
`lineColor` *num* - specifies the color to stroke the edge of the rectangle. eg. `0xff0000ff`, opaque red, `0xrrggbbaa`;  
`lineWidth` *num* - specifies the width of the line to stroke around the ege of the rectangle. defaults to 0;

### `Image` instance inherits from `Object`

**Properties**

`url` *string* - specifies the url of the image to load. supports .png and .jpg;  
`stretchX` *num* - specifies the stretch option to use for the x-coordinate. See `stretch constants`;  
`stretchY` *num* - specifies the stretch option to use for the y-coordinate. See `stretch constants`;  
`ready`  *promise* -  returns a promise for when the object is ready;  
`resource` *resource* - specifies the `imageResource` to be used by this image;

Note that setting url will create an imageResource and set the resource property for this image;  
Note that w or h equal to -1 will cause the image to be rendered using the `imageResource`'s associated property.

### `Image9` instance inherits from `Object`

**Properties**

`url` *string* - specifies the url of the image to load. supports .png and .jpg;  
`insetLeft` *num* - specifies the first slicing offset on the x-axis;  
`insetTop` *num* - specifies the first slicing offset on the y-axis;  
`insetRight` *num* - specifies the second slicing offset on the x-axis;  
`insetBottom` *num* - specifies the second slicing offset on the y-axis;  
`ready`  *promise* -  returns a promise for when the object is ready;  
`resource` *resource* - specifies the `imageResource` to be used by this image9;  **note** that setting url will create an imageResource and set the resource property for this `image9`;

### `Text` instance inherits from `Object`

**Properties**

`text` *string* - specifies the text to render;  
`textColor`  *num* - specifies the color to render the text. eg. `0xff0000ff`,opaque red ,`0xrrggbbaa`. defaults to opaque black `0x000000ff`;  
`pixelSize`  *num* - specifies the pixelSize of the rendered text;  
`fontUrl` *string* - specifies the url of the face to use;  
`ready`  *promise* -  returns a promise for when the object is ready;  
`font` *resource* - specifies the `fontResource` to be used by this text; **note** that setting url will create a `fontResource` and set the font property for this text;

### `TextBox` instance inherits from `Text`

Unlike `Text`, the height and width of `TextBox` does NOT change based on the text value; instead, the height and width values in constructor and assignments are respected, and the text is rendered within those dimensions, according to the property values, below.  

Behaviors for alignment, wordWrap and clip:

* If truncation is none and wrap is true, then the text will continue to ﬂow beyond the height of the bounding rectangle, possibly cut off if clip is applied;
* If truncation is none, and wrap is false, then the line(s) (that /n case has to be taken into consideration) should be centered in the view, even if the text goes beyond its x/width boundaries. If clip is true, text is cut off on either end. Such as:

`alignHorizontal CENTER`

```
The qu|ick brown fox jumped over the la|zy dog
      |ick brown fox jumped over the la|
```

`alignHorizontal RIGHT`

```
The quick br|own fox jumped over the lazy dog|
            |own fox jumped over the lazy dog|
```

**Properties**

`wordWrap` *bool* - when width > 0, and wordWrap is true, wraps text at width of text box. defaults to false;  
`xStartPos` *num* - x position relative to the text box object, in pixels, for the first line of text only, similar to an indent. defaults to 0;  
`xStopPos` *num* - x position where text must stop flowing on the last line of text only. If ellipses are applied, the ellipses must also stop at this location. defaults to 0;

**NOTE**: `xStartPos` and `xStopPos` are only applied when `horizontalAlign=LEFT`.  
**NOTE**: `xStopPos` can only be applied when truncation is true.

`ellipsis` *bool* - indicates if ellipsis shall be shown when truncation occurs. Ignored if `truncation=NONE`; defaults to false;  
`truncation` *num* - see `truncation constants`; defaults to `NONE`;  
`alignHorizontal` *num* - see `horizontal alignment constants`. defaults to `LEFT`;  
`alignVertical` *num* - see `vertical alignment constants`; defaults to `TOP`;  
`leading`  *num* - number of pixels of height to apply between lines of text when multiple lines of text exist. May be negative. defaults to 0;

**Methods**

`measureText` *object* - returns an object with the following properties (measurements are relative to (x,y) of the TextBox object):
* `bounds` - *object* - `{x1:0, y1:0, x2:0, y2:0}`- The two points representing the bounding rectangle of the complete text;
* `charFirst` - `{x:0, y:0}` - The x position represents the left most rendered pixel of the first character on the first line of text. The y position represents the baseline;
* `charLast` - `{x:0, y:0}` - The x position represents the right most rendered pixel of the last character on the last line of text. The y position represents the baseline;

### `Wayland` instance inherits from `Object`

**Properties**

`displayName` *string* - specifies the name of the Wayland display. If no value is assigned a unique name will be generated cmd;  
`cmd` *string* - specifies a command to launch a Wayland client app to connect to the display. Consists of the client name optionally followed by arguments;  
`remoteServer` *string* -  specifies the name of an rtRemote server;  
`clientPID` *num* -  gives the process pid of the connected Wayland client;  
`ﬁllColor` *num* -  specifies the color to ﬁll the interior of the element. eg. `0xff0000ff`, opaque red, `0xrrggbbaa`. This will provide background for the connected Wayland client;  
`hasApi` *bool* -  indicates if the element exports an API;  
`remoteReady` *promise* -  returns a promise for when the remote object is ready;

**Methods**

`suspend(bool)`  - causes the element to cease drawing;  
`resume(bool)`  - causes the element to resume drawing;  
`destroy(bool)`  - destroys the element;

### `SceneContainer` instance inherits from `Object`

**Properties**

`url` *string* - specifies the url of the JavaScript to use for the child scene;

### `Resource` instance

**Note**: Once Resource is created, the url and proxy are readonly.

**Properties**

`url` *string* - specifies the url of the JavaScript to use for the child scene;  
`proxy` *string* - http proxy url to be used while downloading;  
`ready`  *promise* -  returns a promise for when the object is ready;  
`loadStatus` *Array* - array of status values (Read only);

### `imageResource` instance inherits from `Resource`

**Properties**

`w` *num* -  pixel unit width of the image;  
`h` *num* - pixel unit height of the image;

### `fontResource` instance inherits from `Resource`

**Methods**

`getFontMetrics(pixelSize)` object - Returns information about the font based on `pixelSize`. See section 3.a in http://www.freetype.org/freetype2/docs/tutorial/step2.html. The returned object has the following properties:
* `height` - *float* - the distance between baselines;
* `ascent` - *float* - the distance from the baseline to the font ascender (note that this is a hint, not a solid rule);
* `descent` - *float* - the distance from the baseline to the font descender (note that this is a hint, not a solid rule);
* `naturalLeading` - *float* - the distance that must be placed between two lines of text (See `linegap` in section 2 in http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html#section-1);
* `baseline` - *float* - the baseline position for a line of text ;

`measureText(pixelSize, textToMeasure)` *object* - returns an object from which height and width of the measured text can be obtained. **Note** that this measurement does not consider truncation, word wrapping or alignment that could be used in a `TextBox`containing this text.
* `h` - *float* - the height of the text;
* `w` - *float* - the width of the text;

### `Animate`

The Animate object is returned from the animate() method. It can be used to check on the status of an animation, as well as to cancel the animation.

**Properties**

`done`  *promise* -  promise that indicates completion of the animation;  
`type` *num* -  the type (option) of the animation, e.g. whether the animation loops or oscillates. See `option animation constants` for valid values. (Read only);  
`interp` *num* -  Indicates the interpolator used for this animation. See `interpolator animation constants` for valid values. (Read only);  
`status` *string* -  status of the animation. See `status animation constants` for valid values. Note that this is the constant name returned as a string rather than as its numeric value. (Read only);  
`provduration` *num* -  duration value used when the animate function was called (Read only);  
`provcount` *num* -  count value used when the animate function was called. Could be positive numeric value or a constant. See `animation constants` for valid constant values. (Read only);  
`canceled` *bool* -  reﬂects whether or not the animation has been canceled. (Read only);  
`props` *object* - object whose properties are the properties that are being animated, and those property values are the target values for the animation ;  
`details` *object* - object whose properties are the properties that are being animated, and those property values are an AnimationDetail object reﬂecting the current state of the animation for that property;

**Methods**

`cancel` *object* - cancels the animation;

### `Animation Detail`

**Properties**

`from` *num* - the start value for the current state of this animation. (Read only) ;  
`to` *num* - the end value for the current state of this animation. (Read only) ;  
`duration` *num* - the duration value for the current state of this animation. (Read only) ;  
`count` *num* - current count number for this animation. (Read only) ;  
`status` *string* - current status for this animation. See `status animation constants` for valid values. Note that this is the constant name returned as a string rather than as its numeric value. (Read only) ;

### `Animation Constants`
http://www.pxscene.org/examples/px-reference/gallery/dynamics.js demonstrates the different animation interpolators available.

**Properties**

`interpolators`   
Returns a collection of valid TWEEN_ and EASE_ interpolator constants:

`TWEEN_LINEAR` *num* - constant for specifying a linear tween. constant value is 0;   
`TWEEN_EXP1` *num* - constant for specifying a exp1 tween. constant value is 1;   
`TWEEN_EXP2` *num* - constant for specifying a exp2 tween. constant value is 2;   
`TWEEN_EXP3` *num* - constant for specifying a exp3 tween. constant value is 3;   
`TWEEN_STOP` *num* - constant for specifying a stop (easein) tween. constant value is 4;   
`EASE_IN_QUAD` *num* - constant for specifying a "inquad" tween. constant value is 5;   
`EASE_IN_CUBIC` *num* - constant for specifying a "incubic" tween. constant value is 6;   
`EASE_IN_BACK` *num* - constant for specifying a "inback" tween. constant value is 7;   
`EASE_IN_ELASTIC` *num* - constant for specifying a "easeinelastic" tween. constant value is 8;   
`EASE_OUT_ELASTIC` *num* - constant for specifying a "easeoutelastic" tween. constant value is 9;   
`EASE_OUT_BOUNCE` *num* - constant for specifying a "easeoutbounce" tween. constant value is 10;   

`options`   
Returns a collection of valid options constants:

`OPTION_OSCILLATE` *num* - constant for use as the type in the animateTo method. the animation with oscillate back and forth. constant value is 1;   
`OPTION_LOOP` *num* - constant for use as the type in the animateTo method. the animation will loop continuously. constant value is 2;   
`OPTION_FASTFORWARD` *num* - constant for use as an option in the animateTo method. This value can be OR'd with the OPTION type to indicate that any animations in progress should fast forward to their ending values. constant value is 8;   
`OPTION_REWIND` *num* - constant for use as the type in the animateTo method. This value can be OR'd with the OPTION type to indicate that any animations in progress should rewind to their beginning values. constant value is 2;   

`count`   
Returns a collection of valid count constants:

`COUNT_FOREVER` *num* - constant to use as count to indicate the animation should never end. When using this value, the promise for the animation will resolve immediately. constant value is -1;   

`status`   
Returns a collection of valid status constants:

`IDLE` *num* - constant to indicate that the animation has not yet started. constant value is 0;   
`INPROGRESS` *num* - constant to indicate that the animation is in progress. constant value is 1.;   
`CANCELLED` *num* - constant to indicate that the animation has been cancelled. constant value is 2;   
`ENDED` *num* - constant to indicate that the animation has ended. constant value is 3;   

### `Stretch Constants`

**Properties**

`NONE` *num* - constant for use in image stretch options. constant value is 0;   
`STRETCH` *num* - constant for use in image stretch options. constant value is 1;   
`REPEAT` *num* - constant for use in image stretch options. constant value is 2;   

### `Vertical Alignment Constants`

**Properties**

`TOP` *num* - begin text at the upper-most y-coordinate of the `textBox`; constant value is 0;   
`CENTER` *num* - center text vertically within the `textBox`; constant value is 1;   
`BOTTOM` *num* - justify text to the bottom of the `textBox`; constant value is 2;   

### `Horizontal Alignment Constants`

**Properties**

`LEFT` *num* - left-justify text in the `textBox`; constant value is 0;   
`CENTER` *num* - center text horizontally in the `textBox`; constant value is 1;   
`RIGHT` *num* - right-justify text in the `textBox`; constant value is 2;   

### `Truncation Constants`

**Properties**

`NONE` *num* - text is not truncated; constant value is 0;   
`TRUNCATE` *num* - text is truncated at the bottom of the textBox. The last word may be partially truncated; constant value is 1;   
`TRUNCATE_AT_WORD` *num* - text is truncated at the bottom of the textBox. Truncation occurs at the word boundary; constant value is 2;   

Original document: http://www.pxscene.org/docs/apis/