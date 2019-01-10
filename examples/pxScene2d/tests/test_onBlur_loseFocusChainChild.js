// This is invoked by test_onBlur_loseFocusChain.js. Refer to test_onBlur_loseFocusChain.js for test details.

px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;
var resolveCallback;

var bg = scene.create({
    t: 'rect',
    parent: root,
    h: root.h,
    w: root.w,
    fillColor:0x000000ff,
    id:"bg"
});

bg.on('onBlur',handleOnBlurEvent_bg);

var cont1 = scene.create({
    t: 'rect',
    parent: bg,
    x: 20,
    y: 20,
    h: bg.h - 50,
    w: bg.w - 50,
    fillColor:0xBABABAFF,
    id:"cont1"
});

cont1.on('onBlur',handleOnBlurEvent);

var group1 = scene.create({
    t: 'rect',
    parent: cont1,
    x: 20,
    y: 20,
    h: 300,
    w: 300,
    fillColor:0x2B9CD8ff,
    id:"group1"
});

group1.on('onBlur',handleOnBlurEvent);

var btn11 = scene.create({
    t: 'rect',
    parent: group1,
    x: 20,
    y: 20,
    h: 60,
    w: group1.w - 40,
    fillColor:0x000000ff,
    id:"btn11"
});

btn11.on('onBlur',handleOnBlurEvent);

var btn12 = scene.create({
    t: 'rect',
    parent: group1,
    x: 20,
    y: 100,
    h: 60,
    w: group1.w - 40,
    fillColor:0x000000ff,
    id:"btn12"
});

btn12.on('onBlur',handleOnBlurEvent);

var group2 = scene.create({
    t: 'rect',
    parent: cont1,
    x: 20,
    y: 340,
    h: 300,
    w: 300,
    fillColor:0x2B9CD8ff,
    id:"group2"
});

group2.on('onBlur',handleOnBlurEvent);

var btn21 = scene.create({
    t: 'rect',
    parent: group2,
    x: 20,
    y: 20,
    h: 60,
    w: group2.w - 40,
    fillColor:0x000000ff,
    id:"btn21"
});

btn21.on('onBlur',handleOnBlurEvent);

var btn22 = scene.create({
    t: 'rect',
    parent: group2,
    x: 20,
    y: 100,
    h: 60,
    w: group2.w - 40,
    fillColor:0x000000ff,
    id:"btn22"
});

btn22.on('onBlur',handleOnBlurEvent);

module.exports.actual_loseFocusChain = [];
module.exports.actual_onBlurTarget = [];

module.exports.launchTest1 = function() 
{
    return new Promise(function (resolve, reject) 
    {
        console.log("setting focus to btn12");

        resolveCallback = resolve;
        
        btn12.focus = true;
        
        console.log("\nChanging focus from btn12 to btn21");
        
        btn21.focus = true;
    });
}

module.exports.launchTest2 = function(value) 
{
    return new Promise(function (resolve, reject) 
    {
        console.log("\nChanging focus from btn21 to btn22");

        resolveCallback = resolve;
    
        btn22.focus = true;
    });
}

function handleOnBlurEvent(e)
{
    module.exports.actual_onBlurTarget.push(e.target.id);

    if(e.loseFocusChain === true)
    {
        module.exports.actual_loseFocusChain.push(true);
        console.log("In onBlur Event, target:- " + e.target.id + ", loseFocusChain:- true\n");
    }
    else
    {
        module.exports.actual_loseFocusChain.push(false);
        console.log("In onBlur Event, target:- " + e.target.id + ", loseFocusChain:- false\n");
    }
}

function handleOnBlurEvent_bg(e)
{
    module.exports.actual_onBlurTarget.push(e.target.id);

    if(e.loseFocusChain === true)
    {
        module.exports.actual_loseFocusChain.push(true);
        console.log("In onBlur Event, target:- " + e.target.id + ", loseFocusChain:- true\n");
    }
    else
    {
        module.exports.actual_loseFocusChain.push(false);
        console.log("In onBlur Event, target:- " + e.target.id + ", loseFocusChain:- false\n");
    }

    // the onBlur for bg should be the last one to be called. We can now resolve the promise.
    console.log("All onBlur events have been handled. Verify loseFocusChain is set");
    resolveCallback();
}

}).catch(function importFailed(err){
    console.error("Import failed for test_onBlur_loseFocusChain.js: " + err);
});
