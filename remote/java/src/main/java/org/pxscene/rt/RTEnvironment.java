package org.pxscene.rt;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 * the rt environment
 */
public class RTEnvironment {

  /**
   * loop to find object time if didn't found
   */
  public static long FIND_OBBJECT_TIME = 1000;

  /**
   * the logger intance
   */
  private static Logger logger = Logger.getLogger(RTEnvironment.class);

  /**
   * init logger by property file
   */
  public static void init() {
    PropertyConfigurator
        .configure(RTEnvironment.class.getClassLoader().getResource("log4j.properties"));
    logger.debug("Logger init succeed.");
  }
}
