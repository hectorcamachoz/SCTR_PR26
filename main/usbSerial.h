#pragma once

#include "driver/uart.h"
#include "esp_check.h"
#include <stdio.h>
#include <string.h>

class UsbSerial {
private:
  const uart_port_t uart_num = UART_NUM_0;
  uart_config_t uart_config = {
      .baud_rate = 57600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
      //   .uart_sclk_t = UART_SCLK_APB,

  };

  const int uart_buffer_size = 1024 * 2;

public:
  UsbSerial();
  ~UsbSerial();
  void send_string(const char *text);
  void send_int(int val);
};
