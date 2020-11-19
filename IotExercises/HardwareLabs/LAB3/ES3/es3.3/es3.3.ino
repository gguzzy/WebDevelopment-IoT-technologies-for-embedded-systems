#include <MQTTclient.h>
#include <Process.h>
#include <ArduinoJson.h>

#define my_base_topic "/tiot/0"
#define MQTT_HOST "test.mosquitto.org" // 
#define TEMP_PIN A1
const float Vcc = 1023.0;
const float B = 4275.0;
const float R = 100000.0;
const float T0 = 298.0;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_rec(capacity);
DynamicJsonDocument doc_snd(capacity);

volatile int error_t = 0;

void setup() {
    
  // start serial
  Serial.begin(9600);
  // remember the bridge!
  Bridge.begin();
  // begin the client library (initialize host)
  mqtt.begin(MQTT_HOST, 1883);
  pinMode(TEMP_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // make some subscriptions
  mqtt.subscribe(my_base_topic + String("/led"), setLedValue);

}

void loop() {
  mqtt.monitor();
  String message = senMlEncode("temperature", getTemp(), "Cel");//genera json con temperatura
  if(error_t == 0)
  {
    mqtt.publish(my_base_topic + String("/temperature"), message);
  }
  else
  {
    error_t = 0;
    Serial.println("Errore nella lettura del sensore");
  }
  delay(1000);
  
}

float getTemp()//fornisce dato sulla temperatura in C
{
    float Vsig  = analogRead(TEMP_PIN);
    float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
    float T = 1.0/(log(R/R0)/B + 1/T0);
    T = T - 273.15;      
  return T;
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
    if(doc_rec["e"][0]["n"] == "led")//estrapola dal json in arrivo il valore del led
    {
      int value = doc_rec["e"][0]["v"];
      if(value == 1 || value == 0){
        digitalWrite(LED_BUILTIN, value);
      }
    }
  }
  
}
String senMlEncode(String res, float v, String unit)//crea json da inviare
{
  doc_snd.clear();
  doc_snd["bn"] = "Yun";
  if(unit != "")
  {
    doc_snd["e"][0]["u"] = unit;
  }
  else{
    doc_snd["e"][0]["u"] = (char*)NULL;
    error_t = 1;
  }
  if(res != "")
  {
      doc_snd["e"][0]["n"] = res;
      if(v < 50 || v > -20)
      {
        doc_snd["e"][0]["v"] = getTemp();
      }
      else
      {
        error_t = 1;
      }
  }
  doc_snd["e"][0]["t"] = millis();
  

  String output;
  serializeJson(doc_snd, output);
  return output;
}
