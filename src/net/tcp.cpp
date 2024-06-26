#include "net/tcp.h"
#include "common/types.h"


using namespace myos;
using namespace common;
using namespace net;

TransmissionControlProtocolHandler::TransmissionControlProtocolHandler() {}
TransmissionControlProtocolHandler::~TransmissionControlProtocolHandler() {}

uint32_t bigEndian32(uint32_t x) {
    return (x & 0xff000000) >> 24 |
        (x & 0x00ff0000) >> 8 |
        (x & 0x0000ff00) << 8 | 
        (x & 0x000000ff) << 24;
}

bool TransmissionControlProtocolHandler::HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size) {
  return true;
}


TransmissionControlProtocolSocket::TransmissionControlProtocolSocket(TransmissionControlProtocolProvider* backend) {
  this->backend = backend;
  handler = 0;
  state = CLOSED;
}

TransmissionControlProtocolSocket::~TransmissionControlProtocolSocket() {}

bool TransmissionControlProtocolSocket::HandleTransmissionControlProtocolMessage(uint8_t* data, uint16_t size) {
  if (handler != 0) {
    handler->HandleTransmissionControlProtocolMessage(this, data, size);
  }
  return false;
}

void TransmissionControlProtocolSocket::Send(uint8_t* data, uint16_t size) {
  backend->Send(this, data, size);
}

void TransmissionControlProtocolSocket::DisConnect() {
  backend->DisConnect(this);
}

TransmissionControlProtocolProvider::TransmissionControlProtocolProvider(InternetProtocolV4Provider* backend)
  : InternetProtocolV4Handler(backend, 0x06) {
  for (int i = 0; i < 65535; i++) {
    sockets[i] = 0;
  }
  numSockets = 0;
  freePort = 1024;
}

TransmissionControlProtocolProvider::~TransmissionControlProtocolProvider() {}

bool TransmissionControlProtocolProvider::OnInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* internetProtocolPayload, uint32_t size) {
  if (size < 20) return false;

  TransmissionControlProtocolHeader* message = (TransmissionControlProtocolHeader*)internetProtocolPayload;
  uint16_t localPort = message->dstPort;
  uint16_t remotePort = message->srcPort;

  TransmissionControlProtocolSocket* socket = 0;
  for (int i = 0; i < numSockets && socket == 0; i++) {
    if (sockets[i]->localPort == message->dstPort && sockets[i]->localIP == dstIP_BE) {
      if (sockets[i]->state == LISTEN && (((message->flags) & (SYN | ACK)) == SYN)) socket = sockets[i];
      else if (sockets[i]->remotePort == message->srcPort && sockets[i]->remoteIP == srcIP_BE) socket = sockets[i];   
    } 
  }
  if (socket == 0) return false;
  bool reset = false;

  if (message->flags & RST) socket->state = CLOSED;

  if (socket->state != CLOSED) {
    switch ((message->flags) & (SYN | ACK | FIN)) {
    case SYN:
      if (socket->state == LISTEN) {
        socket->state = SYN_RECEIVED;
        socket->remotePort = message->srcPort;
        socket->remoteIP = srcIP_BE;
        socket->acknowledgementNumber = bigEndian32(message->sequenceNumber) + 1;
        socket->sequenceNumber = 0xBEEFCAFE;
        Send(socket, 0, 0, SYN | ACK);
        socket->sequenceNumber++;
      } else {
        reset = true;
      }
      break;
    case SYN | ACK:
      if (socket->state == SYN_SENT) {
        socket->state = ESTABLISHED;
        socket->acknowledgementNumber = bigEndian32(message->sequenceNumber) + 1;
        socket->sequenceNumber++;
        Send(socket, 0, 0, ACK);
      } else {
        reset = true;
      }
      break;
    case SYN | FIN:
    case SYN | FIN | ACK:
      reset = true;
      break;
    case FIN:
    case FIN | ACK:
      if (socket->state == ESTABLISHED) {
        socket->state = CLOSE_WAIT;
        socket->acknowledgementNumber++;
        Send(socket, 0, 0, ACK);
        Send(socket, 0, 0, FIN | ACK);
      } else if (socket->state == CLOSE_WAIT) {
        socket->state = CLOSED;
      } else if (socket->state == FIN_WAIT1 || socket->state == FIN_WAIT2) {
        socket->state = CLOSED;
        socket->acknowledgementNumber++;
        Send(socket, 0, 0, ACK);
      } else {
        reset = true;
      }
      break;
    case ACK:
      if (socket->state == SYN_RECEIVED) {
        socket->state = ESTABLISHED;
        return false;
      } else if (socket->state == FIN_WAIT1) {
        socket->state = FIN_WAIT2;
        return false;
      } else if (socket->state == CLOSE_WAIT) {
        socket->state = CLOSED;
        return false;
      }
      if (message->flags == ACK) break;
    default:
      if (socket->acknowledgementNumber == bigEndian32(message->sequenceNumber)) {
        reset = !(socket->HandleTransmissionControlProtocolMessage(internetProtocolPayload + message->headerSize32 * 4, size - message->headerSize32 * 4));
        if (!reset) {
          socket->acknowledgementNumber += size - message->headerSize32 * 4;
          Send(socket, 0, 0, ACK);
        }
      } else {
        reset = true;
      }
    }
  }
  if (reset) return true;

  if (socket->state == CLOSED) {
    for (int i = 0; i < numSockets && socket == 0; i++) {
      if (sockets[i] == socket) {
        sockets[i] = sockets[--numSockets];
        MemoryManager::activeMemoryManager->free(socket);
        break;
      }
    }
  }
  return false;
}

TransmissionControlProtocolSocket* TransmissionControlProtocolProvider::Connect(uint32_t ip, uint16_t port) {
  TransmissionControlProtocolSocket* socket = (TransmissionControlProtocolSocket*) MemoryManager::activeMemoryManager->malloc(sizeof(TransmissionControlProtocolSocket));
  if (socket == 0) return socket;
  new(socket)TransmissionControlProtocolSocket(this);

  socket->remotePort = port;
  socket->remoteIP = ip;
  socket->localPort = freePort++;
  socket->localIP = backend->GetIPAddress();

  socket->remotePort = ((socket->remotePort & 0xFF00) >> 8) | ((socket->remotePort & 0x00FF) << 8);
  socket->localPort = ((socket->localPort & 0xFF00) >> 8) | ((socket->localPort & 0x00FF) << 8);
  socket->state = SYN_SENT;
  sockets[numSockets++] = socket;
  socket->sequenceNumber = 0xBEEFCAFE;
  Send(socket, 0, 0, SYN);
  return socket;
}
TransmissionControlProtocolSocket* TransmissionControlProtocolProvider::Listen(uint16_t port) {
  TransmissionControlProtocolSocket* socket = (TransmissionControlProtocolSocket*)MemoryManager::activeMemoryManager->malloc(sizeof(TransmissionControlProtocolSocket));
  if (socket == 0) return socket;
  new(socket)TransmissionControlProtocolSocket(this);

  socket->state = LISTEN;
  socket->localPort = port;
  socket->localIP = backend->GetIPAddress();

  socket->localPort = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);
  sockets[numSockets++] = socket;
  return socket;
}

void TransmissionControlProtocolProvider::DisConnect(TransmissionControlProtocolSocket* socket) {
  socket->state = FIN_WAIT1;
  Send(socket, 0, 0, FIN | ACK);
  socket->sequenceNumber++;
}

void TransmissionControlProtocolProvider::Send(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t size, uint16_t flags) {
  uint16_t totalLength = size + sizeof(TransmissionControlProtocolHeader);
  uint16_t lengthIncIPHdr = totalLength + sizeof(TransmissionControlProtocolPesudoHeader);
  uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager->malloc(lengthIncIPHdr);
    TransmissionControlProtocolPesudoHeader* phdr = (TransmissionControlProtocolPesudoHeader*)buffer;
  TransmissionControlProtocolHeader* msg = (TransmissionControlProtocolHeader*)(buffer + sizeof(TransmissionControlProtocolPesudoHeader));
  uint8_t* buffer2 = buffer + lengthIncIPHdr - size;

  msg->headerSize32 = sizeof(TransmissionControlProtocolHeader) / 4;
  msg->srcPort = socket->localPort;
  msg->dstPort = socket->remotePort;
  msg->acknowledgementNumber = bigEndian32(socket->acknowledgementNumber);
  msg->sequenceNumber = bigEndian32(socket->sequenceNumber);
  msg->reserved = 0;
  msg->flags = flags;
  msg->windowSize = 0xFFFF;
  msg->urgentPtr = 0;

  msg->options = ((flags & SYN) != 0) ? 0xb4050402 : 0;

  socket->sequenceNumber += size;

  for (int i = 0; i < size; i++) {
    buffer2[i] = data[i];
  }

  phdr->srcIP = socket->localIP;
  phdr->dstIP = socket->remoteIP;
  phdr->protocol = 0x0600;
  phdr->totalLength = ((totalLength & 0x00ff) << 8) | ((totalLength & 0xff00) >> 8);
  msg->checkSum = 0;
  msg->checkSum = InternetProtocolV4Provider::CheckSum((uint16_t*)buffer, lengthIncIPHdr);
  InternetProtocolV4Handler::Send(socket->remoteIP, (uint8_t*)msg, totalLength);
  MemoryManager::activeMemoryManager->free(buffer);
}

void TransmissionControlProtocolProvider::Bind(TransmissionControlProtocolSocket* socket, TransmissionControlProtocolHandler* handler) {
  socket->handler = handler;
}