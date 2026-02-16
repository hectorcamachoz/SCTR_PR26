#include "driver/uart.h"
#include "hbridge.h"
#include "peripherials/encoder/encoder.h"
#include "peripherials/pid/pid.h"
#include "peripherials/telemetry/telemetry.h"
#include "peripherials/usbserial/usbSerial.h"
#include <stdio.h>
#include <string.h>

extern "C" void app_main(void) {
  // esp_log_level_set("*", ESP_LOG_NONE);
  // Setup
  QueueHandle_t velQueue = xQueueCreate(1, sizeof(float));
  QueueHandle_t outMsgQueue = xQueueCreate(1, sizeof(float));
  QueueHandle_t opQueue = xQueueCreate(1, sizeof(char));

  static PID pid(velQueue, outMsgQueue, NULL, opQueue);
  static Encoder encoder(16, 17);
  static Telemetry tel_manager(&encoder, velQueue);
  static UsbSerial serial(outMsgQueue);
  static HBridge hBridge(13, 14, opQueue);
  tel_manager.start_vel_task();
  pid.start_op_task();
  hBridge.start_op_task();
  serial.start_write_task();
}
