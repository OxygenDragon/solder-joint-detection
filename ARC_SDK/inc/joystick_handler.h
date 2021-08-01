#ifndef OXGENDRAGON_SOLDER_DETECTION_JOYSTICK_HANDLER_H_
#define OXGENDRAGON_SOLDER_DETECTION_JOYSTICK_HANDLER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "hx_drv_tflm.h"

// decode 3 gpios voltage level to joystick state
int8_t GetJoyStickState(hx_drv_gpio_config_t* gpio_0,
    hx_drv_gpio_config_t* gpio_1, hx_drv_gpio_config_t* gpio_2);

// calculate new bias according to joystick state
void GetImageBias(int8_t joystick_state, int8_t* bias_x, int8_t* bias_y);
void JoystickSignalAck();
#endif  // TENSORFLOW_LITE_MICRO_EXAMPLES_HANDWRITING_JOYSTICK_HANDLER_H_
