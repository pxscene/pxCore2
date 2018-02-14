//
// Copyright [2018] [jacobgladish@yahoo.com]
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
package org.pxscene.rt;


import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.remote.RTRemoteSerializer;

/**
 * the basic rtValue
 */
@ToString
public class RTValue {

  /**
   * the value type
   */
  @Getter
  @Setter
  @NonNull
  protected RTValueType type;

  /**
   * the value
   */
  @Getter
  @Setter
  @NonNull
  protected Object value;


  /**
   * create new rtValue
   */
  public RTValue() {
  }

  /**
   * create new rtValue with value
   *
   * @param value the value
   */
  public RTValue(Object value) {
    this.value = value;
    type = RTRemoteSerializer.getMappedType(value);
  }

  /**
   * create new rtValue with int value
   *
   * @param n the value
   */
  public RTValue(int n) {
    value = n;
    type = RTValueType.INT32;
  }

  /**
   * create new rtValue with float value
   *
   * @param f the value
   */
  public RTValue(float f) {
    value = f;
    type = RTValueType.FLOAT;
  }

  /**
   * create new rtValue with double value
   *
   * @param d the value
   */
  public RTValue(double d) {
    value = d;
    type = RTValueType.DOUBLE;
  }

  /**
   * create new rtValue String value
   *
   * @param s the value
   */
  public RTValue(String s) {
    value = s;
    type = RTValueType.STRING;
  }

  /**
   * create new rtValue with rtFunction value
   *
   * @param rtFunction the rtFunction value
   */
  public RTValue(RTFunction rtFunction) {
    this(rtFunction, RTValueType.FUNCTION);
  }

  /**
   * create new rtValue with rt object value
   *
   * @param rtObject the rtObject value
   */
  public RTValue(RTObject rtObject) {
    this(rtObject, RTValueType.OBJECT);
  }

  /**
   * create new rtValue with value
   *
   * @param value the value
   * @param type the rtValue type
   */
  public RTValue(Object value, RTValueType type) {
    this.value = value;
    this.type = type;

    if (this.type.equals(RTValueType.FUNCTION)) {
      RTFunction rtFunction = (RTFunction) value;
      RTEnvironment.getRtFunctionMap().put(rtFunction.getFunctionName(), rtFunction);
    } else if (this.type.equals(RTValueType.OBJECT)) {
      RTObject rtObject = (RTObject) value;
      RTEnvironment.getRtObjectMap().put(rtObject.getId(), rtObject);
    }
  }
}
