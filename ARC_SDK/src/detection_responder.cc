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

#include "detection_responder.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"
#include "string"
namespace {
uint8_t score_output[kCategoryCount];
char* className[] = {
  "Normal", 
  "Cold joint", 
  "Insufficient", 
  "Short",
  "Overheat", 
  "Too much"
};
}
void RespondToDetection(tflite::ErrorReporter* error_reporter, int8_t* score) {

  int maxIndex = -1;
  int maxScore = -255;
  
  TF_LITE_REPORT_ERROR(error_reporter, "\n\n");
  for (int i = 0; i < kCategoryCount; i++) {
    char str[30];
    sprintf(str, "[%s]: %d,", className[i], score[i]);
    TF_LITE_REPORT_ERROR(error_reporter, str);
    if (score[i] > 0 && maxScore < score[i]) {
      maxScore = score[i];
      maxIndex = i;
    }
    score_output[i] = score[i] + 128;
  }

  TF_LITE_REPORT_ERROR(error_reporter, "===== Inference Result =====");
  if(maxIndex != -1) {
    char class_str[30];
    char score_str[30];
    sprintf(class_str, "Result:     %s", className[maxIndex]);
    sprintf(score_str, "Confidence: %d", maxScore);
    TF_LITE_REPORT_ERROR(error_reporter, class_str);
    TF_LITE_REPORT_ERROR(error_reporter, score_str);
  } else {
    TF_LITE_REPORT_ERROR(error_reporter, "Result: unknown");
  }
  TF_LITE_REPORT_ERROR(error_reporter, "============================");

  //send result data out through SPI
  hx_drv_spim_send((uint32_t)score_output, sizeof(int8_t) * kCategoryCount,
      SPI_TYPE_META_DATA);
}
