#include "driver/uart.h"
#include "peripherials/encoder/encoder.h"
#include "peripherials/motorcontrol/motorcontrol.h"
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
  QueueHandle_t outMsgQueue = xQueueCreate(1, 5 * sizeof(int32_t));
  QueueHandle_t opQueue = xQueueCreate(1, sizeof(char));
  QueueHandle_t telTaskStatsQueue = xQueueCreate(1, 5 * sizeof(int32_t));
  QueueHandle_t pidTaskStatsQueue = xQueueCreate(1, 5 * sizeof(int32_t));
  QueueHandle_t mcTaskStatsQueue = xQueueCreate(1, 5 * sizeof(int32_t));
  QueueHandle_t loopTimeQueue = xQueueCreate(1, sizeof(int64_t));
  QueueHandle_t fullTaskStatsQueue = xQueueCreate(1, 5 * sizeof(int32_t));

  static PID pid(velQueue, outMsgQueue, inMsgQueue, opQueue, pidTaskStatsQueue);
  static Encoder encoder(16, 17);
  static Telemetry tel_manager(&encoder, velQueue, telTaskStatsQueue,
                               loopTimeQueue);
  static UsbSerial serial(inMsgQueue, outMsgQueue, telTaskStatsQueue,
                          pidTaskStatsQueue, mcTaskStatsQueue,
                          fullTaskStatsQueue);
  static MotorControl motorControl(GPIO_NUM_32, GPIO_NUM_33, opQueue,
                                   mcTaskStatsQueue, loopTimeQueue,
                                   fullTaskStatsQueue);
  tel_manager.start_vel_task();
  pid.start_op_task();
  motorControl.start_op_task();
  serial.start_write_task();
}
