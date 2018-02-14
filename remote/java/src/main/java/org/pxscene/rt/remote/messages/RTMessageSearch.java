package org.pxscene.rt.remote.messages;


import java.net.URI;
import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;
import lombok.ToString;
import org.pxscene.rt.remote.RTRemoteMessage;
import org.pxscene.rt.remote.RTRemoteMessageType;


/**
 * the object serach entity.
 */
@ToString(callSuper = true)
public class RTMessageSearch extends RTRemoteMessage {

  /**
   * the remote object id
   */
  @Getter
  @Setter
  @NonNull
  private String objectId;

  /**
   * the sender id
   */
  @Getter
  @Setter
  @NonNull
  private int senderId;

  /**
   * the local host udp address, used to recv remote message
   */
  @Getter
  @Setter
  @NonNull
  private URI replyTo;

  /**
   * the entity constructor with type.
   */
  public RTMessageSearch() {
    super(RTRemoteMessageType.SERACH_OBJECT);
  }

}