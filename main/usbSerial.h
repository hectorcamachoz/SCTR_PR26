#pragma once

#include "driver/uart.h"
#include "esp_check.h"
#include <stdio.h>
#include <string.h>

class UsbSerial {
private:
  const uart_port_t uart_num = UART_NUM_0;
  uart_config_t uart_config = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,

  };
  const int uart_buffer_size = 1024 * 2;

  // RTOS Variables
  TickType_t lastTick{0};
  TaskFunction_t write_handler{nullptr};

  // RTOS Functions
  static void write_task(void *arg);
  void write_task_loop();

public:
  UsbSerial();
  ~UsbSerial();
  void start_write_task();
  void send_string(const char *text);
  void send_int(int val);
};
