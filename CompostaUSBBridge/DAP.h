#include "USB.h"
#include "USBHID.h"

extern "C" {
    #include "esp32-hal-tinyusb.h"
    #include "freedap.h"
}

#include "at_handler.h"

extern USBHID HID;

static uint8_t const report_descriptor[] =
{
  TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE)
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
    buffer[0] = FIRMWARE_MAYOR;
    buffer[1] = FIRMWARE_MINOR;
    buffer[2] = FIRMWARE_PATCH;
    return 3;
  }
};

extern DAPHIDDevice DAP;