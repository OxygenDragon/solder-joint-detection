#include "image_provider.h"
#include "model_settings.h"
#include "hx_drv_tflm.h"

namespace{
uint8_t ARDUINO_ADDR = 0x12;
}

int8_t GetJoyStickState(hx_drv_gpio_config_t* gpio_0,
    hx_drv_gpio_config_t* gpio_1, hx_drv_gpio_config_t* gpio_2) {
  hx_drv_gpio_get(gpio_0);
  hx_drv_gpio_get(gpio_1);
  hx_drv_gpio_get(gpio_2);
  return gpio_0->gpio_data * 4 + gpio_1->gpio_data * 2 + gpio_2->gpio_data;
}

void GetImageBias(int8_t joystick_state, int8_t* bias_x, int8_t* bias_y) {
  switch (joystick_state) {
    case 0:
      return;
    case 1: // up
      *bias_x = (*bias_x == -6)? *bias_x : *bias_x - 1;
      break;
    case 2: // down
      *bias_x = (*bias_x == 6)? *bias_x : *bias_x + 1;
      break;
    case 3: // left
      *bias_y = (*bias_y == -16)? *bias_y : *bias_y - 1;
      break;
    case 4: // right
      *bias_y = (*bias_y == 16)? *bias_y : *bias_y + 1;
      break;
    default:
      return;
  }
}

void JoystickSignalAck() {
  char* return_str = "1,0,0,0,0,";
  uint8_t return_str_int8[11];
  for (int8_t i = 0; i < 11; ++i) {
    return_str_int8[i] = (uint8_t)return_str[i];
  }
  hx_drv_share_switch(SHARE_MODE_I2CM); // start using I2C
  // sending acknowledgement to aruduino through i2c
  hx_drv_i2cm_set_data(ARDUINO_ADDR, NULL, 0, return_str_int8, 10);
}
