package org.spark.messages;

import org.spark.SparkMessage;
import org.spark.SparkMessageType;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.StringReader;

// {"message.type":"session.open.request","correlation.key":"dcb73864-b7df-49b5-8c41-66335bf94a34","object.id":"test.lcd"}
class SparkMessageOpenSessionRequest extends SparkMessage {
  private String m_objectId;

  public SparkMessageOpenSessionRequest() {
    super(SparkMessageType.SESSION_OPEN_REQUEST);
  }

  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }

  public static SparkMessageOpenSessionRequest fromString(String s) {
    SparkMessageOpenSessionRequest message = null;

    if (s == null)
      throw new NullPointerException("s");

    JsonReader reader = null;
    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();

      message.setObjectId(obj.getString("object.id"));
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
          .add("object.id", m_objectId)
          .build();
      m_stringRep = obj.toString();
    }
    return m_stringRep;
  }
}
