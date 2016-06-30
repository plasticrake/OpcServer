#pragma once

#include "Config.h"
#include "Platforms.h"

enum OpcCommand {
  OPC_SET_PIXEL_COLORS = 0x00,
  OPC_SYSTEM_EXCLUSIVE = 0xFF,
};

enum OpcSysEx {
  OPC_GLOBAL_COLOR_CORRECTION = 0x00010001,
  OPC_FIRMWARE_CONFIGURATION = 0x00010002
};

static const uint8_t OPC_HEADER_BYTES = 4;

template <class T, int _size>
struct OpcMessage {
  OpcMessage() : channel(0), command(0), lenHigh(0), lenLow(0) {}

  uint16_t getLength() const {
    return lenLow | (unsigned(lenHigh) << 8);
  }

  void setLength(unsigned l) {
    lenLow = (uint8_t)l;
    lenHigh = (uint8_t)(l >> 8);
  }

  uint8_t channel;
  uint8_t command;
  uint8_t lenHigh;
  uint8_t lenLow;
  T data[_size];
};

struct OpcClient {
  enum ClientState {
    CLIENT_STATE_DISCONNECTED = 0,
    CLIENT_STATE_CONNECTED
  };

  OpcClient() : ipAddress((uint32_t)0), state(CLIENT_STATE_DISCONNECTED), bytesAvailable(0), bufferBytesToDiscard(0), bufferLength(0), bufferSize(0), buffer(0) {}

  WiFiClient tcpClient;
  IPAddress ipAddress;
  ClientState state;
  uint32_t bytesAvailable;
  uint32_t bufferBytesToDiscard;
  uint32_t bufferLength;
  uint32_t bufferSize;
  uint8_t* buffer;
};

typedef void (*OpcMsgReceivedCallback)(uint8_t channel, uint8_t command, uint8_t length, uint8_t* data);
typedef void (*OpcClientConnectedCallback)(WiFiClient&);
typedef void (*OpcClientDisconnectedCallback)(OpcClient&);
