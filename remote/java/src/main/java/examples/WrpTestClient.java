package examples;

import java.net.URI;
import java.util.concurrent.Future;

import org.pxscene.rt.RTObject;
import org.pxscene.rt.RTValue;
import org.pxscene.rt.RTValueType;
import org.pxscene.rt.remote.RTRemoteConnectionManager;

public class WrpTestClient {
  public static void main(String[] args) throws Exception  {

    /*
    * URL format : "wrp:/mac_address/service_name/object" 
    * #Change the mac of the device below before testing. 
    */
    URI uri = new URI("wrp://3C:DF:A9:C1:58:97/rtremote/some_name");
    RTObject obj = RTRemoteConnectionManager.getObjectProxy(uri);

    int n = 10;

    while (true) {
      try {
        /* Do Get */  
	Future<RTValue> f = obj.get("prop");
        RTValue v = f.get();
        System.out.println("Value : " + v.getValue());

        Thread.sleep(1000);

        /* Do Set */
        Future<Void> f2 = obj.set("prop", new RTValue(n++));
        f2.get();

        /* Method Call - two args and return type */
        int z = n + 10;
	    
        RTValue a = new RTValue(n, RTValueType.INT32);
        RTValue b = new RTValue(z , RTValueType.INT32) ;
	        
        Future<RTValue> f3 = obj.sendReturns("twoIntNumberSum", a , b);
        RTValue val = f3.get();
        System.out.println("Method Return Value :" + val.getValue());
      } catch (Exception err) {
        err.printStackTrace();
      }
    }   
  }
}

