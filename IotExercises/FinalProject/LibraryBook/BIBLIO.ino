//setup e loop in fondo alla pagina


#include <LiquidCrystal_PCF8574.h>
#include <math.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>

float AC_MIN_TEMP = 22.0;
float AC_MAX_TEMP = 27.0;
float HT_MAX_TEMP = 25.0;
float HT_MIN_TEMP = 18.0;

#define LED_PIN 6
#define FAN_PIN 5
#define PIR_PIN_IN 7
#define PIR_PIN_OUT 8
#define SOUND_PIN 10
#define TEMP_PIN A1

LiquidCrystal_PCF8574 lcd(0x27);
const float B = 4275.0;
const float R = 100000.0;
const float Vcc = 1023.0;
const int timeout_noise = 20000;
const int timeout_print = 5000;
const float T0 = 298.0;

volatile int FLAG_PIR_IN = 0;
volatile int FLAG_PIR_OUT = 0;
volatile int tot_person = 0;
volatile int person_in, person_out = 0;
volatile int FLAG_SOUND = 0;
volatile int FLAG_S = 0;
volatile float det_time_start = 0.0;
volatile int sound_sens_counter = 0;
volatile int presence = 0;
volatile int error_t = 0;

int flag = 1;
int reserved = 0;//posti prenotati
float m_fan = 0.0, q_fan = 0.0;
float m_ht = 0.0, q_ht = 0.0;
float actual_time = 0.0;

const int capacity1 = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(6) + 40;//calcolo memoria necessaria al file json
DynamicJsonDocument doc_snd(capacity1);

BridgeServer server;

void LCDsetup(){
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("BIBLIO");  
}
void TEMP_SENSOR_setup(){//calcola parametri per una regolazione lineare della temperatura
  pinMode(TEMP_PIN, INPUT);
  m_fan = (255.0)/(AC_MAX_TEMP - AC_MIN_TEMP);
  q_fan = AC_MIN_TEMP * m_fan;
  m_ht = (-255.0)/(HT_MAX_TEMP - HT_MIN_TEMP);
  q_ht = HT_MAX_TEMP * m_ht;
  
}
void FAN_manager(int fan_speed)//regola ventola
{
  analogWrite(FAN_PIN, fan_speed);
}
void HT_manager(int ht_power)//regola riscaldamento
{
  analogWrite(LED_PIN, ht_power);
}

float getTemp()
{
    float Vsig  = analogRead(TEMP_PIN);
    float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
    float T = 1.0/(log(R/R0)/B + 1/T0);
    T = T - 273.15;      
  return T;
}
void PIR_IN_detection()//ISR collegata al PIR d'ingresso
{
  int value = digitalRead(PIR_PIN_IN);
  Serial.print("IN: ");
  Serial.println(value);
  if(value == 1)
  {    
    int diff = 20 - tot_person - reserved;
    if(diff < 1)
    {
      FLAG_PIR_IN = 1;
    }
    tot_person++;
  }
      
}
void PIR_OUT_detection()//collegata al PIR di uscita
{
  int value = digitalRead(PIR_PIN_OUT);
  
  if(value == 1 && FLAG_PIR_OUT != 1)
  {
    if(tot_person > 0)
    {
      tot_person--;
      FLAG_PIR_OUT = 1;
      Serial.print("OUT: ");
      Serial.println(value);
    }
  }
  if(value == 0)
  {
    if(FLAG_PIR_OUT == 1)
    {
      FLAG_PIR_OUT = 0;
    }
  }
      
}
void SOUND_detection()//ISR collegata al sensore di rumore
{
    
  int value = digitalRead(SOUND_PIN);
  
  if(value == 0 && FLAG_S != 1)
  {
    Serial.println(sound_sens_counter);
    float det_time_stop = millis() - det_time_start;
    if(sound_sens_counter == 0)
    {
      sound_sens_counter++;
      det_time_start = millis(); 
    }
    else
    {
      
      if(det_time_stop <= timeout_noise)//se la finestra temporale non si è chiusa, procedi
      {
        
        if(sound_sens_counter <= 10) //10 eventi, limite entro il quale non viene notificato ai presenti in sala di fare silenzio
        {
          sound_sens_counter++;          
        }
        else
        {
          FLAG_SOUND = 1;
        }
       
      }
      else
      {        
        if(sound_sens_counter > 10)
        {
          FLAG_SOUND = 1;
        }
        sound_sens_counter = 0;
      }   
    }
    FLAG_S = 1;
    Serial.println(sound_sens_counter);
  }
  else if(value == 1)
  {
    FLAG_S = 0;
  }
  /*Serial.print("numero di rilevamenti sonori:");
  Serial.println(sound_sens_counter);*/
}
float POWER_manager(float temperature, int flag)//gestione erogazione potenza alla ventola o al led
{
  int power;
  if(flag == 1)
  {
    power = m_fan*temperature - q_fan;
  }
  else
  {
    power = m_ht*temperature - q_ht;
  }
 
  if(power > 255)
  {
    power = 255;
  }
  if(power < 0)
  {
    power = 0;
  }
  return power;
}


//le seguenti due funzioni regolano la stampa dei dati su display lcd
void printFirstPage(float temperature, float cool_speed, float ht_speed, int presence)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature,1);
  lcd.print(" Pres: ");
  lcd.print(presence);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("AC:");
  lcd.print(cool_speed, 1);
  lcd.print("% ");
  lcd.print("HT:");
  lcd.print(ht_speed, 1);
  lcd.print("%");
}
void printSecondPage()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AC m:");
  lcd.print(AC_MIN_TEMP, 1);
  lcd.print(" M:");
  lcd.print(AC_MAX_TEMP, 1);
  lcd.setCursor(0, 1);
  lcd.print("HT m:");
  lcd.print(HT_MIN_TEMP, 1);
  lcd.print(" M:");
  lcd.print(HT_MAX_TEMP, 1);
}

void process(BridgeClient client)//processa il client in base alla richiesta e controlla se essa sia valida
{
  String output;
  String command = client.readStringUntil('/');
  command.trim();
   Serial.println(command);
    if(command == "biblio")//fornisce prospetto sulla biblioteca
   {
       int val = analogRead(TEMP_PIN);
       output = senMlEncode(F("temperature"), 0, F("Cel"), 1);
       
       if(error_t == 1)
       {
        error_t = 0;
        printResponse(client, 200, "Error 500, internal server error. Try later.");
       }
       else
       {
        printResponse(client, 200, output);
       }
   }
   else if(command == "booking")//fornisce info sui posti disponibili (20 posti al massimo, c'è il virus)
   {
    if((tot_person + reserved) < 20)
    {
      output = senMlEncode(F("availability"), 1, "null", 0);
      reserved++;
    }
    else
    {
      output = senMlEncode(F("availability"), 0, "null", 0);
    }
    printResponse(client, 200, output);
   }
   else
   {
    printResponse(client, 200, "Error 400, bad request. Try to perform again with another URI");
   }
}

void printResponse(BridgeClient client, int code, String body){//invia risposta al client
  client.println("Status: " + String(code));
  if(code == 200){
    client.println(F("Content-type:application-json; charset=utf-8"));
    client.println();
    client.println(body);
  }
}

String senMlEncode(String res, float v, String unit, int flag)//prepara il file json. IL flag regola se inviare tutti i dati o solo quelli per la prenotazione
{
  doc_snd.clear();//pulisco documento
  doc_snd["bn"] = "Yun";//setto nome
  if(flag == 1)
  {
    if(unit != "")
    {
     doc_snd["e"][0]["u"] = unit;
   }
   else{
     doc_snd["e"][0]["u"] = (char*)NULL;
   }
   if(res != "")
   {
    if(res == "temperature")
     {
       doc_snd["e"][0]["n"] = res;
       if(v < 50 || v > -20)//controllo che il sensore funzioni
        {
         doc_snd["e"][0]["v"] = getTemp();
       }
       else
       {
         error_t = 1;
       }
     }
    }
  
   doc_snd["e"][0]["t"] = millis();

    doc_snd["e1"][0]["n"] = "persons";
    doc_snd["e1"][0]["p"] = tot_person;//persone nella biblioteca
    doc_snd["e1"][0]["r"] = reserved;//posti riservati
    doc_snd["e1"][0]["max_val"] = 20;//capienza massima
  }
  else
  {
    doc_snd["e2"][0]["n"] = res;
    if((int)v == 1)
    {
      doc_snd["e2"][0]["v"] = "true";
    }
    else
    {
      doc_snd["e2"][0]["v"] = "false";
    }
    int diff = 20 - tot_person - reserved;
    if(diff > 0)
    {
      doc_snd["e2"][0]["s"] = diff;
    }
    else
    {
      doc_snd["e2"][0]["s"] = 0;
    }
    
    doc_snd["e2"][0]["u"] = unit;
    doc_snd["e2"][0]["t"] = millis();
  }
  

  String output;
  serializeJson(doc_snd, output);//preparo json per la trasmissione
  return output;
}

void setup() {
  LCDsetup();
  TEMP_SENSOR_setup();
  Serial.begin(9600);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PIR_PIN_IN, INPUT);
  pinMode(PIR_PIN_OUT, INPUT);
  pinMode(SOUND_PIN, INPUT);
  Bridge.begin();
  delay(60000);//tempo di avvio del sensore PIR
  attachInterrupt(digitalPinToInterrupt(PIR_PIN_IN), PIR_IN_detection, RISING);
  server.listenOnLocalhost();
  server.begin();
  Serial.println("Server avviato..");
  actual_time = millis();//serve per far scorrere correttamente le pagine sul display
}

void loop() {
  
  BridgeClient client = server.accept();
  SOUND_detection();//controllo sensore di rumore
  PIR_OUT_detection();//controllo sensore pir di uscita
  if(client)
  {
      process(client);
      client.stop();
  }
  
  if(tot_person > 0)
  {
    presence = 1;
  }
  else
  {
    presence = 0;
  }
  if(presence)
  {
    AC_MIN_TEMP = 20.0;
    AC_MAX_TEMP = 25.0;
    HT_MAX_TEMP = 25.0;
    HT_MIN_TEMP = 23.0;
  }
  else
  {
    AC_MIN_TEMP = 22.0;
    AC_MAX_TEMP = 27.0;
    HT_MAX_TEMP = 25.0;
    HT_MIN_TEMP = 18.0;    
  }
  float temperature = getTemp();  
  float cool_speed = POWER_manager(temperature, 1);
  FAN_manager(cool_speed);
  float ht_speed = POWER_manager(temperature, 0);
  HT_manager(ht_speed);
  cool_speed = cool_speed * 100 / 255;
  ht_speed = ht_speed * 100 / 255;
 // Serial.println(temperature);
 float diff = millis() - actual_time;//se passati 5 secondi cambio visualizzazione su lcd
 if(diff >= timeout_print)
 {
    
   if(FLAG_SOUND)//irrompe in caso di eccessivo rumore
   {
     lcd.clear();
     lcd.setCursor(0, 0);
     lcd.print("BE QUIET!");
     FLAG_SOUND = 0;
   }
   else if(FLAG_PIR_IN == 1)
   {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BIBLIO FULL");
    FLAG_PIR_IN = 0;
   }
   else if(flag == 1)
   {
     printFirstPage(temperature, cool_speed, ht_speed, tot_person);
     flag = 0;
   }
   else if(flag == 0)
   {
     printSecondPage();
     flag = 1;
   }
   actual_time = millis();
 }
}
