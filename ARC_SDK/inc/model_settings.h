#ifndef OXYGENDRAGON_SOLDER_DETECTION_MODEL_SETTINGS_H_
#define OXYGENDRAGON_SOLDER_DETECTION_MODEL_SETTINGS_H_

constexpr int kNumCols = 384;
constexpr int kNumRows = 384;
constexpr int kNumChannels = 1;
constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;
constexpr int kPredictionSize = 13824; // 24 * 24 * 24
constexpr int kSinglePredictSize = 8;
constexpr float kDefectThresh = 0.1;
// quantization parameters
constexpr float kScale = 0.00392117677256465;
constexpr int32_t kZeroPoint = -128;

// output quantization parameters
constexpr double kOutputScale = 0.04109394922852516;
constexpr int8_t kOutputZero = -16;

constexpr int kCategoryCount = 3;
extern const char* kCategoryLabels[kCategoryCount];

#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_HANDWRITING_MODEL_SETTINGS_H_
