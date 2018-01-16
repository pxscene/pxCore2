package org.pxscene.rt.messages;

import org.pxscene.rt.SparkMessage;
import org.pxscene.rt.SparkMessageType;

public class SparkMessageOpenSessionResponse extends SparkMessage {
  private String m_objectId;

  public SparkMessageOpenSessionResponse() {
    super(SparkMessageType.SESSION_OPEN_RESPIONSE);
  }

  public void setObjectId(String objectId) {
    m_objectId = objectId;
  }
}
