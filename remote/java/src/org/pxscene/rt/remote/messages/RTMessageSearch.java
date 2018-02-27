package org.pxscene.rt.remote.messages;


import java.net.URI;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageSearch extends RTRemoteMessage {
  private String m_objectId;
  private int m_senderId;
  private URI m_replyTo;

  public RTMessageSearch() {
    super(RTRemoteMessageType.SERACH_OBJECT);
  }

  public void setReplyTo(URI uri) {
    m_replyTo = uri;
  }

  public URI getReplyTo() {
    return m_replyTo;
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