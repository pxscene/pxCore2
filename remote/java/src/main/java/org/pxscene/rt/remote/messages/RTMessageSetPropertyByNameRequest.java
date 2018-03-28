package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property by name request entity.
 */

public class RTMessageSetPropertyByNameRequest extends RTMessageSetPropertyRequest {

  /**
   * the entity constructor with type.
   */
  public RTMessageSetPropertyByNameRequest() {
    super(RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST);
  }
}
