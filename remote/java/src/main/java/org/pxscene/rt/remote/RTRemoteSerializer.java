package org.pxscene.rt.remote;

import java.io.StringReader;
import java.math.BigInteger;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import javax.json.Json;
import javax.json.JsonArray;
import javax.json.JsonArrayBuilder;
import javax.json.JsonNumber;
import javax.json.JsonObject;
import javax.json.JsonObjectBuilder;
import javax.json.JsonReader;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTEnvironment;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTFunction;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTStatus;
import org.pxscene.rt.RTStatusCode;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTValueType;
import org.pxscene.rt.remote.messages.RTMessageCallMethodRequest;
import org.pxscene.rt.remote.messages.RTMessageCallMethodResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageKeepAliveRequest;
import org.pxscene.rt.remote.messages.RTMessageKeepAliveResponse;
import org.pxscene.rt.remote.messages.RTMessageLocate;
import org.pxscene.rt.remote.messages.RTMessageOpenSessionRequest;
import org.pxscene.rt.remote.messages.RTMessageOpenSessionResponse;
import org.pxscene.rt.remote.messages.RTMessageSearch;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameResponse;


/**
 * the RTRemoteSerializer
 */
public class RTRemoteSerializer {

  /**
   * the serializer charset
   */
  public static final Charset CHARSET = Charset.forName("UTF-8");

  /**
   * the decoders map that used to decode message from remote
   */
  private static final Map<String, Function<JsonObject, RTRemoteMessage>> decoders;

  /**
   * the value type map
   */
  private static final Map<Class, RTValueType> valueTypeMap;

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTRemoteSerializer.class);


  /**
   * create map that java type to rtValue type
   */
  static {
    Map<Class, RTValueType> m = new HashMap<>();
    m.put(Boolean.class, RTValueType.BOOLEAN);
    m.put(Integer.class, RTValueType.INT32);
    m.put(Byte.class,RTValueType.INT8);
    m.put(Short.class, RTValueType.UINT8);
    m.put(Long.class, RTValueType.INT64);
    m.put(Float.class, RTValueType.FLOAT);
    m.put(Double.class, RTValueType.DOUBLE);
    m.put(RTObject.class, RTValueType.OBJECT);
    m.put(RTRemoteObject.class, RTValueType.OBJECT);
    m.put(BigInteger.class, RTValueType.UINT64);
    m.put(String.class, RTValueType.STRING);
    m.put(RTFunction.class, RTValueType.FUNCTION);
    valueTypeMap = Collections.unmodifiableMap(m);
  }


  /**
   * create decoders map
   */
  static {
    decoders = new HashMap<>();
    decoders.put(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST.toString(),
        RTRemoteSerializer::fromJson_GetPropertyByNameRequest);
    decoders.put(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE.toString(),
        RTRemoteSerializer::fromJson_GetPropertyByNameResponse);
    decoders.put(RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST.toString(),
        RTRemoteSerializer::fromJson_SetPropertyByNameRequest);
    decoders.put(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE.toString(),
        RTRemoteSerializer::fromJson_SetPropertyByNameResponse);
    decoders.put(RTRemoteMessageType.LOCATE_OBJECT.toString(),
        RTRemoteSerializer::fromJson_LocateObject);
    decoders.put(RTRemoteMessageType.KEEP_ALIVE_REQUEST.toString(),
        RTRemoteSerializer::fromJson_KeepAliveRequest);
    decoders.put(RTRemoteMessageType.METHOD_CALL_RESPONSE.toString(),
        RTRemoteSerializer::fromJson_CallMethodResponse);
    decoders.put(RTRemoteMessageType.METHOD_CALL_REQUEST.toString(),
        RTRemoteSerializer::fromJson_CallMethodRequest);
    decoders.put(RTRemoteMessageType.SERACH_OBJECT.toString(),
        RTRemoteSerializer::fromJson_SearchObject);
  }

  /**
   * get rtValue type by object
   *
   * @param obj the object
   * @return the rtValue type
   */
  public static RTValueType getMappedType(Object obj) {
    return valueTypeMap.get(obj.getClass());
  }

  /**
   * parse message GetPropertyByNameRequest
   *
   * @param obj the message json object
   * @return the RTMessageGetPropertyByNameRequest
   * @throws NullPointerException if message object is null
   */
  private static RTMessageGetPropertyByNameRequest fromJson_GetPropertyByNameRequest(
      JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("fromJson_GetPropertyByNameRequest obj cannot be null");
    }
    RTMessageGetPropertyByNameRequest req = new RTMessageGetPropertyByNameRequest();
    req.setObjectId(obj.getString(RTConst.OBJECT_ID_KEY));
    req.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    req.setPropertyName(obj.getString(RTConst.PROPERTY_NAME));
    return req;
  }

  /**
   * parse message GetPropertyByNameResponse
   *
   * @param obj the message json object
   * @return the RTMessageGetPropertyByNameResponse
   * @throws NullPointerException if message object is null
   */
  private static RTMessageGetPropertyByNameResponse fromJson_GetPropertyByNameResponse(
      JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("fromJson_GetPropertyByNameResponse obj cannot be null");
    }

    RTMessageGetPropertyByNameResponse res = new RTMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    res.setObjectId(obj.getString(RTConst.OBJECT_ID_KEY));
    res.setStatus(fromJson_SparkStatus(obj));

    if (obj.containsKey(RTConst.VALUE)) {
      JsonObject valueObject = obj.getJsonObject(RTConst.VALUE);
      res.setValue(valueFromJson(valueObject));
    }

    return res;
  }


  /**
   * parse message KeepAliveResponse
   *
   * @param obj the message json object
   * @return the RTMessageKeepAliveResponse
   * @throws NullPointerException if message object is null
   */
  private static RTMessageKeepAliveResponse fromJson_KeepAliveResponse(JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("fromJson_KeepAliveResponse obj cannot be null");
    }

    RTMessageKeepAliveResponse res = new RTMessageKeepAliveResponse();
    res.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    return res;
  }

  /**
   * parse message LocateObject
   *
   * @param obj the message json object
   * @return the RTMessageLocate
   * @throws NullPointerException if message object is null
   */
  private static RTMessageLocate fromJson_LocateObject(JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("fromJson_LocateObject obj cannot be null");
    }

    RTMessageLocate locate = new RTMessageLocate();
    locate.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    locate.setObjectId(obj.getString(RTConst.OBJECT_ID_KEY));
    try {
      locate.setEndpoint(new URI(obj.getString(RTConst.ENDPOINT)));
    } catch (URISyntaxException err) {
      logger.error(err);
    }
    return locate;
  }


  /**
   * parse message SetPropertyByNameRequest
   * example json :
   * "{"message.type":"set.byname.request","object.id":"some_name","property.name":"prop",
   * "correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3","value":{"type":52,"value":11}}"
   *
   * @param obj the message json object
   * @return the RTMessageSetPropertyByNameRequest
   * @throws NullPointerException if message object is null
   */
  private static RTMessageSetPropertyByNameRequest fromJson_SetPropertyByNameRequest(
      JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("obj");
    }

    RTMessageSetPropertyByNameRequest req = new RTMessageSetPropertyByNameRequest();
    req.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    req.setObjectId(obj.getString(RTConst.OBJECT_ID_KEY));
    req.setPropertyName(obj.getString(RTConst.PROPERTY_NAME));
    req.setValue(valueFromJson(obj.getJsonObject(RTConst.VALUE)));
    return req;
  }


  /**
   * parse message SetPropertyByNameResponse
   *
   * example json : "{"message.type":"set.byname.response", "correlation.key":"a815e2e3-560f-4914-8f5b-36a56b057ca3","object.id":"some_name","status.code":0}"
   *
   * @param obj the message json object
   * @return the RTMessageGetPropertyByNameResponse
   * @throws NullPointerException if message object is null
   */
  private static RTMessageGetPropertyByNameResponse fromJson_SetPropertyByNameResponse(
      JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("obj");
    }
    RTMessageGetPropertyByNameResponse res = new RTMessageGetPropertyByNameResponse();
    res.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    res.setObjectId(obj.getString(RTConst.OBJECT_ID_KEY));
    res.setStatus(RTRemoteSerializer.fromJson_SparkStatus(obj));
    return res;
  }

  /**
   * parse keep alive request json object to entity
   * {"message.type":"keep_alive.request","correlation.key":"3f76ed34-40e0-40ac-9ff1-f643c2b1d86c",
   * "keep_alive.ids":["func://9d403604-dd09-4fe9-8841-ce77bd5799ef"]}
   *
   * @param obj the json object
   * @return the keep alive request entity
   * @throws NullPointerException if message object is null
   */
  private static RTMessageKeepAliveRequest fromJson_KeepAliveRequest(JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("fromJson_KeepAliveRequest obj cannot be null");
    }
    RTMessageKeepAliveRequest res = new RTMessageKeepAliveRequest();
    res.setCorrelationKey(obj.getString(RTConst.CORRELATION_KEY));
    res.setKeepAliveIds(obj.getJsonArray(RTConst.KEEP_ALIVE_IDS));
    return res;
  }

  /**
   * prase call method response json object to entity
   *
   * {"message.type":"method.call.response",
   * "correlation.key":"bf326dd6-dc96-4ab1-a158-e443661811e6",
   * "function.return_value":{"type":52,"value":46},"status.code":0}
   *
   * @param object the json object
   * @return the response entity
   * @throws NullPointerException if message object is null
   */
  private static RTMessageCallMethodResponse fromJson_CallMethodResponse(JsonObject object) {
    if (object == null) {
      throw new NullPointerException("fromJson_CallMethodResponse obj cannot be null");
    }
    RTMessageCallMethodResponse response = new RTMessageCallMethodResponse();
    response.setCorrelationKey(object.getString(RTConst.CORRELATION_KEY));
    JsonObject valueJson = object.getJsonObject(RTConst.FUNCTION_RETURN_VALUE);
    if (valueJson != null) {
      response.setValue(valueFromJson(valueJson));
    }
    response.setStatus(fromJson_SparkStatus(object));
    return response;
  }

  /**
   * parse call method request json object to entity
   * {"message.type":"method.call.request","object.id":"global",
   * "correlation.key":"ae5523a6-93be-4fa1-846b-f1e56ed257a1",
   * "function.name":"func://e2127f2a-58db-48f3-b09b-0a6511f71386",
   * "function.args":[{"type":115,"value":"method1FunctionAndReturn10 invoke callback"}]}
   *
   * @param object the json object
   * @return the call method request entity
   * @throws NullPointerException if message object is null
   */
  private static RTMessageCallMethodRequest fromJson_CallMethodRequest(JsonObject object) {
    if (object == null) {
      throw new NullPointerException("fromJson_CallMethodRequest obj cannot be null");
    }
    RTMessageCallMethodRequest request = new RTMessageCallMethodRequest();
    request.setCorrelationKey(object.getString(RTConst.CORRELATION_KEY));
    request.setMethodName(object.getString(RTConst.FUNCTION_KEY));
    request.setObjectId(object.getString(RTConst.OBJECT_ID_KEY));
    JsonArray jsonArray = object.getJsonArray(RTConst.FUNCTION_ARGS);
    if (jsonArray != null && jsonArray.size() > 0) {
      List<RTValue> rtValueList = new ArrayList<>();
      for (int i = 0; i < jsonArray.size(); i++) {
        rtValueList.add(valueFromJson(jsonArray.getJsonObject(i)));
      }
      request.setFunctionArgs(rtValueList);
    }
    request.setRtFunction(jsonToFunctionValue(object));
    return request;
  }

  /**
   * parse search json object to RTMessageSearch
   *
   * {"message.type":"search","correlation.key":"0d8e844c-7afd-41f0-92d8-268f3a1fdb7d",
   * "object.id":"host_object","sender.id":0,"reply-to":"udp://127.0.0.1:52614"}
   *
   * @param object the json object
   * @return the RTMessageSearch
   */
  private static RTMessageSearch fromJson_SearchObject(JsonObject object) {
    RTMessageSearch rtMessageSearch = new RTMessageSearch();
    rtMessageSearch.setCorrelationKey(object.getString(RTConst.CORRELATION_KEY));
    rtMessageSearch.setObjectId(object.getString(RTConst.OBJECT_ID_KEY));
    rtMessageSearch.setSenderId(object.getInt(RTConst.SENDER_ID));

    try {
      rtMessageSearch.setReplyTo(new URI(object.getString(RTConst.REPLY_TO)));
    } catch (URISyntaxException err) {
      logger.error(err);
    }
    return rtMessageSearch;
  }


  /**
   * parse message SparkStatus
   *
   * @param obj the message json object
   * @return the RTStatus
   * @throws NullPointerException if message object is null
   */
  private static RTStatus fromJson_SparkStatus(JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("fromJson_SparkStatus obj cannot be null");
    }

    RTStatus status = new RTStatus();
    if (obj.containsKey(RTConst.STATUS_CODE)) {
      status.setCode(RTStatusCode.fromInt(obj.getInt(RTConst.STATUS_CODE)));
    }
    if (obj.containsKey(RTConst.STATUS_MESSAGE)) {
      status.setMessage(obj.getString(RTConst.STATUS_MESSAGE));
    }
    return status;
  }

  /**
   * convert json object to rtValue function type {"object.id":"host_object","function.name":"func://5f66c627-bd01-4137-9c7c-0b345d6d4342","type":102}
   *
   * @param jsonObject the json object
   * @return the parsed rt value
   */
  private static RTValue jsonToFunctionValue(JsonObject jsonObject) {
    String funcName = jsonObject.getString(RTConst.FUNCTION_KEY);

    RTFunction rtFunction = RTEnvironment.getRtFunctionMap().get(funcName);

    RTValue rtValue = new RTValue();
    rtValue.setType(RTValueType.FUNCTION);

    if (rtFunction == null) {
      // sometimes  RTEnvironment didn't cache the rtFunction, this mean the rtFunction from remote
      // 1. client get backend rtFunction, but client didn't cache it, this mean rtFunction locate in backend
      // 2. server receive client set->rtFunction, but server didn't cache it, this mean rtFunction lcoate in client
      rtFunction = new RTFunction();
      rtFunction.setFunctionName(funcName);
      rtFunction.setObjectId(jsonObject.getString(RTConst.OBJECT_ID_KEY));
    }
    rtValue.setValue(rtFunction);

    return rtValue;
  }

  /**
   * json to rt value object type
   */
  private static RTValue jsonToObjectValue(JsonObject jsonObject) {
    JsonObject value = jsonObject.getJsonObject(RTConst.VALUE);
    String objName = value.getString(RTConst.OBJECT_ID_KEY);
    RTObject object = RTEnvironment.getRtObjectMap().get(objName);
    RTValue rtValue = new RTValue();
    rtValue.setType(RTValueType.OBJECT);
    if (object == null) {
      object = new RTRemoteObject(null, objName);
    }
    rtValue.setValue(object);
    return rtValue;
  }

  /**
   * convert rtValue to json object
   *
   * @param value the rtValue
   * @return the json object
   * @throws NullPointerException if message object is null
   */
  public static JsonObject valueToJson(RTValue value) {
    if (value == null) {
      throw new NullPointerException("valueToJson value cannot be null");
    }
    RTValueType type = value.getType();
    JsonObjectBuilder builder = Json.createObjectBuilder();
    builder.add(RTConst.TYPE, type.getTypeCode());
    switch (type) {
      case VOID:
        break;
      case BOOLEAN:
        builder.add(RTConst.VALUE, (Boolean) value.getValue());
        break;
      case FLOAT:
        builder.add(RTConst.VALUE, (Float) value.getValue());
        break;
      case DOUBLE:
        builder.add(RTConst.VALUE, (Double) value.getValue());
        break;
      case INT8:
        builder.add(RTConst.VALUE, (Byte) value.getValue());
        break;
      case UINT8:
        builder.add(RTConst.VALUE, (Short) value.getValue());
        break;
      case INT32:
        builder.add(RTConst.VALUE, (Integer) value.getValue());
        break;
      case FUNCTION:
        RTFunction rtFunction = (RTFunction) value.getValue();
        builder.add(RTConst.OBJECT_ID_KEY, rtFunction.getObjectId());
        builder.add(RTConst.FUNCTION_KEY, rtFunction.getFunctionName());
        break;
      case OBJECT:
        JsonObjectBuilder objectBuilder = Json.createObjectBuilder();
        objectBuilder.add(RTConst.OBJECT_ID_KEY, ((RTObject) value.getValue()).getId());
        builder.add(RTConst.VALUE, objectBuilder.build());
        break;
      case UINT32:
      case VOIDPTR:
      case INT64:
        builder.add(RTConst.VALUE, (Long) value.getValue());
        break;
      case UINT64: // use big integer to parse u int64
        builder.add(RTConst.VALUE, (BigInteger) value.getValue());
        break;
      case STRING:
        builder.add(RTConst.VALUE, (String) value.getValue());
        break;
      default:
        throw new RuntimeException("type " + type + " not supported");
    }
    return builder.build();
  }

  /**
   * jsonobject to RTValue
   *
   * @param obj the json object
   * @return the rtVallue
   * @throws NullPointerException if message object is null
   */
  public static RTValue valueFromJson(JsonObject obj) {
    if (obj == null) {
      throw new NullPointerException("valueFromJson obj cannot be null");
    }
    RTValue value;
    RTValueType type = RTValueType.fromTypeCode(obj.getInt(RTConst.TYPE));
    switch (type) {
      case BOOLEAN:
        value = new RTValue(obj.getBoolean(RTConst.VALUE), type);
        break;
      case FLOAT:
        value = new RTValue((float) obj.getJsonNumber(RTConst.VALUE).doubleValue(), type);
        break;
      case DOUBLE:
        value = new RTValue(obj.getJsonNumber(RTConst.VALUE).doubleValue(), type);
        break;
      case INT8:
        value = new RTValue((byte) obj.getJsonNumber(RTConst.VALUE).intValue(), type);
        break;
      case UINT8:
        value = new RTValue((short) obj.getJsonNumber(RTConst.VALUE).intValue(), type);
        break;
      case INT32:
        value = new RTValue(obj.getJsonNumber(RTConst.VALUE).intValue(), type);
        break;
      case FUNCTION:
        value = jsonToFunctionValue(obj);
        break;
      case OBJECT:
        value = jsonToObjectValue(obj);
        break;
      case UINT32: // use long value load uint32, int64
      case INT64:
        value = new RTValue(obj.getJsonNumber(RTConst.VALUE).longValue(), type);
        break;
      case VOIDPTR:  // TODO this should be a bug from c++ remote,  the property name should be "value", not "Value"
        JsonNumber jsonNumber = obj.getJsonNumber("Value");
        if (jsonNumber == null) {
          jsonNumber = obj.getJsonNumber(RTConst.VALUE);
        }
        value = new RTValue(jsonNumber.longValue(), type);
        break;
      case UINT64: // use big integer load u int 64
        value = new RTValue(obj.getJsonNumber(RTConst.VALUE).bigIntegerValue(), type);
        break;
      case STRING:
        value = new RTValue(obj.getString(RTConst.VALUE), type);
        break;
      case VOID:
        value = new RTValue(null, type);
        break;
      default:
        throw new RuntimeException("type " + type + " not supported");
    }

    return value;
  }

  /**
   * convert json object to bytes
   *
   * @param obj the json object
   * @return the bytes
   */
  private static byte[] toBytes(JsonObject obj) {
    return obj.toString().getBytes(CHARSET);
  }

  /**
   * parse jsonObject to rt remote message object
   *
   * @param obj the json object
   * @param <T> the rtRemoteMessage type
   * @return the RTRemoteMessage
   * @throws RTException if any other error occurred during operation
   * @throws NullPointerException if decoder object is null
   */
  public <T extends RTRemoteMessage> T fromJson(JsonObject obj) throws RTException {
    if (obj == null) {
      throw new NullPointerException("obj");
    }

    if (!obj.containsKey(RTConst.MESSAGE_TYPE)) {
      throw new RTException("unsupported message type:" + obj.toString());
    }
    Function<JsonObject, RTRemoteMessage> decoder = decoders
        .get(obj.getString(RTConst.MESSAGE_TYPE));

    if (decoder == null) {
      throw new NullPointerException(
          "cannot prase the message where type = " + obj.getString(RTConst.MESSAGE_TYPE));
    }
    return (T) decoder.apply(obj);
  }

  /**
   * prase jsonString to rt remote message object
   *
   * @param s the jsonString
   * @param deserializer the decoder
   * @param <T> the rtRemoteMessage type
   * @return the RTRemoteMessage
   * @throws RTException if any other error occurred during operation
   */
  public <T extends RTRemoteMessage> T fromString(String s, Function<JsonObject, T> deserializer)
      throws RTException {
    T message;
    JsonReader reader = null;
    try {
      reader = Json.createReader(new StringReader(s));
      JsonObject obj = reader.readObject();
      if (deserializer == null) {
        message = fromJson(obj);
      } else {
        message = deserializer.apply(obj);
      }
    } finally {
      if (reader != null) {
        reader.close();
      }
    }
    return message;
  }

  /**
   * prase bytes to rt remote message object
   *
   * @param buff the bytes
   * @param offset the postion offset
   * @param length the bytes length
   * @param <T> the rtRemoteMessage type
   * @return the RTRemoteMessage
   * @throws RTException if any other error occurred during operation
   */
  public <T extends RTRemoteMessage> T fromBytes(byte[] buff, int offset, int length)
      throws RTException {
    return fromString(new String(buff, offset, length, CHARSET), null);
  }


  /**
   * convert RTMessageGetPropertyByNameRequest to json object
   *
   * @param req the RTMessageGetPropertyByNameRequest entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageGetPropertyByNameRequest req) {
    if (req == null) {
      throw new NullPointerException("req");
    }
    JsonObject obj = Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, req.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, req.getCorrelationKey())
        .add(RTConst.PROPERTY_NAME, req.getPropertyName())
        .add(RTConst.OBJECT_ID_KEY, req.getObjectId())
        .build();
    return obj;
  }

  /**
   * convert RTMessageCallMethodResponse to json object
   *
   * @param response the RTMessageCallMethodResponse entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageCallMethodResponse response) {
    if (response == null) {
      throw new NullPointerException("response");
    }
    JsonObjectBuilder build = Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, response.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, response.getCorrelationKey())
        .add(RTConst.STATUS_CODE, response.getStatus().getCode().getCode());

    if (response.getValue() != null) {
      build.add(RTConst.FUNCTION_RETURN_VALUE, valueToJson(response.getValue()));
    }
    return build.build();
  }


  /**
   * convert RTMessageLocate to json object
   *
   * @param locate the RTMessageLocate entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageLocate locate) {
    if (locate == null) {
      throw new NullPointerException("locate");
    }
    JsonObject obj = Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, locate.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, locate.getCorrelationKey())
        .add(RTConst.OBJECT_ID_KEY, locate.getObjectId())
        .add(RTConst.ENDPOINT, locate.getEndpoint().toString())
        .build();
    return obj;
  }


  /**
   * convert RTMessageGetPropertyByNameResponse to json object
   *
   * @param res the RTMessageGetPropertyByNameResponse entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageGetPropertyByNameResponse res) {
    JsonObject obj = Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, res.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, res.getCorrelationKey())
        .add(RTConst.OBJECT_ID_KEY, res.getObjectId())
        .add(RTConst.VALUE, valueToJson(res.getValue()))
        .add(RTConst.STATUS_CODE, res.getStatus().getCode().getCode())
        .build();
    return obj;
  }

  /**
   * convert RTMessageKeepAliveRequest to json object
   *
   * @param req the RTMessageKeepAliveRequest entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageKeepAliveRequest req) {
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, req.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, req.getCorrelationKey())
        .build();
  }

  /**
   * convert RTMessageKeepAliveResponse to json object
   *
   * @param res the RTMessageKeepAliveResponse entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageKeepAliveResponse res) {
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, res.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, res.getCorrelationKey())
        .build();
  }

  /**
   * convert RTMessageOpenSessionRequest to json object
   *
   * @param req the RTMessageOpenSessionRequest entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageOpenSessionRequest req) {
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, req.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, req.getCorrelationKey())
        .build();
  }

  /**
   * convert RTMessageSearch to json object
   *
   * @param search the RTMessageSearch entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageSearch search) {
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, search.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, search.getCorrelationKey())
        .add(RTConst.OBJECT_ID_KEY, search.getObjectId())
        .add(RTConst.SENDER_ID, search.getSenderId())
        .add(RTConst.REPLY_TO, search.getReplyTo().toString())
        .build();
  }

  /**
   * convert RTMessageCallMethodRequest to json object
   *
   * @param request the RTMessageCallMethodRequest entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageCallMethodRequest request) {
    JsonArrayBuilder arrayBuilder = Json.createArrayBuilder();
    if (request.getFunctionArgs() != null && request.getFunctionArgs().size() > 0) {
      for (RTValue rtValue : request.getFunctionArgs()) {
        arrayBuilder.add(valueToJson(rtValue));
      }
    }
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, request.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, request.getCorrelationKey())
        .add(RTConst.OBJECT_ID_KEY, request.getObjectId())
        .add("function.name", request.getMethodName())
        .add(RTConst.FUNCTION_ARGS, arrayBuilder.build())
        .build();
  }

  /**
   * convert RTMessageOpenSessionResponse to json object
   * TODO
   *
   * @param res the RTMessageOpenSessionResponse entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageOpenSessionResponse res) {
    return null;
  }

  /**
   * convert RTMessageSetPropertyByNameRequest to json object
   *
   * @param req the RTMessageSetPropertyByNameRequest entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageSetPropertyByNameRequest req) {
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, req.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, req.getCorrelationKey())
        .add(RTConst.OBJECT_ID_KEY, req.getObjectId())
        .add(RTConst.PROPERTY_NAME, req.getPropertyName())
        .add(RTConst.VALUE, RTRemoteSerializer.valueToJson(req.getValue()))
        .build();
  }


  /**
   * convert RTMessageSetPropertyByNameResponse to json object
   *
   * @param res the RTMessageSetPropertyByNameResponse entity
   * @return the json object
   * @throws NullPointerException if decoder object is null
   */
  private JsonObject toJson(RTMessageSetPropertyByNameResponse res) {
    return Json.createObjectBuilder()
        .add(RTConst.MESSAGE_TYPE, res.getMessageType().toString())
        .add(RTConst.CORRELATION_KEY, res.getCorrelationKey())
        .add(RTConst.OBJECT_ID_KEY, res.getObjectId())
        .add(RTConst.STATUS_CODE, res.getStatusCode().getCode())
        .build();
  }

  /**
   * convert RTMessageCallMethodRequest to bytes
   *
   * @param m the RTMessageCallMethodRequest entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageCallMethodRequest m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }

  /**
   * convert RTMessageCallMethodResponse to bytes
   *
   * @param response the RTMessageCallMethodResponse entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageCallMethodResponse response) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(response));
  }

  /**
   * convert RTMessageCallMethodRequest to bytes
   *
   * @param response the RTMessageCallMethodRequest entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageSetPropertyByNameResponse response) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(response));
  }

  /**
   * convert RTMessageSetPropertyByNameRequest to bytes
   *
   * @param m the RTMessageSetPropertyByNameRequest entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageSetPropertyByNameRequest m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }

  /**
   * convert RTMessageGetPropertyByNameRequest to bytes
   *
   * @param m the RTMessageGetPropertyByNameRequest entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageGetPropertyByNameRequest m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }


  /**
   * convert RTMessageGetPropertyByNameResponse to bytes
   *
   * @param m the RTMessageGetPropertyByNameResponse entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageGetPropertyByNameResponse m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }


  /**
   * convert RTMessageSearch to bytes
   *
   * @param m the RTMessageSearch entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageSearch m) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(m));
  }

  /**
   * convert RTMessageLocate to bytes
   *
   * @param locate the RTMessageLocate entity
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  public byte[] toBytes(RTMessageLocate locate) throws RTException {
    return RTRemoteSerializer.toBytes(toJson(locate));
  }

}
