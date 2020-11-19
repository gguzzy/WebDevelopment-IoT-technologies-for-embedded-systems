#define FAN 5
int current_speed = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(FAN, OUTPUT);//inizializzo pin come uscita
  analogWrite(FAN, (int)current_speed);//imposto dc a 0
  Serial.begin(9600);
  
  
}

void loop() {
  // put your main code here, to run repeatedly:

  char value = Serial.read();
  if(value == '+')
  {
    if(current_speed < 255)
    {
      current_speed += 5;
    }
    else
    {
      current_speed = 255;
    }
    analogWrite(FAN, current_speed);//scrivo nuovo valore della potenza mandata al motore
    Serial.print("Fan speed: ");
    Serial.println(current_speed);
    delay(20);
    
  }
  else if(value == '-')
  {
    if(current_speed > 0)
    {
      current_speed -= 5;
    }
    else
    {
      current_speed = 0;
    }
    analogWrite(FAN, current_speed);
    Serial.print("Fan speed: ");
    Serial.println(current_speed);
    delay(20);
  }

}
