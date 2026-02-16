#include "usbSerial.h"

UsbSerial::UsbSerial(QueueHandle_t _inMsgQueue, QueueHandle_t _outMsgQueue)
    : inMsgQueue(_inMsgQueue), outMsgQueue(_outMsgQueue) {
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
    xQueueReceive(outMsgQueue, &outValues, 0);
    this->send_outMsg();
    this->read_buffer();
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

void UsbSerial::send_outMsg() {
  uint8_t packet[1 + sizeof(OutMsg) + 1];
  packet[0] = 0x01;
  memcpy(&packet[1], &outValues, sizeof(OutMsg));
  uint8_t chk = 0;
  for (size_t i = 0; i < 1 + sizeof(OutMsg); ++i)
    chk ^= packet[i];
  packet[1 + sizeof(OutMsg)] = chk;

  uart_write_bytes(uart_num, reinterpret_cast<const char *>(packet),
                   sizeof(packet));
}

void UsbSerial::read_buffer() {
  uint8_t buffer[6]{0};
  size_t bufferedData{0};
  uart_get_buffered_data_len(uart_num, &bufferedData);
  if (bufferedData > 0) {
    int len = uart_read_bytes(uart_num, &buffer, 6, 0);
    if (len == 6) {
      uint8_t x = 0;
      for (int i = 0; i < 5; ++i)
        x ^= buffer[i];

      bool ok = (x == buffer[5]);

      if (ok) {
        inputMessage.instruction = buffer[0];
        memcpy(&inputMessage.value, &buffer[1], 4);
        xQueueOverwrite(inMsgQueue, &inputMessage);
      }
    }
  }
}

UsbSerial::~UsbSerial() { uart_driver_delete(uart_num); }
