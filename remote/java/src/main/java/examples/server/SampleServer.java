package examples.server;


import java.io.IOException;
import java.net.InetAddress;
import org.apache.log4j.Logger;
import org.pxscene.rt.RTEnvironment;
import org.pxscene.rt.RTException;
import org.pxscene.rt.remote.RTRemoteServer;

public class SampleServer {

  private static final Logger logger = Logger.getLogger(SampleServer.class);

  public static void main(String[] args) throws IOException, RTException {
    RTEnvironment.init();
    RTRemoteServer rtRemoteServer = new RTRemoteServer(InetAddress.getByName("224.10.10.12"), 10004,
        InetAddress.getByName("127.0.0.1"));

    rtRemoteServer.registerObject("host_object", new SampleObject());
    rtRemoteServer.registerObject("obj2", new SampleObject());
    rtRemoteServer.registerObject("obj3", new SampleObject());
    rtRemoteServer.registerObject("obj4", new SampleObject());

    logger.debug("rt server start ...");
  }
}
