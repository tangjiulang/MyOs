#ifndef __GDT_H
#define __GDT_H

#include "common/types.h"

namespace myos 
{
  class GlobalDescriptorTable {
  public:
    class SegmentDescriptor {
      public:
        SegmentDescriptor(myos::common::uint32_t base, myos::common::uint32_t limit, myos::common::uint8_t type);
        myos::common::uint32_t Base();
        myos::common::uint32_t Limit();
      private:
        myos::common::uint16_t limit_lo;
        myos::common::uint16_t base_lo;
        myos::common::uint8_t base_hi;
        myos::common::uint8_t type; 
        myos::common::uint8_t flags_limit_hi;
        myos::common::uint8_t base_vhi;
    } __attribute__((packed));
  // 00000000 0  0  0  0    0000    0 00  0 0000 00000000
  //  Base    G D/B L AVL Seg.limit p DPL S TYPE   BASE
  // 0 ---- 15     16 --- 31
  //   limit      base address
    SegmentDescriptor nullSegmentDescriptor;
    SegmentDescriptor unusedSegmentDescriptor;
    SegmentDescriptor codeSegmentDescriptor;
    SegmentDescriptor dataSegmentDescriptor;
    
  public:
    GlobalDescriptorTable();
    ~GlobalDescriptorTable();

    myos::common::uint16_t CodeSegmentSelector();
    myos::common::uint16_t DataSegmentSelector();
  };
}

#endif