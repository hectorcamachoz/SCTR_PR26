#include "usbSerial.h"

UsbSerial::UsbSerial(QueueHandle_t _outMsgQueue) : outMsgQueue(_outMsgQueue) {
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
    float vel{0};
    xQueueReceive(outMsgQueue, &vel, 0);
    this->send_float(vel);
    xTaskDelayUntil(&lastTick, pdMS_TO_TICKS(100));
  }
}

void UsbSerial::send_string(const char *text) {
  uart_write_bytes(uart_num, (const char *)text, strlen(text));
}

void UsbSerial::start_write_task() {
  xTaskCreatePinnedToCore(&UsbSerial::write_task, "SERIAL_WRITE_TASK", 2048,
                          this, 4, NULL, 0);
}

void UsbSerial::send_float(float val) {
  float tempVal = val;
  uart_write_bytes(uart_num, &tempVal, 4);
}

UsbSerial::~UsbSerial() { uart_driver_delete(uart_num); }
