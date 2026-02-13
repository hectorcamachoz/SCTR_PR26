
#include "hbridge.h"

HBridge::HBridge(unsigned char _pinA, unsigned char _pinB, bool _unit0)
    : pinA(_pinA), pinB(_pinB) {
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

void HBridge::turn_left(float dc) {
  mcpwm_set_duty(unit, timer, MCPWM_GEN_B, 0);
  mcpwm_set_duty(unit, timer, MCPWM_GEN_A, dc);
}

void HBridge::turn_right(float dc) {
  mcpwm_set_duty(unit, timer, MCPWM_GEN_A, 0);
  mcpwm_set_duty(unit, timer, MCPWM_GEN_B, dc);
}

HBridge::~HBridge() {}
