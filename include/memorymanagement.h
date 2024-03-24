#ifndef __MYOS__MEMORYMANAGEMENT_H
#define __MYOS__MEMORYMANAGEMENT_H

#include "common/types.h"

namespace myos {
  struct MemoryChunk {
    MemoryChunk* prev;
    MemoryChunk* next;
    bool allocated;
    common::size_t size;
  };

  class MemoryManager {
  public:
    MemoryManager(common::size_t start, common::size_t size);
    ~MemoryManager();

    void* malloc(common::size_t size);
    void free(void* ptr);

    static MemoryManager* activeMemoryManager;

  private:
    MemoryChunk* first;
  };
}

void* operator new(myos::common::size_t size);
void* operator new[](myos::common::size_t size);

void* operator new(myos::common::size_t size, void* ptr);
void* operator new[](myos::common::size_t size, void* ptr);

void operator delete(void* ptr);
void operator delete[](void* ptr);


#endif

