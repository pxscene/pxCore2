package org.pxscene.rt;

import lombok.NonNull;
import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

/**
 * the RTStatus class.
 */
@ToString
public class RTStatus {

  /**
   * the status code
   */
  @Getter
  @Setter
  @NonNull
  private RTStatusCode code;

  /**
   * the status message
   */
  @Setter
  @Getter
  private String message;

  /**
   * create new rt status
   */
  public RTStatus() {
  }

  /**
   * create new rt status with code
   *
   * @param code the stastus code
   */
  public RTStatus(RTStatusCode code) {
    this.code = code;
    message = null;
  }

  /**
   * create new rt status with code and message
   *
   * @param code the status code
   * @param message the status message
   */
  public RTStatus(RTStatusCode code, String message) {
    this.code = code;
    this.message = message;
  }


  /**
   * check the status code is ok
   *
   * @return the result
   */
  public boolean isOk() {
    return code == RTStatusCode.OK;
  }


  /**
   * convert rt status to json string
   *
   * @return the json string
   */
  public String toString() {
    StringBuilder builder = new StringBuilder();
    if (code != null) {
      builder.append(code.toString());
    }
    if (message != null) {
      if (code != null) {
        builder.append(':');
      }
      builder.append(message);
    }
    return builder.toString();
  }
}
