#ifndef JACC_H_INCLUDED
#define JACC_H_INCLUDED
// Number of samples for gyroscope moving average
#define MOVING_AVERAGE_WINDOW 10
// Time between MPU readings CPU FREQ=40MHZ, must correct period
#define MPU_READ_PERIOD 10 * 2

#define I2C_MASTER_SCL_IO 23      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 22      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mpu6050.h"
#include "circular_buffer.h"

// Accelerometer raw values
static float AccX, AccY, AccZ;
// Accelerometer Moving Average
static float MAAccX, MAAccY, MAAccZ;
// Accelerometer delta values
static float DAccX, DAccY, DAccZ;

// Sampling time control variables
float elapsedTime = 0.0, currentTime = 0.0, previousTime = 0.0, ledTime = 0.0;

// Buffers used to preserve the values for moving average calculation
static CircularBuffer<float, MOVING_AVERAGE_WINDOW> x_buffer;
static CircularBuffer<float, MOVING_AVERAGE_WINDOW> y_buffer;
static CircularBuffer<float, MOVING_AVERAGE_WINDOW> z_buffer;
// the following ensures using the right type for the index variable when iterating buffers
using index_t = decltype(x_buffer)::index_t;

static mpu6050_handle_t mpu6050 = NULL;

static void i2c_bus_init(void)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

static void i2c_sensor_mpu6050_init()
{
    i2c_bus_init();
    mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
    mpu6050_config(mpu6050, ACCE_FS_2G, GYRO_FS_500DPS);
    mpu6050_wake_up(mpu6050);
}

static void readMPU()
{
    // current time actual time read
    currentTime = esp_timer_get_time() / 1000;
    elapsedTime = (currentTime - previousTime);
    double t = (MPU_READ_PERIOD - elapsedTime) > 0 ? MPU_READ_PERIOD - elapsedTime : 0;
    vTaskDelay(t / portTICK_RATE_MS);
    // ESP_LOGI("JACC", "Time between MPU readings: %f", t);

    uint8_t mpu6050_deviceid;
    mpu6050_acce_value_t acce;
    mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);
    mpu6050_get_acce(mpu6050, &acce);

    AccX = acce.acce_x;
    AccY = acce.acce_y;
    AccZ = acce.acce_z;

    // previous time is stored before the actual time read
    previousTime = esp_timer_get_time() / 1000;
}

static void pushToQueue()
{
    readMPU();
    x_buffer.push(AccX);
    y_buffer.push(AccY);
    z_buffer.push(AccZ);
}

static void updateMovingAverage()
{
    using index_t = decltype(x_buffer)::index_t;
    MAAccX = MAAccY = MAAccZ = 0.0;
    for (index_t i = 0; i < MOVING_AVERAGE_WINDOW; i++)
    {
        MAAccX += x_buffer[i];
        MAAccY += y_buffer[i];
        MAAccZ += z_buffer[i];
    }
    // performance
    MAAccX = MAAccX / (float)MOVING_AVERAGE_WINDOW;
    MAAccY = MAAccY / (float)MOVING_AVERAGE_WINDOW;
    MAAccZ = MAAccZ / (float)MOVING_AVERAGE_WINDOW;
}

static void prepareMAQueue()
{
    for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++)
    {
        pushToQueue();
    }
}

static void initMPU()
{
    uint8_t mpu6050_deviceid;
    i2c_sensor_mpu6050_init();
    mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);
    prepareMAQueue();
}

static void updateDAccValues()
{
    pushToQueue();
    updateMovingAverage();

    DAccX = AccX - MAAccX;
    DAccY = AccY - MAAccY;
    DAccZ = AccZ - MAAccZ;
}

#endif