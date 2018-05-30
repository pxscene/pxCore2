package org.pxscene.rt.remote;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URI;
import java.net.URL;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.net.ssl.HttpsURLConnection;
import org.pxscene.rt.RTException;


public class RTRemoteWRPTransport implements RTRemoteTransport{
  private static final Logger log = Logger.getLogger(RTRemoteWRPTransport.class.getName());
  private static final int MAC_COUNT = 2;
  private static final int  DATA_MAX_LEN= 2048;
  
  private String m_macAddr;
  private BlockingQueue<byte[]> m_incomingQueue;

  private HttpsURLConnection m_connection;
  private OutputStreamWriter m_out;
  private RTRemoteSATToken satToken;
  private URL m_url;
  
  
  public RTRemoteWRPTransport(URI uri) throws RTException {
    try {
      m_url = new URL("https://api.xmidt.comcast.net/api/v2/device");
      satToken = RTRemoteSATToken.getInstance();
      m_macAddr = getMac(uri);
      m_incomingQueue = new LinkedBlockingQueue<byte []>();
    } catch (MalformedURLException e) {
      e.printStackTrace();
    }
  }

  private void read() { 

    BufferedReader br = null;
    String inputLine = null;
    StringBuffer response = null;	
    byte[] buff = new byte[DATA_MAX_LEN];
		
    try {
      br = new BufferedReader(new InputStreamReader(m_connection.getInputStream()));
      response = new StringBuffer();
			
      while((inputLine = br.readLine()) != null) {
        response.append(inputLine);
      }
	      
      buff = response.toString().getBytes();
      try {
        m_incomingQueue.put(buff);
      } catch (InterruptedException err) {
        log.log(Level.WARNING, "error reading from stream", err);
      }
      br.close();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  
  public void open() throws RTException {
    try {
      m_connection = (HttpsURLConnection)m_url.openConnection();
      m_connection.setRequestMethod("POST");
      m_connection.setRequestProperty("Content-Type", "octet-stream");
      m_connection.setRequestProperty("Authorization", satToken.getAuth());
      m_connection.setRequestProperty("X-Webpa-Device-Name", getDeciveName());
      m_connection.setRequestProperty("X-Xmidt-Message-Type", "SimpleRequestResponse");
      m_connection.setRequestProperty("X-Xmidt-Transaction-Uuid", "1223");
      m_connection.setRequestProperty("X-Xmidt-Source", "dns:foo.bar");
    } catch (ProtocolException e) {
      e.printStackTrace();
    }  catch (IOException e) {
      e.printStackTrace();
    }
  }

  public void close() throws RTException {
    try {
      m_connection.disconnect();
      m_out.close();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  
  public void send(byte[] buff) throws RTException {
    int responseCode = 0;
    String msg = new String (buff);
	
    try {
      open();
      m_connection.setDoOutput(true);
      m_out = new OutputStreamWriter(m_connection.getOutputStream());
      m_out.write(msg);
      m_out.flush();
      m_out.close();

      responseCode = m_connection.getResponseCode();
      if(responseCode != HttpsURLConnection.HTTP_OK) {
        log.log(Level.WARNING, "Error, Response code : ", responseCode);
      } else {
        read();	
      }
    } catch (IOException e) {
      e.printStackTrace();
    } 
    finally {
      close();
    }
  }

  public byte[] recv() throws RTException {
    try {
      return m_incomingQueue.take();
    } catch (Exception err) {
      log.log(Level.WARNING, "Error : ", err);
      throw new RTException(err);
    }		
  }
  
  private String getDeciveName() {
    return "mac:" + m_macAddr + "/iot" ;
  }

  public String getMac(URI uri) {
    String mac = null;
    if(!uri.toString().isEmpty()) {
      String tokens[] = uri.toString().split("/");
      mac = tokens[MAC_COUNT];
    }
    return mac.toString();
  } 
}
