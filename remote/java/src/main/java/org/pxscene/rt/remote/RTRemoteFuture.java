package org.pxscene.rt.remote;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTStatus;

/**
 * The RTRemoteFuture class.
 * TODO: could pobably replace with java.util.concurrent.FutureTask<V>
 *
 * @param <V> the value type
 */
class RTRemoteFuture<V> implements Future<V> {

  /**
   * the private lock
   */
  private final Lock lock = new ReentrantLock();

  /**
   * the lock condition
   */
  private final Condition haveResponseCond = lock.newCondition();

  /**
   * the dest value
   */
  private V value;

  /**
   * the exception
   */
  private RTException error;

  /**
   * is the future cancelled
   */
  private boolean isCancelled;

  /**
   * is the future ended
   */
  private boolean done;

  /**
   * the correlation key
   */
  private String correlationKey;

  /**
   * the status
   */
  private RTStatus status;

  /**
   * the entity constructor with key.
   *
   * @param correlationKey the correlation key
   */
  public RTRemoteFuture(String correlationKey) {
    this.correlationKey = correlationKey;
  }


  /**
   * cancel future task
   *
   * @param myInterruptIfRunning if that my interrupt
   * @return the result
   */
  public boolean cancel(boolean myInterruptIfRunning) {
    // TODO: not to api spec.
    // https://docs.oracle.com/javase/7/docs/api/java/util/concurrent/Future.html
    lock.lock();
    done = true;
    isCancelled = true;
    lock.unlock();
    return false;
  }

  /**
   * is the future cancelled
   *
   * @return the result
   */
  public boolean isCancelled() {
    boolean cancelled;
    lock.lock();
    cancelled = isCancelled;
    lock.unlock();
    return cancelled;
  }

  /**
   * is the future ended
   */
  public boolean isDone() {
    boolean done;
    lock.lock();
    done = this.done;
    lock.unlock();
    return done;
  }

  /**
   * get the future dest value
   *
   * @return the dest value
   * @throws InterruptedException if user interrupt occurred during operation
   * @throws ExecutionException if any other error occurred during operation
   */
  public V get() throws InterruptedException, ExecutionException {
    V value;
    try {
      value = this.get(Long.MAX_VALUE, TimeUnit.DAYS);
    } catch (TimeoutException e) {
      lock.lock();
      done = true;
      lock.unlock();
      throw new ExecutionException("error waiting for response", e);
    }
    return value;
  }

  /**
   * get the future dest value
   *
   * @param timeout the time
   * @param unit the time unit
   * @return the dest value
   * @throws InterruptedException if user interrupt occurred during operation
   * @throws ExecutionException if any other error occurred during operation
   * @throws TimeoutException if timeout occurred during operation
   */
  public V get(long timeout, TimeUnit unit)
      throws InterruptedException, ExecutionException, TimeoutException {
    V value = null;

    lock.lock();
    try {
      while (!done) {
        haveResponseCond.await(timeout, unit);
      }
      value = this.value;
    } finally {
      lock.unlock();
    }

    if (error != null) {
      throw new ExecutionException(error);
    }

    return value;
  }

  /**
   * complete a future task
   *
   * @param value the dest value
   * @param status the status
   */
  public void complete(V value, RTStatus status) {
    lock.lock();
    try {
      this.value = value;
      done = true;
      this.status = status;
      if (!this.status.isOk()) {
        error = new RTException(status.toString());
      }
      haveResponseCond.signal();
    } finally {
      lock.unlock();
    }
  }

  public String getCorrelationKey() {
    return this.correlationKey;
  }
}
