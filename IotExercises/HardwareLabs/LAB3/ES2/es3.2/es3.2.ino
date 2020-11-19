#include <Process.h>
#include <Bridge.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>
const float Vcc = 1023.0;
const float B = 4275.0;
const float R = 100000.0;
const float T0 = 298.0;
#define TEMP_PIN A1
#define url "http://192.168.1.6:9001/log"
volatile int error_t = 0;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);


void setup() {
  Serial.begin(9600);
  pinMode(TEMP_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);

}

void loop() {
  String output = senMlEncode("temperature", 0, "Cel");//creo json da inviare
  Serial.println(output);
  if(error_t != 1)
  {
    Serial.println(postRequest(output));
  }
  else{
    Serial.println("Errore nella lettura del dato");
    error_t = 0;
  }
  
  delay(10000);
}


float getTemp()//acquisisco temperatura in C
{
    float Vsig  = analogRead(TEMP_PIN);
    float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
    float T = 1.0/(log(R/R0)/B + 1/T0);
    T = T - 273.15;      
  return T;
}

int postRequest(String data){//creo richiesta
  Process p;
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Content-Type: application/json");
  p.addParameter("-X");
  p.addParameter("POST");
  p.addParameter("-d");
  p.addParameter(data);
  p.addParameter(url);
  p.run();

  return p.exitValue();
}
String senMlEncode(String res, float v, String unit)//creo e formatto json da inviare
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
      if(v < 50 || v > -20)//controllo integrità del sensore
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
