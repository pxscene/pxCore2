package examples.server;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;
import lombok.Getter;
import lombok.Setter;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTFunction;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTValue;

/**
 *
 */
public class SampleObject {

  private static final Logger logger = Logger.getLogger(SampleObject.class);

  @Getter
  @Setter
  private String name;

  @Getter
  @Setter
  private Float ffloat;


  @Setter
  @Getter
  private Boolean bbool;

  @Setter
  @Getter
  private Short int8;

  @Setter
  @Getter
  private Short uint8;

  @Setter
  @Getter
  private Integer int32;

  @Setter
  @Getter
  private Long uint32;

  @Setter
  @Getter
  private Long int64;

  @Setter
  @Getter
  private BigInteger uint64;

  @Setter
  @Getter
  private Double ddouble;

  @Setter
  @Getter
  private String string;

  @Setter
  @Getter
  private Long vptr;

  @Getter
  @Setter
  private RTFunction onTick;

  @Setter
  @Getter
  private RTObject objvar;

  private Integer methodValue = 0;

  public Integer method1AndReturn(Integer in) {
    return in * 2;
  }

  public Integer method0AndReturn10() {
    return 10;
  }

  public Integer twoIntNumberSum(Integer in1, Integer in2) {
    return in1 + in2;
  }

  public Float twoFloatNumberSum(Float in1, Float in2) {
    return in1 + in2;
  }

  public void method1IntAndNoReturn(Integer in1) {
    methodValue = in1;
  }

  public void method2FunctionAndNoReturn(RTFunction rtFunction, Integer in1) {
    if (rtFunction != null && rtFunction.getListener() != null) {
      List<RTValue> rtValueList = new ArrayList<>();
      rtValueList.add(new RTValue(in1));
      rtValueList.add(new RTValue(methodValue));
      try {
        rtFunction.getListener().invoke(rtValueList);
      } catch (RTException e) {
        logger.error("invoke failed", e);
      }
    } else {
      logger.debug("rtFunction or listener is null");
    }
  }


  public void doTest() {
    logger.debug("do test..");
  }
}
