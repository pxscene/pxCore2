package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


public class RTMessageGetPropertyByNameResponse extends RTRemoteMessage {
  private String m_objectId;
  private RTValue m_value;
  private RTStatus m_status;

  public RTMessageGetPropertyByNameResponse() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE);
  }


  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }

  public void setValue(RTValue value) {
    m_value = value;
  }

  public void setStatus(RTStatus status) {
    m_status = status;
  }

  public RTStatus getStatus() {
    return m_status;
  }

  public String getObjectId() {
    return m_objectId;
  }

  public RTValue getValue() {
    return m_value;
  }
}
