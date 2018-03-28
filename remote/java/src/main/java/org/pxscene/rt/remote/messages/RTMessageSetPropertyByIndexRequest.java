package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the set property by index request entity.
 */

public class RTMessageSetPropertyByIndexRequest extends RTMessageSetPropertyRequest {


  /**
   * the array index for property
   */
  private Integer index;

  /**
   * the entity constructor with type.
   */
  public RTMessageSetPropertyByIndexRequest() {
    super(RTRemoteMessageType.SET_PROPERTY_BYINDEX_REQUEST);
  }

  public Integer getIndex() {
    return index;
  }

  public void setIndex(Integer index) {
    this.index = index;
  }
}
