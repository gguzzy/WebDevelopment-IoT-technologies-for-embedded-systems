#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>
const float Vcc = 1023.0;
const float B = 4275.0;
const float R = 100000.0;
const float T0 = 298.0;
#define TEMP_PIN A1

const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;//calcolo memoria necessaria al file json
DynamicJsonDocument doc_snd(capacity);
volatile int error_t = 0;
BridgeServer server;


void setup() {
  Serial.begin(9600);
  pinMode(TEMP_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Bridge.begin();
  digitalWrite(LED_BUILTIN, HIGH);
  server.listenOnLocalhost();
  server.begin();
  Serial.println("Server in attesa...");
}

void loop() {
  BridgeClient client = server.accept();//in attesa di connessioni
 
  if(client){
    //Serial.println(client.readString());
    process(client);
    client.stop();
  }
  delay(50);

}

float getTemp()
{
    float Vsig  = analogRead(TEMP_PIN);
    float R0 = ((float)(Vcc/Vsig) - 1.0)*R;
    float T = 1.0/(log(R/R0)/B + 1/T0);
    T = T - 273.15;      
  return T;
}

void process(BridgeClient client)//gestisce riciesta client
{
  String command = client.readStringUntil('/');
  command.trim();
   Serial.println(command);
    if(command == "led"){//controlla che richiesta elaborata correttamente
      int val = client.parseInt();
      if(val == 0 || val == 1)
      {
        digitalWrite(LED_BUILTIN, val);
        printResponse(client, 200, senMlEncode(F("led"), val, F("")));//fornisco risposta tramite creazione di un json
      }
      else{
       printResponse(client, 200, "Error 404, Not found. The server has not found anithing matching the Request-URI");   
     }
    }
   else if(command == "temperature")
   {
       int val = analogRead(TEMP_PIN);
       String output = senMlEncode(F("temperature"), 0, F("Cel"));
       
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
   else
   {
    printResponse(client, 200, "Error 400, bad request. Try to perform again with another URI");
   }

}
void printResponse(BridgeClient client, int code, String body){
  client.println("Status: " + String(code));
  if(code == 200){
    client.println(F("Content-type:application-json; charset=utf-8"));
    client.println();
    client.println(body);
  }
}
String senMlEncode(String res, float v, String unit)//creo json
{
  doc_snd.clear();//pulisco documento
  doc_snd["bn"] = "Yun";//setto nome
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
  

  String output;
  serializeJson(doc_snd, output);//preparo json per la trasmissione
  return output;
}
