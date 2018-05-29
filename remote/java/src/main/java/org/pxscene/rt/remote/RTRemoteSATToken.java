package org.pxscene.rt.remote;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.net.ssl.HttpsURLConnection;

import org.pxscene.rt.RTException;

public class RTRemoteSATToken {
  private static final Logger log = Logger.getLogger(RTRemoteSATToken.class.getName());
  private static RTRemoteSATToken satToken;
	
  private static final int SAT_PARAM_COUNT = 4;
  private static long startTime = 0;
  private String m_satToken;
  private String m_satUrl;
  private String m_satContentType;
  private String m_satClientId;	
  private String m_satClientSecret;
 

  private RTRemoteSATToken() {
    /*Setting the value for SAT*/
    /*Need to update with user details*/
    m_satUrl = "https://sat-prod.codebig2.net/getSAT";
    m_satContentType = "application/json";
    m_satClientId = "x1:rdkc:40XXXX";							//Change this with user client id	
    m_satClientSecret = "f6f20bb82c2b9a4ddd623fXXXXXXXXX";     //Change this with user client secret

    startTime = getCurrentTime();
  }

  /**
   * get instance,  if not already created, creates one 
   * @return : satToken, instance of the class
   */
  public static RTRemoteSATToken getInstance() {
    if (startTime != 0 && isExpired()) {
      satToken = null;
    }
    if(satToken == null) {
      satToken = new RTRemoteSATToken();
    } 
    return satToken;	
  }

  /**
   * get current time in milliseconds
   * @reutrn : curTime, current time in milliseconds 
   */
  private static long getCurrentTime() {
    long curTime = System.currentTimeMillis();
    return curTime;
  }
	
  /**
   * checks if the session is expired 
   * for SAT token its 24 hours.
   * @return : isExpired , true or false
   */
  private static boolean isExpired() {
    Boolean isExpired = false;
    long currentTime = getCurrentTime();
    long sessionTime = currentTime - startTime;
		
    if(sessionTime/(1000*60*60) >= 24) {
      isExpired = true;
    }
    return isExpired;
  }

  /**
   * get sat token 
   * @return : m_satToken. sat token 
   */
  private String getSATToken() throws RTException {
    if (!isExpired() && m_satToken != null) {
      return m_satToken;
    } else {
      URL sat_url = null;
      HttpsURLConnection conn = null;
      try {
        sat_url = new URL(m_satUrl);
	      conn = (HttpsURLConnection)sat_url.openConnection();
			
	      /* Adding Headers */
        conn.setRequestMethod("GET");
        conn.setRequestProperty("Accept", m_satContentType);
        conn.setRequestProperty("X-Client-Id", m_satClientId);
        conn.setRequestProperty("X-Client-Secret" ,m_satClientSecret);		
			
        BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
	
        String inputLine;
        StringBuffer response = new StringBuffer();

        while ((inputLine = br.readLine()) != null) {
          response.append(inputLine);
        }
	
        br.close();	
		
        /* Parsing to get SAT token */
        m_satToken = getParsedSATToken(response.toString());
      }catch (MalformedURLException e) {
        e.printStackTrace();
      } catch (IOException e) {
        e.printStackTrace();
      }
      return m_satToken;
    }		
  }

  /**
   * parsing the token from the response
   * @param : response, obtain from server
   * @return : sat_token, sat token
   */
  private String getParsedSATToken(String response) {
    String sat_token = null;
		
    if(response.length() != 0) {
    /* Splitting the data into tokens*/
      String tokens[] = response.split(",|:");
      if(tokens.length == SAT_PARAM_COUNT) {
        sat_token  = tokens[SAT_PARAM_COUNT-1];						
        sat_token.trim();							/* Removing spaces */
        sat_token = sat_token.substring(1, sat_token.length()-2);		/* Removing quotes and bracket at last*/
      } else {
        log.log(Level.WARNING, "Error : SAT data not valid.");
      }
    } else {
      log.log(Level.WARNING, "Error : SAT data is null.");
    }			
    return sat_token;
  }  

  /**
   * get auth 
   * @return auth token
   */
  public String getAuth() {
    String auth = null;	
    try {
      auth =  "Bearer " + getSATToken();
    } catch (RTException e) {
      e.printStackTrace();
    }
    return auth;
  }
}
