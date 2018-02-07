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
import lombok.Setter;
import org.pxscene.rt.remote.RTRemoteSerializer;

/**
 * the basic rtValue
 */
public class RTValue {

  @Getter
  @Setter
  protected RTValueType type;

  @Getter
  @Setter
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
   * create new rtValue with value
   *
   * @param value the value
   * @param type the rtValue type
   */
  public RTValue(Object value, RTValueType type) {
    this.value = value;
    this.type = type;
  }

  /**
   * convert rtValue to string value
   *
   * @return the string value
   */
  public String toString() {
    return value.toString();
  }
}
