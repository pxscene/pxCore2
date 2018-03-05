package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageSetPropertyByNameRequest extends RTRemoteMessage {
  private String m_propertyName;
  private String m_objectId;
  private RTValue m_value;

  public RTMessageSetPropertyByNameRequest() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST);
  }

  public void setPropertyName(String s) {
    if (s == null)
      throw new NullPointerException("s");
    m_propertyName = s;
  }

  public String getPropertyName() {
    return m_propertyName;
  }

  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }

  public String getObjectId() {
    return m_objectId;
  }

  public void setValue(RTValue value) {
    m_value = value;
  }

  public RTValue getValue() {
    return m_value;
  }

  public String setObjectId() {
    return m_objectId;
  }
}
