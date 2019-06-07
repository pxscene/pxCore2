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

px.import('px:scene.1.js').then(scene => {

  const p = Promise.resolve();
  scene.on("p", () => p);
  scene.on("p_", _p => _p === p);

  const b = true;
  scene.on("b", () => b);
  scene.on("b_", _b => _b === b);

  const s = 'str';
  scene.on("s", () => s);
  scene.on("s_", _s => _s === s);

  const i = 123;
  scene.on("i", () => i);
  scene.on("i_", _i => _i === i);

  const d = 1.23;
  scene.on("d", () => d);
  scene.on("d_", _d => _d === d);

  const buf = new Buffer('');
  scene.on("buf", () => buf);
  scene.on("buf_", _buf => _buf === buf);

  const fn = () => {};
  scene.on("fn", () => fn);
  scene.on("fn_", _fn => _fn === fn);

  const o = {};
  scene.on("o", () => o);
  scene.on("o_", _o => _o === o);

  const a = [];
  scene.on("a", () => a);
  scene.on("a_", _a => _a === a);
});
