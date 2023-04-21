#include <SoftwareSerial.h>

SoftwareSerial ss(11,10);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(4800);
}

void loop() {
  Serial.println(Serial.read());
  Serial.print("lan 1");
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    Serial.println(Serial.available());
    byte ph = Serial.read();
    Serial.println(ph);
    Serial.print("lan 2");
    Serial.println("Check");
  } else {
    Serial.println("loi");
  }

  delay(3000);
}
