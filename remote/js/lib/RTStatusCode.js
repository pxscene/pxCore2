/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt status code
 *
 * @author      TCSCODER
 * @version     1.0
 */

module.exports = {
  UNKNOWN: -1,
  OK: 0,
  ERROR: 1,
  FAIL: 1,
  NOT_ENOUGH_ARGUMENTS: 2,
  INVALID_ARGUMENT: 3,
  PROP_NOT_FOUND: 4,
  OBJECT_NOT_INITIALIZED: 5,
  PROPERTY_NOT_FOUND: 6, // dup of 4
  OBJECT_NO_LONGER_AVAILABLE: 7,
  RESOURCE_NOT_FOUND: 8,
  NO_CONNECTION: 9,
  NOT_IMPLEMENTED: 10,
  TYPE_MISMATCH: 11,
  NOT_ALLOWED: 12,
  TIMEOUT: 1000,
  DUPLICATE_ENTRY: 1001,
  OBJECT_NOT_FOUND: 1002,
  PROTOCOL_ERROR: 1003,
  INVALID_OPERATION: 1004,
  IN_PROGRESS: 1005,
  QUEUE_EMPTY: 1006,
  STREAM_CLOSED: 1007,
};
