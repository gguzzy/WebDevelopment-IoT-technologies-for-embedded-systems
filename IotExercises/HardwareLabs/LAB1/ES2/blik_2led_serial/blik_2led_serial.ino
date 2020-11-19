#include <TimerOne.h>
/*impostare volatile le variabili modificate da interrupt*/
const int RLED_PIN = 12;
const int GLED_PIN = 11;

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

volatile int greenLedState = LOW;
int redLedState = LOW;

void blinkGreen()
{
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}

void serialPrintStatus(){
  if(Serial.available() > 0){
    int inByte = Serial.read();//legge seriale
    if(inByte == '2')//richiesto stato del led verde
    {
      Serial.print("LED 2 Status: ");
      Serial.println(greenLedState);
    }
    else if(inByte == '3')//richiesto stato led rosso
    {
        
      Serial.print("LED 3 Status: ");
      Serial.println(redLedState);    
    }
    else//comando errato
      Serial.println("Error");

     Serial.flush();//pulisco seriale
  }
}

void setup() {
  Serial.begin(9600);
  while(!Serial);//avvio seriale e attendo che sia pronta
  Serial.println("Buongiorno");
  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);//abilito i due pin come uscita dei led
  Timer1.initialize(G_HALF_PERIOD * 1e06);//inizializzo il timer
  Timer1.attachInterrupt(blinkGreen);//attacco interrupt al timer per far brillare il led verde
    
}

void loop() {
   redLedState = !redLedState;
  digitalWrite(RLED_PIN, redLedState);
  delay(R_HALF_PERIOD * 1e03);
  serialPrintStatus();

}
