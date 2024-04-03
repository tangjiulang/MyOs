#include "filesystem/fat.h"

using namespace myos;
using namespace common;
using namespace drivers;

void printf(const char*);
void printf(uint8_t);
namespace myos {
  namespace filesystem {
    void ReadBiosBlock(AdvancedTechnologyAttachment* hd, uint32_t partitionOffset) {
      BiosParameterBlock32 bpb;
      hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

      printf("sectors per cluster: ");
      printf(bpb.sectorPerCluster);
      printf("\n");

      uint32_t fatStart = partitionOffset + bpb.reservedSectors;
      uint32_t fatSize = bpb.tableSize;

      uint32_t dataStart = fatStart + fatSize * bpb.fatCopies;

      uint32_t rootStart = dataStart + bpb.sectorPerCluster * (bpb.rootCluster - 2);

      DirectoryEntryFat32 dirent[16];
      hd->Read28(rootStart, (uint8_t*)&dirent[0], 16 * sizeof(DirectoryEntryFat32));

      for (int i = 0; i < 16; i++) {
        if (dirent[i].name[0] == 0x00) break;
        if ((dirent[i].attributes & 0x0F) == 0x0F) continue;

        if ((dirent[i].attributes & 0x10) == 0x10) continue; // dictory

        uint32_t firstFileCluster = ((uint32_t)dirent[i].firstClusterHi << 16) | (uint32_t)dirent[i].firstClusterLow;
        
        int32_t SIZE = dirent[i].size;
        int32_t nextFileCluster = firstFileCluster;

        while (SIZE > 0) {
          int sectorOffset = 0;
          uint8_t buffer[513];
          uint8_t fatbuffer[513];
          uint32_t fileSector = dataStart + bpb.sectorPerCluster * (nextFileCluster - 2);
          for (; SIZE > 0; SIZE -= 512) {
            hd->Read28(fileSector, buffer, 512);

            buffer[SIZE > 512 ? 512 : SIZE] = '\0';
            printf((char*)buffer);
            if (++sectorOffset > bpb.sectorPerCluster) break;
          }
          uint32_t fatSectorForCurrenCluster = nextFileCluster / (512 / sizeof(uint32_t));
          hd->Read28(fatStart + fatSectorForCurrenCluster, fatbuffer, 512);
          uint32_t fatOffsetInSectorForCurrentCluster = nextFileCluster % (512 / sizeof(uint32_t));
          nextFileCluster = ((uint32_t*)&fatbuffer)[fatOffsetInSectorForCurrentCluster] & 0x0FFFFFFF;
        }
      }

    }
  }
}
