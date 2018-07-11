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

const replLib = require('repl');
const rtRemote = require('./lib');

const repl = replLib.start({});
const showPrompt = () => repl.displayPrompt();
const { context } = repl;

rtRemote.setEndFunction(showPrompt);
// inject rtRemote into repl context
context.rtRemote = rtRemote;

const realEval = repl.eval;
const promiseEval = (cmd, ct, filename, callback) => {
  realEval.call(repl, cmd, ct, filename, (err, res) => {
    if (err) {
      return callback(err);
    }
    return callback(null, res);
  });
};
repl.eval = promiseEval;


/**
 * exit cli mode
 */
repl.on('exit', () => {
  process.exit(0);
});
