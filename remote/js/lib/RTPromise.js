const logger = require('./common/logger');
const repl = require('repl');

/**
 * the remote promise
 */
class RTPromise {
  /**
   * create new RT Promise
   * @param promise
   * @param id
   */
  constructor(promise, id) {
    this.ret = null;
    this.status = '<PENDING>';
    this.promise = promise;

    this.resolveCb = r => r;
    this.errorCb = r => r;

    this.promise.then((result) => {
      this.status = '<DONE>';
      logger.debug(`${id || ''} promise completed ${this.status}.`);
      this.ret = this.resolveCb(result);
      process.stdout.write('\n> ');
    }).catch((err) => {
      this.status = `<ERROR> - ${err}`;
      logger.debug(`${id || ''} promise completed ${this.status}.`);
      process.stdout.write('\n> ');
    });
  }

  then(cb) {
    this.resolveCb = cb;
    return this;
  }

  catch(cb) {
    this.errorCb = cb;
    return this;
  }

  get() {
    return this.ret;
  }
}

module.exports = RTPromise;
