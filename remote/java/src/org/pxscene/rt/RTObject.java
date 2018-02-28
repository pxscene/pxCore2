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

import java.util.concurrent.Future;

/**
 * the interface of rtObject
 */
public interface RTObject {

  /**
   * set remote object property value
   *
   * @param name the property name
   * @param value the value
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<Void> set(String name, RTValue value) throws RTException;


  /**
   * set remote object property value
   *
   * @param index the property index
   * @param value the value
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<Void> set(int index, RTValue value) throws RTException;

  /**
   * get remote object property value
   *
   * @param name the property name
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<RTValue> get(String name) throws RTException;

  /**
   * get remote object property value
   *
   * @param index the property index
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<RTValue> get(int index) throws RTException;

  /**
   * invoke remote method and no return
   *
   * @param name the method name
   * @param arguments the function args
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<Void> send(String name, RTValue... arguments) throws RTException;

  /**
   * invoke remote method and return value
   *
   * @param name the method name
   * @param arguments the function args
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<RTValue> sendReturns(String name, RTValue... arguments) throws RTException;


  /**
   * get object id
   *
   * @return object id
   */
  String getId();
}
