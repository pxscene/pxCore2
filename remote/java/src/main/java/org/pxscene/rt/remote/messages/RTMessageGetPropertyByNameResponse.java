package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * The get property by name response entity.
 */

public class RTMessageGetPropertyByNameResponse extends RTMessageGetPropertyResponse {

  /**
   * the entity constructor with type.
   */
  public RTMessageGetPropertyByNameResponse() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE);
  }
}
