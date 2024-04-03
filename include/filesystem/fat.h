#ifndef __MYOS__FILESYSTEM__FAT_H
#define __MYOS__FILESYSTEM__FAT_H

#include "common/types.h"
#include "drivers/ata.h"

namespace myos {
  namespace filesystem {
    struct BiosParameterBlock32 {
      common::uint8_t jmp[3];
      common::uint8_t softName[8];
      common::uint16_t bytePerSector;
      common::uint8_t sectorPerCluster;
      common::uint16_t reservedSectors;
      common::uint8_t fatCopies;
      common::uint16_t rootDirEntries;
      common::uint16_t totalSectors;
      common::uint8_t mediaType;
      common::uint16_t fatSectorCount;
      common::uint16_t sectorsPerTrack;
      common::uint16_t headCount;
      common::uint32_t hiddenSectors;
      common::uint32_t totalSectorCount;

      common::uint32_t tableSize;
      common::uint16_t extFlags;
      common::uint16_t fatVersion;
      common::uint16_t rootCluster;
      common::uint16_t fatInfo;
      common::uint16_t backupSector;
      common::uint8_t reserved0[12];
      common::uint8_t driveNumber;
      common::uint8_t reserved;
      common::uint8_t bootSignature;
      common::uint32_t volumeId;
      common::uint8_t volumeLabel[11];
      common::uint8_t fatTypeLabel[8];
    }__attribute__((packed));


    struct DirectoryEntryFat32 {
      common::uint8_t name[8];
      common::uint8_t ext[3];
      common::uint8_t attributes;
      common::uint8_t reserved;
      common::uint8_t cTimeTenth;
      common::uint16_t cTime;
      common::uint16_t cData;
      common::uint16_t aTime;

      common::uint16_t firstClusterHi;

      common::uint16_t wTime;
      common::uint16_t wData;
      common::uint16_t firstClusterLow;
      common::uint32_t size; 
    }__attribute__((packed));

    void ReadBiosBlock(myos::drivers::AdvancedTechnologyAttachment* hd, common::uint32_t partitionOffset);
  }
}

#endif