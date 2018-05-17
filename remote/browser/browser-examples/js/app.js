/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the app root script
 *
 * @author      TCSCODER
 * @version     1.0
 */

(() => {
  /**
   * the remote server uri
   * @type {string}
   */
  var websocketURI = '';
  /**
   * the remote defined object names
   * @type {string[]}
   */
  var remoteObjectNames = [ 'host_object', 'obj2', 'obj3', 'obj4' ];
  
  /**
   * the three test objects for three tab
   */
  var testObjects = {
    type: new TestObject(), // type test
    method: new TestObject(), // method test
    multi: new TestObject(), // multi test object
  };
  
  var progressBar = $('#progressBar');  // the progress bar html element
  var totalCount = $('#total'); // the total count html element
  var succeedCount = $('#succeed'); // the suceed count html element
  var exampleTitle = $('#example-title'); // the tab title html element
  var runBtn = $('#run-btn'); // the run all button
  var clearBtn = $('#clear-btn'); // the clear button
  var exampleItemList = $('.example-list'); // the example list container
  
  var currentType = ''; // current tab type
  var timeInterval = null; // this used to save interval handler
  
  /**
   * update html progress bar
   * @param value the progress value
   */
  function updateProgressBar(value) {
    value = isNaN(value) ? 0 : value;
    progressBar.css('width', value.toFixed(2) + '%');
    progressBar.html(value.toFixed(2) + '%');
  }
  
  /**
   * update tab header information
   * @param title the header title
   * @param success the success count number
   * @param total the total number
   * @param testObject the current test object
   */
  function updateInformation(title, success, total, testObject) {
    exampleTitle.html(title + testObject.message);
    succeedCount.html(success);
    totalCount.html(total);
  }
  
  /**
   * updatet the two button status
   * @param testObj the current test object
   */
  function updateButton(testObj) {
    var finished = testObj.finished;
    
    if (!testObj.initialized) {
      runBtn.html('Waiting Initialized');
      runBtn.addClass('disabled');
      clearBtn.addClass('disabled');
      return;
    }
    
    if (finished) {
      runBtn.removeClass('disabled');
      runBtn.html('Run All Examples');
      clearBtn.removeClass('disabled');
    } else {
      runBtn.html('Runing');
      runBtn.addClass('disabled');
      clearBtn.addClass('disabled');
    }
  }
  
  /**
   * selected tab, update tab header and example list
   * @param type the tab type
   * @param title the tab title
   */
  function selected(type, title) {
    
    // clear time interval
    if (timeInterval) {
      clearInterval(timeInterval);
    }
    
    // add active style to current tab
    $('.nav-underline .nav-link').removeClass('active');
    $('#' + type).addClass('active');
    currentType = type;
    
    // update information
    updateInformation(title, testObjects[ type ].succeed, testObjects[ type ].total, testObjects[ type ]);
    updateProgressBar(testObjects[ type ].succeed * 100 / testObjects[ type ].total);
    updateButton(testObjects[ type ]);
    
    exampleItemList.find('.texample-item').remove();
    initExampleListItem();  // create dom example items
    updateExampleListItem(); // update items
    
    // start time interval to update test item details
    timeInterval = setInterval(() => {
      var currentObj = testObjects[ currentType ];
      updateInformation(title, currentObj.succeed, currentObj.total, currentObj);
      updateProgressBar(currentObj.succeed * 100 / currentObj.total);
      updateButton(testObjects[ type ]);
      updateExampleListItem();
    }, 300);
  }
  
  /**
   * create dom test items for current tab
   */
  function initExampleListItem() {
    var currentObj = testObjects[ currentType ];
    for (var i = 0; i < currentObj.items.length; i++) {
      appendExampleItem(currentObj.items[ i ]);
    }
  }
  
  /**
   * update current tab example test items
   */
  function updateExampleListItem() {
    var currentObj = testObjects[ currentType ];
    for (var i = 0; i < currentObj.items.length; i++) {
      var item = currentObj.items[ i ];
      if (!item.node) return;
      item.node.find('.title').html(item.title);
      item.node.find('.description').html(item.description + item.additionText);
      var statusNode = item.node.find('.status');
      statusNode.html(item.status.toUpperCase());
      statusNode.removeClass('pending');
      statusNode.removeClass('passed');
      statusNode.removeClass('failed');
      statusNode.addClass(item.status);
    }
  }
  
  /**
   * append a test item dom node to list
   * @param item the test item data
   */
  function appendExampleItem(item) {
    var node = $('<div class="media text-muted pt-3 texample-item border-bottom">\n' +
      '      <p class="media-body pb-3 mb-0 small lh-125 border-gray item-body">\n' +
      '        <strong class="d-block text-gray-dark title"></strong>\n' +
      '        <span class="description"> </span>\n' +
      '      </p>\n' +
      '      <div class="status"></div>\n' +
      '    </div>');
    item.node = node;
    exampleItemList.append(node);
  }
  
  /**
   * get remote object from remote server and create test items
   * @param type the test type
   * @param method the get remote object and create test items function
   */
  function initTestObject(type, method) {
    var testObject = testObjects[ type ];
    const updateSelected = () => {
      if (currentType === type) {
        selected(type, $('#' + type).html());
      } else if (currentType === '' && type === 'type') {
        selected(type, $('#' + type).html());
      }
    };
    testObject.message = ' - Connectting to ' + websocketURI + ' ...';
    updateSelected(); // update title
    
    return method(websocketURI, remoteObjectNames).then(rsp => {
      // get and create test items succeed
      if (!rsp.items) return rsp;
      testObject.init(rsp.items);
      testObject.message = ' - Connected';
      rsp.rtObject.protocol.transport.on('close', () => {
        testObject.message = ' - Lost connection, please re initialize';
        testObject.clear();
        testObject.initialized = false;
        updateSelected();
        // setTimeout(() => initTestObject(type, method), 2000);
      });
      updateSelected();
      return rsp;
    }).catch(err => {
      testObject.message = ' - Connect failed, please re initialize';
      updateSelected();
      common.showMessage(null, "Connect failed.");
      // setTimeout(() => initTestObject(type, method), 2000);
    });
  }
  
  /**
   * add click event for tab item
   */
  $('.nav-underline .nav-link').click(function () {
    selected($(this).attr('id'), $(this).html());
  });
  
  /**
   * add click event for run button
   */
  runBtn.click(function () {
    if ($(this).hasClass('disabled')) {
      return;
    }
    var currentObj = testObjects[ currentType ];
    if (!currentObj.finished) {
      return;
    }
    currentObj.run();
    updateButton(currentObj);
  });
  
  /**
   * add click event for clear button
   */
  clearBtn.click(function () {
    if ($(this).hasClass('disabled')) {
      return;
    }
    testObjects[ currentType ].clear();
    selected(currentType, $('#' + currentType).html());
  });
  
  $('.btn-init').click(function () {
    
    websocketURI = $('.input-entity').val();
    if (!websocketURI || websocketURI.trim().length === 0) {
      common.showMessage(null, "remote server should not be empty!");
      return;
    }
    websocketURI = 'ws://' + websocketURI;
    initTestObject('type', createTypeTestExamples).then((r) => { // init type test
      if (!r.items) return;
      initTestObject('method', createMethodTestExamples); // init method test
      initTestObject('multi', createMultiTestExamples); // init multi object test
    });
  });
  $('#type').trigger('click');
  
})();
