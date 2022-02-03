
#include "sim7020_lib.h"

SIM7020 nb_modem(14, 13, 25, "28"); //Rx, Tx, Power, RF band

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\nWait...");

  nb_modem.set_NetworkCredentials("virtueyes.com.br","virtu","virtu");

  nb_modem.HwInit();

  nb_modem.set_Host("http", "www.cropnet.us", "80");
  nb_modem.set_HttpRequestOptions("GET", "/test/now");

  nb_modem.set_HttpVersion("HTTP/1.1");
  nb_modem.set_HttpHeader("Host: www.cropnet.us\r\n");

  nb_modem.NbiotManager();
}

void loop() {
  Serial.println("flag");
  delay(5000);
}
