/* The System Communications area -- See SYS:I-SYS;SYSDF1 for details */

#ifndef _SYSTEMCOM_
#define _SYSTEMCOM_

#include <stddef.h>
#include "life_types.h"
#include "emulator.h"

#define SystemCommAreaAddress 0xF8041100L
#define SystemCommAreaSize 256

/* Returns the address of a slot in the SystemComm area */
#define SystemCommSlotAddress(slot) (((ptrdiff_t)SystemCommAreaAddress) / sizeof(EmbWord) + ((ptrdiff_t)(slot)))

// Reads a slot of the SystemComm area using the emulator's VM implementation
//#define ReadSystemCommSlot(slot, objectPointer) VirtualMemoryRead(SystemCommSlotAddress(slot), objectPointer)
void ReadSystemCommSlot(int slot, LispObj *objectPointer);

/* Writes a slot of the SystemComm area using the emulator's VM implementation
 */
#define WriteSystemCommSlot(slot, datum, tag)                                                                          \
    {                                                                                                                  \
        LispObj lispDatum;                                                                                             \
        lispDatum.DATA.u = (Integer)datum;                                                                             \
        lispDatum.TAG = (Tag)tag;                                                                                      \
        VirtualMemoryWrite(SystemCommSlotAddress(slot), &lispDatum);                                                   \
    }

// Genera version of System Communications area
// Define SYS:I-SYS;SYSDF1 line 403+
// Location 0x041100 / Size 0x100 (256) EmbWords
enum SystemCommAreaSlot {
    syscomMajorVersionNumber,
    syscomMinorVersionNumber,
    systemStartup,

    // Address Spacemap
    addressSpaceMapAddress, // Maps quanta to regions.  See STORAGE for format info.
    oblastFreeSize, // Contiguous free quanta in each oblast.

    // Per-area tables.  These are arrays.  They are here for the console program.
    areaName, // A symbol
    areaMaximumQuantumSize,
    areaRegionQuantumSize,
    areaRegionList,
    areaRegionBits,

    // Per-region tables.  These are arrays.  They are here for the console program.
    regionQuantumOrigin,
    regionQuantumLength,
    regionFreePointer, // Number of words actually used
    regionGCPointer, // Number of words scanned by (long-term) GC
    regionBits, // Fixnum of random fields (see %%REGION- bytes)
    regionListThread,
    regionArea,
    regionCreatedPages,
    regionFreePointerBeforeFlip,
    regionConsAlarm,
    pageConsAlarm,
    structureCacheRegion,
    listCacheRegion,
    defaultConsArea,

    // Pointers to critical storage-system tables (these are displaced arrays)
    pht, // Page hash table
    mmptY, // Main Memory Y subscript table
    mmpt, // Main Memory page table
    smpt, // Secondary Memory page table
    loadBitmaps,

    //  The following are red herrings for functionality that is really in FEPCOM.
    //  Presumably they leaked in from L-world during the IMach project.
    loadMap, /* Red herring */
    loadMapDPN, /* Red herring */
    swapMap, /* Red herring */
    swapMapDPN, /* Red herring */
    sysoutBitmaps,

    // Dynamic storage array, 4 bits per PHT bucket.
    phtCollisionCounts,
    mmpt1,
    storageColdBoot,
    flushableQueueHead,
    flushableQueueTail,
    flushableQueueModified,
    wiredPhysicalAddressHigh,
    wiredVirtualAddressHigh,
    enableSysoutAtColdBoot,
    sysoutGenerationNumber,
    sysoutTimestamp1,
    sysoutTimestamp2,

    // microsecond clock at some convenient time
    // of disk-save/sysout.
    sysoutParentTimestamp1,
    sysoutParentTimestamp2,
    initialStackGroup,
    currentStackGroup,
    stackGroupLock,
    currentStackGroupStatusBits,
    inhibitSchedulingFlag,
    controlStackLow,
    bindingStackLow,

    // Floating-point control registers
    floatOperatingMode,
    floatOperationStatus,

    packageNameTable,
    lispReleaseString,
    busMode,
};

typedef EmbWord SystemCommArea[60];

// Minima version of System Communications Area
// typedef struct {
//    EmbWord systemStartup;
//    EmbWord allAreas;
//    EmbWord allPackages;
//    EmbWord saveWorldHeader;
//    EmbWord kernelUseROMEthernet;
//} SystemCommArea;

// extern SystemCommArea *SystemCommAreaPtr;

#endif
