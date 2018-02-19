package examples;

import java.net.InetAddress;
import java.net.URI;
import java.util.concurrent.ExecutionException;
import lombok.Getter;
import lombok.Setter;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTEnvironment;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTFunction;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTValueType;
import org.pxscene.rt.remote.RTRemoteConnectionManager;
import org.pxscene.rt.remote.RTRemoteMulticastResolver;

/**
 * test method
 */
public class MethodTest {

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(MethodTest.class);

  /**
   * the total test number
   */
  @Setter
  @Getter
  private int total = 0;

  /**
   * the succeed number
   */
  @Setter
  @Getter
  private int suceed = 0;


  public static void main(String[] args) throws Exception {

    RTEnvironment.init();
    RTRemoteMulticastResolver resolver = new RTRemoteMulticastResolver(
        InetAddress.getByName("224.10.10.12"),
        10004);

    URI uri = resolver.locateObject("host_object");
    RTObject obj = RTRemoteConnectionManager.getObjectProxy(uri);

    MethodTest methodTest = new MethodTest();

    while (true) {
      try {
        methodTest.setTotal(0);
        methodTest.setSuceed(0);

        // test no args passed, and return 10 , rtMethod1ArgAndReturn
        methodTest
            .checkMethod(obj, "method0AndReturn10",
                new RTValue(10, RTValueType.INT32), null);

        // test method passed two int and return the sum, rtMethod2ArgAndReturn
        RTValue a = new RTValue(123, RTValueType.INT32);
        RTValue b = new RTValue(12, RTValueType.INT32);
        methodTest
            .checkMethod(obj, "twoIntNumberSum", new RTValue(123 + 12, RTValueType.INT32), a,
                b);

        // test method passed two float and return the sum, rtMethod2ArgAndReturn
        RTValue fa = new RTValue(123.3f, RTValueType.FLOAT);
        RTValue fb = new RTValue(12.3f, RTValueType.FLOAT);
        methodTest.checkMethod(obj, "twoFloatNumberSum",
            new RTValue(123.3f + 12.3f, RTValueType.FLOAT), fa, fb);

        // test method that passed 1 arg and no return, rtMethod1ArgAndNoReturn
        methodTest
            .checkMethodNoReturn(obj, "method1IntAndNoReturn",
                new RTValue(11, RTValueType.INT32));

        // test method that passed RtFunction and invoke this function , rtMethod2ArgAndNoReturn
        RTFunction function = new RTFunction(rtValueList -> {
          logger.debug("function invoke by remote, args count = " + rtValueList.size());
          for (RTValue rtValue : rtValueList) {
            logger.debug(
                "value=" + rtValue.getValue() + ", type=" + rtValue.getType().toString());
          }
          logger.debug("function invoke by remote done");
        });

        methodTest
            .checkMethodNoReturn(obj, "method2FunctionAndNoReturn",
                new RTValue(function), new RTValue(10, RTValueType.INT32));

        logger.debug(String
            .format("========= %d of %d example succeed, %d failed.", methodTest.getSuceed(),
                methodTest.getTotal(), methodTest.getTotal() - methodTest.getSuceed()));

        logger.debug("test completed, next test will at 10s ...");
      } catch (Exception err) {
        err.printStackTrace();
      }
      Thread.sleep(1000 * 10);
    }
  }

  /**
   * check method invoke and check the return value
   *
   * @param rtObject the rtObject
   * @param methodName the method name
   * @param expectedValue the expected value
   * @param args the function args
   */
  private void checkMethod(RTObject rtObject, String methodName, RTValue expectedValue,
      RTValue... args) throws RTException, ExecutionException, InterruptedException {
    RTValue value = rtObject.sendReturns(methodName, args).get();
    total += 1;
    boolean result;
    if (expectedValue == null) {
      result = true;
    } else if (expectedValue.getType().equals(RTValueType.FLOAT)) {
      result = checkEqualsFloat(expectedValue.getValue(), value.getValue());
    } else {
      result = value.getValue().equals(expectedValue.getValue());
    }
    if (result) {
      suceed += 1;
    }
    logger.debug("test method " + methodName + " result = [" + result + "]");
  }

  /**
   * check method with no return
   *
   * @param rtObject the rtObject
   * @param methodName the method name
   * @param args the args method args
   */
  private void checkMethodNoReturn(RTObject rtObject, String methodName, RTValue... args)
      throws RTException, ExecutionException, InterruptedException {
    rtObject.send(methodName, args).get(); // don't need check return
    total += 1;
    suceed += 1;
    logger.debug("test method " + methodName + " result = [" + true + "]");
  }


  /**
   * check float equals
   *
   * @param v1 the value 1
   * @param v2 the value 2
   */
  private boolean checkEqualsFloat(Object v1, Object v2) {
    float eps = 0.001f;
    if (Math.abs((float) v1 - (float) v2) <= eps) {
      return true;
    }
    return false;
  }
}
