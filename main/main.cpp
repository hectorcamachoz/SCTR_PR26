#include "driver/uart.h"
#include "hbridge.h"
#include "peripherials/encoder/encoder.h"
#include "peripherials/telemetry/telemetry.h"
#include "peripherials/usbserial/usbSerial.h"
#include <stdio.h>
#include <string.h>

extern "C" void app_main(void) {
  // esp_log_level_set("*", ESP_LOG_NONE);
  // Setup
  static Encoder encoder(16, 17);
  static Telemetry tel_manager(&encoder);
  static UsbSerial serial(&tel_manager);
  tel_manager.start_vel_task();
  serial.start_write_task();
  HBridge hBridge(13, 14);
  hBridge.turn_left();
  // serial.start_write_task();
}
