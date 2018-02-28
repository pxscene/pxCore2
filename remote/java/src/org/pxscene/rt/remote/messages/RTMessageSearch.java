package org.pxscene.rt.remote.messages;


import java.net.URI;
import org.pxscene.rt.RTHelper;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


/**
 * the object serach entity.
 */
public class RTMessageSearch extends RTRemoteMessage {

  /**
   * the remote object id
   */
  private String objectId;

  /**
   * the sender id
   */
  private int senderId;

  /**
   * the local host udp address, used to recv remote message
   */
  private URI replyTo;

  /**
   * the entity constructor with type.
   */
  public RTMessageSearch() {
    super(RTRemoteMessageType.SERACH_OBJECT);
  }


  public String getObjectId() {
    return this.objectId;
  }

  public void setObjectId(String objectId) {
    RTHelper.ensureNotNull(objectId, "objectId");
    this.objectId = objectId;
  }

  public int getSenderId() {
    return this.senderId;
  }

  public void setSenderId(int senderId) {
    this.senderId = senderId;
  }

  public URI getReplyTo() {
    return this.replyTo;
  }

  public void setReplyTo(URI replyTo) {
    RTHelper.ensureNotNull(replyTo, "replyTo");
    this.replyTo = replyTo;
  }
}