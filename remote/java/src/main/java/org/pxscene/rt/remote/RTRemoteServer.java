package org.pxscene.rt.remote;


import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URI;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedDeque;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTEnvironment;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTFunction;
import org.pxscene.rt.RTHelper;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTValueType;
import org.pxscene.rt.remote.messages.RTMessageCallMethodRequest;
import org.pxscene.rt.remote.messages.RTMessageCallMethodResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageLocate;
import org.pxscene.rt.remote.messages.RTMessageSearch;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameResponse;

/**
 * the remote server
 */
public class RTRemoteServer {

  /**
   * the logger instance.
   */
  private static final Logger logger = Logger.getLogger(RTRemoteServer.class);

  /**
   * the register objects map, thread safe
   */
  private static final Map<String, Object> registerObjectMap = new ConcurrentHashMap<>();


  /**
   * the rt remote serializer instance
   */
  private RTRemoteSerializer serializer = new RTRemoteSerializer();

  /**
   * the task message queue
   */
  private Queue<RTRemoteTask> messageQueue;

  /**
   * the server socket instance
   */
  private ServerSocket serverSocket;

  /**
   * create new rtServer
   *
   * @param udpGroupAddr the multicast udp address
   * @param udpPort the multicast udp port
   * @param rpcAddr the tcp address
   * @param rpcPort the tcp port
   * @throws RTException raise error if create failed
   * @throws IOException rasie error if bind failed
   */
  public RTRemoteServer(InetAddress udpGroupAddr, int udpPort, InetAddress rpcAddr, int rpcPort)
      throws RTException, IOException {

    RTEnvironment.setRunMode(RTConst.SERVER_MODE); // set as server mode
    messageQueue = new ConcurrentLinkedDeque<>();
    MulticastSocket socketIn = new MulticastSocket(udpPort);
    DatagramSocket replySocketOut = new DatagramSocket();
    socketIn.joinGroup(udpGroupAddr);
    logger.debug("server bind multicast socket succeed");
    openRpcSocketServer(rpcAddr, rpcPort);
    new Thread(() -> { // start new thread to receive search object message
      while (true) {
        byte[] recvBuff = new byte[4096];
        DatagramPacket pkt = new DatagramPacket(recvBuff, recvBuff.length);
        try {
          socketIn.receive(pkt);
          RTMessageSearch search = serializer.fromBytes(recvBuff, 0, recvBuff.length);
          if (registerObjectMap.containsKey(search.getObjectId())) { // searched
            RTMessageLocate locate = new RTMessageLocate();
            locate.setEndpoint(new URI("tcp://" + rpcAddr.getHostAddress() + ":" + rpcPort));
            locate.setObjectId(search.getObjectId());
            locate.setSenderId(search.getSenderId());
            locate.setCorrelationKey(search.getCorrelationKey());
            byte[] locateBytes = serializer.toBytes(locate);
            replySocketOut.send(new DatagramPacket(locateBytes, 0, locateBytes.length,
                InetAddress.getByName(search.getReplyTo().getHost()),
                search.getReplyTo().getPort())); // send back locate message
          } else { // cannot found object, skip it
            logger.debug(
                "client want search object " + search.getObjectId() + ", but server not found");
          }
          Thread.sleep(1); // release cpu
        } catch (Exception e) {
          logger.error("udp read error", e);
          socketIn.close(); // close udp in socket
          replySocketOut.close(); // close udp out socket
          break;
        }
      }
    }).start();

    startProcessMessages();
  }

  /**
   * create create new rtServer
   *
   * @param udpGroupAddr the multicast udp address
   * @param udpPort the multicast udp port
   * @param rpcAddr the tcp address
   * @throws RTException raise error if create failed
   * @throws IOException rasie error if bind failed
   */
  public RTRemoteServer(InetAddress udpGroupAddr, int udpPort, InetAddress rpcAddr)
      throws RTException, IOException {
    this(udpGroupAddr, udpPort, rpcAddr, RTHelper.findRandomOpenPortOnAllLocalInterfaces());
  }

  /**
   * open socket server, so that can work as rpc server note that, serverSocket should not closed by
   * any exception, it should be work until closed by manually
   *
   * @param rpcAddr the tcp address
   * @param rpcPort the tcp port
   * @throws IOException if bind failed
   */
  private void openRpcSocketServer(InetAddress rpcAddr, int rpcPort) throws IOException {
    serverSocket = new ServerSocket(rpcPort, RTConst.BACK_LOG, rpcAddr);
    new Thread(() -> { // open new thread to accpet new connections
      while (true) {
        if (serverSocket == null || serverSocket.isClosed()) {
          break;
        }
        try {
          Socket socket = serverSocket.accept();
          RTRemoteTransport transport = new RTRemoteTCPTransport(socket);
          RTRemoteProtocol protocol = new RTRemoteProtocol(transport, true);
          protocol.setRtRemoteServer(this); // inject server to protocol
          Thread.sleep(1); // release cpu
        } catch (Exception e) {  // if accept failed, server should be log this error and wait next continue process next client
          logger.error("protocal error", e);
        }
      }
    }).start();
    logger.debug("rpc socket bind succeed.");
  }


  /**
   * start process message
   * create new thread to get message from task message queue
   */
  private void startProcessMessages() {
    new Thread(() -> { // create new thread to get message
      while (true) {
        try {
          if (messageQueue.size() > 0) {
            RTRemoteTask messageTask = messageQueue.poll(); // fetch a message
            handlerMessage(messageTask);
          }
          Thread.sleep(1); // release cpu
        } catch (Exception e) {
          logger.error("process message failed", e);
          break;
        }
      }
    }).start();
  }

  /**
   * get server object from cache
   *
   * @param name the object name
   * @return the server object
   */
  private Object getObjectByName(String name) {
    Object object = registerObjectMap.get(name);
    if (object == null) {
      throw new NullPointerException("object " + name + " should not be null");
    }
    return object;
  }

  /**
   * process message
   *
   * @param rtRemoteTask the task message
   * @throws RTException raise error if process failed
   */
  private void handlerMessage(RTRemoteTask rtRemoteTask) throws RTException {
    switch (rtRemoteTask.getMessage().getMessageType()) {
      case GET_PROPERTY_BYNAME_REQUEST:
        this.handlerGetPropertyByNameRequest(rtRemoteTask);
        break;
      case SET_PROPERTY_BYNAME_REQUEST:
        this.handlerSetPropertyByNameRequest(rtRemoteTask);
        break;
      case METHOD_CALL_REQUEST:
        this.handlerCallRequest(rtRemoteTask);
        break;
      default:
        throw new RTException(
            "handlerMessage -> don't support message type " + rtRemoteTask.getMessage()
                .getMessageType().toString() + "");
    }
  }

  /**
   * process call request message task
   *
   * @param rtRemoteTask the request message task
   * @throws RTException raise error if process failed
   */
  private void handlerCallRequest(RTRemoteTask rtRemoteTask) throws RTException {
    RTMessageCallMethodRequest callMethodRequest = (RTMessageCallMethodRequest) rtRemoteTask
        .getMessage();

    if (callMethodRequest.getFunctionArgs() != null
        && callMethodRequest.getFunctionArgs().size() > 0) {
      for (RTValue rtValue : callMethodRequest.getFunctionArgs()) {
        if (rtValue.getType().equals(RTValueType.FUNCTION)) {
          rtValue.setValue(RTHelper.updateListenerForRTFuction(rtRemoteTask.getProtocol(),
              (RTFunction) rtValue.getValue()));
        }
      }
    }
    RTMessageCallMethodResponse callMethodResponse = RTHelper
        .invokeMethod(getObjectByName(callMethodRequest.getObjectId()), callMethodRequest);
    rtRemoteTask.getProtocol().getTransport().send(serializer.toBytes(callMethodResponse));
  }


  /**
   * process get request message task
   *
   * @param rtRemoteTask the request message task
   * @throws RTException raise error if process failed
   */
  private void handlerGetPropertyByNameRequest(RTRemoteTask rtRemoteTask) throws RTException {
    RTRemoteMessage message = rtRemoteTask.getMessage();
    RTMessageGetPropertyByNameRequest getRequest = (RTMessageGetPropertyByNameRequest) message;
    RTMessageGetPropertyByNameResponse getResponse = RTHelper
        .getPropertyByName(getObjectByName(getRequest.getObjectId()), getRequest);
    rtRemoteTask.getProtocol().getTransport().send(serializer.toBytes(getResponse));
  }

  /**
   * process set request message task
   *
   * @param rtRemoteTask the request message task
   * @throws RTException raise error if process failed
   */
  private void handlerSetPropertyByNameRequest(RTRemoteTask rtRemoteTask) throws RTException {
    RTRemoteMessage message = rtRemoteTask.getMessage();
    RTMessageSetPropertyByNameRequest setRequest = (RTMessageSetPropertyByNameRequest) message;

    if (setRequest.getValue().getType().equals(RTValueType.FUNCTION)) { // update listener
      RTFunction oldFunction = (RTFunction) setRequest.getValue().getValue();
      setRequest.getValue()
          .setValue(RTHelper.updateListenerForRTFuction(rtRemoteTask.getProtocol(), oldFunction));
    }

    RTMessageSetPropertyByNameResponse setResponse = RTHelper
        .setPropertyByName(getObjectByName(setRequest.getObjectId()), setRequest);
    rtRemoteTask.getProtocol().getTransport().send(serializer.toBytes(setResponse));
  }


  /**
   * register a object with object name
   *
   * @param objectName the object name
   * @param object the object
   */
  public void registerObject(String objectName, Object object) {
    registerObjectMap.put(objectName, object);
    RTHelper.dumpObject(objectName, object);
  }


  /**
   * unregister object from server
   *
   * @param objectName the object name
   * @throws RTException throw errors if object not exist
   */
  public void unRegisterObject(String objectName) throws RTException {
    Object object = registerObjectMap.get(objectName);
    if (object == null) {
      throw new RTException("cannot found register object named " + objectName);
    }
    registerObjectMap.remove(objectName);
  }

  public Queue<RTRemoteTask> getMessageQueue() {
    return this.messageQueue;
  }
}
