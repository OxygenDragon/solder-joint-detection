#include "main_functions.h"

#include "detection_responder.h"
#include "joystick_handler.h"
#include "image_provider.h"
#include "model_data.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr; 

int8_t bias_x = 0;
int8_t bias_y = 0;
hx_drv_gpio_config_t gpio_0;
hx_drv_gpio_config_t gpio_1;
hx_drv_gpio_config_t gpio_2;

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 730 * 1024;
#if (defined(__GNUC__) || defined(__GNUG__)) && !defined (__CCAC__)
alignas(16) static uint8_t tensor_arena[kTensorArenaSize] __attribute__(
    (section(".tensor_arena")));
#else
#pragma Bss(".tensor_arena")
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];
#pragma Bss()
#endif // if defined (_GNUC_) && !defined (_CCAC_)
}  // namespace

void setup() {
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  hx_drv_uart_initial(UART_BR_921600);
  // hx_drv_uart_initial(UART_BR_115200);
  TF_LITE_REPORT_ERROR(error_reporter, "start inferencing...");

  model = tflite::GetModel(model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  static tflite::MicroMutableOpResolver<3> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddRelu6();
  // micro_op_resolver.AddStridedSlice();
  // micro_op_resolver.AddMul();
  // micro_op_resolver.AddResizeNearestNeighbor();
  // micro_op_resolver.AddConcatenation();

  // GPIO port initialization
  gpio_0.gpio_pin = HX_DRV_PGPIO_0;
  gpio_1.gpio_pin = HX_DRV_PGPIO_1;
  gpio_2.gpio_pin = HX_DRV_PGPIO_2;
  gpio_0.gpio_direction = HX_DRV_GPIO_INPUT;
  gpio_1.gpio_direction = HX_DRV_GPIO_INPUT;
  gpio_2.gpio_direction = HX_DRV_GPIO_INPUT;
  gpio_0.gpio_data = 0;
  gpio_1.gpio_data = 0;
  gpio_2.gpio_data = 0;
  hx_drv_gpio_initial(&gpio_0);
  hx_drv_gpio_initial(&gpio_1);
  hx_drv_gpio_initial(&gpio_2);

  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }
  input = interpreter->input(0);
  output = interpreter->output(0);
  TF_LITE_REPORT_ERROR(error_reporter, "Setup Finish");
}

void loop() {
  // get joystick status
  int8_t joystick_state = GetJoyStickState(&gpio_0, &gpio_1, &gpio_2);
  GetImageBias(joystick_state, &bias_x, &bias_y);
  JoystickSignalAck();

  // Get image from provider
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels,
        input->data.int8, bias_x, bias_y)) {
    TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
  }

  // invoke model for inferencing
  if (kTfLiteOk != interpreter->Invoke()) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
  }

  // sending predictions signals
  for (uint8_t i = 0; i < 10; ++i) {
    hx_drv_uart_print("8");
  }
  for (uint32_t i = 0; i < kPredictionSize; ++i) {
    hx_drv_uart_print("%d\n", (output->data.int8)[i]);
  }

  TF_LITE_REPORT_ERROR(error_reporter, "Send prediction done.");
  // respond to detection result and send to arduino
  RespondToDetection(error_reporter, output->data.int8);
}
