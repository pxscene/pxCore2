package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTHelper;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * The get property by name response entity.
 */

public class RTMessageGetPropertyByNameResponse extends RTRemoteMessage {

  /**
   * the remote object id
   */
  private String objectId;


  /**
   * the response value
   */
  private RTValue value;

  /**
   * the response status
   */
  private RTStatus status;

  /**
   * the entity constructor with type.
   */
  public RTMessageGetPropertyByNameResponse() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE);
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
