#define MOVING_WINDOW_SIZE 60 

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"

#include "xyz_model.h"
#include "jacc.h"

JazzTinyML::JAcc acc;

// Globals, used for compatibility with Arduino-style sketches.
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Create an area of memory to use for input, output, and intermediate arrays.
// Finding the minimum value for your model may require some trial and error.
constexpr int kTensorArenaSize = 14 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// Number of samples in the moving window
CircularBuffer<float, MOVING_WINDOW_SIZE> x_buffer;
CircularBuffer<float, MOVING_WINDOW_SIZE> y_buffer;
CircularBuffer<float, MOVING_WINDOW_SIZE> z_buffer;

void pushToBuffer(){
  acc.updateDAccValues();
  x_buffer.push(acc.DAccX);
  y_buffer.push(acc.DAccY);
  z_buffer.push(acc.DAccZ); 
}

void setup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(xyz_model);
  
  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::ops::micro::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Prepare first window
  for(int i=0; i<MOVING_WINDOW_SIZE; i++){
    pushToBuffer();
  }

  Serial.begin(115200);
}

void loop() {
  
  pushToBuffer();
  //montar tensor para inferência
  for(int i=0; i< MOVING_WINDOW_SIZE; i++){
    input->data.f[3*i] = x_buffer[i];
    input->data.f[3*i+1] = y_buffer[i];
    input->data.f[3*i+2] = z_buffer[i];
  } 

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    error_reporter->Report("Invoke failed\n");
    return;
  }

  // print X,Y,Z to serial
  String lbl[4] = {"X","Y","Z","N"};
    
  float maxp = output->data.f[0];
  int maxi = 0;
  for(int i=1;i<4;i++){
    if(output->data.f[i] > maxp){
      maxp = output->data.f[i];
      maxi = i;
    }
  }

  if(maxp>0.5 && maxi!=3){
    Serial.print(lbl[maxi]);
    Serial.println();
  }
  
}
