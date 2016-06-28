//==============================================================================
// Simple Example
// * Replace <<SSID>> and <<PASS>> with your WiFi credientials
// * At startup the device will output it's IP and OPC Port to Serial.
// * Connect your OPC Client and send data. Received headers will be output to Serial.
//==============================================================================

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(PARTICLE) || defined(SPARK)
#include "application.h"
#else
#include <SPI.h>
#include <WiFi101.h>
#endif

#include "OpcServer.h"

//==============================================================================
// Configuration
//==============================================================================
const char* WIFI_SSID = "<<SSID>>";
const char* WIFI_PASS = "<<PASS>>";

const int OPC_PORT = 7890;
const int OPC_CHANNEL = 1;      // Channel to respond to in addition to 0 (broadcast channel)
const int OPC_MAX_CLIENTS = 2;  // Maxiumum Number of Simultaneous OPC Clients

// Size of OPC Read Buffer
// * Should probably size to number of pixels * 3 plus 4 byte header
// * If an OPC Message is received that is longer, the remaining pixels are discarded
// * If you are receiving OPC Messages that are longer than the number of pixels you may see increased
//   performance by increasing the buffer size to hold the entire message
const int OPC_MAX_PIXELS = 64;
const int OPC_BUFFER_SIZE = OPC_MAX_PIXELS * 3 + OPC_HEADER_BYTES;
//------------------------------------------------------------------------------

// Callback when a full OPC Message has been received
void cbOpcMessage(uint8_t channel, uint8_t command, uint8_t length, uint8_t* data) {
  Serial.print("chn:");
  Serial.print(channel);
  Serial.print("cmd:");
  Serial.print(command);
  Serial.print("len:");
  Serial.println(length);
}

// Callback when a client is connected
void cbOpcClientConnected(WiFiClient& client) {
  Serial.print("New OPC Client: ");
#if defined(ESP8266) || defined(PARTICLE) || defined(SPARK)
  Serial.println(client.remoteIP());
#else
  Serial.println("(IP Unknown)");
#endif
}

// Callback when a client is disconnected
void cbOpcClientDisconnected(OpcClient& opcClient) {
  Serial.print("Client Disconnected: ");
  Serial.println(opcClient.ipAddress);
}

//==============================================================================
// OpcServer Init
//==============================================================================
WiFiServer server = WiFiServer(OPC_PORT);
OpcClient opcClients[OPC_MAX_CLIENTS];
uint8_t buffer[OPC_BUFFER_SIZE * OPC_MAX_CLIENTS];
OpcServer opcServer = OpcServer(server, OPC_CHANNEL, opcClients, OPC_MAX_CLIENTS, buffer, OPC_BUFFER_SIZE);
//------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  if (WiFi.SSID() != WIFI_SSID) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  opcServer.setMsgReceivedCallback(cbOpcMessage);
  opcServer.setClientConnectedCallback(cbOpcClientConnected);
  opcServer.setClientDisconnectedCallback(cbOpcClientDisconnected);

  opcServer.begin();

  Serial.print("\nOPC Server: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(OPC_PORT);
}

void loop() { opcServer.process(); }
