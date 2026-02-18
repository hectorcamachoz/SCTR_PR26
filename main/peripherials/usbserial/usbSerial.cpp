#include "usbSerial.h"

UsbSerial::UsbSerial(QueueHandle_t _inMsgQueue, QueueHandle_t _outMsgQueue,
                     QueueHandle_t telTaskStatsQueue,
                     QueueHandle_t pidTaskStatsQueue,
                     QueueHandle_t pwmTaskStatsQueue,
                     QueueHandle_t fullTaskStatsQueue)
    : inMsgQueue(_inMsgQueue), outMsgQueue(_outMsgQueue),
      telTaskStatsQueue(telTaskStatsQueue),
      pidTaskStatsQueue(pidTaskStatsQueue),
      pwmTaskStatsQueue(pwmTaskStatsQueue),
      fullTaskStatsQueue(fullTaskStatsQueue) {
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
  BaseType_t telTaskAvailable{pdFALSE};
  BaseType_t pidTaskAvailable{pdFALSE};
  BaseType_t pwmTaskAvailable{pdFALSE};
  BaseType_t fullTaskAvailable{pdFALSE};
  for (;;) {
    xQueueReceive(outMsgQueue, &outValues, 0);
    telTaskAvailable = xQueueReceive(telTaskStatsQueue, &telTaskStats, 1);
    this->send_outMsg();
    if (telTaskAvailable == pdTRUE)
      this->send_telTaskStats();
    pidTaskAvailable = xQueueReceive(pidTaskStatsQueue, &pidTaskStats, 0);
    if (pidTaskAvailable == pdTRUE)
      this->send_pidTaskStats();
    pwmTaskAvailable = xQueueReceive(pwmTaskStatsQueue, &pwmTaskStats, 0);
    if (pwmTaskAvailable == pdTRUE)
      this->send_pwmTaskStats();
    this->read_buffer();
    fullTaskAvailable = xQueueReceive(fullTaskStatsQueue, &fullTaskStats, 0);
    if (fullTaskAvailable == pdTRUE)
      this->send_fullTaskStats();
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

void UsbSerial::send_telTaskStats() {
  uint8_t packet[1 + sizeof(OutMsg) + 1];
  packet[0] = 0x02;
  memcpy(&packet[1], &telTaskStats, sizeof(OutMsg));
  uint8_t chk = 0;
  for (size_t i = 0; i < 1 + sizeof(OutMsg); ++i)
    chk ^= packet[i];
  packet[1 + sizeof(OutMsg)] = chk;

  uart_write_bytes(uart_num, reinterpret_cast<const char *>(packet),
                   sizeof(packet));
}

void UsbSerial::send_pidTaskStats() {
  uint8_t packet[1 + sizeof(OutMsg) + 1];
  packet[0] = 0x03;
  memcpy(&packet[1], &pidTaskStats, sizeof(OutMsg));
  uint8_t chk = 0;
  for (size_t i = 0; i < 1 + sizeof(OutMsg); ++i)
    chk ^= packet[i];
  packet[1 + sizeof(OutMsg)] = chk;

  uart_write_bytes(uart_num, reinterpret_cast<const char *>(packet),
                   sizeof(packet));
}

void UsbSerial::send_pwmTaskStats() {
  uint8_t packet[1 + sizeof(OutMsg) + 1];
  packet[0] = 0x04;
  memcpy(&packet[1], &pwmTaskStats, sizeof(OutMsg));
  uint8_t chk = 0;
  for (size_t i = 0; i < 1 + sizeof(OutMsg); ++i)
    chk ^= packet[i];
  packet[1 + sizeof(OutMsg)] = chk;

  uart_write_bytes(uart_num, reinterpret_cast<const char *>(packet),
                   sizeof(packet));
}

void UsbSerial::send_fullTaskStats() {
  uint8_t packet[1 + sizeof(OutMsg) + 1];
  packet[0] = 0x05;
  memcpy(&packet[1], &fullTaskStats, sizeof(OutMsg));
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
