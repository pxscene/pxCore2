package org.pxscene.rt.remote.messages;

import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property index entity.
 */
public class RTMessageSetPropertyByIndexResponse extends RTMessageSetPropertyResponse {

  /**
   * the entity constructor with type
   */
  public RTMessageSetPropertyByIndexResponse() {
    super(RTRemoteMessageType.SET_PROPERTY_BYINDEX_RESPONSE);
  }
}
