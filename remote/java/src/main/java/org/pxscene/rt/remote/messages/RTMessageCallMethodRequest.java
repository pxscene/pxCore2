package org.pxscene.rt.remote.messages;


import java.util.ArrayList;
import java.util.List;
import org.pxscene.rt.RTHelper;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


/**
 * The call method request entity.
 */
public class RTMessageCallMethodRequest extends RTRemoteMessage {

  /**
   * the call request call args
   */
  private List<RTValue> functionArgs = new ArrayList<>();

  /**
   * the method object id.
   */
  private String objectId;


  /**
   * the call request method name
   */
  private String methodName;

  /**
   * the call request rt function, it is should be a local or remote function
   */
  private RTValue rtFunction;

  /**
   * the entity constructor with type
   */
  public RTMessageCallMethodRequest() {
    super(RTRemoteMessageType.METHOD_CALL_REQUEST);
  }


  public List<RTValue> getFunctionArgs() {
    return this.functionArgs;
  }

  public void setFunctionArgs(List<RTValue> functionArgs) {
    this.functionArgs = functionArgs;
  }

  public String getObjectId() {
    return this.objectId;
  }

  public void setObjectId(String objectId) {
    RTHelper.ensureNotNull(objectId, "objectId");
    this.objectId = objectId;
  }

  public String getMethodName() {
    return this.methodName;
  }

  public void setMethodName(String methodName) {
    RTHelper.ensureNotNull(methodName, "methodName");
    this.methodName = methodName;
  }

  public RTValue getRtFunction() {
    return this.rtFunction;
  }

  public void setRtFunction(RTValue rtFunction) {
    this.rtFunction = rtFunction;
  }
}
