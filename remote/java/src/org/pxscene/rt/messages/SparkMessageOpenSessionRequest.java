package org.pxscene.rt.messages;

import org.pxscene.rt.SparkMessage;
import org.pxscene.rt.SparkMessageType;

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
