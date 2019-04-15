/*
   For historical reasons, the VM accessors return -1 on failure and 0 on
   success
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdio.h>

#include "ivory.h"

extern int VirtualMemoryWriteBlockConstant(Integer vma, LispObj *object, int count, int increment);
extern int VirtualMemoryWriteBlock(Integer vma, LispObj *object, int count);
extern int VirtualMemoryReadBlock(Integer vma, LispObj *object, int count);
extern int VirtualMemoryWrite(Integer vma, LispObj *object);
extern int VirtualMemoryRead(Integer vma, LispObj *object);
extern Tag *MapVirtualAddressTag(Integer vma);
extern Integer *MapVirtualAddressData(Integer vma);

// FIXME - deleted ", Boolean faultp" to match source code
extern Integer EnsureVirtualAddressRange(Integer virtualaddress, int count, Boolean faultp);
// FIXME - deleted ", Boolean faultp" to match source code
extern Integer EnsureVirtualAddress(Integer vma);

/* VLM virtual-memory "coprocessor" interface */
typedef uint8_t VMAttribute;

#define VMAttributeAccessFault 01          // 00000001
#define VMAttributeWriteFault 02           // 00000010
#define VMAttributeTransportFault 04       // 00000100
#define VMAttributeTransportDisable 010    // 00001000
#define VMAttributeEphemeral 020           // 00010000
#define VMAttributeModified 040            // 00100000
#define VMAttributeExists 0100             // 01000000

#define VMCreatedDefault (VMAttributeAccessFault | VMAttributeTransportFault | VMAttributeExists)

#define VMAccessFault(a)                ((a) & VMAttributeAccessFault)
#define VMWriteFault(a)                 ((a) & VMAttributeWriteFault)
#define VMTransportFault(a)             ((a) & VMAttributeTransportFault)
#define VMTransportDisable(a)           ((a) & VMAttributeTransportDisable)
#define VMEphemeral(a)                  ((a) & VMAttributeEphemeral)
#define VMModified(a)                   ((a) & VMAttributeModified)
#define VMExists(a)                     ((a) & VMAttributeExists)

#define SetVMAccessFault(a)             ((a) |= VMAttributeAccessFault)
#define SetVMWriteFault(a)              ((a) |= VMAttributeWriteFault)
#define SetVMTransportFault(a)          ((a) |= VMAttributeTransportFault)
#define SetVMTransportDisable(a)        ((a) |= VMAttributeTransportDisable)
#define SetVMEphemeral(a)               ((a) |= VMAttributeEphemeral)
#define SetVMModified(a)                ((a) |= VMAttributeModified)
#define SetVMExists(a)                  ((a) |= VMAttributeExists)

#define ClearVMAccessFault(a)           ((a) &= ~VMAttributeAccessFault)
#define ClearVMWriteFault(a)            ((a) &= ~VMAttributeWriteFault)
#define ClearVMTransportFault(a)        ((a) &= ~VMAttributeTransportFault)
#define ClearVMTransportDisable(a)      ((a) &= ~VMAttributeTransportDisable)
#define ClearVMEphemeral(a)             ((a) &= ~VMAttributeEphemeral)
#define ClearVMModified(a)              ((a) &= ~VMAttributeModified)
#define ClearVMExists(a)                ((a) &= ~VMAttributeExists)

extern Boolean EnableIDS; // WTF is that?
#define DefaultAttributes(faultp, worldp)                                                                              \
    ((VMAttributeExists | VMAttributeEphemeral) | ((faultp) ? VMAttributeAccessFault : 0)                             \
        | ((EnableIDS && (worldp)) ? 0 : VMAttributeModified))

typedef enum _VMRegisterNumber {
    VMRegisterCommand = 01100,
    VMRegisterAddress,
    VMRegisterExtent,
    VMRegisterAttributes,
    VMRegisterDestination,
    VMRegisterData
} VMRegisterNumber;

typedef enum _VMOpcode {
    VMOpcodeLookup, /* reply is index */
    VMOpcodeCreate,
    VMOpcodeDestroy,

    VMOpcodeReadAttributes, /* operand is index */
    VMOpcodeWriteAttributes, /* operand is index */

    VMOpcodeFill, /* operand is increment (of fill data) */
    VMOpcodeSearch, /* operand is increment (of address) */
    VMOpcodeCopy, /* operand is memory-cycle? */

    VMOpcodeScan,
    VMOpcodeEnable
} VMOpcode;

typedef enum _VMResultCode { VMResultSuccess, VMResultFailure } VMResultCode;

int VMCommand(int command);
#define VMCommandOpcode(command) ((VMOpcode)ldb(13, 19, command))
#define VMCommandOperand(command) ((int)ldb(19, 0, command))

#define SetVMReplyResult(reply, result) (dpb((int)(result ? VMResultSuccess : VMResultFailure), 13, 19, reply))

typedef struct _VMState {
    Integer CommandRegister;
    Integer AddressRegister;
    Integer ExtentRegister;
    Integer AttributesRegister;
    Integer DestinationRegister;
    LispObj DataRegister;
} VMState;

extern VMState VM;

static int ComputeProtection(VMAttribute attr);
Integer MapWorldLoad(Integer vma, int length, int worldfile, off_t dataoffset, off_t tagoffset);

#endif
