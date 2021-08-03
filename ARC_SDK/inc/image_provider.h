#ifndef OXYGENDRAGON_SOLDER_DETECTION_IMAGE_PROVIDER_H_
#define OXYGENDRAGON_SOLDER_DETECTION_IMAGE_PROVIDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data,
                      int8_t bias_x, int8_t bias_y);

#endif  // OXYGENDRAGON_SOLDER_DETECTION_IMAGE_PROVIDER_H_
