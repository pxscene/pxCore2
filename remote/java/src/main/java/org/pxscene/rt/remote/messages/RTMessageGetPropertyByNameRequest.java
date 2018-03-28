package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageGetPropertyByNameRequest extends RTMessageGetPropertyRequest {

  /**
   * the entity constructor
   */
  public RTMessageGetPropertyByNameRequest() {
    super(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST);
  }
}
