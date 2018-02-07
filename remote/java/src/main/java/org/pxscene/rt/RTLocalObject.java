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
 * the rt local object
 */
public class RTLocalObject extends RTValue {

  /**
   * the static map used to store local object, so that getObject can found the real local object
   */
  private static Map<String, RTLocalObject> objectMap
      = new ConcurrentHashMap<>();

  /**
   * the logger instance
   */
  private static Logger logger = Logger.getLogger(RTLocalObject.class);

  /**
   * the object name
   */
  @Setter
  @Getter
  protected String objName;

  /**
   * create new RTLocalObject
   *
   * @param init is that need init, if is true, the object will put in static map
   */
  public RTLocalObject(boolean init) {
    if (init) {
      this.objName = "obj://" + UUID.randomUUID().toString();
      value = buildValue();
      objectMap.put(this.objName, this);
    }
    type = RTValueType.OBJECT;
  }

  /**
   * convert json object to RTLocalObject
   *
   * @param jsonObject the json object
   * @return the RTLocalObject
   */
  public static RTLocalObject fromJson(JsonObject jsonObject) {
    JsonObject value = jsonObject.getJsonObject("value");
    String objName = value.getString("object.id");
    RTLocalObject localObject = objectMap.get(objName);
    if (localObject == null) {
      RTLocalObject.logger.warn("cannot found object in local, obj name = " + objName);
      localObject = new RTLocalObject(false);
      localObject.setObjName(objName);
      localObject.setValue(localObject.buildValue());
    }
    return localObject;
  }


  /**
   * build json value
   *
   * @return the json object
   */
  public JsonObject buildValue() {
    JsonObjectBuilder builder = Json.createObjectBuilder();
    builder.add("object.id", this.objName);
    return builder.build();
  }
}
