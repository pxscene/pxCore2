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
