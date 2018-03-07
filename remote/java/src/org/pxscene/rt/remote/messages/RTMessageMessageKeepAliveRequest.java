package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


public class RTMessageMessageKeepAliveRequest extends RTRemoteMessage {
  public RTMessageMessageKeepAliveRequest() {
    super(RTRemoteMessageType.KEEP_ALIVE_REQUEST);
  }
}
