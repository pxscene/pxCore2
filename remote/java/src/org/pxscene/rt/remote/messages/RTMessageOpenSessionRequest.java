package org.pxscene.rt.remote.messages;

import org.pxscene.rt.RTHelper;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the open session request entity,
 */
public class RTMessageOpenSessionRequest extends RTRemoteMessage {


  /**
   * the remote object id
   */
  private String objectId;

  /**
   * the entity constructor with type.
   */
  public RTMessageOpenSessionRequest() {
    super(RTRemoteMessageType.SESSION_OPEN_REQUEST);
  }

  public String getObjectId() {
    return this.objectId;
  }

  public void setObjectId(String objectId) {
    RTHelper.ensureNotNull(objectId, "objectId");
    this.objectId = objectId;
  }
}
