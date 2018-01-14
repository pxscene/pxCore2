package org.spark.messages;

import org.spark.SparkMessage;
import org.spark.SparkMessageType;


public class SparkMessageKeepAliveRequest extends SparkMessage {
  public SparkMessageKeepAliveRequest() {
    super(SparkMessageType.KEEP_ALIVE_REQUEST);
  }
}
