package org.pxscene.rt.remote;

/**
 * the rt remote message type.
 */
public enum RTRemoteMessageType {
  SESSION_OPEN_REQUEST("session.open.request"),
  SESSION_OPEN_RESPIONSE("session.open.response"),
  GET_PROPERTY_BYNAME_REQUEST("get.byname.request"),
  GET_PROPERTY_BYNAME_RESPONSE("get.byname.response"),
  SET_PROPERTY_BYNAME_REQUEST("set.byname.request"),
  SET_PROPERTY_BYNAME_RESPONSE("set.byname.response"),
  GET_PROPERTY_BYINDEX_REQUEST("get.byindex.request"),
  GET_PROPERTY_BYINDEX_RESPONSE("get.byindex.response"),
  SET_PROPERTY_BYINDEX_REQUEST("set.byindex.request"),
  SET_PROPERTY_BYINDEX_RESPONSE("set.byindex.response"),
  KEEP_ALIVE_REQUEST("keep_alive.request"),
  KEEP_ALIVE_RESPONSE("keep_alive.response"),
  METHOD_CALL_RESPONSE("method.call.response"),
  METHOD_CALL_REQUEST("method.call.request"),
  SERACH_OBJECT("search"),
  LOCATE_OBJECT("locate");


  /**
   * the string type
   */
  private final String stringRep;

  /**
   * the entity constructor with type string
   */
  RTRemoteMessageType(String s) {
    stringRep = s;
  }

  /**
   * convert RTRemoteMessageType to string value
   *
   * @return the string value
   */
  public String toString() {
    return stringRep;
  }
}
