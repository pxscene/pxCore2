package org.spark.messages;

import org.spark.SparkMessage;
import org.spark.SparkMessageType;

public class SparkMessageOpenSessionRequest extends SparkMessage {
  private String m_objectId;

  public SparkMessageOpenSessionRequest() {
    super(SparkMessageType.SESSION_OPEN_REQUEST);
  }

  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }
}
