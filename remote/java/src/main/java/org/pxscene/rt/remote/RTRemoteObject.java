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
package org.pxscene.rt.remote;

import java.util.concurrent.Future;
import lombok.Getter;
import lombok.Setter;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTValue;

/**
 * the rt remote object
 */
public class RTRemoteObject implements RTObject {

  /**
   * the remote protocol instance
   */
  private RTRemoteProtocol protocol;

  /**
   * the remote object id
   */
  @Getter
  private String id;

  /**
   * create new remote object
   *
   * @param proto the remote protocol instance
   * @param objectId the remote object id
   */
  public RTRemoteObject(RTRemoteProtocol proto, String objectId) {
    protocol = proto;
    id = objectId;
  }

  /**
   * set remote object property value
   *
   * @param name the property name
   * @param value the value
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<Void> set(String name, RTValue value) throws RTException {
    return protocol.sendSetByName(id, name, value);
  }

  /**
   * set remote object property value
   *
   * @param index the property index
   * @param value the value
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<Void> set(int index, RTValue value) throws RTException {
    return protocol.sendSetById(id, index, value);
  }

  /**
   * get remote object property value
   *
   * @param name the property name
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<RTValue> get(String name) throws RTException {
    return protocol.sendGetByName(id, name);
  }

  /**
   * get remote object property value
   *
   * @param index the property index
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<RTValue> get(int index) throws RTException {
    return protocol.sendGetById(id, index);
  }

  /**
   * invoke remote method and no return
   *
   * @param name the method name
   * @param arguments the function args
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<Void> send(String name, RTValue... arguments) throws RTException {
    return protocol.sendCallByNameAndNoReturns(id, name, arguments);
  }

  /**
   * invoke remote method and return value
   *
   * @param name the method name
   * @param arguments the function args
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<RTValue> sendReturns(String name, RTValue... arguments) throws RTException {
    return protocol.sendCallByNameAndReturns(id, name, arguments);
  }
}
