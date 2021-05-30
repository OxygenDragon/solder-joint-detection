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
#include <cstring>
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
uint8_t ARDUINO_ADDR = 0x12;
uint32_t int_buff;
uint32_t int_part;
uint32_t fra_part;
}

void FloatStrConversion(float input_float) {
  int_buff = input_float * 1000;
  int_part = int_buff / 1000;
  fra_part = int_buff % 1000;
}

void RespondToDetection(tflite::ErrorReporter* error_reporter, int8_t* score) {
  int maxIndex = -1;
  int maxScore = -255;
  float maxScore_float = -2.0;
  hx_drv_share_switch(SHARE_MODE_I2CM); // start using I2C
  
  TF_LITE_REPORT_ERROR(error_reporter, "\n\n");
  for (int i = 0; i < kCategoryCount; i++) {
    char str[30];
    float score_float = (score[i] + 128.0) / 2.55;
    FloatStrConversion(score_float);
    sprintf(str, "[%s]: %ld.%ld", className[i], int_part, fra_part);
    TF_LITE_REPORT_ERROR(error_reporter, str);
    if (score[i] > 0 && maxScore < score[i]) {
      maxScore = score[i];
      maxScore_float = score_float;
      maxIndex = i;
    }
    score_output[i] = score[i] + 128;
  }

  char result_str[30];
  uint8_t result_str_int8[30]; // to be sent by I2C
  TF_LITE_REPORT_ERROR(error_reporter, "===== Inference Result =====");
  if(maxIndex != -1) {
    char class_str[30];
    char score_str[30];
    FloatStrConversion(maxScore_float);

    sprintf(class_str, "Result:     %s", className[maxIndex]);
    sprintf(score_str, "Confidence: %ld.%ld", int_part, fra_part);
    sprintf(result_str, "%s\n%ld.%ld", className[maxIndex], int_part, fra_part);
    TF_LITE_REPORT_ERROR(error_reporter, class_str);
    TF_LITE_REPORT_ERROR(error_reporter, score_str);
  } else {
    TF_LITE_REPORT_ERROR(error_reporter, "Result: unknown");
    sprintf(result_str, "unknown");
  }
  TF_LITE_REPORT_ERROR(error_reporter, "============================");

  for (int i = 0; i < strlen(result_str); i++) {
    result_str_int8[i] = (uint8_t)result_str[i];
  }

  //send result data out through SPI
  hx_drv_spim_send((uint32_t)score_output, sizeof(int8_t) * kCategoryCount,
      SPI_TYPE_META_DATA);

  TF_LITE_REPORT_ERROR(error_reporter, "I2C Transmission: %d",
      hx_drv_i2cm_set_data(ARDUINO_ADDR, NULL, 0, result_str_int8, strlen(result_str)));
}