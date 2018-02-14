package org.pxscene.rt.remote.messages;


import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * The call method response entity.
 */
@ToString(callSuper = true)
public class RTMessageCallMethodResponse extends RTRemoteMessage {


  /**
   * the call method response value
   */
  @Getter
  @Setter
  @NonNull
  RTValue value;

  /**
   * the call method response status
   */
  @Getter
  @Setter
  @NonNull
  RTStatus status;


  /**
   * the entity constructor with type
   */
  public RTMessageCallMethodResponse() {
    super(RTRemoteMessageType.METHOD_CALL_RESPONSE);
  }


}
