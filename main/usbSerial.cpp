#include "usbSerial.h"

UsbSerial::UsbSerial() {
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size,
                                      uart_buffer_size, 0, NULL, 0));
}

void UsbSerial::write_task(void *arg) {
  auto *self = static_cast<UsbSerial *>(arg);
  self->write_task_loop();
}

void UsbSerial::write_task_loop() {
  lastTick = xTaskGetTickCount();
  for (;;) {
    this->send_int(static_cast<int>(2));
    xTaskDelayUntil(&lastTick, pdMS_TO_TICKS(1000));
  }
}

void UsbSerial::send_string(const char *text) {
  uart_write_bytes(uart_num, (const char *)text, strlen(text));
}

void UsbSerial::start_write_task() {
  xTaskCreatePinnedToCore(&UsbSerial::write_task, "SERIAL_WRITE_TASK", 2048,
                          this, 4, NULL, 0);
}

void UsbSerial::send_int(int val) {
  uint16_t val16 = static_cast<int16_t>(2);
  uart_write_bytes(uart_num, &val16, 2);
}

UsbSerial::~UsbSerial() { uart_driver_delete(uart_num); }
