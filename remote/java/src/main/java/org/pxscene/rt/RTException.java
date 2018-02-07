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

/**
 * This is the base exception of the Client exception.
 */
public class RTException extends Exception {

  /**
   * create new RTException
   */
  public RTException() {
  }

  /**
   * create new RTException with message
   *
   * @param message the message
   */
  public RTException(String message) {
    super(message);
  }

  /**
   * create new RTException with message and cause
   *
   * @param message the exception message
   * @param cause the cause exception
   */
  public RTException(String message, Throwable cause) {
    super(message, cause);
  }

  /**
   * create new RTException with  cause
   *
   * @param cause the cause exception
   */
  public RTException(Throwable cause) {
    super(cause);
  }
}
