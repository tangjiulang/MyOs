#include "net/icmp.h"
#include "common/types.h"
#include "net/ipv4.h"

using namespace myos;
using namespace common;
using namespace net;

void printf(const char*);
void printf(uint16_t);

InternetControlMessageProtocolHandler::InternetControlMessageProtocolHandler(InternetProtocolV4Provider* backend) : InternetProtocolV4Handler(backend) {}
bool InternetControlMessageProtocolHandler::OnInternetProtocolReceive(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* internelProtocolPayload, uint32_t size) {
  if (size < sizeof(InternetControlMessageProtocolMessage)) return false;
  InternetControlMessageProtocolMessage* message = (InternetControlMessageProtocolMessage*)internelProtocolPayload;
  switch (message->type) {
  case 0:
    printf("ping response from ");
    printf(srcIP_BE & 0xFF);
    printf("."); printf((srcIP_BE >> 8) & 0xFF);
    printf("."); printf((srcIP_BE >> 16) & 0xFF);
    printf("."); printf((srcIP_BE >> 24) & 0xFF);
    printf("\n");
    break;
  case 8:
    message->type = 0;
    message->checkSum = InternetProtocolV4Provider::CheckSum((uint16_t*)message, sizeof(InternetControlMessageProtocolMessage));
    return true;
  }

  return false;
}


void InternetControlMessageProtocolHandler::RequestEchoReply(common::uint32_t ip_be) {
   InternetControlMessageProtocolMessage icmp;
   icmp.type = 8;
   icmp.code = 0;
   icmp.data = 0x3713;
   icmp.checkSum = InternetProtocolV4Provider::CheckSum((uint16_t*)&icmp, sizeof(InternetControlMessageProtocolMessage));

   InternetProtocolV4Handler::Send(ip_be, (uint8_t*)&icmp, sizeof(InternetControlMessageProtocolMessage));
}