package org.spark.net;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;

import java.io.StringReader;
import java.nio.charset.Charset;
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


public class SparkSerializer {
  private static final Map<String, Function<JsonObject, SparkMessage>> m_decoders;
  private static final Charset m_charset = Charset.forName("UTF-8");

  static {
    m_decoders = new HashMap<>();
    m_decoders.put(SparkMessageType.GET_PROPERTY_BYNAME_REQUEST.toString(), (JsonObject json) -> {
      return SparkSerializer.fromJson_GetPropertyByNameRequest(json);
    });
    m_decoders.put(SparkMessageType.GET_PROPERTY_BYNAME_RESPONSE.toString(), (JsonObject json) -> {
      return SparkSerializer.fromJson_GetPropertyByNameResponse(json);
    });
    m_decoders.put(SparkMessageType.SET_PROPERTY_BYNAME_REQUEST.toString(), (JsonObject json) -> {
      return SparkSerializer.fromJson_SetPropertyByNameRequest(json);
    });
    m_decoders.put(SparkMessageType.SET_PROPERTY_BYNAME_RESPONSE.toString(), (JsonObject json) -> {
      return SparkSerializer.fromJson_SetPropertyByNameResponse(json);
    });
  }

  public <T extends SparkMessage> T fromJson(JsonObject obj) throws SparkException {
    if (obj == null)
      throw new NullPointerException("obj");

    if (!obj.containsKey("message.type"))
      throw new SparkException("unsupported message type:" + obj.toString());

    Function<JsonObject, SparkMessage> decoder = m_decoders.get(obj.getString("message.type"));
    return (T) decoder.apply(obj);
  }

  public <T extends SparkMessage> T fromString(String s, Function<JsonObject, T> deserializer)
      throws SparkException {
    T message = null;
    JsonReader reader = null;

    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();
      if (deserializer == null)
        message = fromJson(obj);
      else
        message = deserializer.apply(obj);
    } finally {
      if (reader != null)
        reader.close();
    }
    return message;
  }

  public <T extends SparkMessage> T fromBytes(byte[] buff, int offset, int length) throws SparkException {
    return fromString(new String(buff, offset, length, m_charset) ,null);
  }

  private static SparkMessageGetPropertyByNameRequest fromJson_GetPropertyByNameRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");
    SparkMessageGetPropertyByNameRequest req = new SparkMessageGetPropertyByNameRequest();
    req.setObjectId(obj.getString("object.id"));
    req.setCorrelationKey(obj.getString("correlation.key"));
    req.setPropertyName(obj.getString("property.name"));
    return req;
  }

  private static SparkMessageGetPropertyByNameResponse fromJson_GetPropertyByNameResponse(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkMessageGetPropertyByNameResponse res = new SparkMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    res.setObjectId(obj.getString("object.id"));
    res.setStatus(fromJson_SparkStatus(obj));

    if (obj.containsKey("value")) {
      JsonObject valueObject = obj.getJsonObject("value");
      res.setValue(SparkValueSerializer.fromJson(valueObject));
    }

    return res;
  }

  private static SparkMessageKeepAliveRequest fromJson_KeepAliveRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkMessageKeepAliveRequest req = new SparkMessageKeepAliveRequest();
    req.setCorrelationKey(obj.getString("correlation.key"));
    return req;
  }

  private static SparkMessageKeepAliveResponse fromJson_KeepAliveResponse(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkMessageKeepAliveResponse res = new SparkMessageKeepAliveResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    return res;
  }

  // "{"message.type":"set.byname.request","object.id":"some_name","property.name":"prop",
  // "correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3","value":{"type":52,"value":11}}"
  private static SparkMessageSetPropertyByNameRequest fromJson_SetPropertyByNameRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkMessageSetPropertyByNameRequest req = new SparkMessageSetPropertyByNameRequest();
    req.setCorrelationKey(obj.getString("correlation.key"));
    req.setObjectId(obj.getString("object.id"));
    req.setPropertyName(obj.getString("property.name"));
    req.setValue(SparkValueSerializer.fromJson(obj.getJsonObject("value")));
    return req;
  }

  // "{"message.type":"set.byname.response","correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3",
  // "object.id":"some_name","status.code":0}"
  private static SparkMessageGetPropertyByNameResponse fromJson_SetPropertyByNameResponse(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkMessageGetPropertyByNameResponse res = new SparkMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    res.setObjectId(obj.getString("object.id"));
    res.setStatus(SparkSerializer.fromJson_SparkStatus(obj));
    return res;
  }

  private static SparkStatus fromJson_SparkStatus(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkStatus status = new SparkStatus();
    if (obj.containsKey("status.code"))
      status.setCode(SparkStatusCode.fromInt(obj.getInt("status.code")));
    if (obj.containsKey("status.message"))
      status.setMessge(obj.getString("status.message"));
    return status;
  }

  // toJson
  public JsonObject toJson(SparkMessageGetPropertyByNameRequest req) {
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

  public JsonObject toJson(SparkMessageGetPropertyByNameResponse res) {
    JsonObject obj = Json.createObjectBuilder()
        .add("message.type", res.getMessageType().toString())
        .add("correlation.key", res.getCorrelationKey())
        .add("object.id", res.getObjectId())
        .build();
    return obj;
  }

  public JsonObject toJson(SparkMessageKeepAliveRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .build();
  }

  public JsonObject toJson(SparkMessageKeepAliveResponse res) {
    return Json.createObjectBuilder()
        .add("message.type", res.getMessageType().toString())
        .add("correlation.key", res.getCorrelationKey())
        .build();
  }

  public JsonObject toJson(SparkMessageOpenSessionRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .build();
  }

  public JsonObject toJson(SparkMessageOpenSessionResponse res) {
    return null;
  }

  // "{"message.type":"set.byname.request","object.id":"some_name","property.name":"prop",
  // "correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3","value":{"type":52,"value":11}}"
  public JsonObject toJson(SparkMessageSetPropertyByNameRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .add("object.id", req.getObjectId())
        .add("property.name", req.getPropertyName())
        .add("value", SparkValueSerializer.toJson(req.getValue()))
        .build();
  }

  public byte[] toBytes(SparkMessageSetPropertyByNameRequest m) throws SparkException {
    return SparkSerializer.toBytes(toJson(m));
  }

  public byte[] toBytes(SparkMessageGetPropertyByNameRequest m) throws SparkException {
    return SparkSerializer.toBytes(toJson(m));
  }

  // "{"message.type":"set.byname.response","correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3",
  // "object.id":"some_name","status.code":0}"
  public JsonObject toJson(SparkMessageSetPropertyByNameResponse res) {
    return null;
  }

  private static byte[] toBytes(JsonObject obj) {
    return obj.toString().getBytes(m_charset);
  }
}
