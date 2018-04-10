package org.pxscene.rt.remote;

/**
 * the rt const values
 */
public final class RTConst {

  // const values
  public static final String FUNCTION_KEY = "function.name";
  public static final String FUNCTION_GLOBAL_SCOPE = "global";
  public static final String CORRELATION_KEY = "correlation.key";
  public static final String OBJECT_ID_KEY = "object.id";
  public static final String PROPERTY_NAME = "property.name";
  public static final String FUNCTION_ARGS = "function.args";
  public static final String STATUS_MESSAGE = "status.message";
  public static final String MESSAGE_TYPE = "message.type";
  public static final String KEEP_ALIVE_IDS = "keep_alive.ids";
  public static final String FUNCTION_RETURN_VALUE = "function.return_value";
  public static final String SERVER_MODE = "SERVER_MODE";
  public static final String CLIENT_MODE = "CLIENT_MODE";
  public static final String TYPE = "type";
  public static final String SENDER_ID = "sender.id";
  public static final String REPLY_TO = "reply-to";
  public static final String ENDPOINT = "endpoint";
  public static final String STATUS_CODE = "status.code";
  public static final String VALUE = "value";

  /**
   * the first times to find object, then exponential backoff, the unit is ms
   */
  public static final long FIRST_FIND_OBJECT_TIME = 10;

  /**
   * the tcp backlog
   */
  public static final int BACK_LOG = 50;


}
