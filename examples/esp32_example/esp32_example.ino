
#include "sim7020_lib.h"

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\nWait...");

  sim7020_HwInit("28");

  sim7020_NbiotManager();
}

void loop() {
  // put your main code here, to run repeatedly:

}
