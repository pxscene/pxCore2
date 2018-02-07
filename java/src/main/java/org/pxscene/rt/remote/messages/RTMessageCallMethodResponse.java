package org.pxscene.rt.remote.messages;


import lombok.Getter;
import lombok.Setter;
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
  @Getter
  @Setter
  RTValue value;

  /**
   * the call method response status
   */
  @Getter
  @Setter
  RTStatus status;


  /**
   * the entity constructor with type
   */
  public RTMessageCallMethodResponse() {
    super(RTRemoteMessageType.METHOD_CALL_RESPONSE);
  }


}
