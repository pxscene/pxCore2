package org.pxscene.rt.remote;

import java.net.URI;
import java.util.HashMap;
import java.util.Map;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTException;
import org.pxscene.rt.RTObject;

/**
 * The remote connection manager class.
 */
public class RTRemoteConnectionManager {

  /**
   * the connections map object
   */
  private static final Map<String, RTRemoteClientConnection> connections = new HashMap<>();

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTRemoteConnectionManager.class);

  /**
   * get remote object by uri
   *
   * @param uri the object uri
   * @return the remote
   * @throws RTException if any other error occurred during operation
   */
  public static RTObject getObjectProxy(URI uri) throws RTException {
    RTRemoteClientConnection connection;

    String connectionSpec = uri.getScheme() + "//" + uri.getHost() + ":" + uri.getPort();
    if (connections.containsKey(connectionSpec)) {
      connection = connections.get(connectionSpec);
    } else {
      connection = RTRemoteConnectionManager.createConnectionFromSpec(uri);
      connections.put(connectionSpec, connection);
    }

    return connection.getProxyObject(uri.getPath().substring(1));
  }

  /**
   * create connection from uri
   *
   * @param uri the dest uri
   * @return the RTRemoteClientConnection
   * @throws RTException if any other error occurred during operation
   */
  private static RTRemoteClientConnection createConnectionFromSpec(URI uri) throws RTException {
    RTRemoteClientConnection connection;
    logger.info("start connection " + uri.toString());
    switch (uri.getScheme()) {
      case "tcp":
        connection = RTRemoteClientConnection
            .createTCPClientConnection(uri.getHost(), uri.getPort());
        break;
      default:
        throw new RTException("unsupported scheme:" + uri.getScheme());
    }
    return connection;
  }
}
