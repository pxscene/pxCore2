package org.pxscene.rt.remote.messages;


import org.pxscene.rt.RTHelper;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageGetPropertyByNameRequest extends RTRemoteMessage {

  /**
   * the set property name
   */

  private String propertyName;


  /**
   * the remote object id
   */
  private String objectId;

  /**
   * the entity constructor with type
   */
  public RTMessageGetPropertyByNameRequest() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST);
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
}
