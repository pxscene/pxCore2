package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property response entity.
 */
public class RTMessageSetPropertyByNameResponse extends RTRemoteMessage {


  /**
   * the entity constructor with type
   */
  public RTMessageSetPropertyByNameResponse() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE);
  }
}
