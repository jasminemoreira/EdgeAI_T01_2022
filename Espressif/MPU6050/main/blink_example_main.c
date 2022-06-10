/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "mpu6050.h"

#define I2C_MASTER_SCL_IO 23      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 22      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

static const char *TAG = "mpu6050 test";
static mpu6050_handle_t mpu6050 = NULL;

/**
 * @brief i2c master initialization
 */
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
   esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
   ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

/**
 * @brief i2c master initialization
 */
static void i2c_sensor_mpu6050_init()
{
   esp_err_t ret;
   i2c_bus_init();
   mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
   ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
   ret = mpu6050_wake_up(mpu6050);
}

void app_main(void)
{
   esp_err_t ret;
   uint8_t mpu6050_deviceid;
   mpu6050_acce_value_t acce;
   mpu6050_gyro_value_t gyro;

   i2c_sensor_mpu6050_init();
   ret = mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);
   ret = mpu6050_get_acce(mpu6050, &acce);

   ESP_LOGI(TAG, "X:%f", acce.acce_x);
   //  ret = mpu6050_get_gyro(mpu6050, &gyro);

   
}
