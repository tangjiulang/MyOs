#include "common/types.h"
#include "gdt.h"
#include "drivers/driver.h"
#include "hardwarecommunication/interrupts.h"
#include "hardwarecommunication/pci.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "drivers/amd_am79c973.h"
#include "drivers/ata.h"
#include "gui/desktop.h"
#include "gui/window.h"
#include "syscalls.h"
#include "multitasking.h"
#include "memorymanagement.h"
#include "net/etherframe.h"
#include "net/arp.h"
#include "net/ipv4.h"
#include "net/icmp.h"
#include "net/udp.h"
 

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
using namespace myos::net;

void printf(const char* str) {
    static uint16_t* VideoMemory = (uint16_t*) 0xb8000;

    static uint8_t x = 0, y = 0;
    for (int i = 0; str[i]; i++) {
        switch(str[i]) {
        case '\n':
            y++;
            x = 0;
            break;
        default:
            VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | str[i];
            x++;
            break;
        }

        
        if (x >= 80) {
            x = 0;
            y++;
        }

        if (y >= 25) {
            for (y = 0; y < 25; y++) {
                for (x = 0; x < 80; x++) {
                    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | ' ';
                }
            }
            x = 0, y = 0;
        }
    }
}

void printf(uint8_t key) {
    char* foo = (char*)"00";
    const char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0x0f];
    foo[1] = hex[key & 0x0f];
    printf((const char*)foo);
}

void printf(uint16_t key) {
    printf((uint8_t)((key >> 8) & 0xFF));
    printf((uint8_t)(key & 0xFF));
}

void printf(uint32_t key) {
    printf((uint16_t)((key >> 16) & 0xFFFF));
    printf((uint16_t)(key & 0xFFFF));
}

class PrintKeyboardEventHandler : public KeyboardEventHandler {
public:
    void OnKeyDown(char c) {
        char* foo = (char*)" ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler {
public:
    MouseToConsole() : x(40), y(12) {}

    void OnActivate() {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[y * 80 + x] = ((VideoMemory[y * 80 + x] & 0xf000) >> 4) | 
                                    ((VideoMemory[y * 80 + x] & 0x0f00) << 4) | 
                                    (VideoMemory[y * 80 + x] & 0x00ff);
    }

    void OnMouseMove(int8_t nx, int8_t ny) override {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[y * 80 + x] = ((VideoMemory[y * 80 + x] & 0xf000) >> 4) | 
                              ((VideoMemory[y * 80 + x] & 0x0f00) << 4) | 
                              (VideoMemory[y * 80 + x] & 0x00ff);

        x += nx;
        if (x < 0) x = 0;
        else if (x >= 80) x = 79;

        y += ny;
        if (y < 0) y = 0;
        else if (y >= 25) y = 24;

        VideoMemory[y * 80 + x] = ((VideoMemory[y * 80 + x] & 0xf000) >> 4) | 
                                ((VideoMemory[y * 80 + x] & 0x0f00) << 4) | 
                                (VideoMemory[y * 80 + x] & 0x00ff);
    }

private:
    int8_t x, y;
};

typedef void (*constructor) ();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
}

void sysprintf(char* str) {
    asm("int $0x80" : : "a" (4), "b" (str));
}

void taskA() {
    while(1) sysprintf((char*)"A");
}

void taskB() {
    while(1) sysprintf((char*)"B");
}

class PrintfUDPHandler : public UserDatagramProtocolHandler {
public:
    void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size) {
        char* foo = (char*)"";
        for (int i = 0; i < size; i++) {
            foo[0] = data[i];
            printf(foo);
        }
    }
};

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {

    // SET GLOBAL DESCRIPTOR TABLE
    GlobalDescriptorTable gdt;

    // SET MEMORYMANAGER -> HEAP 10MB 
    size_t heap = 10 * 1024 * 1024;
    uint32_t*  memupper = (uint32_t*)((size_t)multiboot_structure + 8);
    MemoryManager memoryManager(heap, (*memupper) * 1024 - heap - 10 * 1024);
    printf("heap: 0x");
    printf(heap);

    void* allocated = memoryManager.malloc(1024);
    printf("\nallocated: 0x");
    printf((size_t)allocated);
    printf("\n");

    // SET TASKING MANAGER
    TaskManager taskManager;
    // Task task1(&gdt, taskA);
    // Task task2(&gdt, taskB);
    // taskManager.AddTask(&task1);
    // taskManager.AddTask(&task2);

    // SET INTERRUPT MANAGER
    InterruptManager interrupts(0x20, &gdt, &taskManager);

    SyscallHandler syscalls(0x80, &interrupts);

// #define GRAPHICMODE

#ifdef GRAPHICMODE
    Desktop desktop(320, 200, 0x00, 0x00, 0xA8);
#endif

    // SET DRIVERMANAGER
    DriverManager drvManager;

    // SET KEY BOARD DRIVER
#ifdef  GRAPHICMODE
    KeyBoardDriver keyboard(&interrupts, &desktop);
#else    
    PrintKeyboardEventHandler kbhandler;
    KeyBoardDriver keyboard(&interrupts, &kbhandler);    
#endif
    drvManager.AddDriver(&keyboard);

    // SET MOUSE DRIVER
#ifdef  GRAPHICMODE
    MouseDriver mouse(&interrupts, &desktop);
#else
    MouseToConsole mousehandler;
    MouseDriver mouse(&interrupts, &mousehandler);
#endif
    drvManager.AddDriver(&mouse);

    // SET PCI DRIVER
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDriver(&drvManager, &interrupts);

    VideoGraphicsArray vga;


    drvManager.ActivateAll();
    
#ifdef GRAPHICMODE
    vga.SetMode(320, 200, 8);
    Window w1(&desktop, 10, 10, 20, 20, 0xA8, 0x00, 0x00);
    desktop.AddChild(&w1);
    Window w2(&desktop, 40, 15, 30, 30, 0x00, 0xA8, 0x00);
    desktop.AddChild(&w2);
#endif
    
    // interrupt 14
    // AdvancedTechnologyAttachment ata0m(0x1F0, true);
    // printf("ATA Primary Master: ");
    // ata0m.Identify();
    // AdvancedTechnologyAttachment ata0s(0x1F0, false);
    // printf("ATA Primary Slave: ");
    // ata0s.Identify();
    // char* atabuffer = (char*)"http://ATangyl.de";
    // ata0s.Write28(0, (uint8_t*)atabuffer, 25);
    // ata0s.Flush();
    // ata0s.Read28(0, (uint8_t*)atabuffer, 25);


    // interrupt 15
    // AdvancedTechnologyAttachment ata1m(0x170, true);
    // AdvancedTechnologyAttachment ata1s(0x170, false);
    
    // third: 0x1E8
    // fourth: 0x168
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
    uint32_t ip_be = ((uint32_t)ip4 << 24)
                   | ((uint32_t)ip3 << 16)
                   | ((uint32_t)ip2 << 8)
                   | ((uint32_t)ip1);
    
    uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
    uint32_t gip_be = ((uint32_t)gip4 << 24)
                   | ((uint32_t)gip3 << 16)
                   | ((uint32_t)gip2 << 8)
                   | ((uint32_t)gip1);
    
    amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);

    eth0->SetIPAddress(ip_be);

    EtherFrameProvider etherframe(eth0);

    AddressResolutionProtocol arp(&etherframe);

    uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;

    uint32_t subnet_be = ((uint32_t)subnet4 << 24)
                   | ((uint32_t)subnet3 << 16)
                   | ((uint32_t)subnet2 << 8)
                   | ((uint32_t)subnet1);
    
    InternetProtocolV4Provider ipv4(&etherframe, &arp, gip_be, subnet_be);
    InternetControlMessageProtocolHandler icmp(&ipv4);
    UserDatagramProtocolProvider udp(&ipv4);
    interrupts.Activate();
    // arp.Resolve(gip_be);
    // ipv4.Send(gip_be, 0x0008, (uint8_t*)"Hello Network", 13);
    arp.BroadcastMACAddress(gip_be);
    icmp.RequestEchoReply(gip_be);
    PrintfUDPHandler udpHandler;
    UserDatagramProtocolSocket* socket = udp.Listen(1234);
    udp.Bind(socket, &udpHandler);
    // udp.Send(socket, (uint8_t*)"Hello World!", 12);

    while(1) {
#ifdef GRAPHICMODE
      desktop.Draw(&vga);
#endif
    }
}