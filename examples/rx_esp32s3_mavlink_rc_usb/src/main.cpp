#include <Arduino.h>
#include "HX_ESPNOW_RC_Slave.h"
#include "rx_config.h"
#include "hx_mavlink_rc_encoder.h"
#include "HX_ESPNOW_RC_SerialBuffer.h"

#include <esp_task_wdt.h>

#include <ArduinoOTA.h>
#include "USB.h"
#include "USBCDC.h"
#include "USBHID.h"
#include "esp32-hal-tinyusb.h"

#define WDT_TIMEOUT_SECONDS 3  

HXRCSlave hxrcSlave;
HXRCSerialBuffer<512> hxrcTelemetrySerial( &hxrcSlave );
HXMavlinkRCEncoder hxMavlinkRCEncoder;

unsigned long lastStats = millis();

//0 - got connection once
//1 - switched to normal mode
//2 - waiting for connection
//other - millis() at startup
uint8_t state = 2;
unsigned long startTime = millis();

HardwareSerial mavlinkSerial(2);
USBCDC mavlinkUsbSerial;

static const uint8_t keyboardReportDescriptor[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_REPORT_ID_KEYBOARD))
};

//=====================================================================
//=====================================================================
// Provides keyboard reports without advertising the BIOS boot-keyboard protocol.
// Quest keeps boot keyboards in its input subsystem but can withhold that composite
// device from application UsbManager, which also makes the CDC interfaces invisible.
// Report protocol preserves runtime keyboard input while allowing GS to open CDC.
class ReportProtocolKeyboard : public USBHIDDevice
{
private:
  USBHID hid;
  hid_keyboard_report_t report;

  void sendReport()
  {
    hid.SendReport(HID_REPORT_ID_KEYBOARD, &report, sizeof(report));
  }

public:
  ReportProtocolKeyboard()
    : hid(HID_ITF_PROTOCOL_NONE), report{}
  {
    USBHID::addDevice(this, sizeof(keyboardReportDescriptor));
  }

  uint16_t _onGetDescriptor(uint8_t *buffer) override
  {
    memcpy(buffer, keyboardReportDescriptor, sizeof(keyboardReportDescriptor));
    return sizeof(keyboardReportDescriptor);
  }

  void begin()
  {
    hid.begin();
  }

  void press(uint8_t keycode)
  {
    for (size_t i = 0; i < sizeof(report.keycode); i++)
    {
      if (report.keycode[i] == keycode)
      {
        return;
      }
      if (report.keycode[i] == 0)
      {
        report.keycode[i] = keycode;
        sendReport();
        return;
      }
    }
  }

  void release(uint8_t keycode)
  {
    for (size_t i = 0; i < sizeof(report.keycode); i++)
    {
      if (report.keycode[i] == keycode)
      {
        report.keycode[i] = 0;
        sendReport();
        return;
      }
    }
  }
};

ReportProtocolKeyboard usbKeyboard;

Stream& getMavlinkSerial()
{
#if USE_USB_CDC
  return mavlinkUsbSerial;
#else
  return mavlinkSerial;
#endif
}

struct KeyBinding
{
  uint8_t gpio;
  uint8_t keycode;
  bool isPressed;
  unsigned long debounceUntilMs;
};

KeyBinding keyBindings[] = {
    {KEY_UP_GPIO, HID_KEY_ARROW_UP, false, 0},
    {KEY_DOWN_GPIO, HID_KEY_ARROW_DOWN, false, 0},
    {KEY_LEFT_GPIO, HID_KEY_ARROW_LEFT, false, 0},
    {KEY_RIGHT_GPIO, HID_KEY_ARROW_RIGHT, false, 0},
    {KEY_ENTER_GPIO, HID_KEY_ENTER, false, 0},
    {KEY_R_GPIO, HID_KEY_R, false, 0},
    {KEY_G_GPIO, HID_KEY_G, false, 0},
};
const size_t keyBindingsCount = sizeof(keyBindings) / sizeof(keyBindings[0]);
const unsigned long KEY_DEBOUNCE_MS = 100;

//=====================================================================
//=====================================================================
void processIncomingTelemetry()
{
  Stream& mavlink = getMavlinkSerial();
  while ( hxrcTelemetrySerial.getAvailable() > 0 && mavlink.availableForWrite() > 0)
  {
    uint8_t c = hxrcTelemetrySerial.read();
    //TODO: support incoming telemetry
  }
}

//=====================================================================
//=====================================================================
void fillOutgoingTelemetry()
{
  Stream& mavlink = getMavlinkSerial();
  while ( (mavlink.available() > 0) && (hxrcTelemetrySerial.getAvailableForWrite() > 0) )
  {
    uint8_t c = mavlink.read();
    hxrcTelemetrySerial.write(c);
  }
}

//=====================================================================
//=====================================================================
void updateKeyboard()
{
  unsigned long now = millis();

  for (size_t i = 0; i < keyBindingsCount; i++)
  {
    if ((long)(now - keyBindings[i].debounceUntilMs) < 0)
    {
      continue;
    }

    bool pressed = (digitalRead(keyBindings[i].gpio) == LOW);
    if (pressed == keyBindings[i].isPressed)
    {
      continue;
    }

    keyBindings[i].isPressed = pressed;
    keyBindings[i].debounceUntilMs = now + KEY_DEBOUNCE_MS;
    if (pressed)
    {
      usbKeyboard.press(keyBindings[i].keycode);
    }
    else
    {
      usbKeyboard.release(keyBindings[i].keycode);
    }
  }
}

//=====================================================================
//=====================================================================
void onOTAprogress( uint a, uint b )  
{
  esp_task_wdt_reset();
}

//=====================================================================
//=====================================================================
void setup()
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  esp_task_wdt_config_t watchdogConfig = {};
  watchdogConfig.timeout_ms = WDT_TIMEOUT_SECONDS * 1000;
  // This firmware explicitly subscribes and feeds the Arduino loop task below.
  // Do not also watch the idle tasks: ESP-NOW/USB startup can legitimately keep
  // a core busy for three seconds while the application loop remains healthy.
  watchdogConfig.idle_core_mask = 0;
  watchdogConfig.trigger_panic = true;

  // Arduino 3 may initialize the task watchdog before setup(), so apply this
  // firmware's timeout to the existing watchdog instead of leaving its default.
  if (esp_task_wdt_init(&watchdogConfig) == ESP_ERR_INVALID_STATE)
  {
    esp_task_wdt_reconfigure(&watchdogConfig);
  }
#else
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true); //enable panic so ESP32 restarts
#endif
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.begin(115200);

  pinMode(KEY_R_GPIO, INPUT_PULLUP);
  if (digitalRead(KEY_R_GPIO) == LOW)
  {
    delay(100);
    if (digitalRead(KEY_R_GPIO) == LOW)
    {
      usb_persist_restart(RESTART_BOOTLOADER);
    }
  }

#if USE_USB_CDC
  mavlinkUsbSerial.begin(TELEMETRY_BAUDRATE);
#endif
  usbKeyboard.begin();
  // Describe this as an interface-classed composite device. Arduino's default
  // 0xEF/0x02/0x01 tuple is also claimed by Meta Remote Desktop on Quest, which
  // makes every receiver replug launch its overlay. CDC and HID remain declared
  // by their individual interface descriptors when the device tuple is zero.
  USB.usbClass(0);
  USB.usbSubClass(0);
  USB.usbProtocol(0);
  USB.begin();

#if !USE_USB_CDC
  pinMode(MAVLINK_TX_PIN, OUTPUT);
  mavlinkSerial.begin(TELEMETRY_BAUDRATE, SERIAL_8N1, MAVLINK_RX_PIN, MAVLINK_TX_PIN);
#endif

  for (size_t i = 0; i < keyBindingsCount; i++)
  {
    pinMode(keyBindings[i].gpio, INPUT_PULLUP);
  }

  hxMavlinkRCEncoder.init( MAVLINK_RC_PACKET_RATE_MS, USE_MAVLINK_V1 );

  hxrcSlave.init(
      HXRCConfig(
          USE_WIFI_CHANNEL,
          USE_KEY,
          USE_LR_MODE,
          -1, false));

  //REVIEW: receiver does not work if AP is not initialized?
  WiFi.softAP("hxrcmavlink", NULL, USE_WIFI_CHANNEL);

  ArduinoOTA.onProgress(&onOTAprogress);
  ArduinoOTA.begin();  
}

//=====================================================================
//=====================================================================
void updateOutput()
{
  //set failsafe flag
  bool failsafe = hxrcSlave.getReceiverStats().isFailsafe();
  hxMavlinkRCEncoder.setFailsafe( failsafe);


  //inject RSSI into channel 16
  hxMavlinkRCEncoder.setChannelValue( USE_MAVLINK_V1 ? MAVLINK_RC_CHANNELS_COUNT_V1 - 1 : MAVLINK_RC_CHANNELS_COUNT-1, 1000 + ((uint16_t)hxrcSlave.getReceiverStats().getRSSI())*10 );

  if ( !failsafe ) //keep last channel values on failsafe
  {
    HXRCChannels channels = hxrcSlave.getChannels();
    for ( int i = 0; i < MAVLINK_RC_CHANNELS_COUNT-1; i++)
    {
      hxMavlinkRCEncoder.setChannelValue( i, channels.getChannelValue(i) );
    }

    if ( state == 1 )
    {
      Serial.println("Rebooting to LR mode");
      delay(100);
      ESP.restart();
      delay(1000);
    }
    state = 0;
  }

#if USE_USB_CDC
  hxMavlinkRCEncoder.loop(getMavlinkSerial());
#else
  hxMavlinkRCEncoder.loop(getMavlinkSerial());
#endif
}

//=====================================================================
//=====================================================================
void loop()
{
  esp_task_wdt_reset();

  hxrcTelemetrySerial.flushIn();
  processIncomingTelemetry();
  
  fillOutgoingTelemetry();
  hxrcTelemetrySerial.flushOut();

  hxrcSlave.setA2(hxrcSlave.getReceiverStats().getRSSI());

  hxrcSlave.loop();

/*
  if (millis() - lastStats > 1000)
  {
    lastStats = millis();

    Serial.print("Peer:");
    HXRCPrintMac(hxrcSlave.getPeerMac());
    
    Serial.print(" packets:");
    Serial.print(capture.packetsCount);

    Serial.print(" rssi:");
    Serial.print(capture.rssi);

    Serial.print(" noise_floor:");
    Serial.print(capture.noiseFloor);

    Serial.print(" rate:");
    Serial.println(capture.rate);

    hxrcSlave.getTransmitterStats().printStats();
    hxrcSlave.getReceiverStats().printStats();
  }
*/
  updateOutput();
  updateKeyboard();

  if ( hxrcSlave.getReceiverStats().isFailsafe() )
  {
    ArduinoOTA.handle();  
  }

  if ( 
      USE_LR_MODE &&
      (state == 2) && 
      ((NORMAL_MODE_DELAY_MS) > 0) &&
      ((millis() - startTime) > (NORMAL_MODE_DELAY_MS) ) 
      )
  {
    //if there is no transmitter connection after 1 minute after powerup, and LR more is enabled, 
    //switch to normal mode to show AP and allow OTA updates

    Serial.println("Switching to normal mode");

    state = 1;

    if (esp_wifi_set_protocol (WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N ) != ESP_OK)
    {
      Serial.println("HXRC: Error: Failed to enable normal mode");
    }

  }
}
 
