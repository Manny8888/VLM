

#ifdef _C_EMULATOR_

#include "memory.h"
#include "emulator.h"
#include "SystemComm.h"

void ReadSystemCommSlot(int slot, LispObj *objectPointer)
{
    VirtualMemoryRead(SystemCommSlotAddress(slot), objectPointer);
}

#endif
