#include "lora_ra02.h"
#include <Arduino_JSON.h>

//pin difinitions
#define p90_led 15
#define p75_led 33
#define p50_led 32
#define p25_led 17
#define p5_led 16
#define display_pin 14

#define PotPin 25
#define DisplayOn digitalWrite(display_pin, HIGH);
#define DisplayOff digitalWrite(display_pin, LOW);

#define buzzer 26
#define radio_led 27
void setup() {
  // put (your setup code here, to run once:
  pinMode(buzzer, OUTPUT);
  
  pinMode(p90_led, OUTPUT);
  pinMode(p75_led, OUTPUT);
  pinMode(p50_led, OUTPUT);
  pinMode(p25_led, OUTPUT);
  pinMode(p5_led, OUTPUT);
  
  pinMode(radio_led, OUTPUT);
  
  pinMode(display_pin, OUTPUT);
  pinMode(PotPin, INPUT);
  
  DisplayOff;
}
float totalVoltage;
float nominalCapacity;

long currentTime = 0;
long intervalTime = 60*4*1000;
long receivedTime =0;
void loop() {
  // put your main code here, to run repeatedly:
   if(receivedMessage )
  {
    digitalWrite(radio_led, HIGH);
    DisplayOn;
    receivedTime =millis();
    //deserialize the received data to get the potentio-meter reading
    JSONVar myObject = JSON.parse(incoming.c_str());
    if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed! no json data");
        return;
  }
  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject)); // prints: object
  
   if (myObject.hasOwnProperty("totalVoltage"))
    {
      Serial.print("myObject[\"totalVoltage\"] = ");
      String voltage= myObject["totalVoltage"]   ;
      totalVoltage = atof(voltage.c_str());
      Serial.println(totalVoltage);   
      Serial.print("Battery capacity");
      Serial.println(totalVoltage);
      
  }
  if (myObject.hasOwnProperty("nominalCapacity"))
    {
      Serial.print("myObject[\"nominalCapacity\"] = ");
      String capacity= myObject["nominalCapacity"]   ;
      nominalCapacity = atof(capacity.c_str());
      Serial.println(nominalCapacity);   
      Serial.print("Battery capacity");
      Serial.println(nominalCapacity);
      
    }
    
  }
  if(millis() - currentTime >= intervalTime )
  {
    float BatteryValue = map(totalVoltage,45.44,50.92,0,100);
    if(BatteryValue>=90)
    {
      digitalWrite(p90_led, HIGH);
      digitalWrite(p75_led, LOW);
      digitalWrite(p50_led, LOW);
      digitalWrite(p25_led, LOW);
      digitalWrite(p5_led, LOW);
      
       digitalWrite(buzzer, HIGH);
        delay(500);
        digitalWrite(buzzer, LOW);
        
    }
    else if(BatteryValue>=75 && BatteryValue<90)
    {
      digitalWrite(p90_led, LOW);
      digitalWrite(p75_led, HIGH);
      digitalWrite(p50_led, LOW);
      digitalWrite(p25_led, LOW);
      digitalWrite(p5_led, LOW);
      
      digitalWrite(buzzer, HIGH);
        delay(500);
        digitalWrite(buzzer, LOW);
    }
    else if(BatteryValue>=50 && BatteryValue<75)
    {
      digitalWrite(p90_led, LOW);
      digitalWrite(p75_led, LOW);
      digitalWrite(p50_led, HIGH);
      digitalWrite(p25_led, LOW);
      digitalWrite(p5_led, LOW);
      
      digitalWrite(buzzer, HIGH);
        delay(500);
        digitalWrite(buzzer, LOW);
    }
    else if(BatteryValue>=25 && BatteryValue<50)
    {
      digitalWrite(p90_led, LOW);
      digitalWrite(p75_led, LOW);
      digitalWrite(p50_led, LOW);
      digitalWrite(p25_led, HIGH);
      digitalWrite(p5_led, LOW);
      
      digitalWrite(buzzer, HIGH);
        delay(500);
        digitalWrite(buzzer, LOW);
    }
    else if(BatteryValue>=5 && BatteryValue<25)
    {
      digitalWrite(p90_led, LOW);
      digitalWrite(p75_led, LOW);
      digitalWrite(p50_led, LOW);
      digitalWrite(p25_led, LOW);
      digitalWrite(p5_led, HIGH);  
      
      digitalWrite(buzzer, HIGH);
        delay(500);
        digitalWrite(buzzer, LOW);    
    }
    else{
      digitalWrite(p90_led, LOW);
      digitalWrite(p75_led, LOW);
      digitalWrite(p50_led, LOW);
      digitalWrite(p25_led, LOW);
      digitalWrite(p5_led, HIGH);
      buzzerBeep();
    }
    //construc the pot voltage object
    analogRead(PotPin);
    
     JSONVar myObject;
     myObject["totalVoltage"] = PotPin;
     
     Serial.print("my keys = ");
        Serial.println(myObject.keys());
        
        String jsonString = JSON.stringify(myObject);
         Serial.print("JSON.stringify(myObject) = ");
         Serial.println(jsonString);
         
     sendMessage(jsonString);
  }
  if(millis()-receivedTime >=(2*60*1000))
  {
    DisplayOff;
  }
}
void buzzerBeep(){
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(2000);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(2000);
  
  return;
}
