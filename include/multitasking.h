#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include "common/types.h"
#include "gdt.h"

namespace myos {
  struct CPUState {
    common::uint32_t eax, ebx, ecx, edx, esi, edi, ebp;

    common::uint32_t error, eip, cs, eflags, esp, ss;
  } __attribute__((packed));
  class TaskManager;
  class Task {
    friend class TaskManager;
  public:
    Task(GlobalDescriptorTable* gdt, void entrypoint());
    ~Task();
  private:
    common::uint8_t stack[4096];
    CPUState* cpustate;
  };

  class TaskManager {
  public:
    TaskManager();
    ~TaskManager();
    bool AddTask(Task*);
    CPUState* Schedule(CPUState* cpustate);
  private:
    Task* tasks[256];
    common::int32_t numTasks;
    common::int32_t currentTask;
  };
}


#endif