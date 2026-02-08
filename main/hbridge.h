#include "driver/mcpwm.h"

class HBridge {
private:
  mcpwm_unit_t unit = MCPWM_UNIT_0;
  mcpwm_timer_t timer = MCPWM_TIMER_0;
  unsigned char pinA{0};
  unsigned char pinB{0};

public:
  HBridge(unsigned char _pinA, unsigned char _pinB, bool _unit0 = true);
  ~HBridge();
  void turn_left(float _dc = 50.0f);
  void turn_right(float _dc = 50.0f);
};
