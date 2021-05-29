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

namespace {
hx_drv_sensor_image_config_t g_pimg_config;

// quantization parameters
double kScale = 0.00392117677256465;
double kZeroPoint = -128;
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
  hx_drv_spim_send(g_pimg_config.jpeg_address, g_pimg_config.jpeg_size,
                   SPI_TYPE_JPG);

  hx_drv_image_rescale((uint8_t*)g_pimg_config.raw_address,
                       g_pimg_config.img_width, g_pimg_config.img_height,
                       image_data, image_width, image_height);

  float pixel;
  for (int i = 0; i < 128*128; ++i) { // 128x128
    pixel = float(image_data[i] + 128) / 255; // normalize
    pixel = pixel / kScale + kZeroPoint;
    image_data[i] = (int8_t)pixel;
  }
  
  return kTfLiteOk;
}
