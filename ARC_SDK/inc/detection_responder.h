#ifndef OXYGENDRAGON_SOLDER_DETECTION_RESPONDER_H_
#define OXYGENDRAGON_SOLDER_DETECTION_RESPONDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

void RespondToDetection(tflite::ErrorReporter* error_reporter, int8_t* score);

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_HANDWRITING_RESPONDER_H_
