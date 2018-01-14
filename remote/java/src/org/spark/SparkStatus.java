package org.spark;

public class SparkStatus {
  private SparkStatusCode m_code;
  private String m_message;

  public SparkStatus() {
  }

  public SparkStatus(SparkStatusCode code) {
    m_code = code;
    m_message = null;
  }

  public SparkStatus(SparkStatusCode code, String message) {
    m_code = code;
    m_message = message;
  }

  public void setCode(SparkStatusCode code) {
    m_code = code;
  }

  public void setMessge(String message) {
    m_message = message;
  }

  public SparkStatusCode getCode() {
    return m_code;
  }

  public boolean isOk() {
    return m_code == SparkStatusCode.OK;
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
