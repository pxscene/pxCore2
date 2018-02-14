package org.pxscene.rt.remote.messages;


import java.util.ArrayList;
import java.util.List;
import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


/**
 * The call method request entity.
 */
@ToString
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
  @NonNull
  private String methodName;

  /**
   * the call request rt function, it is should be a local or remote function
   */
  @Getter
  @Setter
  private RTValue rtFunction;

  /**
   * the entity constructor with type
   */
  public RTMessageCallMethodRequest() {
    super(RTRemoteMessageType.METHOD_CALL_REQUEST);
  }

}
