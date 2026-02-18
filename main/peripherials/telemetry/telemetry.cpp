#include "telemetry.h"
#include "cmath"

Telemetry::Telemetry(Encoder *enc, QueueHandle_t vQueue,
                     QueueHandle_t statsQueue, QueueHandle_t loopTimeQueue)
    : encoder(enc), velQueue(vQueue), taskStatsQueue(statsQueue),
      loopTimeQueue(loopTimeQueue) {}

void Telemetry::calc_vel_task(void *arg) {
  auto *self = static_cast<Telemetry *>(arg);
  self->calc_vel_loop();
}

void Telemetry::calc_vel_loop() {
  lastVelTick = xTaskGetTickCount();
  lastVelTime = esp_timer_get_time();
  lastPos = encoder->get_count();

  int64_t latMax = INT64_MIN;
  int64_t latMin = INT64_MAX;
  int32_t cLat = 0;
  uint32_t k = 0;
  uint32_t i{0};
  float delta{0};
  float delta2{0};
  float M2{0};
  float mean{0};
  float stdJitter{-1};
  float maxJitter{0};
  float velRaw{0};

  for (;;) {
    int64_t cTime = esp_timer_get_time();
    int32_t currentPos = encoder->get_count();
    int64_t dt = cTime - lastVelTime;
    int64_t dPeriod{0};
    float dCount = (currentPos - lastPos) / 400.0f;
    if (dt > 0) {
      float dt_s = static_cast<float>(dt) * 1e-6f;

      velRaw = static_cast<float>(dCount) * 60 / dt_s;
    } else {
      velRaw = 0;
    }

    vel = vel + 0.16 * (velRaw - vel);

    xQueueOverwrite(loopTimeQueue, &cTime);
    xQueueOverwrite(velQueue, &vel);
    this->lastPos = currentPos;
    this->lastVelTime = cTime;

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

      if (k % 100 == 0) {
        dPeriod = cTime - lastExecTime;
        taskStats.latMax = latMax;
        taskStats.meanLat = mean;
        taskStats.maxJitter = maxJitter;
        taskStats.stdJitter = stdJitter;
        taskStats.period = static_cast<int>(dPeriod);
        xQueueOverwrite(taskStatsQueue, &taskStats);
      }
    }

    if (i == UINT32_MAX) {
      i = 0;
      k = 0;
      delta = 0;
      delta2 = 0;
      M2 = 0;
      mean = 0;
    }
    lastExecTime = cTime;
    xTaskDelayUntil(&lastVelTick, pdMS_TO_TICKS(3));
  }
}

void Telemetry::start_vel_task() {
  xTaskCreatePinnedToCore(&Telemetry::calc_vel_task, "TEL_VEL_TASK", 2048, this,
                          4, &velHandler, 0);
}

int Telemetry::get_vel() { return vel; }

Telemetry::~Telemetry() {}
