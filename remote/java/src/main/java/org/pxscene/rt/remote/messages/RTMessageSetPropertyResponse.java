package org.pxscene.rt.remote.messages;

import org.pxscene.rt.RTStatusCode;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property response entity.
 */
public class RTMessageSetPropertyResponse extends RTRemoteMessage {


  /**
   * the status code
   */
  protected RTStatusCode statusCode;

  /**
   * the set object id
   */
  protected String objectId;


  /**
   * the entity constructor with type
   */
  public RTMessageSetPropertyResponse(RTRemoteMessageType type) {
    super(type);
  }

  public RTStatusCode getStatusCode() {
    return this.statusCode;
  }

  public void setStatusCode(RTStatusCode statusCode) {
    this.statusCode = statusCode;
  }

  public String getObjectId() {
    return this.objectId;
  }

  public void setObjectId(String objectId) {
    this.objectId = objectId;
  }
}
