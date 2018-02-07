package org.pxscene.rt.remote.messages;


import java.util.ArrayList;
import java.util.List;
import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;
import org.pxscene.rt.RTFunction;

/**
 * The call method request entity.
 */
public class RTMessageCallMethodRequest extends RTRemoteMessage {

  /**
   * the call request call args
   */
  @Getter
  @Setter
  private List<RTValue> functionArgs = new ArrayList<>();

  /**
   * the method object id.
   */
  @Getter
  @Setter
  @NonNull
  private String objectId;


  /**
   * the call request method name
   */
  @Getter
  @Setter
  private String methodName;

  /**
   * the call request rt function, it may reflex to a local rt function listener
   */
  @Getter
  @Setter
  private RTFunction rtFunction;

  /**
   * the entity constructor with type
   */
  public RTMessageCallMethodRequest() {
    super(RTRemoteMessageType.METHOD_CALL_REQUEST);
  }

}
