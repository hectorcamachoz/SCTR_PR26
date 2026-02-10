#include "driver/uart.h"
#include "encoder.h"
#include "hbridge.h"
#include "usbSerial.h"
#include <stdio.h>
#include <string.h>

extern "C" void app_main(void) {
  // Setup
  Encoder encoder(16, 17);
  UsbSerial serial(&encoder);
  // HBridge hBridge(13, 14);
  // hBridge.turn_left(90);
  encoder.start_task();
  serial.start_task();
}
