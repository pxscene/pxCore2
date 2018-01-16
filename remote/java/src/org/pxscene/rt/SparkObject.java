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

public interface SparkObject {

  /**
   *
   * @param name
   * @return
   * @throws SparkException
   */
  Future<Void> set(String name, SparkValue value) throws SparkException;

  /**
   *
   * @param index
   * @param value
   * @return
   * @throws SparkException
   */
  Future<Void> set(int index, SparkValue value) throws SparkException;

  /**
   *
   * @param name
   * @return
   * @throws SparkException
   */
  Future<SparkValue> get(String name) throws SparkException;

  /**
   *
   * @param index
   * @return
   * @throws SparkException
   */
  Future<SparkValue> get(int index) throws SparkException;

  /**
   *
   * @param name
   * @param arguments
   * @return
   * @throws SparkException
   */
  Future<Void> send(String name, SparkValue ... arguments) throws SparkException;

  /**
   *
   * @param name
   * @param arguments
   * @return
   * @throws SparkException
   */
  Future<SparkValue> sendReturns(String name, SparkValue ... arguments) throws SparkException;


  /**
   *
   * @return
   */
  String getId();
}
