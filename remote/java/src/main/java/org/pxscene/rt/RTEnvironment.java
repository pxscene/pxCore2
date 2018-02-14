package org.pxscene.rt;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import lombok.Getter;
import lombok.Setter;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.pxscene.rt.remote.RTConst;

/**
 * the rt environment
 */
public class RTEnvironment {


  /**
   * the static rt function map, used to store local function, thread safe
   */
  @Getter
  @Setter
  private static final Map<String, RTFunction> rtFunctionMap = new ConcurrentHashMap<>();

  /**
   * the static rt local object map, used to store local object, thread safe
   */
  @Getter
  @Setter
  private static final Map<String, RTObject> rtObjectMap = new ConcurrentHashMap<>();

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTEnvironment.class);

  /**
   * the rt program run mode, server or client
   */
  @Getter
  @Setter
  private static String runMode = RTConst.CLIENT_MODE;

  /**
   * init logger by property file
   */
  public static void init() {
    PropertyConfigurator
        .configure(RTEnvironment.class.getClassLoader().getResource("log4j.properties"));
    logger.debug("Logger init succeed.");
  }

  /**
   * check this program run is server or client mode
   */
  public static boolean isServerMode() {
    return runMode.equals(RTConst.SERVER_MODE);
  }
}
