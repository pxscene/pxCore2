package examples;

import java.util.UUID;
import java.util.concurrent.Future;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTValue;

/**
 * the test object to test setter/getter
 */
public class TestObject implements RTObject {

  private Logger logger = Logger.getLogger(TestObject.class);

  private String property;
  private String objectId;


  public TestObject(String property) {
    this.property = property;
    this.objectId = "obj://" + UUID.randomUUID().toString();
  }

  public void doTest() {
    logger.debug("test of local object" + property);
  }

  @Override
  public Future<Void> set(String name, RTValue value) throws RTException {
    return null;
  }

  @Override
  public Future<Void> set(int index, RTValue value) throws RTException {
    return null;
  }

  @Override
  public Future<RTValue> get(String name) throws RTException {
    return null;
  }

  @Override
  public Future<RTValue> get(int index) throws RTException {
    return null;
  }

  @Override
  public Future<Void> send(String name, RTValue... arguments) throws RTException {
    return null;
  }

  @Override
  public Future<RTValue> sendReturns(String name, RTValue... arguments) throws RTException {
    return null;
  }

  @Override
  public String getId() {
    return this.objectId;
  }
}
