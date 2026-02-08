#include "usbSerial.h"

UsbSerial::UsbSerial() {
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size,
                                      uart_buffer_size, 0, NULL, 0));
}

void UsbSerial::send_string(const char *text) {
  uart_write_bytes(uart_num, (const char *)text, strlen(text));
}

void UsbSerial::send_int(int val) {
  int16_t val16 = static_cast<int16_t>(val);
  uart_write_bytes(uart_num, &val16, 2);
}

UsbSerial::~UsbSerial() { uart_driver_delete(uart_num); }
