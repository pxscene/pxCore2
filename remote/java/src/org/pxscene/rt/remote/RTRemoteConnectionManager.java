package org.pxscene.rt.remote;

import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;

import java.net.URI;
import java.util.Map;
import java.util.HashMap;

public class RTRemoteConnectionManager {
  private static final Map<String, RTRemoteClientConnection> m_connections = new HashMap<>();

  public static RTObject getObjectProxy(URI uri) throws RTException {
    RTRemoteClientConnection connection = null;

    String connectionSpec = uri.getScheme() + "//" + uri.getHost() + ":" + uri.getPort();
    if (m_connections.containsKey(connectionSpec)) {
      connection = m_connections.get(connectionSpec);
    } else {
      connection = RTRemoteConnectionManager.createConnectionFromSpec(uri);
      m_connections.put(connectionSpec, connection);
    }

    return connection.getProxyObject(uri.getPath().substring(1));
  }

  private static RTRemoteClientConnection createConnectionFromSpec(URI uri) throws RTException {
    RTRemoteClientConnection connection = null;
    switch (uri.getScheme()) {
      case "tcp":
        connection = RTRemoteClientConnection.createTCPClientConnection(uri.getHost(), uri.getPort());
        break;
      default:
        throw new RTException("unsupported scheme:" + uri.getScheme());
    }
    return connection;
  }
}
