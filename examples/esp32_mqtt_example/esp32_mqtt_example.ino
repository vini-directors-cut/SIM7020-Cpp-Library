#include <string>
#include "sim7020_lib.h"

SIM7020 nb_modem(26, 27, 4, "28"); //Rx, Tx, Power, RF band

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\nWait...");

  nb_modem.set_NetworkCredentials("virtueyes.com.br","virtu","virtu");
  nb_modem.HwInit();
  
  nb_modem.set_Host("mqtt", "a3e1a29a.us-east-1.emqx.cloud" ,"15539");
  nb_modem.set_MqttSubscriptionOptions("esp32/Nbiot","0");
  nb_modem.NbiotManager();
}


void loop() {
  Serial.println("flag");
  delay(5000);
}
