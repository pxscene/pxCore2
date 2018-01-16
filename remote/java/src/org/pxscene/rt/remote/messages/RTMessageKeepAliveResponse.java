package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageKeepAliveResponse extends RTRemoteMessage {
  public RTMessageKeepAliveResponse() {
    super(RTRemoteMessageType.KEEP_ALIVE_RESPONSE);
  }
}
