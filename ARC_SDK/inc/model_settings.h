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

#ifndef TENSORFLOW_LITE_MICRO_EXAMPLES_HANDWRITING_MODEL_SETTINGS_H_
#define TENSORFLOW_LITE_MICRO_EXAMPLES_HANDWRITING_MODEL_SETTINGS_H_

constexpr int kNumCols = 384;
constexpr int kNumRows = 384;
constexpr int kNumChannels = 1;
constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;
constexpr int kPredictionSize = 13824; // 24 * 24 * 24
constexpr int kSinglePredictSize = 8;
constexpr float kDefectThresh = 0.1;
// quantization parameters
constexpr float kScale = 0.0039;
constexpr int32_t kZeroPoint = -128;

// output quantization parameters
constexpr double kOutputScale = 0.024364763870835304;
constexpr int8_t kOutputZero = -23;

constexpr int kCategoryCount = 3;
extern const char* kCategoryLabels[kCategoryCount];

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_HANDWRITING_MODEL_SETTINGS_H_
