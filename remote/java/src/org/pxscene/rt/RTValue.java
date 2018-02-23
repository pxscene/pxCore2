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

import org.pxscene.rt.remote.RTRemoteSerializer;

public class RTValue {
  private RTValueType m_type;
  private Object m_value;

  public RTValue(Object value) {
    m_value = value;
    m_type = RTRemoteSerializer.getMappedType(value);
  }

  public RTValue(int n) {
    m_value = n;
    m_type = RTValueType.INT32;
  }

  public RTValue(float f) {
    m_value = f;
    m_type = RTValueType.FLOAT;
  }

  public RTValue(double d) {
    m_value = d;
    m_type = RTValueType.DOUBLE;
  }

  public RTValue(String s) {
    m_value = s;
    m_type = RTValueType.STRING;
  }

  public RTValue(Object value, RTValueType type) {
    m_value = value;
    m_type = type;
  }

  public RTValueType getType() {
    return m_type;
  }

  public Object getValue() {
    return m_value;
  }

  public String toString() {
    return m_value.toString();
  }
}
