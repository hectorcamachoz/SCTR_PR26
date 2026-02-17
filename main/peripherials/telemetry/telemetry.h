#pragma once

#include "atomic"
#include "encoder.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
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
  QueueHandle_t velQueue{nullptr};
  QueueHandle_t taskStatsQueue{nullptr};

  struct {
    int32_t latMax{0};
    int32_t latMin{0};
    float meanJitter{0};
    float stdJitter{0};
  } taskStats;

  // RTOS Functions
  static void calc_vel_task(void *arg);
  void calc_vel_loop();

public:
  Telemetry(Encoder *_encoder, QueueHandle_t _velQueue,
            QueueHandle_t _statsQueue);
  void start_vel_task();
  int get_vel();
  ~Telemetry();
};
