package org.pxscene.rt.remote.messages;


import lombok.NonNull;
import java.net.URI;
import lombok.Getter;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;

/**
 * the Locate (search done) response entity.
 */
@ToString(callSuper = true)
public class RTMessageLocate extends RTRemoteMessage {


  /**
   * the remote object id
   */
  @Getter
  @Setter
  @NonNull
  private String objectId;

  /**
   * the remote sender id
   */
  @Getter
  @Setter
  @NonNull
  private int senderId;

  /**
   * the rpc endpoint uri
   */
  @Setter
  @Getter
  @NonNull
  private URI endpoint;

  /**
   * the entity constructor with type.
   */
  public RTMessageLocate() {
    super(RTRemoteMessageType.LOCATE_OBJECT);
  }
}