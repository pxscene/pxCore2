package org.pxscene.rt.remote.messages;

import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageGetPropertyByNameRequest extends RTRemoteMessage {

  /**
   * the set property name
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
   * the entity constructor with type
   */
  public RTMessageGetPropertyByNameRequest() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST);
  }

}
