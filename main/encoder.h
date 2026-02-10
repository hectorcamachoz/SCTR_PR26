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
  // Variables
  unsigned char pinA;
  unsigned char pinB;
  pcnt_unit_handle_t pcnt_unit = NULL;
  pcnt_channel_handle_t pcnt_chan_a = NULL;
  pcnt_channel_handle_t pcnt_chan_b = NULL;
  volatile int count{0};

  // RTOS Variables
  TaskHandle_t update_handler{nullptr};
  TickType_t lastTick{0};

  // Functions
  void update_count();
  static void update_task(void *args);

public:
  Encoder(unsigned char _pinA, unsigned char _pinB);
  void start_task();
  ~Encoder();
  int get_count();
};
