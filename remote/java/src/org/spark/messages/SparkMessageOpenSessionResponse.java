package org.spark.messages;

import org.spark.SparkMessage;
import org.spark.SparkMessageType;

public class SparkMessageOpenSessionResponse extends SparkMessage {
  private String m_objectId;

  public SparkMessageOpenSessionResponse() {
    super(SparkMessageType.SESSION_OPEN_RESPIONSE);
  }

  public void setObjectId(String objectId) {
    m_objectId = objectId;
  }
}
