
#include "motorcontrol.h"

MotorControl::MotorControl(gpio_num_t _pwmPin, gpio_num_t _directionPin,
                           QueueHandle_t _opQueue,
                           QueueHandle_t _taskStatsQueue,
                           QueueHandle_t _loopTimeQueue,
                           QueueHandle_t _fullTaskStatsQueue)
    : pwmPin(_pwmPin), directionPin(_directionPin), opQueue(_opQueue),
      taskStatsQueue(_taskStatsQueue), loopTimeQueue(_loopTimeQueue),
      fullTaskStatsQueue(_fullTaskStatsQueue) {

  mcpwm_config_t config{};
  config.cmpr_a = 100;
  config.frequency = 20000;
  config.counter_mode = MCPWM_UP_COUNTER;

  mcpwm_gpio_init(unit, MCPWM0A, pwmPin);
  mcpwm_set_duty_type(unit, timer, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
  mcpwm_init(unit, timer, &config);
  mcpwm_start(unit, timer);

  gpio_config_t directionPin_conf = {
      .pin_bit_mask = (1ULL << _directionPin),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&directionPin_conf);
  gpio_set_level(directionPin, 0);
}

void MotorControl::start_op_task() {
  xTaskCreatePinnedToCore(MotorControl::send_op_task, "MotorControl_OP_TASK",
                          2048, this, 3, &opHandler, 1);
}

void MotorControl::send_op_task(void *args) {
  auto *self = static_cast<MotorControl *>(args);
  self->loop_send_op();
}

void MotorControl::loop_send_op() {
  int op{0};
  lastPWMTick = xTaskGetTickCount();

  int64_t cTime{0};
  uint32_t i{0};
  uint32_t k{0};
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

  int64_t startLoopTime{0};
  int64_t latMaxLoop = INT64_MIN;
  int64_t latMinLoop = INT64_MAX;
  int32_t cLatLoop = 0;
  float deltaLoop{0};
  float delta2Loop{0};
  float M2Loop{0};
  float meanLoop{0};
  float stdJitterLoop{0};
  float maxJitterLoop{-1};

  for (;;) {
    cTime = esp_timer_get_time();
    xQueueReceive(opQueue, &op, 3);
    if (op == 0) {
      this->turn_off();
    } else {
      this->turn_left(op);
    }
    int64_t endTime = esp_timer_get_time();

    xQueueReceive(loopTimeQueue, &startLoopTime, 0);
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

      dPeriod = cTime - lastExecTime;
      taskStats.latMax = latMax;
      taskStats.meanLat = mean;
      taskStats.maxJitter = maxJitter;
      taskStats.stdJitter = stdJitter;
      taskStats.period = dPeriod;
      xQueueOverwrite(taskStatsQueue, &taskStats);

      /// System Loop
      cLatLoop = endTime - startLoopTime;

      if (cLatLoop > latMaxLoop)
        latMaxLoop = cLatLoop;
      if (cLatLoop < latMinLoop)
        latMinLoop = cLatLoop;

      deltaLoop = cLatLoop - meanLoop;
      meanLoop += deltaLoop / k;
      delta2Loop = cLatLoop - meanLoop;
      M2Loop += delta2Loop * deltaLoop;

      stdJitterLoop = M2Loop / (k - 1);
      stdJitterLoop = sqrtf(stdJitterLoop);
      if (maxJitterLoop < stdJitterLoop)
        maxJitterLoop = stdJitterLoop;

      fullTaskStats.latMax = latMaxLoop;
      fullTaskStats.meanLat = meanLoop;
      fullTaskStats.maxJitter = maxJitterLoop;
      fullTaskStats.stdJitter = stdJitterLoop;
      xQueueOverwrite(fullTaskStatsQueue, &fullTaskStats);
    }
    if (k == UINT32_MAX) {
      i = 0;
      k = 0;
      delta = 0;
      delta2 = 0;
      M2 = 0;
      mean = 0;
      deltaLoop = 0;
      delta2Loop = 0;
      M2Loop = 0;
      meanLoop = 0;
    }

    lastExecTime = cTime;
    vTaskDelayUntil(&lastPWMTick, pdMS_TO_TICKS(10));
  }
}

void MotorControl::turn_left(char dc) {
  // mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 100);
  // esp_rom_delay_us(10);
  // gpio_set_level(directionPin, 0);
  mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 100 - dc);
}

void MotorControl::turn_right(char dc) {
  // mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 100);
  // esp_rom_delay_us(10);
  // gpio_set_level(directionPin, 1);
  // mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 100 - dc);
}

void MotorControl::turn_off() { mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 100); }

MotorControl::~MotorControl() {}
