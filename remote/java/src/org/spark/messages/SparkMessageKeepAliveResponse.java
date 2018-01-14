package org.spark.messages;


import org.spark.SparkMessage;
import org.spark.SparkMessageType;

public class SparkMessageKeepAliveResponse extends SparkMessage {
  public SparkMessageKeepAliveResponse() {
    super(SparkMessageType.KEEP_ALIVE_RESPONSE);
  }
}
