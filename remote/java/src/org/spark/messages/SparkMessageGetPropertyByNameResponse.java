package org.spark.messages;


import org.spark.*;
import org.spark.net.SparkValueSerializer;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.StringReader;

// "{"message.type":"get.byname.response","correlation.key":"c5f376cc-73f5-4c6a-8c37-dfdf5412b5e9",
// "object.id":"some_name","value":{"type":52,"value":10},"status.code":0}"

public class SparkMessageGetPropertyByNameResponse extends SparkMessage {
  private String m_objectId;
  private SparkValue m_value;
  private SparkStatus m_status;

  public SparkMessageGetPropertyByNameResponse() {
    super(SparkMessageType.GET_PROPERTY_BYNAME_RESPONSE);
  }


  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }

  public void setValue(SparkValue value) {
    m_value = value;
  }

  public void setStatus(SparkStatus status) {
    m_status = status;
  }

  public SparkStatus getStatus() {
    return m_status;
  }

  public SparkValue getValue() {
    return m_value;
  }

  public static SparkMessageGetPropertyByNameResponse fromString(String s) {
    SparkMessageGetPropertyByNameResponse message = null;

    if (s == null)
      throw new NullPointerException("s");

    JsonReader reader = null;

    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();
      message = SparkMessageGetPropertyByNameResponse.fromJson(obj);
    } finally {
      if (reader != null)
        reader.close();
    }

    return message;
  }

  public static SparkMessageGetPropertyByNameResponse fromJson(JsonObject obj) {
    SparkMessageGetPropertyByNameResponse message = new SparkMessageGetPropertyByNameResponse();
    message.setCorrelationKey(obj.getString("correlation.key"));
    message.setObjectId(obj.getString("object.id"));

    // TODO: separate JSON to/from from the actual objects and had reader/writer
    SparkStatus status = new SparkStatus();
    if (obj.containsKey("status.code"))
      status.setCode(SparkStatusCode.fromInt(obj.getInt("status.code")));
    if (obj.containsKey("status.message"))
      status.setMessge(obj.getString("status.message"));

    if (obj.containsKey("value")) {
      JsonObject valueObject = obj.getJsonObject("value");
      message.setValue(SparkValueSerializer.fromJson(valueObject));
    }

    message.setStatus(status);

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
