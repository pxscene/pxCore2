package org.pxscene.rt.remote.messages;

import javax.json.JsonArray;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


/**
 * keep aliva request entity.
 */
public class RTMessageKeepAliveRequest extends RTRemoteMessage {

  /**
   * the keep alive id array.
   */
  private JsonArray keepAliveIds;


  /**
   * the entity constructor with type.
   */
  public RTMessageKeepAliveRequest() {
    super(RTRemoteMessageType.KEEP_ALIVE_REQUEST);
  }

  public JsonArray getKeepAliveIds() {
    return this.keepAliveIds;
  }

  public void setKeepAliveIds(JsonArray keepAliveIds) {
    this.keepAliveIds = keepAliveIds;
  }
}
