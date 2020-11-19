#include <LiquidCrystal_PCF8574.h>
#include <math.h>

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
const int timeout_sound = 1000;
const float T0 = 298.0;

volatile int FLAG_PIR = 0;
volatile int FLAG_SOUND = 0;
volatile float no_det_time_start = 0.0;
volatile float no_det_time_stop = 0.0;
volatile float moving_timer_PIR = 0.0;
volatile float actual_sound_sens = 0.0;
volatile int sound_sens_counter = 0;
volatile int presence = 0;
float m_fan = 0.0, q_fan = 0.0;
float m_ht = 0.0, q_ht = 0.0;
float temp = 0;

void LCDsetup(){
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("HOME AUTOMATION v1");  
}
void TEMP_SENSOR_setup(){//imposta parametri di potenza per i termostati
  
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

float getTemp()//fornisce temperatura in gradi Celsius
{
    float Vsig  = analogRead(TEMP_PIN);
    float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
    float T = 1.0/(log(R/R0)/B + 1/T0);
    T = T - 273.15;      
  return T;
}
void PIR_detection()//ISR del PIR
{
  int value = digitalRead(PIR_PIN);
 Serial.println(value);
  if(value == 1)
  {
    FLAG_PIR = 1;
  }
      
}
//ISR sensore di rumore
void SOUND_detection()//punto 4
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
/*void SOUND_detection()//punto 9 del lab
{
  int value = digitalRead(SOUND_PIN);
  Serial.println(value);
  if(value == 1)
  {
    float diff = millis() - actual_sound_sens;
    Serial.println(value);
    Serial.println(diff);
    if(FLAG_SOUND == 0)
    {
      FLAG_SOUND = 1;
      actual_sound_sens = millis();
    }
    else if(diff <= timeout_sound)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      FLAG_SOUND = 0;
    }
    else
    {
      FLAG_SOUND = 0;
    }
    
  }
}*/
float POWER_manager(float temperature, int flag)//
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

void checkElapsedTime()//controlla finestra temporale
{
  no_det_time_stop = millis() - no_det_time_start;
  float diff = moving_timer_PIR - no_det_time_stop;
    Serial.print("Actual");
    Serial.println(no_det_time_stop);
    Serial.print("Target");
    Serial.println(moving_timer_PIR);
    Serial.print(FLAG_PIR);
    Serial.print(",");
    Serial.println(FLAG_SOUND);
    Serial.println(diff);
    if(diff <= 0)
    {
      moving_timer_PIR = millis() + timeout_pir;
      no_det_time_start = millis(); 
      if(FLAG_PIR || FLAG_SOUND)
      {
        sound_sens_counter = 0;
        FLAG_PIR = 0;
        FLAG_SOUND = 0; //aggiungere provando punto 4
        presence = 1;
      }
      else
      {
        sound_sens_counter = 0;
        presence = 0;
      }          
    }
    
}

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
void setup() {
  LCDsetup();
  TEMP_SENSOR_setup();
  pinMode(TEMP_PIN, INPUT);
  Serial.begin(115200);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(60000);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), PIR_detection, RISING);
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), SOUND_detection, CHANGE);
  
  no_det_time_start = millis();
  moving_timer_PIR = millis() + timeout_pir;
  
}

void loop() {
  checkElapsedTime();
  if(presence)
  {
    AC_MIN_TEMP = 20.0;
    AC_MAX_TEMP = 25.0;
    HT_MAX_TEMP = 25.0;
    HT_MIN_TEMP = 23.0;
    if(temp != AC_MIN_TEMP)//ricalcola parametri di potenza dei termostati in caso di cambio dei set point
    {
      TEMP_SENSOR_setup();
      temp = AC_MIN_TEMP; 
    } 
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
  float cool_speed = POWER_manager(temperature, 1);//calcolo potenza da erogare alla ventola
  FAN_manager(cool_speed);//mando potenza alla ventola
  float ht_speed = POWER_manager(temperature, 0);//calcolo potenza da mandare al termostato
  HT_manager(ht_speed);//mando potenza al termostato
  cool_speed = cool_speed * 100 / 255;
  ht_speed = ht_speed * 100 / 255;
 // Serial.println(temperature);
  printFirstPage(temperature, cool_speed, ht_speed, presence);
  delay(5000);
  printSecondPage();
  delay(5000);
}
