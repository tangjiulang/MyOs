#ifndef __MYOS__NET__ICMP_H
#define __MYOS__NET__ICMP_H

#include "common/types.h"
#include "net/ipv4.h"
namespace myos {
  namespace net {
    struct InternetControlMessageProtocolMessage {
      common::uint8_t type;
      common::uint8_t code;
      common::uint16_t checkSum;

      common::uint32_t data;
    }__attribute__((packed)); // icmp message
    class InternetControlMessageProtocolHandler : public InternetProtocolV4Handler {
    public:
      InternetControlMessageProtocolHandler(InternetProtocolV4Provider* backend);
      ~InternetControlMessageProtocolHandler();

      virtual bool OnInternetProtocolReceive(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, common::uint8_t* internetProtocolPayload, common::uint32_t size);
      void RequestEchoReply(common::uint32_t ip_be);
    };
  }
}


#endif