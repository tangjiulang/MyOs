#include "net/etherframe.h"
#include "common/types.h"
#include "drivers/amd_am79c973.h"
#include "memorymanagement.h"


using namespace myos;
using namespace common;
using namespace drivers;
using namespace net;

EtherFrameHandler::EtherFrameHandler(EtherFrameProvider* backend, uint16_t etherTYPE_BE) : backend(backend){
  this->etherTYPE_BE = ((etherTYPE_BE & 0x00FF) << 8)
                     | ((etherTYPE_BE & 0xFF00) >> 8);
  backend->handlers[etherTYPE_BE] = this;
}
EtherFrameHandler::~EtherFrameHandler() {
  if (backend->handlers[etherTYPE_BE] == this)
    backend->handlers[etherTYPE_BE] = 0;
}

bool EtherFrameHandler::OnEtherFrameReceived(uint8_t* etherframePayload, uint32_t size) {
  return false;
}

void EtherFrameHandler::Send(uint64_t dstMAC_BE, uint8_t* data, uint32_t size) {
  backend->Send(dstMAC_BE, etherTYPE_BE, data, size);
}

uint32_t EtherFrameHandler::GetIPAddress() {
    return backend->GetIPAddress();
}

EtherFrameProvider::EtherFrameProvider(amd_am79c973* backend) : RawDataHandler(backend) {
  for (int i = 0; i < 65535; i++) handlers[i] = 0;
}

EtherFrameProvider::~EtherFrameProvider() {}

bool EtherFrameProvider::OnRawDataReceived(uint8_t* buffer, uint32_t size) {
  if (size < sizeof(EtherFrameHeader))
    return false;
  EtherFrameHeader* frame = (EtherFrameHeader*)buffer;
  bool sendBack = false;

  if (frame->dstMAC_BE == 0xFFFFFFFFFFFF ||
      frame->dstMAC_BE == backend->GetMACAddress()) {

    if (handlers[frame->etherTYPE_BE] != 0)
      sendBack = handlers[frame->etherTYPE_BE]->OnEtherFrameReceived(buffer + sizeof(EtherFrameHeader), size - sizeof(EtherFrameHeader));
  }

  if (sendBack) {
    frame->dstMAC_BE = frame->srcMAC_BE;
    frame->srcMAC_BE = backend->GetMACAddress();
  }

  return sendBack;
}
void EtherFrameProvider::Send(uint64_t dstMAC_BE, uint16_t etherTYPE_BE, uint8_t* buffer, uint32_t size) {
  uint8_t* buffer2 = (uint8_t*)MemoryManager::activeMemoryManager->malloc(sizeof(EtherFrameHeader) + size);
  EtherFrameHeader* frame = (EtherFrameHeader*)buffer2;
  frame->dstMAC_BE = dstMAC_BE;
  frame->srcMAC_BE = backend->GetMACAddress();
  frame->etherTYPE_BE = etherTYPE_BE;

  uint8_t* src = buffer;
  uint8_t* dst = buffer2 + sizeof(EtherFrameHeader);

  for (uint32_t i = 0; i < size; i++) dst[i] = src[i];

  backend->Send(buffer2, size + sizeof(EtherFrameHeader));
  MemoryManager::activeMemoryManager->free(buffer2);
}


uint32_t EtherFrameProvider::GetIPAddress() {
  return backend->GetIPAddress();
}

uint64_t EtherFrameProvider::GetMACAddress() {
  return backend->GetMACAddress();
}