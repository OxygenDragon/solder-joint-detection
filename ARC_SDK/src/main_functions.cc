/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "main_functions.h"

#include "detection_responder.h"
#include "image_provider.h"
#include "model_data.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr; 

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 730 * 1024;
#if (defined(__GNUC__) || defined(__GNUG__)) && !defined (__CCAC__)
alignas(16) static uint8_t tensor_arena[kTensorArenaSize] __attribute__(
    (section(".tensor_arena")));
#else
#pragma Bss(".tensor_arena")
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];
#pragma Bss()
#endif // if defined (_GNUC_) && !defined (_CCAC_)
}  // namespace

void setup() {
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  hx_drv_uart_initial(UART_BR_921600);
  // hx_drv_uart_initial(UART_BR_115200);
  TF_LITE_REPORT_ERROR(error_reporter, "start inferencing...");

  model = tflite::GetModel(model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  static tflite::MicroMutableOpResolver<3> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddRelu6();
  // micro_op_resolver.AddStridedSlice();
  // micro_op_resolver.AddMul();
  // micro_op_resolver.AddResizeNearestNeighbor();
  // micro_op_resolver.AddConcatenation();

  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }
  input = interpreter->input(0);
  output = interpreter->output(0);
  TF_LITE_REPORT_ERROR(error_reporter, "Setup Finish");
}

void loop() {
  // Get image from provider
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
        input->data.int8)) {
    TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
  }

  // invoke model for inferencing
  if (kTfLiteOk != interpreter->Invoke()) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
  }

  for (uint8_t i = 0; i < 6; ++i) {
    hx_drv_uart_print("%d ", (output->data.int8)[i]);
  }

  // sending predictions signals
  for (uint8_t i = 0; i < 10; ++i) {
    hx_drv_uart_print("8");
  }
  for (uint32_t i = 0; i < kPredictionSize; ++i) {
    hx_drv_uart_print("%c", (output->data.int8)[i]);
  }

  TF_LITE_REPORT_ERROR(error_reporter, "Send prediction done.");
  RespondToDetection(error_reporter, output->data.int8);
}
