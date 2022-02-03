#include <string>
#include "sim7020_lib.h"

SIM7020 nb_modem(14, 13, 25, "28"); //Rx, Tx, Power, RF band

void setup() {
  Serial.begin(115200);
  delay(100);
    
  Serial.println("\nWait...");

  nb_modem.set_NetworkCredentials("virtueyes.com.br","virtu","virtu");

  nb_modem.HwInit();
  
  nb_modem.set_Host("http", "www.cropnet.us", "80");
  nb_modem.set_HttpRequestOptions("POST", "/pic/gateway/data");
  nb_modem.set_HttpVersion("HTTP/1.1");
  
  nb_modem.set_HttpHeader("Host: www.cropnet.us\r\n"
    "Accept: */*\r\n"
    "uptime: 2\r\n"
    "MacAddress: 6e:22:81:c1:04:fd\r\n"
    "PICName: hdw2599\r\n"
    "PICVersion: 4.5.6\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 303\r\n");
  
  nb_modem.set_Payload("[{\"macAddress\":\"6e:22:81:c1:04:fd\",\"picDataList\":[{\"updated\":\"2022-02-01 15:59:59\","
  "\"started\":\"2022-02-01 15:00:00\",\"finished\":\"2022-02-01 16:00:00\",\"rain\":0.0,\"created\":\"2022-02-"
  "01 15:00:26\",\"rain2\":0.0}],\"picName\":\"hdw2599\",\"pivotDataList\":[],\"picRebootHistoryList\":[],\"pic"
  "Version\":\"4.5.6\",\"uptime\":2}]");
  
  nb_modem.NbiotManager(); //state machine
}

void loop() {
  Serial.println("flag");
  delay(5000);
}
