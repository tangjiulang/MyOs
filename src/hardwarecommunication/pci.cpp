#include "hardwarecommunication/pci.h"
#include "drivers/amd_am79c973.h"
#include "memorymanagement.h"


using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;
using namespace myos::drivers;


PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor() {}
PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor() {}

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
  : dataPort(0xCFC), commandPort(0xCF8) {}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController() {}

uint32_t PeripheralComponentInterconnectController::Read(uint8_t bus, 
                                                         uint8_t device,
                                                         uint8_t function,
                                                         uint8_t registeroffset) {
  uint32_t id = (1 << 31) |
                ((bus & 0xff) << 16) |
                ((device & 0x1f) << 11) |
                ((function & 0x07) << 8) |
                (registeroffset & 0xfc);
  commandPort.Write(id);
  uint32_t result = dataPort.Read();
  return (result >> (8 * (registeroffset % 4)));
}
                        

void PeripheralComponentInterconnectController::Write(uint8_t bus, 
                                                      uint8_t device,
                                                      uint8_t function,
                                                      uint8_t registeroffset,
                                                      uint32_t value) {
  uint32_t id = (1 << 31) |
                ((bus & 0xff) << 16) |
                ((device & 0x1f) << 11) |
                ((function & 0x07) << 8) |
                (registeroffset & 0xfc);
  commandPort.Write(id);
  dataPort.Write(value);
}

bool PeripheralComponentInterconnectController::DiveceHasFunctions(uint8_t bus, uint8_t device) {
  return (Read(bus, device, 0, 0x0E) & (1 << 7));
}

void printf(const char*);
void printf(uint8_t);
void printf(uint16_t);

void PeripheralComponentInterconnectController::SelectDriver(DriverManager* driverManager, InterruptManager* interrupts) {
  for (uint16_t bus = 0; bus < 256; bus++) {
    for (uint8_t device = 0; device < 32; device++) {
      int numFunctions = this->DiveceHasFunctions((uint8_t)bus, device) ? 8 : 1;
      for (uint8_t function = 0; function < numFunctions; function++) {
        PeripheralComponentInterconnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);
        if (dev.vendor_id == 0 || dev.vendor_id == 0xFFFF) continue;

        printf("PCI BUS ");
        printf((uint8_t)(bus & 0xFF));
        printf(", DEVICE ");
        printf(device);
        printf(", FUNCTION ");
        printf(function);
        printf(" = VENDOR ");
        printf(dev.vendor_id);
        printf(", DEVICE ");
        printf(dev.device_id);
        printf("\n");

        for (uint8_t barNum = 0; barNum < 6; barNum++) {
          BaseAddressRegister bar = GetBaseAddressRegister(bus, device, function, barNum);
          if (bar.address && (bar.type == InputOutput)) {
            dev.portBase = (uint32_t)bar.address;
          }
        }
        Driver* driver = GetDriver(dev, interrupts);  
        if (driver != 0)
          driverManager->AddDriver(driver);

      }
    }
  }
}

Driver* PeripheralComponentInterconnectController::GetDriver(PeripheralComponentInterconnectDeviceDescriptor dev, InterruptManager* interrupts) {
  Driver* driver = 0;
  switch (dev.vendor_id) {
  case 0x1022: // AMD
    switch (dev.device_id) {
      case 0x2000:
        printf("AMD am79c973\n");
        driver = (amd_am79c973*)MemoryManager::activeMemoryManager->malloc(sizeof(amd_am79c973));
        if (driver != 0) {
          new (driver)amd_am79c973(&dev, interrupts);
        }
        return driver;
        break;
    }
    break;
  case 0x8086: // intel
    break;
  }

  switch (dev.class_id) {
  case 0x03:
    switch (dev.subclass_id) {
    case 0x00: // VGA
      printf("VGA ");
      break;
    }
    break;
  }
  return 0;
}

PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint8_t bus, uint8_t device, uint8_t function) {
  PeripheralComponentInterconnectDeviceDescriptor result;

  result.bus = bus;
  result.device = device;
  result.function = function;

  result.vendor_id = Read(bus, device, function, 0);
  result.device_id = Read(bus, device, function, 0x02);
  result.revision = Read(bus, device, function, 0x08);
  result.iterface_id = Read(bus, device, function, 0x09);
  result.subclass_id = Read(bus, device, function, 0x0A);
  result.class_id = Read(bus, device, function, 0x0B);
  result.interrupt = Read(bus, device, function, 0x3C);

  return result;
}

BaseAddressRegister PeripheralComponentInterconnectController::GetBaseAddressRegister(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar) {
  BaseAddressRegister result;

  uint32_t headertype = Read(bus, device, function, 0x0E) & 0x7E;
  int maxBARs = 6 - 4 * headertype;
  if (bar >= maxBARs) return result;

  uint32_t bar_value = Read(bus, device, function, 0x10 + 4 * bar);
  result.type = (bar_value & 1) ? InputOutput : MemoryMapping;

  if (result.type == MemoryMapping) {
    switch ((bar_value >> 1) & 0x3) {
    case 0: // 32
    case 1: // 20
    case 2: // 64
    break;
    } 
  } else {
      result.address = (uint8_t*)(bar_value & ~0x3);
      result.perfetchable = false;
  }
  return result;
}