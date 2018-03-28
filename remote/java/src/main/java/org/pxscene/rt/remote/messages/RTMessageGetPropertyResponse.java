package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTHelper;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * The get property by name response entity.
 */

public class RTMessageGetPropertyResponse extends RTRemoteMessage {

  /**
   * the remote object id
   */
  protected String objectId;


  /**
   * the response value
   */
  protected RTValue value;

  /**
   * the response status
   */
  protected RTStatus status;

  /**
   * the entity constructor with type.
   */
  public RTMessageGetPropertyResponse(RTRemoteMessageType type) {
    super(type);
  }


  public String getObjectId() {
    return this.objectId;
  }

  public void setObjectId(String objectId) {
    RTHelper.ensureNotNull(objectId, "objectId");
    this.objectId = objectId;
  }

  public RTValue getValue() {
    return this.value;
  }

  public void setValue(RTValue value) {
    RTHelper.ensureNotNull(value, "value");
    this.value = value;
  }

  public RTStatus getStatus() {
    return this.status;
  }

  public void setStatus(RTStatus status) {
    RTHelper.ensureNotNull(status, "status");
    this.status = status;
  }
}
