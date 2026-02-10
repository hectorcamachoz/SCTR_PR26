#include "usbSerial.h"

UsbSerial::UsbSerial(Encoder *encoder) : _encoder(encoder) {
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size,
                                      uart_buffer_size, 0, NULL, 0));
}

void UsbSerial::start_task() {
  xTaskCreatePinnedToCore(&UsbSerial::write_task, "SERIAL_TASK", 2048, this, 3,
                          &write_handler, 1);
}

void UsbSerial::write_task(void *args) {
  auto *self = static_cast<UsbSerial *>(args);
  self->write_msg();
}

void UsbSerial::write_msg() {
  lastTick = xTaskGetTickCount();
  while (1) {
    int count = _encoder->get_count();
    send_int(count);
    vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(100));
  }
}

void UsbSerial::send_string(const char *text) {
  uart_write_bytes(uart_num, (const char *)text, strlen(text));
}

void UsbSerial::send_int(int val) {
  int16_t val16 = static_cast<int16_t>(val);
  uart_write_bytes(uart_num, &val16, 2);
}

UsbSerial::~UsbSerial() { uart_driver_delete(uart_num); }
