/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The Test object for manager tests
 *
 * @author      TCSCODER
 * @version     1.0
 */

/**
 * test object constructor
 */
function TestObject() {}

/**
 * succeed number for this test
 * @type {number}
 */
TestObject.prototype.succeed = 0;

/**
 * total number for this test
 * @type {number}
 */
TestObject.prototype.total = 0;

/**
 * is finished
 * @type {boolean}
 */
TestObject.prototype.finished = true;

/**
 * is this object initialized
 * @type {boolean}
 */
TestObject.prototype.initialized = false;

/**
 * addtional message for this test object
 * @type {string}
 */
TestObject.prototype.message = '';

/**
 * the examples items
 * @type {Array}
 */
TestObject.prototype.items = [];

/**
 * init test object
 * @param items the example tests
 */
TestObject.prototype.init = function (items) {
  this.items = items;
  this.total = this.items.length;
  this.initialized = true;
};

/**
 * run all example items, and update items status
 */
TestObject.prototype.run = function () {
  this.clear();

  var index = 0;
  var that = this;

  function _run() {
    if (index >= that.items.length) {
      that.finished = true;
      return;
    }
    that.items[ index ].task(that.items[ index ]).then(() => {
      index++;
      that.succeed += 1;
      setTimeout(() => _run(), 100);
    }).catch((err) => {
      index++;
      console.error(err);
      err.item.status = 'failed';
      err.item.additionText = '<span class="red-text"> , error = ' + err.err + '</span>';
      _run();
    });
  }

  _run();
};

/**
 * clear this test object, example items and status
 */
TestObject.prototype.clear = function () {
  this.finished = true;
  this.succeed = 0;
  for (var i = 0; i < this.items.length; i++) {
    this.items[ i ].additionText = '';
    this.items[ i ].status = 'pending';
  }
};

window.TestObject = TestObject;
