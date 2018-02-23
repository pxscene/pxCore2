package org.pxscene.rt.remote;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.UUID;

import org.pxscene.rt.RTException;
import org.pxscene.rt.remote.messages.RTMessageSearch;
import org.pxscene.rt.remote.messages.RTMessageLocate;

public class RTRemoteMulticastResolver {
  private MulticastSocket m_socketOut;
  private DatagramSocket m_socketIn;
  private InetAddress m_group;
  private int m_port;

  public RTRemoteMulticastResolver(InetAddress group, int port) throws IOException {
    m_group = group;
    m_port = port;
    m_socketOut = new MulticastSocket();
    m_socketOut.joinGroup(m_group);
    m_socketIn = new DatagramSocket(new InetSocketAddress(InetAddress.getLoopbackAddress(), 0));
  }

  private URI getReplyEndopint() throws RTException {
    StringBuilder builder = new StringBuilder();
    builder.append("udp://");
    builder.append(m_socketIn.getLocalAddress().getHostAddress());
    builder.append(":");
    builder.append(m_socketIn.getLocalPort());

    URI uri = null;

    try {
      uri = new URI(builder.toString());
    } catch (URISyntaxException err) {
      throw new RTException("failed parsing uri:" + builder.toString(), err);
    }

    return uri;
  }

  public URI locateObject(String name) throws RTException {
    URI uri = null;
    String temp = null;
    String correlationKey = UUID.randomUUID().toString();

    RTMessageSearch search = new RTMessageSearch();
    search.setObjectId(name);
    search.setCorrelationKey(correlationKey);
    search.setSenderId(0);
    search.setReplyTo(getReplyEndopint());

    RTRemoteSerializer m_serializer = new RTRemoteSerializer();
    m_serializer.toBytes(search);

    byte[] buff = m_serializer.toBytes(search);
    try {
      m_socketOut.send(new DatagramPacket(buff, buff.length, m_group, m_port));

      buff = new byte[4096];
      DatagramPacket pkt = new DatagramPacket(buff, buff.length);
      m_socketIn.receive(pkt);


      RTMessageLocate locate = (RTMessageLocate) m_serializer.fromBytes(buff, 0, buff.length);

      temp = locate.getEndpoint().toString();
      temp += "/";
      temp += name;

      uri = new URI(temp);

    } catch (IOException ioe) {
      throw new RTException("failed to send datagram packet for search", ioe);
    } catch (URISyntaxException syntaxError) {
      throw new RTException("failed to parse URI string:" + temp, syntaxError);
    }

    return uri;
  }
}
