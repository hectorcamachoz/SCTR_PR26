#include "cmath"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdint.h>

class MotorControl {
private:
  mcpwm_unit_t unit = MCPWM_UNIT_0;
  mcpwm_timer_t timer = MCPWM_TIMER_0;
  gpio_num_t pwmPin{};
  gpio_num_t directionPin{};

  int64_t lastExecTime{0};

  TickType_t lastPWMTick{0};

  TaskHandle_t opHandler{nullptr};
  QueueHandle_t opQueue{nullptr};
  QueueHandle_t taskStatsQueue{nullptr};
  QueueHandle_t loopTimeQueue{nullptr};
  QueueHandle_t fullTaskStatsQueue{nullptr};

  struct {
    int32_t latMax{0};
    float meanLat{0};
    float maxJitter{0};
    float stdJitter{0};
    int32_t period{0};
  } taskStats;

  struct {
    int32_t latMax{0};
    float meanLat{0};
    float maxJitter{0};
    float stdJitter{0};
    int32_t period{0};
  } fullTaskStats;

  static void send_op_task(void *args);
  void loop_send_op();

public:
  MotorControl(gpio_num_t _pwmPin, gpio_num_t _directionPin,
               QueueHandle_t _opQueue, QueueHandle_t taskStatsQueue,
               QueueHandle_t loopTimeQueue, QueueHandle_t fullTaskStatsQueue);
  ~MotorControl();
  void start_op_task();
  void turn_off();
  void turn_left(char _dc = 50);
  void turn_right(char _dc = 50);
};
