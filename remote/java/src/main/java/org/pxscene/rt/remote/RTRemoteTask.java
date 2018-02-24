package org.pxscene.rt.remote;


import org.pxscene.rt.RTHelper;

/**
 * the rtRemote task, used to store the messages(include protocol) into task queue
 */
public class RTRemoteTask {


  /**
   * the protocol transport
   */
  private RTRemoteProtocol protocol;

  /**
   * the message from client
   */
  private RTRemoteMessage message;

  /**
   * create task with protocol and message
   *
   * @param protocol the protocol
   * @param message the message entity
   */
  public RTRemoteTask(RTRemoteProtocol protocol, RTRemoteMessage message) {
    this.protocol = protocol;
    this.message = message;
  }

  public RTRemoteProtocol getProtocol() {
    return this.protocol;
  }

  public void setProtocol(RTRemoteProtocol protocol) {
    RTHelper.ensureNotNull(protocol, "protocol");
    this.protocol = protocol;
  }

  public RTRemoteMessage getMessage() {
    return this.message;
  }

  public void setMessage(RTRemoteMessage message) {
    RTHelper.ensureNotNull(message, "message");
    this.message = message;
  }
}
