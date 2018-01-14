package org.spark;

import org.spark.SparkException;
import org.spark.SparkStatus;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;


// TODO: could pobably replace with java.util.concurrent.FutureTask<V>
class SparkFuture<V> implements Future<V> {
  private V m_value;
  private SparkException m_error;
  private boolean m_isCancelled;
  private boolean m_done;
  private String m_correlationKey;

  private final Lock m_lock = new ReentrantLock();
  private final Condition m_haveResponseCond = m_lock.newCondition();

  public SparkFuture(String correlationKey) {
    m_correlationKey = correlationKey;
  }

  public String getCorrelationKey() {
    return m_correlationKey;
  }

  public boolean cancel(boolean myInterruptIfRunning) {
    // TODO
    return false;
  }

  public boolean isCancelled() {
    boolean cancelled = false;
    m_lock.lock();
    cancelled = m_isCancelled;
    m_lock.unlock();
    return cancelled;
  }

  public boolean isDone() {
    boolean done = false;
    m_lock.lock();
    done = m_done;
    m_lock.unlock();
    return done;
  }

  public V get() throws InterruptedException, ExecutionException {
    V value = null;
    try {
      value = this.get(Long.MAX_VALUE, TimeUnit.DAYS);
    } catch (TimeoutException e) {
      m_lock.lock();
      m_done = true;
      m_lock.unlock();
      throw new RuntimeException("error waiting for response", e);
    }
    return value;
  }

  public V get(long timeout, TimeUnit unit) throws InterruptedException, ExecutionException, TimeoutException {
    V value = null;

    m_lock.lock();
    try {
      while (m_value == null && m_error == null)
        m_haveResponseCond.await(timeout, unit);
      value = m_value;
      m_done = true;
    } finally {
      m_lock.unlock();
    }

    if (value == null)
      throw new TimeoutException();

    if (m_error != null)
      throw new ExecutionException(m_error);

    return value;
  }

  public void setResponse(V value) {
    m_lock.lock();
    try {
      m_value = value;
      m_done = true;
      m_haveResponseCond.signal();
    } finally {
      m_lock.unlock();
    }
  }

  public void setFailed(SparkStatus status) {
    m_lock.lock();
    try {
      m_value = null;
      m_done = true;
      m_haveResponseCond.signal();
      m_error = new SparkException(status.toString());
    } finally {
      m_lock.unlock();
    }
  }
}
