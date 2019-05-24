/* #define DEBUGICACHE 42 */

#include "std.h"

#include <sys/file.h>

#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "aistat.h" /* Alpha-Ivory state */
#include "aihead.h" /* Alpha-Ivory constants */
#include "traps.h" /* Alpha-Ivory traps */
#include "ivoryrep.h" /* Prototypes for this file */
#include "asmfuns.h" /* Prototypes for the dispatch functions */
#include "utilities.h"

/* In this file, we establish all datastructures used by the ASM
 * interpreter.  Here we have functions for initializing the datastructures,
 * accessing the datastructures, and debugging them.
 */

/* This is the parameter section.  Variables here adjust certain aspects
 * of the implementation.
 */

int icachesize = (CacheLine_Mask + 1); /* around 65K instructions. */
int stackcachesize = Stack_CacheSize; /* 16K Ivory words */
LispObjRecordp stackcache = NULL; /* Allocate once only */
CACHELINEP instructioncache = NULL; /* Allocate first time, then clear */

/* These are the memory cycle action tables */

int MemoryActionTable[13][64] = { { 014, 06, 014, 010, 05, 05, 05, 05, 0, 0, 0, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04,
                                      04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 0, 0, 044, 0, 024, 010, 04, 04, 04,
                                      04, 04, 04, 04, 04, 04, 04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 06, 010, 010, 05, 05, 05, 05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        044, 0, 0, 010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 04, 06, 014, 010, 04, 05, 05, 05, 0, 0, 0, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04,
        04, 04, 04, 0, 0, 04, 0, 04, 010, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0 },
    { 0, 06, 010, 010, 0, 05, 05, 05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 04, 05, 014, 010, 04, 05, 05, 05, 0, 0, 0, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04,
        04, 04, 04, 0, 0, 04, 0, 04, 010, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0 },
    { 0, 05, 010, 010, 0, 05, 05, 05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 014, 014, 04, 0, 014, 014, 05, 014, 010, 010, 010, 014, 014, 014, 014, 014, 014, 014, 014, 014, 014, 014, 014,
        014, 014, 014, 014, 014, 014, 014, 014, 014, 010, 010, 014, 010, 014, 010, 014, 014, 014, 014, 014, 014, 014,
        014, 014, 014, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010, 010 },
    { 0, 0, 0, 0, 0, 0, 05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 04, 04, 04, 0, 04, 04, 04, 04, 0, 0, 0, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04,
        04, 04, 04, 0, 0, 04, 0, 04, 010, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0 },
    { 0, 0, 010, 010, 0, 0, 05, 05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 020,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

/* These are the master dispatch tables.  All entries are entry addresses of
 * ASM coded interpretation routines. The halfword instructions are duplicated
 * four times to provide operand decode speedup.  This is a somewhat dubious
 * speedup because of the effect on datacache misses, but if we get enough
 * emulated iCache hits, it will more than make up for it, and this way we
 * will benefit from future increases in Alpha DCache size increases.
 */
DISPATCHTABLE(halfworddispatch, 256 * 4) = {
    DoCarFP, DoCarLP, DoCarSP, DoCarIM, /* #o00 */
    DoCdrFP, DoCdrLP, DoCdrSP, DoCdrIM, /* #o01 */
    DoEndpFP, DoEndpLP, DoEndpSP, DoEndpIM, /* #o02 */
    DoSetup1DArrayFP, DoSetup1DArrayLP, DoSetup1DArraySP, DoSetup1DArrayIM, /* #o03 */
    DoSetupForce1DArrayFP, DoSetupForce1DArrayLP, DoSetupForce1DArraySP, DoSetupForce1DArrayIM, /* #o04 */
    DoBindLocativeFP, DoBindLocativeLP, DoBindLocativeSP, DoBindLocativeIM, /* #o05 */
    DoRestoreBindingStackFP, DoRestoreBindingStackLP, DoRestoreBindingStackSP, DoRestoreBindingStackIM, /* #o06 */
    DoEphemeralpFP, DoEphemeralpLP, DoEphemeralpSP, DoEphemeralpIM, /* #o07 */
    DoStartCallFP, DoStartCallLP, DoStartCallSP, DoStartCallIM, /* #o010 */
    DoJumpFP, DoJumpLP, DoJumpSP, DoJumpIM, /* #o011 */
    DoTagFP, DoTagLP, DoTagSP, DoTagIM, /* #o012 */
    DoDereferenceFP, DoDereferenceLP, DoDereferenceSP, DoDereferenceIM, /* #o013 */
    DoLogicTailTestFP, DoLogicTailTestLP, DoLogicTailTestSP, DoLogicTailTestIM, /* #o014 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o015 +++ Used for breakpoints!!! */
    DoDoubleFloatOpFP, DoDoubleFloatOpLP, DoDoubleFloatOpSP, DoDoubleFloatOpIM, /* #o016 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o017 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o020 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o021 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o022 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o023 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o024 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o025 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o026 */
    DoPushLexicalVarNFP, DoPushLexicalVarNLP, DoPushLexicalVarNSP, DoPushLexicalVarNIM, /* #o027 */
    DoBlock0WriteFP, DoBlock0WriteLP, DoBlock0WriteSP, DoBlock0WriteIM, /* #o030 */
    DoBlock1WriteFP, DoBlock1WriteLP, DoBlock1WriteSP, DoBlock1WriteIM, /* #o031 */
    DoBlock2WriteFP, DoBlock2WriteLP, DoBlock2WriteSP, DoBlock2WriteIM, /* #o032 */
    DoBlock3WriteFP, DoBlock3WriteLP, DoBlock3WriteSP, DoBlock3WriteIM, /* #o033 */
    DoZeropFP, DoZeropLP, DoZeropSP, DoZeropIM, /* #o034 */
    DoMinuspFP, DoMinuspLP, DoMinuspSP, DoMinuspIM, /* #o035 */
    DoPluspFP, DoPluspLP, DoPluspSP, DoPluspIM, /* #o036 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o037 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o040 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o041 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o042 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o043 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o044 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o045 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o046 */
    DoTypeMemberFP, DoTypeMemberLP, DoTypeMemberSP, DoTypeMemberIM, /* #o047 */
    DoLocateLocalsFP, DoLocateLocalsLP, DoLocateLocalsSP, DoLocateLocalsIM, /* #o050 */
    DoCatchCloseFP, DoCatchCloseLP, DoCatchCloseSP, DoCatchCloseIM, /* #o051 */
    DoGenericDispatchFP, DoGenericDispatchLP, DoGenericDispatchSP, DoGenericDispatchIM, /* #o052 */
    DoMessageDispatchFP, DoMessageDispatchLP, DoMessageDispatchSP, DoMessageDispatchIM, /* #o053 */
    DoCheckPreemptRequestFP, DoCheckPreemptRequestLP, DoCheckPreemptRequestSP, DoCheckPreemptRequestIM, /* #o054 */
    DoPushGlobalLogicVariableFP, DoPushGlobalLogicVariableLP, DoPushGlobalLogicVariableSP,
    DoPushGlobalLogicVariableIM, /* #o055 */
    DoNoOpFP, DoNoOpLP, DoNoOpSP, DoNoOpIM, /* #o056 */
    DoHaltFP, DoHaltLP, DoHaltSP, DoHaltIM, /* #o057 */
    DoBranchTrueFP, DoBranchTrueLP, DoBranchTrueSP, DoBranchTrueIM, /* #o060 */
    DoBranchTrueElseExtraPopFP, DoBranchTrueElseExtraPopLP, DoBranchTrueElseExtraPopSP,
    DoBranchTrueElseExtraPopIM, /* #o061 */
    DoBranchTrueAndExtraPopFP, DoBranchTrueAndExtraPopLP, DoBranchTrueAndExtraPopSP,
    DoBranchTrueAndExtraPopIM, /* #o062 */
    DoBranchTrueExtraPopFP, DoBranchTrueExtraPopLP, DoBranchTrueExtraPopSP, DoBranchTrueExtraPopIM, /* #o063 */
    DoBranchTrueNoPopFP, DoBranchTrueNoPopLP, DoBranchTrueNoPopSP, DoBranchTrueNoPopIM, /* #o064 */
    DoBranchTrueAndNoPopFP, DoBranchTrueAndNoPopLP, DoBranchTrueAndNoPopSP, DoBranchTrueAndNoPopIM, /* #o065 */
    DoBranchTrueElseNoPopFP, DoBranchTrueElseNoPopLP, DoBranchTrueElseNoPopSP, DoBranchTrueElseNoPopIM, /* #o066 */
    DoBranchTrueAndNoPopElseNoPopExtraPopFP, DoBranchTrueAndNoPopElseNoPopExtraPopLP,
    DoBranchTrueAndNoPopElseNoPopExtraPopSP, DoBranchTrueAndNoPopElseNoPopExtraPopIM, /* #o067 */
    DoBranchFalseFP, DoBranchFalseLP, DoBranchFalseSP, DoBranchFalseIM, /* #o070 */
    DoBranchFalseElseExtraPopFP, DoBranchFalseElseExtraPopLP, DoBranchFalseElseExtraPopSP,
    DoBranchFalseElseExtraPopIM, /* #o071 */
    DoBranchFalseAndExtraPopFP, DoBranchFalseAndExtraPopLP, DoBranchFalseAndExtraPopSP,
    DoBranchFalseAndExtraPopIM, /* #o072 */
    DoBranchFalseExtraPopFP, DoBranchFalseExtraPopLP, DoBranchFalseExtraPopSP, DoBranchFalseExtraPopIM, /* #o073 */
    DoBranchFalseNoPopFP, DoBranchFalseNoPopLP, DoBranchFalseNoPopSP, DoBranchFalseNoPopIM, /* #o074 */
    DoBranchFalseAndNoPopFP, DoBranchFalseAndNoPopLP, DoBranchFalseAndNoPopSP, DoBranchFalseAndNoPopIM, /* #o075 */
    DoBranchFalseElseNoPopFP, DoBranchFalseElseNoPopLP, DoBranchFalseElseNoPopSP, DoBranchFalseElseNoPopIM, /* #o076 */
    DoBranchFalseAndNoPopElseNoPopExtraPopFP, DoBranchFalseAndNoPopElseNoPopExtraPopLP,
    DoBranchFalseAndNoPopElseNoPopExtraPopSP, DoBranchFalseAndNoPopElseNoPopExtraPopIM, /* #o077 */
    DoPushFP, DoPushLP, DoPushSP, DoPushIM, /* #o0100 */
    DoPushNNilsFP, DoPushNNilsLP, DoPushNNilsSP, DoPushNNilsIM, /* #o0101 */
    DoPushAddressSpRelativeFP, DoPushAddressSpRelativeLP, DoPushAddressSpRelativeSP,
    DoPushAddressSpRelativeIM, /* #o0102 */
    DoPushLocalLogicVariablesFP, DoPushLocalLogicVariablesLP, DoPushLocalLogicVariablesSP,
    DoPushLocalLogicVariablesIM, /* #o0103 */
    DoReturnMultipleFP, DoReturnMultipleLP, DoReturnMultipleSP, DoReturnMultipleIM, /* #o0104 */
    DoReturnKludgeFP, DoReturnKludgeLP, DoReturnKludgeSP, DoReturnKludgeIM, /* #o0105 */
    DoTakeValuesFP, DoTakeValuesLP, DoTakeValuesSP, DoTakeValuesIM, /* #o0106 */
    DoUnbindNFP, DoUnbindNLP, DoUnbindNSP, DoUnbindNIM, /* #o0107 */
    DoPushInstanceVariableFP, DoPushInstanceVariableLP, DoPushInstanceVariableSP, DoPushInstanceVariableIM, /* #o0110 */
    DoPushAddressInstanceVariableFP, DoPushAddressInstanceVariableLP, DoPushAddressInstanceVariableSP,
    DoPushAddressInstanceVariableIM, /* #o0111 */
    DoPushInstanceVariableOrderedFP, DoPushInstanceVariableOrderedLP, DoPushInstanceVariableOrderedSP,
    DoPushInstanceVariableOrderedIM, /* #o0112 */
    DoPushAddressInstanceVariableOrderedFP, DoPushAddressInstanceVariableOrderedLP,
    DoPushAddressInstanceVariableOrderedSP, DoPushAddressInstanceVariableOrderedIM, /* #o0113 */
    DoUnaryMinusFP, DoUnaryMinusLP, DoUnaryMinusSP, DoUnaryMinusIM, /* #o0114 */
    DoReturnSingleFP, DoReturnSingleLP, DoReturnSingleSP, DoReturnSingleIM, /* #o0115 */
    DoMemoryReadFP, DoMemoryReadLP, DoMemoryReadSP, DoMemoryReadIM, /* #o0116 */
    DoMemoryReadFP, DoMemoryReadLP, DoMemoryReadSP, DoMemoryReadIM, /* #o0117 */
    DoBlock0ReadFP, DoBlock0ReadLP, DoBlock0ReadSP, DoBlock0ReadIM, /* #o0120 */
    DoBlock1ReadFP, DoBlock1ReadLP, DoBlock1ReadSP, DoBlock1ReadIM, /* #o0121 */
    DoBlock2ReadFP, DoBlock2ReadLP, DoBlock2ReadSP, DoBlock2ReadIM, /* #o0122 */
    DoBlock3ReadFP, DoBlock3ReadLP, DoBlock3ReadSP, DoBlock3ReadIM, /* #o0123 */
    DoBlock0ReadShiftFP, DoBlock0ReadShiftLP, DoBlock0ReadShiftSP, DoBlock0ReadShiftIM, /* #o0124 */
    DoBlock1ReadShiftFP, DoBlock1ReadShiftLP, DoBlock1ReadShiftSP, DoBlock1ReadShiftIM, /* #o0125 */
    DoBlock2ReadShiftFP, DoBlock2ReadShiftLP, DoBlock2ReadShiftSP, DoBlock2ReadShiftIM, /* #o0126 */
    DoBlock3ReadShiftFP, DoBlock3ReadShiftLP, DoBlock3ReadShiftSP, DoBlock3ReadShiftIM, /* #o0127 */
    DoBlock0ReadTestFP, DoBlock0ReadTestLP, DoBlock0ReadTestSP, DoBlock0ReadTestIM, /* #o0130 */
    DoBlock1ReadTestFP, DoBlock1ReadTestLP, DoBlock1ReadTestSP, DoBlock1ReadTestIM, /* #o0131 */
    DoBlock2ReadTestFP, DoBlock2ReadTestLP, DoBlock2ReadTestSP, DoBlock2ReadTestIM, /* #o0132 */
    DoBlock3ReadTestFP, DoBlock3ReadTestLP, DoBlock3ReadTestSP, DoBlock3ReadTestIM, /* #o0133 */
    DoFinishCallNFP, DoFinishCallNLP, DoFinishCallNSP, DoFinishCallNIM, /* #o0134 */
    DoFinishCallNFP, DoFinishCallNLP, DoFinishCallNSP, DoFinishCallNIM, /* #o0135 */
    DoFinishCallTosFP, DoFinishCallTosLP, DoFinishCallTosSP, DoFinishCallTosIM, /* #o0136 */
    DoFinishCallTosFP, DoFinishCallTosLP, DoFinishCallTosSP, DoFinishCallTosIM, /* #o0137 */
    DoSetToCarFP, DoSetToCarLP, DoSetToCarSP, DoSetToCarIM, /* #o0140 */
    DoSetToCdrFP, DoSetToCdrLP, DoSetToCdrSP, DoSetToCdrIM, /* #o0141 */
    DoSetToCdrPushCarFP, DoSetToCdrPushCarLP, DoSetToCdrPushCarSP, DoSetToCdrPushCarIM, /* #o0142 */
    DoIncrementFP, DoIncrementLP, DoIncrementSP, DoIncrementIM, /* #o0143 */
    DoDecrementFP, DoDecrementLP, DoDecrementSP, DoDecrementIM, /* #o0144 */
    DoPointerIncrementFP, DoPointerIncrementLP, DoPointerIncrementSP, DoPointerIncrementIM, /* #o0145 */
    DoSetCdrCode1FP, DoSetCdrCode1LP, DoSetCdrCode1SP, DoSetCdrCode1IM, /* #o0146 */
    DoSetCdrCode2FP, DoSetCdrCode2LP, DoSetCdrCode2SP, DoSetCdrCode2IM, /* #o0147 */
    DoPushAddressFP, DoPushAddressLP, DoPushAddressSP, DoPushAddressIM, /* #o0150 */
    DoSetSpToAddressFP, DoSetSpToAddressLP, DoSetSpToAddressSP, DoSetSpToAddressIM, /* #o0151 */
    DoSetSpToAddressSaveTosFP, DoSetSpToAddressSaveTosLP, DoSetSpToAddressSaveTosSP,
    DoSetSpToAddressSaveTosIM, /* #o0152 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0153 */
    DoReadInternalRegisterFP, DoReadInternalRegisterLP, DoReadInternalRegisterSP, DoReadInternalRegisterIM, /* #o0154 */
    DoWriteInternalRegisterFP, DoWriteInternalRegisterLP, DoWriteInternalRegisterSP,
    DoWriteInternalRegisterIM, /* #o0155 */
    DoCoprocessorReadFP, DoCoprocessorReadLP, DoCoprocessorReadSP, DoCoprocessorReadIM, /* #o0156 */
    DoCoprocessorWriteFP, DoCoprocessorWriteLP, DoCoprocessorWriteSP, DoCoprocessorWriteIM, /* #o0157 */
    DoBlock0ReadAluFP, DoBlock0ReadAluLP, DoBlock0ReadAluSP, DoBlock0ReadAluIM, /* #o0160 */
    DoBlock1ReadAluFP, DoBlock1ReadAluLP, DoBlock1ReadAluSP, DoBlock1ReadAluIM, /* #o0161 */
    DoBlock2ReadAluFP, DoBlock2ReadAluLP, DoBlock2ReadAluSP, DoBlock2ReadAluIM, /* #o0162 */
    DoBlock3ReadAluFP, DoBlock3ReadAluLP, DoBlock3ReadAluSP, DoBlock3ReadAluIM, /* #o0163 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0164 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0165 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0166 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0167 */
    DoLdbFP, DoLdbLP, DoLdbSP, DoLdbIM, /* #o0170 */
    DoCharLdbFP, DoCharLdbLP, DoCharLdbSP, DoCharLdbIM, /* #o0171 */
    DoPLdbFP, DoPLdbLP, DoPLdbSP, DoPLdbIM, /* #o0172 */
    DoPTagLdbFP, DoPTagLdbLP, DoPTagLdbSP, DoPTagLdbIM, /* #o0173 */
    DoBranchFP, DoBranchLP, DoBranchSP, DoBranchIM, /* #o0174 */
    DoLoopDecrementTosFP, DoLoopDecrementTosLP, DoLoopDecrementTosSP, DoLoopDecrementTosIM, /* #o0175 */
    DoEntryRestAcceptedFP, DoEntryRestAcceptedLP, DoEntryRestAcceptedSP, DoEntryRestAcceptedIM, /* #o0176 */
    DoEntryRestNotAcceptedFP, DoEntryRestNotAcceptedLP, DoEntryRestNotAcceptedSP, DoEntryRestNotAcceptedIM, /* #o0177 */
    DoRplacaFP, DoRplacaLP, DoRplacaSP, DoRplacaIM, /* #o0200 */
    DoRplacdFP, DoRplacdLP, DoRplacdSP, DoRplacdIM, /* #o0201 */
    DoMultiplyFP, DoMultiplyLP, DoMultiplySP, DoMultiplyIM, /* #o0202 */
    DoQuotientFP, DoQuotientLP, DoQuotientSP, DoQuotientIM, /* #o0203 */
    DoCeilingFP, DoCeilingLP, DoCeilingSP, DoCeilingIM, /* #o0204 */
    DoFloorFP, DoFloorLP, DoFloorSP, DoFloorIM, /* #o0205 */
    DoTruncateFP, DoTruncateLP, DoTruncateSP, DoTruncateIM, /* #o0206 */
    DoRoundFP, DoRoundLP, DoRoundSP, DoRoundIM, /* #o0207 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o0210 +++ Use for DoRemainder */
    DoRationalQuotientFP, DoRationalQuotientLP, DoRationalQuotientSP, DoRationalQuotientIM, /* #o0211 */
    DoMinFP, DoMinLP, DoMinSP, DoMinIM, /* #o0212 */
    DoMaxFP, DoMaxLP, DoMaxSP, DoMaxIM, /* #o0213 */
    DoAluFP, DoAluLP, DoAluSP, DoAluIM, /* #o0214 */
    DoLogandFP, DoLogandLP, DoLogandSP, DoLogandIM, /* #o0215 */
    DoLogxorFP, DoLogxorLP, DoLogxorSP, DoLogxorIM, /* #o0216 */
    DoLogiorFP, DoLogiorLP, DoLogiorSP, DoLogiorIM, /* #o0217 */
    DoRotFP, DoRotLP, DoRotSP, DoRotIM, /* #o0220 */
    DoLshFP, DoLshLP, DoLshSP, DoLshIM, /* #o0221 */
    DoMultiplyDoubleFP, DoMultiplyDoubleLP, DoMultiplyDoubleSP, DoMultiplyDoubleIM, /* #o0222 */
    DoLshcBignumStepFP, DoLshcBignumStepLP, DoLshcBignumStepSP, DoLshcBignumStepIM, /* #o0223 */
    DoStackBltFP, DoStackBltLP, DoStackBltSP, DoStackBltIM, /* #o0224 */
    DoRgetfFP, DoRgetfLP, DoRgetfSP, DoRgetfIM, /* #o0225 */
    DoMemberFP, DoMemberLP, DoMemberSP, DoMemberIM, /* #o0226 */
    DoAssocFP, DoAssocLP, DoAssocSP, DoAssocIM, /* #o0227 */
    DoPointerPlusFP, DoPointerPlusLP, DoPointerPlusSP, DoPointerPlusIM, /* #o0230 */
    DoPointerDifferenceFP, DoPointerDifferenceLP, DoPointerDifferenceSP, DoPointerDifferenceIM, /* #o0231 */
    DoAshFP, DoAshLP, DoAshSP, DoAshIM, /* #o0232 */
    DoStoreConditionalFP, DoStoreConditionalLP, DoStoreConditionalSP, DoStoreConditionalIM, /* #o0233 */
    DoMemoryWriteFP, DoMemoryWriteLP, DoMemoryWriteSP, DoMemoryWriteIM, /* #o0234 */
    DoPStoreContentsFP, DoPStoreContentsLP, DoPStoreContentsSP, DoPStoreContentsIM, /* #o0235 */
    DoBindLocativeToValueFP, DoBindLocativeToValueLP, DoBindLocativeToValueSP, DoBindLocativeToValueIM, /* #o0236 */
    DoUnifyFP, DoUnifyLP, DoUnifySP, DoUnifyIM, /* #o0237 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0240 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0241 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0242 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0243 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0244 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0245 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0246 */
    DoPopLexicalVarNFP, DoPopLexicalVarNLP, DoPopLexicalVarNSP, DoPopLexicalVarNIM, /* #o0247 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0250 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0251 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0252 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0253 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0254 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0255 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0256 */
    DoMovemLexicalVarNFP, DoMovemLexicalVarNLP, DoMovemLexicalVarNSP, DoMovemLexicalVarNIM, /* #o0257 */
    DoEqualNumberFP, DoEqualNumberLP, DoEqualNumberSP, DoEqualNumberIM, /* #o0260 */
    DoLesspFP, DoLesspLP, DoLesspSP, DoLesspIM, /* #o0261 */
    DoGreaterpFP, DoGreaterpLP, DoGreaterpSP, DoGreaterpIM, /* #o0262 */
    DoEqlFP, DoEqlLP, DoEqlSP, DoEqlIM, /* #o0263 */
    DoEqualNumberFP, DoEqualNumberLP, DoEqualNumberSP, DoEqualNumberIM, /* #o0264 */
    DoLesspFP, DoLesspLP, DoLesspSP, DoLesspIM, /* #o0265 */
    DoGreaterpFP, DoGreaterpLP, DoGreaterpSP, DoGreaterpIM, /* #o0266 */
    DoEqlFP, DoEqlLP, DoEqlSP, DoEqlIM, /* #o0267 */
    DoEqFP, DoEqLP, DoEqSP, DoEqIM, /* #o0270 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o0271 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o0272 */
    DoLogtestFP, DoLogtestLP, DoLogtestSP, DoLogtestIM, /* #o0273 */
    DoEqFP, DoEqLP, DoEqSP, DoEqIM, /* #o0274 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o0275 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /* #o0276 */
    DoLogtestFP, DoLogtestLP, DoLogtestSP, DoLogtestIM, /* #o0277 */
    DoAddFP, DoAddLP, DoAddSP, DoAddIM, /* #o0300 */
    DoSubFP, DoSubLP, DoSubSP, DoSubIM, /* #o0301 */
    Do32BitPlusFP, Do32BitPlusLP, Do32BitPlusSP, Do32BitPlusIM, /* #o0302 */
    Do32BitDifferenceFP, Do32BitDifferenceLP, Do32BitDifferenceSP, Do32BitDifferenceIM, /* #o0303 */
    DoAddBignumStepFP, DoAddBignumStepLP, DoAddBignumStepSP, DoAddBignumStepIM, /* #o0304 */
    DoSubBignumStepFP, DoSubBignumStepLP, DoSubBignumStepSP, DoSubBignumStepIM, /* #o0305 */
    DoMultiplyBignumStepFP, DoMultiplyBignumStepLP, DoMultiplyBignumStepSP, DoMultiplyBignumStepIM, /* #o0306 */
    DoDivideBignumStepFP, DoDivideBignumStepLP, DoDivideBignumStepSP, DoDivideBignumStepIM, /* #o0307 */
    DoAset1FP, DoAset1LP, DoAset1SP, DoAset1IM, /* #o0310 */
    DoAllocateListBlockFP, DoAllocateListBlockLP, DoAllocateListBlockSP, DoAllocateListBlockIM, /* #o0311 */
    DoAref1FP, DoAref1LP, DoAref1SP, DoAref1IM, /* #o0312 */
    DoAloc1FP, DoAloc1LP, DoAloc1SP, DoAloc1IM, /* #o0313 */
    DoStoreArrayLeaderFP, DoStoreArrayLeaderLP, DoStoreArrayLeaderSP, DoStoreArrayLeaderIM, /* #o0314 */
    DoAllocateStructureBlockFP, DoAllocateStructureBlockLP, DoAllocateStructureBlockSP,
    DoAllocateStructureBlockIM, /* #o0315 */
    DoArrayLeaderFP, DoArrayLeaderLP, DoArrayLeaderSP, DoArrayLeaderIM, /* #o0316 */
    DoAlocLeaderFP, DoAlocLeaderLP, DoAlocLeaderSP, DoAlocLeaderIM, /* #o0317 */
    DoPopInstanceVariableFP, DoPopInstanceVariableLP, DoPopInstanceVariableSP, DoPopInstanceVariableIM, /* #o0320 */
    DoMovemInstanceVariableFP, DoMovemInstanceVariableLP, DoMovemInstanceVariableSP,
    DoMovemInstanceVariableIM, /* #o0321 */
    DoPopInstanceVariableOrderedFP, DoPopInstanceVariableOrderedLP, DoPopInstanceVariableOrderedSP,
    DoPopInstanceVariableOrderedIM, /* #o0322 */
    DoMovemInstanceVariableOrderedFP, DoMovemInstanceVariableOrderedLP, DoMovemInstanceVariableOrderedSP,
    DoMovemInstanceVariableOrderedIM, /* #o0323 */
    DoInstanceRefFP, DoInstanceRefLP, DoInstanceRefSP, DoInstanceRefIM, /* #o0324 */
    DoInstanceSetFP, DoInstanceSetLP, DoInstanceSetSP, DoInstanceSetIM, /* #o0325 */
    DoInstanceLocFP, DoInstanceLocLP, DoInstanceLocSP, DoInstanceLocIM, /* #o0326 */
    DoSetTagFP, DoSetTagLP, DoSetTagSP, DoSetTagIM, /* #o0327 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0330 */
    DoUnsignedLesspFP, DoUnsignedLesspLP, DoUnsignedLesspSP, DoUnsignedLesspIM, /* #o0331 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0332 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0333 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0334 */
    DoUnsignedLesspFP, DoUnsignedLesspLP, DoUnsignedLesspSP, DoUnsignedLesspIM, /* #o0335 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0336 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0337 */
    DoPopFP, DoPopLP, DoPopSP, DoPopIM, /* #o0340 */
    DoMovemFP, DoMovemLP, DoMovemSP, DoMovemIM, /* #o0341 */
    DoMergeCdrNoPopFP, DoMergeCdrNoPopLP, DoMergeCdrNoPopSP, DoMergeCdrNoPopIM, /* #o0342 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0343 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0344 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0345 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0346 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0347 */
    DoFastAref1FP, DoFastAref1LP, DoFastAref1SP, DoFastAref1IM, /* #o0350 */
    DoFastAset1FP, DoFastAset1LP, DoFastAset1SP, DoFastAset1IM, /* #o0351 */
    DoStackBltAddressFP, DoStackBltAddressLP, DoStackBltAddressSP, DoStackBltAddressIM, /* #o0352 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0353 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0354 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0355 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0356 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0357 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0360 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0361 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0362 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0363 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0364 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0365 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0366 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0367 */
    DoDpbFP, DoDpbLP, DoDpbSP, DoDpbIM, /* #o0370 */
    DoCharDpbFP, DoCharDpbLP, DoCharDpbSP, DoCharDpbIM, /* #o0371 */
    DoPDpbFP, DoPDpbLP, DoPDpbSP, DoPDpbIM, /* #o0372 */
    DoPTagDpbFP, DoPTagDpbLP, DoPTagDpbSP, DoPTagDpbIM, /* #o0373 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0374 */
    DoLoopIncrementTosLessThanFP, DoLoopIncrementTosLessThanLP, DoLoopIncrementTosLessThanSP,
    DoLoopIncrementTosLessThanIM, /* #o0375 */
    DoCatchOpenFP, DoCatchOpenLP, DoCatchOpenSP, DoCatchOpenIM, /* #o0376 */
    DoSpareOpFP, DoSpareOpLP, DoSpareOpSP, DoSpareOpIM, /*#o0377 */
};

DISPATCHTABLE(fullworddispatch, 48) = {
    nullfw, /* #o00 = DTP-NULL */
    monitorforwardfw, /* #o01 = DTP-MONITOR-FORWARD */
    headerpfw, /* #o02 = DTP-HEADER-P */
    headerifw, /* #o03 = DTP-HEADER-I */
    valuecell, /* #o04 = DTP-EXTERNAL-VALUE-CELL-POINTER */
    oneqforwardfw, /* #o05 = DTP-ONE-Q-FORWARD */
    headerforwardfw, /* #o06 = DTP-HEADER-FORWARD */
    elementforwardfw, /* #o07 = DTP-ELEMENT-FORWARD */
    pushconstantvalue, /* #o10 = DTP-FIXNUM */
    pushconstantvalue, /* #o11 = DTP-SMALL-RATIO */
    pushconstantvalue, /* #o12 = DTP-SINGLE-FLOAT */
    pushconstantvalue, /* #o13 = DTP-DOUBLE-FLOAT */
    pushconstantvalue, /* #o14 = DTP-BIGNUM */
    pushconstantvalue, /* #o15 = DTP-BIG-RATIO */
    pushconstantvalue, /* #o16 = DTP-COMPLEX */
    pushconstantvalue, /* #o17 = DTP-SPARE-NUMBER */
    pushconstantvalue, /* #o20 = DTP-INSTANCE */
    pushconstantvalue, /* #o21 = DTP-LIST-INSTANCE */
    pushconstantvalue, /* #o22 = DTP-ARRAY-INSTANCE */
    pushconstantvalue, /* #o23 = DTP-STRING-INSTANCE */
    pushconstantvalue, /* #o24 = DTP-NIL */
    pushconstantvalue, /* #o25 = DTP-LIST */
    pushconstantvalue, /* #o26 = DTP-ARRAY */
    pushconstantvalue, /* #o27 = DTP-STRING */
    pushconstantvalue, /* #o30 = DTP-SYMBOL */
    pushconstantvalue, /* #o31 = DTP-LOCATIVE */
    pushconstantvalue, /* #o32 = DTP-LEXICAL-CLOSURE */
    pushconstantvalue, /* #o33 = DTP-DYNAMIC-CLOSURE */
    pushconstantvalue, /* #o34 = DTP-COMPILED-FUNCTION */
    pushconstantvalue, /* #o35 = DTP-GENERIC-FUNCTION */
    pushconstantvalue, /* #o36 = DTP-SPARE-POINTER-1 */
    pushconstantvalue, /* #o37 = DTP-SPARE-POINTER-2 */
    pushconstantvalue, /* #o40 = DTP-PHYSICAL-ADDRESS */
    nativeinstruction, /* #o41 = DTP-SPARE-IMMEDIATE-1 *Hijacked for
                          nativeinstruction **/
    boundlocationfw, /* #o42 = DTP-BOUND-LOCATION */
    pushconstantvalue, /* #o43 = DTP-CHARACTER */
    logicvariablefw, /* #o44 = DTP-LOGIC-VARIABLE */
    gcforwardfw, /* #o45 = DTP-GC-FORWARD */
    pushconstantvalue, /* #o46 = DTP-EVEN-PC */
    pushconstantvalue, /* #o47 = DTP-ODD-PC */
    callcompiledeven, /* #o50 = DTP-CALL-COMPILED-EVEN */
    callcompiledodd, /* #o51 = DTP-CALL-COMPILED-ODD */
    callindirect, /* #o52 = DTP-CALL-INDIRECT */
    callgeneric, /* #o53 = DTP-CALL-GENERIC */
    callcompiledevenprefetch, /* #o54 = DTP-CALL-COMPILED-EVEN-PREFETCH
                               */
    callcompiledoddprefetch, /* #o55 = DTP-CALL-COMPILED-ODD-PREFETCH */
    callindirectprefetch, /* #o56 = DTP-CALL-INDIRECT-PREFETCH */
    callgenericprefetch /* #o57 = DTP-CALL-GENERIC-PREFETCH */
};

DISPATCHTABLE(internalregisterread1, 43) = {
    ReadRegisterError, /* ReadRegisterEA */
    ReadRegisterFP, ReadRegisterLP, ReadRegisterSP, ReadRegisterError, /* ReadRegisterMacroSP */
    ReadRegisterStackCacheLowerBound, ReadRegisterBARx, ReadRegisterError, /* ReadRegisterPHTHashx */
    ReadRegisterError, /* ReadRegisterEPC */
    ReadRegisterError, /* ReadRegisterDPC */
    ReadRegisterContinuation, ReadRegisterAluAndRotateControl, ReadRegisterControlRegister, ReadRegisterCRArgumentSize,
    ReadRegisterEphemeralOldspaceRegister, ReadRegisterZoneOldspaceRegister, ReadRegisterChipRevision,
    ReadRegisterFPCoprocessorPresent, ReadRegisterError, ReadRegisterPreemptRegister, ReadRegisterIcacheControl,
    ReadRegisterPrefetcherControl, ReadRegisterMapCacheControl, ReadRegisterMemoryControl,
    ReadRegisterError, /* ReadRegisterECCLog */
    ReadRegisterError, /* ReadRegisterECCLogAddress */
    ReadRegisterError, /* ReadRegisterInvalidateMapx */
    ReadRegisterError, /* ReadRegisterLoadMapx */
    ReadRegisterStackCacheOverflowLimit, ReadRegisterError, /* ReadRegisterUcodeROMContents */
    ReadRegisterError, ReadRegisterError, /* ReadRegisterAddressMask */
    ReadRegisterError, /* ReadRegisterEntryMaximumArguments */
    ReadRegisterError, /* ReadRegisterLexicalVariable */
    ReadRegisterError, /* ReadRegisterInstruction */
    ReadRegisterError, ReadRegisterError, /* ReadRegisterMemoryData */
    ReadRegisterError, /* ReadRegisterDataPins */
    ReadRegisterError, /* ReadRegisterExtensionRegister */
    ReadRegisterMicrosecondClock, ReadRegisterError, /* ReadRegisterArrayHeaderLength */
    ReadRegisterError, ReadRegisterError /* ReadRegisterLoadBAR */
};

DISPATCHTABLE(internalregisterread2, 34) = { ReadRegisterTOS, ReadRegisterEventCount, ReadRegisterBindingStackPointer,
    ReadRegisterCatchBlockList, ReadRegisterControlStackLimit, ReadRegisterControlStackExtraLimit,
    ReadRegisterBindingStackLimit, ReadRegisterPHTBase, ReadRegisterPHTMask, ReadRegisterCountMapReloads,
    ReadRegisterListCacheArea, ReadRegisterListCacheAddress, ReadRegisterListCacheLength,
    ReadRegisterStructureCacheArea, ReadRegisterStructureCacheAddress, ReadRegisterStructureCacheLength,
    ReadRegisterDynamicBindingCacheBase, ReadRegisterDynamicBindingCacheMask, ReadRegisterChoicePointer,
    ReadRegisterStructureStackChoicePointer, ReadRegisterFEPModeTrapVectorAddress, ReadRegisterError,
    ReadRegisterError, /* ReadRegisterMappingTableCache */
    ReadRegisterError, /* ReadRegisterMappingTableLength */
    ReadRegisterStackFrameMaximumSize, ReadRegisterStackCacheDumpQuantum, ReadRegisterError, ReadRegisterError,
    ReadRegisterError, ReadRegisterError, ReadRegisterError, ReadRegisterError, ReadRegisterConstantNIL,
    ReadRegisterConstantT };

DISPATCHTABLE(internalregisterwrite1, 43) = {
    WriteRegisterError, /* WriteRegisterEA */
    WriteRegisterFP, WriteRegisterLP, WriteRegisterSP, WriteRegisterError, /* WriteRegisterMacroSP */
    WriteRegisterStackCacheLowerBound, WriteRegisterBARx, WriteRegisterError, /* WriteRegisterPHTHashx */
    WriteRegisterError, /* WriteRegisterEPC */
    WriteRegisterError, /* WriteRegisterDPC */
    WriteRegisterContinuation, WriteRegisterAluAndRotateControl, WriteRegisterControlRegister,
    WriteRegisterError, /* WriteRegisterCRArgumentSize */
    WriteRegisterEphemeralOldspaceRegister, WriteRegisterZoneOldspaceRegister,
    WriteRegisterError, /* WriteRegisterChipRevision */
    WriteRegisterFPCoprocessorPresent, WriteRegisterError, WriteRegisterPreemptRegister,
    WriteRegisterError, /* WriteRegisterIcacheControl */
    WriteRegisterError, /* WriteRegisterPrefetcherControl */
    WriteRegisterError, /* WriteRegisterMapCacheControl */
    WriteRegisterError, /* WriteRegisterMemoryControl */
    WriteRegisterError, /* WriteRegisterECCLog */
    WriteRegisterError, /* WriteRegisterECCLogAddress */
    WriteRegisterError, /* WriteRegisterInvalidateMapx */
    WriteRegisterError, /* WriteRegisterLoadMapx */
    WriteRegisterStackCacheOverflowLimit, WriteRegisterError, /* WriteRegisterUcodeROMContents */
    WriteRegisterError, WriteRegisterError, /* WriteRegisterAddressMask */
    WriteRegisterError, /* WriteRegisterEntryMaximumArguments */
    WriteRegisterError, /* WriteRegisterLexicalVariable */
    WriteRegisterError, /* WriteRegisterInstruction */
    WriteRegisterError, WriteRegisterError, /* WriteRegisterMemoryData */
    WriteRegisterError, /* WriteRegisterDataPins */
    WriteRegisterError, /* WriteRegisterExtensionRegister */
    WriteRegisterError, /* WriteRegisterMicrosecondClock */
    WriteRegisterError, /* WriteRegisterArrayHeaderLength */
    WriteRegisterError, WriteRegisterError /* WriteRegisterLoadBAR */
};

DISPATCHTABLE(internalregisterwrite2, 34) = {
    WriteRegisterTOS, WriteRegisterEventCount, WriteRegisterBindingStackPointer, WriteRegisterCatchBlockList,
    WriteRegisterControlStackLimit, WriteRegisterControlStackExtraLimit, WriteRegisterBindingStackLimit,
    WriteRegisterError, /* WriteRegisterPHTBase */
    WriteRegisterError, /* WriteRegisterPHTMask */
    WriteRegisterError, /* WriteRegisterCountMapReloads */
    WriteRegisterListCacheArea, WriteRegisterListCacheAddress, WriteRegisterListCacheLength,
    WriteRegisterStructureCacheArea, WriteRegisterStructureCacheAddress, WriteRegisterStructureCacheLength,
    WriteRegisterDynamicBindingCacheBase, WriteRegisterDynamicBindingCacheMask, WriteRegisterChoicePointer,
    WriteRegisterStructureStackChoicePointer, WriteRegisterFEPModeTrapVectorAddress, WriteRegisterError,
    WriteRegisterMappingTableCache, WriteRegisterError, /* WriteRegisterMappingTableLength */
    WriteRegisterError, /* WriteRegisterStackFrameMaximumSize */
    WriteRegisterError, /* WriteRegisterStackCacheDumpQuantum */
    WriteRegisterError, WriteRegisterError, WriteRegisterError, WriteRegisterError, WriteRegisterError,
    WriteRegisterError, WriteRegisterError, /* WriteRegisterConstantNIL */
    WriteRegisterError /* WriteRegisterConstant */
};

// hack
// extern void ICACHEMISS(void);
extern void *ICACHEMISS;

#define FLUSHICACHE                                                                                                    \
    {                                                                                                                  \
        CACHELINEP cp = &instructioncache[-1];                                                                         \
        int i;                                                                                                         \
        for (i = 0; i < icachesize + 4; i++, cp++) {                                                                   \
            cp->code = (char *)/*&*/ ICACHEMISS;                                                                       \
            cp->nextcp = (char *)cp;                                                                                   \
        }                                                                                                              \
    }
#define FLUSHSTACKCACHE memset(stackcache, 0, stackcachesize * sizeof(LispObjRecord))

void flushicache(void)
{
    processor->cp = NULL;
    FLUSHICACHE;
}

#define ALPHAPAGESIZE 8192

void InitializeInstructionCache(void)
{
    if (instructioncache != NULL) {
        /* We have been here before, simple flush the icache that already
         * exists*/
        FLUSHICACHE;
    } else {
        // There are 4 extra cache lines allocated for the instruction cache
        // so that entries at the front and end of the cache don't have to
        // be patched up for pointing out of the cache.  Instead the extra
        // entries will either a) cause a cache-miss, due to the PC
        // mismatching, or b) force a cache miss, because their .code field
        // sends you there.  There is one spare line at the front of the cache
        // (for the backup case) and 3 spare lines at the end (for the forward
        // 2 case).  We align the cache on a page for better block fills. 
        caddr_t cp = (caddr_t)malloc((icachesize + 4) * sizeof(CACHELINE) + 2 * ALPHAPAGESIZE);

        if (!cp)
            vpunt(NULL, "Unable to allocate internal data structures");
        if (!(((uint64_t)cp) & (~(ALPHAPAGESIZE - 1)))) {
            /* if already aligned, put a blank page at front */
            cp += ALPHAPAGESIZE;
        } else {
            /* move up to page bound */
            cp = (caddr_t)(((uint64_t)cp + ALPHAPAGESIZE - 1) & (~(ALPHAPAGESIZE - 1)));
}

        /* we know there is at least 1 cacheline in front of us (as required
         * above) */
        instructioncache = (CACHELINEP)cp;
        FLUSHICACHE;
    }
}

void InitializeStatistics(void) {}

void InitializeIvoryInterpreterState(void) {}

void InitializeStackCache(void)
{
    if (stackcache != NULL) {
        /* The stackcache is already allocated, lets flush it */
        FLUSHSTACKCACHE;
    } else {
        /* --- we shouldn't get here any more since the cache is allocated
         * with the processor structure for better d-cache utilization
         */
        stackcache = (LispObjRecordp)malloc(stackcachesize * sizeof(LispObjRecord));
        if (!stackcache)
            vpunt(NULL, "Unable to allocate internal data structures");
    };
}

/* Fin */
