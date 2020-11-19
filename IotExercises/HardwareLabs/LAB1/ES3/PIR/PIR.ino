const int LED_PIN = 6;
const int PIR_PIN = 0;

volatile int tot_count = 0;

void setup() {
  Serial.begin(9600);  
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Initialization...");
  delay(60000);//ritardo necessario alla corretta attivazione del sensore
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, CHANGE);//collego isr al pin del sensore e imposto l'ascolto su entrambi i fronti  
  
  Serial.println("Ready");
}

void loop() {
   
  Serial.print("Rilevamenti: ");
  Serial.println(tot_count);
  delay(30000);
  
}

void checkPresence()//isr del pir
{
  if(digitalRead(PIR_PIN) == 1)
  {
    tot_count++; 
    Serial.println(tot_count);
  }
  int led_val = digitalRead(PIR_PIN);
  digitalWrite(LED_PIN, led_val);//comunico valore letto dal sensore tramite l'accensione o lo spegnimento del led
   
}
