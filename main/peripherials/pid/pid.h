#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

class PID {
private:
  float kp{0.0};
  float ki{1000.0};
  float kd{0.0};

  float vel{0};
  float op{0};

  struct {
    float _vel{0};
    float _op{0};
  } outValues;

  QueueHandle_t velQueue{nullptr};
  QueueHandle_t outMsgQueue{nullptr};
  QueueHandle_t pidInParamsQueue{nullptr};
  QueueHandle_t opQueue{nullptr};

  TaskHandle_t opHandler{nullptr};

  static void calc_op_task(void *params);
  void calc_op_loop();

public:
  PID(QueueHandle_t _velQueue, QueueHandle_t _outMsgQueue,
      QueueHandle_t _pidInParamsQueue, QueueHandle_t _opQueue);
  void start_op_task();
  ~PID();
};
