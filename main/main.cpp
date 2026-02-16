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
  QueueHandle_t inMsgQueue = xQueueCreate(1, 8);
  QueueHandle_t outMsgQueue = xQueueCreate(1, 4 * sizeof(float));
  QueueHandle_t opQueue = xQueueCreate(1, sizeof(char));

  static PID pid(velQueue, outMsgQueue, inMsgQueue, opQueue);
  static Encoder encoder(16, 17);
  static Telemetry tel_manager(&encoder, velQueue);
  static UsbSerial serial(inMsgQueue, outMsgQueue);
  static HBridge hBridge(32, 33, opQueue);
  tel_manager.start_vel_task();
  pid.start_op_task();
  hBridge.start_op_task();
  serial.start_write_task();
}
