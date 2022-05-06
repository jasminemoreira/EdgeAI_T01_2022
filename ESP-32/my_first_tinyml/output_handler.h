#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "Arduino.h"

// The pin of the Arduino's built-in LED
#if defined(LED_BUILTIN)
int led = LED_BUILTIN;
#elif defined(ARDUINO_M5Stick_C)
int led = 10;
#else
int led = -1;
#endif

// Track whether the function has run at least once
bool initialized = false;

// Animates a dot across the screen to represent the current x and y values
void HandleOutput(tflite::ErrorReporter* error_reporter, float x_value,
                  float y_value) {
  // Do this only once
  if (!initialized) {
    // Set the LED pin to output
    if (led != -1) {
      pinMode(led, OUTPUT);
      ledcSetup(0, 1200, 8);
      ledcAttachPin(led, 0);
    }
    initialized = true;
  }

  // Calculate the brightness of the LED such that y=-1 is fully off
  // and y=1 is fully on. The LED's brightness can range from 0-255.
  int brightness = (int)(127.5 * (y_value+1));

  // Set the brightness of the LED. If the specified pin does not support PWM,
  // this will result in the LED being on when y > 127, off otherwise.
  if (led != -1) {
    ledcWrite(0, brightness);
  }

  // Log the current brightness value for display in the Arduino plotter
  error_reporter->Report("%d", brightness);
}
