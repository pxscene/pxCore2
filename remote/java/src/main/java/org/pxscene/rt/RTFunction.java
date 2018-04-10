package org.pxscene.rt;


import java.util.UUID;
import org.pxscene.rt.remote.RTConst;


/**
 * the local rt function object
 */
public class RTFunction {

  /**
   * the function name
   */
  private String functionName;

  /**
   * the function object id, default is global
   */
  private String objectId = RTConst.FUNCTION_GLOBAL_SCOPE;

  /**
   * the function invoke listener
   */
  private RTFunctionListener listener;

  /**
   * create a new rtFunction
   */
  public RTFunction() {
    this(null);
  }

  /**
   * create a new rtFunction with listener
   *
   * @param listener the function listener
   */
  public RTFunction(RTFunctionListener listener) {
    this.listener = listener;
    this.functionName = "func://" + UUID.randomUUID().toString();
  }


  public String getFunctionName() {
    return this.functionName;
  }

  public void setFunctionName(String functionName) {
    this.functionName = functionName;
  }

  public String getObjectId() {
    return this.objectId;
  }

  public void setObjectId(String objectId) {
    this.objectId = objectId;
  }

  public RTFunctionListener getListener() {
    return this.listener;
  }

  public void setListener(RTFunctionListener listener) {
    this.listener = listener;
  }
}
