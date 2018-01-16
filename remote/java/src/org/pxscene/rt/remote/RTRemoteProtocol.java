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

import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTException;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameRequest;

import java.util.function.BiConsumer;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Map;
import java.util.HashMap;
import java.util.UUID;
import java.util.concurrent.Future;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

class RTRemoteProtocol implements Runnable {
  private static final Logger log = Logger.getLogger(RTRemoteProtocol.class.getName());
  private final Lock m_lock = new ReentrantLock();

  private RTRemoteSerializer m_serializer = new RTRemoteSerializer();
  private Map<String, SparkCallContext> m_futures = new HashMap<>();
  private RTRemoteTransport m_transport;
  private boolean m_running;
  private Thread m_thread;


  private static class SparkCallContext {
    private BiConsumer<RTRemoteMessage, RTRemoteFuture> m_closure;
    private RTRemoteFuture m_future;
    public SparkCallContext(RTRemoteFuture future, BiConsumer<RTRemoteMessage, RTRemoteFuture> closure) {
      m_closure = closure;
      m_future = future;
    }
    public void complete(RTRemoteMessage message) {
      if (m_closure != null) {
        m_closure.accept(message, m_future);
      }
    }
  }

  public RTRemoteProtocol(RTRemoteTransport transport) throws RTException {
    m_transport = transport;
    m_transport.open();
    m_running = true;
    m_thread = new Thread(this);
    m_thread.start();
  }

  public Future<RTValue> sendGetByName(String objectId, String name) throws RTException {
    String correlationKey = RTRemoteProtocol.newCorrelationKey();
    RTRemoteFuture<RTValue> future = new RTRemoteFuture<>(correlationKey);
    SparkCallContext context = new SparkCallContext(future, (message, closure) -> {
      try {
        RTMessageGetPropertyByNameResponse res = (RTMessageGetPropertyByNameResponse) message;
        closure.complete(res.getValue(), res.getStatus());
      } catch (Exception err) {
        log.log(Level.WARNING, "error updating status of Future", err);
      }
    });

    put(correlationKey, context);

    RTMessageGetPropertyByNameRequest m = new RTMessageGetPropertyByNameRequest();
    m.setObjectId(objectId);
    m.setPropertyName(name);
    m.setCorrelationKey(correlationKey);
    m_transport.send(m_serializer.toBytes(m));

    return future;
  }

  public Future<RTValue> sendGetById(String objectId, int index) throws RTException {
    throw new RTException("not implemented");
  }

  public Future<Void> sendSetById(String objectId, int index, RTValue value) throws RTException {
    throw new RTException("not implemented");
  }

  public Future<Void> sendSetByName(String objectId, String name, RTValue value) throws RTException {
    String correlationKey = RTRemoteProtocol.newCorrelationKey();
    RTRemoteFuture<Void> future = new RTRemoteFuture<>(correlationKey);

    SparkCallContext context = new SparkCallContext(future, (message, closure) -> {
      try {
        RTMessageGetPropertyByNameResponse res = (RTMessageGetPropertyByNameResponse) message;
        closure.complete(null, res.getStatus());
      } catch (Exception err) {
        log.log(Level.WARNING, "error updating status of Future", err);
      }
    });

    put(correlationKey, context);

    RTMessageSetPropertyByNameRequest m = new RTMessageSetPropertyByNameRequest();
    m.setObjectId(objectId);
    m.setPropertyName(name);
    m.setCorrelationKey(correlationKey);
    m.setValue(value);
    m_transport.send(m_serializer.toBytes(m));

    return future;
  }

  private void put(String correlationKey, SparkCallContext context) {
    m_lock.lock();
    try {
      m_futures.put(correlationKey, context);
    } finally {
      m_lock.unlock();
    }
  }

  private SparkCallContext get(String correlationKey) {
    SparkCallContext context = null;
    m_lock.lock();
    try {
      if (m_futures.containsKey(correlationKey)) {
        context = m_futures.remove(correlationKey);
      }
    } finally {
      m_lock.unlock();
    }
    return context;
  }

  private static String newCorrelationKey() {
    return UUID.randomUUID().toString();
  }

  public void run() {
    while (m_running) {
      try {
        byte[] buff = m_transport.recv();
        RTRemoteMessage message = m_serializer.fromBytes(buff, 0, buff.length);
        SparkCallContext context = get(message.getCorrelationKey());
        context.complete(message);
      } catch (RTException err) {
        log.log(Level.WARNING, "error dispatching incomgin messages", err);
      }
    }
  }
}
