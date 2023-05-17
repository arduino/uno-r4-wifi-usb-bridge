
// Arduino Santiago Composta / Leven
// ESP32-S3 serial-to-usb bridge and wifi host
// Serial refers to tinyUSB CDC port
// Serial1 refers to gpio 20/21 ( -> will become Serial object in RA4 variant P109 / P110)
// Serial2 refers to gpio 2/3 ( -> will become SerialNina object in RA4 variant P501 / P502)
// https://arduino.atlassian.net/wiki/spaces/HWU4/pages/3656351882/Programming+RA4M1+via+ESP32-C3

// implement AT server via github.com/SudoMaker/chAT

#include "at_handler.h"

#include "USB.h"
#include "USBCDC.h"

//#define DEBUG_AT

#define SERIAL_USER            USBSerial
#define SERIAL_DEBUG           USBSerial
#define SERIAL_USER_INTERNAL   Serial

#ifdef DEBUG_AT
#define SERIAL_AT              USBSerial
#else
#define SERIAL_AT              Serial1
#endif

static void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


static uint32_t baud = 0;
static CAtHandler atHandler(&SERIAL_AT);
USBCDC USBSerial(0);

#define GPIO_BOOT   9
#define GPIO_RST    4

bool enableSTA(bool enable);
bool enableAP(bool enable);



/* -------------------------------------------------------------------------- */
void setup() {
/* -------------------------------------------------------------------------- */  

  pinMode(GPIO_BOOT, OUTPUT);
  pinMode(GPIO_RST, OUTPUT);
  digitalWrite(GPIO_BOOT, HIGH);
  digitalWrite(GPIO_RST, HIGH);

#ifdef DEBUG_AT
  SERIAL_AT.begin(115200);
  while (!SERIAL_AT);
  SERIAL_AT.println("READY");
#else
  USB.VID(0x2341);
  USB.PID(0x1002);
  USB.begin();
  SERIAL_USER.onEvent(usbEventCallback);
  SERIAL_USER.enableReboot(false);
  SERIAL_USER.begin(115200);
  SERIAL_USER.setRxBufferSize(0);
  SERIAL_USER.setRxBufferSize(2048);
  SERIAL_USER_INTERNAL.setRxBufferSize(8192);
  SERIAL_USER_INTERNAL.setTxBufferSize(8192);
  SERIAL_USER_INTERNAL.begin(115200, SERIAL_8N1, 44, 43);
  SERIAL_AT.setRxBufferSize(8192);
  SERIAL_AT.setTxBufferSize(8192);
  SERIAL_AT.begin(115200, SERIAL_8N1, 6, 5);
#endif
  

}

/*
   arduino-builder -compile -fqbn=espressif:esp32:esp32s3:JTAGAdapter=default,PSRAM=disabled,FlashMode=qio,FlashSize=4M,LoopCore=1,EventsCore=1,USBMode=default,CDCOnBoot=cdc,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,PartitionScheme=huge_app,CPUFreq=240,UploadSpeed=921600,DebugLevel=none,EraseFlash=none -vid-pid=303A_1001 -ide-version=10820 CompostaUSBBridge.ino
*/

/* -------------------------------------------------------------------------- */
void loop() {
/* -------------------------------------------------------------------------- */  

#ifndef DEBUG_AT
  if (SERIAL_USER.baudRate() != baud) {
    baud = SERIAL_USER.baudRate();
  }

  uint8_t buf[2048];
  int i = 0;
  while (SERIAL_USER.available() && i < sizeof(buf)) {
    buf[i++] = SERIAL_USER.read();
  }
  if (i > 0) {
    SERIAL_USER_INTERNAL.write(buf, i);
  }
  i = 0;
  while (SERIAL_USER_INTERNAL.available() && i < sizeof(buf)) {
    buf[i++] = SERIAL_USER_INTERNAL.read();
  }
  if (i > 0) {
    SERIAL_USER.write(buf, i);
  }
#endif

  atHandler.run();
  
  yield();
}



  


void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == ARDUINO_USB_CDC_EVENTS) {
    arduino_usb_cdc_event_data_t * data = (arduino_usb_cdc_event_data_t*)event_data;
    switch (event_id) {
      case ARDUINO_USB_CDC_LINE_CODING_EVENT:
        auto baud = data->line_coding.bit_rate;
        if (baud == 1200) {
          digitalWrite(GPIO_BOOT, HIGH);
          digitalWrite(GPIO_RST, LOW);
          delay(100);
          digitalWrite(GPIO_RST, HIGH);
          delay(100);
          digitalWrite(GPIO_RST, LOW);
          delay(100);
          digitalWrite(GPIO_RST, HIGH);
        } else if (baud == 2400) {
          digitalWrite(GPIO_BOOT, LOW);
          digitalWrite(GPIO_RST, HIGH);
          delay(100);
          digitalWrite(GPIO_RST, LOW);
          delay(100);
          digitalWrite(GPIO_RST, HIGH);
        } else {
          SERIAL_USER_INTERNAL.updateBaudRate(baud);
        }
        break;
    }
  }
}