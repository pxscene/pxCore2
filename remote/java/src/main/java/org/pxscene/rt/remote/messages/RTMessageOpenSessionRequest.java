package org.pxscene.rt.remote.messages;

import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the open session request entity,
 */
public class RTMessageOpenSessionRequest extends RTRemoteMessage {


  /**
   * the remote object id
   */
  @Getter
  @Setter
  @NonNull
  private String objectId;

  /**
   * the entity constructor with type.
   */
  public RTMessageOpenSessionRequest() {
    super(RTRemoteMessageType.SESSION_OPEN_REQUEST);
  }
}
