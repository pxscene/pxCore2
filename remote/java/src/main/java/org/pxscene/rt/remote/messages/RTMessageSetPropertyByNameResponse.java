package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property by name response entity.
 */
public class RTMessageSetPropertyByNameResponse extends RTMessageSetPropertyResponse {


  /**
   * the entity constructor with type
   */
  public RTMessageSetPropertyByNameResponse() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE);
  }
}
