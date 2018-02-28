package org.pxscene.rt.remote.messages;

import org.pxscene.rt.RTStatusCode;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property response entity.
 */
public class RTMessageSetPropertyByNameResponse extends RTRemoteMessage {


  /**
   * the status code
   */
  private RTStatusCode statusCode;

  /**
   * the set object id
   */
  private String objectId;


  /**
   * the entity constructor with type
   */
  public RTMessageSetPropertyByNameResponse() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE);
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
