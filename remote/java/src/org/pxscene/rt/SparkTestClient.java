package org.pxscene.rt;

import java.net.URI;
import java.util.concurrent.Future;

public class SparkTestClient {
  public static void main(String[] args) throws Exception {

    // tcp://127.0.0.1:123455/some_name
    // <scheme>://<host>:<port>/<object name>
    URI uri = new URI(args[0]);

    SparkObject obj = SparkConnectionManager.getObjectProxy(uri);

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
