#include "USB.h"
#include "USBHID.h"

extern "C" {
    #include "esp32-hal-tinyusb.h"
    #include "freedap.h"
}

#include "at_handler.h"

extern USBHID HID;

#define TUD_HID_REPORT_DESC_R4_INOUT_FEATURE(report_size, ...) \
    HID_USAGE_PAGE_N ( HID_USAGE_PAGE_VENDOR, 2   ),\
    HID_USAGE        ( 0x01                       ),\
    HID_COLLECTION   ( HID_COLLECTION_APPLICATION ),\
      /* Report ID if any */\
      __VA_ARGS__ \
      /* Input */ \
      HID_USAGE       ( 0x02                                   ),\
      HID_LOGICAL_MIN ( 0x00                                   ),\
      HID_LOGICAL_MAX_N ( 0xff, 2                              ),\
      HID_REPORT_SIZE ( 8                                      ),\
      HID_REPORT_COUNT( report_size                            ),\
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),\
      /* Output */ \
      HID_USAGE       ( 0x03                                    ),\
      HID_LOGICAL_MIN ( 0x00                                    ),\
      HID_LOGICAL_MAX_N ( 0xff, 2                               ),\
      HID_REPORT_SIZE ( 8                                       ),\
      HID_REPORT_COUNT( report_size                             ),\
      HID_OUTPUT      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),\
      /* Feature */ \
      HID_USAGE       ( 0x04),\
      HID_LOGICAL_MIN ( 0x00),\
      HID_LOGICAL_MAX_N ( 0xff, 2),\
      HID_REPORT_SIZE (8),\
      HID_REPORT_COUNT(report_size),\
      HID_FEATURE     (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),\
    HID_COLLECTION_END \

static uint8_t const report_descriptor[] =
{
  TUD_HID_REPORT_DESC_R4_INOUT_FEATURE(CFG_TUD_HID_EP_BUFSIZE)
};

class DAPHIDDevice: public USBHIDDevice {
public:
  DAPHIDDevice(void){
    static bool initialized = false;
    if(!initialized){
      initialized = true;
      HID.addDevice(this, sizeof(report_descriptor));
    }
  }
  
  void begin(void){
    HID.begin();
    dap_init();
  }
    
  uint16_t _onGetDescriptor(uint8_t* buffer){
    memcpy(buffer, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
  }
  void _onOutput(uint8_t report_id, const uint8_t* buffer, uint16_t len) {
    static uint8_t TxDataBuffer[CFG_TUD_HID_EP_BUFSIZE];
    dap_process_request((uint8_t*)buffer, len, TxDataBuffer, sizeof(TxDataBuffer));
    HID.SendReport(report_id, TxDataBuffer, sizeof(TxDataBuffer), 5);
  }
  void _onSetFeature(uint8_t report_id, const uint8_t* buffer, uint16_t len){
    if (buffer[0] == 0xAA) {
      usb_persist_restart(RESTART_BOOTLOADER);
    }
  }
  uint16_t _onGetFeature(uint8_t report_id, uint8_t* buffer, uint16_t len) {
    buffer[0] = FIRMWARE_MAJOR;
    buffer[1] = FIRMWARE_MINOR;
    buffer[2] = FIRMWARE_PATCH;
    return 3;
  }
};

extern DAPHIDDevice DAP;