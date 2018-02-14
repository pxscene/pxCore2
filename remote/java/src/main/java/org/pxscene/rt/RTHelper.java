package org.pxscene.rt;


import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.ServerSocket;
import java.util.List;
import org.apache.log4j.Logger;
import org.pxscene.rt.remote.RTRemoteProtocol;
import org.pxscene.rt.remote.messages.RTMessageCallMethodRequest;
import org.pxscene.rt.remote.messages.RTMessageCallMethodResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameResponse;

/**
 * the remote helper methods
 */
public final class RTHelper {

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTHelper.class);

  /**
   * find a free tcp port to use
   *
   * @return the port
   * @throws RTException it will raise a error if cannot found a port
   */
  public static int findRandomOpenPortOnAllLocalInterfaces() throws RTException {
    try {
      ServerSocket socket = new ServerSocket(0);
      int port = socket.getLocalPort();
      socket.close();
      return port;
    } catch (IOException e) {
      logger.error("cannot get random rpc port, please check your network interface", e);
      throw new RTException(e);
    }
  }

  /**
   * first capitalize a string
   *
   * @param str the word string
   * @return the word string with first capitalize
   */
  public static String firstCapitalize(String str) {
    return str.substring(0, 1).toUpperCase() + str.substring(1);
  }

  /**
   * dump object with fields and methods
   *
   * @param name the object name
   * @param object the object value
   */
  public static void dumpObject(String name, Object object) {
    Class clazz = object.getClass();
    for (Field field : clazz.getFields()) {
      logger.debug("object name=" + name + ",field = " + field.getName()
          + ", type =" + field.getType().getName());
    }
    for (Method method : clazz.getMethods()) {
      logger.debug("object name=" + name + ", method = " + method.getName() + ", return type ="
          + method.getReturnType().getName());
    }
  }


  /**
   * get value by property name
   * first find object field or method get*
   *
   * @param object the object value
   * @param getRequest the request message
   * @return the get response with value
   */
  public static RTMessageGetPropertyByNameResponse getPropertyByName(
      Object object, RTMessageGetPropertyByNameRequest getRequest) {

    Class clazz = object.getClass();
    String propertyName = getRequest.getPropertyName();
    RTStatus rtStatus = new RTStatus(RTStatusCode.OK);
    RTMessageGetPropertyByNameResponse getResponse = new RTMessageGetPropertyByNameResponse();
    getResponse.setObjectId(getRequest.getObjectId());
    getResponse.setStatus(rtStatus);
    getResponse.setCorrelationKey(getRequest.getCorrelationKey());

    Field field = null;
    try {
      field = clazz.getField(propertyName);
    } catch (NoSuchFieldException ignored) {
    }

    Method method = null;
    try {
      method = clazz.getMethod("get" + firstCapitalize(propertyName));
    } catch (NoSuchMethodException ignored) {
    }

    if (field == null && method == null) { // field or method not exist
      logger.error("cannot found property name = " + propertyName);
      rtStatus.setCode(RTStatusCode.PROPERTY_NOT_FOUND);
    }

    try {
      if (field != null) {
        Object value = field.get(object);
        getResponse.setValue(new RTValue(value));
      } else {
        Object value = method.invoke(object);
        if (value != null && value instanceof RTObject) {
          getResponse.setValue(new RTValue(value, RTValueType.OBJECT));
        } else {
          getResponse.setValue(new RTValue(value));
        }
      }
    } catch (IllegalAccessException e) {
      logger.error("set property failed", e);
      rtStatus.setCode(RTStatusCode.TYPE_MISMATCH);
    } catch (InvocationTargetException e) {
      logger.error("invoke failed", e);
      rtStatus.setCode(RTStatusCode.TYPE_MISMATCH);
    }
    return getResponse;
  }


  /**
   * invoke object method by method name
   *
   * @param object the object value
   * @param request the invoke request message
   * @return the response with value
   */
  public static RTMessageCallMethodResponse invokeMethod(Object object,
      RTMessageCallMethodRequest request) {
    RTMessageCallMethodResponse response = new RTMessageCallMethodResponse();
    response.setCorrelationKey(request.getCorrelationKey());
    response.setStatus(new RTStatus(RTStatusCode.OK));

    Class clazz = object.getClass();
    List<RTValue> args = request.getFunctionArgs();
    Method destMethod = null;
    int methodParamsCount = args == null ? 0 : args.size();

    // because of java method need signature to found and rtRemote call request didn't have signatures
    // so i need for each methods to find first method by the method name and count, and ignore java overloading
    // TODO note that method overloading ignored for now, maybe remote need update protocol
    Method[] methods = clazz.getMethods();
    for (Method method : methods) {
      if (method.getName().equals(request.getMethodName())
          && methodParamsCount == method.getParameterCount()) {
        destMethod = method;
        break;
      }
    }

    if (destMethod == null) {
      response.setStatus(new RTStatus(RTStatusCode.PROPERTY_NOT_FOUND));
    } else {
      try {
        Object[] callArgs = new Object[methodParamsCount];
        if (args != null && args.size() > 0) {
          for (int i = 0; i < methodParamsCount; i += 1) {
            callArgs[i] = args.get(i).getValue();
          }
        }

        Object returnValue = destMethod.invoke(object, callArgs);  // invoke method
        if (returnValue != null) {
          response.setValue(new RTValue(returnValue)); // set value
        }
      } catch (Exception e) {
        logger.error("invoke method " + request.getMethodName() + " failed", e);
        response.setStatus(new RTStatus(RTStatusCode.INVALID_ARGUMENT));
      }
    }
    return response;
  }

  /**
   * set object value by property name
   *
   * @param object the object value
   * @param request the set request message
   * @return the response
   */
  public static RTMessageSetPropertyByNameResponse setPropertyByName(
      Object object, RTMessageSetPropertyByNameRequest request) {

    RTMessageSetPropertyByNameResponse setResponse = new RTMessageSetPropertyByNameResponse();
    setResponse.setCorrelationKey(request.getCorrelationKey());
    setResponse.setStatusCode(RTStatusCode.UNKNOWN);
    setResponse.setObjectId(request.getObjectId());

    Class clazz = object.getClass();
    String propertyName = request.getPropertyName();
    Object value = request.getValue().getValue();

    Field field = null;
    try {
      field = clazz.getField(propertyName);
    } catch (NoSuchFieldException ignored) {
    }

    Method method = null;
    try {
      Class setType = value.getClass();
      if (request.getValue().getType().equals(RTValueType.FUNCTION)) {
        setType = RTFunction.class;
      } else if (request.getValue().getType().equals(RTValueType.OBJECT)) {
        setType = RTObject.class;
      }
      method = clazz.getMethod("set" + firstCapitalize(propertyName), setType);
    } catch (NoSuchMethodException ignored) {
    }

    if (field == null && method == null) {
      logger.error("cannot found property name = " + propertyName);
      setResponse.setStatusCode(RTStatusCode.PROPERTY_NOT_FOUND);
    } else {
      try {
        if (field != null) {
          field.set(propertyName, value);
          setResponse.setStatusCode(RTStatusCode.OK);
        } else {
          method.invoke(object, value);
          setResponse.setStatusCode(RTStatusCode.OK);
        }
      } catch (Exception e) {
        logger.error("set property failed", e);
        setResponse.setStatusCode(RTStatusCode.TYPE_MISMATCH);
      }
    }
    return setResponse;
  }


  /**
   * create new listener if function listener is null
   *
   * @param protocol the protocol that send call request
   * @param oldFunction the old function
   * @return the newFunction
   */
  public static RTFunction updateListenerForRTFuction(RTRemoteProtocol protocol,
      RTFunction oldFunction) {

    if (oldFunction != null && oldFunction.getListener() != null) {
      return oldFunction;
    }

    RTFunction rtFunction = new RTFunction(rtValueList -> {
      RTValue[] args;
      if (rtValueList == null || rtValueList.size() == 0) {
        args = new RTValue[0];
      } else {
        args = rtValueList.toArray(new RTValue[rtValueList.size()]);
      }
      try {
        protocol
            .sendCallByNameAndNoReturns(oldFunction.getObjectId(),
                oldFunction.getFunctionName(),
                args).get();  // send call request

      } catch (Exception e) {
        logger.error("invoke remote function failed", e);
        throw new RTException(e);
      }
    });
    rtFunction.setFunctionName(oldFunction.getFunctionName());
    rtFunction.setObjectId(oldFunction.getObjectId());
    RTEnvironment.getRtFunctionMap().put(rtFunction.getFunctionName(), rtFunction);
    return rtFunction;
  }

}
