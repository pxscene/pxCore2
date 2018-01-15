package org.spark;

import java.net.URI;
import java.util.Map;
import java.util.HashMap;

public class SparkConnectionManager {
  private static final Map<String, SparkClientConnection> m_connections = new HashMap<>();

  public static SparkObject getObjectProxy(URI uri) throws SparkException {
    SparkClientConnection connection = null;

    String connectionSpec = uri.getScheme() + "//" + uri.getHost() + ":" + uri.getPort();
    if (m_connections.containsKey(connectionSpec)) {
      connection = m_connections.get(connectionSpec);
    } else {
      connection = SparkConnectionManager.createConnectionFromSpec(uri);
      m_connections.put(connectionSpec, connection);
    }

    return connection.getProxyObject(uri.getPath().substring(1));
  }

  private static SparkClientConnection createConnectionFromSpec(URI uri) throws SparkException {
    SparkClientConnection connection = null;
    switch (uri.getScheme()) {
      case "tcp":
        connection = SparkClientConnection.createTCPClientConnection(uri.getHost(), uri.getPort());
        break;
      default:
        throw new SparkException("unsupported scheme:" + uri.getScheme());
    }
    return connection;
  }
}
