package org.pxscene.rt.remote.messages;


import java.net.URI;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageLocate extends RTRemoteMessage {
  private String m_objectId;
  private int m_senderId;
  private URI m_rpcEndpoint;

  public RTMessageLocate() {
    super(RTRemoteMessageType.SERACH_OBJECT);
  }

  public void setEndpoint(URI endpoint) {
    m_rpcEndpoint = endpoint;
  }

  public URI getEndpoint() {
    return m_rpcEndpoint;
  }

  public void setObjectId(String objectId) {
    m_objectId = objectId;
  }

  public String getObjectId() {
    return m_objectId;
  }

  public void setSenderId(int id) {
    m_senderId = id;
  }

  public int getSenderId() {
    return m_senderId;
  }
}