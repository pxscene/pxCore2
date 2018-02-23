package org.pxscene.rt;

public class RTStatus {
  private RTStatusCode m_code;
  private String m_message;

  public RTStatus() {
  }

  public RTStatus(RTStatusCode code) {
    m_code = code;
    m_message = null;
  }

  public RTStatus(RTStatusCode code, String message) {
    m_code = code;
    m_message = message;
  }

  public void setCode(RTStatusCode code) {
    m_code = code;
  }

  public void setMessge(String message) {
    m_message = message;
  }

  public RTStatusCode getCode() {
    return m_code;
  }

  public boolean isOk() {
    return m_code == RTStatusCode.OK;
  }

  public String getMessage() {
    return m_message;
  }

  public String toString() {
    StringBuilder builder = new StringBuilder();
    if (m_code != null)
      builder.append(m_code.toString());
    if (m_message != null) {
      if (m_code != null)
        builder.append(':');
      builder.append(m_message);
    }
    return builder.toString();
  }
}
