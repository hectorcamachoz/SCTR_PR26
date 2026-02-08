#pragma once

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdint.h>

class Encoder {
private:
  unsigned char pinA;
  unsigned char pinB;
  pcnt_unit_handle_t pcnt_unit = NULL;
  pcnt_channel_handle_t pcnt_chan_a = NULL;
  pcnt_channel_handle_t pcnt_chan_b = NULL;
  int count{0};

public:
  Encoder(unsigned char _pinA, unsigned char _pinB);
  ~Encoder();
  int get_count();
};
