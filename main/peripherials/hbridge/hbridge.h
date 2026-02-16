#include "cmath"
#include "driver/mcpwm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

class HBridge {
private:
  mcpwm_unit_t unit = MCPWM_UNIT_0;
  mcpwm_timer_t timer = MCPWM_TIMER_0;
  unsigned char pinA{0};
  unsigned char pinB{0};

  TaskHandle_t opHandler{nullptr};
  QueueHandle_t opQueue{nullptr};

  static void send_op_task(void *args);
  void loop_send_op();

public:
  HBridge(unsigned char _pinA, unsigned char _pinB, QueueHandle_t _opQueue,
          bool _unit0 = true);
  ~HBridge();
  void start_op_task();
  void turn_off();
  void turn_left(char _dc = 50);
  void turn_right(char _dc = 50);
};
