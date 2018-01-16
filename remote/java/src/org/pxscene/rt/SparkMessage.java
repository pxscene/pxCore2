package org.pxscene.rt;

public class SparkMessage {
  protected SparkMessageType m_type;
  protected String m_correlationKey;

  protected SparkMessage(SparkMessageType kind) {
    m_type = kind;
  }

  public String getCorrelationKey() {
    return m_correlationKey;
  }

  public void setCorrelationKey(String key) {
    if (key == null)
      throw new NullPointerException("key");
    m_correlationKey = key;
  }

  public SparkMessageType getMessageType() {
    return m_type;
  }
}

