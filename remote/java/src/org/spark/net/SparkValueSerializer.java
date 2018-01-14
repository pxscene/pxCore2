package org.spark.net;

import org.spark.SparkObject;
import org.spark.SparkValue;
import org.spark.SparkValueType;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonObjectBuilder;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

// {"type":52,"value":10}
public class SparkValueSerializer {
  private static final Map<Class, SparkValueType> m_valueTypeMap;

  static {
    Map<Class, SparkValueType> m = new HashMap<>();
    m.put(Boolean.class, SparkValueType.BOOLEAN);
    m.put(Integer.class, SparkValueType.INT32);
    m.put(Long.class, SparkValueType.INT64);
    m.put(Float.class, SparkValueType.FLOAT);
    m.put(Double.class, SparkValueType.DOUBLE);
    m.put(SparkObject.class, SparkValueType.OBJECT);
    m_valueTypeMap = Collections.unmodifiableMap(m);
  };

  public static SparkValueType getMappedType(Object obj) {
    return m_valueTypeMap.get(obj.getClass());
  }

  public JsonObject toJson(SparkValue value) {
    if (value == null)
      throw new NullPointerException("value");

    SparkValueType type = value.getType();
    JsonObjectBuilder builder = Json.createObjectBuilder();

    builder.add("type", type.getTypeCode());

    switch (type) {
      case BOOLEAN:
        builder.add("type", ((Boolean)value.getValue()).booleanValue());
        break;
      case FLOAT:
        builder.add("type", ((Float)value.getValue()).floatValue());
        break;
      case INT32:
        builder.add("type", ((Integer)value.getValue()).intValue());
        break;
      case STRING:
        builder.add("type", (String) value.getValue());
        break;
      default:
        throw new RuntimeException("type " + type + " not supported");
    }

    return builder.build();
  }

  public static SparkValue fromJson(JsonObject obj) {
    if (obj == null)
      throw new NullPointerException("obj");

    SparkValue value = null;
    SparkValueType type = SparkValueType.fromTypeCode(obj.getInt("type"));

    // TODO: lots of types not supported
    switch (type) {
      case BOOLEAN:
        value = new SparkValue(obj.getBoolean("value"), type);
        break;
      case FLOAT:
        value = new SparkValue((float) obj.getJsonNumber("value").doubleValue(), type);
        break;
      case INT32:
        value = new SparkValue(obj.getJsonNumber("value").intValue(), type);
        break;
      case STRING:
        value = new SparkValue(obj.getString("value"), type);
        break;

      default:
        throw new RuntimeException("type " + type + " not supported");
    }

    return value;
  }
}
