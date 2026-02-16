
#include "hbridge.h"

HBridge::HBridge(unsigned char _pinA, unsigned char _pinB,
                 QueueHandle_t _opQueue, bool _unit0)
    : pinA(_pinA), pinB(_pinB), opQueue(_opQueue) {
  if (!_unit0) {
    unit = MCPWM_UNIT_1;
    timer = MCPWM_TIMER_1;
  }

  mcpwm_config_t config{};
  config.cmpr_a = 0;
  config.cmpr_b = 0;
  config.frequency = 15000;
  config.counter_mode = MCPWM_UP_COUNTER;

  mcpwm_gpio_init(unit, MCPWM0A, pinA);
  mcpwm_gpio_init(unit, MCPWM0B, pinB);
  mcpwm_set_duty_type(unit, timer, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
  mcpwm_set_duty_type(unit, timer, MCPWM_GEN_B, MCPWM_DUTY_MODE_0);
  mcpwm_init(unit, timer, &config);
  mcpwm_start(unit, timer);
}

void HBridge::start_op_task() {
  xTaskCreatePinnedToCore(HBridge::send_op_task, "HBRIDGE_OP_TASK", 2048, this,
                          3, &opHandler, 1);
}

void HBridge::send_op_task(void *args) {
  auto *self = static_cast<HBridge *>(args);
  self->loop_send_op();
}

void HBridge::loop_send_op() {
  char op{0};
  for (;;) {
    xQueueReceive(opQueue, &op, 1);
    if (op > 100)
      op = 100;
    if (op < -100)
      op = -100;
    if (op == 0) {
      this->turn_off();
    } else if (op < 0) {
      op = -15 + ((op * 85) / 100);
      this->turn_right(-op);
    } else {
      op = 15 + ((op * 85) / 100);
      this->turn_left(op);
    }
  }
}

void HBridge::turn_left(char dc) {
  mcpwm_set_duty(unit, timer, MCPWM_GEN_B, 0);
  mcpwm_set_duty(unit, timer, MCPWM_GEN_A, dc);
}

void HBridge::turn_right(char dc) {
  mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 0);
  mcpwm_set_duty(unit, timer, MCPWM_GEN_B, dc);
}

void HBridge::turn_off() {
  mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 0);
  mcpwm_set_duty(unit, timer, MCPWM_GEN_B, 0);
}

HBridge::~HBridge() {}
