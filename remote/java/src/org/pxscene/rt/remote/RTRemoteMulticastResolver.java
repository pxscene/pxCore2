package org.pxscene.rt.remote;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTException;
import org.pxscene.rt.remote.messages.RTMessageLocate;
import org.pxscene.rt.remote.messages.RTMessageSearch;

/**
 * The rt remote multicast resolver.
 */
public class RTRemoteMulticastResolver {

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTRemoteMulticastResolver.class);

  /**
   * the remote object map, thread safe
   */
  private static final Map<String, URI> objectMap = new ConcurrentHashMap<>();

  /**
   * the rt remote serializer instance
   */
  private RTRemoteSerializer serializer = new RTRemoteSerializer();

  /**
   * the multicast socket out channel
   */
  private MulticastSocket socketOut;

  /**
   * the udp in channel
   */
  private DatagramSocket socketIn;

  /**
   * the multicast group
   */
  private InetAddress group;

  /**
   * the multicast port
   */
  private int port;

  /**
   * create new remote multicast resolver.
   *
   * @param group the multicast group
   * @param port the port
   * @throws IOException if connect failed
   */
  public RTRemoteMulticastResolver(InetAddress group, int port) throws IOException {
    this.group = group;
    this.port = port;
    socketOut = new MulticastSocket();
    socketOut.joinGroup(this.group);
    socketIn = new DatagramSocket(new InetSocketAddress(InetAddress.getLoopbackAddress(), 0));

    new Thread(() -> { // start new thread to receive packet
      while (true) {
        byte[] recvBuff = new byte[4096];
        DatagramPacket pkt = new DatagramPacket(recvBuff, recvBuff.length);
        try {
          socketIn.receive(pkt);
          RTMessageLocate locate = serializer.fromBytes(recvBuff, 0, recvBuff.length);
          objectMap.put(locate.getObjectId(), new URI(
              locate.getEndpoint().toString() + "/" + locate.getObjectId()));
          Thread.sleep(1);
        } catch (Exception e) {
          logger.error("process locate message failed",e);
          break;
        }
      }
    }).start();
  }

  /**
   * get local udp uri to recv packet
   *
   * @return the udp uri
   * @throws RTException if build uri faild
   */
  private URI getReplyEndopint() throws RTException {
    StringBuilder builder = new StringBuilder();
    builder.append("udp://");
    builder.append(socketIn.getLocalAddress().getHostAddress());
    builder.append(":");
    builder.append(socketIn.getLocalPort());
    try {
      return new URI(builder.toString());
    } catch (URISyntaxException err) {
      throw new RTException("failed parsing uri:" + builder.toString(), err);
    }
  }

  /**
   * search remote object , this method will be block thread
   * 1. send udp packet
   * 2. check remote object found or not
   * 3. if not found and time > 1s , then send udp packet again
   *
   * @param name the object name
   * @return the remote object uri
   * @throws RTException if any other error occurred during operation
   */
  public URI locateObject(String name) throws RTException {
    String correlationKey = UUID.randomUUID().toString();

    RTMessageSearch search = new RTMessageSearch();
    search.setObjectId(name);
    search.setCorrelationKey(correlationKey);
    search.setSenderId(0);
    search.setReplyTo(getReplyEndopint());

    byte[] buff = serializer.toBytes(search);
    Long preCheckTime = System.currentTimeMillis();
    Long diff, now, seachTimeMultiple = 1L, totalCostTime = 0L;
    URI uri;
    try {
      socketOut.send(new DatagramPacket(buff, buff.length, group, port));
    } catch (IOException e) {
      e.printStackTrace();
    }
    while (true) {
      try {
        now = System.currentTimeMillis();
        diff = now - preCheckTime;
        uri = objectMap.get(name);
        if (uri != null) {
          objectMap.remove(name);
          return uri;
        }
        Long currentTime = RTConst.FIRST_FIND_OBJECT_TIME * seachTimeMultiple;
        if (diff >= currentTime) {
          totalCostTime += currentTime;
          logger.debug("searching object " + name + ", cost = " + totalCostTime / 1000.0 + "s");
          seachTimeMultiple *= 2;
          socketOut.send(new DatagramPacket(buff, buff.length, group, port));
          preCheckTime = now;
        }
        Thread.sleep(10);
      } catch (Exception e) {
        logger.error(e);
        throw new RTException("locateObject failed", e);
      }
    }
  }
}
