#include "image_provider.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"
#include "string"
#include "stdlib.h"

namespace {
hx_drv_sensor_image_config_t g_pimg_config;
int8_t bias_weight = 8;
}

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data,
                      int8_t bias_x, int8_t bias_y) {
  static bool is_initialized = false;
  hx_drv_uart_initial(UART_BR_921600);
  // hx_drv_uart_initial(UART_BR_115200);

  if (!is_initialized) {
    if (hx_drv_sensor_initial(&g_pimg_config) != HX_DRV_LIB_PASS) {
      return kTfLiteError;
    }
    if (hx_drv_spim_init() != HX_DRV_LIB_PASS) {
      return kTfLiteError;
    }
    is_initialized = true;
  }

  //capture image by sensor
  hx_drv_sensor_capture(&g_pimg_config);
  uint8_t* img_ptr = (uint8_t*) 
    malloc(sizeof(uint8_t) * image_width * image_height);
  uint16_t width_delta = (640 - image_width) / 2;
  uint16_t height_delta = (480 - image_height) / 2;
  int8_t bias_val_x = bias_x * bias_weight;
  int8_t bias_val_y = bias_y * bias_weight;
  int8_t row_send_signal = 1; // used for control which bytes to send
  int8_t col_send_signal = 1; 
  uint32_t img_index = 0;	

  // start signal
  for (uint8_t i = 0; i < 10; ++i) {
    hx_drv_uart_print("7");
  }

  for (uint16_t i = width_delta + bias_val_y; i < 640 - width_delta + bias_val_y; ++i) {
    for (uint16_t j = height_delta + bias_val_x; j < 480 - height_delta + bias_val_x; ++j) {
      // copying interest region of captured image
      img_ptr[img_index] = *(((uint8_t*) g_pimg_config.raw_address) +
          i * 640 + j);
      // image transferring
      if (row_send_signal && col_send_signal)
        hx_drv_uart_print("%c", img_ptr[img_index++]);
      row_send_signal = !row_send_signal;
    }
    col_send_signal = !col_send_signal;
  }

  // quantization input setup
  float pixel;
  for (int i = 0; i < image_width * image_height; ++i) { 
    pixel = (float)((uint8_t)(img_ptr[i])) / 255; // normalize
    int32_t npixel = (int32_t)(pixel / kScale) + kZeroPoint;
    image_data[i] = (int8_t)npixel;
  }

  free(img_ptr);
  return kTfLiteOk;
}
