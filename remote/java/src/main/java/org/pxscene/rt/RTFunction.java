package org.pxscene.rt;

import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonObjectBuilder;
import lombok.Getter;
import lombok.Setter;
import org.apache.log4j.Logger;

/**
 * the rt function type extend from rtValue
 */
public class RTFunction extends RTValue {

  /**
   * the static listener map, used to store local listener, thread safe
   */
  private static Map<String, RTFunctionListener> listenerConcurrentHashMap
      = new ConcurrentHashMap<>();

  /**
   * the logger instance
   */
  private static Logger logger = Logger.getLogger(RTFunction.class);

  /**
   * the private self RTFunctionListener
   */
  @Getter
  protected RTFunctionListener listener;

  /**
   * the function name
   */
  @Getter
  @Setter
  protected String funcName;

  /**
   * private constructor
   */
  private RTFunction() {
  }

  /**
   * create new rtFunctionw with listener and put it in static map
   *
   * @param listener the RTFunctionListener entity
   */
  public RTFunction(RTFunctionListener listener) {
    this.funcName = "func://" + UUID.randomUUID().toString();
    this.listener = listener;
    value = buildValue();
    type = RTValueType.FUNCTION;

    // put it to map
    listenerConcurrentHashMap.put(funcName, listener);
  }


  /**
   * parase rtFunction from remote
   * note that, this function will be found listern from static map by function name
   *
   * @param jsonObject the json object
   * @return the RTFunction entity
   */
  public static RTFunction fromJson(JsonObject jsonObject) {
    RTFunction rtFunction = new RTFunction();
    rtFunction.setFuncName(jsonObject.getString("function.name"));
    rtFunction.setType(RTValueType.FUNCTION);
    rtFunction.setValue(rtFunction.buildValue());

    RTFunctionListener listener = listenerConcurrentHashMap.get(rtFunction.getFuncName());
    if (listener == null) {
      RTFunction.logger
          .warn("cannot found local function listener, func name = " + rtFunction.getFuncName());
    }
    rtFunction.setListener(listener);
    return rtFunction;
  }


  /**
   * build json value
   *
   * @return the json object value
   */
  public JsonObject buildValue() {
    JsonObjectBuilder builder = Json.createObjectBuilder();
    builder.add("object.id", "global");
    builder.add("function.name", funcName);
    return builder.build();
  }

  /**
   * set RTFunctionListener to rtFunction
   *
   * @param listener the RTFunctionListener entity
   */
  public void setListener(RTFunctionListener listener) {
    if (listener == null) {
      listenerConcurrentHashMap.remove(funcName);
    } else {
      listenerConcurrentHashMap.put(funcName, listener);
    }
    this.listener = listener;
  }
}
