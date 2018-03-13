package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the keep alive response entity.
 */
public class RTMessageKeepAliveResponse extends RTRemoteMessage {

  /**
   * the entity constructor with type.
   */
  public RTMessageKeepAliveResponse() {
    super(RTRemoteMessageType.KEEP_ALIVE_RESPONSE);
  }
}
