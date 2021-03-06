#include "detection_responder.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"
#include "string"
#include <cstring>
namespace {
uint8_t ARDUINO_ADDR = 0x12;
uint32_t int_buff;
uint32_t int_part;
uint32_t fra_part;
uint8_t has_defect_joint = 0;
}

void FloatStrConversion(float input_float) {
  int_buff = input_float * 1000;
  int_part = int_buff / 1000;
  fra_part = int_buff % 1000;
}

// relu1 output, clipped when ouptut greater than 1 and less than 0
float GetFloatOutput(int8_t input) {
  if ((float)((input - kOutputZero) * kOutputScale) > 1) 
    return 1.0;
  else if ((float)((input - kOutputZero) * kOutputScale) < 0) 
    return 0;
  else
    return (float)((input - kOutputZero) * kOutputScale);
}

void RespondToDetection(tflite::ErrorReporter* error_reporter, int8_t* score) {
  
  int8_t max_score = -127, max_class = 0;
  float confidence;
  uint8_t predict_count[3] = {0};
  for (int32_t i = 4; i < kPredictionSize; i += kSinglePredictSize) {
    confidence = GetFloatOutput(score[i]);
    if (confidence < kDefectThresh) continue;
    for (int8_t j = i + 1; j < i + 4; ++j) {
      if (score[j] > max_score) {
        max_score = score[j];
        max_class = j - i - 1;
      }
    }
    if (confidence * GetFloatOutput(max_score) > kDefectThresh) {
      predict_count[max_class]++;
    }
    max_score = -127;
  }
  has_defect_joint = (predict_count[0] + predict_count[1] +
      predict_count[2] != 0);

  // preparing result string to sent
  // passed: 0,0,x,x,x
  // not passed: 0,1,insufficient,short,too_much
  char result_str[20];
  uint8_t result_str_int8[20];
  sprintf(result_str, "0,%d,%d,%d,%d,",
      has_defect_joint, predict_count[0], predict_count[1], predict_count[2]); 

  for (uint8_t i = 0; i < strlen(result_str); ++i) {
    result_str_int8[i] = (uint8_t)result_str[i];
  }

  hx_drv_uart_print("i2c start: %d with address: %d\n",
      hx_drv_share_switch(SHARE_MODE_I2CM), ARDUINO_ADDR); // start using I2C
  // sending detections result to aruduino through i2c
  TF_LITE_REPORT_ERROR(error_reporter, "I2C Transmission: %d",
      hx_drv_i2cm_set_data(ARDUINO_ADDR, NULL, 0,
        result_str_int8, strlen(result_str)));
}
