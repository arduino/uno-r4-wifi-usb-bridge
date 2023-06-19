
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
#include "DAP.h"

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


static uint32_t _baud = 0;
static CAtHandler atHandler(&SERIAL_AT);
USBCDC USBSerial(0);

#define GPIO_BOOT   9
#define GPIO_RST    4

bool enableSTA(bool enable);
bool enableAP(bool enable);

/* -------------------------------------------------------------------------- */
void CAtHandler::onWiFiEvent(WiFiEvent_t event) {
/* -------------------------------------------------------------------------- */   
   switch (event) {
      case ARDUINO_EVENT_WIFI_READY:
        wifi_status = WIFI_ST_IDLE_STATUS;
        break;
      case ARDUINO_EVENT_WIFI_SCAN_DONE:
        wifi_status = WIFI_ST_SCAN_COMPLETED;
        break;
      case ARDUINO_EVENT_WIFI_STA_START:
        wifi_status = WIFI_ST_IDLE_STATUS;
        break;
      case ARDUINO_EVENT_WIFI_STA_STOP:
        wifi_status = WIFI_ST_NO_MODULE;
        break;
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        wifi_status = WIFI_ST_CONNECTED;
        break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        wifi_status = WIFI_ST_IDLE_STATUS;
        break;
      case ARDUINO_EVENT_WIFI_AP_START:  
        wifi_status = WIFI_ST_AP_LISTENING;
        break;
      case ARDUINO_EVENT_WIFI_AP_STOP:
        wifi_status = WIFI_ST_IDLE_STATUS;
        break;
      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        wifi_status = WIFI_ST_AP_CONNECTED;
        break;
      case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        

        break;
      case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
      case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
      case ARDUINO_EVENT_WIFI_FTM_REPORT:
      default:
        break;
  }

}

TaskHandle_t atTask;
void atLoop(void* param) {
  while (1) {
    if (_baud != 1200 && _baud != 2400) {
      atHandler.run();
    }
    yield();
  }
}

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
  USB.productName("UNO WiFi R4 CMSIS-DAP");
  //USB.enableDFU();
  DAP.begin();
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
  USB.begin();
#endif
  /* Set up wifi event */
  WiFi.onEvent(CAtHandler::onWiFiEvent);

  xTaskCreatePinnedToCore(
      atLoop, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &atTask,  /* Task handle. */
      0); /* Core where the task should run */
}

/*
   arduino-builder -compile -fqbn=espressif:esp32:esp32s3:JTAGAdapter=default,PSRAM=disabled,FlashMode=qio,FlashSize=4M,LoopCore=1,EventsCore=1,USBMode=default,CDCOnBoot=cdc,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,PartitionScheme=huge_app,CPUFreq=240,UploadSpeed=921600,DebugLevel=none,EraseFlash=none -vid-pid=303A_1001 -ide-version=10820 CompostaUSBBridge.ino
*/

static uint8_t buf[2048];

/* -------------------------------------------------------------------------- */
void loop() {
/* -------------------------------------------------------------------------- */  

  if (SERIAL_USER.baudRate() != _baud) {
    _baud = SERIAL_USER.baudRate();
  }

  int i = 0;

  if (SERIAL_USER.available()) {
    i = min((unsigned int)SERIAL_USER.available(), sizeof(buf));
    SERIAL_USER.readBytes(buf, i);
  }

  if (i > 0) {
    SERIAL_USER_INTERNAL.write(buf, i);
  }

  i = 0;
  if (SERIAL_USER_INTERNAL.available()) {
    i = min((unsigned int)SERIAL_USER_INTERNAL.available(), sizeof(buf));
    SERIAL_USER_INTERNAL.readBytes(buf, i);
  }
  if (i > 0) {
    SERIAL_USER.write(buf, i);
  }

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

extern "C" void mylogchar(char c) {
    SERIAL_USER.print(c);
}