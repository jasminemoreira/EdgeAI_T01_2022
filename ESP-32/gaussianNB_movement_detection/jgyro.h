#ifndef JGYRO_H_INCLUDED
#define JGYRO_H_INCLUDED
//Number of samples for gyroscope moving average
#define MOVING_AVERAGE_WINDOW 10 
//Time between MPU readings
#define MPU_READ_PERIOD 20

#include <Wire.h>
#include <Math.h>
#include <CircularBuffer.h>

namespace JazzTinyML{

  class JGyro{
    
    // I2C address of the MPU-6050
    const int MPU_addr=0x68; 
    //Gyroscope raw values
    float GyroX, GyroY, GyroZ;
    //Gyroscope Moving Average
    float MAGyroX, MAGyroY, MAGyroZ;
    //Sampling time control variables
    float elapsedTime = 0.0, currentTime = 0.0, previousTime = 0.0, ledTime = 0.0;
    //Buffers used to preserve the values for moving average calculation
    CircularBuffer<float, MOVING_AVERAGE_WINDOW> x_buffer;
    CircularBuffer<float, MOVING_AVERAGE_WINDOW> y_buffer;
    CircularBuffer<float, MOVING_AVERAGE_WINDOW> z_buffer;
    // the following ensures using the right type for the index variable when iterating buffers
    using index_t = decltype(x_buffer)::index_t;

    // public methods
    public:
      // Absolute Angular Drift 
      // sum of absolute delta values for each sample --> abs(sample-mean)
      float AADX, AADY, AADZ, AADM;   

      //constructor
      JGyro(){
        initMPU();
        prepareMAQueue();
      }
      
      void updateAAD(){
        pushToQueue();
        updateMovingAverage();
              
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
      
    protected:   
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
        //read gyro data
        Wire.beginTransmission(MPU_addr);
        // Gyro data first register address 0x43
        Wire.write(0x43);
        Wire.endTransmission(false);
        // Read 4 registers total, each axis value is stored in 2 registers
        Wire.requestFrom(MPU_addr, 6, true);
        //for a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
        GyroX = int16_t(Wire.read() << 8 | Wire.read()) / 10.0;
        GyroY = int16_t(Wire.read() << 8 | Wire.read()) / 10.0;
        GyroZ = int16_t(Wire.read() << 8 | Wire.read()) / 10.0;
      }
   
      void prepareMAQueue(){
        for(int i=0;i<MOVING_AVERAGE_WINDOW;i++){
          pushToQueue();
        }
      }

      void pushToQueue(){
        initMPU();
        readMPU();
        x_buffer.push(GyroX);
        y_buffer.push(GyroY);
        z_buffer.push(GyroZ);
      }  

      void updateMovingAverage(){
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
      }
  };
}

#endif
 

 
