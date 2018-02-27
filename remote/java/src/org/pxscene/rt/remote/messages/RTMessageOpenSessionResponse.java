package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageOpenSessionResponse extends RTRemoteMessage {
  private String m_objectId;

  public RTMessageOpenSessionResponse() {
    super(RTRemoteMessageType.SESSION_OPEN_RESPIONSE);
  }

  public void setObjectId(String objectId) {
    m_objectId = objectId;
  }
}
