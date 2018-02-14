package org.pxscene.rt.remote;


import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;

/**
 * the rtRemote task, used to store the messages(include protocol) into task queue
 */
@AllArgsConstructor
public class RTRemoteTask {


  /**
   * the protocol transport
   */
  @Getter
  @Setter
  @NonNull
  private RTRemoteProtocol protocol;

  /**
   * the message from client
   */
  @Getter
  @Setter
  @NonNull
  private RTRemoteMessage message;
}
