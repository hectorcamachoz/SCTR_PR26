#pragma once

#include "driver/uart.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "telemetry.h"
#include <stdio.h>
#include <string.h>

class UsbSerial {
private:
  const uart_port_t uart_num = UART_NUM_0;
  uart_config_t uart_config = {
      .baud_rate = 460800,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,

  };
  const int uart_buffer_size = 1024 * 2;
  Telemetry *telemetry{nullptr};

  struct {
    uint8_t instruction{0};
    float value{0};
  } inputMessage;

  struct OutMsg {
    float _vel{0};
    float _sp{0};
    float _error{0};
    int32_t _op{0};
    int32_t padding{0};
  } outValues;

  struct {
    int32_t latMax{0};
    int32_t latMin{0};
    float meanJitter{0};
    float stdJitter{0};
    float period{0};
  } telTaskStats;
  struct {
    int32_t latMax{0};
    int32_t latMin{0};
    float meanJitter{0};
    float stdJitter{0};
    int32_t period{0};
  } pidTaskStats;
  struct {
    int32_t latMax{0};
    int32_t latMin{0};
    float meanJitter{0};
    float stdJitter{0};
    int32_t period{0};
  } pwmTaskStats;
  struct {
    int32_t latMax{0};
    int32_t latMin{0};
    float meanJitter{0};
    float stdJitter{0};
    int32_t period{0};
  } fullTaskStats;

  // RTOS Variables
  TickType_t lastTick{0};
  TaskFunction_t write_handler{nullptr};

  // Queues
  QueueHandle_t inMsgQueue{nullptr};
  QueueHandle_t outMsgQueue{nullptr};
  QueueHandle_t telTaskStatsQueue{nullptr};
  QueueHandle_t pidTaskStatsQueue{nullptr};
  QueueHandle_t pwmTaskStatsQueue{nullptr};
  QueueHandle_t fullTaskStatsQueue{nullptr};

  // RTOS Functions
  static void write_task(void *arg);
  void write_task_loop();

public:
  UsbSerial(QueueHandle_t inMsgQueue, QueueHandle_t outMsgQueue,
            QueueHandle_t telTaskStatsQueue, QueueHandle_t pidTaskStatsQueue,
            QueueHandle_t pwmTaskStatsQueue, QueueHandle_t fullTaskStatsQueue);
  ~UsbSerial();
  void start_write_task();
  void send_string(const char *text);
  void send_float(float val);
  void send_outMsg();
  void send_telTaskStats();
  void send_pidTaskStats();
  void send_pwmTaskStats();
  void send_fullTaskStats();
  void read_buffer();
};
