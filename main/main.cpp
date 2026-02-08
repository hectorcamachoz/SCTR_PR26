#include "driver/uart.h"
#include "encoder.h"
#include "hbridge.h"
#include "usbSerial.h"
#include <stdio.h>
#include <string.h>

extern "C" void app_main(void) {
  // Setup
  UsbSerial serial;
  Encoder encoder(16, 17);
  HBridge hBridge(13, 14);
  hBridge.turn_left(90);

  int count{0};
  while (1) {
    count = encoder.get_count();
    serial.send_int(count);
    vTaskDelay(pdTICKS_TO_MS(50));
  }
}
