package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageGetPropertyByNameRequest extends RTRemoteMessage {
  private String m_propertyName;
  private String m_objectId;

  public RTMessageGetPropertyByNameRequest() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST);
  }

  public String getPropertyName() {
    return m_propertyName;
  }

  public void setPropertyName(String s) {
    if (s == null)
      throw new NullPointerException("s");
    m_propertyName = s;
  }

  public String getObjectId() {
    return m_objectId;
  }

  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }
}
