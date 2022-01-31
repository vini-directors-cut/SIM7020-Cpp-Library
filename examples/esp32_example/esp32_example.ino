#include "sim7020_lib.h"

SIM7020 nb_modem(26, 27, 12, "28"); //Rx, Tx, Power, RF band

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\nWait...");

  nb_modem.set_NetworkCredentials("virtueyes.com.br","virtu","virtu");
  
  nb_modem.HwInit();

  //sim7020_NbiotManager();
}

void loop() {
  Serial.println("flag");
  delay(5000);
}
