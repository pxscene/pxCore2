package examples;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.URI;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
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
 * test all types, set type with new value and check get value equals old value.
 */
public class TypeTest {

  private static final Logger logger = Logger.getLogger(TypeTest.class);

  /**
   * the total examples
   */
  private int totalExamplesCount = 0;

  /**
   * the suceed examples
   */
  private int succeedExamplesCount = 0;

  public static void main(String[] args) throws Exception {

    RTEnvironment.init();
    RTRemoteMulticastResolver resolver = new RTRemoteMulticastResolver(
        InetAddress.getByName("224.10.10.12"),
        10004);
    URI uri = resolver.locateObject("host_object");
    RTObject obj = RTRemoteConnectionManager.getObjectProxy(uri);
    TypeTest rtRemoteTestClient = new TypeTest();
    while (true) {
      try {
        rtRemoteTestClient.setTotalExamplesCount(0);
        rtRemoteTestClient.setSucceedExamplesCount(0);
        rtRemoteTestClient.doFunctionTest(obj, "onTick");
        rtRemoteTestClient.doObjectTest(obj, "objvar");
        rtRemoteTestClient.testAllTypes(obj);
        logger.debug("test completed, next test will at 10s ...");
        Thread.sleep(1000 * 10);
      } catch (Exception err) {
        err.printStackTrace();
      }
    }
  }

  /**
   * test all basic type
   * float,bool,int8,uint8,int32,uint32,int64,uint64,double,string,void ptr
   */
  public void testAllTypes(RTObject rtObject)
      throws RTException, ExecutionException, InterruptedException {
    logger.debug("======== start test rtValue type =======");

    // float last 7 digit
    doBasicTest(rtObject, RTValueType.FLOAT, 12.0123123f, "ffloat");
    doBasicTest(rtObject, RTValueType.FLOAT, -123.8818f, "ffloat");
    doBasicTest(rtObject, RTValueType.FLOAT, 199123123.91f, "ffloat");

    //test bool
    doBasicTest(rtObject, RTValueType.BOOLEAN, true, "bbool");
    doBasicTest(rtObject, RTValueType.BOOLEAN, false, "bbool");

    // int8 range [-128,127]
    doBasicTest(rtObject, RTValueType.INT8, (byte) -128, "int8");
    doBasicTest(rtObject, RTValueType.INT8, (byte) 0, "int8");
    doBasicTest(rtObject, RTValueType.INT8, (byte) 127, "int8");

    //test uint8, the data range is[0,255]
    doBasicTest(rtObject, RTValueType.UINT8, (short) 0, "uint8");
    doBasicTest(rtObject, RTValueType.UINT8, (short) 255, "uint8");

    //test int32, range is 	[–2147483648 , 2147483647]
    doBasicTest(rtObject, RTValueType.INT32, -2147483648, "int32");
    doBasicTest(rtObject, RTValueType.INT32, 0, "int32");
    doBasicTest(rtObject, RTValueType.INT32, 123, "int32");
    doBasicTest(rtObject, RTValueType.INT32, 2147483647, "int32");

    // test uint32, range is [0 - 4,294,967,295]
    doBasicTest(rtObject, RTValueType.UINT32, 0L, "uint32");
    doBasicTest(rtObject, RTValueType.UINT32, 4294967295L, "uint32");
    doBasicTest(rtObject, RTValueType.UINT32, 123123L, "uint32");

    // test int64, range is [–9223372036854775808  9223372036854775807]
    doBasicTest(rtObject, RTValueType.INT64, -9223372036854775808L, "int64");
    doBasicTest(rtObject, RTValueType.INT64, 9223372036854775807L, "int64");
    doBasicTest(rtObject, RTValueType.INT64, 0L, "int64");
    doBasicTest(rtObject, RTValueType.INT64, 123123L, "int64");

    // test uint64, range is [0 - 18446744073709551615]
    doBasicTest(rtObject, RTValueType.UINT64, new BigInteger("18446744073709551615"), "uint64");
    doBasicTest(rtObject, RTValueType.UINT64, BigInteger.valueOf(0), "uint64");
    doBasicTest(rtObject, RTValueType.UINT64, BigInteger.valueOf(123123123), "uint64");
    doBasicTest(rtObject, RTValueType.UINT64, BigInteger.valueOf(123), "uint64");

    // test double
    doBasicTest(rtObject, RTValueType.DOUBLE, 1231.12312312312, "ddouble");
    doBasicTest(rtObject, RTValueType.DOUBLE, -1231.12312312312, "ddouble");
    doBasicTest(rtObject, RTValueType.DOUBLE, -0.12, "ddouble");

    // test string
    doBasicTest(rtObject, RTValueType.STRING,
        "implemented in both /Library/Java/JavaVirtualMachines/jdk1.8.0_40.jdk/Conten", "string");
    doBasicTest(rtObject, RTValueType.STRING,
        "{\"jsonKey\":\"values\"}", "string");
    doBasicTest(rtObject, RTValueType.STRING, "1", "string");

    // void ptr is a uint32 or uint64
    doBasicTest(rtObject, RTValueType.VOIDPTR, 723123231L, "vptr");
    doBasicTest(rtObject, RTValueType.VOIDPTR, 789892349L, "vptr");

    logger.debug(String
        .format("========= %d of %d example succeed, %d failed.", succeedExamplesCount,
            totalExamplesCount, totalExamplesCount - succeedExamplesCount));
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
    totalExamplesCount += 1;

    boolean result;
    if (type == RTValueType.FLOAT) {
      result = checkEqualsFloat(value, newVal);
    } else if (type == RTValueType.DOUBLE) {
      result = checkEqualsDouble(value, newVal);
    } else {
      result = value.equals(newVal);
    }
    if (result) {
      succeedExamplesCount += 1;
    }
    logger.debug(getTestResult(type.toString(), value, newVal, result));
  }


  /**
   * test function type
   */
  private void doFunctionTest(RTObject rtObject, String propertiesName)
      throws RTException, ExecutionException, InterruptedException {

    RTValue oldRtValue = new RTValue(new RTFunction(rtValueList -> {
      logger.debug("doFunctionTest test...");
      logger.debug(rtValueList);
    }));

    rtObject.set(propertiesName, oldRtValue).get();
    Future<RTValue> valueFuture = rtObject.get(propertiesName);
    RTValue rtValue = valueFuture.get();
    ((RTFunction) rtValue.getValue()).getListener().invoke(null);
    totalExamplesCount += 1;
    boolean result = checkEqualsFunction(oldRtValue, rtValue);
    if (result) {
      succeedExamplesCount += 1;
    }

    logger.debug(
        getTestResult(RTValueType.FUNCTION.toString(), oldRtValue.getValue(),
            rtValue, result));
  }

  /**
   * test object type
   */
  private void doObjectTest(RTObject rtObject, String propertiesName)
      throws RTException, ExecutionException, InterruptedException {
    TestObject oldObj = new TestObject("type test");
    RTValue oldRtValue = new RTValue(oldObj, RTValueType.OBJECT);
    rtObject.set(propertiesName, oldRtValue).get();
    RTValue newObj = rtObject.get(propertiesName).get();

    TestObject testObject = (TestObject) newObj.getValue();
    testObject.doTest();
    totalExamplesCount += 1;

    boolean result = testObject.getId().equals(oldObj.getId());
    if (result) {
      succeedExamplesCount += 1;
    }
    logger.debug(
        getTestResult(RTValueType.OBJECT.toString(), oldObj.getId(),
            testObject.getId(), result));
  }

  /**
   * check function is equals
   */
  private boolean checkEqualsFunction(RTValue v1, RTValue v2) {
    return ((RTFunction) v1.getValue()).getFunctionName()
        .equals(((RTFunction) v2.getValue()).getFunctionName());
  }

  /**
   * floating point values can be off by a little bit, so they may not report as exactly equal.
   * so i need use eps to check equal
   */
  private boolean checkEqualsFloat(Object v1, Object v2) {
    float eps = 0.001f;
    if (Math.abs((float) v1 - (float) v2) < eps) {
      return true;
    }
    return false;
  }

  /**
   * double  values can be off by a little bit, so they may not report as exactly equal.
   */
  private boolean checkEqualsDouble(Object v1, Object v2) {
    double eps = 0.0001;
    if (Math.abs((double) v1 - (double) v2) < eps) {
      return true;
    }
    return false;
  }

  /**
   * get test result to debug
   *
   * @param type the test type
   * @param old the old value (set value)
   * @param newValue the new value (get from remote)
   * @param result the equals result
   */
  private String getTestResult(String type, Object old, Object newValue, boolean result) {
    return type + " test => set val = " + old + ", rpc result = " + newValue
        + ", passed = ["
        + result + "]";
  }

  public int getTotalExamplesCount() {
    return this.totalExamplesCount;
  }

  public int getSucceedExamplesCount() {
    return this.succeedExamplesCount;
  }

  public void setTotalExamplesCount(int totalExamplesCount) {
    this.totalExamplesCount = totalExamplesCount;
  }

  public void setSucceedExamplesCount(int succeedExamplesCount) {
    this.succeedExamplesCount = succeedExamplesCount;
  }
}
