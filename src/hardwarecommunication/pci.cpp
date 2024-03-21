#include "hardwarecommunication/pci.h"
#include "common/types.h"
#include "drivers/driver.h"


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
void printfHex(uint8_t);
void printfHex(uint16_t);

void PeripheralComponentInterconnectController::SelectDriver(DriverManager* driverManager) {
  for (uint16_t bus = 0; bus < 256; bus++) {
    for (uint8_t device = 0; device < 32; device++) {
      int numFunctions = this->DiveceHasFunctions((uint8_t)bus, device) ? 8 : 1;
      for (uint8_t function = 0; function < numFunctions; function++) {
        PeripheralComponentInterconnectDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);
        if (dev.vendor_id == 0 || dev.vendor_id == 0xFFFF) break;

        printf("PCI BUS ");
        printfHex((uint8_t)(bus & 0xFF));
        printf(", DEVICE ");
        printfHex(device);
        printf(", FUNCTION ");
        printfHex(function);
        printf(" = VENDOR ");
        printfHex(dev.vendor_id);
        printf(", DEVICE ");
        printfHex(dev.device_id);
        printf("\n");
      }
    }
  }
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