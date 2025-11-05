#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <math.h>
#include <stdio.h>

// Include your trained model data
#include "model.cc"

// Memory for the TensorFlow Lite interpreter
constexpr int kTensorArenaSize = 10 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

extern const unsigned char g_model[];
extern const unsigned int g_model_len;

extern "C" void app_main(void) {
  printf("ESP32 TensorFlow Lite Micro: Sine Wave Demo\n");

  // Load the model
  const tflite::Model *model = tflite::GetModel(g_model);

  // Prepare the interpreter
  // NEW (ESP-IDF version)
  tflite::MicroMutableOpResolver<10> resolver; // allow up to 10 ops
  resolver.AddFullyConnected();
  resolver.AddReshape();
  resolver.AddQuantize();
  resolver.AddDequantize();
  resolver.AddLogistic(); // or whatever ops your model needs
  static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
                                              kTensorArenaSize);
  if (interpreter.AllocateTensors() != kTfLiteOk) {
    printf("AllocateTensors() failed\n");
    return;
  }

  TfLiteTensor *input = interpreter.input(0);
  TfLiteTensor *output = interpreter.output(0);

  // Loop through several x values
  for (float x = 0.0f; x <= 2 * 3.1416f; x += 0.1f) {
    input->data.f[0] = x;
    interpreter.Invoke();
    float y_pred = output->data.f[0];
    float y_true = sinf(x);

    // Print CSV format: x, y_true, y_pred
    printf("%.3f, %.3f, %.3f\n", x, y_true, y_pred);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  printf("Done!\n");
}
