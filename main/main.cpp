#include "driver/uart.h"
#include "encoder.h"
#include "hbridge.h"
#include "usbSerial.h"
#include <stdio.h>
#include <string.h>

extern "C" void app_main(void) {
  esp_log_level_set("*", ESP_LOG_NONE);
  // Setup
  static UsbSerial serial;
  serial.send_int(2);
  Encoder encoder(16, 17);
  HBridge hBridge(13, 14);
  serial.start_write_task();
}
