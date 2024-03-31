#include "syscalls.h"
#include "common/types.h"

using namespace myos;
using namespace common;
using namespace hardwarecommunication;

void printf(const char*);
void printf(uint8_t);

SyscallHandler::SyscallHandler(uint8_t interruptNumber, InterruptManager* interruptManager) 
  : InterruptHandler(interruptNumber + interruptManager->HardwareInterruptOffset(), interruptManager) {}
SyscallHandler::~SyscallHandler() {}



uint32_t SyscallHandler::HandleInterrupt(uint32_t esp) {
  CPUState* cpu = (CPUState*)esp;
  switch (cpu->eax) {
    case 1: break;
    case 2: break;
    case 3: break;
    case 4:
      printf((char*)cpu->ebx);
      break;
    default:
      break;
  }
  
  return esp;
}