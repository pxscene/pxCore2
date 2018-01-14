package org.spark.messages;


import org.spark.SparkMessage;
import org.spark.SparkMessageType;

public class SparkMessageSetPropertyByNameRequest extends SparkMessage {
  private String m_propertyName;
  private String m_objectId;

  public SparkMessageSetPropertyByNameRequest() {
    super(SparkMessageType.SET_PROPERTY_BYNAME_REQUEST);
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

  public String setObjectId() {
    return m_objectId;
  }
}
