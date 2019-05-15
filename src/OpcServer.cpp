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
  uint32_t msgLength = 0;

  if(opcClient.bytesAvailable < opcClient.header.dataLength) {
    readLen = client.read((uint8_t*)buf + opcClient.bufferLength, opcClient.bytesAvailable);
    opcClient.bufferLength += readLen;
    msgLength = OPC_HEADER_BYTES + opcClient.header.dataLength;
    debug_sprint("Received TCP with ", opcClient.bytesAvailable,  "Bytes Appended to buffer now at ", opcClient.bufferLength, " data bytes\n");
  } else {
    // We have a header! Read Msg
    debug_sprint("New Data Packet\n");
    opcClient.header.channel = buf[0];
    opcClient.header.command = buf[1];
    opcClient.header.lenHigh = buf[2];
    opcClient.header.lenLow = buf[3];
    opcClient.header.dataLength = opcClient.header.lenLow | (unsigned(opcClient.header.lenHigh) << 8);

    msgLength = OPC_HEADER_BYTES + opcClient.header.dataLength;

    readLen = client.read((uint8_t*)buf + opcClient.bufferLength, opcClient.bytesAvailable);
    opcClient.bufferLength += readLen;
  }

  if (opcClient.bufferLength < msgLength) {
    // Waiting for more data
    debug_sprint("Waiting for more data only Received ", opcClient.bufferLength, "\n");
  } else {
    // Full OPC Message Read
    debug_sprint("Received Complete Data Packet\n");
    opcMsgReceivedCallback_(opcClient.header.channel, opcClient.header.command, opcClient.header.dataLength, buf + OPC_HEADER_BYTES);

    // Set to start buffer over on next call
    opcClient.bufferLength = 0;
    opcClient.header.channel = 0;
    opcClient.header.command = 0;
    opcClient.header.lenHigh = 0;
    opcClient.header.lenLow = 0;
    opcClient.header.dataLength = 0;
  }
}
