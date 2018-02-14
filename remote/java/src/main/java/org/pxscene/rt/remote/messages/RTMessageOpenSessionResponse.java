package org.pxscene.rt.remote.messages;

import lombok.NonNull;
import lombok.Getter;
import lombok.Setter;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the open session response.
 */
public class RTMessageOpenSessionResponse extends RTRemoteMessage {

  /**
   * the remote object id.
   */
  @Setter
  @Getter
  @NonNull
  private String objectId;

  /**
   * the entity constructor with type.
   */
  public RTMessageOpenSessionResponse() {
    super(RTRemoteMessageType.SESSION_OPEN_RESPIONSE);
  }

}
