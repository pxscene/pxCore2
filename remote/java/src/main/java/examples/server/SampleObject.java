package examples.server;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;
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

  private String name;

  private Float ffloat;


  private Boolean bbool;

  private Byte int8;

  private Short uint8;

  private Integer int32;

  private Long uint32;

  private Long int64;

  private BigInteger uint64;

  private Double ddouble;

  private String string;

  private Long vptr;

  private RTFunction onTick;

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

  public String getName() {
    return this.name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Float getFfloat() {
    return this.ffloat;
  }

  public void setFfloat(Float ffloat) {
    this.ffloat = ffloat;
  }

  public Boolean getBbool() {
    return this.bbool;
  }

  public void setBbool(Boolean bbool) {
    this.bbool = bbool;
  }

  public Byte getInt8() {
    return this.int8;
  }

  public void setInt8(Byte int8) {
    this.int8 = int8;
  }

  public Short getUint8() {
    return this.uint8;
  }

  public void setUint8(Short uint8) {
    this.uint8 = uint8;
  }

  public Integer getInt32() {
    return this.int32;
  }

  public void setInt32(Integer int32) {
    this.int32 = int32;
  }

  public Long getUint32() {
    return this.uint32;
  }

  public void setUint32(Long uint32) {
    this.uint32 = uint32;
  }

  public Long getInt64() {
    return this.int64;
  }

  public void setInt64(Long int64) {
    this.int64 = int64;
  }

  public BigInteger getUint64() {
    return this.uint64;
  }

  public void setUint64(BigInteger uint64) {
    this.uint64 = uint64;
  }

  public Double getDdouble() {
    return this.ddouble;
  }

  public void setDdouble(Double ddouble) {
    this.ddouble = ddouble;
  }

  public String getString() {
    return this.string;
  }

  public void setString(String string) {
    this.string = string;
  }

  public Long getVptr() {
    return this.vptr;
  }

  public void setVptr(Long vptr) {
    this.vptr = vptr;
  }

  public RTFunction getOnTick() {
    return this.onTick;
  }

  public void setOnTick(RTFunction onTick) {
    this.onTick = onTick;
  }

  public RTObject getObjvar() {
    return this.objvar;
  }

  public void setObjvar(RTObject objvar) {
    this.objvar = objvar;
  }
}
