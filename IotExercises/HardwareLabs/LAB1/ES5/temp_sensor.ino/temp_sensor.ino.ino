#include <math.h>

const int TEMP_PIN = A1;
const float B = 4275.0;
const float R = 100000.0;
const float Vcc = 1023.0;
const float T0 = 298.0;
void setup() {
  // put your setup code here, to run once:
  pinMode(TEMP_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  float Vsig  = analogRead(TEMP_PIN);//leggo tensione dal sensore
  float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
  float T = 1.0/(log(R/R0)/B + 1/T0);
  T = T - 273.15;//calcolo e converto temperatura in C
  Serial.print("Temperatura: ");
  Serial.println(T);
  delay(200);
}
