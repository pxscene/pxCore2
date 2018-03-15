package org.pxscene.rt.remote.messages;


import java.net.URI;
import org.pxscene.rt.RTHelper;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the Locate (search done) response entity.
 */
public class RTMessageLocate extends RTRemoteMessage {


  /**
   * the remote object id
   */
  private String objectId;

  /**
   * the remote sender id
   */
  private int senderId;

  /**
   * the rpc endpoint uri
   */
  private URI endpoint;

  /**
   * the entity constructor with type.
   */
  public RTMessageLocate() {
    super(RTRemoteMessageType.LOCATE_OBJECT);
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

  public URI getEndpoint() {
    return this.endpoint;
  }

  public void setEndpoint(URI endpoint) {
    RTHelper.ensureNotNull(endpoint, "endpoint");
    this.endpoint = endpoint;
  }
}