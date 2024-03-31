#ifndef __MYOS__NET__IPV4_H
#define __MYOS__NET__IPV4_H

#include "common/types.h"
#include "net/arp.h"
#include "net/etherframe.h"

namespace myos {
  namespace net {
    struct InternetProtocolV4Message {
      common::uint8_t version : 4;
      common::uint8_t headerLength : 4;
      common::uint8_t tos;
      common::uint16_t totalLength;

      common::uint16_t ident;
      common::uint16_t flagsAndOffset;

      common::uint8_t timeToLive;
      common::uint8_t protocol;
      common::uint16_t checksum;

      common::uint32_t srcIP;
      common::uint32_t dstIP;
   
    } __attribute__((packed));

    class InternetProtocolV4Provider;
    class InternetProtocolV4Handler {
    public:
      InternetProtocolV4Handler(InternetProtocolV4Provider* backend, common::uint8_t protocol);
      ~InternetProtocolV4Handler();

      virtual bool OnInternetProtocolReceive(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, common::uint8_t* internelProtocolPayload, common::uint32_t size);

      void Send(common::uint32_t dstIP_BE, common::uint8_t* internelProtocolPayload, common::uint32_t size);
    protected:
      InternetProtocolV4Provider* backend;
      common::uint8_t protocol;
    };

    class InternetProtocolV4Provider : public EtherFrameHandler {
      friend class InternetProtocolV4Handler;
    public:
      InternetProtocolV4Provider(EtherFrameProvider* backend, AddressResolutionProtocol* arp, common::uint32_t gatewayIP, common::uint32_t subnetMask);
      ~InternetProtocolV4Provider();
      bool OnEtherFrameReceived(common::uint8_t* etherframePayload, common::uint32_t size);
      void Send(common::uint32_t dstIP_BE, common::uint8_t protocol, common::uint8_t* data, common::uint32_t size);
      static common::uint16_t CheckSum(common::uint16_t* data, common::uint32_t size);

    protected:
      InternetProtocolV4Handler* handlers[255];
      AddressResolutionProtocol* arp;
      common::uint32_t gatewayIP;
      common::uint32_t subnetMask;
    };
  }
}

#endif