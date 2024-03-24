#ifndef __MYOS__HARDWARECOMMUNICATION__PCI_H
#define __MYOS__HARDWARECOMMUNICATION__PCI_H

#include "common/types.h"
#include "interrupts.h"
#include "port.h"
#include "drivers/driver.h"

#include "memorymanagement.h"
namespace myos 
{
  namespace hardwarecommunication 
  {

    class PeripheralComponentInterconnectController;

    enum BaseAddressRegisterType {
      MemoryMapping = 0,
      InputOutput = 1
    };

    class BaseAddressRegister {
      friend PeripheralComponentInterconnectController;
    private:
      bool perfetchable;
      myos::common::uint8_t* address;
      myos::common::uint32_t size;
      BaseAddressRegisterType type;
    };
    
    class PeripheralComponentInterconnectDeviceDescriptor {
    public:
      PeripheralComponentInterconnectDeviceDescriptor();
      ~PeripheralComponentInterconnectDeviceDescriptor();

      myos::common::uint32_t portBase;
      myos::common::uint32_t interrupt;

      myos::common::uint8_t bus;
      myos::common::uint8_t device;
      myos::common::uint8_t function;

      myos::common::uint16_t device_id;
      myos::common::uint16_t vendor_id;
      myos::common::uint8_t class_id;
      myos::common::uint8_t subclass_id;
      myos::common::uint8_t iterface_id;
      myos::common::uint8_t revision;

    };


    class PeripheralComponentInterconnectController {
    public:
    //     0         0000000      00000000    00000       000           00000000     
    // Enable Bit    Reserved       Bus       Device    Function    Rigister Offset
      PeripheralComponentInterconnectController();
      ~PeripheralComponentInterconnectController();
      myos::common::uint32_t Read(myos::common::uint8_t bus, 
                                  myos::common::uint8_t device,
                                  myos::common::uint8_t function,
                                  myos::common::uint8_t registeroffset);
      void Write(myos::common::uint8_t bus, 
                 myos::common::uint8_t device,
                 myos::common::uint8_t function,
                 myos::common::uint8_t registeroffset,
                 myos::common::uint32_t value);
      bool DiveceHasFunctions(myos::common::uint8_t bus,
                              myos::common::uint8_t device);
      void SelectDriver(myos::drivers::DriverManager* dirverManager, myos::hardwarecommunication::InterruptManager* interrrupts);

      PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(myos::common::uint8_t bus, 
                                                                          myos::common::uint8_t device,
                                                                          myos::common::uint8_t function);
      myos::drivers::Driver* GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, myos::hardwarecommunication::InterruptManager* interrupts);

      BaseAddressRegister GetBaseAddressRegister(myos::common::uint8_t bus, 
                                                 myos::common::uint8_t device,
                                                 myos::common::uint8_t function,
                                                 myos::common::uint8_t bar);
      
    private:
      Port32Bit dataPort;
      Port32Bit commandPort;
    };
  }
}

#endif