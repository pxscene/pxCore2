package org.pxscene.rt.remote;


import org.pxscene.rt.RTHelper;

/**
 * the base RTRemote Message class.
 */
public class RTRemoteMessage {


  /**
   * the rt remote message type
   */
  protected RTRemoteMessageType messageType;

  /**
   * the correlation key
   */
  protected String correlationKey;

  /**
   * the entity constructor with type
   */
  protected RTRemoteMessage(RTRemoteMessageType kind) {
    messageType = kind;
  }


  public RTRemoteMessageType getMessageType() {
    return this.messageType;
  }

  public String getCorrelationKey() {
    return this.correlationKey;
  }

  public void setCorrelationKey(String correlationKey) {
    RTHelper.ensureNotNull(correlationKey, "correlationKey");
    this.correlationKey = correlationKey;
  }
}

