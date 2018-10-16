/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

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
	func();
	vObj.propC();
	vObj.methodA();
	print('methodB: ' + vObj.methodB());
	print('methodC: ' + vObj.methodC("hi"));
}

var vObj = _testObjectReturnFunc();
vObj.methodD = function() {};
print('methodD: ' + vObj.methodD);
