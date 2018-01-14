package org.spark;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public class SparkTestClient {
  public static void main(String[] args) throws Exception {
    String host = "127.0.0.1";
    int port = Integer.parseInt(args[0]);

    SparkClientConnection con = SparkClientConnection.createTCPClientConnection(host, port);
    SparkObject obj = con.getObjectProxy("some_name");

    int n = 10;

    while (true) {
      try {
        // do get
        // future blocks until operation completes, also supports timed-wait
        // SparkValue v = f.get(1000, TimeUnit.MILLISECONDS);
        Future<SparkValue> f = obj.get("prop");
        SparkValue v = f.get();
        System.out.println("get n:" + v);

        Thread.sleep(1000);

        // do set
        Future<Void> f2 = obj.set("prop", new SparkValue(n++));
        f2.get();
        System.out.println("set");


      } catch (Exception err) {
        err.printStackTrace();
      }
    }
  }
}
