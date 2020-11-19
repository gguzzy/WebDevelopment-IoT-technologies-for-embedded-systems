#include <MQTTclient.h>

#include <LiquidCrystal_PCF8574.h>
#include <math.h>
#include <ArduinoJson.h>
#include <Bridge.h>
#include <Process.h>

#define MQTT_HOST "85.119.83.194" // test.mosquitto.org
const int capacity1 = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(4) + 40;
const int capacity2 = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 40;
DynamicJsonDocument doc_rec(capacity2);
DynamicJsonDocument doc_snd(capacity1);

float AC_MIN_TEMP = 22.0;
float AC_MAX_TEMP = 27.0;
float HT_MAX_TEMP = 25.0;
float HT_MIN_TEMP = 18.0;

#define LED_PIN 6
#define FAN_PIN 5
#define PIR_PIN 7
#define SOUND_PIN 1
#define TEMP_PIN A1

LiquidCrystal_PCF8574 lcd(0x27);
const float B = 4275.0;
const float R = 100000.0;
const float Vcc = 1023.0;
const int timeout_pir = 30000;
const float T0 = 298.0;

volatile int FLAG_PIR = 0;
volatile int FLAG_SOUND = 0;
volatile float no_det_time_start = 0.0;
volatile float no_det_time_stop = 0.0;
volatile float moving_timer_PIR = 0.0;
volatile int sound_sens_counter = 0;
volatile int presence = 0;
float temp = 0;
float m_fan = 0.0, q_fan = 0.0;
float m_ht = 0.0, q_ht = 0.0;


void LCDsetup(){
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("HOME AUTOMATION v1");  
}
void TEMP_SENSOR_setup(){//prepara valori 
  m_fan = (255.0)/(AC_MAX_TEMP - AC_MIN_TEMP);
  q_fan = AC_MIN_TEMP * m_fan;
  m_ht = (-255.0)/(HT_MAX_TEMP - HT_MIN_TEMP);
  q_ht = HT_MAX_TEMP * m_ht;
  
}
void FAN_manager(int fan_speed)
{
  analogWrite(FAN_PIN, fan_speed);
}
void HT_manager(int ht_power)
{
  analogWrite(LED_PIN, ht_power);
}

float getTemp()//fornisce valore di temperatura in gradi
{
    float Vsig  = analogRead(TEMP_PIN);
    float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
    float T = 1.0/(log(R/R0)/B + 1/T0);
    T = T - 273.15;      
  return T;
}
void PIR_detection()//ISR sensore PIR
{
  int value = digitalRead(PIR_PIN);
 Serial.println(value);
  if(value == 1)
  {
    FLAG_PIR = 1;
  }
      
}
void SOUND_detection()//ISR per sensore di rumore
{
    
  int value = digitalRead(SOUND_PIN);
   
  if(value == 1)
  {
    sound_sens_counter++;
    if(sound_sens_counter >= 30)
    {
      FLAG_SOUND = 1;
    }
    
  }
  Serial.print("numero di rilevamenti sonori:");
  Serial.println(sound_sens_counter);
}
float POWER_manager(float temperature, int flag)//calcola valore di potenza da erogare in base alla richiesta
{
  int power;
  if(flag == 1)
  {
    power = m_fan*temperature - q_fan;
  }
  else
  {
    power = abs(m_ht*temperature + q_ht);
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

void checkElapsedTime()//controlla finestra temporale per stabilire se ci sono persone o meno nella stanza
{
  no_det_time_stop = millis() - no_det_time_start;
  float diff = moving_timer_PIR - no_det_time_stop;
    /*Serial.print("Actual");
    Serial.println(no_det_time_stop);
    Serial.print("Target");
    Serial.println(moving_timer_PIR);
    Serial.print(FLAG_PIR);
    Serial.print(",");
    Serial.println(FLAG_SOUND);
    Serial.println(diff);*/
    if(diff <= 0)
    {
      moving_timer_PIR = millis() + timeout_pir;
      no_det_time_start = millis(); 
      if(FLAG_PIR || FLAG_SOUND)//controllo incrociato
      {
        sound_sens_counter = 0;
        FLAG_PIR = 0;
        FLAG_SOUND = 0;
        presence = 1;
      }
      else
      {
        sound_sens_counter = 0;
        presence = 0;
      }          
    }
    
}


void setup() {
  LCDsetup();
  pinMode(TEMP_PIN, INPUT);
  TEMP_SENSOR_setup();
  Serial.begin(9600);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(60000);//caricamento sensore PIR
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), PIR_detection, RISING);//collego pin del pir alla ISR
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), SOUND_detection, CHANGE);//collego pin del sensore di rumore alla ISR
  
  no_det_time_start = millis();
  moving_timer_PIR = millis() + timeout_pir;
  Bridge.begin();
  mqtt.begin(MQTT_HOST, 1883);
  
  mqtt.subscribe("test/mqttclient/led/#", setLedValue);
  mqtt.subscribe("test/mqttclient/message/#", msgPrinter);
  
}

void loop() {
  mqtt.monitor();
  checkElapsedTime();
  if(presence)//in caso di presenza i valori di temperatura cambiano
  {
    AC_MIN_TEMP = 20.0;
    AC_MAX_TEMP = 25.0;
    HT_MAX_TEMP = 25.0;
    HT_MIN_TEMP = 23.0;
    if(temp != AC_MIN_TEMP)
    {
      TEMP_SENSOR_setup();
      temp = AC_MIN_TEMP;
    } //ricalcola parametri di potenza dei termostati
  }
  else
  {    
    AC_MIN_TEMP = 22.0;
    AC_MAX_TEMP = 27.0;
    HT_MAX_TEMP = 25.0;
    HT_MIN_TEMP = 18.0;
    if(temp != AC_MIN_TEMP)
    {
      TEMP_SENSOR_setup();
      temp = AC_MIN_TEMP;
    } 
    
       
  }
  float temperature = getTemp();  
  float cool_speed = POWER_manager(temperature, 1);//calcola valore da mandare alla ventola
  FAN_manager(cool_speed);//imposta valore ventola
  float ht_speed = POWER_manager(temperature, 0);//calcola valore riscaldamento
  HT_manager(ht_speed);//imposta valore riscaldamento
  cool_speed = cool_speed * 100 / 255;
  ht_speed = ht_speed * 100 / 255;
 // Serial.println(temperature);

  infoSender();
}

//sharing part
String senMlEncode()//prepara file json da inviare
{
  doc_snd.clear();
  doc_snd["bn"] = "Yun";
  doc_snd["e1"][0]["u"] = "Cel";
  doc_snd["e1"][0]["v"] = getTemp();
  doc_snd["e1"][0]["t"] = millis();
  if(presence)
  {
    doc_snd["e2"][0]["p"] = true;
  }
  else
  {
    doc_snd["e2"][0]["p"] = false;
  }
  
  doc_snd["e3"][0]["n"] = sound_sens_counter;

  String output;
  serializeJson(doc_snd, output);
  return output;
}
void infoSender()//manda messaggio
{
  String message = senMlEncode();
  mqtt.publish("test/mqttclient/log", message);
}
void setLedValue(const String& topic, const String& subtopic, const String& message) { 
  
  DeserializationError err = deserializeJson(doc_rec, message);

  if(err)
  {
    Serial.print(F("deserializeJson() failed with code: "));
    Serial.println(err.c_str());
  }
  else
  {
    if(doc_rec["e"][0]["n"] == "led")//controlla nome del json per acquisire il valore del led
    {
      int value = doc_rec["e"][0]["v"];
      if(value == 1 || value == 0){
        digitalWrite(LED_BUILTIN, value);
      }
    }
  }
  
}

void msgPrinter(const String& topic, const String& subtopic, const String& message)//stampa messaggio a video
{
    lcd.clear();
    lcd.setCursor(0, 0);
    DeserializationError err = deserializeJson(doc_rec, message);

  if(err)
  {
    Serial.print(F("deserializeJson() failed with code: "));
    Serial.println(err.c_str());
  }
  else
  {
      lcd.print(message); 
  }
    
}
