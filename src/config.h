#ifndef CONFIG_H
#define CONFIG_H

// Bluetooth-Adress of the Q900
static NimBLEAddress q900Address("00:00:00:00:00:00");

// WLAN Configuration
const char* ssid = "ssid";
const char* password = "password";
const char* hostname = "Q900";

const char* ota_password = "otapass_change_me";

#endif // CONFIG_H