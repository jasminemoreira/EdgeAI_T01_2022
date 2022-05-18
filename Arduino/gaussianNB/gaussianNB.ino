//#include <LowPower.h>
#include <Wire.h>
#include <Math.h>
#include <CircularBuffer.h>

#define MOVING_AVERAGE_WINDOW 10 
#define MPU_READ_PERIOD 20  

const int MPU_addr=0x68; // I2C address of the MPU-6050

float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ, MAGyroX, MAGyroY, MAGyroZ, AADX, AADY, AADZ, AADM; // Absolute Angular Drift

float elapsedTime = 0.0, currentTime = 0.0, previousTime = 0.0, ledTime = 0.0;

CircularBuffer<float, MOVING_AVERAGE_WINDOW> x_buffer;
CircularBuffer<float, MOVING_AVERAGE_WINDOW> y_buffer;
CircularBuffer<float, MOVING_AVERAGE_WINDOW> z_buffer;

// Gaussian Naive Bayes Model
int predict(float *x) {
    float votes[2] = { 0.0f };
    float theta[1] = { 0 };
    float sigma[1] = { 0 };
    theta[0] = 0.0;
    sigma[0] = 5.6356e-08;
    votes[0] = 0.5 - gauss(x, theta, sigma);
    theta[0] = 9.2606;
    sigma[0] = 69.832972536356;
    votes[1] = 0.5 - gauss(x, theta, sigma);
    // return argmax of votes
    uint8_t classIdx = 0;
    float maxVotes = votes[0];

    for (uint8_t i = 1; i < 2; i++) {
        if (votes[i] > maxVotes) {
            classIdx = i;
            maxVotes = votes[i];
        }
    }
    return classIdx;
}

float gauss(float *x, float *theta, float *sigma) {
    float gauss = 0.0f;

    for (uint16_t i = 0; i < 1; i++) {
        gauss += log(sigma[i]);
        gauss += abs(x[i] - theta[i]) / sigma[i];
    }
    return gauss;
}
            


void initMPU(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0x00); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_addr);     // inicia comunicação com endereço do MPU6050
  Wire.write(0x1B);                      // envia o registro com o qual se deseja trabalhar
  Wire.write(0x00);                      // escreve o valor no registro
  Wire.endTransmission(true);           // termina a transmissão
}

void readMPU(){

  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) ; // Divide by 1000 to get seconds
  previousTime = currentTime;        // Previous time is stored before the actual time read
  int t = (MPU_READ_PERIOD - elapsedTime) > 0 ? int(MPU_READ_PERIOD - elapsedTime) : 0;
  delay(t);
  
    // === Read acceleromter data === //
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);  
  Wire.requestFrom(MPU_addr, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
   
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet  
  // and for correct conversion you must use int16_t
  AccX = int16_t(Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = int16_t(Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = int16_t(Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
   
  // === Read gyro data === //
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = int16_t(Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
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
  //AADX = log(AADX+1);
  //AADY = log(AADY+1);
  //AADZ = log(AADZ+1);
}
 
void setup(){
  initMPU();
  prepareMAQueue();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}
 
void loop(){
   
  updateMAValues();

  if(predict(&AADM) == 1){
    digitalWrite(LED_BUILTIN,HIGH);
    ledTime = 100;
  }
  ledTime -= elapsedTime;
  if(ledTime <= 0){
    digitalWrite(LED_BUILTIN,LOW);
    ledTime = 0;
  }
}
