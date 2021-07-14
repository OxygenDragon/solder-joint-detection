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

#include "image_provider.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"
#include "string"
#include "stdlib.h"

namespace {
hx_drv_sensor_image_config_t g_pimg_config;
uint32_t width = 416;
uint32_t height = 416;
}

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {
  static bool is_initialized = false;

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

  //send jpeg image data out through SPI
  //  hx_drv_spim_send(g_pimg_config.jpeg_address, g_pimg_config.jpeg_size,
  //                   SPI_TYPE_JPG);
  //
  // hx_drv_image_rescale((uint8_t*)g_pimg_config.raw_address,
  //                      g_pimg_config.img_width, g_pimg_config.img_height,
  //                      image_data, image_width, image_height);
  
  uint8_t* img_ptr = (uint8_t*) malloc(sizeof(uint8_t) * width * height);
  uint32_t width_delta = (640 - width) / 2;
  uint32_t height_delta = (480 - height) / 2;
  uint32_t img_index = 0;	
  // copying interest region of image
  for (uint32_t i = width_delta; i < 640 - width_delta; ++i) {
    for (uint32_t j = height_delta; j < 480 - height_delta; ++j) {
      img_ptr[img_index++] = *(((uint8_t*) g_pimg_config.raw_address) +
          i * 640 + j);
    }
  }
  image_data = (int8_t*) img_ptr;
  // start signal
  for (uint32_t i = 0; i < 10; ++i) {
    hx_drv_uart_print("7");
  }
  // image transferring
  for (uint32_t i = 0; i < width * height; ++i){
    hx_drv_uart_print("%c", img_ptr[i]);
  }
  // quantization parameters
  double kScale = 0.00392117677256465;
  int32_t kZeroPoint = -128;

  float pixel;
  for (int i = 0; i < 128*128; ++i) { // 128x128
    pixel = float(int32_t(image_data[i]) + 128) / 255; // normalize
    float npixel = pixel / kScale + kZeroPoint;
    image_data[i] = (int8_t)npixel;
  }
  
  return kTfLiteOk;
}
