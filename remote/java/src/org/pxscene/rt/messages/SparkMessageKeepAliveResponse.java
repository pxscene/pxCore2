package org.pxscene.rt.messages;


import org.pxscene.rt.SparkMessage;
import org.pxscene.rt.SparkMessageType;

public class SparkMessageKeepAliveResponse extends SparkMessage {
  public SparkMessageKeepAliveResponse() {
    super(SparkMessageType.KEEP_ALIVE_RESPONSE);
  }
}
