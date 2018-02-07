package org.pxscene.rt.remote;

import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;

/**
 * the base RTRemote Message class.
 */
public class RTRemoteMessage {


  /**
   * the rt remote message type
   */
  @Getter
  protected RTRemoteMessageType messageType;

  /**
   * the correlation key
   */
  @Getter
  @Setter
  @NonNull
  protected String correlationKey;

  /**
   * the entity constructor with type
   */
  protected RTRemoteMessage(RTRemoteMessageType kind) {
    messageType = kind;
  }
}

