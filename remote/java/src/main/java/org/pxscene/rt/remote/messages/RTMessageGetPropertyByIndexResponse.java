package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * The get property by index response entity.
 */

public class RTMessageGetPropertyByIndexResponse extends RTMessageGetPropertyResponse {

  /**
   * the entity constructor with type.
   */
  public RTMessageGetPropertyByIndexResponse() {
    super(RTRemoteMessageType.GET_PROPERTY_BYINDEX_RESPONSE);
  }
}
