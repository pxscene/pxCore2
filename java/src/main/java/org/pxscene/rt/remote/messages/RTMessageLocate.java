package org.pxscene.rt.remote.messages;


import java.net.URI;
import lombok.Getter;
import lombok.Setter;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the Locate (search done) response entity.
 */
public class RTMessageLocate extends RTRemoteMessage {


  /**
   * the remote object id
   */
  @Getter
  @Setter
  private String objectId;

  /**
   * the remote sender id
   */
  @Getter
  @Setter
  private int senderId;

  /**
   * the rpc endpoint uri
   */
  @Setter
  @Getter
  private URI endpoint;

  /**
   * the entity constructor with type.
   */
  public RTMessageLocate() {
    super(RTRemoteMessageType.SERACH_OBJECT);
  }
}