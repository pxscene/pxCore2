package examples;

import java.net.InetAddress;
import java.net.URI;
import java.util.concurrent.ExecutionException;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTEnvironment;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTValueType;
import org.pxscene.rt.remote.RTRemoteConnectionManager;
import org.pxscene.rt.remote.RTRemoteMulticastResolver;

/**
 * thread test, found 4 object,  and create 4 thread for 4 objs to test.
 */
public class ThreadTest {


  public static void main(String[] args) throws Exception {

    RTEnvironment.init();
    RTRemoteMulticastResolver resolver = new RTRemoteMulticastResolver(
        InetAddress.getByName("224.10.10.12"),
        10004);

    URI uri = resolver.locateObject("host_object");
    RTObject obj1 = RTRemoteConnectionManager.getObjectProxy(uri);

    uri = resolver.locateObject("obj2");
    RTObject obj2 = RTRemoteConnectionManager.getObjectProxy(uri);

    uri = resolver.locateObject("obj3");
    RTObject obj3 = RTRemoteConnectionManager.getObjectProxy(uri);

    uri = resolver.locateObject("obj4");
    RTObject obj4 = RTRemoteConnectionManager.getObjectProxy(uri);

    while (true) {
      try {
        ObjectTest objectTest1 = new ObjectTest(obj1);
        ObjectTest objectTest2 = new ObjectTest(obj2);
        ObjectTest objectTest3 = new ObjectTest(obj3);
        ObjectTest objectTest4 = new ObjectTest(obj4);

        objectTest1.start();
        objectTest2.start();
        objectTest3.start();
        objectTest4.start();
      } catch (Exception e) {
        e.printStackTrace();
      }

      Thread.sleep(1000 * 10); // sleep 10s
    }
  }
}

class ObjectTest extends Thread {

  private static Logger logger = Logger.getLogger(ObjectTest.class);
  private RTObject rtObject;

  public ObjectTest(RTObject rtObject) {
    this.rtObject = rtObject;
  }

  @Override
  public void run() {
    try {
      Thread.sleep((long) (Math.random() * 1000.0));

      // do get set test
      doBasicTest(rtObject, RTValueType.INT32, getRandomInt(), "int32");
      doBasicTest(rtObject, RTValueType.INT8, getRandomShort(), "int8");
      doBasicTest(rtObject, RTValueType.INT64, getRandomLong(), "int64");
      doBasicTest(rtObject, RTValueType.STRING, "SampleString", "string");

      // do method test
      int ia = getRandomInt();
      int ib = getRandomInt();
      RTValue a = new RTValue(ia, RTValueType.INT32);
      RTValue b = new RTValue(ib, RTValueType.INT32);
      checkMethod(rtObject, "twoIntNumberSum", new RTValue(ia + ib, RTValueType.INT32), a,
          b);
    } catch (InterruptedException | ExecutionException | RTException e) {
      e.printStackTrace();
    }
  }

  /**
   * get random Integer value
   *
   * @return the Integer value
   */
  private Integer getRandomInt() {
    return (int) (Math.random() * 10000000.0);
  }

  /**
   * get random Short value
   *
   * @return the Short value
   */
  private Short getRandomShort() {
    return (short) (Math.random() * 150);
  }

  /**
   * get random Long value
   *
   * @return the Long value
   */
  private Long getRandomLong() {
    return (long) (Math.random() * 1239912391923L);
  }

  /**
   * log message
   *
   * @param value the log message
   */
  private void log(String value) {
    logger.debug("[" + this.rtObject.getId() + "] " + value);
  }


  /**
   * do basic test examples
   *
   * @param rtObject the rt remote Object
   * @param type the rtObject type
   * @param value the rtValue
   * @param propertiesName the properties name
   */
  private void doBasicTest(RTObject rtObject, RTValueType type, Object value,
      String propertiesName) throws RTException, ExecutionException, InterruptedException {
    rtObject.set(propertiesName, new RTValue(value, type)).get();
    Object newVal = rtObject.get(propertiesName).get().getValue();
    boolean result = value.equals(newVal);
    log("set request=" + value + ",returned=" + newVal + ",type=" + type.toString() + ",result=["
        + result + "]");
  }

  /**
   * check method test
   *
   * @param rtObject the rtObject
   * @param methodName the method name
   * @param expectedValue the exptected value
   * @param args the method args
   */
  private void checkMethod(RTObject rtObject, String methodName, RTValue expectedValue,
      RTValue... args) throws RTException, ExecutionException, InterruptedException {
    RTValue value = rtObject.sendReturns(methodName, args).get();
    boolean result;
    if (expectedValue == null) { // return void method
      result = true;
    } else {
      result = value.getValue().equals(expectedValue.getValue());
    }
    log("method " + methodName + " result = [" + result + "]");
  }
}
