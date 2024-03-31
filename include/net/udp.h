#ifndef __MYOS__NET__UDP_H
#define __MYOS__NET__UDP_H

#include "common/types.h"
#include "net/icmp.h"
#include "net/ipv4.h"
namespace myos {
  namespace net {

    struct UserDatagramProtocolMessage {
      common::uint16_t srcPort;
      common::uint16_t dstPort;
      common::uint16_t length;
      common::uint16_t checkSum;
    }__attribute__((packed));

    class UserDatagramProtocolSocket;
    class UserDatagramProtocolProvider;

    class UserDatagramProtocolHandler {
    public:
      UserDatagramProtocolHandler();
      ~UserDatagramProtocolHandler();

      virtual void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
    };

    class UserDatagramProtocolSocket {
      friend class UserDatagramProtocolHandler;
    public:
      UserDatagramProtocolSocket(UserDatagramProtocolProvider* backend);
      ~UserDatagramProtocolSocket();

      virtual void HandleUserDatagramProtocolMessage(common::uint8_t* data, common::uint16_t size);
      virtual void Send(common::uint8_t* data, common::uint16_t size);
      virtual void DisConnected();
    protected:
      common::uint16_t remotePort;
      common::uint16_t remoteIP;
      common::uint16_t localPort;
      common::uint16_t localIP;

      bool listenning;
      UserDatagramProtocolProvider* backend;
      UserDatagramProtocolHandler* handler;
    };
    
    class UserDatagramProtocolProvider : public InternetProtocolV4Handler {
    public:
      UserDatagramProtocolProvider(InternetProtocolV4Provider* backend);
      ~UserDatagramProtocolProvider();

      virtual bool OnInternetProtocolReceive(common::uint32_t srcIP_BE, common::uint32_t dstIP_BE, common::uint8_t* internelProtocolPayload, common::uint32_t size);

      virtual UserDatagramProtocolSocket* Connect(common::uint32_t ip, common::uint16_t port);
      virtual UserDatagramProtocolSocket* Listen(common::uint16_t port);
      virtual void* DisConnect(UserDatagramProtocolSocket* socket);
      virtual void Send(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size);
      virtual void Bind(UserDatagramProtocolSocket* socket, UserDatagramProtocolHandler* handler);
    protected:
      UserDatagramProtocolSocket* socket[65535];
      int numSockets;
    };
  }
} 

#endif