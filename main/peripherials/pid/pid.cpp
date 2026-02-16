#include "pid.h"

PID::PID(QueueHandle_t _velQueue, QueueHandle_t _outMsgQueue,
         QueueHandle_t _pidInParamsQueue, QueueHandle_t _opQueue)
    : velQueue(_velQueue), outMsgQueue(_outMsgQueue),
      pidInParamsQueue(_pidInParamsQueue), opQueue(_opQueue) {}

void PID::start_op_task() {
  xTaskCreatePinnedToCore(&PID::calc_op_task, "PID_OP_TASK", 2048, this, 3,
                          &opHandler, 1);
}

void PID::calc_op_task(void *params) {
  auto *self = static_cast<PID *>(params);
  self->calc_op_loop();
}

void PID::calc_op_loop() {
  float vel{0};
  char op{10};
  for (;;) {
    xQueueReceive(velQueue, &vel, 1);
    xQueueOverwrite(outMsgQueue, &vel);
    xQueueOverwrite(opQueue, &op);
  }
}

PID::~PID() {}
