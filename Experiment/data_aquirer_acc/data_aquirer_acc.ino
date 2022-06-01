#include <Wire.h>
#include <math.h>
#include <CircularBuffer.h>

//Number of samples for gyroscope moving average
#define MOVING_AVERAGE_WINDOW 10 
//Time between MPU readings
#define MPU_READ_PERIOD 10

// I2C address of the MPU-6050
const int MPU_addr=0x68; 
//Accelerometer raw values
float AccX, AccY, AccZ;
//Accelerometer Moving Average
float MAAccX, MAAccY, MAAccZ;

//Sampling time control variables
float elapsedTime = 0.0, currentTime = 0.0, previousTime = 0.0, ledTime = 0.0;

//Buffers used to preserve the values for moving average calculation
CircularBuffer<float, MOVING_AVERAGE_WINDOW> x_buffer;
CircularBuffer<float, MOVING_AVERAGE_WINDOW> y_buffer;
CircularBuffer<float, MOVING_AVERAGE_WINDOW> z_buffer;

//Initialize MPU
void initMPU(){
  Wire.begin(22,23);
  Wire.beginTransmission(MPU_addr);
  // PWR_MGMT_1 register
  Wire.write(0x6B);
  //wakes up the MPU-6050)
  Wire.write(0x00);
  Wire.endTransmission(true);
  //start communication with MPU6050
  Wire.beginTransmission(MPU_addr);
  //set working register
  Wire.write(0x1B);
  //write to register
  Wire.write(0x00);
  // end communication
  Wire.endTransmission(true);
}

void readMPU(){
  //current time actual time read
  currentTime = millis();
  elapsedTime = (currentTime - previousTime);
  //previous time is stored before the actual time read
  previousTime = currentTime;        
  int t = (MPU_READ_PERIOD - elapsedTime) > 0 ? int(MPU_READ_PERIOD - elapsedTime) : 0;
  delay(t);
  
  //read acceleromter data 
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  //read 6 registers total, each axis value is stored in 2 registers
  Wire.requestFrom(MPU_addr, 6, true);
  //for a range of +-2g, we need to divide the raw values by 16384, according to the datasheet  
  //and for correct conversion you must use int16_t
  AccX = int16_t(Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = int16_t(Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = int16_t(Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
}


void prepareMAQueue(){
  for(int i=0;i<MOVING_AVERAGE_WINDOW;i++){
      readMPU();
      x_buffer.push(AccX);
      y_buffer.push(AccY);
      z_buffer.push(AccZ);
  }
}

void updateMAValues(){
  readMPU();
  x_buffer.push(AccX);
  y_buffer.push(AccY);
  z_buffer.push(AccZ);
  // the following ensures using the right type for the index variable
  using index_t = decltype(x_buffer)::index_t;
  MAAccX = MAAccY = MAAccZ = 0.0;
  for (index_t i = 0; i < MOVING_AVERAGE_WINDOW; i++) {
      MAAccX += x_buffer[i];
      MAAccY += y_buffer[i];
      MAAccZ += z_buffer[i];
  }
  // performance
  MAAccX = MAAccX/(float)MOVING_AVERAGE_WINDOW;
  MAAccY = MAAccY/(float)MOVING_AVERAGE_WINDOW;
  MAAccZ = MAAccZ/(float)MOVING_AVERAGE_WINDOW; 
}
 
void setup(){
  initMPU();
  prepareMAQueue();
  Serial.begin(115200);
}
 
void loop(){
  //calculate moving averages for gyroscope
  updateMAValues();
  
  //=== Send data (Serial) === //
  // accelerometer minus DC level
  Serial.print(AccX-MAAccX);
  Serial.print(";");
  Serial.print(AccY-MAAccY);
  Serial.print(";");
  Serial.print(AccZ-MAAccZ);
  Serial.print(";");
  Serial.println(); 
}
