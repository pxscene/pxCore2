package org.pxscene.rt.messages;

import org.pxscene.rt.SparkMessage;
import org.pxscene.rt.SparkMessageType;


public class SparkMessageKeepAliveRequest extends SparkMessage {
  public SparkMessageKeepAliveRequest() {
    super(SparkMessageType.KEEP_ALIVE_REQUEST);
  }
}
