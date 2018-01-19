package org.pxscene.rt.remote;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonObjectBuilder;
import javax.json.JsonReader;

import java.io.StringReader;
import java.nio.charset.Charset;
import java.util.Collections;
import java.util.Map;
import java.util.HashMap;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.function.Function;

import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.RTStatusCode;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTValueType;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageMessageKeepAliveRequest;
import org.pxscene.rt.remote.messages.RTMessageKeepAliveResponse;
import org.pxscene.rt.remote.messages.RTMessageOpenSessionRequest;
import org.pxscene.rt.remote.messages.RTMessageOpenSessionResponse;
import org.pxscene.rt.remote.messages.RTMessageSearch;
import org.pxscene.rt.remote.messages.RTMessageLocate;


public class RTRemoteSerializer {
  private static final Map<String, Function<JsonObject, RTRemoteMessage>> m_decoders;
  private static final Charset m_charset = Charset.forName("UTF-8");

  private static final Map<Class, RTValueType> m_valueTypeMap;

  static {
    Map<Class, RTValueType> m = new HashMap<>();
    m.put(Boolean.class, RTValueType.BOOLEAN);
    m.put(Integer.class, RTValueType.INT32);
    m.put(Long.class, RTValueType.INT64);
    m.put(Float.class, RTValueType.FLOAT);
    m.put(Double.class, RTValueType.DOUBLE);
    m.put(RTObject.class, RTValueType.OBJECT);
    m_valueTypeMap = Collections.unmodifiableMap(m);
  };

  static {
    m_decoders = new HashMap<>();
    m_decoders.put(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST.toString(), (JsonObject json) -> {
      return RTRemoteSerializer.fromJson_GetPropertyByNameRequest(json);
    });
    m_decoders.put(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE.toString(), (JsonObject json) -> {
      return RTRemoteSerializer.fromJson_GetPropertyByNameResponse(json);
    });
    m_decoders.put(RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST.toString(), (JsonObject json) -> {
      return RTRemoteSerializer.fromJson_SetPropertyByNameRequest(json);
    });
    m_decoders.put(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE.toString(), (JsonObject json) -> {
      return RTRemoteSerializer.fromJson_SetPropertyByNameResponse(json);
    });
    m_decoders.put(RTRemoteMessageType.LOCATE_OBJECT.toString(), (JsonObject json) -> {
      return RTRemoteSerializer.fromJson_LocateObject(json);
    });
  }

  public static RTValueType getMappedType(Object obj) {
    return m_valueTypeMap.get(obj.getClass());
  }

  public <T extends RTRemoteMessage> T fromJson(JsonObject obj) throws RTException {
    if (obj == null)
      throw new NullPointerException("obj");

    if (!obj.containsKey("message.type"))
      throw new RTException("unsupported message type:" + obj.toString());

    Function<JsonObject, RTRemoteMessage> decoder = m_decoders.get(obj.getString("message.type"));
    return (T) decoder.apply(obj);
  }

  public <T extends RTRemoteMessage> T fromString(String s, Function<JsonObject, T> deserializer)
      throws RTException {
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

  public <T extends RTRemoteMessage> T fromBytes(byte[] buff, int offset, int length) throws RTException {
    return fromString(new String(buff, offset, length, m_charset) ,null);
  }

  private static RTMessageGetPropertyByNameRequest fromJson_GetPropertyByNameRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");
    RTMessageGetPropertyByNameRequest req = new RTMessageGetPropertyByNameRequest();
    req.setObjectId(obj.getString("object.id"));
    req.setCorrelationKey(obj.getString("correlation.key"));
    req.setPropertyName(obj.getString("property.name"));
    return req;
  }

  private static RTMessageGetPropertyByNameResponse fromJson_GetPropertyByNameResponse(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTMessageGetPropertyByNameResponse res = new RTMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    res.setObjectId(obj.getString("object.id"));
    res.setStatus(fromJson_SparkStatus(obj));

    if (obj.containsKey("value")) {
      JsonObject valueObject = obj.getJsonObject("value");
      res.setValue(valueFromJson(valueObject));
    }

    return res;
  }

  private static RTMessageMessageKeepAliveRequest fromJson_KeepAliveRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTMessageMessageKeepAliveRequest req = new RTMessageMessageKeepAliveRequest();
    req.setCorrelationKey(obj.getString("correlation.key"));
    return req;
  }

  private static RTMessageKeepAliveResponse fromJson_KeepAliveResponse(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTMessageKeepAliveResponse res = new RTMessageKeepAliveResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    return res;
  }

  private static RTMessageLocate fromJson_LocateObject(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTMessageLocate locate = new RTMessageLocate();
    locate.setCorrelationKey(obj.getString("correlation.key"));
    locate.setObjectId(obj.getString("object.id"));
    try {
      locate.setEndpoint(new URI(obj.getString("endpoint")));
    } catch (URISyntaxException err) {
      // TODO:
    }
    return locate;
  }

  // "{"message.type":"set.byname.request","object.id":"some_name","property.name":"prop",
  // "correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3","value":{"type":52,"value":11}}"
  private static RTMessageSetPropertyByNameRequest fromJson_SetPropertyByNameRequest(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTMessageSetPropertyByNameRequest req = new RTMessageSetPropertyByNameRequest();
    req.setCorrelationKey(obj.getString("correlation.key"));
    req.setObjectId(obj.getString("object.id"));
    req.setPropertyName(obj.getString("property.name"));
    req.setValue(valueFromJson(obj.getJsonObject("value")));
    return req;
  }

  // "{"message.type":"set.byname.response","correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3",
  // "object.id":"some_name","status.code":0}"
  private static RTMessageGetPropertyByNameResponse fromJson_SetPropertyByNameResponse(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTMessageGetPropertyByNameResponse res = new RTMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString("correlation.key"));
    res.setObjectId(obj.getString("object.id"));
    res.setStatus(RTRemoteSerializer.fromJson_SparkStatus(obj));
    return res;
  }

  private static RTStatus fromJson_SparkStatus(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTStatus status = new RTStatus();
    if (obj.containsKey("status.code"))
      status.setCode(RTStatusCode.fromInt(obj.getInt("status.code")));
    if (obj.containsKey("status.message"))
      status.setMessge(obj.getString("status.message"));
    return status;
  }

  // toJson
  public JsonObject toJson(RTMessageGetPropertyByNameRequest req) {
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

  public JsonObject toJson(RTMessageGetPropertyByNameResponse res) {
    JsonObject obj = Json.createObjectBuilder()
        .add("message.type", res.getMessageType().toString())
        .add("correlation.key", res.getCorrelationKey())
        .add("object.id", res.getObjectId())
        .build();
    return obj;
  }

  public JsonObject toJson(RTMessageMessageKeepAliveRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .build();
  }

  public JsonObject toJson(RTMessageKeepAliveResponse res) {
    return Json.createObjectBuilder()
        .add("message.type", res.getMessageType().toString())
        .add("correlation.key", res.getCorrelationKey())
        .build();
  }

  public JsonObject toJson(RTMessageOpenSessionRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .build();
  }

  public JsonObject toJson(RTMessageSearch search) {
    return Json.createObjectBuilder()
        .add("message.type", search.getMessageType().toString())
        .add("correlation.key", search.getCorrelationKey())
        .add("object.id", search.getObjectId())
        .add("sender.id", search.getSenderId())
        .add("reply-to", search.getReplyTo().toString())
        .build();
  }

  public JsonObject toJson(RTMessageOpenSessionResponse res) {
    return null;
  }

  // "{"message.type":"set.byname.request","object.id":"some_name","property.name":"prop",
  // "correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3","value":{"type":52,"value":11}}"
  public JsonObject toJson(RTMessageSetPropertyByNameRequest req) {
    return Json.createObjectBuilder()
        .add("message.type", req.getMessageType().toString())
        .add("correlation.key", req.getCorrelationKey())
        .add("object.id", req.getObjectId())
        .add("property.name", req.getPropertyName())
        .add("value", RTRemoteSerializer.valueToJson(req.getValue()))
        .build();
  }

  public byte[] toBytes(RTMessageSetPropertyByNameRequest m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }

  public byte[] toBytes(RTMessageGetPropertyByNameRequest m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }

  public byte[] toBytes(RTMessageSearch m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }

  // "{"message.type":"set.byname.response","correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3",
  // "object.id":"some_name","status.code":0}"
  public JsonObject toJson(RTMessageSetPropertyByNameResponse res) {
    return null;
  }

  public static JsonObject valueToJson(RTValue value) {
    if (value == null)
      throw new NullPointerException("value");

    RTValueType type = value.getType();
    JsonObjectBuilder builder = Json.createObjectBuilder();

    builder.add("type", type.getTypeCode());

    switch (type) {
      case BOOLEAN:
        builder.add("value", ((Boolean)value.getValue()).booleanValue());
        break;
      case FLOAT:
        builder.add("value", ((Float)value.getValue()).floatValue());
        break;
      case INT32:
        builder.add("value", ((Integer)value.getValue()).intValue());
        break;
      case STRING:
        builder.add("value", (String) value.getValue());
        break;
      default:
        throw new RuntimeException("type " + type + " not supported");
    }

    return builder.build();
  }

  public static RTValue valueFromJson(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    RTValue value = null;
    RTValueType type = RTValueType.fromTypeCode(obj.getInt("type"));

    // TODO: lots of types not supported
    switch (type) {
      case BOOLEAN:
        value = new RTValue(obj.getBoolean("value"), type);
        break;
      case FLOAT:
        value = new RTValue((float) obj.getJsonNumber("value").doubleValue(), type);
        break;
      case INT32:
        value = new RTValue(obj.getJsonNumber("value").intValue(), type);
        break;
      case STRING:
        value = new RTValue(obj.getString("value"), type);
        break;

      default:
        throw new RuntimeException("type " + type + " not supported");
    }

    return value;
  }

  private static byte[] toBytes(JsonObject obj) {
    return obj.toString().getBytes(m_charset);
  }
}
