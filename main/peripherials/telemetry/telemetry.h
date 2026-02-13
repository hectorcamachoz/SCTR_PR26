#pragma once

#include "encoder.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "stdint.h"

class Telemetry {
private:
  Encoder *encoder;
  float vel;
  int64_t lastVelTime{0};
  int32_t lastPos{0};

  // RTOS Variables
  TickType_t lastVelTick{0};
  TaskHandle_t velHandler{nullptr};

  // RTOS Functions
  static void calc_vel_task(void *arg);
  void calc_vel_loop();

public:
  Telemetry(Encoder *encoder);
  void start_vel_task();
  int get_vel();
  ~Telemetry();
};
