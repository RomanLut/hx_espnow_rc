#pragma once

#define LED_PIN 2
#define USE_SERIAL1_RX_PIN 27
#define SPORT_PIN 18

#define CP2102_RX_PIN  1

#define USE_WIFI_CHANNEL 3
#define USE_KEY 0

//when SPORT support is enabled:
//- UART0 TX and RX pins are remaped from USB to SPORT_PIN
//- UART0 initialized to 57600 baud rate
//- Software UART 115200 is created and attached to USB
//- esp_log_set_vprintf() redirects ESP logging library to software uart
//- HXRCLOG is set to SoftwareSerial
#define USE_SPORT

//= Dependent definitions ========================================================
#ifdef USE_SPORT
#else
#endif


/*
//=====================================================================
//=====================================================================
void HXRCGetLogStream(const char *fmt, va_list args)
{
  char buffer[256];
  vsnprintf(buffer, 256, fmt, args);
  buffer[255] = 0;
  Serial.print ("Redirected:");
  Serial.print (buffer);

  return strlen(buffer);
}

*/