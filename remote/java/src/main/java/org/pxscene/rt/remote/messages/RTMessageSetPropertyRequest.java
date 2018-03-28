package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTHelper;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property by name request entity.
 */

public class RTMessageSetPropertyRequest extends RTRemoteMessage {

  /**
   * the remote object property name
   */
  protected String propertyName;

  /**
   * the remote object id
   */
  protected String objectId;

  /**
   * the set value
   */
  protected RTValue value;

  /**
   * the entity constructor with type.
   */
  public RTMessageSetPropertyRequest(RTRemoteMessageType type) {
    super(type);
  }


  public String getPropertyName() {
    return this.propertyName;
  }

  public void setPropertyName(String propertyName) {
    RTHelper.ensureNotNull(propertyName, "propertyName");
    this.propertyName = propertyName;
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
    this.value = value;
  }
}
