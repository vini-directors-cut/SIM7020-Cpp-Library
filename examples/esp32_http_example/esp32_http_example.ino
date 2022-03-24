#include <string>
#include "sim7020_lib.h"

SIM7020 nb_modem(14, 13, 25, "28"); //Rx, Tx, Power, RF band

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\nWait...");

  nb_modem.set_NetworkCredentials("virtueyes.com.br","virtu","virtu");
  nb_modem.HwInit();
    
  nb_modem.set_Host("http", "httpbin.org", "80");
  nb_modem.set_HttpRequestOptions("GET", "/get");
  nb_modem.set_HttpVersion("HTTP/1.1");
  
  nb_modem.set_HttpHeader("Host: www.cropnet.us\r\n"
    "Accept: */*\r\n"
    "uptime: 2\r\n"
    "MacAddress: 6e:22:81:c1:04:fd\r\n"
    "PICName: hdw2599\r\n"
    "PICVersion: 4.5.6\r\n"
    "Content-Type: application/json\r\n");
  
  nb_modem.set_Packet("[{\"macAddress\":\"6e:22:81:c1:04:fd\",\"picDataList\":[{\"updated\":\"2022-02-01 15:59:49\","
  "\"started\":\"2022-02-01 15:00:00\",\"finished\":\"2022-02-01 16:00:00\",\"temperatureInst\":\"25.64\",\"rain\":0.0,\"created\":\"2022-02-"
  "01 15:01:26\",\"rain2\":0.0}],\"picName\":\"hdw2599\",\"pivotDataList\":[],\"picRebootHistoryList\":[],\"pic"
  "Version\":\"4.5.6\",\"uptime\":2}]");
  
  nb_modem.NbiotManager(); //state machine
  
  //nb_modem.PowerSaveMode(true);
  //Serial.println("SIM7020 entered in sleep mode ZzZzZzZzZZZz...");
  //delay(20000);

  //nb_modem.PowerSaveMode(false);
  
}

void loop() {
  Serial.println("flag");
  delay(5000);
}
