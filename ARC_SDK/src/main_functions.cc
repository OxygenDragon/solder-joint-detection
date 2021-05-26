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

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 136 * 1024;
#if (defined(__GNUC__) || defined(__GNUG__)) && !defined (__CCAC__)
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((section(".tensor_arena")));
#else
#pragma Bss(".tensor_arena")
static uint8_t tensor_arena[kTensorArenaSize];
#pragma Bss()
#endif // if defined (_GNUC_) && !defined (_CCAC_)
}  // namespace

void setup() {
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = tflite::GetModel(yolov3_tiny_test_float16_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  static tflite::MicroMutableOpResolver<11> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddRelu();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddAdd();
  micro_op_resolver.AddSub();
  micro_op_resolver.AddMul();
  micro_op_resolver.AddLogistic();
  micro_op_resolver.AddSplit();
  micro_op_resolver.AddConcatenation();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddStridedSlice();


  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }

  input = interpreter->input(0);
}

void loop() {
  // Get image from provider, which is image sensor in handwriting case.
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
                            input->data.int8)) {
    TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
  }

  if (kTfLiteOk != interpreter->Invoke()) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
  }

  
  TfLiteTensor* output = interpreter->output(0);
  RespondToDetection(error_reporter, (int8_t*)output->data.uint8);
}

/*
output detail
[{
  'name': 'Identity', 
  'index': 168, 
  'shape': array([   1, 2535,   80], dtype=int32), 
  'shape_signature': array([ 1, -1, 80], dtype=int32), 
  'dtype': <class 'numpy.float32'>, 
  'quantization': (0.0, 0),
  'quantization_parameters': {'scales': array([], dtype=float32), 
  'zero_points': array([], dtype=int32), 
  'quantized_dimension': 0}, 
  'sparsity_parameters': {}
}, 
{
  'name': 'Identity_1', 
  'index': 189, 
  'shape': array([   1, 2535,    4], dtype=int32), 
  'shape_signature': array([ 1, -1,  4], dtype=int32), 
  'dtype': <class 'numpy.float32'>,
  'quantization': (0.0, 0), 
  'quantization_parameters': {'scales': array([], dtype=float32), 
  'zero_points': array([], dtype=int32), 
  'quantized_dimension': 0}, 
  'sparsity_parameters': {}
  }
]

*/