package org.pxscene.rt;


import java.util.UUID;
import lombok.Getter;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.remote.RTConst;


/**
 * the local rt function object
 */
@ToString(exclude = {"listener"})
public class RTFunction {

  /**
   * the function name
   */
  @Getter
  @Setter
  private String functionName;

  /**
   * the function object id, default is global
   */
  @Getter
  @Setter
  private String objectId = RTConst.FUNCTION_GLOBAL_SCOPE;

  /**
   * the function invoke listener
   */
  @Getter
  @Setter
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
}
