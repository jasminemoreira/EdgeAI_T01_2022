#include <Wire.h>
#include <math.h>
#include <CircularBuffer.h>

//Number of samples for gyroscope moving average
#define MOVING_AVERAGE_WINDOW 10 
//Time between MPU readings
#define MPU_READ_PERIOD 20
//Values for manual offset removing (obtained graphically on SerialPlot)
//#define X_OFFSET 8.2
//#define Y_OFFSET 0.3
//#define Z_OFFSET 0.8

// I2C address of the MPU-6050
const int MPU_addr=0x68; 
//Accelerometer raw values
float AccX, AccY, AccZ;
//Gyroscope raw values
float GyroX, GyroY, GyroZ;
//Gyroscope Moving Average
float MAGyroX, MAGyroY, MAGyroZ;
// Absolute Angular Drift 
// sum of absolute delta values for each sample --> abs(sample-mean)
float AADX, AADY, AADZ, AADM; 

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
   
  //read gyro data
  Wire.beginTransmission(MPU_addr);
  // Gyro data first register address 0x43
  Wire.write(0x43);
  Wire.endTransmission(false);
  // Read 4 registers total, each axis value is stored in 2 registers
  Wire.requestFrom(MPU_addr, 6, true);
  //for a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroX = int16_t(Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = int16_t(Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = int16_t(Wire.read() << 8 | Wire.read()) / 131.0;
}


void prepareMAQueue(){
  for(int i=0;i<MOVING_AVERAGE_WINDOW;i++){
      readMPU();
      x_buffer.push(GyroX);
      y_buffer.push(GyroY);
      z_buffer.push(GyroZ);
  }
}

void updateMAValues(){
  readMPU();
  x_buffer.push(GyroX);
  y_buffer.push(GyroY);
  z_buffer.push(GyroZ);
  // the following ensures using the right type for the index variable
  using index_t = decltype(x_buffer)::index_t;
  MAGyroX = MAGyroY = MAGyroZ = 0.0;
  for (index_t i = 0; i < MOVING_AVERAGE_WINDOW; i++) {
      MAGyroX += x_buffer[i];
      MAGyroY += y_buffer[i];
      MAGyroZ += z_buffer[i];
  }
  // performance
  MAGyroX = MAGyroX/(float)MOVING_AVERAGE_WINDOW;
  MAGyroY = MAGyroY/(float)MOVING_AVERAGE_WINDOW;
  MAGyroZ = MAGyroZ/(float)MOVING_AVERAGE_WINDOW; 
  
  AADX = AADY = AADZ = AADM = 0.0; 
  int dx, dy, dz;
  for (index_t i = 0; i < MOVING_AVERAGE_WINDOW; i++) {
      dx = x_buffer[i]-MAGyroX;
      dy = y_buffer[i]-MAGyroY;
      dz = z_buffer[i]-MAGyroZ;
      AADX += abs(dx);
      AADY += abs(dy);
      AADZ += abs(dz);
  }
  AADM = sqrt(sq(AADX)+sq(AADY)+sq(AADZ));
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
  
  // raw accelerometer
  Serial.print(AccX);
  Serial.print(";");
  Serial.print(AccY);
  Serial.print(";");
  Serial.print(AccZ);
  Serial.print(";");
  Serial.println(); 

  // raw gyroscope 
  Serial.print(GyroX);
  Serial.print(";");
  Serial.print(GyroY);
  Serial.print(";");
  Serial.print(GyroZ);
  Serial.print(";");

  //gyroscope moving average === 
  Serial.print(MAGyroX);
  Serial.print(";");
  Serial.print(MAGyroY);
  Serial.print(";");
  Serial.print(MAGyroZ);
  Serial.print(";");
  
  //gyroscope minus moving average
  //aproximately gyroscope minus offset 
  Serial.print(GyroX - MAGyroX);
  Serial.print(";");
  Serial.print(GyroY - MAGyroY);
  Serial.print(";");
  Serial.print(GyroZ - MAGyroZ);
  Serial.print(";");
  
  //absolute angular drift
  Serial.print(AADX);
  Serial.print(";");
  Serial.print(AADY);
  Serial.print(";");
  Serial.print(AADZ); 
  Serial.print(";");
  Serial.print(AADM);
  Serial.println(); 
 
}
