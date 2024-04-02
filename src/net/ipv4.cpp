#include "net/ipv4.h"


using namespace myos;
using namespace common;
using namespace net;

InternetProtocolV4Handler::InternetProtocolV4Handler(InternetProtocolV4Provider* backend, uint8_t protocol) {
  this->backend = backend;
  this->protocol = protocol;
  backend->handlers[protocol] = this;
}

InternetProtocolV4Handler::~InternetProtocolV4Handler() {
  if (backend->handlers[protocol] == this) {
    backend->handlers[protocol] = 0;
  }
}

bool InternetProtocolV4Handler::OnInternetProtocolReceive(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* internetProtocolPayload, uint32_t size) {
  return false;
}
void InternetProtocolV4Handler::Send(uint32_t dstIP_BE, uint8_t* internetProtocolPayload, uint32_t size) {
    backend->Send(dstIP_BE, protocol, internetProtocolPayload, size);
}

InternetProtocolV4Provider::InternetProtocolV4Provider(EtherFrameProvider* backend, AddressResolutionProtocol* arp, common::uint32_t gatewayIP, common::uint32_t subnetMask) 
  : EtherFrameHandler(backend, 0x800) {
  for (int i = 0; i < 255; i++) 
    handlers[i] = 0;
  this->arp = arp;
  this->gatewayIP = gatewayIP;
  this->subnetMask = subnetMask;
}

bool InternetProtocolV4Provider::OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size) {
  if (size < sizeof(InternetProtocolV4Message))
    return false;
  InternetProtocolV4Message* ip_message = (InternetProtocolV4Message*)etherframePayload;
  bool sendBack = false;

  if (ip_message->dstIP == backend->GetIPAddress()) {
    int length = ip_message->totalLength;
    if (length > size) {
      length = size;
    }

    if (handlers[ip_message->protocol] != 0) {
      sendBack = handlers[ip_message->protocol]->OnInternetProtocolReceive(ip_message->srcIP, ip_message->dstIP, etherframePayload + 4 * ip_message->headerLength, length - 4 * ip_message->headerLength);
    }
  }
  
  if (sendBack) {
    uint32_t tmp = ip_message->dstIP;
    ip_message->dstIP = ip_message->srcIP;
    ip_message->srcIP = tmp;

    ip_message->timeToLive = 0x40;
    ip_message->checksum = CheckSum((uint16_t*) ip_message, 4 * ip_message->headerLength);
  }
  return sendBack;
}
void printf(const char*);

void InternetProtocolV4Provider::Send(uint32_t dstIP_BE, uint8_t protocol, uint8_t* data, uint32_t size) {
  uint8_t* buffer = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(InternetProtocolV4Message) + size);
  InternetProtocolV4Message* message = (InternetProtocolV4Message*)buffer;

  message->version = 4;
  message->headerLength = sizeof(InternetProtocolV4Message) / 4;
  message->tos = 0;
  message->totalLength = size + sizeof(InternetProtocolV4Message);
  message->totalLength = ((message->totalLength & 0xFF00) >> 8) | ((message->totalLength & 0x00FF) << 8);

  message->ident = 0x0100;
  message->flagsAndOffset = 0x0040;
  message->timeToLive = 0x40;
  message->protocol = protocol;

  message->dstIP = dstIP_BE;
  message->srcIP = backend->GetIPAddress();
  
  message->checksum = CheckSum((uint16_t*)message, sizeof(InternetProtocolV4Message));

  uint8_t* databuffer = buffer + sizeof(InternetProtocolV4Message);

  for (int i = 0; i < size; i++)  databuffer[i] = data[i];

  uint32_t route = dstIP_BE;
  if ((dstIP_BE & subnetMask) != (message->srcIP & subnetMask)) {
    route = gatewayIP;
  }
  backend->Send(arp->Resolve(route), this->etherTYPE_BE, buffer, sizeof(InternetProtocolV4Message) + size);
  MemoryManager::activeMemoryManager->free(buffer);
}

uint16_t InternetProtocolV4Provider::CheckSum(uint16_t* data, uint32_t size) {
  uint32_t tmp = 0;
  for (int i = 0; i < size / 2; i++) {
    tmp += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);
  }

  if (size % 2) tmp += ((uint16_t)((char*)data)[size - 1]) << 8;

  while (tmp & 0xFFFF0000) tmp = (tmp & 0xFFFF) + (tmp >> 16);

  return ((~tmp & 0xff00) >> 8) | ((~tmp & 0x00ff) << 8);
}