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

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.Future;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.BiConsumer;
import javax.json.Json;
import javax.json.JsonObjectBuilder;
import lombok.Getter;
import lombok.Setter;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTEnvironment;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTFunction;
import org.pxscene.rt.RTStatusCode;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.remote.messages.RTMessageCallMethodRequest;
import org.pxscene.rt.remote.messages.RTMessageCallMethodResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameRequest;

/**
 * the rt remote protocol entity.
 */
public class RTRemoteProtocol implements Runnable {

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTRemoteProtocol.class);

  /**
   * the lock
   */
  private final Lock lock = new ReentrantLock();

  /**
   * the rt remote serializer
   */
  private RTRemoteSerializer serializer = new RTRemoteSerializer();

  /**
   * the futures with call context map
   */
  private Map<String, CallContext> futures = new HashMap<>();

  /**
   * the connection transport
   */
  @Getter
  private RTRemoteTransport transport;


  /**
   * the rt remote server
   */
  @Setter
  private RTRemoteServer rtRemoteServer;

  /**
   * if the protocol running
   */
  private boolean running;


  /**
   * create new protocol
   *
   * @param transport the connection transport
   * @param transportOpened is the transport already opened
   * @throws RTException if any other error occurred during operation
   */
  RTRemoteProtocol(RTRemoteTransport transport, boolean transportOpened) throws RTException {
    this.transport = transport;
    if (!transportOpened) {
      this.transport.open();
    }
    running = true;
    new Thread(this).start();
  }


  /**
   * return new correlation key
   *
   * @return the key
   */
  private static String newCorrelationKey() {
    return UUID.randomUUID().toString();
  }

  /**
   * set remote object by name
   *
   * @param objectId the object id
   * @param name the properties name
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<RTValue> sendGetByName(String objectId, String name) throws RTException {
    String correlationKey = RTRemoteProtocol.newCorrelationKey();
    RTRemoteFuture<RTValue> future = new RTRemoteFuture<>(correlationKey);
    CallContext context = new CallContext(future, (message, closure) -> {
      try {
        RTMessageGetPropertyByNameResponse res = (RTMessageGetPropertyByNameResponse) message;
        closure.complete(res.getValue(), res.getStatus());
      } catch (Exception err) {
        logger.warn("error updating status of Future", err);
      }
    });
    put(correlationKey, context);
    RTMessageGetPropertyByNameRequest m = new RTMessageGetPropertyByNameRequest();
    m.setObjectId(objectId);
    m.setPropertyName(name);
    m.setCorrelationKey(correlationKey);
    transport.send(serializer.toBytes(m));
    return future;
  }

  /**
   * send get by property by id request
   * TODO not implemented
   *
   * @param objectId the object id
   * @param index the property index
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<RTValue> sendGetById(String objectId, int index) throws RTException {
    throw new RTException("not implemented");
  }

  /**
   * send set by property by id request
   * TODO not implemented
   *
   * @param objectId the object id
   * @param index the property index
   * @param value the new value
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<Void> sendSetById(String objectId, int index, RTValue value) throws RTException {
    throw new RTException("not implemented");
  }


  /**
   * send call by name and no return
   *
   * @param objectId the object id
   * @param methodName the method data
   * @param arguments the rtvalues arguments
   * @return the future task
   * @throws RTException if any other error occurred during operation
   **/
  public Future<Void> sendCallByNameAndNoReturns(String objectId, String methodName,
      RTValue... arguments) throws RTException {

    String correlationKey = RTRemoteProtocol.newCorrelationKey();
    RTRemoteFuture<Void> future = new RTRemoteFuture<>(correlationKey);
    CallContext context = new CallContext(future, (message, closure) -> {
      try {
        RTMessageCallMethodResponse res = (RTMessageCallMethodResponse) message;
        closure.complete(null, res.getStatus());
      } catch (Exception err) {
        logger.warn("error updating status of Future", err);
      }
    });
    sendCallByNameData(context, objectId, methodName, correlationKey, arguments);
    return future;
  }

  /**
   * send call by name and return values
   *
   * @param objectId the remote object id
   * @param methodName the method name
   * @param arguments the rtvalue arguments
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  Future<RTValue> sendCallByNameAndReturns(String objectId, String methodName,
      RTValue... arguments) throws RTException {
    String correlationKey = RTRemoteProtocol.newCorrelationKey();
    RTRemoteFuture<RTValue> future = new RTRemoteFuture<>(correlationKey);

    CallContext context = new CallContext(future, (message, closure) -> {
      try {
        RTMessageCallMethodResponse res = (RTMessageCallMethodResponse) message;
        closure.complete(res.getValue(), res.getStatus());
      } catch (Exception err) {
        logger.warn("error updating status of Future", err);
      }
    });
    sendCallByNameData(context, objectId, methodName, correlationKey, arguments);
    return future;
  }

  /**
   * build method request data and send to remote used for return or no return.
   *
   * @param context the call context
   * @param objectId the object id
   * @param methodName the method name
   * @param correlationKey the correlation key
   * @param arguments the rt values arguments
   * @return void
   * @throws RTException if any other error occurred during operation
   */
  private void sendCallByNameData(CallContext context, String objectId, String methodName,
      String correlationKey,
      RTValue... arguments)
      throws RTException {
    put(correlationKey, context);

    RTMessageCallMethodRequest request = new RTMessageCallMethodRequest();
    request.setCorrelationKey(correlationKey);
    request.setObjectId(objectId);
    request.setMethodName(methodName);
    if (arguments != null) {
      request.setFunctionArgs(Arrays.asList(arguments));
    }
    byte[] bytes = serializer.toBytes(request);
    transport.send(bytes);
  }

  /**
   * send set by property name request
   *
   * @param objectId the remote object id
   * @param name the properties name
   * @param value the new value
   * @return the future task
   * @throws RTException if any other error occurred during operation
   */
  public Future<Void> sendSetByName(String objectId, String name, RTValue value)
      throws RTException {
    String correlationKey = RTRemoteProtocol.newCorrelationKey();
    RTRemoteFuture<Void> future = new RTRemoteFuture<>(correlationKey);

    CallContext context = new CallContext(future, (message, closure) -> {
      try {
        RTMessageGetPropertyByNameResponse res = (RTMessageGetPropertyByNameResponse) message;
        closure.complete(null, res.getStatus());
      } catch (Exception err) {
        logger.warn("error updating status of Future", err);
      }
    });

    put(correlationKey, context);
    RTMessageSetPropertyByNameRequest m = new RTMessageSetPropertyByNameRequest();
    m.setObjectId(objectId);
    m.setPropertyName(name);
    m.setCorrelationKey(correlationKey);
    m.setValue(value);
    transport.send(serializer.toBytes(m));

    return future;
  }


  /**
   * put call context to map
   *
   * @param correlationKey the key
   * @param context the call context
   */
  private void put(String correlationKey, CallContext context) {
    lock.lock();
    try {
      futures.put(correlationKey, context);
    } finally {
      lock.unlock();
    }
  }

  /**
   * get call context by key
   *
   * @param correlationKey the key
   * @return the call context
   */
  private CallContext get(String correlationKey) {
    CallContext context = null;
    lock.lock();
    try {
      if (futures.containsKey(correlationKey)) {
        context = futures.remove(correlationKey);
      }
    } finally {
      lock.unlock();
    }
    return context;
  }

  /**
   * the new thread run
   */
  public void run() {
    while (running) {
      try {
        byte[] buff = transport.recv();
        RTRemoteMessage message = serializer.fromBytes(buff, 0, buff.length);
        CallContext context = get(message.getCorrelationKey());
        if (context != null) {
          context.complete(message);
        } else {
          if (RTEnvironment.isServerMode()) {
            processMessageInServerMode(message);
          } else {
            processMessageInClientMode(message);
          }
        }
      } catch (RTException err) {
        logger.error("error dispatching incomgin messages", err);
      }
    }
  }

  private void processMessageInClientMode(RTRemoteMessage message) throws RTException {
    if (message.getMessageType().equals(RTRemoteMessageType.KEEP_ALIVE_REQUEST)) {
      // TODO this mean server send keep alive request to client, simple continue for now
    } else if (message.getMessageType().equals(RTRemoteMessageType.METHOD_CALL_REQUEST)) {
      // this mean server invoke client function,and these is no CallContext
      // let do this in this thread for now
      RTMessageCallMethodRequest request = (RTMessageCallMethodRequest) message;
      if (request.getRtFunction() != null
          && request.getRtFunction().getValue() != null) {
        RTFunction rtFunction = ((RTFunction) request.getRtFunction().getValue());
        if (rtFunction.getListener() != null) {
          rtFunction.getListener().invoke(request.getFunctionArgs());
        }
      }
      sendCallResponse(request.getCorrelationKey()); // send call response to server
    } else {
      logger.error("unexpected message " + message);
    }
  }

  private void processMessageInServerMode(RTRemoteMessage message) {
    rtRemoteServer.getMessageQueue().add(new RTRemoteTask(this, message));
  }

  /**
   * when server send call request to client, client need return response to server
   *
   * @param correlationKey the call request correlation key
   * @throws RTException if any other error occurred during operation
   */
  public void sendCallResponse(String correlationKey) throws RTException {
    JsonObjectBuilder builder = Json.createObjectBuilder();
    JsonObjectBuilder typeBuilder = Json.createObjectBuilder();
    typeBuilder.add(RTConst.TYPE, RTStatusCode.OK.getCode());
    builder.add(RTConst.MESSAGE_TYPE, RTRemoteMessageType.METHOD_CALL_RESPONSE.toString());
    builder.add(RTConst.CORRELATION_KEY, correlationKey);
    builder.add(RTConst.STATUS_CODE, RTStatusCode.OK.getCode());
    builder.add(RTConst.FUNCTION_RETURN_VALUE, typeBuilder.build());
    transport.send(builder.build().toString().getBytes(RTRemoteSerializer.CHARSET));
  }

  /**
   * the call context class
   */
  private static class CallContext {

    /**
     * the bi consumer
     */
    private BiConsumer<RTRemoteMessage, RTRemoteFuture> closure;

    /**
     * the future
     */
    private RTRemoteFuture future;

    /**
     * create new bi consumer
     *
     * @param future the futrue task
     * @param closure the bi consumer
     */
    CallContext(RTRemoteFuture future, BiConsumer<RTRemoteMessage, RTRemoteFuture> closure) {
      this.closure = closure;
      this.future = future;
    }

    /**
     * complete context
     *
     * @param message the rt remote message
     */
    void complete(RTRemoteMessage message) {
      if (closure != null) {
        closure.accept(message, future);
      }
    }
  }
}
