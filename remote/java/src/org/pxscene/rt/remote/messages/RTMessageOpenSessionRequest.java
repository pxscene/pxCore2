package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageOpenSessionRequest extends RTRemoteMessage {
  private String m_objectId;

  public RTMessageOpenSessionRequest() {
    super(RTRemoteMessageType.SESSION_OPEN_REQUEST);
  }

  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }
}
