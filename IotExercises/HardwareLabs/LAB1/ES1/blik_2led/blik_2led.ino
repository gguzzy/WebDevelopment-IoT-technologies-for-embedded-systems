#include <TimerOne.h>
/*esercizio due modifica di questo*/
/*impostare volatile le variabili modificate da interrupt*/
const int RLED_PIN = 12;//led rosso
const int GLED_PIN = 11;//led verde

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

volatile int greenLedState = LOW;
volatile int redLedState = LOW;

void blinkGreen()
{
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}
void setup() {
  
  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  Timer1.initialize(G_HALF_PERIOD * 1e06);//imposto semiperiodo a 3,5 secondi
  Timer1.attachInterrupt(blinkGreen);//abiliito interrupt del timer collegandolo alla funzione
    
}

void loop() {
   redLedState = !redLedState;
  digitalWrite(RLED_PIN, redLedState);
  delay(R_HALF_PERIOD * 1e03);//semiperiodo a 1,5 secondi

}
