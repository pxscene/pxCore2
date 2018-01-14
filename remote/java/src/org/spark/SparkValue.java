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
package org.spark;

import org.spark.net.SparkValueSerializer;

public class SparkValue {
  private SparkValueType m_type;
  private Object m_value;

  public SparkValue(Object value) {
    m_value = value;
    m_type = SparkValueSerializer.getMappedType(value);
  }

  public SparkValue(Object value, SparkValueType type) {
    m_value = value;
    m_type = type;
  }

  public SparkValueType getType() {
    return m_type;
  }

  public Object getValue() {
    return m_value;
  }

  public String toString() {
    return m_value.toString();
  }
}
