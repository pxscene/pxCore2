package org.spark.messages;


import org.spark.SparkMessage;
import org.spark.SparkMessageType;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.StringReader;

// {"message.type":"get.byname.request","object.id":"test.lcd",
// "property.name":"connections","correlation.key":"73e4683a-2ba1-4f11-822f-c7bc95989d7c"}
public class SparkMessageGetPropertyByNameRequest extends SparkMessage {
  private String m_propertyName;
  private String m_objectId;

  public SparkMessageGetPropertyByNameRequest() {

    super(SparkMessageType.GET_PROPERTY_BYNAME_REQUEST);
  }

  public void setPropertyName(String s) {
    if (s == null)
      throw new NullPointerException("s");
    m_propertyName = s;
  }

  public void setObjectId(String objectId) {
    if (objectId == null)
      throw new NullPointerException("objectId");
    m_objectId = objectId;
  }

  public static SparkMessageGetPropertyByNameRequest fromString(String s) {
    SparkMessageGetPropertyByNameRequest message = null;

    if (s == null)
      throw new NullPointerException("s");

    JsonReader reader = null;
    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();

      message.setCorrelationKey(obj.getString("correlation.key"));
      message.setObjectId(obj.getString("object.id"));
      message.setPropertyName(obj.getString("property.name"));
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
          .add("property.name", m_propertyName)
          .add("object.id", m_objectId)
          .build();
      m_stringRep = obj.toString();
    }
    return m_stringRep;
  }
}
