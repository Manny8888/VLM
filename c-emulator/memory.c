
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <unistd.h>
#include <string.h>

#include "aihead.h"
#include "emulator.h"
#include "ivory.h"
#include "memory.h"
#include "utilities.h"

/* --- need a better place */

const LispObj ObjectT = { TypeSymbol, AddressT };
const LispObj ObjectNIL = { TypeNIL, AddressNIL };
const LispObj ObjectCdrMask = { TagCdrMask, 0 };

extern Integer memory_vma;

/* Superstition says threads go at 1<<32 */
Tag *TagSpace = (Tag *)((long)2 << 32); /* 1<<32 bytes of tages */
Integer *DataSpace = (Integer *)((long)3 << 32); /* 4<<32 bytes of data */

/*
   --- We know underlying machine uses 8192-byte pages, we have to
   create a page at a time, and tags are char (byte) sized, so we have
   to create a page of tags at a time
 */

#define MemoryPageSize 0x2000     // 8,192 = 2^13
#define MemoryPageMask (MemoryPageSize - 1)
#define MemoryAddressPageShift 13

#define MemoryPageNumber(vma) ((vma) >> MemoryAddressPageShift)
#define MemoryPageOffset(vma) ((vma) & MemoryPageMask)
#define PageNumberMemory(vpn) ((vpn) << MemoryAddressPageShift)

/* This could be a sparse array, should someone want to implement it */
VMAttribute VMAttributeTable[1 << (32 - MemoryAddressPageShift)];

#define Created(vma) VMExists(VMAttributeTable[MemoryPageNumber(vma)])
#define SetCreated(vma) (VMAttributeTable[MemoryPageNumber(vma)] = VMCreatedDefault)
#define ClearCreated(vma) (VMAttributeTable[MemoryPageNumber(vma)] = 0)


/**** Virtual memory system ****/

Integer EnsureVirtualAddress(Integer vma)
{
    caddr_t data, tag;
    Integer aligned_vma = vma - MemoryPageOffset(vma);

    if (Created(vma)) {
        return (vma);
}

    data = (caddr_t)&DataSpace[aligned_vma];
    tag = (caddr_t)&TagSpace[aligned_vma];
    if (data
        != mmap(data, sizeof(Integer[MemoryPageSize]), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
            -1, 0)) {
        printf("Couldn't map data page at %s for VMA %016lx", data, vma);
    }
    if (tag
        != mmap(
            tag, sizeof(Tag[MemoryPageSize]), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0)) {
        printf("Couldn't map tag page at %s for VMA %016lx", tag, vma);
    }

    SetCreated(vma);
    return (vma);
}

Integer EnsureVirtualAddressRange(Integer virtualaddress, int count, Boolean faultp)
{
    int pages = ceiling(count, MemoryPageSize);
    caddr_t data, tag;
    Integer aligned_vma = virtualaddress - MemoryPageOffset(virtualaddress);
    int n;

    while (pages) {
        n = 0;
        while (!Created(virtualaddress) && pages) {
            n++;
            pages--;
            SetCreated(virtualaddress);
            virtualaddress += MemoryPageSize;
        }
        if (n) {
            data = (caddr_t)&DataSpace[aligned_vma];
            tag = (caddr_t)&TagSpace[aligned_vma];

            if (data
                != mmap(data, n * sizeof(Integer[MemoryPageSize]), PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0)) {
                printf("Couldn't map %d data pages at %s for VMA %016lx", n, data, aligned_vma);
}

            if (tag
                != mmap(tag, n * sizeof(Tag[MemoryPageSize]), PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0)) {
                printf("Couldn't map %d tag pages at %s for VMA %016lx", n, tag, aligned_vma);
}
            aligned_vma += n * MemoryPageSize;
        }

        while (Created(virtualaddress) && pages) {
            pages--;
            virtualaddress += MemoryPageSize;
            aligned_vma += MemoryPageSize;
        }
    }
    return (virtualaddress);
}

Integer DestroyVirtualAddress(Integer vma)
{
    caddr_t data, tag;
    Integer aligned_vma = vma - MemoryPageOffset(vma);

    if (!Created(vma)) {
        return (vma);
}

    data = (caddr_t)&DataSpace[aligned_vma];
    tag = (caddr_t)&TagSpace[aligned_vma];
    if (munmap(data, sizeof(Integer[MemoryPageSize]))) {
        printf("Couldn't unmap data page at %s for VMA %016lx", data, vma);
}
    if (munmap(tag, sizeof(Tag[MemoryPageSize]))) {
        printf("Couldn't unmap tag page at %s for VMA %016lx", tag, vma);
}

    ClearCreated(vma);
    return (vma);
}

Integer DestroyVirtualAddressRange(Integer vma, int count)
{
    int pages = ceiling(count, MemoryPageSize);

    for (; pages--; vma += MemoryPageSize) {
        DestroyVirtualAddress(vma);
}

    return (vma);
}

Integer *MapVirtualAddressData(Integer vma) { return (&DataSpace[vma]); }

Tag *MapVirtualAddressTag(Integer vma) { return (&TagSpace[vma]); }

int VirtualMemoryRead(Integer vma, LispObj *object)
{
    /* set memory_vma for SEGV handler */
    memory_vma = vma;

    object->DATA.u = DataSpace[vma];
    object->TAG = TagSpace[vma];
    return (0);
}

int VirtualMemoryWrite(Integer vma, LispObj *object)
{
    /* set memory_vma for SEGV handler */
    memory_vma = vma;

    DataSpace[vma] = object->DATA.u;
    TagSpace[vma] = object->TAG;
    return (0);
}

int VirtualMemoryReadBlock(Integer vma, LispObj *object, int count)
{
    Integer *data = &DataSpace[vma];
    Tag *tag = &TagSpace[vma];
    Integer *edata = &DataSpace[vma + count];

    /* set memory_vma for SEGV handler */
    memory_vma = vma;

    for (; data < edata; object++, memory_vma++) {
        object->DATA.u = *data++;
        object->TAG = *tag++;
    }
    return (0);
}

int VirtualMemoryWriteBlock(Integer vma, LispObj *object, int count)
{
    Integer *data = &DataSpace[vma];
    Tag *tag = &TagSpace[vma];
    Integer *edata = &DataSpace[vma + count];

    /* set memory_vma for SEGV handler */
    memory_vma = vma;

    for (; data < edata; object++, memory_vma++) {
        *data++ = object->DATA.u;
        *tag++ = object->TAG;
    }
    return (0);
}

int VirtualMemoryWriteBlockConstant(Integer vma, LispObj *object, int count, int increment)
{
    Integer *data = &DataSpace[vma];
    Tag *tag = &TagSpace[vma];
    Tag ctag = object->TAG;
    Integer cdata = object->DATA.u;
    Integer *edata = &DataSpace[vma + count];

    /* set memory_vma for SEGV handler */
    memory_vma = vma;

    (void)memset((unsigned char *)tag, (unsigned char)ctag, count * sizeof(Tag));

    switch (increment) {
    case 0:
        if (cdata == 0) {
            (void)memset((unsigned char *)data, (unsigned char)0, count * sizeof(Integer));
        } else {
            for (; data < edata; *data++ = cdata, memory_vma++) {
                ;
}
}
        break;
    case 1:
        for (; data < edata; *data++ = cdata++, memory_vma++) {
            ;
}
        break;
    default:
        for (; data < edata; *data++ = cdata, cdata += increment, memory_vma++) {
            ;
}
    }
    return (0);
}

/* --- bleah, this probably has to use data-read cycles */
Boolean VirtualMemorySearch(Integer *vma, LispObj *object, int count)
{
    Tag *tag = &TagSpace[*vma];
    Tag *etag = &TagSpace[*vma + count];
    Tag ctag = object->TAG;
    Integer cdata = object->DATA.u;

    for (; tag < etag;) {
        tag = (Tag *)memchr((unsigned char *)tag, (unsigned char)ctag, (etag - tag) * sizeof(Tag));
        if (tag == NULL) {
            return (False);
}

        /* set memory_vma for SEGV handler */
        memory_vma = tag - TagSpace;
        if (DataSpace[memory_vma] == cdata) {
            *vma = memory_vma;
            return (True);
        }
        tag++;
    }
    return (False);
}

int VirtualMemoryCopy(Integer from, Integer to, int count, Byte row[])
{
    Integer *fromdata = &DataSpace[from];
    Tag *fromtag = &TagSpace[from];
    Tag *etag = &TagSpace[from + count];
    Integer *todata = &DataSpace[to];
    Tag *totag = &TagSpace[to];
    LispObj obj;
    Tag tag;
    int action;

    /* set memory_vma for SEGV handler */
    memory_vma = from;

    if (row == MemoryActionTable[CycleRaw]) {
        (void)memmove((unsigned char *)totag, (unsigned char *)fromtag, count * sizeof(Tag));
        (void)memmove((unsigned char *)todata, (unsigned char *)fromdata, count * sizeof(Integer));
        return (0);
    }

    for (; fromtag < etag;) {
        /* Transport takes precedence over anything but trap */
        if (((action = row[tag = *fromtag]) & (MemoryActionTransport | MemoryActionTrap)) == MemoryActionTransport) {
            if (OldspaceAddressP(*fromdata)) {
                TakeMemoryTrap(TransportTrapVector, *fromdata);
            }
        }

        if (action) {
            MemoryReadInternal(fromtag - TagSpace, &obj, row);
            *totag++ = obj.TAG;
            *todata++ = obj.DATA.u;
            fromtag++;
            fromdata++;
        } else {
            *totag++ = tag;
            fromtag++;
            *todata++ = *fromdata++;
        }
        memory_vma++;
    }

    return (0);
}

Boolean VirtualMemoryScan(Integer *vma, int count)
{
    VMAttribute *attr = &VMAttributeTable[MemoryPageNumber(*vma)];

    for (; count > 0; attr++, count -= MemoryPageSize) {
        if (VMTransportFault(*attr)) {
            Integer scanvma = PageNumberMemory(attr - VMAttributeTable);
            Tag *tag = &TagSpace[scanvma];
            Tag *etag = &TagSpace[scanvma + (MemoryPageSize < count ? MemoryPageSize : count)];

            for (; tag < etag; tag++) {
                if (PointerTypeP(*tag) && (OldspaceAddressP(DataSpace[tag - TagSpace]))) {
                    *vma = tag - TagSpace;
                    return (True);
                }
            }
        }
    }
    return (False);
}

void VirtualMemoryEnable(Integer vma, int count)
{
    VMAttribute *attr = &VMAttributeTable[MemoryPageNumber(vma)];
    VMAttribute *eattr = &VMAttributeTable[MemoryPageNumber(vma + count)];

    for (; attr < eattr; attr++) {
        VMAttribute a = *attr;
        if (VMExists(a) && !VMTransportDisable(a)) {
            *attr = SetVMTransportFault(a);
}
    }
}

VMState VM;

int VMCommand(int command)
{
    VMState *vm = &VM;

    switch
        VMCommandOpcode(command)
        {
        case VMOpcodeLookup: {
            int vpn = MemoryPageNumber(vm->AddressRegister);

            return (SetVMReplyResult(vpn, VMExists(VMAttributeTable[vpn])));
        }

        case VMOpcodeCreate:
            EnsureVirtualAddressRange(vm->AddressRegister, vm->ExtentRegister, False);
            return (SetVMReplyResult(0, True));

        case VMOpcodeDestroy:
            DestroyVirtualAddressRange(vm->AddressRegister, vm->ExtentRegister);
            return (SetVMReplyResult(0, True));

        case VMOpcodeReadAttributes: {
            VMAttribute attr = VMAttributeTable[VMCommandOperand(command)];

            if
                VMExists(attr)
                {
                    vm->AttributesRegister = VMAttributeTable[VMCommandOperand(command)];
                    return (SetVMReplyResult(command, True));
                }
            else
                return (SetVMReplyResult(command, False));
        }

        case VMOpcodeWriteAttributes: {
            VMAttribute attr = VMAttributeTable[VMCommandOperand(command)];

            if
                VMExists(attr)
                {
                    /* ensure Lisp doesn't clear exists bit */
                    VMAttributeTable[VMCommandOperand(command)] = SetVMExists(vm->AttributesRegister);
                    return (SetVMReplyResult(command, True));
                }
            else
                return (SetVMReplyResult(command, False));
        }

        case VMOpcodeFill:
            VirtualMemoryWriteBlockConstant(
                vm->AddressRegister, &vm->DataRegister, vm->ExtentRegister, VMCommandOperand(command));
            return (SetVMReplyResult(0, True));

        case VMOpcodeSearch: {
            Boolean result = VirtualMemorySearch(&vm->AddressRegister, &vm->DataRegister, vm->ExtentRegister);
            return (SetVMReplyResult(0, result));
        }

        case VMOpcodeCopy: {
            Boolean result = VirtualMemoryCopy(vm->AddressRegister, vm->DestinationRegister, vm->ExtentRegister,
                MemoryActionTable[VMCommandOperand(command)]);
            return (SetVMReplyResult(0, result));
        }
        case VMOpcodeScan: {
            Boolean result = VirtualMemoryScan(&vm->AddressRegister, vm->ExtentRegister);
            return (SetVMReplyResult(0, result));
        }

        case VMOpcodeEnable: {
            VirtualMemoryEnable(vm->AddressRegister, vm->ExtentRegister);
            return (SetVMReplyResult(0, True));
        }
        }
}



/* Wads are clusters of pages for swap contiguity.  The current value is
/* chosen so that all the attributes of a wad fit in one long */
/* Note that MemoryWad_AddressShift = MemoryPage_AddressShift + 3 */
#define MemoryWad_AddressShift 16
#define MemoryWad_Size (1 << MemoryWad_AddressShift)
#define MemoryWad_Mask (MemoryWad_Size - 1)
#define MemoryWadNumber(vma) ((vma) >> MemoryWad_AddressShift)
#define MemoryWadOffset(vma) ((vma) & MemoryWad_Mask)
#define WadNumberMemory(vwn) ((vwn) << MemoryWad_AddressShift)

#define WadExistsMask 0x4040404040404040 /* f-ing poor excuse for a macro language */
#define WadCreated(vma) ((((int64_t *)VMAttributeTable)[MemoryWadNumber(vma)]) & WadExistsMask)


static int unmapped_world_words = 0;
static int mapped_world_words = 0;
static int file_map_entries = 0;
static int swap_map_entries = 0;
static int ComputeProtection(VMAttribute attr);

Integer MapWorldLoad(Integer vma, int length, int worldfile, off_t dataoffset, off_t tagoffset)
{
    caddr_t data;
    caddr_t tag;

    /* According to the doc, by mapping PRIVATE, writes to the address
    /* will not go to the file, so we get copy-on-write for free.  The
    /* only reason we map read-only, is to catch modified for IDS */

    /* --- for now, we don't try to discover modified: it seems to run us
    /* out of map entries */
    VMAttribute attr = DefaultAttributes(False, True);
    int prot = ComputeProtection(attr);
    size_t dataCount, tagCount;
    int words;

    for (; length > 0;) {
        /* sigh, have to copy partial pages and pages that already exist
        /* (e.g., shared FEP page) */
        for (; (length > 0) && (MemoryWadOffset(vma) || Created(vma) || (length < MemoryWad_Size));) {
            words = MemoryPage_Size - MemoryPageOffset(vma);
            if (words > length) {
                words = length;
            }
            EnsureVirtualAddress(vma);

            dataCount = sizeof(Integer) * words;
            if (dataoffset != lseek(worldfile, dataoffset, SEEK_SET))
                vpunt(NULL, "Unable to seek to data offset %d in world file", dataoffset);
            if (dataCount != read(worldfile, MapVirtualAddressData(vma), dataCount))
                vpunt(NULL, "Unable to read data page %d from world file", MemoryPageNumber(vma));

            tagCount = sizeof(Tag) * words;
            if (tagoffset != lseek(worldfile, tagoffset, SEEK_SET))
                vpunt(NULL, "Unable to seek to tag offset %d in world file", tagoffset);
            if (tagCount != read(worldfile, MapVirtualAddressTag(vma), tagCount))
                vpunt(NULL, "Unable to read tag page %d from world file", MemoryPageNumber(vma));

            /* Adjust the protection to catch modifications to world pages */
            SetCreated(vma);

            vma += words;
            dataoffset += dataCount;
            tagoffset += tagCount;
            length -= words;
            unmapped_world_words += words;
        }
        swap_map_entries += 1;

        if (length > 0) {
            int limit = length - MemoryWadOffset(length);

            /* Set the attributes for mapped in pages */
            for (words = 0; (words < limit) && !WadCreated(vma + words);) {
                int wadlimit = words + MemoryWad_Size;
                VMAttribute *pattr = &VMAttributeTable[MemoryPageNumber(vma + words)];

                for (; words < wadlimit; words += MemoryPage_Size, pattr++)
                    *pattr = attr;
            }

            data = (caddr_t)&DataSpace[vma];
            tag = (caddr_t)&TagSpace[vma];
            if (data
                != mmap(data, dataCount = sizeof(Integer) * words, PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_FILE | MAP_PRIVATE | MAP_FIXED, worldfile, dataoffset))
                vpunt(NULL, "Couldn't map %d world data pages at %lx for VMA %x", MemoryPageNumber(words), data, vma);
            if (tag
                != mmap(tag, tagCount = sizeof(Tag) * words, prot, MAP_FILE | MAP_PRIVATE | MAP_FIXED, worldfile,
                        tagoffset))
                vpunt(NULL, "Couldn't map %d world tag pages at %lx for VMA %x", MemoryPageNumber(words), tag, vma);

            vma += words;
            dataoffset += dataCount;
            tagoffset += tagCount;
            length -= words;
            mapped_world_words += words;
            file_map_entries += 2;
        }
    }
    return (vma);
}

