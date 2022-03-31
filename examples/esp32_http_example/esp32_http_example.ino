#include <string>
#include "sim7020_lib.h"

SIM7020 nb_modem(26, 27, 25, "28"); //Rx, Tx, Power, RF band

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\nWait...");

  nb_modem.set_NetworkCredentials("virtueyes.com.br","virtu","virtu");
  nb_modem.HwInit();
    
  nb_modem.set_Host("http", "www.gnu.org", "80");
  nb_modem.set_HttpRequestOptions("GET", "/licenses/lgpl-3.0.html");
  nb_modem.set_HttpVersion("HTTP/1.1");  
  nb_modem.set_HttpHeader("Host: www.gnu.org\r\n");
  nb_modem.set_Packet("[{\"macAddress\":\"6e:22:81:ff:04:fd\",\"AmbientDataList\":[{\"temperatureInst\":\"115.64\",\"pressureInst\":1215,\"brixDegree\":26.50}],\"Identifier\":\"LJA29\", \"Version\":\"4.5.6\"}]");
  nb_modem.NbiotManager(); //state machine 

  nb_modem.set_Host("http", "httpbin.org", "80");
  nb_modem.set_HttpHeader("Host: httpbin.org\r\n"
  "Host: www.cropnet.us\r\n"
  "Accept: */*\r\n"
  "Content-Type: application/json\r\n");
  nb_modem.set_HttpRequestOptions("POST", "/post");
  nb_modem.NbiotManager();
}

void loop() {
  Serial.println("flag");
  delay(5000);
}
