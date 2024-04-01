#include "net/udp.h"
#include "common/types.h"
#include "memorymanagement.h"
#include "net/ipv4.h"

using namespace myos;
using namespace net;
using namespace common;

UserDatagramProtocolHandler::UserDatagramProtocolHandler() {}
// void UserDatagramProtocolHandler::HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size) {}

UserDatagramProtocolSocket::UserDatagramProtocolSocket(UserDatagramProtocolProvider* backend) 
: backend(backend) {
  handler = 0;
  listening = true;
}
UserDatagramProtocolSocket::~UserDatagramProtocolSocket() {}

void UserDatagramProtocolSocket::HandleUserDatagramProtocolMessage(uint8_t* data, uint16_t size) {
  if (handler != 0) handler->HandleUserDatagramProtocolMessage(this, data, size);
}

void UserDatagramProtocolSocket::Send(uint8_t* data, uint16_t size) {
  backend->Send(this, data, size);
}

void UserDatagramProtocolSocket::DisConnect() {
  backend->DisConnect(this);
}

UserDatagramProtocolProvider::UserDatagramProtocolProvider(InternetProtocolV4Provider* backend) 
  : InternetProtocolV4Handler(backend, 0x11) {
    for (int i = 0; i < 65535; i++) {
      sockets[i] = 0;
    }
    numSockets = 0;
    freePort = 1024;
  }
UserDatagramProtocolProvider::~UserDatagramProtocolProvider() {}

bool UserDatagramProtocolProvider::OnInternetProtocolReceive(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* internetProtocolPayload, uint32_t size) {
  if (size < sizeof(UserDatagramProtocolMessage)) return false;
  UserDatagramProtocolMessage* message = (UserDatagramProtocolMessage*) internetProtocolPayload;
  uint16_t localPort = message->dstPort;
  uint16_t remotePort = message->srcPort;
  UserDatagramProtocolSocket* socket = 0;
  for (int i = 0; i < numSockets && socket == 0; i++) {
    if (sockets[i]->localPort == localPort && sockets[i]->localIP == dstIP_BE) {
      if (sockets[i]->listening) {
        socket = sockets[i];
        socket->listening = false;
        socket->remotePort = message->srcPort;
        socket->remoteIP = srcIP_BE;
      } else if (sockets[i]->remotePort == message->srcPort && sockets[i]->remoteIP == srcIP_BE) {
        socket = sockets[i];
      }
    }
  }

  if (socket != 0) {
    socket->HandleUserDatagramProtocolMessage(internetProtocolPayload + sizeof(UserDatagramProtocolMessage), size - sizeof(UserDatagramProtocolMessage));
  }

  return false;
}

UserDatagramProtocolSocket* UserDatagramProtocolProvider::Connect(common::uint32_t ip, common::uint16_t port) {
  UserDatagramProtocolSocket* socket = (UserDatagramProtocolSocket*) MemoryManager::activeMemoryManager->malloc(sizeof(UserDatagramProtocolMessage));
  if (socket != 0) {
    new(socket)UserDatagramProtocolSocket(this);

    socket->remotePort = port;
    socket->remoteIP = ip;
    socket->localPort = freePort++;
    socket->localIP = backend->GetIPAddress();

    socket->remotePort = ((socket->remotePort & 0xFF00) >> 8) | ((socket->remotePort & 0x00FF) << 8);
    socket->localPort = ((socket->localPort & 0xFF00) >> 8) | ((socket->localPort & 0x00FF) << 8);
    sockets[numSockets++] = socket;
  }

  return socket;
}

UserDatagramProtocolSocket* UserDatagramProtocolProvider::Listen(common::uint16_t port) {
  UserDatagramProtocolSocket* socket = (UserDatagramProtocolSocket*)MemoryManager::activeMemoryManager->malloc(sizeof(UserDatagramProtocolSocket));
  if (socket != 0) {
    new(socket)UserDatagramProtocolSocket(this);

    socket->listening = true;
    socket->localPort = port;
    socket->localIP = backend->GetIPAddress();

    socket->localPort = ((socket->localPort & 0xFF00) >> 8) | ((socket->localPort & 0x00FF) << 8);
    sockets[numSockets++] = socket;
  }

  return socket;
}

void UserDatagramProtocolProvider::DisConnect(UserDatagramProtocolSocket* socket) {
  for (int i = 0; i < numSockets; i++) {
    if (sockets[i] == socket) {
      sockets[i] = sockets[--numSockets];
      MemoryManager::activeMemoryManager->free(socket);
      break;
    } 
  }
}

void UserDatagramProtocolProvider::Send(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size) {
  uint16_t totalLength = size + sizeof(UserDatagramProtocolSocket);
  uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager->malloc(totalLength);
  uint8_t* buffer2 = buffer + sizeof(UserDatagramProtocolSocket);

  UserDatagramProtocolMessage* msg = (UserDatagramProtocolMessage*)buffer;
  msg->srcPort = socket->localPort;
  msg->dstPort = socket->remotePort;
  msg->length = ((totalLength & 0xFF00) >> 8) | ((totalLength & 0x00FF) << 8);

  for (int i = 0; i < size; i++) {
    buffer2[i] = data[i];
  }
  msg->checkSum = 0;
  InternetProtocolV4Handler::Send(socket->remoteIP, buffer, totalLength);
  MemoryManager::activeMemoryManager->free(buffer);
} 

void UserDatagramProtocolProvider::Bind(UserDatagramProtocolSocket* socket, UserDatagramProtocolHandler* handler) {
  socket->handler = handler;
}