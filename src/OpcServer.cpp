#include "OpcServer.h"
#include "Definitions.h"

OpcServer::OpcServer(WiFiServer& server,
                     uint8_t opcChannel,
                     OpcClient opcClients[],
                     uint8_t clientSize,
                     uint8_t buffer[],
                     uint32_t bufferSize,
                     OpcMsgReceivedCallback OpcMsgReceivedCallback,
                     OpcClientConnectedCallback opcClientConnectedCallback,
                     OpcClientDisconnectedCallback opcClientDisconnectedCallback)
    : opcClients_(opcClients),
      server_(server),
      opcMsgReceivedCallback_(OpcMsgReceivedCallback),
      opcClientConnectedCallback_(opcClientConnectedCallback),
      opcClientDisconnectedCallback_(opcClientDisconnectedCallback),
      bufferSize_(bufferSize),
      opcChannel_(opcChannel),
      clientSize_(clientSize),
      clientCount_(0) {
  for (size_t i = 0; i < clientSize_; i++) {
    opcClients_[i].bufferSize = bufferSize_;
    opcClients_[i].buffer = buffer + (bufferSize_ * i);
  }
}

uint8_t OpcServer::getClientCount() const {
  return clientCount_;
}

uint8_t OpcServer::getClientSize() const {
  return clientSize_;
}

uint32_t OpcServer::getBufferSize() const {
  return bufferSize_;
}

uint16_t OpcServer::getBufferSizeInPixels() const {
  return ((bufferSize_ - OPC_HEADER_BYTES) / 3);
}

uint32_t OpcServer::getBytesAvailable() const {
  uint32_t b = 0;
  for (size_t i = 0; i < clientSize_; i++) {
    b += opcClients_[i].bytesAvailable;
  }
  return b;
}

void OpcServer::setMsgReceivedCallback(OpcMsgReceivedCallback opcMsgReceivedCallback) {
  opcMsgReceivedCallback_ = opcMsgReceivedCallback;
}

void OpcServer::setClientConnectedCallback(OpcClientConnectedCallback opcClientConnectedCallback) {
  opcClientConnectedCallback_ = opcClientConnectedCallback;
}

void OpcServer::setClientDisconnectedCallback(OpcClientDisconnectedCallback opcClientDisconnectedCallback) {
  opcClientDisconnectedCallback_ = opcClientDisconnectedCallback;
}

bool OpcServer::begin() {
#if SERVER_BEGIN_BOOL
  if (!server_.begin()) {
    return false;
  }
#else
  server_.begin();
#endif

  return true;
}

void OpcServer::process() {
  // process existing clients
  clientCount_ = 0;
  for (size_t i = 0; i < clientSize_; i++) {
    if (processClient(opcClients_[i])) {
      clientCount_++;
    } else if (opcClients_[i].state == OpcClient::CLIENT_STATE_CONNECTED) {
      opcClients_[i].state = OpcClient::CLIENT_STATE_DISCONNECTED;
      opcClients_[i].tcpClient.stop();
      opcClients_[i].bytesAvailable = 0;
      opcClientDisconnectedCallback_(opcClients_[i]);
    }
  }

  // Any new clients?
  WiFiClient tcpClient = server_.available();
  if (tcpClient) {
    opcClientConnectedCallback_(tcpClient);

    // Check if we have room for a new client
    if (clientCount_ >= clientSize_) {
      info_sprint("Too many clients, Connection Refused\n");
      tcpClient.stop();
    } else {
      for (size_t i = 0; i < clientSize_; i++) {
        if (!opcClients_[i].tcpClient.connected()) {
          if (opcClients_[i].state != OpcClient::CLIENT_STATE_DISCONNECTED) {
            opcClientDisconnectedCallback_(opcClients_[i]);
          }
          opcClients_[i].tcpClient.stop();
          opcClients_[i].bytesAvailable = 0;
          opcClients_[i].tcpClient = tcpClient;
#if HAS_REMOTE_IP
          opcClients_[i].ipAddress = tcpClient.remoteIP();
#endif
          opcClients_[i].state = OpcClient::CLIENT_STATE_CONNECTED;
          break;
        }
      }
    }
  }
}

bool OpcServer::processClient(OpcClient& opcClient) {
  if (opcClient.tcpClient.connected()) {
    opcClient.state = OpcClient::CLIENT_STATE_CONNECTED;

    if ((opcClient.bytesAvailable = opcClient.tcpClient.available()) > 0) {
      opcRead(opcClient);
      perf_sprint("*");
    } else {
      perf_sprint(".");
    }
    return true;
  }
  return false;
}

void OpcServer::opcRead(OpcClient& opcClient) {
  size_t readLen;

  WiFiClient client = opcClient.tcpClient;
  uint8_t* buf = opcClient.buffer;

  if (opcClient.bufferBytesToDiscard > 0) {
    warn_sprint(F("*** DISCARDING BYTES *** bytesToDiscard: "), opcClient.bufferBytesToDiscard, F(" bytesAvailable: "), opcClient.bytesAvailable);
    warn_sprint(F(" buffersize: "), bufferSize_, '\n');
    while (opcClient.bufferBytesToDiscard > 0 && opcClient.bytesAvailable > 0) {
      readLen = client.read((uint8_t*)buf, min(opcClient.bytesAvailable, min(opcClient.bufferBytesToDiscard, bufferSize_)));
      if (readLen == 0) { break; }
      opcClient.bytesAvailable -= readLen;
      opcClient.bufferBytesToDiscard -= readLen;
    }
    if (opcClient.bufferBytesToDiscard > 0 || opcClient.bytesAvailable == 0) {
      // waiting for more bytes to discard OR no more bytes to read
      debug_sprint(F("Waiting for bytes to discard\n"));
      return;
    } else {
      warn_sprint(F("=== DISCARD DONE ===\n"));
    }
  }

  if (opcClient.bufferLength < OPC_HEADER_BYTES) {
    // Read Header
    readLen = client.read((uint8_t*)buf + opcClient.bufferLength, min(opcClient.bytesAvailable, OPC_HEADER_BYTES));
    opcClient.bufferLength += readLen;

    if (opcClient.bufferLength < OPC_HEADER_BYTES) {
      // Still waiting for a header
      debug_sprint(F("Waiting for Header\n"));
    }
  }

  // We have a header! Read Msg
  uint8_t channel = buf[0];
  uint8_t command = buf[1];
  uint8_t lenHigh = buf[2];
  uint8_t lenLow = buf[3];
  uint16_t dataLength = lenLow | (unsigned(lenHigh) << 8);

  uint32_t msgLength = OPC_HEADER_BYTES + dataLength;
  uint32_t adjMsgLength = min(msgLength, bufferSize_);

  readLen = client.read((uint8_t*)buf + opcClient.bufferLength, min(opcClient.bytesAvailable, adjMsgLength - opcClient.bufferLength));
  opcClient.bufferLength += readLen;

  if (opcClient.bufferLength < adjMsgLength) {
    // Waiting for more data
    debug_sprint("Waiting for more data\n");
  }

  // Full OPC Message Read
  opcMsgReceivedCallback_(channel, command, dataLength, buf + OPC_HEADER_BYTES);

  // Set to start buffer over on next call
  opcClient.bufferLength = 0;

  // Set to discard remaining bytes on next call
  opcClient.bufferBytesToDiscard = msgLength - adjMsgLength;
}
