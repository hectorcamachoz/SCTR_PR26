#include "pid.h"

PID::PID(QueueHandle_t _velQueue, QueueHandle_t _outMsgQueue,
         QueueHandle_t _inMsgQueue, QueueHandle_t _opQueue)
    : velQueue(_velQueue), outMsgQueue(_outMsgQueue), inMsgQueue(_inMsgQueue),
      opQueue(_opQueue) {}

void PID::start_op_task() {
  xTaskCreatePinnedToCore(&PID::calc_op_task, "PID_OP_TASK", 2048, this, 3,
                          &opHandler, 1);
}

void PID::calc_op_task(void *params) {
  auto *self = static_cast<PID *>(params);
  self->calc_op_loop();
}

void PID::calc_op_loop() {
  lastPIDTick = xTaskGetTickCount();
  float vel{0};
  float error{0};
  int8_t op{10};
  float fOp{0};
  for (;;) {
    BaseType_t ok = xQueueReceive(inMsgQueue, &inputMessage, 0);
    if (ok == pdTRUE) {
      switch (inputMessage.instruction) {
      case 0x01:
        sp = inputMessage.value;
        break;
      case 0x02:
        kp = inputMessage.value;
        break;
      case 0x03:
        ki = inputMessage.value;
        break;
      case 0x04:
        kd = inputMessage.value;
        break;

      default:
        break;
      }
      if (inputMessage.instruction != 0x01)
        recalc_PID();
    }
    xQueueReceive(velQueue, &vel, 1);
    error = sp - vel;

    op = static_cast<int8_t>(lastOp + b0 * error + b1 * pError[0] +
                             b2 * pError[1]);
    if (op > 100) {
      op = 100;
    } else if (op < -100) {
      op = -100;
    }
    fOp = op;

    outValues._error = error;
    outValues._op = op;
    outValues._sp = sp;
    outValues._vel = vel;

    xQueueOverwrite(outMsgQueue, &outValues);
    xQueueOverwrite(opQueue, &op);

    lastOp = op;
    pError[1] = pError[0];
    pError[0] = error;
    vTaskDelayUntil(&lastPIDTick, pdMS_TO_TICKS(ts * 1000));
  }
}

void PID::recalc_PID() {
  b0 = kp + ki * ts + kd / ts;
  b1 = -2 * kd / ts - kp;
  b2 = kd / ts;
}

PID::~PID() {}
