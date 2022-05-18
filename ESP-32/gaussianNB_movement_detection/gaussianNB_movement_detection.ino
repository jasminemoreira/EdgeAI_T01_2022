#include "nb.h"
#include "jgyro.h"

Eloquent::ML::Port::GaussianNB gnb;
JazzTinyML::JGyro gyro;

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}
 
void loop(){
  gyro.updateAAD();
  
  if(gnb.predict(&gyro.AADM) == 1){
    digitalWrite(LED_BUILTIN,HIGH);
    delay(100);
  }else{
    digitalWrite(LED_BUILTIN,LOW);
  }
}
