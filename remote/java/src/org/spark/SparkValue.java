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


// "{"message.type":"get.byname.response","correlation.key":"ae01cddf-615f-4ff2-a8b9-4b7434687dc7"," +
//    ""object.id":"some_name","value":{"type":52,"value":13},"status.code":0}"
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

  public String toString() {
    return m_value.toString();
    /*
    JsonObject obj = Json.createObjectBuilder()
        .add("type", (int) m_type.getTypeCode())
        .add("value", getValueAsString())
        .build();
    return obj.toString();
    */
  }
}
