#include "BMS_CUSTOM.h"
#include "lora_ra02.h"
#include <Arduino_JSON.h>

 String jsonString;
 
//system pin difinitions
#define magneticSwitch 34
#define ESCMospin 12
#define ESC_enable digitalWrite(ESCMospin, HIGH)
#define ESC_disable digitalWrite(ESCMospin, LOW)
#define ESC_PWM 13

//pwm output constants
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 16;
long updateTime = 0;
long timeout    = (10*60*1000); //10min timeout

    uint16_t totalVoltage = 0;
    uint16_t current = 0;
    uint16_t residualCapacity =0;
    uint16_t nominalCapacity = 0;
    uint16_t cycleLife = 0;
    uint8_t firstTemp = 0;
    int sendData = 0;
    int rcPotentiomenterVolt = 0;
    uint16_t PWMtoECu = 0;
    
    
void setup() {
  // put your setup code here, to run once:
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ESC_PWM, ledChannel);
  pinMode(magneticSwitch,INPUT);
  pinMode(ESCMospin,OUTPUT);
  
  initSerialinterface();
  init_lora();
  ESC_disable;
}
  
  const long getDataInterval  = 2*60*1000;
  long currentTime            = 0;
  const long sendDataInterval = 60*1000;
  long sendCurrentTime = 0;

void loop() {
  // put your main code here, to run repeatedly:
  while(millis() - currentTime > getDataInterval)
  {
    sendCommandToRS485(read_status, getBasicInfo);
  }
  if(receivedMessage)
  {
    //deserialize the received data to get the potentio-meter reading
    JSONVar myObject = JSON.parse(incoming.c_str());
 if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed! no json data");
    return;
  }
 
  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject)); // prints: object
  
   if (myObject.hasOwnProperty("PotVolt"))
    {
      Serial.print("myObject[\"PotADC\"] = ");
      String potADC= myObject["PotADC"]   ;
      rcPotentiomenterVolt = potADC.toInt();
      Serial.println(potADC);   
      Serial.print("Potentiomenter voltage");
      Serial.println(rcPotentiomenterVolt);
      PWMtoECu = map(rcPotentiomenterVolt, 0, 4095, 0, 65535);
      
       ledcWrite(ESC_PWM, PWMtoECu);
       ESC_enable;
       updateTime = millis();
    }    

  }
  uint8_t receivedData = getSerialData();
  
  if(in_processing && (receivedData!=0))
  {
    //process received data
   totalVoltage = ((rxBuff[0]<<8)|rxBuff[1]); //taking 0 & 1 convert to integer
    totalVoltage = totalVoltage/100;
    
    current =      ((rxBuff[2]<<8)|rxBuff[3]); 
    current = (65536-current)/100;
    
     residualCapacity = ((rxBuff[4]<<8)|rxBuff[5]);
     nominalCapacity =  ((rxBuff[6]<<8)|rxBuff[7]);
     cycleLife = ((rxBuff[8]<<8)|rxBuff[9]);
    //uint16_t productDate =
   // uint16_t balanceStatus = 
   // uint16_t balanceStatusHigh = 
    //uint16_t protectionStatus = 
    //uint8_t version = ((rxBuff[18]))
    uint8_t RSOC = (rxBuff[19]);
    uint8_t FET = (rxBuff[20]);
    uint8_t cellBlocks = (rxBuff[21]);
    uint8_t NTC_number = (rxBuff[22]);
     firstTemp  = (rxBuff[23]<<8) |  (rxBuff[24]<<8);
    //after processing we set the starting squence to recive more data
    state = find_start_byte;
    sendData = 1;
  }
  /*constructing json package for transport
  */
  if(sendData == 1){
    JSONVar myObject;
       myObject["totalVoltage"] = totalVoltage;
       myObject["current"]      =      current;
       myObject["residualCapacity"] = residualCapacity;
       myObject["nominalCapacity"] = nominalCapacity;
       myObject["cycleLife"] = cycleLife;
       myObject["firstTemp"] = firstTemp;
       
        Serial.print("my keys = ");
        Serial.println(myObject.keys());
        
         jsonString = JSON.stringify(myObject);
         Serial.print("JSON.stringify(myObject) = ");
         Serial.println(jsonString);
       
       
  //snprintf(outgoing, sizeof(outgoing), "()",totalVoltage,);
 
      
      sendData = 0;
  }
  if(millis() -sendCurrentTime>= sendDataInterval )
  {
    sendMessage(jsonString);
    sendCurrentTime = millis();
  }
  //handle the power on functionality and the pwm output functionality
  if(millis()-updateTime > timeout)
  {
     ESC_disable;
  }
}
/*
- radio connected
- we need to send data on battery level
- we receive the analog value from the potentiometer
- send analog value to the ouput pin
- magnetic switch monitor and turning on the speed controller mosfet
-
*/

