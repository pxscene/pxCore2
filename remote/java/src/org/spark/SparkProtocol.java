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

import org.spark.messages.SparkMessageGetPropertyByNameRequest;
import org.spark.messages.SparkMessageGetPropertyByNameResponse;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.StringReader;
import java.nio.charset.Charset;
import java.util.function.BiConsumer;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Map;
import java.util.HashMap;
import java.util.UUID;
import java.util.concurrent.Future;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

class SparkProtocol implements Runnable {
  private static final Logger log = Logger.getLogger(SparkProtocol.class.getName());

  private static class SparkCallContext {
    private BiConsumer<JsonObject, SparkFuture> m_closure;
    private SparkFuture m_future;



    public SparkCallContext(SparkFuture future, BiConsumer<JsonObject, SparkFuture> closure) {
      m_closure = closure;
      m_future = future;
    }

    public void complete(JsonObject obj) {
      if (m_closure != null) {
        m_closure.accept(obj, m_future);
      }
    }
  }

  private Map<String, SparkCallContext> m_futures = new HashMap<>();
  private SparkTransport m_transport;
  private boolean m_running;
  private Thread m_thread;
  private Charset m_charset = Charset.forName("UTF-8");
  ;

  private final Lock m_lock = new ReentrantLock();

  public SparkProtocol(SparkTransport transport) throws SparkException {
    m_transport = transport;
    m_transport.open();
    m_running = true;
    m_thread = new Thread(this);
    m_thread.start();
  }

  public Future<SparkValue> sendGetByName(String objectId, String name) throws SparkException {
    if (log.isLoggable(Level.INFO))
      log.info("sendGetByName objectId:" + objectId + " name:" + name);

    String correlationKey = SparkProtocol.newCorrelationKey();
    SparkFuture<SparkValue> future = new SparkFuture<SparkValue>(correlationKey);
    SparkCallContext context = new SparkCallContext(future, (json, closure) -> {
      try {
        SparkMessageGetPropertyByNameResponse res = SparkMessageGetPropertyByNameResponse.fromJson(json);
        SparkStatus status = res.getStatus();
        if (status.isOk()) {
          closure.setResponse(res.getValue());
        } else {
          closure.setFailed(status);
        }
      } catch (Exception err) {
        log.log(Level.WARNING, "error updating status of Future", err);
      }
    });

    put(correlationKey, context);

    SparkMessageGetPropertyByNameRequest m = new SparkMessageGetPropertyByNameRequest();
    m.setObjectId(objectId);
    m.setPropertyName(name);
    m.setCorrelationKey(correlationKey);

    String s = m.toString();
    if (log.isLoggable(Level.INFO))
      log.info("send:" + s);
    m_transport.send(s.getBytes(m_charset));

    return future;
  }

  public Future<SparkValue> sendGetById(String objectId, int index) throws SparkException {
    return null;
  }

  public Future<Void> sendSetById(String objectId, int index, SparkValue value) throws SparkException {
    return null;
  }

  public Future<Void> sendSetByName(String objectId, String name, SparkValue value) throws SparkException {
    return null;
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
        log.info("got:" + buff.length);
        String s = new String(buff, m_charset);
        log.info("read:" + s);

        JsonReader reader = null;

        try {
          reader = Json.createReader(new StringReader(s));
          JsonObject obj = reader.readObject();

          if (!obj.containsKey("correlation.key"))
            throw new SparkException("message missing correlation key");

          String correlationKey = obj.getString("correlation.key");
          SparkCallContext context = get(correlationKey);
          context.complete(obj);
        } finally {
          if (reader != null)
            reader.close();
        }

      } catch (SparkException err) {
        log.log(Level.WARNING, "error dispatching incomgin messages", err);
      }
    }
  }
}
