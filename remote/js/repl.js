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
const rtRemote = require('./lib');


const replico = require('replico');

const repl = replico({
  prompt: '> ',
  terminal: true,
});
const showPrompt = () => repl.displayPrompt();
const { context } = repl;

rtRemote.setEndFunction(showPrompt);
// inject rtRemote into repl context
context.rtRemote = rtRemote;


repl.eval = function (cmd, ct, filename, callback) {
  if (cmd === 'foo') {
    // ... your own implementation
    return callback(null, 'bar');
  } else if (/yield/.test(cmd)) {
    // Or use replico's eval (with bluebird-co):
    return replico.coEval.call(this, cmd, ct, filename, callback);
  }
  // If you need it, superEval is a reference to the default repl eval
  return this.superEval(cmd, ct, filename, callback);
};


/**
 * exit cli mode
 */
repl.on('exit', () => {
  process.exit(0);
});
