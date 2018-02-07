package org.pxscene.rt;

import java.util.List;

/**
 * the local rt function listener
 */
public interface RTFunctionListener {

  /**
   * invoke rtFunction
   *
   * @param rtValueList the function args
   */
  void invoke(List<RTValue> rtValueList);
}
