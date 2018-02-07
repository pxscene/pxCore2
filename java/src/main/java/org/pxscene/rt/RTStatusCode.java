package org.pxscene.rt;

import lombok.Getter;

/**
 * the rt status code
 */
public enum RTStatusCode {
  UNKNOWN(-1),
  OK(0),
  ERROR(1),
  FAIL(1),
  NOT_ENOUGH_ARGUMENTS(2),
  INVALID_ARGUMENT(3),
  PROP_NOT_FOUND(4),
  OBJECT_NOT_INITIALIZED(5),
  PROPERTY_NOT_FOUND(6), // dup of 4
  OBJECT_NO_LONGER_AVAILABLE(7),
  RESOURCE_NOT_FOUND(8),
  NO_CONNECTION(9),
  NOT_IMPLEMENTED(10),
  TYPE_MISMATCH(11),
  NOT_ALLOWED(12),
  TIMEOUT(1000),
  DUPLICATE_ENTRY(1001),
  OBJECT_NOT_FOUND(1002),
  PROTOCOL_ERROR(1003),
  INVALID_OPERATION(1004),
  IN_PROGRESS(1005),
  QUEUE_EMPTY(1006),
  STREAM_CLOSED(1007);

  /**
   * the status code
   */
  @Getter
  private int code;

  /**
   * create new RTStatusCode with code
   *
   * @param code the status code
   */
  RTStatusCode(int code) {
    this.code = code;
  }

  /**
   * convert int to RTStatusCode
   *
   * @param n the code value
   * @return the RTStatusCode entity
   */
  public static RTStatusCode fromInt(int n) {
    for (RTStatusCode code : RTStatusCode.values()) {
      if (code.getCode() == n) {
        return code;
      }
    }
    return RTStatusCode.UNKNOWN;
  }
}
