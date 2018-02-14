package org.pxscene.rt.remote.messages;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.RTStatusCode;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property response entity.
 */
@ToString(callSuper = true)
public class RTMessageSetPropertyByNameResponse extends RTRemoteMessage {


  /**
   * the status code
   */
  @Getter
  @Setter
  private RTStatusCode statusCode;

  /**
   * the set object id
   */
  @Getter
  @Setter
  private String objectId;


  /**
   * the entity constructor with type
   */
  public RTMessageSetPropertyByNameResponse() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE);
  }
}
