package org.spark.messages;


import org.spark.SparkMessage;
import org.spark.SparkMessageType;
import org.spark.SparkStatus;
import org.spark.SparkValue;


public class SparkMessageGetPropertyByNameResponse extends SparkMessage {
  private String m_objectId;
  private SparkValue m_value;
  private SparkStatus m_status;

  public SparkMessageGetPropertyByNameResponse() {
    super(SparkMessageType.GET_PROPERTY_BYNAME_RESPONSE);
  }


  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }

  public void setValue(SparkValue value) {
    m_value = value;
  }

  public void setStatus(SparkStatus status) {
    m_status = status;
  }

  public SparkStatus getStatus() {
    return m_status;
  }

  public String getObjectId() {
    return m_objectId;
  }

  public SparkValue getValue() {
    return m_value;
  }
}
