#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <NimBLEDevice.h>

// Bluetooth service and characteristic UUIDs
constexpr char serviceUUID[] = "ffe0";
constexpr char characteristicUUID[] = "ffe1";

#include "config.h"
#include "html.h"

constexpr int STATUS_LED = 2;
constexpr bool DEBUG = true;

// Wifi and Webserver objects
WiFiClient espClient;
WebServer server(80);
BLERemoteCharacteristic *pRemoteCharacteristic;

// helper functions for debugging
String debugmsg = "";
void debugPrint(String message)
{
  DEBUG ? Serial.print(message) : false;
}
void debugPrintln(String message)
{
  DEBUG ? Serial.println(message) : false;
}

// function to calculate the CRC16 checksum
uint16_t calculateCRC(const uint8_t *data, uint8_t length)
{
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < length; i++)
  {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

// recive buffer and packet
int reciveCount = 0;
bool reciveReady = false;
uint8_t recivePacket[40] = {};
uint8_t reciveBuf[40] = {};

// Callback function for receiving data from the Q900
void reciveData(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  debugPrint("reciving data: ");
  for (size_t i = 0; i < length; i++)
  {
    debugPrint(String(pData[i], HEX) + " ");
  }
  debugPrintln("");

  // Copy the data into the receive buffer
  // ignore the first 4 bytes (prefix) and the last 2 bytes (CRC)
  if (reciveCount == 0)
  {
    memcpy(reciveBuf, &pData[4], length - 4);
    reciveCount += length - 4;
  }
  else
  {
    memcpy(&reciveBuf[reciveCount], pData, length);
    reciveCount += length;
  }

  // CRC from dataLength to the end of data
  uint16_t crc = calculateCRC(reciveBuf, reciveCount - 2); // 4 prefix + length + command + data
  if ((reciveBuf[reciveCount - 2] == (crc >> 8)) && (reciveBuf[reciveCount - 1] == (crc & 0xFF)))
  {
    debugPrintln("CRC OK");
    memcpy(recivePacket, &reciveBuf[2], reciveCount - 4);
    reciveReady = true;
  }
}

// Function to connect to the device
bool connectToServer()
{
  debugPrint("start bluetooth connection..");
  auto pClient = NimBLEDevice::createClient();
  pClient->setConnectionParams(11, 11, 0, 400);
  pClient->setConnectTimeout(5);

  if (!pClient->connect(q900Address, false))
  {
    debugPrintln(" connection failed");
    return false;
  }
  debugPrintln(" connected");

  if (!pClient->discoverAttributes())
  {
    debugPrintln(" attribute discovery failed");
    return false;
  }

  debugPrintln(pClient->toString().c_str());

  auto pService = pClient->getService(serviceUUID);
  if (pService)
  {
    pRemoteCharacteristic = pService->getCharacteristic(characteristicUUID);

    if (pRemoteCharacteristic->canNotify())
    {
      pRemoteCharacteristic->subscribe(true, reciveData);
    }
    else
    {
      debugPrintln(" cannot notify");
      return false;
    }
    return pRemoteCharacteristic != nullptr;
  }
  return false;
}

// Function to send a command to the Q900
bool sendQ900Command(uint8_t commandType, const uint8_t *data, uint8_t dataSize)
{
  if (!pRemoteCharacteristic)
  {
    if (!connectToServer())
    {
      debugPrintln("command failed, no connection");
      return false;
    }
  }
  // reset recive buffer
  reciveCount = 0;
  reciveReady = false;

  // Data length from command type to the end of the checksum
  uint8_t dataLength = dataSize + 3;
  uint8_t packet[20] = {0xA5, 0xA5, 0xA5, 0xA5, dataLength, commandType};
  memcpy(&packet[6], data, dataSize);

  // CRC from the 5th byte to the end of data
  uint8_t crcpacket[20] = {};
  memcpy(&crcpacket, &packet[4], dataSize + 2);
  uint16_t crc = calculateCRC(crcpacket, dataSize + 2);
  packet[dataSize + 6] = (crc >> 8);
  packet[dataSize + 7] = crc & 0xFF;

  String packetStr = "";
  for (int i = 0; i < dataSize + 8; i++)
  {
    packetStr += String(packet[i], HEX) + " ";
  }
  debugPrintln("sending command: " + packetStr);

  if (pRemoteCharacteristic->writeValue(packet, dataSize + 8))
  {
    debugPrintln("command sent");
  }
  else
  {
    pRemoteCharacteristic->unsubscribe();
    NimBLEDevice::deleteClient(pRemoteCharacteristic->getRemoteService()->getClient());
    pRemoteCharacteristic = nullptr;
    debugPrintln("error while sending command, disconnecting..");
    return false;
  }
  delay(100);
  return true;
}

// default handler for error messages
void handleArgumentError(const char *msg)
{
  server.send(500, "text/plain", "{\"error\": \"" + String(msg) + "\"}");
}

// default handler for info messages
void handleArgumentInfo(String msg)
{
  server.send(200, "text/plain", "{\"message\": \"" + msg + "\"}");
}

// Handler for setting the frequency
void handleSetFrequency()
{
  if (server.hasArg("vfoa") && server.hasArg("vfob"))
  {
    uint32_t vfoa = server.arg("vfoa").toInt();
    uint32_t vfob = server.arg("vfob").toInt();
    uint8_t data[8] = {
        static_cast<uint8_t>(vfoa >> 24), static_cast<uint8_t>(vfoa >> 16),
        static_cast<uint8_t>(vfoa >> 8), static_cast<uint8_t>(vfoa),
        static_cast<uint8_t>(vfob >> 24), static_cast<uint8_t>(vfob >> 16),
        static_cast<uint8_t>(vfob >> 8), static_cast<uint8_t>(vfob)};
    sendQ900Command(0x09, data, sizeof(data));
    handleArgumentInfo("frequency set");
    return;
  }
  handleArgumentError("missing parameter: vfoa and vfob required");
}

void handleSwitchOff()
{
  uint8_t data[] = {0x00};
  sendQ900Command(0x0C, data, sizeof(data)) ? handleArgumentInfo("Q900 switched off") : handleArgumentError("error while switching off");
}

void handleSwitchOn()
{
  uint8_t data[] = {0x01};
  sendQ900Command(0x0C, data, sizeof(data)) ? handleArgumentInfo("Q900 switched on") : handleArgumentError("error while switching on");
}

// Handler for setting the mode
void handleSetMode()
{
  if (server.hasArg("modeA") && server.hasArg("modeB"))
  {
    uint8_t modeA = server.arg("modeA").toInt();
    uint8_t modeB = server.arg("modeB").toInt();
    uint8_t data[2] = {modeA, modeB};
    sendQ900Command(0x0A, data, sizeof(data)) ? handleArgumentInfo("mode set") : handleArgumentError("error while setting mode");
  }
  else
  {
    handleArgumentError("missing parameter: modeA and modeB required");
  }
}

// Handler for setting the A/B frequency
void handleABFrequency()
{
  if (server.hasArg("ab"))
  {
    uint8_t ab = server.arg("ab").toInt();
    uint8_t data[] = {ab};
    sendQ900Command(0x1B, data, sizeof(data)) ? handleArgumentInfo("a/b-frequency set") : handleArgumentError("error while setting a/b-frequency");
  }
  else
  {
    handleArgumentError("missing parameter: ab required");
  }
}

// Handler for setting the preamplifier
void handlePreamplifier()
{
  if (server.hasArg("amp"))
  {
    uint8_t amp = server.arg("amp").toInt();
    uint8_t data[] = {amp};
    sendQ900Command(0x17, data, sizeof(data)) ? handleArgumentInfo("preamplifier set") : handleArgumentError("error while setting preamplifier");
  }
  else
  {
    handleArgumentError("missing parameter: amp required");
  }
}

// Handler for setting the filter
void handleFilter()
{
  if (server.hasArg("filter"))
  {
    uint8_t filter = server.arg("filter").toInt();
    uint8_t data[] = {filter};
    sendQ900Command(0x18, data, sizeof(data)) ? handleArgumentInfo("filter set") : handleArgumentError("error while setting filter");
  }
  else
  {
    handleArgumentError("missing parameter: filter required");
  }
}

// Handler for getting the status
void handleGetStatus()
{
  uint8_t data[] = {};
  sendQ900Command(0X0B, data, sizeof(data));

  // Wait for the response
  unsigned long start = millis();
  while (!reciveReady && millis() - start < 1000)
  {
    delay(10);
  }
  if (!reciveReady)
  {
    sendQ900Command(0X0B, data, sizeof(data)); // retry
    start = millis();
    while (!reciveReady && millis() - start < 1000)
    {
      delay(10);
    }
    if (!reciveReady)
    {
      handleArgumentError("error while reading parameters");
      return;
    }
  }

  if (reciveCount > 23)
  {
    String status = "{";
    status += "\"Transceiver_Status\": " + String(recivePacket[0]) + ", ";                                                                           // Transceiver-Status
    status += "\"VFOA_Mode\": " + String(recivePacket[1]) + ", ";                                                                                    // VFOA-Modus
    status += "\"VFOB_Mode\": " + String(recivePacket[2]) + ", ";                                                                                    // VFOB-Modus
    status += "\"VFOA_Frequency\": " + String((recivePacket[3] << 24) | (recivePacket[4] << 16) | (recivePacket[5] << 8) | recivePacket[6]) + ", ";  // VFOA-Frequenz
    status += "\"VFOB_Frequency\": " + String((recivePacket[7] << 24) | (recivePacket[8] << 16) | (recivePacket[9] << 8) | recivePacket[10]) + ", "; // VFOB-Frequenz
    status += "\"A_B\": " + String(recivePacket[11]) + ", ";                                                                                         // A/B-Frequenz
    status += "\"NR_NB\": " + String(recivePacket[12]) + ", ";                                                                                       // NR/NB-Status
    status += "\"RXT\": " + String(recivePacket[13]) + ", ";                                                                                         // RXT
    status += "\"XIT\": " + String(recivePacket[14]) + ", ";                                                                                         // XIT
    status += "\"Filter_Bandwidth\": " + String(recivePacket[15]) + ", ";                                                                            // Filterbandbreite
    status += "\"Spectrum_Bandwidth\": " + String(recivePacket[16]) + ", ";                                                                          // Spektrumbandbreite
    status += "\"Voltage\": " + String(recivePacket[17] / 10.0) + ", ";                                                                              // Spannung (in Dezimal)
    status += "\"UTC_Time\": \"" + String(recivePacket[18]) + ":" + String(recivePacket[19]) + ":" + String(recivePacket[20]) + "\", ";              // Uhrzeit
    status += "\"Status_Bar\": " + String(recivePacket[21]) + ", ";                                                                                  // Statusleiste
    status += "\"S_Table\": " + String(recivePacket[22]) + ", ";                                                                                     // S-Tabelle
    status += "\"PO_Table\": " + String(recivePacket[23]) + ", ";                                                                                    // Po-Tabelle
    status += "\"SWR_AUD_ALC\": " + String(recivePacket[24]);                                                                                        // SWR/AUD/ALC-Status
    status += "}";
    server.send(200, "application/json", status);
    return;
  }
  else
  {
    handleArgumentError("error while reading parameters");
    return;
  }
}

// Wifi reconnect event
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  debugPrint("WiFi disconnected, reason: ");
  debugPrintln(String(info.wifi_sta_disconnected.reason));
  debugPrint("reconnecting..");
  WiFi.reconnect();
}

// Setup WiFi
void setup_wifi()
{
  // delete old config
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  delay(10);
  debugPrintln("");
  debugPrint("connecting to ");
  debugPrint(ssid);
  debugPrint("..");

  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  for (int i = 1; WiFi.status() != WL_CONNECTED; i++)
  {
    delay(500);
    // Switch LED
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    debugPrint(".");
    i++;
    if (i >= 20)
      ESP.restart();
  }
  digitalWrite(STATUS_LED, HIGH);
  debugPrintln("connected");
  debugPrint("IP address: ");
  debugPrintln(WiFi.localIP().toString());
}

// reconnect to wifi
void reconnect()
{
  // Warte auf Wlan oder restarte den ESP
  if (WiFi.status() != WL_CONNECTED)
  {
    debugPrint("wifi connection lost, try to reconnect..");
    // WiFi.reconnect();
    for (int i = 1; WiFi.status() != WL_CONNECTED; i++)
    {
      digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
      debugPrint(".");
      if (i >= 20)
        ESP.restart();
      delay(500);
    }
    digitalWrite(STATUS_LED, HIGH);
    debugPrintln("connected");
  }
}

void setup()
{
  if (DEBUG)
    Serial.begin(115200);

  // Print Informationen über den Neustart
  debugPrintln("Restarting..");
  debugPrint("Chip Revision: ");
  debugPrintln(String(ESP.getChipRevision()));
  debugPrint("Chip Model: ");
  debugPrintln(String(ESP.getChipModel()));
  debugPrint("Free Heap: ");
  debugPrintln(String(ESP.getFreeHeap()));
  debugPrint("Free Sketch Space: ");
  debugPrintln(String(ESP.getFreeSketchSpace()));

  // init PinModes
  pinMode(STATUS_LED, OUTPUT);

  setup_wifi();

  // OTA Konfiguration
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.onStart([]()
                     {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    debugPrintln("ota start: " + type); });
  ArduinoOTA.onEnd([]()
                   { debugPrintln("\nota done"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { debugPrint("ota progress: ");
                          debugPrint(String(progress / (total / 100)));
                          debugPrintln("%\r"); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    debugPrint("error[");
    debugPrint(String(error));
    debugPrint("]: ");
    switch (error) {
      case OTA_AUTH_ERROR: debugPrintln("auth error"); break;
      case OTA_BEGIN_ERROR: debugPrintln("start error"); break;
      case OTA_CONNECT_ERROR: debugPrintln("connect error"); break;
      case OTA_RECEIVE_ERROR: debugPrintln("receive error"); break;
      case OTA_END_ERROR: debugPrintln("end error"); break;
    } });
  ArduinoOTA.begin();
  ArduinoOTA.handle();

  // configure webserver
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", html_page); });
  server.on("/setFrequency", handleSetFrequency);
  server.on("/favicon.ico", HTTP_GET, []()
            { server.send(404, "text/plain", "Not found"); });
  server.on("/off", handleSwitchOff);
  server.on("/on", handleSwitchOn);
  server.on("/mode", handleSetMode);
  server.on("/abFrequency", handleABFrequency);
  server.on("/preamplifier", handlePreamplifier);
  server.on("/filter", handleFilter);
  server.on("/getstatus", HTTP_GET, handleGetStatus);
  server.begin();

  // init bluetooth
  NimBLEDevice::init("");
  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
  NimBLEDevice::setPower(ESP_PWR_LVL_N0);
  connectToServer();
}

void loop()
{
  server.handleClient();
  ArduinoOTA.handle();
}