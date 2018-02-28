package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTHelper;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * The call method response entity.
 */
public class RTMessageCallMethodResponse extends RTRemoteMessage {


  /**
   * the call method response value
   */
  RTValue value;

  /**
   * the call method response status
   */
  RTStatus status;


  /**
   * the entity constructor with type
   */
  public RTMessageCallMethodResponse() {
    super(RTRemoteMessageType.METHOD_CALL_RESPONSE);
  }


  public RTValue getValue() {
    return this.value;
  }

  public void setValue(RTValue value) {
    RTHelper.ensureNotNull(value, "value");
    this.value = value;
  }

  public RTStatus getStatus() {
    return this.status;
  }

  public void setStatus(RTStatus status) {
    RTHelper.ensureNotNull(status, "status");
    this.status = status;
  }
}
