/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the manual page
 *
 * @author      TCSCODER
 * @version     1.0
 */



(() => {
  
  var RTValueType = RTRemote.RTValueType;
  var RTValueHelper = RTRemote.RTValueHelper;
  var BigNumber = RTRemote.BigNumber;
  
  var initialized = false;
  var websocketURI = '';
  var rtObj = null;
  
  var doingSet = false;
  var doingGet = false;
  
  /**
   *update buttons
   */
  function updateBtns() {
    if (initialized) {
      $('.btn-action-do').removeClass('disabled');
      
      if (doingSet) {
        $('.btn-action-set').html("Waitting");
        $('.btn-action-set').addClass("disabled");
      } else {
        $('.btn-action-set').html("Set new Value");
        $('.btn-action-set').removeClass("disabled");
      }
      
      if (doingGet) {
        $('.btn-action-get').html("Waitting");
        $('.btn-action-get').addClass("disabled");
      } else {
        $('.btn-action-get').html("Get Value");
        $('.btn-action-get').removeClass("disabled");
      }
    } else {
      $('.btn-action-do').addClass('disabled');
    }
  }
  
  /**
   * init websocket
   */
  function init() {
    RTRemote.RTRemoteConnectionManager.getObjectProxy(websocketURI, 'empty').then(rtObject => {
      rtObject.protocol.transport.on('close', () => {
        common.showMessage(null, "Connecttion closed.");
        initialized = false;
        updateBtns();
      });
      common.showMessage("primary", "Connecttion created suceessfully.");
      initialized = true;
      updateBtns();
      rtObj = rtObject;
    }).catch(() => {
      common.showMessage(null, "Connect failed.")
    });
  }
  
  $('.btn-init').click(function () {
    websocketURI = $('.input-entity').val();
    if (!websocketURI || websocketURI.trim().length === 0) {
      common.showMessage(null, "remote server should not be empty!");
      return;
    }
    websocketURI = 'ws://' + websocketURI;
    init();
  });
  
  updateBtns();
  
  $('.btn-action-set').click(function () {
    if ($(this).hasClass('disabled')) {
      return;
    }
    doingSet = true;
    updateBtns();
    rtObj.id = $('#set-obj-name').val();
    var pName = $('#set-prop-name').val();
    var value = $('#set-prop-value').val();
    var type = parseInt($('#set-type').val());
    if (type === RTValueType.UINT64) {
      value = new BigNumber(value);
    } else if (type === RTValueType.INT8
      || type === RTValueType.UINT8
      || type === RTValueType.INT32
      || type === RTValueType.UINT32
      || type === RTValueType.INT64
    ) {
      value = parseInt(value);
    } else if (type === RTValueType.FLOAT
      || type === RTValueType.DOUBLE
    ) {
      value = parseFloat(value);
    } else if (type === RTValueType.BOOLEAN) {
      value = (value || 'false').toLowerCase() === 'true';
    }
    var v = RTValueHelper.create(value, type);
    rtObj.set(pName, v).then(() => {
      doingSet = false;
      updateBtns();
      common.showMessage("primary", "set " + JSON.stringify(v) + " for " + rtObj.id + "." + pName + " succeed");
    }).catch(err => {
      common.showMessage(null, "set " + JSON.stringify(v) + " for " + rtObj.id + "." + pName + " failed, message=" + err);
      doingSet = false;
      updateBtns();
    });
    
  });
  
  $('.btn-action-get').click(function () {
    if ($(this).hasClass('disabled')) {
      return;
    }
    doingGet = true;
    updateBtns();
    rtObj.id = $('#get-obj-name').val();
    var pName = $('#get-prop-name').val();
    rtObj.get(pName).then(rtValue => {
      common.showMessage("primary", "get " + pName + " value = " + JSON.stringify(rtValue));
      doingGet = false;
      updateBtns();
    }).catch(err => {
      common.showMessage(null, "get " + pName + " failed, message=" + err);
      doingGet = false;
      updateBtns();
    });
  });
  
})();
