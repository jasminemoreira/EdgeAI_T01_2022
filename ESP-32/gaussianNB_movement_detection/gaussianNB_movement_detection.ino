#include "nb.h"
#include "gyro.h"

void setup(){
  startGyro();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}
 
void loop(){
  updateGyro();
  
  if(predict(&AADM) == 1){
    digitalWrite(LED_BUILTIN,HIGH);
    delay(100);
  }else{
    digitalWrite(LED_BUILTIN,LOW);
  } 
}
