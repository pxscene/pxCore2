package org.pxscene.rt.remote;

import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;

/**
 * the connection class between client and remote.
 */
public class RTRemoteClientConnection {

  /**
   * the remote protocol
   */
  private RTRemoteProtocol proto;


  /**
   * the entity constructor with type.
   *
   * @param protocol the remote protocol
   */
  private RTRemoteClientConnection(RTRemoteProtocol protocol) {
    proto = protocol;
  }

  /**
   * create a TCP client connection
   *
   * @param host the host name
   * @param port the port
   * @return the RTRemoteClientConnection
   * @throws RTException if any other error occurred during operation
   */
  public static RTRemoteClientConnection createTCPClientConnection(String host, int port)
      throws RTException {
    RTRemoteTransport transport = new RTRemoteTCPTransport(host, port);
    return new RTRemoteClientConnection(new RTRemoteProtocol(transport, false));
  }

  /**
   * create new client-remote object
   *
   * @param objectId the remote object id
   * @return the rtObject
   */
  public RTObject getProxyObject(String objectId) {
    return new RTRemoteObject(proto, objectId);
  }
}
