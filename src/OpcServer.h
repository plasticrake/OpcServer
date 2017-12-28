#pragma once

#include "Platforms.h"

#include "Config.h"
#include "Opc.h"

class OpcServer {
 public:
  OpcServer(WiFiServer& server,
            uint8_t opcChannel,
            OpcClient opcClients[],
            uint8_t clientSize,
            uint8_t buffer[],
            uint32_t bufferSize,
            OpcMsgReceivedCallback opcMsgReceivedCallback = [](uint8_t channel, uint8_t command, uint16_t length, uint8_t* data) -> void {},
            OpcClientConnectedCallback opcClientConnectedCallback = [](WiFiClient&) -> void {},
            OpcClientDisconnectedCallback opcClientDisconnectedCallback = [](OpcClient&) -> void {});

  bool begin();
  void process();

  uint32_t getBufferSize() const;
  uint16_t getBufferSizeInPixels() const;
  uint32_t getBytesAvailable() const;
  uint8_t getClientCount() const;
  uint8_t getClientSize() const;

  void setClientConnectedCallback(OpcClientConnectedCallback opcClientConnectedCallback);
  void setClientDisconnectedCallback(OpcClientDisconnectedCallback opcClientDisconnectedCallback);
  void setMsgReceivedCallback(OpcMsgReceivedCallback opcMsgReceivedCallback);

#if defined(ESP32)
  void mDNSBegin(String hostname);
#endif

 private:
  bool processClient(OpcClient& opcClient);
  void opcRead(OpcClient& opcClient);

  OpcClient* opcClients_;

  WiFiServer& server_;

  OpcMsgReceivedCallback opcMsgReceivedCallback_;
  OpcClientConnectedCallback opcClientConnectedCallback_;
  OpcClientDisconnectedCallback opcClientDisconnectedCallback_;

  uint32_t bufferSize_;

  uint8_t opcChannel_;
  uint8_t clientSize_;
  uint8_t clientCount_;
};
