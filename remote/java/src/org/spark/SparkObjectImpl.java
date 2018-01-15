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

import java.util.concurrent.Future;

public class SparkObjectImpl implements SparkObject {
  private SparkProtocol m_protocol;
  private String m_id;

  public SparkObjectImpl(SparkProtocol proto, String objectId) {
    m_protocol = proto;
    m_id = objectId;
  }

  public Future<Void> set(String name, SparkValue value) throws SparkException {
    return m_protocol.sendSetByName(m_id, name, value);
  }

  public Future<Void> set(int index, SparkValue value) throws SparkException {
    return m_protocol.sendSetById(m_id, index, value);
  }

  public Future<SparkValue> get(String name) throws SparkException {
    return m_protocol.sendGetByName(m_id, name);
  }

  public Future<SparkValue> get(int index) throws SparkException {
    return m_protocol.sendGetById(m_id, index);
  }

  public Future<Void> send(String name, SparkValue ... arguments) throws SparkException {
    return null;
  }

  public Future<SparkValue> sendReturns(String name, SparkValue ... arguments) throws SparkException {
    return null;
  }

  public String getId() {
    return m_id;
  }
}
