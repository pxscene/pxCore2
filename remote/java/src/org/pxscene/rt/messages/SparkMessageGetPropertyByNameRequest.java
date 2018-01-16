package org.pxscene.rt.messages;

import org.pxscene.rt.SparkMessage;
import org.pxscene.rt.SparkMessageType;

public class SparkMessageGetPropertyByNameRequest extends SparkMessage {
  private String m_propertyName;
  private String m_objectId;

  public SparkMessageGetPropertyByNameRequest() {
    super(SparkMessageType.GET_PROPERTY_BYNAME_REQUEST);
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
