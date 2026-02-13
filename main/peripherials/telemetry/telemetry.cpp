#include "telemetry.h"

Telemetry::Telemetry(Encoder *enc) : encoder(enc) {}

void Telemetry::calc_vel_task(void *arg) {
  auto *self = static_cast<Telemetry *>(arg);
  self->calc_vel_loop();
}

void Telemetry::calc_vel_loop() {
  lastVelTick = xTaskGetTickCount();
  lastVelTime = esp_timer_get_time();
  lastPos = encoder->get_count();
  for (;;) {
    int64_t cTime = esp_timer_get_time();
    int32_t currentPos = encoder->get_count();
    int64_t dt = cTime - lastVelTime;
    float degrees = (currentPos - lastPos) * static_cast<float>(360.0f / 2400);
    if (dt > 0) {
      float dt_s = static_cast<float>(dt) * 1e-6f;

      vel = static_cast<float>(degrees) / dt_s;
    } else {
      vel = 0;
    }
    // printf("Velocity: %f\n", vel);
    this->lastPos = currentPos;
    this->lastVelTime = cTime;
    xTaskDelayUntil(&lastVelTick, pdMS_TO_TICKS(5));
  }
}

void Telemetry::start_vel_task() {
  xTaskCreatePinnedToCore(&Telemetry::calc_vel_task, "TEL_VEL_TASK", 2048, this,
                          4, &velHandler, 0);
}

int Telemetry::get_vel() { return vel; }

Telemetry::~Telemetry() {}
