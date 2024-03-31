#ifndef __MYOS__DRIVERS__ATA_H
#define __MYOS__DRIVERS__ATA_H

#include "common/types.h"
#include "hardwarecommunication/port.h"
namespace myos {
  namespace drivers {
    class AdvancedTechnologyAttachment {
    protected:
      hardwarecommunication::Port16Bit dataPort;
      hardwarecommunication::Port8Bit errorPort;
      hardwarecommunication::Port8Bit sectorPort;
      hardwarecommunication::Port8Bit lbaLowPort;
      hardwarecommunication::Port8Bit lbaMidPort;
      hardwarecommunication::Port8Bit lbaHighPort;
      hardwarecommunication::Port8Bit devicePort;
      hardwarecommunication::Port8Bit commandPort;
      hardwarecommunication::Port8Bit controlPort;

      bool master;
      common::uint16_t bytesPeSector;
    public:
      AdvancedTechnologyAttachment(common::uint16_t portBase, bool master);
      ~AdvancedTechnologyAttachment();

      void Identify();
      void Read28(common::uint32_t sector, common::uint8_t* data, int count);
      void Write28(common::uint32_t sector, common::uint8_t* data, int count);
      void Flush();
    };
  }
}


#endif