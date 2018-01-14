package org.spark;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public class SparkTestClient {
  public static void main(String[] args) throws InterruptedException {
    String host = "127.0.0.1";
    int port = Integer.parseInt(args[0]);

    try {

      SparkClientConnection con = SparkClientConnection.createTCPClientConnection(host, port);
      SparkObject obj = con.getObjectProxy("some_name");

      try {
        Future<SparkValue> f = obj.get("prop");

        // future blocks until operation completes, also supports timed-wait
        SparkValue v = f.get();
        // SparkValue v = f.get(1000, TimeUnit.MILLISECONDS);

        System.out.println("prop:" + v.toString());

      } catch (ExecutionException ee) {
        ee.printStackTrace();
      }
    } catch (SparkException err) {
      err.printStackTrace();
    }
  }
}
