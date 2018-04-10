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


import org.pxscene.rt.RTException;

/**
 * The interface of remote tranport.
 */
public interface RTRemoteTransport {

  /**
   * open transport
   *
   * @throws RTException @throws RTException if any other error occurred during operation
   */
  void open() throws RTException;

  /**
   * close transport
   *
   * @throws RTException if any other error occurred during operation
   */
  void close() throws RTException;

  /**
   * read bytes from remote
   *
   * @return the bytes
   * @throws RTException if any other error occurred during operation
   */
  byte[] recv() throws RTException;

  /**
   * send bytes to remote
   *
   * @param bytes the bytes
   * @throws RTException if any other error occurred during operation
   */
  void send(byte[] bytes) throws RTException;
}
