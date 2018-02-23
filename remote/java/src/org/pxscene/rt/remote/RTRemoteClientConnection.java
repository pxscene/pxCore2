package org.pxscene.rt.remote;

import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;
import org.pxscene.rt.remote.RTRemoteProtocol;

public class RTRemoteClientConnection {
  private RTRemoteProtocol m_proto;

  private RTRemoteClientConnection(RTRemoteProtocol protocol) {
    m_proto = protocol;
  }

  public static RTRemoteClientConnection createTCPClientConnection(String host, int port) throws RTException {
    RTRemoteTransport transport = new RTRemoteTCPTransport(host, port);
    return new RTRemoteClientConnection(new RTRemoteProtocol(transport));
  }

  public RTObject getProxyObject(String objectId) {
    return new RTRemoteObject(m_proto, objectId);
  }
}
