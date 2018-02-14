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
 * The get property by name response entity.
 */

@ToString
public class RTMessageGetPropertyByNameResponse extends RTRemoteMessage {

  /**
   * the remote object id
   */
  @Getter
  @Setter
  @NonNull
  private String objectId;


  /**
   * the response value
   */
  @Getter
  @Setter
  @NonNull
  private RTValue value;

  /**
   * the response status
   */
  @Getter
  @Setter
  @NonNull
  private RTStatus status;

  /**
   * the entity constructor with type.
   */
  public RTMessageGetPropertyByNameResponse() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE);
  }
}
