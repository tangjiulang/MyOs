#include "filesystem/msdospart.h"

using namespace myos;
using namespace common;
using namespace filesystem;
using namespace drivers;


void printf(const char*);
void printf(uint8_t);

void MSDOSPartitionTable::ReadPartitions(AdvancedTechnologyAttachment *hd) {
  MasterBootRecord mbr;
  hd->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));

  printf("MBR: ");
  for (int i = 0x1BE; i <= 0x1FF; i++) {
    printf(((uint8_t*)&mbr)[i]);
    printf(" ");
  }
  printf("\n");
  if (mbr.magicnumber != 0xAA55) {
    printf("illegal MBR");
    return;
  }

  for (int i = 0; i < 4; i++) {
    if (mbr.primaryPartition[i].partition_id == 0) continue;
    printf(" Partition ");
    printf(i & 0xFF);
    if (mbr.primaryPartition[i].bootable == 0x80) 
      printf(" bootable");
    else 
      printf(" not bootable. Type ");

    printf(mbr.primaryPartition[i].partition_id);
    ReadBiosBlock(hd, mbr.primaryPartition[i].start_lba);
  }

}

