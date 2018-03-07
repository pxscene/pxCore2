package org.pxscene.rt.remote;

public class RTRemoteMessage {
  protected RTRemoteMessageType m_type;
  protected String m_correlationKey;

  protected RTRemoteMessage(RTRemoteMessageType kind) {
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

  public RTRemoteMessageType getMessageType() {
    return m_type;
  }
}

