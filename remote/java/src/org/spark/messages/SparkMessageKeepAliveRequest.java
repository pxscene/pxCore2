package org.spark.messages;

import org.spark.SparkMessage;
import org.spark.SparkMessageType;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.StringReader;

// {"message.type":"keep_alive.request","correlation.key":"52dea93c-5aac-4124-9937-a2fc3c50f9f9"}
public class SparkMessageKeepAliveRequest extends SparkMessage {
  public SparkMessageKeepAliveRequest() {
    super(SparkMessageType.KEEP_ALIVE_REQUEST);
  }

  public static SparkMessageKeepAliveRequest fromString(String s) {
    SparkMessageKeepAliveRequest message = null;

    if (s == null)
      throw new NullPointerException("s");

    JsonReader reader = null;
    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();

      message.setCorrelationKey(obj.getString("correlation.key"));
    } finally {
      if (reader != null)
        reader.close();
    }

    return message;
  }

  public String toString() {
    if (m_stringRep == null) {
      JsonObject obj = Json.createObjectBuilder()
          .add("message.type", m_type.toString())
          .add("correlation.key", m_correlationKey)
          .build();
      m_stringRep = obj.toString();
    }
    return m_stringRep;
  }
}
