package org.spark.net;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;

import java.io.StringReader;
import java.util.Map;
import java.util.HashMap;
import java.util.function.Function;

import org.spark.*;
import org.spark.messages.SparkMessageGetPropertyByNameResponse;
import org.spark.messages.SparkMessageGetPropertyByNameRequest;
import org.spark.messages.SparkMessageSetPropertyByNameRequest;
import org.spark.messages.SparkMessageSetPropertyByNameResponse;
import org.spark.messages.SparkMessageKeepAliveRequest;
import org.spark.messages.SparkMessageKeepAliveResponse;
import org.spark.messages.SparkMessageOpenSessionRequest;
import org.spark.messages.SparkMessageOpenSessionResponse;


public class SparkMessageSerializer {
  private static final Map<String, Function<JsonObject, SparkMessage>> m_decoders;

  static {
    m_decoders = new HashMap<>();
    m_decoders.put(SparkMessageType.GET_PROPERTY_BYNAME_REQUEST.toString(), (JsonObject json) -> {
      return SparkMessageSerializer.fromJson_GetPropertyByNameRequest(json);
    });
    m_decoders.put(SparkMessageType.GET_PROPERTY_BYNAME_RESPONSE.toString(), (JsonObject json) -> {
      return SparkMessageSerializer.fromJson_GetPropertyByNameResponse(json);
    });
    m_decoders.put(SparkMessageType.SET_PROPERTY_BYNAME_REQUEST.toString(), (JsonObject json) -> {
      return SparkMessageSerializer.fromJson_SetPropertyByNameRequest(json);
    });
    m_decoders.put(SparkMessageType.SET_PROPERTY_BYNAME_RESPONSE.toString(), (JsonObject json) -> {
      return SparkMessageSerializer.fromJson_SetPropertyByNameResponse(json);
    });
  }

  public static <T extends SparkMessage> T fromJson(JsonObject obj) throws SparkException {
    if (obj == null)
      throw new NullPointerException("obj");

    if (!obj.containsKey("message.type"))
      throw new SparkException("unsupported message type:" + obj.toString());

    Function<JsonObject, SparkMessage> decoder = m_decoders.get(obj.getString("message.type"));
    return (T) decoder.apply(obj);
  }

  public static <T extends SparkMessage> T fromString(String s, Function<JsonObject, T> deserializer)
      throws SparkException {
    T message = null;
    JsonReader reader = null;

    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();
      if (deserializer == null)
        message = SparkMessageSerializer.fromJson(obj);
      else
        message = deserializer.apply(obj);
    } finally {
      if (reader != null)
        reader.close();
    }
    return message;
  }

  public static SparkMessageGetPropertyByNameRequest fromJson_GetPropertyByNameRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");
    SparkMessageGetPropertyByNameRequest req = new SparkMessageGetPropertyByNameRequest();
    req.setObjectId(obj.getString("object.id"));
    req.setCorrelationKey(obj.getString("correlation.key"));
    req.setPropertyName(obj.getString("property.name"));
    return req;
  }

  public static SparkMessageGetPropertyByNameResponse fromJson_GetPropertyByNameResponse(JsonObject obj) {
    SparkMessageGetPropertyByNameResponse res = new SparkMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    res.setObjectId(obj.getString("object.id"));

    SparkStatus status = new SparkStatus();
    if (obj.containsKey("status.code"))
      status.setCode(SparkStatusCode.fromInt(obj.getInt("status.code")));
    if (obj.containsKey("status.message"))
      status.setMessge(obj.getString("status.message"));

    if (obj.containsKey("value")) {
      JsonObject valueObject = obj.getJsonObject("value");
      res.setValue(SparkValueSerializer.fromJson(valueObject));
    }

    res.setStatus(status);
    return res;
  }

  public static SparkMessageKeepAliveRequest fromJson_KeepAliveRequest(JsonObject obj) {
    SparkMessageKeepAliveRequest req = new SparkMessageKeepAliveRequest();
    req.setCorrelationKey(obj.getString("correlation.key"));
    return req;
  }

  public static SparkMessageKeepAliveResponse fromJson_KeepAliveResponse(JsonObject obj) {
    SparkMessageKeepAliveResponse res = new SparkMessageKeepAliveResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    return res;
  }

  public static SparkMessageSetPropertyByNameRequest fromJson_SetPropertyByNameRequest(JsonObject obj) {
    return null;
  }

  public static SparkMessageGetPropertyByNameResponse fromJson_SetPropertyByNameResponse(JsonObject obj) {
    return null;
  }

  // toJson
  public static JsonObject toJson(SparkMessageGetPropertyByNameRequest req) {
    if (req == null)
      throw new NullPointerException("req");
    JsonObject obj = Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .add("property.name", req.getPropertyName())
        .add("object.id", req.getObjectId())
        .build();
    return obj;
  }

  public static JsonObject toJson(SparkMessageGetPropertyByNameResponse res) {
    JsonObject obj = Json.createObjectBuilder()
        .add("message.type", res.getMessageType().toString())
        .add("correlation.key", res.getCorrelationKey())
        .add("object.id", res.getObjectId())
        .build();
    return obj;
  }

  public static JsonObject toJson(SparkMessageKeepAliveRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .build();
  }

  public static JsonObject toJson(SparkMessageKeepAliveResponse res) {
    return Json.createObjectBuilder()
        .add("message.type", res.getMessageType().toString())
        .add("correlation.key", res.getCorrelationKey())
        .build();
  }

  public static JsonObject toJson(SparkMessageOpenSessionRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .build();
  }

  public static JsonObject toJson(SparkMessageOpenSessionResponse res) {
    return null;
  }

  public static JsonObject toJson(SparkMessageSetPropertyByNameRequest req) {
    return null;
  }

  public static JsonObject toJson(SparkMessageSetPropertyByNameResponse res) {
    return null;
  }
}
