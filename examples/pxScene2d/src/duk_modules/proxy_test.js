
px.import({ scene: 'px:scene.1' }).then( function importsAreReady(imports)
{
    {
        var vArr = _testArrayReturnFunc();
        print('length=' + vArr.length);
        for (i = 0; i < vArr.length; ++i ){
            print('idx[' + i + '] ' +vArr[i]);
        }
    
        vArr[3] = 4;
    
        print('length=' + vArr.length);
        for (i = 0; i < vArr.length; ++i ){
            print('idx[' + i + '] ' +vArr[i]);
        }
    }

    {
        var vMap = _testMapReturnFunc();
        print("a1: " + vMap["a1"]);
        print("a2: " + vMap["a2"]);
        print("a3: " + vMap.a3);
        vMap["a4"] = 4;
        print("a4: " + vMap.a4);
    }

    {
        var vObj = _testObjectReturnFunc();
        print("propA: " + vObj.propA);
        print("propB: " + vObj.propB);
        vObj.propA = 3;
        print("propA: " + vObj.propA);
        vObj.propC = function () { print("propC()"); return 1; };
        print("propC: " + vObj.propC);
        var func = vObj.propC;
        func.apply();
        func();
        vObj.propC.apply();
        vObj.propC();
        vObj.methodA();
        print('methodB: ' + vObj.methodB());
        print('methodC: ' + vObj.methodC("hi"));
    }

    var vObj = _testObjectReturnFunc();
    vObj.methodD = function() {};
    print('methodD: ' + vObj.methodD);

}).catch( function importFailed(err){
    console.error("Import failed for proxy_test.js: " + err.stack);
});
