package org.pxscene.rt.remote.messages;


import org.pxscene.rt.remote.RTRemoteMessageType;

public class RTMessageGetPropertyByIndexRequest extends RTMessageGetPropertyRequest {
  /**
   * the property index
   */
  private Integer index;

  /**
   * the entity constructor with type
   */
  public RTMessageGetPropertyByIndexRequest() {
    super(RTRemoteMessageType.GET_PROPERTY_BYINDEX_REQUEST);
  }


  public Integer getIndex() {
    return index;
  }

  public void setIndex(Integer index) {
    this.index = index;
  }
}
