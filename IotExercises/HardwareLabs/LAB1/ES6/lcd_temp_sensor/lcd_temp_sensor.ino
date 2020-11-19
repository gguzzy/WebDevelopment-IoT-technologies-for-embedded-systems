#include <LiquidCrystal_PCF8574.h>
#include <math.h>

LiquidCrystal_PCF8574 lcd(0x27);

const int TEMP_PIN = A1;
const float B = 4275.0;
const float R = 100000.0;
const float Vcc = 1023.0;
const float T0 = 298.0;

void setup() {
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();//setup del display
  lcd.print("Temperature:");
  pinMode(TEMP_PIN, INPUT);
}

void loop() {
  float Vsig  = analogRead(TEMP_PIN);
  float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
  float T = 1.0/(log(R/R0)/B + 1/T0);
  T = T - 273.15;//calcolo e conversione della temperatura in C
  lcd.setCursor(1, 1);//imposto posizione cursore alla seconda riga, aggiornando solo il dato di temperatura 
  lcd.print(T);
  lcd.print("C");

  delay(10000);//ritardo da 10 secondi
}
