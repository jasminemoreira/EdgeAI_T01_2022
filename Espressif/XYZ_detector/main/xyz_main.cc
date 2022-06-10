//#define DATA_COLLECTOR_MODE

#define MOVING_WINDOW_SIZE 60

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "circular_buffer.h"
#include "jacc.h"
#include "xyz_model.h"

#ifndef DATA_COLLECTOR_MODE

static const char *TAG_MAIN = "XYZ Detector";

// Number of samples in the moving window
static CircularBuffer<float, MOVING_WINDOW_SIZE> wx_buffer;
static CircularBuffer<float, MOVING_WINDOW_SIZE> wy_buffer;
static CircularBuffer<float, MOVING_WINDOW_SIZE> wz_buffer;

tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

constexpr int kTensorArenaSize = 14 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
static tflite::MicroErrorReporter micro_error_reporter;
static tflite::AllOpsResolver resolver;

static void pushToBuffer()
{
    updateDAccValues();
    wx_buffer.push(DAccX);
    wy_buffer.push(DAccY);
    wz_buffer.push(DAccZ);
}
#endif // not DATA_COLLECTOR_MODE

extern "C" void app_main(void)
{
#ifndef DATA_COLLECTOR_MODE
    // print X,Y,Z to serial
    static char lbl[4][2] = {"X", "Y", "Z", "N"};

    error_reporter = &micro_error_reporter;
    model = tflite::GetModel(xyz_model);
    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    initMPU();
    // Prepare first window
    for (int i = 0; i < MOVING_WINDOW_SIZE; i++)
    {
        pushToBuffer();
    }

#endif // not DATA_COLLECTOR_MODE

#ifdef DATA_COLLECTOR_MODE
    initMPU();
#endif

    while (1)
    {
#ifdef DATA_COLLECTOR_MODE
        updateDAccValues();
#endif

#ifndef DATA_COLLECTOR_MODE
        pushToBuffer();

        // montar tensor para inferência
        for (int i = 0; i < MOVING_WINDOW_SIZE; i++)
        {
            input->data.f[3 * i] = wx_buffer[i];
            input->data.f[3 * i + 1] = wy_buffer[i];
            input->data.f[3 * i + 2] = wz_buffer[i];
        }

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk)
        {
            // error_reporter->Report("Invoke failed\n");
            ESP_LOGI(TAG_MAIN, "Invoke failed");
            return;
        }

        float maxp = output->data.f[0];
        int maxi = 0;
        for (int i = 1; i < 4; i++)
        {
            if (output->data.f[i] > maxp)
            {
                maxp = output->data.f[i];
                maxi = i;
            }
        }
        // printf("%f;%f;%f;%f\n", output->data.f[0], output->data.f[1], output->data.f[2], output->data.f[3]);

        if (maxp > 0.5 && maxi != 3)
        {
            ESP_LOGI(TAG_MAIN, "Dominant Axis: %s", lbl[maxi]);
        }
#endif // not DATA_COLLECTOR_MODE

#ifdef DATA_COLLECTOR_MODE
        printf("%f;%f;%f\n", DAccX, DAccY, DAccZ);
#endif
    }
}
