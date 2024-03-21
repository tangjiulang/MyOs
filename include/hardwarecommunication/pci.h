#ifndef __MYOS__HARDWARECOMMUNICATION__PCI_H
#define __MYOS__HARDWARECOMMUNICATION__PCI_H

#include "common/types.h"
#include "interrupts.h"
#include "port.h"
#include "drivers/driver.h"

namespace myos 
{
  namespace hardwarecommunication 
  {
    class PeripheralComponentInterconnectController;
    
    class PeripheralComponentInterconnectDeviceDescriptor {
    public:
      PeripheralComponentInterconnectDeviceDescriptor();
      ~PeripheralComponentInterconnectDeviceDescriptor();

    private:
      friend PeripheralComponentInterconnectController;
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
      void SelectDriver(myos::drivers::DriverManager* dirverManager);

      PeripheralComponentInterconnectDeviceDescriptor GetDeviceDescriptor(myos::common::uint8_t bus, 
                                                                          myos::common::uint8_t device,
                                                                          myos::common::uint8_t function);


    private:
      Port32Bit dataPort;
      Port32Bit commandPort;
    };
  }
}

#endif