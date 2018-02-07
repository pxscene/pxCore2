package org.pxscene.rt.remote.messages;


import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property by name request entity.
 */
public class RTMessageSetPropertyByNameRequest extends RTRemoteMessage {

  /**
   * the remote object property name
   */
  @Getter
  @Setter
  @NonNull
  private String propertyName;

  /**
   * the remote object id
   */
  @Getter
  @Setter
  @NonNull
  private String objectId;

  /**
   * the set value
   */
  @Setter
  @Getter
  private RTValue value;

  /**
   * the entity constructor with type.
   */
  public RTMessageSetPropertyByNameRequest() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST);
  }

}
