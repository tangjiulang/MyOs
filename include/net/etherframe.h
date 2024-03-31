#ifndef __MYOS__NET__ETHERFRAME_H
#define __MYOS__NET__ETHERFRAME_H

#include "drivers/amd_am79c973.h"
#include "common/types.h"
#include "memorymanagement.h"

namespace myos {
  namespace net {

    struct EtherFrameHeader {
      common::uint64_t dstMAC_BE : 48;
      common::uint64_t srcMAC_BE : 48;
      common::uint16_t etherTYPE_BE;
    }__attribute__((packed));

    typedef common::uint32_t EtherFrameFooter;


    class EtherFrameProvider;
    class EtherFrameHandler {
    protected:
      EtherFrameProvider* backend;
      common::uint16_t etherTYPE_BE;
    public:
      EtherFrameHandler(EtherFrameProvider* backend, common::uint16_t etherTYPE_BE);
      ~EtherFrameHandler();

      virtual bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);
      void Send(common::uint64_t dstMAC_BE, common::uint8_t* data, common::uint32_t size);
      common::uint32_t GetIPAddress();
    };
    
    class EtherFrameProvider : public myos::drivers::RawDataHandler {
    public:
      EtherFrameHandler* handlers[65535];
    public:
      EtherFrameProvider(drivers::amd_am79c973* backend);
      ~EtherFrameProvider(); 
      bool OnRawDataReceived(common::uint8_t* buffer, common::uint32_t size);
      void Send(common::uint64_t dstMAC_BE, common::uint16_t etherTYPE_BE, common::uint8_t* buffer, common::uint32_t size);
      void SetHandlers(EtherFrameHandler* handler, common::uint16_t etherTYPE_BE);

      common::uint32_t GetIPAddress();
      common::uint64_t GetMACAddress();
    };
  }
}

#endif