#pragma once
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <cmath>

class PID {
private:
  TickType_t lastPIDTick{0};

  bool manual{false};

  float sp{0};
  float kp{0.0};
  float ki{1000.0};
  float kd{0.0};

  float b0{0};
  float b1{0};
  float b2{0};

  float vel{0};

  float pError[2]{0};
  int32_t lastOp{0};

  const float ts{0.01};

  int64_t lastExectTime{0};

  struct {
    float _vel{0};
    float _sp{0};
    float _error{0};
    int32_t _op{0};
    int32_t padding{0};
  } outValues;

  struct {
    int32_t latMax{0};
    float meanLat{0};
    float maxJitter{0};
    float stdJitter{0};
    int32_t period{0};
  } taskStats;

  struct {
    uint8_t instruction{0};
    float value{0};
  } inputMessage;

  QueueHandle_t velQueue{nullptr};
  QueueHandle_t outMsgQueue{nullptr};
  QueueHandle_t inMsgQueue{nullptr};
  QueueHandle_t opQueue{nullptr};
  QueueHandle_t taskStatsQueue{nullptr};

  TaskHandle_t opHandler{nullptr};

  static void calc_op_task(void *params);
  void calc_op_loop();

public:
  PID(QueueHandle_t _velQueue, QueueHandle_t _outMsgQueue,
      QueueHandle_t _inMsgQueue, QueueHandle_t _opQueue,
      QueueHandle_t _taskStatsQueue);
  void start_op_task();
  void recalc_PID();
  ~PID();
};
