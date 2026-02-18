#include "pid.h"

PID::PID(QueueHandle_t _velQueue, QueueHandle_t _outMsgQueue,
         QueueHandle_t _inMsgQueue, QueueHandle_t _opQueue,
         QueueHandle_t _taskStatsQueue)
    : velQueue(_velQueue), outMsgQueue(_outMsgQueue), inMsgQueue(_inMsgQueue),
      opQueue(_opQueue), taskStatsQueue(_taskStatsQueue) {}

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
  int32_t op{10};

  int64_t cTime{0};
  uint32_t k{0};
  uint32_t i{0};
  int64_t latMax = INT64_MIN;
  int64_t latMin = INT64_MAX;
  int32_t cLat = 0;
  float delta{0};
  float delta2{0};
  float M2{0};
  float mean{0};
  float stdJitter{0};
  float maxJitter{-1};
  int32_t dPeriod{0};

  for (;;) {
    cTime = esp_timer_get_time();
    BaseType_t ok = xQueueReceive(inMsgQueue, &inputMessage, 0);
    if (ok == pdTRUE) {
      switch (inputMessage.instruction) {
      case 0x01:
        manual = false;
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
      case 0x05:
        manual = true;
        op = static_cast<int32_t>(inputMessage.value);
        break;

      default:
        break;
      }
      if ((inputMessage.instruction != 0x01) |
          (inputMessage.instruction != 0x05))
        recalc_PID();
    }
    xQueueReceive(velQueue, &vel, 1);
    error = sp - vel;

    if (!manual) {
      op = static_cast<int32_t>(lastOp + b0 * error + b1 * pError[0] +
                                b2 * pError[1]);
    }
    if (op > 100) {
      op = 100;
    } else if (op < 0) {
      op = 0;
    }

    outValues._error = error;
    outValues._op = op;
    outValues._sp = sp;
    outValues._vel = vel;

    xQueueOverwrite(outMsgQueue, &outValues);
    xQueueOverwrite(opQueue, &op);
    if (!manual) {
      lastOp = op;
      pError[1] = pError[0];
      pError[0] = error;
    }

    int64_t endTime = esp_timer_get_time();
    i++;
    if (i > 5) {
      k++;
      cLat = endTime - cTime;
      // printf("Value: %ld\n", cLat);

      if (cLat > latMax)
        latMax = cLat;
      if (cLat < latMin)
        latMin = cLat;

      delta = cLat - mean;
      mean += delta / k;
      delta2 = cLat - mean;
      M2 += delta2 * delta;

      stdJitter = M2 / (k - 1);
      stdJitter = sqrtf(stdJitter);
      if (maxJitter < stdJitter)
        maxJitter = stdJitter;

      dPeriod = cTime - lastExectTime;
      taskStats.latMax = latMax;
      taskStats.meanLat = mean;
      taskStats.maxJitter = maxJitter;
      taskStats.stdJitter = stdJitter;
      taskStats.period = dPeriod;
      xQueueOverwrite(taskStatsQueue, &taskStats);
    }

    if (i == UINT32_MAX) {
      i = 0;
      k = 0;
      delta = 0;
      delta2 = 0;
      M2 = 0;
      mean = 0;
    }

    lastExectTime = cTime;
    vTaskDelayUntil(&lastPIDTick, pdMS_TO_TICKS(ts * 1000));
  }
}

void PID::recalc_PID() {
  b0 = kp * (1.0f + ts / ki + kd / ts);
  b1 = -kp * (1.0f + 2.0f * kd / ts);
  b2 = kp * (kd / ts);
}

PID::~PID() {}
