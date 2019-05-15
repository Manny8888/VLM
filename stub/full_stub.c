/*
 * OG "C" instruction stubs
 */

#define _GNU_SOURCE
#include <fenv.h>

#include "../std.h"

#include "aihead.h"
#include "ivoryrep.h"
#include "embed.h"
#include "traps.h"

#include "ivory.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef char s8;
typedef int s32;
typedef long s64;

#define MemoryActionIndirect 01
#define MemoryActionMonitor 02
#define MemoryActionTransport 04
#define MemoryActionTrap 010
#define MemoryActionTransform 020
#define MemoryActionBinding 040

#define CACHELINESIZE 48
#define TWOCACHELINESIZE (2 * CACHELINESIZE)
#define FOURCACHELINESIZE (4 * CACHELINESIZE)

#define AutoArrayRegMask 224
#define AutoArrayRegSize 32
#define AutoArrayRegShift 0

#define PROCESSORSTATE_DATAREAD -504
#define PROCESSORSTATE_DATAREAD_MASK -512

/*
t1      1       instn
t2      2       iword
t3      3       ecp
t4      4       ocp
t5      5       icsize
t6      6       epc
t7      7       opc
t8      8       count
iPC     9
iFP     10
iLP     11
iSP     12
iCP     13
ivory   14              ; ivory processor object
arg1    16
arg2    17
arg3    18
arg4    19
arg5    20      hwopmask
arg6    21      fwdispatch
t9      22      hwdispatch
t10     23
t11     24
t12     25
ra      r26
pv      r27
gp      r29
sp      r30

none            31
instn           1       ; = T1
iword           2       ; = T2
ecp             3       ; = T3
ocp             4       ; = T4
icsize          5       ; = T5 (icache size in bytes
epc             6       ; = T6
opc             7       ; = T7
count           8       ; = T8
hwopmask        20      ; = ARG5 (the halfword operand mask
fwdispatch      21      ; = ARG6 (the fullword dispatch table
hwdispatch      22      ; = T9 (the halfword dispatch table)
*/

////u64 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
// static u64 r0, instn, iword, ecp, ocp, icsize, epc, opc, count;

#define r1 instn
#define r2 iword
#define r3 ecp
#define r4 ocp
#define r5 icsize
#define r6 epc
#define r7 opc
#define r8 count
// static u64 r9, r10, r11, r12, r13, r14, r15;
// static u64 r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r29;
// static u64 sp;
#define r30 sp
// static u64 r31 = 0;

//#define zero 0
#define zero r31

#define t1 r1
#define t2 r2
#define t3 r3
#define t4 r4
#define t5 r5
#define t6 r6
#define t7 r7
#define t8 r8
#define iPC r9
#define iFP r10
#define iLP r11
#define iSP r12
#define iCP r13
#define arg1 r16
#define arg2 r17
#define arg3 r18
#define arg4 r19
#define arg5 r20
#define arg6 r21
#define t9 r22
#define t10 r23
#define t11 r24
#define t12 r25
#define ra r26
#define pv r27
#define gp r29
//#define sp    r30

//#define instn         r1
//#define iword         r2
//#define ecp           r3
//#define ocp           r4
//#define icsize        r5
//#define epc           r6
//#define opc           r7
//#define count         r8

#define hwopmask r20
#define fwdispatch r21
#define hwdispatch r22

#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A"(val))

static u64 old_rdtsc;

// these need to be in-line for DECODEFAULT to work
#define LDQ_U(ptr) *(u64 *)(ptr & ~7L)
#define STQ_U(ptr, v) *(u64 *)(ptr & ~7L) = v

static u64 f0, f1, f2, f3, f31;

#include "float.c"

u64 CMPBGE(u64 a, u64 b)
{
    u64 res = 0;
    u8 aa, bb;
    int i;

    //  printf("CMPBGE %p %p ", a, b);

    for (i = 0; i < 8; i++) {
        aa = a & 0xff;
        a >>= 8;
        bb = b & 0xff;
        b >>= 8;
        if (aa >= bb)
            res |= 1 << i;
    }

    //  printf("-> %p\n", res);
    return res;
}

#define CHECK_OFLO32(r)                                                                                                \
    if (((r)&0x8000000000000000) == 0 && ((r) >> 31)) {                                                                \
        printf("arithmeticexception; oflo32 file %s line %d\n", __FILE__, __LINE__);                                   \
        goto arithmeticexception;                                                                                      \
    }

#define CHECK_OFLO()                                                                                                   \
    if (oflo) {                                                                                                        \
        printf("arithmeticexception; file %s line %d\n", __FILE__, __LINE__);                                          \
        goto arithmeticexception;                                                                                      \
    }

int oflo;

void exception(int which, u64 r)
{
    if (r & 0x8000000000000000)
        return;
    printf("exception(%d, %p)!!!\n", which, r);
}

char *halfwordnames[256 * 4] = {
    "DoCarFP", "DoCarLP", "DoCarSP", "DoCarIM", /* #o00 */
    "DoCdrFP", "DoCdrLP", "DoCdrSP", "DoCdrIM", /* #o01 */
    "DoEndpFP", "DoEndpLP", "DoEndpSP", "DoEndpIM", /* #o02 */
    "DoSetup1DArrayFP", "DoSetup1DArrayLP", "DoSetup1DArraySP", "DoSetup1DArrayIM", /* #o03 */
    "DoSetupForce1DArrayFP", "DoSetupForce1DArrayLP", "DoSetupForce1DArraySP", "DoSetupForce1DArrayIM", /* #o04 */
    "DoBindLocativeFP", "DoBindLocativeLP", "DoBindLocativeSP", "DoBindLocativeIM", /* #o05 */
    "DoRestoreBindingStackFP", "DoRestoreBindingStackLP", "DoRestoreBindingStackSP",
    "DoRestoreBindingStackIM", /* #o06 */
    "DoEphemeralpFP", "DoEphemeralpLP", "DoEphemeralpSP", "DoEphemeralpIM", /* #o07 */
    "DoStartCallFP", "DoStartCallLP", "DoStartCallSP", "DoStartCallIM", /* #o010 */
    "DoJumpFP", "DoJumpLP", "DoJumpSP", "DoJumpIM", /* #o011 */
    "DoTagFP", "DoTagLP", "DoTagSP", "DoTagIM", /* #o012 */
    "DoDereferenceFP", "DoDereferenceLP", "DoDereferenceSP", "DoDereferenceIM", /* #o013 */
    "DoLogicTailTestFP", "DoLogicTailTestLP", "DoLogicTailTestSP", "DoLogicTailTestIM", /* #o014 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /* #o015 +++ Used for breakpoints!!! */
    "DoDoubleFloatOpFP", "DoDoubleFloatOpLP", "DoDoubleFloatOpSP", "DoDoubleFloatOpIM", /* #o016 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /* #o017 */
    "DoPushLexicalVar0FP", "DoPushLexicalVar0LP", "DoPushLexicalVar0SP", "DoPushLexicalVar0IM", /* #o020 */
    "DoPushLexicalVar1FP", "DoPushLexicalVar1LP", "DoPushLexicalVar1SP", "DoPushLexicalVar1IM", /* #o021 */
    "DoPushLexicalVar2FP", "DoPushLexicalVar2LP", "DoPushLexicalVar2SP", "DoPushLexicalVar2IM", /* #o022 */
    "DoPushLexicalVar3FP", "DoPushLexicalVar3LP", "DoPushLexicalVar3SP", "DoPushLexicalVar3IM", /* #o023 */
    "DoPushLexicalVar4FP", "DoPushLexicalVar4LP", "DoPushLexicalVar4SP", "DoPushLexicalVar4IM", /* #o024 */
    "DoPushLexicalVar5FP", "DoPushLexicalVar5LP", "DoPushLexicalVar5SP", "DoPushLexicalVar5IM", /* #o025 */
    "DoPushLexicalVar6FP", "DoPushLexicalVar6LP", "DoPushLexicalVar6SP", "DoPushLexicalVar6IM", /* #o026 */
    "DoPushLexicalVar7FP", "DoPushLexicalVar7LP", "DoPushLexicalVar7SP", "DoPushLexicalVar7IM", /* #o027 */
    "DoBlock0WriteFP", "DoBlock0WriteLP", "DoBlock0WriteSP", "DoBlock0WriteIM", /* #o030 */
    "DoBlock1WriteFP", "DoBlock1WriteLP", "DoBlock1WriteSP", "DoBlock1WriteIM", /* #o031 */
    "DoBlock2WriteFP", "DoBlock2WriteLP", "DoBlock2WriteSP", "DoBlock2WriteIM", /* #o032 */
    "DoBlock3WriteFP", "DoBlock3WriteLP", "DoBlock3WriteSP", "DoBlock3WriteIM", /* #o033 */
    "DoZeropFP", "DoZeropLP", "DoZeropSP", "DoZeropIM", /* #o034 */
    "DoMinuspFP", "DoMinuspLP", "DoMinuspSP", "DoMinuspIM", /* #o035 */
    "DoPluspFP", "DoPluspLP", "DoPluspSP", "DoPluspIM", /* #o036 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o037 */
    "DoTypeMember0FP", "DoTypeMember0LP", "DoTypeMember0SP", "DoTypeMember0IM", /* #o040 */
    "DoTypeMember1FP", "DoTypeMember1LP", "DoTypeMember1SP", "DoTypeMember1IM", /* #o041 */
    "DoTypeMember2FP", "DoTypeMember2LP", "DoTypeMember2SP", "DoTypeMember2IM", /* #o042 */
    "DoTypeMember3FP", "DoTypeMember3LP", "DoTypeMember3SP", "DoTypeMember3IM", /* #o043 */
    "DoTypeMemberNoPop0FP", "DoTypeMemberNoPop0LP", "DoTypeMemberNoPop0SP", "DoTypeMemberNoPop0IM", /* #o044 */
    "DoTypeMemberNoPop1FP", "DoTypeMemberNoPop1LP", "DoTypeMemberNoPop1SP", "DoTypeMemberNoPop1IM", /* #o045 */
    "DoTypeMemberNoPop2FP", "DoTypeMemberNoPop2LP", "DoTypeMemberNoPop2SP", "DoTypeMemberNoPop2IM", /* #o046 */
    "DoTypeMemberNoPop3FP", "DoTypeMemberNoPop3LP", "DoTypeMemberNoPop3SP", "DoTypeMemberNoPop3IM", /* #o047 */
    "DoLocateLocalsFP", "DoLocateLocalsLP", "DoLocateLocalsSP", "DoLocateLocalsIM", /* #o050 */
    "DoCatchCloseFP", "DoCatchCloseLP", "DoCatchCloseSP", "DoCatchCloseIM", /* #o051 */
    "DoGenericDispatchFP", "DoGenericDispatchLP", "DoGenericDispatchSP", "DoGenericDispatchIM", /* #o052 */
    "DoMessageDispatchFP", "DoMessageDispatchLP", "DoMessageDispatchSP", "DoMessageDispatchIM", /* #o053 */
    "DoCheckPreemptRequestFP", "DoCheckPreemptRequestLP", "DoCheckPreemptRequestSP",
    "DoCheckPreemptRequestIM", /* #o054 */
    "DoPushGlobalLogicVariableFP", "DoPushGlobalLogicVariableLP", "DoPushGlobalLogicVariableSP",
    "DoPushGlobalLogicVariableIM", /* #o055 */
    "DoNoOpFP", "DoNoOpLP", "DoNoOpSP", "DoNoOpIM", /* #o056 */
    "DoHaltFP", "DoHaltLP", "DoHaltSP", "DoHaltIM", /* #o057 */
    "DoBranchTrueFP", "DoBranchTrueLP", "DoBranchTrueSP", "DoBranchTrueIM", /* #o060 */
    "DoBranchTrueElseExtraPopFP", "DoBranchTrueElseExtraPopLP", "DoBranchTrueElseExtraPopSP",
    "DoBranchTrueElseExtraPopIM", /* #o061 */
    "DoBranchTrueAndExtraPopFP", "DoBranchTrueAndExtraPopLP", "DoBranchTrueAndExtraPopSP",
    "DoBranchTrueAndExtraPopIM", /* #o062 */
    "DoBranchTrueExtraPopFP", "DoBranchTrueExtraPopLP", "DoBranchTrueExtraPopSP", "DoBranchTrueExtraPopIM", /* #o063 */
    "DoBranchTrueNoPopFP", "DoBranchTrueNoPopLP", "DoBranchTrueNoPopSP", "DoBranchTrueNoPopIM", /* #o064 */
    "DoBranchTrueAndNoPopFP", "DoBranchTrueAndNoPopLP", "DoBranchTrueAndNoPopSP", "DoBranchTrueAndNoPopIM", /* #o065 */
    "DoBranchTrueElseNoPopFP", "DoBranchTrueElseNoPopLP", "DoBranchTrueElseNoPopSP",
    "DoBranchTrueElseNoPopIM", /* #o066 */
    "DoBranchTrueAndNoPopElseNoPopExtraPopFP", "DoBranchTrueAndNoPopElseNoPopExtraPopLP",
    "DoBranchTrueAndNoPopElseNoPopExtraPopSP", "DoBranchTrueAndNoPopElseNoPopExtraPopIM", /* #o067 */
    "DoBranchFalseFP", "DoBranchFalseLP", "DoBranchFalseSP", "DoBranchFalseIM", /* #o070 */
    "DoBranchFalseElseExtraPopFP", "DoBranchFalseElseExtraPopLP", "DoBranchFalseElseExtraPopSP",
    "DoBranchFalseElseExtraPopIM", /* #o071 */
    "DoBranchFalseAndExtraPopFP", "DoBranchFalseAndExtraPopLP", "DoBranchFalseAndExtraPopSP",
    "DoBranchFalseAndExtraPopIM", /* #o072 */
    "DoBranchFalseExtraPopFP", "DoBranchFalseExtraPopLP", "DoBranchFalseExtraPopSP",
    "DoBranchFalseExtraPopIM", /* #o073 */
    "DoBranchFalseNoPopFP", "DoBranchFalseNoPopLP", "DoBranchFalseNoPopSP", "DoBranchFalseNoPopIM", /* #o074 */
    "DoBranchFalseAndNoPopFP", "DoBranchFalseAndNoPopLP", "DoBranchFalseAndNoPopSP",
    "DoBranchFalseAndNoPopIM", /* #o075 */
    "DoBranchFalseElseNoPopFP", "DoBranchFalseElseNoPopLP", "DoBranchFalseElseNoPopSP",
    "DoBranchFalseElseNoPopIM", /* #o076 */
    "DoBranchFalseAndNoPopElseNoPopExtraPopFP", "DoBranchFalseAndNoPopElseNoPopExtraPopLP",
    "DoBranchFalseAndNoPopElseNoPopExtraPopSP", "DoBranchFalseAndNoPopElseNoPopExtraPopIM", /* #o077 */
    "DoPushFP", "DoPushLP", "DoPushSP", "DoPushIM", /* #o0100 */
    "DoPushNNilsFP", "DoPushNNilsLP", "DoPushNNilsSP", "DoPushNNilsIM", /* #o0101 */
    "DoPushAddressSpRelativeFP", "DoPushAddressSpRelativeLP", "DoPushAddressSpRelativeSP",
    "DoPushAddressSpRelativeIM", /* #o0102 */
    "DoPushLocalLogicVariablesFP", "DoPushLocalLogicVariablesLP", "DoPushLocalLogicVariablesSP",
    "DoPushLocalLogicVariablesIM", /* #o0103 */
    "DoReturnMultipleFP", "DoReturnMultipleLP", "DoReturnMultipleSP", "DoReturnMultipleIM", /* #o0104 */
    "DoReturnKludgeFP", "DoReturnKludgeLP", "DoReturnKludgeSP", "DoReturnKludgeIM", /* #o0105 */
    "DoTakeValuesFP", "DoTakeValuesLP", "DoTakeValuesSP", "DoTakeValuesIM", /* #o0106 */
    "DoUnbindNFP", "DoUnbindNLP", "DoUnbindNSP", "DoUnbindNIM", /* #o0107 */
    "DoPushInstanceVariableFP", "DoPushInstanceVariableLP", "DoPushInstanceVariableSP",
    "DoPushInstanceVariableIM", /* #o0110 */
    "DoPushAddressInstanceVariableFP", "DoPushAddressInstanceVariableLP", "DoPushAddressInstanceVariableSP",
    "DoPushAddressInstanceVariableIM", /* #o0111 */
    "DoPushInstanceVariableOrderedFP", "DoPushInstanceVariableOrderedLP", "DoPushInstanceVariableOrderedSP",
    "DoPushInstanceVariableOrderedIM", /* #o0112 */
    "DoPushAddressInstanceVariableOrderedFP", "DoPushAddressInstanceVariableOrderedLP",
    "DoPushAddressInstanceVariableOrderedSP", "DoPushAddressInstanceVariableOrderedIM", /* #o0113 */
    "DoUnaryMinusFP", "DoUnaryMinusLP", "DoUnaryMinusSP", "DoUnaryMinusIM", /* #o0114 */
    "DoReturnSingleFP", "DoReturnSingleLP", "DoReturnSingleSP", "DoReturnSingleIM", /* #o0115 */
    "DoMemoryReadFP", "DoMemoryReadLP", "DoMemoryReadSP", "DoMemoryReadIM", /* #o0116 */
    "DoMemoryReadAddressFP", "DoMemoryReadAddressLP", "DoMemoryReadAddressSP", "DoMemoryReadAddressIM", /* #o0117 */
    "DoBlock0ReadFP", "DoBlock0ReadLP", "DoBlock0ReadSP", "DoBlock0ReadIM", /* #o0120 */
    "DoBlock1ReadFP", "DoBlock1ReadLP", "DoBlock1ReadSP", "DoBlock1ReadIM", /* #o0121 */
    "DoBlock2ReadFP", "DoBlock2ReadLP", "DoBlock2ReadSP", "DoBlock2ReadIM", /* #o0122 */
    "DoBlock3ReadFP", "DoBlock3ReadLP", "DoBlock3ReadSP", "DoBlock3ReadIM", /* #o0123 */
    "DoBlock0ReadShiftFP", "DoBlock0ReadShiftLP", "DoBlock0ReadShiftSP", "DoBlock0ReadShiftIM", /* #o0124 */
    "DoBlock1ReadShiftFP", "DoBlock1ReadShiftLP", "DoBlock1ReadShiftSP", "DoBlock1ReadShiftIM", /* #o0125 */
    "DoBlock2ReadShiftFP", "DoBlock2ReadShiftLP", "DoBlock2ReadShiftSP", "DoBlock2ReadShiftIM", /* #o0126 */
    "DoBlock3ReadShiftFP", "DoBlock3ReadShiftLP", "DoBlock3ReadShiftSP", "DoBlock3ReadShiftIM", /* #o0127 */
    "DoBlock0ReadTestFP", "DoBlock0ReadTestLP", "DoBlock0ReadTestSP", "DoBlock0ReadTestIM", /* #o0130 */
    "DoBlock1ReadTestFP", "DoBlock1ReadTestLP", "DoBlock1ReadTestSP", "DoBlock1ReadTestIM", /* #o0131 */
    "DoBlock2ReadTestFP", "DoBlock2ReadTestLP", "DoBlock2ReadTestSP", "DoBlock2ReadTestIM", /* #o0132 */
    "DoBlock3ReadTestFP", "DoBlock3ReadTestLP", "DoBlock3ReadTestSP", "DoBlock3ReadTestIM", /* #o0133 */
    "DoFinishCallNFP", "DoFinishCallNLP", "DoFinishCallNSP", "DoFinishCallNIM", /* #o0134 */
    "DoFinishCallNApplyFP", "DoFinishCallNApplyLP", "DoFinishCallNApplySP", "DoFinishCallNApplyIM", /* #o0135 */
    "DoFinishCallTosFP", "DoFinishCallTosLP", "DoFinishCallTosSP", "DoFinishCallTosIM", /* #o0136 */
    "DoFinishCallTosApplyFP", "DoFinishCallTosApplyLP", "DoFinishCallTosApplySP", "DoFinishCallTosApplyIM", /* #o0137 */
    "DoSetToCarFP", "DoSetToCarLP", "DoSetToCarSP", "DoSetToCarIM", /* #o0140 */
    "DoSetToCdrFP", "DoSetToCdrLP", "DoSetToCdrSP", "DoSetToCdrIM", /* #o0141 */
    "DoSetToCdrPushCarFP", "DoSetToCdrPushCarLP", "DoSetToCdrPushCarSP", "DoSetToCdrPushCarIM", /* #o0142 */
    "DoIncrementFP", "DoIncrementLP", "DoIncrementSP", "DoIncrementIM", /* #o0143 */
    "DoDecrementFP", "DoDecrementLP", "DoDecrementSP", "DoDecrementIM", /* #o0144 */
    "DoPointerIncrementFP", "DoPointerIncrementLP", "DoPointerIncrementSP", "DoPointerIncrementIM", /* #o0145 */
    "DoSetCdrCode1FP", "DoSetCdrCode1LP", "DoSetCdrCode1SP", "DoSetCdrCode1IM", /* #o0146 */
    "DoSetCdrCode2FP", "DoSetCdrCode2LP", "DoSetCdrCode2SP", "DoSetCdrCode2IM", /* #o0147 */
    "DoPushAddressFP", "DoPushAddressLP", "DoPushAddressSP", "DoPushAddressIM", /* #o0150 */
    "DoSetSpToAddressFP", "DoSetSpToAddressLP", "DoSetSpToAddressSP", "DoSetSpToAddressIM", /* #o0151 */
    "DoSetSpToAddressSaveTosFP", "DoSetSpToAddressSaveTosLP", "DoSetSpToAddressSaveTosSP",
    "DoSetSpToAddressSaveTosIM", /* #o0152 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0153 */
    "DoReadInternalRegisterFP", "DoReadInternalRegisterLP", "DoReadInternalRegisterSP",
    "DoReadInternalRegisterIM", /* #o0154 */
    "DoWriteInternalRegisterFP", "DoWriteInternalRegisterLP", "DoWriteInternalRegisterSP",
    "DoWriteInternalRegisterIM", /* #o0155 */
    "DoCoprocessorReadFP", "DoCoprocessorReadLP", "DoCoprocessorReadSP", "DoCoprocessorReadIM", /* #o0156 */
    "DoCoprocessorWriteFP", "DoCoprocessorWriteLP", "DoCoprocessorWriteSP", "DoCoprocessorWriteIM", /* #o0157 */
    "DoBlock0ReadAluFP", "DoBlock0ReadAluLP", "DoBlock0ReadAluSP", "DoBlock0ReadAluIM", /* #o0160 */
    "DoBlock1ReadAluFP", "DoBlock1ReadAluLP", "DoBlock1ReadAluSP", "DoBlock1ReadAluIM", /* #o0161 */
    "DoBlock2ReadAluFP", "DoBlock2ReadAluLP", "DoBlock2ReadAluSP", "DoBlock2ReadAluIM", /* #o0162 */
    "DoBlock3ReadAluFP", "DoBlock3ReadAluLP", "DoBlock3ReadAluSP", "DoBlock3ReadAluIM", /* #o0163 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0164 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0165 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0166 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0167 */
    "DoLdbFP", "DoLdbLP", "DoLdbSP", "DoLdbIM", /* #o0170 */
    "DoCharLdbFP", "DoCharLdbLP", "DoCharLdbSP", "DoCharLdbIM", /* #o0171 */
    "DoPLdbFP", "DoPLdbLP", "DoPLdbSP", "DoPLdbIM", /* #o0172 */
    "DoPTagLdbFP", "DoPTagLdbLP", "DoPTagLdbSP", "DoPTagLdbIM", /* #o0173 */
    "DoBranchFP", "DoBranchLP", "DoBranchSP", "DoBranchIM", /* #o0174 */
    "DoLoopDecrementTosFP", "DoLoopDecrementTosLP", "DoLoopDecrementTosSP", "DoLoopDecrementTosIM", /* #o0175 */
    "DoEntryRestAcceptedFP", "DoEntryRestAcceptedLP", "DoEntryRestAcceptedSP", "DoEntryRestAcceptedIM", /* #o0176 */
    "DoEntryRestNotAcceptedFP", "DoEntryRestNotAcceptedLP", "DoEntryRestNotAcceptedSP",
    "DoEntryRestNotAcceptedIM", /* #o0177 */
    "DoRplacaFP", "DoRplacaLP", "DoRplacaSP", "DoRplacaIM", /* #o0200 */
    "DoRplacdFP", "DoRplacdLP", "DoRplacdSP", "DoRplacdIM", /* #o0201 */
    "DoMultiplyFP", "DoMultiplyLP", "DoMultiplySP", "DoMultiplyIM", /* #o0202 */
    "DoQuotientFP", "DoQuotientLP", "DoQuotientSP", "DoQuotientIM", /* #o0203 */
    "DoCeilingFP", "DoCeilingLP", "DoCeilingSP", "DoCeilingIM", /* #o0204 */
    "DoFloorFP", "DoFloorLP", "DoFloorSP", "DoFloorIM", /* #o0205 */
    "DoTruncateFP", "DoTruncateLP", "DoTruncateSP", "DoTruncateIM", /* #o0206 */
    "DoRoundFP", "DoRoundLP", "DoRoundSP", "DoRoundIM", /* #o0207 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0210 */
    "DoRationalQuotientFP", "DoRationalQuotientLP", "DoRationalQuotientSP", "DoRationalQuotientIM", /* #o0211 */
    "DoMinFP", "DoMinLP", "DoMinSP", "DoMinIM", /* #o0212 */
    "DoMaxFP", "DoMaxLP", "DoMaxSP", "DoMaxIM", /* #o0213 */
    "DoAluFP", "DoAluLP", "DoAluSP", "DoAluIM", /* #o0214 */
    "DoLogandFP", "DoLogandLP", "DoLogandSP", "DoLogandIM", /* #o0215 */
    "DoLogxorFP", "DoLogxorLP", "DoLogxorSP", "DoLogxorIM", /* #o0216 */
    "DoLogiorFP", "DoLogiorLP", "DoLogiorSP", "DoLogiorIM", /* #o0217 */
    "DoRotFP", "DoRotLP", "DoRotSP", "DoRotIM", /* #o0220 */
    "DoLshFP", "DoLshLP", "DoLshSP", "DoLshIM", /* #o0221 */
    "DoMultiplyDoubleFP", "DoMultiplyDoubleLP", "DoMultiplyDoubleSP", "DoMultiplyDoubleIM", /* #o0222 */
    "DoLshcBignumStepFP", "DoLshcBignumStepLP", "DoLshcBignumStepSP", "DoLshcBignumStepIM", /* #o0223 */
    "DoStackBltFP", "DoStackBltLP", "DoStackBltSP", "DoStackBltIM", /* #o0224 */
    "DoRgetfFP", "DoRgetfLP", "DoRgetfSP", "DoRgetfIM", /* #o0225 */
    "DoMemberFP", "DoMemberLP", "DoMemberSP", "DoMemberIM", /* #o0226 */
    "DoAssocFP", "DoAssocLP", "DoAssocSP", "DoAssocIM", /* #o0227 */
    "DoPointerPlusFP", "DoPointerPlusLP", "DoPointerPlusSP", "DoPointerPlusIM", /* #o0230 */
    "DoPointerDifferenceFP", "DoPointerDifferenceLP", "DoPointerDifferenceSP", "DoPointerDifferenceIM", /* #o0231 */
    "DoAshFP", "DoAshLP", "DoAshSP", "DoAshIM", /* #o0232 */
    "DoStoreConditionalFP", "DoStoreConditionalLP", "DoStoreConditionalSP", "DoStoreConditionalIM", /* #o0233 */
    "DoMemoryWriteFP", "DoMemoryWriteLP", "DoMemoryWriteSP", "DoMemoryWriteIM", /* #o0234 */
    "DoPStoreContentsFP", "DoPStoreContentsLP", "DoPStoreContentsSP", "DoPStoreContentsIM", /* #o0235 */
    "DoBindLocativeToValueFP", "DoBindLocativeToValueLP", "DoBindLocativeToValueSP",
    "DoBindLocativeToValueIM", /* #o0236 */
    "DoUnifyFP", "DoUnifyLP", "DoUnifySP", "DoUnifyIM", /* #o0237 */
    "DoPopLexicalVar0FP", "DoPopLexicalVar0LP", "DoPopLexicalVar0SP", "DoPopLexicalVar0IM", /* #o0240 */
    "DoPopLexicalVar1FP", "DoPopLexicalVar1LP", "DoPopLexicalVar1SP", "DoPopLexicalVar1IM", /* #o0241 */
    "DoPopLexicalVar2FP", "DoPopLexicalVar2LP", "DoPopLexicalVar2SP", "DoPopLexicalVar2IM", /* #o0242 */
    "DoPopLexicalVar3FP", "DoPopLexicalVar3LP", "DoPopLexicalVar3SP", "DoPopLexicalVar3IM", /* #o0243 */
    "DoPopLexicalVar4FP", "DoPopLexicalVar4LP", "DoPopLexicalVar4SP", "DoPopLexicalVar4IM", /* #o0244 */
    "DoPopLexicalVar5FP", "DoPopLexicalVar5LP", "DoPopLexicalVar5SP", "DoPopLexicalVar5IM", /* #o0245 */
    "DoPopLexicalVar6FP", "DoPopLexicalVar6LP", "DoPopLexicalVar6SP", "DoPopLexicalVar6IM", /* #o0246 */
    "DoPopLexicalVar7FP", "DoPopLexicalVar7LP", "DoPopLexicalVar7SP", "DoPopLexicalVar7IM", /* #o0247 */
    "DoMovemLexicalVar0FP", "DoMovemLexicalVar0LP", "DoMovemLexicalVar0SP", "DoMovemLexicalVar0IM", /* #o0250 */
    "DoMovemLexicalVar1FP", "DoMovemLexicalVar1LP", "DoMovemLexicalVar1SP", "DoMovemLexicalVar1IM", /* #o0251 */
    "DoMovemLexicalVar2FP", "DoMovemLexicalVar2LP", "DoMovemLexicalVar2SP", "DoMovemLexicalVar2IM", /* #o0252 */
    "DoMovemLexicalVar3FP", "DoMovemLexicalVar3LP", "DoMovemLexicalVar3SP", "DoMovemLexicalVar3IM", /* #o0253 */
    "DoMovemLexicalVar4FP", "DoMovemLexicalVar4LP", "DoMovemLexicalVar4SP", "DoMovemLexicalVar4IM", /* #o0254 */
    "DoMovemLexicalVar5FP", "DoMovemLexicalVar5LP", "DoMovemLexicalVar5SP", "DoMovemLexicalVar5IM", /* #o0255 */
    "DoMovemLexicalVar6FP", "DoMovemLexicalVar6LP", "DoMovemLexicalVar6SP", "DoMovemLexicalVar6IM", /* #o0256 */
    "DoMovemLexicalVar7FP", "DoMovemLexicalVar7LP", "DoMovemLexicalVar7SP", "DoMovemLexicalVar7IM", /* #o0257 */
    "DoEqualNumberFP", "DoEqualNumberLP", "DoEqualNumberSP", "DoEqualNumberIM", /* #o0260 */
    "DoLesspFP", "DoLesspLP", "DoLesspSP", "DoLesspIM", /* #o0261 */
    "DoGreaterpFP", "DoGreaterpLP", "DoGreaterpSP", "DoGreaterpIM", /* #o0262 */
    "DoEqlFP", "DoEqlLP", "DoEqlSP", "DoEqlIM", /* #o0263 */
    "DoEqualNumberNoPopFP", "DoEqualNumberNoPopLP", "DoEqualNumberNoPopSP", "DoEqualNumberNoPopIM", /* #o0264 */
    "DoLesspNoPopFP", "DoLesspNoPopLP", "DoLesspNoPopSP", "DoLesspNoPopIM", /* #o0265 */
    "DoGreaterpNoPopFP", "DoGreaterpNoPopLP", "DoGreaterpNoPopSP", "DoGreaterpNoPopIM", /* #o0266 */
    "DoEqlNoPopFP", "DoEqlNoPopLP", "DoEqlNoPopSP", "DoEqlNoPopIM", /* #o0267 */
    "DoEqFP", "DoEqLP", "DoEqSP", "DoEqIM", /* #o0270 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0271 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0272 */
    "DoLogtestFP", "DoLogtestLP", "DoLogtestSP", "DoLogtestIM", /* #o0273 */
    "DoEqNoPopFP", "DoEqNoPopLP", "DoEqNoPopSP", "DoEqNoPopIM", /* #o0274 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0275 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0276 */
    "DoLogtestNoPopFP", "DoLogtestNoPopLP", "DoLogtestNoPopSP", "DoLogtestNoPopIM", /* #o0277 */
    "DoAddFP", "DoAddLP", "DoAddSP", "DoAddIM", /* #o0300 */
    "DoSubFP", "DoSubLP", "DoSubSP", "DoSubIM", /* #o0301 */
    "Do32BitPlusFP", "Do32BitPlusLP", "Do32BitPlusSP", "Do32BitPlusIM", /* #o0302 */
    "Do32BitDifferenceFP", "Do32BitDifferenceLP", "Do32BitDifferenceSP", "Do32BitDifferenceIM", /* #o0303 */
    "DoAddBignumStepFP", "DoAddBignumStepLP", "DoAddBignumStepSP", "DoAddBignumStepIM", /* #o0304 */
    "DoSubBignumStepFP", "DoSubBignumStepLP", "DoSubBignumStepSP", "DoSubBignumStepIM", /* #o0305 */
    "DoMultiplyBignumStepFP", "DoMultiplyBignumStepLP", "DoMultiplyBignumStepSP", "DoMultiplyBignumStepIM", /* #o0306 */
    "DoDivideBignumStepFP", "DoDivideBignumStepLP", "DoDivideBignumStepSP", "DoDivideBignumStepIM", /* #o0307 */
    "DoAset1FP", "DoAset1LP", "DoAset1SP", "DoAset1IM", /* #o0310 */
    "DoAllocateListBlockFP", "DoAllocateListBlockLP", "DoAllocateListBlockSP", "DoAllocateListBlockIM", /* #o0311 */
    "DoAref1FP", "DoAref1LP", "DoAref1SP", "DoAref1IM", /* #o0312 */
    "DoAloc1FP", "DoAloc1LP", "DoAloc1SP", "DoAloc1IM", /* #o0313 */
    "DoStoreArrayLeaderFP", "DoStoreArrayLeaderLP", "DoStoreArrayLeaderSP", "DoStoreArrayLeaderIM", /* #o0314 */
    "DoAllocateStructureBlockFP", "DoAllocateStructureBlockLP", "DoAllocateStructureBlockSP",
    "DoAllocateStructureBlockIM", /* #o0315 */
    "DoArrayLeaderFP", "DoArrayLeaderLP", "DoArrayLeaderSP", "DoArrayLeaderIM", /* #o0316 */
    "DoAlocLeaderFP", "DoAlocLeaderLP", "DoAlocLeaderSP", "DoAlocLeaderIM", /* #o0317 */
    "DoPopInstanceVariableFP", "DoPopInstanceVariableLP", "DoPopInstanceVariableSP",
    "DoPopInstanceVariableIM", /* #o0320 */
    "DoMovemInstanceVariableFP", "DoMovemInstanceVariableLP", "DoMovemInstanceVariableSP",
    "DoMovemInstanceVariableIM", /* #o0321 */
    "DoPopInstanceVariableOrderedFP", "DoPopInstanceVariableOrderedLP", "DoPopInstanceVariableOrderedSP",
    "DoPopInstanceVariableOrderedIM", /* #o0322 */
    "DoMovemInstanceVariableOrderedFP", "DoMovemInstanceVariableOrderedLP", "DoMovemInstanceVariableOrderedSP",
    "DoMovemInstanceVariableOrderedIM", /* #o0323 */
    "DoInstanceRefFP", "DoInstanceRefLP", "DoInstanceRefSP", "DoInstanceRefIM", /* #o0324 */
    "DoInstanceSetFP", "DoInstanceSetLP", "DoInstanceSetSP", "DoInstanceSetIM", /* #o0325 */
    "DoInstanceLocFP", "DoInstanceLocLP", "DoInstanceLocSP", "DoInstanceLocIM", /* #o0326 */
    "DoSetTagFP", "DoSetTagLP", "DoSetTagSP", "DoSetTagIM", /* #o0327 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0330 */
    "DoUnsignedLesspFP", "DoUnsignedLesspLP", "DoUnsignedLesspSP", "DoUnsignedLesspIM", /* #o0331 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0332 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0333 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0334 */
    "DoUnsignedLesspNoPopFP", "DoUnsignedLesspNoPopLP", "DoUnsignedLesspNoPopSP", "DoUnsignedLesspNoPopIM", /* #o0335 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0336 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0337 */
    "DoPopFP", "DoPopLP", "DoPopSP", "DoPopIM", /* #o0340 */
    "DoMovemFP", "DoMovemLP", "DoMovemSP", "DoMovemIM", /* #o0341 */
    "DoMergeCdrNoPopFP", "DoMergeCdrNoPopLP", "DoMergeCdrNoPopSP", "DoMergeCdrNoPopIM", /* #o0342 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0343 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0344 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0345 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0346 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0347 */
    "DoFastAref1FP", "DoFastAref1LP", "DoFastAref1SP", "DoFastAref1IM", /* #o0350 */
    "DoFastAset1FP", "DoFastAset1LP", "DoFastAset1SP", "DoFastAset1IM", /* #o0351 */
    "DoStackBltAddressFP", "DoStackBltAddressLP", "DoStackBltAddressSP", "DoStackBltAddressIM", /* #o0352 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0353 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0354 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0355 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0356 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0357 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0360 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0361 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0362 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0363 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0364 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0365 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0366 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0367 */
    "DoDpbFP", "DoDpbLP", "DoDpbSP", "DoDpbIM", /* #o0370 */
    "DoCharDpbFP", "DoCharDpbLP", "DoCharDpbSP", "DoCharDpbIM", /* #o0371 */
    "DoPDpbFP", "DoPDpbLP", "DoPDpbSP", "DoPDpbIM", /* #o0372 */
    "DoPTagDpbFP", "DoPTagDpbLP", "DoPTagDpbSP", "DoPTagDpbIM", /* #o0373 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0374 */
    "DoLoopIncrementTosLessThanFP", "DoLoopIncrementTosLessThanLP", "DoLoopIncrementTosLessThanSP",
    "DoLoopIncrementTosLessThanIM", /* #o0375 */
    "DoCatchOpenFP", "DoCatchOpenLP", "DoCatchOpenSP", "DoCatchOpenIM", /* #o0376 */
    "DoSpareOpFP", "DoSpareOpLP", "DoSpareOpSP", "DoSpareOpIM", /*#o0377 */
};

void dumpstack(void)
{
#if 0
  u64 *p = (u64 *)iSP;
  int i;

  printf("iPC %p, iSP %p\n", iPC, iSP);
  for (i = 0; i < 5; i++) {
    printf("%p: %016llx\n", p, *p);
    if ((u64)p == 0xfffffc000)
      break;
    p--;
  }

  //  if (iPC == 0x1f000000e) exit(1);
  //  { static int c = 0; if (++c == 10000) exit(1); }
#endif
}

/* idispat */
int iInterpret(PROCESSORSTATEP ivoryp)
{
    PROCESSORSTATEP processor;
    u64 ivory = (u64)ivoryp;
    int loops = 0;
    int _trace = 0;
    int _show = 0;
    u64 cpustack[1024];

    u64 r0, instn, iword, ecp, ocp, icsize, epc, opc, count;
    u64 r9, r10, r11, r12, r13, r14, r15;
    u64 r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r29;
    u64 sp;
    u64 r31 = 0;

#include "dispatch"

    // Just to make sure that the end of the dispatch file doesn't creat an issue (IF THAT ACTUALLY WORKS...)
    ;

    // Used no where at all. Deleted to get rid of C standard error about nested functions.
    // void dumpcache(PROCESSORSTATEP p)
    // {
    //     CACHELINEP c, ce;
    //     int i, n;
    //     char *name;

    //     printf("icachebase %p, endicache %p\n", processor->icachebase, processor->endicache);

    //     ce = (CACHELINEP)processor->endicache;
    //     for (c = (CACHELINEP)processor->icachebase, n = 0; c <= ce; c++, n++) {

    //         //    if (n > 16) break;
    //         if (c->pctag == 0 && c->pcdata == 0)
    //             continue;

    //         for (i = 0; i < 256 * 4; i++)
    //             if (_halfworddispatch[i] == c->code)
    //                 break;
    //         if (i == 256 * 4)
    //             name = "unknown";
    //         else
    //             name = halfwordnames[i];

    //         printf("%p: nextcp %p pc %08x %08x inst %08x code %p %s\n", c, c->nextcp, c->pctag, c->pcdata,
    //             c->instruction, c->code, name);
    //     }
    // }

    // See comment in stub.c 1108
    // void show_loc(void)
    // {
    //     static int c = 0;
    //     static u64 bsp;
    //     u64 *p = (u64 *)iSP;
    //     u64 tos = *p;
    //     u32 cc, t, v;
    //     char *str = 0;
    //     int i;

    //     cc = ((tos >> 32) & 0xc0) >> 6;
    //     t = (tos >> 32) & 0x3f;
    //     v = (u32)tos;

    //     c++;
    //     //  if (c >= 20) exit(1);
    //     if (c == 1)
    //         bsp = iSP;
    //     //  printf("%d: ", c);

    //     for (i = 0; i < 256 * 4; i++)
    //         if (_halfworddispatch[i] == ((CACHELINEP)iCP)->code)
    //             break;

    //     if (i == 256 * 4)
    //         str = 0;
    //     else
    //         str = halfwordnames[i];

    //     printf("PC %08x(%s), SP: %08x, TOS: %d.%02x.%08x,%s%s\n", (int)iPC / 2, (iPC & 1) ? "Odd" : "Even",
    //         (int)(0xf8000101 + ((iSP - bsp) / 8)), cc, t, v, str ? " " : "", str ? str : "");
    // }

    printf("[iInterpret]\n");

    processor = (PROCESSORSTATEP)((char *)ivory - PROCESSORSTATE_SIZE);
    printf("%p\n", processor);
    printf("ivory %p\n", ivory);
    printf("epc %p, fp %p, lp %p, sp %p, cp %p\n", processor->epc, processor->fp, processor->lp, processor->sp,
        processor->cp);
    printf("icachebase %p, endicache %p\n", processor->icachebase, processor->endicache);

    /* i still can't believe this works */
    processor->halfworddispatch = (int64_t)_halfworddispatch;
    processor->fullworddispatch = (int64_t)_fullworddispatch;

    processor->internalregisterread1 = (int64_t)_internalregisterread1;
    processor->internalregisterread2 = (int64_t)_internalregisterread2;
    processor->internalregisterwrite1 = (int64_t)_internalregisterwrite1;
    processor->internalregisterwrite2 = (int64_t)_internalregisterwrite2;

    processor->stop_interpreter = 0;

    arg1 = (u64)ivoryp;
    ra = (u64) && iguessimdone;

    sp = (u64)&cpustack[1024];

    if (processor->epc > 0x1f0000000) {
#if 0
//    _trace = 1;
    _show = 1;
#endif
    }

    feclearexcept(FE_ALL_EXCEPT);
    fedisableexcept(FE_ALL_EXCEPT);

    {
        extern void *DECODEFAULT, *ICACHEMISS;
        DECODEFAULT = &&decodefault;
        ICACHEMISS = &&ICACHEMISS;
    }

    goto iinterpret;

iguessimdone:
    printf("I guess I'm done!! r1 %p\n", (int)r1);
    // if (_show) while (1);
    return r1;

/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunhead.as
 ************************************************************************/

  /* Entry points into the interpretation loop. */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunhead.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/idispat.as
 ************************************************************************/

  /* This file implements the main instruction dispatch loop. */
/* start DummyDoNothingSubroutine */


dummydonothingsubroutine:
  if (_trace) printf("dummydonothingsubroutine:\n");
  goto continuecurrentinstruction;   

/* end DummyDoNothingSubroutine */
/* start MemoryReadData */


memoryreaddata:
  if (_trace) printf("memoryreaddata:\n");
  /* Memory Read Internal */

vma_memory_read30393:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30395;

vma_memory_read30394:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30397;

vma_memory_read30403:
  goto *r0; /* ret */

memoryreaddatadecode:
  if (_trace) printf("memoryreaddatadecode:\n");
  if (t6 == 0) 
    goto vma_memory_read30396;

vma_memory_read30395:
  if (_trace) printf("vma_memory_read30395:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read30394;   

vma_memory_read30397:
  if (_trace) printf("vma_memory_read30397:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30396;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30393;   

vma_memory_read30396:
  if (_trace) printf("vma_memory_read30396:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read30400:
  if (_trace) printf("vma_memory_read30400:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read30399;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read30403;   

vma_memory_read30399:

vma_memory_read30398:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

/* end MemoryReadData */
/* start MemoryReadGeneral */


memoryreadgeneral:
  if (_trace) printf("memoryreadgeneral:\n");
  /* Memory Read Internal */

vma_memory_read30404:
  t7 = arg2 + ivory;
  t8 = (arg3 * 4);   		// Cycle-number -> table offset 
  arg5 = LDQ_U(t7);   
  t8 = (t8 * 4) + ivory;   
  arg6 = (t7 * 4);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)(t8 + PROCESSORSTATE_DATAREAD_MASK);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30406;

vma_memory_read30405:
  t8 = t8 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30408;

vma_memory_read30414:
  goto *r0; /* ret */

memoryreadgeneraldecode:
  if (_trace) printf("memoryreadgeneraldecode:\n");
  if (t6 == 0) 
    goto vma_memory_read30407;

vma_memory_read30406:
  if (_trace) printf("vma_memory_read30406:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read30405;   

vma_memory_read30408:
  if (_trace) printf("vma_memory_read30408:\n");

vma_memory_read30407:
  if (_trace) printf("vma_memory_read30407:\n");
  t8 = (arg3 * 4);   		// Cycle-number -> table offset 
  t8 = (t8 * 4) + ivory;   
  t8 = *(u64 *)(t8 + PROCESSORSTATE_DATAREAD);   
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read30412:
  if (_trace) printf("vma_memory_read30412:\n");
  t6 = t8 & MemoryActionIndirect;
  if (t6 == 0) 
    goto vma_memory_read30411;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30404;   

vma_memory_read30411:
  if (_trace) printf("vma_memory_read30411:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read30410;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read30414;   

vma_memory_read30410:

vma_memory_read30409:
  /* Perform memory action */
  arg1 = t8;
  arg2 = arg3;
  goto performmemoryaction;

/* end MemoryReadGeneral */
/* start MemoryReadHeader */


memoryreadheader:
  if (_trace) printf("memoryreadheader:\n");
  /* Memory Read Internal */

vma_memory_read30415:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30417;

vma_memory_read30416:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30419;

vma_memory_read30423:
  goto *r0; /* ret */

memoryreadheaderdecode:
  if (_trace) printf("memoryreadheaderdecode:\n");
  if (t6 == 0) 
    goto vma_memory_read30418;

vma_memory_read30417:
  if (_trace) printf("vma_memory_read30417:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read30416;   

vma_memory_read30419:
  if (_trace) printf("vma_memory_read30419:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30418;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30415;   

vma_memory_read30418:
  if (_trace) printf("vma_memory_read30418:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read30420:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end MemoryReadHeader */
/* start MemoryReadCdr */


memoryreadcdr:
  if (_trace) printf("memoryreadcdr:\n");
  /* Memory Read Internal */

vma_memory_read30424:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30426;

vma_memory_read30425:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30428;

vma_memory_read30432:
  goto *r0; /* ret */

memoryreadcdrdecode:
  if (_trace) printf("memoryreadcdrdecode:\n");
  if (t6 == 0) 
    goto vma_memory_read30427;

vma_memory_read30426:
  if (_trace) printf("vma_memory_read30426:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read30425;   

vma_memory_read30428:
  if (_trace) printf("vma_memory_read30428:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30427;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30424;   

vma_memory_read30427:
  if (_trace) printf("vma_memory_read30427:\n");
  t8 = *(u64 *)&(processor->cdr);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read30429:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 9;
  goto performmemoryaction;

/* end MemoryReadCdr */
/* start DoICacheFill */


doicachefill:
  if (_trace) printf("doicachefill:\n");

ICACHEMISS:
  if (_trace) printf("ICACHEMISS:\n");
  /* Here when instruction cache miss detected.  Fill the cache from */
  /* PC and then resume interpreter loop */
  /* First round the PC down to an even halfword address */
  arg2 = *(u64 *)&(processor->icachebase);   		// get the base of the icache 
  epc = iPC & ~1L;		// the even PC 
  ecp = epc >> (CacheLine_RShift & 63);   
  arg1 = zero + -1;   
  arg1 = arg1 + ((4) << 16);   
  ecp = ecp << (CacheLine_LShift & 63);   
  instn = iPC >> 1;   		// instn is instruction address here 
  ecp = epc + ecp;
  ecp = ecp & arg1;
  arg3 = ecp << 5;   		// temp=cpos*32 
  ecp = ecp << 4;   		// cpos=cpos*16 
  arg4 = arg2 + arg3;		// temp2=base+cpos*32 
  ecp = arg4 + ecp;		// cpos=base+cpos*48 
  opc = epc | 1;		// the odd PC 
  iCP = ecp;		// Assume iPC is the even PC 
  arg1 = (iPC == opc) ? 1 : 0;   		// See if iPC is the odd PC 
  ocp = ecp + CACHELINE_SIZE;
  if (arg1)   		// Stash the odd cache pointer if iPC is the odd PC 
    iCP = ocp;
  hwdispatch = *(u64 *)&(processor->halfworddispatch);   
  hwopmask = zero + 1023;   
  fwdispatch = *(u64 *)&(processor->fullworddispatch);   
  count = zero + 20;   
  t11 = instn + ivory;
  iword = (t11 * 4);   
  arg4 = LDQ_U(t11);   
  iword = *(s32 *)iword;   
  arg4 = (u8)(arg4 >> ((t11&7)*8));   
  goto fillicacheprefetched;   

pcbackone:
  if (_trace) printf("pcbackone:\n");
  /* Wire in continuation for even half */
  *(u64 *)&((CACHELINEP)ocp)->nextpcdata = epc;   
  t10 = ecp - CACHELINE_SIZE;   		// Backup in cache too 
  *(u64 *)&((CACHELINEP)ocp)->nextcp = ecp;   
  arg1 = epc - 1;   		// Backup PC one halfword 
  *(u64 *)&((CACHELINEP)ecp)->nextcp = t10;   
  /* TagType. */
  arg4 = arg4 & 63;		// arg4=tag-cdr code 
  *(u64 *)&((CACHELINEP)ecp)->nextpcdata = arg1;   
  /* Wire in continuation for odd half */
  goto maybeunpack;   

pcadvone:
  if (_trace) printf("pcadvone:\n");
  *(u64 *)&((CACHELINEP)ecp)->nextpcdata = opc;   		// Simple advance of PC one halfword. 
  arg1 = opc + 1;
  *(u64 *)&((CACHELINEP)ecp)->nextcp = ocp;   
  t10 = ocp + CACHELINE_SIZE;
  *(u64 *)&((CACHELINEP)ocp)->nextpcdata = arg1;   
  /* TagType. */
  arg4 = arg4 & 63;		// arg4=tag-cdr code 
  *(u64 *)&((CACHELINEP)ocp)->nextcp = t10;   
  goto maybeunpack;   
  /* This is the cache fill loop. */

fillicache:
  if (_trace) printf("fillicache:\n");
  t11 = instn + ivory;
  iword = (t11 * 4);   
  arg4 = LDQ_U(t11);   
  iword = *(s32 *)iword;   
  arg4 = (u8)(arg4 >> ((t11&7)*8));   

fillicacheprefetched:
  if (_trace) printf("fillicacheprefetched:\n");
  *(u64 *)&((CACHELINEP)ecp)->pcdata = epc;   		// Set address of even cache posn. 
  arg1 = arg4 & 192;		// CDR code << 6 
  /* TagType. */
  arg4 = arg4 & 63;		// Strip cdr 
  *(u64 *)&((CACHELINEP)ocp)->pcdata = opc;   		// Set address of odd cache posn. 
  iword = (u32)iword;   		// Strip nasty bits out. 

force_alignment30433:
  if (_trace) printf("force_alignment30433:\n");
  arg2 = arg4 << 32;   		// ready to remerge 
  if (arg1 == 0) 		// Zerotag means advance one HW 
    goto pcadvone;
  arg1 = arg1 - 128;   		// 2<<6 
  if (arg1 == 0) 		// Tag=2 means backup one HW 
    goto pcbackone;
  if ((s64)arg1 < 0)   		// Tag=1 means end of compiled function 
    goto pcendcf;

pcadvtwo:
  if (_trace) printf("pcadvtwo:\n");
  /* Tag=3 means advance over one full word */
  /* Wire in continuation for even half */
  arg1 = epc + 2;		// Next word 
  r31 = r31 | r31;
  t10 = ecp + TWOCACHELINESIZE;		// corresponding CP entry 
  *(u64 *)&((CACHELINEP)ecp)->nextpcdata = arg1;   		// Next PC even of next word 
  arg1 = epc + 4;		// Skip one fullword 
  *(u64 *)&((CACHELINEP)ecp)->nextcp = t10;   		// Next CP 
  /* Wire in continuation for odd half */
  t10 = ecp + FOURCACHELINESIZE;		// corresponding CP entry 
  *(u64 *)&((CACHELINEP)ocp)->nextpcdata = arg1;   
  /* TagType. */
  arg4 = arg4 & 63;		// arg4=tag-cdr code 
  *(u64 *)&((CACHELINEP)ocp)->nextcp = t10;   
  goto maybeunpack;   

decodepackedword:
  if (_trace) printf("decodepackedword:\n");
  /* Here to decode a packed word */
  arg4 = iword >> 18;   		// arg4 contains the odd packedword 
  t10 = iword >> 8;   		// even opcode+2bits 
  *(u64 *)&((CACHELINEP)ocp)->instruction = arg4;   		// Save the odd instruction 
  t11 = iword << 54;   		// First phase of even operand sign extension. 
  t12 = iword & hwopmask;		// even operand+2bits 
#ifndef CACHEMETERING
  *(u64 *)&((CACHELINEP)ocp)->annotation = zero;   
#endif
  t10 = t10 & hwopmask;		// even opcode 
  t11 = (s64)t11 >> 38;   		// Second phase of even operand sign extension. 
  arg2 = t10 - 92;   
  t10 = (t10 * 8) + hwdispatch;  
  t12 = t11 | t12;		// Merge signed/unsigned even operand 
  arg2 = arg2 & ~3L;
  *(u32 *)&((CACHELINEP)ecp)->operand = t12;   
  if (arg2 == 0)   		// clear count if finish-call seen 
    count = arg2;
  arg2 = arg4 >> 8;   		// odd opcode+2bits 
  t11 = arg4 << 54;   		// First phase of odd operand sign extension. 
  arg1 = arg4 & hwopmask;		// odd operand+2bits 
  t10 = *(u64 *)t10;   
  arg2 = arg2 & hwopmask;		// odd opcode 
  t11 = (s64)t11 >> 38;   		// Second phase of odd operand sign extension. 
  *(u64 *)&((CACHELINEP)ecp)->code = t10;   
  t12 = arg2 - 92;   
  arg2 = (arg2 * 8) + hwdispatch;  
  arg1 = t11 | arg1;		// Merge signed/unsigned odd operand 
  *(u32 *)&((CACHELINEP)ocp)->operand = arg1;   
  t12 = t12 & ~3L;
  arg2 = *(u64 *)arg2;   
  if (t12 == 0)   		// clear count if finish-call seen 
    count = t12;
  *(u64 *)&((CACHELINEP)ocp)->code = arg2;   
  goto enddecode;   

maybeunpack:
  if (_trace) printf("maybeunpack:\n");
  iword = arg2 | iword;		// reassemble tag and word. 
  *(u64 *)&((CACHELINEP)ecp)->instruction = iword;   		// save the even instruction 
  t10 = arg4 - 48;   		// t10>=0 if packed 
  if ((s64)t10 >= 0)   		// B. if a packed instruction 
    goto decodepackedword;
  t11 = (arg4 * 8) + fwdispatch;  		// t11 is the fwdispatch index 
  t12 = *(u64 *)&(processor->i_stage_error_hook);   
  arg1 = arg4 - 33;   
  t11 = *(u64 *)t11;   		// Extract the opcode handler 
  *(u64 *)&((CACHELINEP)ocp)->code = t12;   		// Store I-STATE-ERROR at odd pc 
  if (arg1 == 0)   		// clear count if native instn seen 
    count = arg1;
  *(u64 *)&((CACHELINEP)ecp)->code = t11;   

enddecode:
  if (_trace) printf("enddecode:\n");
  /* Here we decide if to stop filling the cache and return to the */
  /* instruction interpretation stream, or whether to fill further */
  instn = instn + 1;
  if ((s64)count <= 0)  		// If count is zero, resume 
    goto cachevalid;
  epc = instn << 1;   
  count = count - 1;   		// decrement count 
  opc = epc | 1;
  t10 = *(u64 *)&(processor->endicache);   		// pointer to the end of icache 
  ocp = ocp + TWOCACHELINESIZE;
  ecp = ecp + TWOCACHELINESIZE;
  t10 = ocp - t10;   
  if ((s64)t10 <= 0)  		// Still room for more 
    goto fillicache;
  goto cachevalid;   

pcendcf:
  if (_trace) printf("pcendcf:\n");
  t11 = *(u64 *)&(processor->i_stage_error_hook);   
  count = r31 | r31;		// We reached the end of the fcn. 
  *(u64 *)&((CACHELINEP)ecp)->code = t11;   		// Store I-STATE-ERROR dispatch at even and odd pc 
  *(u64 *)&((CACHELINEP)ocp)->code = t11;   
  goto enddecode;   

/* end DoICacheFill */
  /* These are the instruction reentry points.  Instructions end by returning */
  /* control to one of these tags.  Most normal instructions reenter by jumping */
  /* to NEXTINSTRUCTION, which advances the PC and continues normally.   */
  /* Instructions that change the PC usually go directly to INTERPRETINSTRUCTION. */
  /* Instructions that fail/trap/exception etc, go to one of the other places. */
/* start iInterpret */


iinterpret:
  if (_trace) printf("iinterpret:\n");
  *(u64 *)&processor->asrr9 = r9;   
  *(u64 *)&processor->asrr10 = r10;   
  *(u64 *)&processor->asrr11 = r11;   
  *(u64 *)&processor->asrr12 = r12;   
  *(u64 *)&processor->asrr13 = r13;   
  *(u64 *)&processor->asrr15 = r15;   
  *(u64 *)&processor->asrr26 = r26;   
  *(u64 *)&processor->asrr27 = r27;   
  *(u64 *)&processor->asrr29 = r29;   
  *(u64 *)&processor->asrr30 = r30;   
  *(u64 *)&processor->asrr14 = r14;   
  ivory = arg1;		// Setup our processor object handle 
  /* Upon entry, load cached state. */
  iCP = *(u64 *)&(processor->cp);   
  iPC = *(u64 *)&(processor->epc);   
  iSP = *(u64 *)&(processor->sp);   
  iFP = *(u64 *)&(processor->fp);   
  iLP = *(u64 *)&(processor->lp);   
  if (iCP != 0)   		// First time in iCP will be zero. 
    goto INTERPRETINSTRUCTION;
  goto ICACHEMISS;   		// If this is the first time in cache is empty! 

interpretinstructionpredicted:
  if (_trace) printf("interpretinstructionpredicted:\n");
  t2 = *(u64 *)&(((CACHELINEP)arg2)->pcdata);   		// Get the PC to check cache hit. 
  arg1 = iFP;   		// Assume FP mode 
  r0 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop? 
  arg4 = iSP + -8;   		// SP-pop mode constant 
  arg3 = *(u64 *)&(((CACHELINEP)arg2)->instruction);   		// Grab the instruction/operand while stalled 
  t1 = iPC - t2;   
  if (t1 != 0)   
    goto interpretinstructionforbranch;
  iCP = arg2;
  if (r0 != 0)   		// Stop the world! someone wants out. 
    goto traporsuspendmachine;
  goto continuecurrentinstruction;   

interpretinstructionforjump:
  if (_trace) printf("interpretinstructionforjump:\n");

interpretinstructionforbranch:
  if (_trace) printf("interpretinstructionforbranch:\n");
  t5 = *(u64 *)&(processor->icachebase);   		// get the base of the icache 
  t4 = zero + -1;   
  t4 = t4 + ((4) << 16);   
  arg2 = iPC >> 10;   
  t3 = zero + -64;   
  arg2 = arg2 & t3;
  arg2 = iPC + arg2;
  arg2 = arg2 & t4;
  t4 = arg2 << 5;   		// temp=cpos*32 
  arg2 = arg2 << 4;   		// cpos=cpos*16 
  t5 = t5 + t4;		// temp2=base+cpos*32 

force_alignment30434:
  if (_trace) printf("force_alignment30434:\n");
  arg2 = t5 + arg2;		// cpos=base+cpos*48 
#ifndef CACHEMETERING
  *(u64 *)&((CACHELINEP)iCP)->annotation = arg2;   
#endif
  iCP = arg2;

INTERPRETINSTRUCTION:
  if (_trace) printf("INTERPRETINSTRUCTION:\n");
  r30 = *(u64 *)&(processor->asrr30);   
  r0 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop? 
  arg1 = iFP;   		// Assume FP mode 
  arg3 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// Grab the instruction/operand while stalled 
  arg4 = iSP + -8;   		// SP-pop mode constant 
  t2 = *(u64 *)&(((CACHELINEP)iCP)->pcdata);   		// Get the PC to check cache hit. 
  if (r0 != 0)   		// Stop the world! someone wants out. 
    goto traporsuspendmachine;
  goto continuecurrentinstruction;   

/* end iInterpret */



/* End of file automatically generated from ../alpha-emulator/idispat.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifuncom1.as
 ************************************************************************/

  /* The most commonly used instructions, part 1.  */
/* start DoPush */

  /* Halfword operand from stack instruction - DoPush */
  /* arg2 has the preloaded 8 bit operand. */

dopush:
  if (_trace) printf("dopush:\n");

DoPushSP:
  if (_trace) printf("DoPushSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushLP:
  if (_trace) printf("DoPushLP:\n");

DoPushFP:
  if (_trace) printf("DoPushFP:\n");

begindopush:
  if (_trace) printf("begindopush:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = iSP + 8;		// Push the new value 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t1 = *(s32 *)(arg1 + 4);   		// Get the tag/data 
  t2 = *(s32 *)arg1;   
  *(u32 *)iSP = t2;   		// Store the data word 

force_alignment30441:
  if (_trace) printf("force_alignment30441:\n");
  /* TagType. */
  t1 = t1 & 63;		// make it CDR NEXT 
  *(u32 *)(iSP + 4) = t1;   		// Store the TAG - this *DOES* dual issue! 

/* end DoPush */
  /* End of Halfword operand from stack instruction - DoPush */
/* start nextInstruction */


nextinstruction:
  if (_trace) printf("nextinstruction:\n");

cachevalid:
  if (_trace) printf("cachevalid:\n");
  arg3 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// Grab the instruction/operand while stalled 
  arg1 = iFP;   		// Assume FP mode 
  t2 = *(u64 *)&(((CACHELINEP)iCP)->pcdata);   		// Get the PC to check cache hit. 
  arg4 = iSP + -8;   		// SP-pop mode constant 

continuecurrentinstruction:
  if (_trace) printf("continuecurrentinstruction:\n");
  t3 = *(u64 *)&(((CACHELINEP)iCP)->code);   		// Instruction handler 
  arg5 = iSP + -2040;   		// SP mode constant 
  *(u64 *)&processor->restartsp = iSP;   		// Need this in case we take a trap 
  t4 = (u8)(arg3 >> ((5&7)*8));   		// Get the mode bits 
  t2 = t2 - iPC;   		// check for HIT. 
  arg6 = *(u64 *)iSP;   		// Load TOS in free di slot 
  arg2 = (u8)(arg3 >> ((4&7)*8));   		// Extract (8-bit, unsigned) operand 
  if (t2 != 0)   		// PC didn't match, take a cache miss 
    goto takeicachemiss;
  if (t4 & 1)   		// LP or Immediate mode 
   arg1 = iLP;
    goto *t3; /* jmp */   		// Jump to the handler 
  /* Here to advance the PC and begin a new instruction.  Most */
  /* instructions come here when they have finished.  Instructions */
  /* that explicitly update the PC (and CP) go to interpretInstruction. */

NEXTINSTRUCTION:
  if (_trace) printf("NEXTINSTRUCTION:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   		// Load the next PC from the cache 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   		// Advance cache position 
  goto cachevalid;   

takeicachemiss:
  if (_trace) printf("takeicachemiss:\n");
  goto ICACHEMISS;

/* end nextInstruction */
/* start DoPushImmediateHandler */


dopushimmediatehandler:
  if (_trace) printf("dopushimmediatehandler:\n");

DoPushIM:
  if (_trace) printf("DoPushIM:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg2;   		// Push it with CDR-NEXT onto the stack 
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

/* end DoPushImmediateHandler */
/* start DoBranchTrue */

  /* Halfword 10 bit immediate instruction - DoBranchTrue */

dobranchtrue:
  if (_trace) printf("dobranchtrue:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueIM:
  if (_trace) printf("DoBranchTrueIM:\n");

DoBranchTrueSP:
  if (_trace) printf("DoBranchTrueSP:\n");

DoBranchTrueLP:
  if (_trace) printf("DoBranchTrueLP:\n");

DoBranchTrueFP:
  if (_trace) printf("DoBranchTrueFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto dobrpopelsepop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrpopelsepop:
  if (_trace) printf("dobrpopelsepop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrue */
  /* End of Halfword operand from stack instruction - DoBranchTrue */
/* start DoBranchFalse */

  /* Halfword 10 bit immediate instruction - DoBranchFalse */

dobranchfalse:
  if (_trace) printf("dobranchfalse:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseIM:
  if (_trace) printf("DoBranchFalseIM:\n");

DoBranchFalseSP:
  if (_trace) printf("DoBranchFalseSP:\n");

DoBranchFalseLP:
  if (_trace) printf("DoBranchFalseLP:\n");

DoBranchFalseFP:
  if (_trace) printf("DoBranchFalseFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto dobrnpopelsepop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrnpopelsepop:
  if (_trace) printf("dobrnpopelsepop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalse */
  /* End of Halfword operand from stack instruction - DoBranchFalse */
/* start DoReturnSingle */

  /* Halfword 10 bit immediate instruction - DoReturnSingle */

doreturnsingle:
  if (_trace) printf("doreturnsingle:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoReturnSingleIM:
  if (_trace) printf("DoReturnSingleIM:\n");

DoReturnSingleSP:
  if (_trace) printf("DoReturnSingleSP:\n");

DoReturnSingleLP:
  if (_trace) printf("DoReturnSingleLP:\n");

DoReturnSingleFP:
  if (_trace) printf("DoReturnSingleFP:\n");
  /* arg1 has operand preloaded. */
  /* Fetch value based on immediate, interleaved with compute disposition dispatch */
  arg5 = *(s32 *)&processor->control;   
  arg3 = arg6 << 26;   		// Clear cdr 
  t3 = (12) << 16;   
  t1 = *(u64 *)&(processor->niladdress);   
  arg3 = arg3 >> 26;   		// Clear cdr 
  t2 = *(u64 *)&(processor->taddress);   
  t3 = t3 & arg5;		// mask disposition bits 
  t3 = t3 >> 18;   		// shift disposition bits into place 
  arg6 = *(u64 *)&(processor->stackcachedata);   
  /* arg2 is 8 bits of "kludge operand" 0=TOS 40=NIL 41=T */
  if ((s64)arg2 > 0)   
    arg3 = t1;
  arg4 = t3 - 2;   		// arg4 -2=effect -1=value 0=return 1=multiple 
  if (arg2 & 1)   
   arg3 = t2;

returncommontail:
  if (_trace) printf("returncommontail:\n");
  /* Restore machine state from frame header. */
  t3 = *(s32 *)iFP;   
  t1 = (1792) << 16;   
  t5 = *(s32 *)&processor->continuation;   
  t1 = arg5 & t1;		// Mask 
  t2 = *(s32 *)(iFP + 4);   
  t7 = iCP;
  if (t1 != 0)   		// Need to cleanup frame first 
    goto returnsinglecleanup;
  t3 = (u32)t3;   
  t4 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t6 = *(s32 *)(iFP + 8);   		// Get saved control register 
  /* TagType. */
  t2 = t2 & 63;
  /* Restore the PC. */
  if (arg4 == 0) 
    goto abandon_frame_simple30449;
  iPC = t5 << 1;   		// Assume even PC 
  t1 = t4 & 1;
  t7 = *(u64 *)&(processor->continuationcp);   
  iPC = iPC + t1;

abandon_frame_simple30449:
  if (_trace) printf("abandon_frame_simple30449:\n");
  /* Restore the saved continuation */
  *((u32 *)(&processor->continuation)+1) = t2;   
  t1 = arg5 >> 9;   		// Get the caller frame size into place 
  *(u32 *)&processor->continuation = t3;   
  iSP = iFP - 8;   		// Restore the stack pointer. 
  *(u64 *)&processor->continuationcp = zero;   
  t1 = t1 & 255;		// Mask just the caller frame size. 
  t1 = (t1 * 8) + 0;  		// *8 
  t2 = (2048) << 16;   
  t2 = t2 & arg5;
  t3 = *(s32 *)&processor->interruptreg;   		// Get the preempt-pending bit 
  t6 = t2 | t6;		// Sticky trace pending bit. 
  t4 = *(u64 *)&(processor->please_stop);   		// Get the trap/suspend bits 
  iFP = iFP - t1;   		// Restore the frame pointer. 
  *(u32 *)&processor->control = t6;   		// Restore the control register 
  t1 = t6 & 255;		// extract the argument size 
  t3 = t3 & 1;
  t3 = t4 | t3;
  *(u64 *)&processor->stop_interpreter = t3;   
  iLP = (t1 * 8) + iFP;  		// Restore the local pointer. 

force_alignment30450:
  if (_trace) printf("force_alignment30450:\n");
  arg6 = ((u64)iFP < (u64)arg6) ? 1 : 0;   		// ARG6 = stack-cache underflow 
  /* arg4 -2=effect -1=value 0=return 1=multiple */
  if (arg4 == 0) 
    goto returnsinglereturn;
  if ((arg4 & 1) == 0)   
    goto returnsingleeffect;
  *(u64 *)(iSP + 8) = arg3;   
  iSP = iSP + 8;
  if ((s64)arg4 > 0)   
    goto returnsinglemultiple;

returnsingleeffect:
  if (_trace) printf("returnsingleeffect:\n");

returnsingledone:
  if (_trace) printf("returnsingledone:\n");
  if (arg6 != 0)   
    goto returnsingleunderflow;
  if (t7 == 0) 		// No prediction, validate cache 
    goto interpretinstructionforbranch;
  iCP = t7;
  goto INTERPRETINSTRUCTION;   

returnsinglemultiple:
  if (_trace) printf("returnsinglemultiple:\n");
  t8 = Type_Fixnum;		// Multiple-value group 
  t8 = t8 << 32;   
  iSP = iSP + 8;
  t8 = t8 | 1;
  *(u64 *)iSP = t8;   		// Push Fixnum 
  goto returnsingledone;   

returnsinglereturn:
  if (_trace) printf("returnsinglereturn:\n");
  if (arg2 != 0)   
    goto returnsingledone;
  *(u64 *)(iSP + 8) = arg3;   
  iSP = iSP + 8;
  goto returnsingledone;   

returnsinglecleanup:
  if (_trace) printf("returnsinglecleanup:\n");
  goto handleframecleanup;

returnsingleunderflow:
  if (_trace) printf("returnsingleunderflow:\n");
  goto stackcacheunderflowcheck;

/* end DoReturnSingle */
  /* End of Halfword operand from stack instruction - DoReturnSingle */
/* start callindirect */

  /*  */
  /*  */
  /* Fullword instruction - callindirect */
  /* ======================= */

callindirect:
  if (_trace) printf("callindirect:\n");

callindirectprefetch:
  if (_trace) printf("callindirectprefetch:\n");
  arg2 = (u32)arg3;   		// Get operand 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg3 = zero;		// No extra arg 
  /* Memory Read Internal */

vma_memory_read30451:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30453;

vma_memory_read30452:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30455;

vma_memory_read30462:
  t5 = arg5 - Type_CompiledFunction;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto startcallagain;
  arg5 = Type_EvenPC;
  t7 = *((s32 *)(&processor->continuation)+1);   
  iSP = iSP + 16;		// prepare to push continuation/control register 
  t3 = *(s32 *)&processor->control;   
  t6 = Type_Fixnum+0xC0;
  t8 = *(s32 *)&processor->continuation;   
  t5 = (64) << 16;   
  t7 = t7 | 192;		// Set CDR code 3 
  *(u32 *)(iSP + -8) = t8;   		// push continuation 
  *(u32 *)(iSP + -4) = t7;   		// write the stack cache 
  t8 = t3 | t5;		// Set call started bit in CR 
  t5 = zero + 256;   
  *(u32 *)iSP = t3;   		// Push control register 
  *(u32 *)(iSP + 4) = t6;   		// write the stack cache 
  t8 = t8 & ~t5;		// Clear the extra arg bit 
  *(u32 *)&processor->control = t8;   		// Save control with new state 
  /* End of push-frame */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)&processor->continuation = arg6;   
  *((u32 *)(&processor->continuation)+1) = arg5;   
  *(u64 *)&processor->continuationcp = zero;   
  if (arg3 != 0)   
    goto callindirectextra;
  goto cachevalid;   

callindirectextra:
  if (_trace) printf("callindirectextra:\n");
  t1 = *(s32 *)&processor->control;   
  t2 = zero + 256;   
  t3 = arg3 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg4;   		// Push the extra arg. 
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  iSP = iSP + 8;
  t1 = t1 | t2;		// Set the extra arg bit 
  *(u32 *)&processor->control = t1;   		// Save control with new state 
  goto cachevalid;   

vma_memory_read30455:
  if (_trace) printf("vma_memory_read30455:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30454;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30451;   

vma_memory_read30454:
  if (_trace) printf("vma_memory_read30454:\n");

vma_memory_read30453:
  if (_trace) printf("vma_memory_read30453:\n");
  r0 = (u64)&&return0001;
  goto memoryreaddatadecode;
return0001:
  goto vma_memory_read30462;   

/* end callindirect */
  /* End of Fullword instruction - callindirect */
  /* ============================== */
  /*  */
/* start DoFinishCallN */

  /* Halfword 10 bit immediate instruction - DoFinishCallN */

dofinishcalln:
  if (_trace) printf("dofinishcalln:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoFinishCallNIM:
  if (_trace) printf("DoFinishCallNIM:\n");

DoFinishCallNSP:
  if (_trace) printf("DoFinishCallNSP:\n");

DoFinishCallNLP:
  if (_trace) printf("DoFinishCallNLP:\n");

DoFinishCallNFP:
  if (_trace) printf("DoFinishCallNFP:\n");
  /* arg1 has operand preloaded. */
  /* arg2 contains the 8 bit N+1 */
  arg1 = (u8)(arg3 >> ((5&7)*8));   		// arg1 contains the disposition (two bits) 
  arg2 = (arg2 * 8) + zero;  		// convert N to words (stacked words that is) 

finishcallmerge:
  if (_trace) printf("finishcallmerge:\n");
  arg3 = arg3 >> 7;   
  t6 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t3 = zero + 128;   
  t4 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t3 = (t3 * 8) + iSP;  		// SCA of desired end of cache 
  t4 = (t6 * 8) + t4;  		// SCA of current end of cache 
  t6 = ((s64)t3 <= (s64)t4) ? 1 : 0;   
  if (t6 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check30463;
  arg3 = arg3 & 8;		// 0 if not apply, 8 if apply 
  t1 = *(s32 *)&processor->control;   		// Get the control register 
  /* Compute the new LP */
  iLP = iSP + 8;   		// Assume not Apply case. 
  iLP = iLP - arg3;   		// For apply, iLP==iSP 
  /* Compute the new FP */
  t3 = t1 >> 5;   		// extra arg bit<<3 
  t2 = iSP - arg2;   
  t3 = t3 & 8;		// 8 if extra arg, 0 otherwise. 
  t2 = t2 - t3;   		// This! is the new frame pointer! 
  /* compute arg size */
  t4 = iLP - t2;   
  t4 = t4 >> 3;   		// arg size in words. 
  /* compute caller frame size. */
  t5 = t2 - iFP;   
  t5 = t5 >> 3;   		// caller frame size in words. 
  /* Now hack the control register! */
  t7 = arg1 << 18;   		// Get value disposition into place 
  t6 = *(u64 *)&(processor->fccrmask);   		// cr.caller-frame-size 
  t5 = t5 << 9;   		// Shift caller frame size into place 
  t7 = t7 | t4;		// Add arg size to new bits. 
  t4 = arg3 << 14;   		// Apply bit in place 
  t7 = t5 | t7;		// Add frame size to new bits 
  t7 = t4 | t7;		// All new bits assembled! 
  /* Set the return continuation. */
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   		// Next instruction hw format 
  t1 = t1 & t6;		// Mask off unwanted bits 
  t4 = *(s32 *)&processor->continuation;   		// Get the new PC tag/data 
  t1 = t1 | t7;		// Add argsize, apply, disposition, caller FS 
  t3 = *((s32 *)(&processor->continuation)+1);   
  /* Update the PC */
  /* Convert PC to a real continuation. */
  t6 = t5 & 1;
  t7 = t5 >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  t4 = (u32)t4;   
  /* Convert real continuation to PC. */
  iPC = t3 & 1;
  iPC = t4 + iPC;
  iPC = t4 + iPC;
  *(u32 *)&processor->continuation = t7;   
  *((u32 *)(&processor->continuation)+1) = t6;   		// Set return address 
  /* Update CP */
  t7 = (4096) << 16;   
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = t7 & t1;
  t7 = t7 >> 1;   		// Shift into trace pending place 
  *(u64 *)&processor->continuationcp = t5;   
  t1 = t1 | t7;		// Set the cr.trace pending if appropriate. 
  *(u32 *)&processor->control = t1;   		// Set the control register 
  iFP = t2;		// Install the new frame pointer 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  /* Check for stack overflow */
  t1 = t1 >> 30;   		// Isolate trap mode 
  t3 = *(s32 *)&processor->cslimit;   		// Limit for emulator mode 
  t4 = *(s32 *)&processor->csextralimit;   		// Limit for extra stack and higher modes 
  if (t1)   		// Get the right limit for the current trap mode 
    t3 = t4;
  t3 = (u32)t3;   		// Might have been sign extended 
  /* Convert stack cache address to VMA */
  t4 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = iSP - t4;   		// stack cache base relative offset 
  t4 = t4 >> 3;   		// convert byte address to word address 
  t1 = t4 + t1;		// reconstruct VMA 
  t4 = ((s64)t1 < (s64)t3) ? 1 : 0;   		// Check for overflow 
  if (t4 == 0) 		// Jump if overflow 
    goto stackoverflow;
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  /* Begin execution at the computed address */
  goto interpretinstructionforbranch;   

stack_cache_overflow_check30463:
  if (_trace) printf("stack_cache_overflow_check30463:\n");
  arg2 = 0;
  goto stackcacheoverflowhandler;   

/* end DoFinishCallN */
  /* End of Halfword operand from stack instruction - DoFinishCallN */
/* start DoEntryRestNotAccepted */

  /* Field Extraction instruction - DoEntryRestNotAccepted */

doentryrestnotaccepted:
  if (_trace) printf("doentryrestnotaccepted:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoEntryRestNotAcceptedIM:
  if (_trace) printf("DoEntryRestNotAcceptedIM:\n");

DoEntryRestNotAcceptedSP:
  if (_trace) printf("DoEntryRestNotAcceptedSP:\n");

DoEntryRestNotAcceptedLP:
  if (_trace) printf("DoEntryRestNotAcceptedLP:\n");

DoEntryRestNotAcceptedFP:
  if (_trace) printf("DoEntryRestNotAcceptedFP:\n");
  arg5 = *(s32 *)&processor->control;   		// The control register 
  arg4 = arg3 >> 18;   		// Pull down the number of optionals 
  arg1 = (u8)(arg3 >> ((5&7)*8));   		// Extract the 'ptr' field while we are waiting 
  arg4 = arg4 & 255;
  /* arg1=ptr field, arg2=required, arg3=instn, arg4=optionals arg5=control-register */
  t2 = arg5 >> 27;   		// Get the cr.trace-pending bit 
  t1 = arg5 & 255;		// The supplied args 
  if (t2 & 1)   
    goto tracetrap;
  t3 = arg5 >> 17;   
  t4 = *(s32 *)(iSP + 4);   		// Get the tag of the stack top. 

force_alignment30466:
  if (_trace) printf("force_alignment30466:\n");
  if (t3 & 1)   		// J. if apply args 
    goto b_apply_argument_supplied30464;

b_apply_argument_supplied30465:
  t2 = t1 - arg2;   		// t2=supplied-minimum 
  if ((s64)t2 < 0)   		// B. if too few args. 
    goto retryernatoofew;
  arg1 = arg4 - t1;   		// maximum-supplied 
  if ((s64)arg1 < 0)   		// B. if too many args. 
    goto retryernatoomany;
  /* Compute entry position and advance PC/CP accordingly. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   		// get the next PC 
  t3 = t2 << 1;   		// Adjust index to halfword 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0) 		// J. if index zero, no adjustment. 
    goto INTERPRETINSTRUCTION;
  iPC = iPC + t3;		// Compute the new address 
  iPC = iPC & ~1L;		// Make it an DTP-EVEN-PC 
  goto interpretinstructionforjump;   

applysupprna:
  if (_trace) printf("applysupprna:\n");
  arg1 = arg4 - t1;   
  if ((s64)arg1 <= 0)  		// B. if too many args. 
    goto retryernatoomany;
  goto pullapplyargs;   

retryernatoomany:
  if (_trace) printf("retryernatoomany:\n");
  arg5 = 0;
  arg2 = 78;
  goto illegaloperand;

retryernatoofew:
  if (_trace) printf("retryernatoofew:\n");
  arg5 = 0;
  arg2 = 77;
  goto illegaloperand;

b_apply_argument_supplied30464:
  if (_trace) printf("b_apply_argument_supplied30464:\n");
  t4 = t4 & 63;
  t4 = t4 - Type_NIL;   
  if (t4 != 0)   		// J. if apply args supplied not nil. 
    goto applysupprna;
  t3 = t3 & 1;		// keep just the apply bit! 
  t3 = t3 << 17;   		// reposition the apply bit 
  iSP = iSP - 8;   		// Pop off the null applied arg. 
  arg5 = arg5 & ~t3;		// Blast the apply arg bit away 
  *(u32 *)&processor->control = arg5;   		// Reset the stored cr bit 
  goto b_apply_argument_supplied30465;   

/* end DoEntryRestNotAccepted */
  /* End of Halfword operand from stack instruction - DoEntryRestNotAccepted */
/* start VerifyGenericArity */


verifygenericarity:
  if (_trace) printf("verifygenericarity:\n");
  t11 = (2) << 16;   
  t11 = t11 & arg2;
  if (t11 == 0) 		// not applying 
    goto verify_generic_arity30467;
  arg1 = zero - arg5;   		// 4 - argsize 
  goto pullapplyargs;   

verify_generic_arity30467:
  if (_trace) printf("verify_generic_arity30467:\n");
  arg5 = 0;
  arg2 = 77;
  goto illegaloperand;

/* end VerifyGenericArity */
/* start PullApplyArgs */


pullapplyargs:
  if (_trace) printf("pullapplyargs:\n");
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t2 = *(s32 *)iSP;   
  t1 = *(s32 *)(iSP + 4);   
  t2 = (u32)t2;   
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t5 = (t4 == Type_List) ? 1 : 0;   

force_alignment30496:
  if (_trace) printf("force_alignment30496:\n");
  if (t5 == 0) 
    goto basic_dispatch30470;
  /* Here if argument TypeList */
  t5 = t2 - arg5;   		// Stack cache offset 
  t6 = ((u64)t5 < (u64)arg6) ? 1 : 0;   		// In range? 
  t4 = *(u64 *)&(processor->stackcachedata);   
  if (t6 == 0) 		// J. if not in cache 
    goto pull_apply_args30468;
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  t7 = zero;
  t5 = zero + 128;   
  t6 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t5 = t5 + arg1;		// Account for what we're about to push 
  t5 = (t5 * 8) + iSP;  		// SCA of desired end of cache 
  t6 = (arg6 * 8) + t6;  		// SCA of current end of cache 
  t10 = ((s64)t5 <= (s64)t6) ? 1 : 0;   
  if (t10 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check30477;
  iSP = iSP - 8;   		// Pop Stack. 
  goto pull_apply_args_quickly30476;   

pull_apply_args_quickly30471:
  if (_trace) printf("pull_apply_args_quickly30471:\n");
  t9 = *(s32 *)t4;   
  t8 = *(s32 *)(t4 + 4);   
  t9 = (u32)t9;   
  t7 = t7 + 1;
  t4 = t4 + 8;
  t5 = t8 & 192;		// Extract CDR code. 
  if (t5 != 0)   
    goto basic_dispatch30479;
  /* Here if argument 0 */
  t5 = t8 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  t5 = (t7 == arg1) ? 1 : 0;   
  if (t5 == 0) 
    goto pull_apply_args_quickly30471;
  goto pull_apply_args_quickly30472;   

basic_dispatch30479:
  if (_trace) printf("basic_dispatch30479:\n");
  t6 = (t5 == 64) ? 1 : 0;   

force_alignment30491:
  if (_trace) printf("force_alignment30491:\n");
  if (t6 == 0) 
    goto basic_dispatch30480;
  /* Here if argument 64 */
  t5 = t8 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;

pull_apply_args_quickly30474:
  if (_trace) printf("pull_apply_args_quickly30474:\n");
  t5 = *(s32 *)&processor->control;   
  t6 = t5 & 255;		// Get current arg size. 
  t5 = t5 & ~255L;
  t6 = t6 + t7;
  t5 = t6 + t5;		// Update the arg size 
  t6 = (2) << 16;   
  t5 = t5 & ~t6;		// turn off cr.apply 
  *(u32 *)&processor->control = t5;   
  iLP = (t7 * 8) + iLP;  
  goto INTERPRETINSTRUCTION;   

basic_dispatch30480:
  if (_trace) printf("basic_dispatch30480:\n");
  t6 = (t5 == 128) ? 1 : 0;   

force_alignment30492:
  if (_trace) printf("force_alignment30492:\n");
  if (t6 == 0) 
    goto basic_dispatch30481;
  /* Here if argument 128 */
  t5 = t8 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  t11 = *(s32 *)t4;   
  t10 = *(s32 *)(t4 + 4);   
  t11 = (u32)t11;   
  t5 = t10 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_List) ? 1 : 0;   

force_alignment30487:
  if (_trace) printf("force_alignment30487:\n");
  if (t6 == 0) 
    goto basic_dispatch30483;
  /* Here if argument TypeList */
  t5 = t11 - arg5;   		// Stack cache offset 
  t6 = ((u64)t5 < (u64)arg6) ? 1 : 0;   		// In range? 
  t4 = *(u64 *)&(processor->stackcachedata);   
  if (t6 == 0) 		// J. if not in cache 
    goto pull_apply_args_quickly30473;
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  goto pull_apply_args_quickly30476;   

basic_dispatch30483:
  if (_trace) printf("basic_dispatch30483:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment30488:
  if (_trace) printf("force_alignment30488:\n");
  if (t6 == 0) 
    goto basic_dispatch30484;
  /* Here if argument TypeNIL */
  goto pull_apply_args_quickly30474;   

basic_dispatch30484:
  if (_trace) printf("basic_dispatch30484:\n");
  /* Here for all other cases */

pull_apply_args_quickly30473:
  if (_trace) printf("pull_apply_args_quickly30473:\n");
  t5 = t10 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto pull_apply_args_quickly30475;   

basic_dispatch30482:
  if (_trace) printf("basic_dispatch30482:\n");

basic_dispatch30481:
  if (_trace) printf("basic_dispatch30481:\n");
  /* Here for all other cases */
  t7 = t7 - 1;   
  t4 = t4 - 8;   
  goto pull_apply_args_quickly30472;   

basic_dispatch30478:
  if (_trace) printf("basic_dispatch30478:\n");

pull_apply_args_quickly30476:
  t5 = (t7 == arg1) ? 1 : 0;   
  if (t5 == 0) 
    goto pull_apply_args_quickly30471;

pull_apply_args_quickly30472:
  if (_trace) printf("pull_apply_args_quickly30472:\n");
  /* Here if count=n, or bad cdr */
  /* Convert stack cache address to VMA */
  t5 = *(u64 *)&(processor->stackcachedata);   
  t5 = t4 - t5;   		// stack cache base relative offset 
  t5 = t5 >> 3;   		// convert byte address to word address 
  t9 = t5 + arg5;		// reconstruct VMA 
  t5 = Type_List;
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;

pull_apply_args_quickly30475:
  if (_trace) printf("pull_apply_args_quickly30475:\n");
  t5 = *(s32 *)&processor->control;   
  t6 = t5 & 255;		// Get current arg size. 
  t5 = t5 & ~255L;
  t6 = t6 + t7;
  t5 = t6 + t5;		// Update the arg size 
  *(u32 *)&processor->control = t5;   
  iLP = (t7 * 8) + iLP;  
  arg1 = arg1 - t7;   
  if ((s64)arg1 <= 0)  
    goto INTERPRETINSTRUCTION;
  goto pullapplyargsslowly;

basic_dispatch30470:
  if (_trace) printf("basic_dispatch30470:\n");
  t5 = (t4 == Type_NIL) ? 1 : 0;   

force_alignment30497:
  if (_trace) printf("force_alignment30497:\n");
  if (t5 == 0) 
    goto basic_dispatch30493;
  /* Here if argument TypeNIL */
  t6 = *(s32 *)&processor->control;   		// Get the control register 
  t7 = (2) << 16;   
  iSP = iSP - 8;   		// Discard that silly nil 
  t6 = t6 & ~t7;		// Blast away the apply arg bit. 
  *(u32 *)&processor->control = t6;   
  goto INTERPRETINSTRUCTION;   

basic_dispatch30493:
  if (_trace) printf("basic_dispatch30493:\n");
  /* Here for all other cases */
  arg1 = arg1;		// Pull apply args trap needs nargs in ARG1 
  goto pullapplyargstrap;

pull_apply_args30468:
  if (_trace) printf("pull_apply_args30468:\n");
  arg1 = arg1;
  goto pullapplyargsslowly;

basic_dispatch30469:
  if (_trace) printf("basic_dispatch30469:\n");

stack_cache_overflow_check30477:
  if (_trace) printf("stack_cache_overflow_check30477:\n");
  arg2 = arg1;
  goto stackcacheoverflowhandler;   

/* end PullApplyArgs */
/* start valuecell */

  /*  */
  /*  */
  /* Fullword instruction - valuecell */
  /* ======================= */

valuecell:
  if (_trace) printf("valuecell:\n");
  arg2 = (u32)arg3;   		// Get address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read30498:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30500;

vma_memory_read30499:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30502;

vma_memory_read30509:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t3 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;   		// Push the result 
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

vma_memory_read30502:
  if (_trace) printf("vma_memory_read30502:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30501;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30498;   

vma_memory_read30501:
  if (_trace) printf("vma_memory_read30501:\n");

vma_memory_read30500:
  if (_trace) printf("vma_memory_read30500:\n");
  r0 = (u64)&&return0002;
  goto memoryreaddatadecode;
return0002:
  goto vma_memory_read30509;   

/* end valuecell */
  /* End of Fullword instruction - valuecell */
  /* ============================== */
  /*  */
/* start pushconstantvalue */

  /*  */
  /*  */
  /* Fullword instruction - pushconstantvalue */
  /* ======================= */

pushconstantvalue:
  if (_trace) printf("pushconstantvalue:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)(iSP + 8) = arg3;   
  iSP = iSP + 8;
  goto cachevalid;   

/* end pushconstantvalue */
  /* End of Fullword instruction - pushconstantvalue */
  /* ============================== */
  /*  */
/* start DoZerop */

  /* Halfword operand from stack instruction - DoZerop */
  /* arg2 has the preloaded 8 bit operand. */

dozerop:
  if (_trace) printf("dozerop:\n");

DoZeropSP:
  if (_trace) printf("DoZeropSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoZeropLP:
  if (_trace) printf("DoZeropLP:\n");

DoZeropFP:
  if (_trace) printf("DoZeropFP:\n");

begindozerop:
  if (_trace) printf("begindozerop:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = *(s32 *)(arg1 + 4);   
  t12 = *(u64 *)&(processor->taddress);   
  t2 = *(s32 *)arg1;   
  LDS(1, f1, *(u32 *)arg1 );   
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t5 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment30515:
  if (_trace) printf("force_alignment30515:\n");
  if (t5 == 0) 
    goto basic_dispatch30511;
  /* Here if argument TypeFixnum */
  iPC = t6;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0)   		// T if predicate succeeds 
    t11 = t12;
  *(u64 *)(iSP + 8) = t11;   
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch30511:
  if (_trace) printf("basic_dispatch30511:\n");
  t5 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment30516:
  if (_trace) printf("force_alignment30516:\n");
  if (t5 == 0) 
    goto basic_dispatch30512;
  /* Here if argument TypeSingleFloat */
  iPC = t6;
  *(u64 *)(iSP + 8) = t12;   
  iSP = iSP + 8;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (FLTU64(1, f1) == 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch30512:
  if (_trace) printf("basic_dispatch30512:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch30510:
  if (_trace) printf("basic_dispatch30510:\n");

DoZeropIM:
  if (_trace) printf("DoZeropIM:\n");
  t2 = *(u64 *)&(processor->taddress);   
  iSP = iSP + 8;
  t1 = *(u64 *)&(processor->niladdress);   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (arg2 == 0)   
    t1 = t2;
  *(u64 *)iSP = t1;   		// yes Virginia, we dual issue with above yahoo 
  goto cachevalid;   

/* end DoZerop */
  /* End of Halfword operand from stack instruction - DoZerop */
/* start DoSetSpToAddress */

  /* Halfword operand from stack instruction - DoSetSpToAddress */
  /* arg2 has the preloaded 8 bit operand. */

dosetsptoaddress:
  if (_trace) printf("dosetsptoaddress:\n");

DoSetSpToAddressSP:
  if (_trace) printf("DoSetSpToAddressSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetSpToAddressLP:
  if (_trace) printf("DoSetSpToAddressLP:\n");

DoSetSpToAddressFP:
  if (_trace) printf("DoSetSpToAddressFP:\n");

begindosetsptoaddress:
  if (_trace) printf("begindosetsptoaddress:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = arg1;		// Set iSP=address of operand 
  goto cachevalid;   

DoSetSpToAddressIM:
  goto doistageerror;

/* end DoSetSpToAddress */
  /* End of Halfword operand from stack instruction - DoSetSpToAddress */
/* start DoEq */

  /* Halfword operand from stack instruction - DoEq */
  /* arg2 has the preloaded 8 bit operand. */

doeq:
  if (_trace) printf("doeq:\n");

DoEqSP:
  if (_trace) printf("DoEqSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoeq;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoEqLP:
  if (_trace) printf("DoEqLP:\n");

DoEqFP:
  if (_trace) printf("DoEqFP:\n");

begindoeq:
  if (_trace) printf("begindoeq:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  arg3 = arg3 >> 12;   
  t12 = *(u64 *)&(processor->taddress);   
  arg1 = *(u64 *)arg1;   		// load op2 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  arg3 = arg3 & 1;		// 1 if no-pop, 0 if pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t3 = arg6 ^ arg1;   		// compare tag and data 
  t3 = t3 << 26;   		// shift off the cdr code 
  iSP = (arg3 * 8) + iSP;  		// Either a stack-push or a stack-write 
  if (t3 == 0)   		// pick up T or NIL 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

/* end DoEq */
  /* End of Halfword operand from stack instruction - DoEq */
/* start DoAref1 */

  /* Halfword operand from stack instruction - DoAref1 */
  /* arg2 has the preloaded 8 bit operand. */

doaref1:
  if (_trace) printf("doaref1:\n");

DoAref1SP:
  if (_trace) printf("DoAref1SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto headdoaref1;
  arg1 = arg6;		// SP-pop mode, TOS->arg1 
  arg6 = *(u64 *)arg4;   		// Reload TOS 
  iSP = arg4;		// Adjust SP 
  goto begindoaref1;   

DoAref1LP:
  if (_trace) printf("DoAref1LP:\n");

DoAref1FP:
  if (_trace) printf("DoAref1FP:\n");

headdoaref1:
  if (_trace) printf("headdoaref1:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoaref1:
  if (_trace) printf("begindoaref1:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (u32)arg6;   		// Get the array tag/data 
  arg2 = (s32)arg1 + (s32)0;		// (sign-extended, for fast bounds check) Index Data 
  t8 = zero + AutoArrayRegMask;   
  t8 = arg4 & t8;
  arg1 = arg1 >> 32;   		// Index Tag 
  t7 = (u64)&processor->ac0array;   
  t7 = t7 + t8;		// This is the address if the array register block. 
  t1 = arg1 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto aref1illegal;

aref1merge:
  if (_trace) printf("aref1merge:\n");
  if (arg4 == 0) 
    goto aref1regset;
  t8 = *(u64 *)&(((ARRAYCACHEP)t7)->array);   		// Cached array object. 
  t1 = arg3 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto reallyaref1exc;
  t8 = (arg4 == t8) ? 1 : 0;   		// t8==1 iff cached array is ours. 
  if (t8 == 0) 		// Go and setup the array register. 
    goto aref1regset;
#ifdef SLOWARRAYS
  goto aref1regset;   
#endif
  arg6 = *(u64 *)&(((ARRAYCACHEP)t7)->arword);   
  t9 = *(u64 *)&(((ARRAYCACHEP)t7)->locat);   		// high order bits all zero 
  t3 = *(u64 *)&(((ARRAYCACHEP)t7)->length);   		// high order bits all zero 
  t5 = arg6 << 42;   
  t4 = *(u64 *)&(processor->areventcount);   
  t5 = t5 >> 42;   
  t2 = ((u64)arg2 < (u64)t3) ? 1 : 0;   
  t6 = t4 - t5;   
  if (t6 != 0)   		// J. if event count ticked. 
    goto aref1regset;
  if (t2 == 0) 
    goto aref1bounds;
  arg5 = arg6 >> (Array_RegisterBytePackingPos & 63);   
  arg4 = arg6 >> (Array_RegisterByteOffsetPos & 63);   
  t8 = arg6 >> (Array_RegisterElementTypePos & 63);   
  arg4 = arg4 & Array_RegisterByteOffsetMask;
  arg5 = arg5 & Array_RegisterBytePackingMask;
  arg6 = t8 & Array_RegisterElementTypeMask;

aref1restart:
  if (_trace) printf("aref1restart:\n");
  if (arg5 != 0)   
    goto new_aref_1_internal30517;
  t1 = t9 + arg2;

new_aref_1_internal30518:
  if (_trace) printf("new_aref_1_internal30518:\n");
  /* Memory Read Internal */

vma_memory_read30525:
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t5 = t1 + ivory;
  t3 = *(s32 *)&processor->scovlimit;   
  t9 = (t5 * 4);   
  arg3 = LDQ_U(t5);   
  t2 = t1 - t2;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->dataread_mask);   
  t3 = ((u64)t2 < (u64)t3) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  arg3 = (u8)(arg3 >> ((t5&7)*8));   
  if (t3 != 0)   
    goto vma_memory_read30527;

vma_memory_read30526:
  t5 = zero + 240;   
  t6 = t6 >> (arg3 & 63);   
  t5 = t5 >> (arg3 & 63);   
  t9 = (u32)t9;   
  if (t6 & 1)   
    goto vma_memory_read30529;

vma_memory_read30536:
  if (arg5 != 0)   
    goto new_aref_1_internal30519;

new_aref_1_internal30520:
  if (_trace) printf("new_aref_1_internal30520:\n");
  r31 = r31 | r31;
  t1 = arg6 - 2;   
  if ((s64)t1 <= 0)  
    goto new_aref_1_internal30521;
  /* TagType. */
  arg3 = arg3 & 63;

new_aref_1_internal30522:
  if (_trace) printf("new_aref_1_internal30522:\n");
  *(u32 *)(iSP + 4) = arg3;   
  t5 = (arg5 == 0) ? 1 : 0;   
  if (t5 == 0) 
    goto case_others_182;

case_0_176:
  if (_trace) printf("case_0_176:\n");
  r31 = r31 | r31;
  if (t1 == 0) 
    goto new_aref_1_internal30523;
  *(u32 *)iSP = t9;   
  goto NEXTINSTRUCTION;   

case_2_177:
  if (_trace) printf("case_2_177:\n");
  /* AREF1-8B */
  r31 = r31 | r31;
  t5 = arg2 & 3;
  t6 = (u8)(t9 >> ((t5&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal30523;
  *(u32 *)iSP = t6;   
  goto NEXTINSTRUCTION;   

case_3_178:
  if (_trace) printf("case_3_178:\n");
  /* AREF1-4B */
  r31 = r31 | r31;
  t5 = arg2 & 7;		// byte-index 
  t5 = t5 << 2;   		// byte-position 
  t6 = t9 >> (t5 & 63);   		// byte in position 
  t6 = t6 & 15;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal30523;
  *(u32 *)iSP = t6;   
  goto NEXTINSTRUCTION;   

case_5_179:
  if (_trace) printf("case_5_179:\n");
  /* AREF1-1B */
  r31 = r31 | r31;
  t5 = arg2 & 31;		// byte-index 
  r31 = r31 | r31;
  t6 = t9 >> (t5 & 63);   		// byte in position 
  t6 = t6 & 1;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal30523;
  *(u32 *)iSP = t6;   
  goto NEXTINSTRUCTION;   

case_1_180:
  if (_trace) printf("case_1_180:\n");
  /* AREF1-16B */
  t5 = arg2 & 1;
  t5 = t5 + t5;		// Bletch, it's a byte ref 
  t6 = (u16)(t9 >> ((t5&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal30523;
  *(u32 *)iSP = t6;   
  goto NEXTINSTRUCTION;   

case_others_182:
  if (_trace) printf("case_others_182:\n");
  r31 = r31 | r31;
  t5 = (arg5 == 2) ? 1 : 0;   
  t6 = (arg5 == 3) ? 1 : 0;   
  if (t5 != 0)   
    goto case_2_177;
  t5 = (arg5 == 5) ? 1 : 0;   
  if (t6 != 0)   
    goto case_3_178;
  t6 = (arg5 == 1) ? 1 : 0;   
  if (t5 != 0)   
    goto case_5_179;
  if (t6 != 0)   
    goto case_1_180;

case_4_181:
  if (_trace) printf("case_4_181:\n");
  /* AREF1-2B */
  r31 = r31 | r31;
  t5 = arg2 & 15;		// byte-index 
  t5 = t5 << 1;   		// byte-position 
  t6 = t9 >> (t5 & 63);   		// byte in position 
  t6 = t6 & 3;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal30523;
  *(u32 *)iSP = t6;   
  goto NEXTINSTRUCTION;   

new_aref_1_internal30517:
  if (_trace) printf("new_aref_1_internal30517:\n");
  arg2 = arg4 + arg2;
  t1 = arg2 >> (arg5 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  goto new_aref_1_internal30518;   

new_aref_1_internal30519:
  if (_trace) printf("new_aref_1_internal30519:\n");
  t1 = arg3 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto new_aref_1_internal30524;
  goto new_aref_1_internal30520;   

new_aref_1_internal30521:
  if (_trace) printf("new_aref_1_internal30521:\n");
  arg3 = Type_Character;
  if (arg6 & 1)   
    goto new_aref_1_internal30522;
  arg3 = Type_Fixnum;
  if (arg6 == 0) 
    goto new_aref_1_internal30522;
  t2 = *(u64 *)&(processor->niladdress);   
  t3 = *(u64 *)&(processor->taddress);   
  goto new_aref_1_internal30522;   

new_aref_1_internal30523:
  if (_trace) printf("new_aref_1_internal30523:\n");
  if (t6)   
    t2 = t3;
  *(u64 *)iSP = t2;   
  goto NEXTINSTRUCTION;   

new_aref_1_internal30524:
  if (_trace) printf("new_aref_1_internal30524:\n");
  arg5 = t1;
  arg2 = 25;
  goto illegaloperand;

DoAref1IM:
  if (_trace) printf("DoAref1IM:\n");
  t8 = zero + AutoArrayRegMask;   
  arg4 = *(s32 *)iSP;   		// Get the array tag/data 
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  t7 = (u64)&processor->ac0array;   
  t8 = arg4 & t8;
  t7 = t7 + t8;		// This is the address of the array register block. 
  goto aref1merge;   

vma_memory_read30527:
  if (_trace) printf("vma_memory_read30527:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg3 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read30526;   

vma_memory_read30529:
  if (_trace) printf("vma_memory_read30529:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read30528;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read30525;   

vma_memory_read30528:
  if (_trace) printf("vma_memory_read30528:\n");
  t6 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read30533:
  if (_trace) printf("vma_memory_read30533:\n");
  t5 = t6 & MemoryActionTransform;
  if (t5 == 0) 
    goto vma_memory_read30532;
  arg3 = arg3 & ~63L;
  arg3 = arg3 | Type_ExternalValueCellPointer;
  goto vma_memory_read30536;   

vma_memory_read30532:

vma_memory_read30531:
  /* Perform memory action */
  arg1 = t6;
  arg2 = 0;
  goto performmemoryaction;

/* end DoAref1 */
  /* End of Halfword operand from stack instruction - DoAref1 */
/* start DoTypeMember */

  /* Halfword 10 bit immediate instruction - DoTypeMember */

dotypemember:
  if (_trace) printf("dotypemember:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoTypeMemberIM:
  if (_trace) printf("DoTypeMemberIM:\n");

DoTypeMemberSP:
  if (_trace) printf("DoTypeMemberSP:\n");

DoTypeMemberLP:
  if (_trace) printf("DoTypeMemberLP:\n");

DoTypeMemberFP:
  if (_trace) printf("DoTypeMemberFP:\n");
  /* arg1 has operand preloaded. */
  t6 = arg3 >> 6;   		// Position the opcode 
  t4 = *(u64 *)&(processor->taddress);   
  arg4 = *(s32 *)(iSP + 4);   		// get op1's tag 
  t1 = 1;
  t5 = *(u64 *)&(processor->niladdress);   
  t7 = arg3 >> 12;   		// Get pop-bit while stalled 
  arg1 = t6 & 60;		// Get field-number*4 from the opcode 
  /* TagType. */
  arg4 = arg4 & 63;		// Strip off CDR code. 
  t1 = t1 << (arg4 & 63);   		// T1 is type type code bit position. 
  t7 = t7 & 1;		// Pop bit 
  t2 = arg2 << (arg1 & 63);   		// t2 is the mask. 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = (t7 * 8) + iSP;  
  t3 = t2 & t1;		// t3 is the result. 

force_alignment30537:
  if (_trace) printf("force_alignment30537:\n");
  if (t3)   
    t5 = t4;
  *(u64 *)iSP = t5;   
  goto cachevalid;   

/* end DoTypeMember */
  /* End of Halfword operand from stack instruction - DoTypeMember */
/* start DoPointerPlus */

  /* Halfword operand from stack instruction - DoPointerPlus */
  /* arg2 has the preloaded 8 bit operand. */

dopointerplus:
  if (_trace) printf("dopointerplus:\n");

DoPointerPlusSP:
  if (_trace) printf("DoPointerPlusSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindopointerplus;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoPointerPlusLP:
  if (_trace) printf("DoPointerPlusLP:\n");

DoPointerPlusFP:
  if (_trace) printf("DoPointerPlusFP:\n");

begindopointerplus:
  if (_trace) printf("begindopointerplus:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t2 = *(s32 *)arg1;   		// Get the data of op2 
  t3 = (s32)arg6 + (s32)t2;		// (%32-bit-plus (data arg1) (data arg2)) 
  *(u32 *)iSP = t3;   		// Put result back on the stack 
  goto cachevalid;   

DoPointerPlusIM:
  if (_trace) printf("DoPointerPlusIM:\n");
  t2 = arg2 << 56;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t2 = (s64)t2 >> 56;   

force_alignment30538:
  if (_trace) printf("force_alignment30538:\n");
  t3 = (s32)arg6 + (s32)t2;		// (%32-bit-plus (data arg1) (data arg2)) 
  *(u32 *)iSP = t3;   		// Put result back on the stack 
  goto cachevalid;   

/* end DoPointerPlus */
  /* End of Halfword operand from stack instruction - DoPointerPlus */
/* start DoLdb */

  /* Field Extraction instruction - DoLdb */

doldb:
  if (_trace) printf("doldb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoLdbIM:
  if (_trace) printf("DoLdbIM:\n");

DoLdbSP:
  if (_trace) printf("DoLdbSP:\n");

DoLdbLP:
  if (_trace) printf("DoLdbLP:\n");

DoLdbFP:
  if (_trace) printf("DoLdbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (u32)arg6;   		// get ARG1 tag/data 
  /* TagType. */
  t8 = arg3 & 63;
  t9 = t8 - Type_Fixnum;   
  t3 = arg4 << (arg2 & 63);   		// Shift ARG1 left to get new high bits 
  if (t9 != 0)   		// Not a fixnum 
    goto ldbexception;
  t7 = zero + -2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t6 = (u32)(t3 >> ((4&7)*8));   		// Get new low bits 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = t7 << (arg1 & 63);   		// Unmask 
  t3 = t3 | t6;		// Glue two parts of shifted operand together 
  *(u32 *)(iSP + 4) = t8;   		// T8 is TypeFixnum from above 
  t3 = t3 & ~t7;		// T3= masked value. 
  *(u32 *)iSP = t3;   
  goto cachevalid;   

/* end DoLdb */
  /* End of Halfword operand from stack instruction - DoLdb */
/* start DoSetSpToAddressSaveTos */

  /* Halfword operand from stack instruction - DoSetSpToAddressSaveTos */
  /* arg2 has the preloaded 8 bit operand. */

dosetsptoaddresssavetos:
  if (_trace) printf("dosetsptoaddresssavetos:\n");

DoSetSpToAddressSaveTosSP:
  if (_trace) printf("DoSetSpToAddressSaveTosSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindosetsptoaddresssavetos;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoSetSpToAddressSaveTosLP:
  if (_trace) printf("DoSetSpToAddressSaveTosLP:\n");

DoSetSpToAddressSaveTosFP:
  if (_trace) printf("DoSetSpToAddressSaveTosFP:\n");

begindosetsptoaddresssavetos:
  if (_trace) printf("begindosetsptoaddresssavetos:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = arg1;		// Set the stack top as specified. 
  *(u64 *)arg1 = arg6;   		// Restore the TOS. 
  goto cachevalid;   

DoSetSpToAddressSaveTosIM:
  goto doistageerror;

/* end DoSetSpToAddressSaveTos */
  /* End of Halfword operand from stack instruction - DoSetSpToAddressSaveTos */
/* start DoPop */

  /* Halfword operand from stack instruction - DoPop */
  /* arg2 has the preloaded 8 bit operand. */

dopop:
  if (_trace) printf("dopop:\n");

DoPopSP:
  if (_trace) printf("DoPopSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindopop;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoPopLP:
  if (_trace) printf("DoPopLP:\n");

DoPopFP:
  if (_trace) printf("DoPopFP:\n");

begindopop:
  if (_trace) printf("begindopop:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   		// Pop Stack. 
  *(u64 *)arg1 = arg6;   		// Store all 40 bits on stack 
  goto cachevalid;   

DoPopIM:
  goto doistageerror;

/* end DoPop */
  /* End of Halfword operand from stack instruction - DoPop */
/* start DoMovem */

  /* Halfword operand from stack instruction - DoMovem */
  /* arg2 has the preloaded 8 bit operand. */

domovem:
  if (_trace) printf("domovem:\n");

DoMovemSP:
  if (_trace) printf("DoMovemSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindomovem;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoMovemLP:
  if (_trace) printf("DoMovemLP:\n");

DoMovemFP:
  if (_trace) printf("DoMovemFP:\n");

begindomovem:
  if (_trace) printf("begindomovem:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)arg1 = arg6;   		// Store all 40 bits of TOS on stack 
  goto cachevalid;   

DoMovemIM:
  goto doistageerror;

/* end DoMovem */
  /* End of Halfword operand from stack instruction - DoMovem */
/* start DoPushAddress */

  /* Halfword operand from stack instruction - DoPushAddress */
  /* arg2 has the preloaded 8 bit operand. */

dopushaddress:
  if (_trace) printf("dopushaddress:\n");

DoPushAddressSP:
  if (_trace) printf("DoPushAddressSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushAddressLP:
  if (_trace) printf("DoPushAddressLP:\n");

DoPushAddressFP:
  if (_trace) printf("DoPushAddressFP:\n");

begindopushaddress:
  if (_trace) printf("begindopushaddress:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = arg1 - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t3 = Type_Locative;
  *(u32 *)(iSP + 8) = t1;   
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

DoPushAddressIM:
  goto doistageerror;

/* end DoPushAddress */
  /* End of Halfword operand from stack instruction - DoPushAddress */
/* start DoMemoryRead */

  /* Halfword 10 bit immediate instruction - DoMemoryRead */

domemoryread:
  if (_trace) printf("domemoryread:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoMemoryReadIM:
  if (_trace) printf("DoMemoryReadIM:\n");

DoMemoryReadSP:
  if (_trace) printf("DoMemoryReadSP:\n");

DoMemoryReadLP:
  if (_trace) printf("DoMemoryReadLP:\n");

DoMemoryReadFP:
  if (_trace) printf("DoMemoryReadFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  t1 = arg3 >> 10;   		// Low bit clear if memory-read, set if memory-read-address 
  t2 = arg1 & 32;		// T2 = fixnum check 
  t3 = arg1 & 16;		// T3 = reset CDR code 
  arg3 = arg1 >> 6;   		// arg3 = cycle type 
  arg1 = (u32)(arg6 >> ((4&7)*8));   
  arg2 = (u32)arg6;   		// Get tag/data 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read30539:
  t7 = arg2 + ivory;
  t8 = (arg3 * 4);   		// Cycle-number -> table offset 
  arg5 = LDQ_U(t7);   
  t8 = (t8 * 4) + ivory;   
  arg6 = (t7 * 4);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)(t8 + PROCESSORSTATE_DATAREAD_MASK);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30541;

vma_memory_read30540:
  t8 = t8 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30543;

vma_memory_read30550:
  if (t2 == 0) 		// J. if no check for fixnum. 
    goto mrdataok;
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto mrnotfixnum;

mrdataok:
  if (_trace) printf("mrdataok:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (t1 & 1)   		// Get original tag if memory-read-address 
   arg5 = arg1;
  if (t3 == 0) 		// J. if no reset CDR code 
    goto mrcdrunch;
  /* TagType. */
  arg5 = arg5 & 63;

mrcdrunch:
  if (_trace) printf("mrcdrunch:\n");
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t1 & 1)   		// Get forwarded address if memory-read-address 
   arg6 = arg2;
  *(u32 *)iSP = arg6;   
  *(u32 *)(iSP + 4) = arg5;   		// write the stack cache 
  goto cachevalid;   

mrnotfixnum:
  if (_trace) printf("mrnotfixnum:\n");
  arg5 = 0;
  arg2 = 5;
  goto illegaloperand;

vma_memory_read30543:
  if (_trace) printf("vma_memory_read30543:\n");

vma_memory_read30541:
  if (_trace) printf("vma_memory_read30541:\n");
  r0 = (u64)&&return0003;
  goto memoryreadgeneraldecode;
return0003:
  goto vma_memory_read30550;   

/* end DoMemoryRead */
  /* End of Halfword operand from stack instruction - DoMemoryRead */
/* start DoBranch */

  /* Halfword 10 bit immediate instruction - DoBranch */

dobranch:
  if (_trace) printf("dobranch:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchIM:
  if (_trace) printf("DoBranchIM:\n");

DoBranchSP:
  if (_trace) printf("DoBranchSP:\n");

DoBranchLP:
  if (_trace) printf("DoBranchLP:\n");

DoBranchFP:
  if (_trace) printf("DoBranchFP:\n");
  arg1 = (s64)arg3 >> 48;   
  /* arg1 has signed operand preloaded. */
#ifndef CACHEMETERING
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
#endif
  iPC = iPC + arg1;		// Update the PC in halfwords 
#ifndef CACHEMETERING
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
#endif
  goto interpretinstructionforbranch;   

/* end DoBranch */
  /* End of Halfword operand from stack instruction - DoBranch */
/* start DoGenericDispatch */

  /* Halfword operand from stack instruction - DoGenericDispatch */
  /* arg2 has the preloaded 8 bit operand. */

dogenericdispatch:
  if (_trace) printf("dogenericdispatch:\n");

DoGenericDispatchSP:
  if (_trace) printf("DoGenericDispatchSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoGenericDispatchLP:
  if (_trace) printf("DoGenericDispatchLP:\n");

DoGenericDispatchFP:
  if (_trace) printf("DoGenericDispatchFP:\n");

begindogenericdispatch:
  if (_trace) printf("begindogenericdispatch:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg2 = *(s32 *)&processor->control;   
  arg1 = *(s32 *)(iFP + 20);   		// get generic tag and data 
  t1 = *(s32 *)(iFP + 16);   
  arg5 = arg2 & 255;		// get number of arguments 
  arg3 = *(s32 *)(iFP + 28);   		// get instance tag and data 
  arg4 = *(s32 *)(iFP + 24);   
  arg5 = arg5 - 4;   		// done if 2 or more arguments (plus 2 extra words) 
  if ((s64)arg5 < 0)   
    goto verifygenericarity;
  t1 = (u32)t1;   
  arg4 = (u32)arg4;   
  r0 = (u64)&&return0004;
  goto lookuphandler;
return0004:
  t3 = t4 - Type_EvenPC;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto generic_dispatch30552;
  t3 = t6 & 63;		// Strip CDR code 
  t3 = t3 - Type_NIL;   
  if (t3 == 0) 
    goto generic_dispatch30551;
  *(u32 *)(iFP + 16) = t7;   
  *(u32 *)(iFP + 20) = t6;   		// write the stack cache 

generic_dispatch30551:
  if (_trace) printf("generic_dispatch30551:\n");
  /* Convert real continuation to PC. */
  iPC = t4 & 1;
  iPC = t9 + iPC;
  iPC = t9 + iPC;
  goto interpretinstructionforjump;   

generic_dispatch30552:
  if (_trace) printf("generic_dispatch30552:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = iSP - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t3 = t2 + t3;		// reconstruct VMA 
  arg5 = t3;
  arg2 = 37;
  goto illegaloperand;

DoGenericDispatchIM:
  goto doistageerror;

/* end DoGenericDispatch */
  /* End of Halfword operand from stack instruction - DoGenericDispatch */
/* start LookupHandler */


lookuphandler:
  if (_trace) printf("lookuphandler:\n");
  sp = sp + -8;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t5 = arg3 - Type_Instance;   
  t5 = t5 & 60;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto instance_descriptor_info30556;
  arg2 = arg4;		// Don't clobber instance if it's forwarded 
  /* Memory Read Internal */

vma_memory_read30557:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30559;

vma_memory_read30558:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30561;

vma_memory_read30566:

instance_descriptor_info30555:
  if (_trace) printf("instance_descriptor_info30555:\n");
  arg2 = arg6;
  /* Memory Read Internal */

vma_memory_read30567:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30569;

vma_memory_read30568:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30571;

vma_memory_read30578:
  t2 = arg6;
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto instance_descriptor_info30553;
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read30579:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30581;

vma_memory_read30580:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30583;

vma_memory_read30590:
  t3 = arg6;
  t5 = arg5 - Type_Locative;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto instance_descriptor_info30554;
  arg2 = t2 & t1;
  t5 = arg2 << 1;   
  arg4 = arg2 + t5;		// (* (logand mask data) 3) 
  /* TagType. */
  arg1 = arg1 & 63;

lookup_handler30592:
  if (_trace) printf("lookup_handler30592:\n");
  arg2 = t3 + arg4;
  arg4 = arg4 + 3;
  /* Read key */
  /* Memory Read Internal */

vma_memory_read30593:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30595;

vma_memory_read30594:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30597;

vma_memory_read30604:
  /* TagType. */
  arg5 = arg5 & 63;
  t5 = (arg5 == Type_NIL) ? 1 : 0;   
  if (t5 != 0)   
    goto lookup_handler30591;
  t5 = (arg1 == arg5) ? 1 : 0;   
  if (t5 == 0) 
    goto lookup_handler30592;
  t5 = (s32)t1 - (s32)arg6;   
  if (t5 != 0)   
    goto lookup_handler30592;

lookup_handler30591:
  if (_trace) printf("lookup_handler30591:\n");
  /* Read method */
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read30605:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30607;

vma_memory_read30606:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30609;

vma_memory_read30616:
  t4 = arg5;
  arg3 = arg6;
  /* Read parameter */
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read30617:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30619;

vma_memory_read30618:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30621;

vma_memory_read30628:
  t6 = arg5;
  t7 = arg6;
  t9 = arg3;
  sp = sp + 8;   
  goto *r0; /* ret */

vma_memory_read30621:
  if (_trace) printf("vma_memory_read30621:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30620;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30617;   

vma_memory_read30620:
  if (_trace) printf("vma_memory_read30620:\n");

vma_memory_read30619:
  if (_trace) printf("vma_memory_read30619:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0005;
  goto memoryreaddatadecode;
return0005:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30628;   

vma_memory_read30609:
  if (_trace) printf("vma_memory_read30609:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30608;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30605;   

vma_memory_read30608:
  if (_trace) printf("vma_memory_read30608:\n");

vma_memory_read30607:
  if (_trace) printf("vma_memory_read30607:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0006;
  goto memoryreaddatadecode;
return0006:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30616;   

vma_memory_read30597:
  if (_trace) printf("vma_memory_read30597:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30596;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30593;   

vma_memory_read30596:
  if (_trace) printf("vma_memory_read30596:\n");

vma_memory_read30595:
  if (_trace) printf("vma_memory_read30595:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0007;
  goto memoryreaddatadecode;
return0007:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30604;   

vma_memory_read30583:
  if (_trace) printf("vma_memory_read30583:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30582;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30579;   

vma_memory_read30582:
  if (_trace) printf("vma_memory_read30582:\n");

vma_memory_read30581:
  if (_trace) printf("vma_memory_read30581:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0008;
  goto memoryreaddatadecode;
return0008:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30590;   

vma_memory_read30571:
  if (_trace) printf("vma_memory_read30571:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30570;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30567;   

vma_memory_read30570:
  if (_trace) printf("vma_memory_read30570:\n");

vma_memory_read30569:
  if (_trace) printf("vma_memory_read30569:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0009;
  goto memoryreaddatadecode;
return0009:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30578;   

vma_memory_read30561:
  if (_trace) printf("vma_memory_read30561:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30560;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30557;   

vma_memory_read30560:
  if (_trace) printf("vma_memory_read30560:\n");

vma_memory_read30559:
  if (_trace) printf("vma_memory_read30559:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0010;
  goto memoryreadheaderdecode;
return0010:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30566;   

instance_descriptor_info30556:
  if (_trace) printf("instance_descriptor_info30556:\n");
  /* not an instance, flavor description comes from magic vector */
  arg2 = *(u64 *)&(processor->trapvecbase);   
  /* TagType. */
  t5 = arg3 & 63;
  arg2 = arg2 + 2560;   
  arg2 = t5 + arg2;
  /* Memory Read Internal */

vma_memory_read30629:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30631;

vma_memory_read30630:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30633;
  goto instance_descriptor_info30555;   

instance_descriptor_info30553:
  if (_trace) printf("instance_descriptor_info30553:\n");
  arg5 = arg2;
  arg2 = 34;
  goto illegaloperand;

instance_descriptor_info30554:
  if (_trace) printf("instance_descriptor_info30554:\n");
  arg5 = arg2;
  arg2 = 35;
  goto illegaloperand;

vma_memory_read30633:
  if (_trace) printf("vma_memory_read30633:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30632;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30629;   

vma_memory_read30632:
  if (_trace) printf("vma_memory_read30632:\n");

vma_memory_read30631:
  if (_trace) printf("vma_memory_read30631:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0011;
  goto memoryreaddatadecode;
return0011:
  r0 = *(u64 *)sp;   
  goto instance_descriptor_info30555;   

/* end LookupHandler */
/* start DoSetTag */

  /* Halfword operand from stack instruction - DoSetTag */
  /* arg2 has the preloaded 8 bit operand. */

dosettag:
  if (_trace) printf("dosettag:\n");

DoSetTagSP:
  if (_trace) printf("DoSetTagSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetTagLP:
  if (_trace) printf("DoSetTagLP:\n");

DoSetTagFP:
  if (_trace) printf("DoSetTagFP:\n");

begindosettag:
  if (_trace) printf("begindosettag:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)(arg1 + 4);   		// Get tag/data of op2 
  arg2 = *(s32 *)arg1;   
  t3 = t1 - Type_Fixnum;   
  t3 = t3 & 63;		// Strip CDR code 
  if (t3 != 0)   
    goto settagexc;

DoSetTagIM:
  if (_trace) printf("DoSetTagIM:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + 4) = arg2;   		// Set TAG of op1 
  goto cachevalid;   

settagexc:
  if (_trace) printf("settagexc:\n");
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

/* end DoSetTag */
  /* End of Halfword operand from stack instruction - DoSetTag */
/* start DoCar */

  /* Halfword operand from stack instruction - DoCar */
  /* arg2 has the preloaded 8 bit operand. */

docar:
  if (_trace) printf("docar:\n");

DoCarSP:
  if (_trace) printf("DoCarSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoCarLP:
  if (_trace) printf("DoCarLP:\n");

DoCarFP:
  if (_trace) printf("DoCarFP:\n");

begindocar:
  if (_trace) printf("begindocar:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg5 = *(s32 *)(arg1 + 4);   		// Get the operand from the stack. 
  arg6 = *(s32 *)arg1;   
  r0 = (u64)&&return0012;
  goto carinternal;
return0012:
  t5 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

DoCarIM:
  goto doistageerror;

/* end DoCar */
  /* End of Halfword operand from stack instruction - DoCar */
/* start CarInternal */


carinternal:
  if (_trace) printf("carinternal:\n");
  sp = sp + -8;   
  arg2 = (u32)(arg6 >> ((zero&7)*8));   
  t5 = arg5 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_List) ? 1 : 0;   

force_alignment30660:
  if (_trace) printf("force_alignment30660:\n");
  if (t6 == 0) 
    goto basic_dispatch30643;
  /* Here if argument TypeList */

car_internal30640:
  /* Memory Read Internal */

vma_memory_read30644:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30646;

vma_memory_read30645:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30648;

vma_memory_read30655:

basic_dispatch30642:
  if (_trace) printf("basic_dispatch30642:\n");

car_internal30641:
  if (_trace) printf("car_internal30641:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch30643:
  if (_trace) printf("basic_dispatch30643:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment30661:
  if (_trace) printf("force_alignment30661:\n");
  if (t6 != 0)   
    goto basic_dispatch30642;

basic_dispatch30656:
  if (_trace) printf("basic_dispatch30656:\n");
  t6 = (t5 == Type_Locative) ? 1 : 0;   

force_alignment30662:
  if (_trace) printf("force_alignment30662:\n");
  if (t6 != 0)   
    goto car_internal30640;

basic_dispatch30657:
  if (_trace) printf("basic_dispatch30657:\n");
  /* Here for all other cases */
  arg6 = arg5;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

vma_memory_read30648:
  if (_trace) printf("vma_memory_read30648:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30647;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30644;   

vma_memory_read30647:
  if (_trace) printf("vma_memory_read30647:\n");

vma_memory_read30646:
  if (_trace) printf("vma_memory_read30646:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0013;
  goto memoryreaddatadecode;
return0013:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30655;   

/* end CarInternal */
/* start DoCdr */

  /* Halfword operand from stack instruction - DoCdr */
  /* arg2 has the preloaded 8 bit operand. */

docdr:
  if (_trace) printf("docdr:\n");

DoCdrSP:
  if (_trace) printf("DoCdrSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoCdrLP:
  if (_trace) printf("DoCdrLP:\n");

DoCdrFP:
  if (_trace) printf("DoCdrFP:\n");

begindocdr:
  if (_trace) printf("begindocdr:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg5 = *(s32 *)(arg1 + 4);   		// Get the operand from the stack. 
  arg6 = *(s32 *)arg1;   
  r0 = (u64)&&return0014;
  goto cdrinternal;
return0014:
  t5 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

DoCdrIM:
  goto doistageerror;

/* end DoCdr */
  /* End of Halfword operand from stack instruction - DoCdr */
/* start CdrInternal */


cdrinternal:
  if (_trace) printf("cdrinternal:\n");
  sp = sp + -8;   
  arg2 = (u32)arg6;   
  t5 = arg5 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_List) ? 1 : 0;   

force_alignment30698:
  if (_trace) printf("force_alignment30698:\n");
  if (t6 == 0) 
    goto basic_dispatch30666;
  /* Here if argument TypeList */
  /* Memory Read Internal */

vma_memory_read30667:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30669;

vma_memory_read30668:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30671;

vma_memory_read30676:
  t5 = arg5 & 192;		// Extract CDR code. 
  if (t5 != 0)   
    goto basic_dispatch30678;
  /* Here if argument 0 */
  arg6 = arg2 + 1;		// Address of next position is CDR 
  arg5 = Type_List;

basic_dispatch30677:
  if (_trace) printf("basic_dispatch30677:\n");

basic_dispatch30665:
  if (_trace) printf("basic_dispatch30665:\n");

cdr_internal30664:
  if (_trace) printf("cdr_internal30664:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch30666:
  if (_trace) printf("basic_dispatch30666:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment30699:
  if (_trace) printf("force_alignment30699:\n");
  if (t6 != 0)   
    goto basic_dispatch30665;

basic_dispatch30694:
  if (_trace) printf("basic_dispatch30694:\n");
  t6 = (t5 == Type_Locative) ? 1 : 0;   

force_alignment30700:
  if (_trace) printf("force_alignment30700:\n");
  if (t6 != 0)   
    goto cdr_internal30663;

basic_dispatch30695:
  if (_trace) printf("basic_dispatch30695:\n");
  /* Here for all other cases */
  arg6 = arg5;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

basic_dispatch30678:
  if (_trace) printf("basic_dispatch30678:\n");
  t6 = (t5 == 128) ? 1 : 0;   

force_alignment30701:
  if (_trace) printf("force_alignment30701:\n");
  if (t6 == 0) 
    goto basic_dispatch30679;
  /* Here if argument 128 */
  arg2 = arg2 + 1;

cdr_internal30663:
  if (_trace) printf("cdr_internal30663:\n");
  /* Memory Read Internal */

vma_memory_read30680:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30682;

vma_memory_read30681:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30684;
  goto cdr_internal30664;   

basic_dispatch30679:
  if (_trace) printf("basic_dispatch30679:\n");
  t6 = (t5 == 64) ? 1 : 0;   

force_alignment30702:
  if (_trace) printf("force_alignment30702:\n");
  if (t6 == 0) 
    goto basic_dispatch30691;
  /* Here if argument 64 */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto cdr_internal30664;   

basic_dispatch30691:
  if (_trace) printf("basic_dispatch30691:\n");
  /* Here for all other cases */
  arg5 = arg2;
  arg2 = 15;
  goto illegaloperand;

vma_memory_read30684:
  if (_trace) printf("vma_memory_read30684:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30683;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30680;   

vma_memory_read30683:
  if (_trace) printf("vma_memory_read30683:\n");

vma_memory_read30682:
  if (_trace) printf("vma_memory_read30682:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0015;
  goto memoryreaddatadecode;
return0015:
  r0 = *(u64 *)sp;   
  goto cdr_internal30664;   

vma_memory_read30671:
  if (_trace) printf("vma_memory_read30671:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30670;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30667;   

vma_memory_read30670:
  if (_trace) printf("vma_memory_read30670:\n");

vma_memory_read30669:
  if (_trace) printf("vma_memory_read30669:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0016;
  goto memoryreadcdrdecode;
return0016:
  r0 = *(u64 *)sp;   
  goto vma_memory_read30676;   

/* end CdrInternal */
/* start DoReadInternalRegister */

  /* Halfword 10 bit immediate instruction - DoReadInternalRegister */

doreadinternalregister:
  if (_trace) printf("doreadinternalregister:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoReadInternalRegisterIM:
  if (_trace) printf("DoReadInternalRegisterIM:\n");

DoReadInternalRegisterSP:
  if (_trace) printf("DoReadInternalRegisterSP:\n");

DoReadInternalRegisterLP:
  if (_trace) printf("DoReadInternalRegisterLP:\n");

DoReadInternalRegisterFP:
  if (_trace) printf("DoReadInternalRegisterFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  t2 = *(u64 *)&(processor->internalregisterread2);   
  t3 = (s32)arg1 - (s32)512;   
  t1 = *(u64 *)&(processor->internalregisterread1);   
  if ((s64)t3 >= 0)   		// We're in the 1000's 
    goto internal_register_dispatch30703;
  t3 = arg1 & 63;		// Keep only six bits 
  t2 = ((s64)t3 <= (s64)42) ? 1 : 0;   		// In range for the low registers? 
  t3 = (t3 * 8) + t1;  
  if (t2 == 0) 
    goto ReadRegisterError;
  t3 = *(u64 *)t3;   
    goto *t3; /* jmp */   		// Jump to the handler 

internal_register_dispatch30703:
  if (_trace) printf("internal_register_dispatch30703:\n");
  t1 = ((s64)t3 <= (s64)33) ? 1 : 0;   		// In range for the high registers? 
  t3 = (t3 * 8) + t2;  
  if (t1 == 0) 
    goto ReadRegisterError;
  t3 = *(u64 *)t3;   
    goto *t3; /* jmp */   		// Jump to the handler 

/* end DoReadInternalRegister */
  /* End of Halfword operand from stack instruction - DoReadInternalRegister */
/* start DoWriteInternalRegister */

  /* Halfword 10 bit immediate instruction - DoWriteInternalRegister */

dowriteinternalregister:
  if (_trace) printf("dowriteinternalregister:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoWriteInternalRegisterIM:
  if (_trace) printf("DoWriteInternalRegisterIM:\n");

DoWriteInternalRegisterSP:
  if (_trace) printf("DoWriteInternalRegisterSP:\n");

DoWriteInternalRegisterLP:
  if (_trace) printf("DoWriteInternalRegisterLP:\n");

DoWriteInternalRegisterFP:
  if (_trace) printf("DoWriteInternalRegisterFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u32)(arg6 >> ((4&7)*8));   
  arg3 = (u32)arg6;   		// Arg2=tag arg3=data 
  iSP = iSP - 8;   		// Pop Stack. 
  t2 = *(u64 *)&(processor->internalregisterwrite2);   
  t3 = (s32)arg1 - (s32)512;   
  t1 = *(u64 *)&(processor->internalregisterwrite1);   
  if ((s64)t3 >= 0)   		// We're in the 1000's 
    goto internal_register_dispatch30704;
  t3 = arg1 & 63;		// Keep only six bits 
  t2 = ((s64)t3 <= (s64)42) ? 1 : 0;   		// In range for the low registers? 
  t3 = (t3 * 8) + t1;  
  if (t2 == 0) 
    goto WriteRegisterError;
  t3 = *(u64 *)t3;   
    goto *t3; /* jmp */   		// Jump to the handler 

internal_register_dispatch30704:
  if (_trace) printf("internal_register_dispatch30704:\n");
  t1 = ((s64)t3 <= (s64)33) ? 1 : 0;   		// In range for the high registers? 
  t3 = (t3 * 8) + t2;  
  if (t1 == 0) 
    goto WriteRegisterError;
  t3 = *(u64 *)t3;   
    goto *t3; /* jmp */   		// Jump to the handler 

/* end DoWriteInternalRegister */
  /* End of Halfword operand from stack instruction - DoWriteInternalRegister */
/* start WriteRegisterBARx */


WriteRegisterBARx:
  if (_trace) printf("WriteRegisterBARx:\n");
  t2 = arg1 >> 7;   		// BAR number into T2 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t3 = arg2 << 32;   		// Make a quadword from tag and data 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t1 = (u64)&processor->bar0;   
  t1 = (t2 * 8) + t1;  		// Now T1 points to the BAR 
  t3 = t3 | arg3;		// Construct the combined word 
  *(u64 *)t1 = t3;   
  goto cachevalid;   

/* end WriteRegisterBARx */
/* start DoBlock3Read */

  /* Halfword 10 bit immediate instruction - DoBlock3Read */

doblock3read:
  if (_trace) printf("doblock3read:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock3ReadIM:
  if (_trace) printf("DoBlock3ReadIM:\n");

DoBlock3ReadSP:
  if (_trace) printf("DoBlock3ReadSP:\n");

DoBlock3ReadLP:
  if (_trace) printf("DoBlock3ReadLP:\n");

DoBlock3ReadFP:
  if (_trace) printf("DoBlock3ReadFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg4 = (u64)&processor->bar3;   
  goto blockread;   

/* end DoBlock3Read */
  /* End of Halfword operand from stack instruction - DoBlock3Read */
/* start DoBlock2Read */

  /* Halfword 10 bit immediate instruction - DoBlock2Read */

doblock2read:
  if (_trace) printf("doblock2read:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock2ReadIM:
  if (_trace) printf("DoBlock2ReadIM:\n");

DoBlock2ReadSP:
  if (_trace) printf("DoBlock2ReadSP:\n");

DoBlock2ReadLP:
  if (_trace) printf("DoBlock2ReadLP:\n");

DoBlock2ReadFP:
  if (_trace) printf("DoBlock2ReadFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg4 = (u64)&processor->bar2;   
  goto blockread;   

/* end DoBlock2Read */
  /* End of Halfword operand from stack instruction - DoBlock2Read */
/* start DoBlock1Read */

  /* Halfword 10 bit immediate instruction - DoBlock1Read */

doblock1read:
  if (_trace) printf("doblock1read:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock1ReadIM:
  if (_trace) printf("DoBlock1ReadIM:\n");

DoBlock1ReadSP:
  if (_trace) printf("DoBlock1ReadSP:\n");

DoBlock1ReadLP:
  if (_trace) printf("DoBlock1ReadLP:\n");

DoBlock1ReadFP:
  if (_trace) printf("DoBlock1ReadFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg4 = (u64)&processor->bar1;   

blockread:
  if (_trace) printf("blockread:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = *(s32 *)arg4;   		// Get the vma 
  arg3 = arg1 >> 6;   		// cycle type 
  t2 = arg1 & 4;		// =no-incrementp 
  t3 = arg1 & 16;		// =cdr-code-nextp 
  t4 = arg1 & 32;		// =fixnum onlyp 
  arg2 = (u32)arg2;   
  /* Do the read cycle */
  /* Memory Read Internal */

vma_memory_read30708:
  t7 = arg2 + ivory;
  t8 = (arg3 * 4);   		// Cycle-number -> table offset 
  arg5 = LDQ_U(t7);   
  t8 = (t8 * 4) + ivory;   
  arg6 = (t7 * 4);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)(t8 + PROCESSORSTATE_DATAREAD_MASK);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30710;

vma_memory_read30709:
  t8 = t8 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30712;

vma_memory_read30719:
  if (t4 != 0)   		// J. if we have to test for fixnump. 
    goto i_block_n_read30705;

i_block_n_read30706:
  t4 = arg2 + 1;		// Compute Incremented address 

force_alignment30720:
  if (_trace) printf("force_alignment30720:\n");
  if (t2 == 0)   		// Conditionally update address 
    arg2 = t4;
  *(u32 *)arg4 = arg2;   		// Store updated vma in BAR 
  t2 = arg5 & 63;		// Compute CDR-NEXT 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (t3)   		// Conditionally Set CDR-NEXT 
    arg5 = t2;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + 8) = arg6;   
  *(u32 *)(iSP + 12) = arg5;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

i_block_n_read30707:
  if (_trace) printf("i_block_n_read30707:\n");
  arg5 = arg2;
  arg2 = 23;
  goto illegaloperand;

vma_memory_read30712:
  if (_trace) printf("vma_memory_read30712:\n");

vma_memory_read30710:
  if (_trace) printf("vma_memory_read30710:\n");
  r0 = (u64)&&return0017;
  goto memoryreadgeneraldecode;
return0017:
  goto vma_memory_read30719;   

i_block_n_read30705:
  if (_trace) printf("i_block_n_read30705:\n");
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto i_block_n_read30707;
  goto i_block_n_read30706;   

/* end DoBlock1Read */
  /* End of Halfword operand from stack instruction - DoBlock1Read */
/* start DoBlock2Write */

  /* Halfword operand from stack instruction - DoBlock2Write */

doblock2write:
  if (_trace) printf("doblock2write:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoBlock2WriteIM:
  if (_trace) printf("DoBlock2WriteIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment30721:
  if (_trace) printf("force_alignment30721:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoblock2write;   

DoBlock2WriteSP:
  if (_trace) printf("DoBlock2WriteSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock2WriteLP:
  if (_trace) printf("DoBlock2WriteLP:\n");

DoBlock2WriteFP:
  if (_trace) printf("DoBlock2WriteFP:\n");

headdoblock2write:
  if (_trace) printf("headdoblock2write:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoblock2write:
  if (_trace) printf("begindoblock2write:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg3 = *(s32 *)&processor->bar2;   
  arg2 = (u64)&processor->bar2;   
  goto blockwrite;   

/* end DoBlock2Write */
  /* End of Halfword operand from stack instruction - DoBlock2Write */
/* start DoBlock1Write */

  /* Halfword operand from stack instruction - DoBlock1Write */

doblock1write:
  if (_trace) printf("doblock1write:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoBlock1WriteIM:
  if (_trace) printf("DoBlock1WriteIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment30725:
  if (_trace) printf("force_alignment30725:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoblock1write;   

DoBlock1WriteSP:
  if (_trace) printf("DoBlock1WriteSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock1WriteLP:
  if (_trace) printf("DoBlock1WriteLP:\n");

DoBlock1WriteFP:
  if (_trace) printf("DoBlock1WriteFP:\n");

headdoblock1write:
  if (_trace) printf("headdoblock1write:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoblock1write:
  if (_trace) printf("begindoblock1write:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg3 = *(s32 *)&processor->bar1;   
  arg2 = (u64)&processor->bar1;   

blockwrite:
  if (_trace) printf("blockwrite:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg3 = (u32)arg3;   		// Unsigned vma 
  t2 = arg1 >> 32;   		// Get tag 
  t3 = (u32)arg1;   		// Get data 
  t8 = arg3 + ivory;
  t6 = (t8 * 4);   
  t5 = LDQ_U(t8);   
  t4 = arg3 - t11;   		// Stack cache offset 
  t7 = ((u64)t4 < (u64)t12) ? 1 : 0;   		// In range? 
  t4 = (t2 & 0xff) << ((t8&7)*8);   
  t5 = t5 & ~(0xffL << (t8&7)*8);   

force_alignment30724:
  if (_trace) printf("force_alignment30724:\n");
  t5 = t5 | t4;
  STQ_U(t8, t5);   
  *(u32 *)t6 = t3;   
  if (t7 != 0)   		// J. if in cache 
    goto vma_memory_write30723;

vma_memory_write30722:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  arg3 = arg3 + 1;		// Increment the address 
  *(u32 *)arg2 = arg3;   		// Store updated vma in BAR 
  goto cachevalid;   

vma_memory_write30723:
  if (_trace) printf("vma_memory_write30723:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t4 = arg3 - t11;   		// Stack cache offset 
  t8 = (t4 * 8) + t8;  		// reconstruct SCA 
  *(u32 *)t8 = t3;   		// Store in stack 
  *(u32 *)(t8 + 4) = t2;   		// write the stack cache 
  goto vma_memory_write30722;   

/* end DoBlock1Write */
  /* End of Halfword operand from stack instruction - DoBlock1Write */
/* start DoBranchTrueNoPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueNoPop */

dobranchtruenopop:
  if (_trace) printf("dobranchtruenopop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueNoPopIM:
  if (_trace) printf("DoBranchTrueNoPopIM:\n");

DoBranchTrueNoPopSP:
  if (_trace) printf("DoBranchTrueNoPopSP:\n");

DoBranchTrueNoPopLP:
  if (_trace) printf("DoBranchTrueNoPopLP:\n");

DoBranchTrueNoPopFP:
  if (_trace) printf("DoBranchTrueNoPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto NEXTINSTRUCTION;
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueNoPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueNoPop */
/* start DoBranchFalseNoPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseNoPop */

dobranchfalsenopop:
  if (_trace) printf("dobranchfalsenopop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseNoPopIM:
  if (_trace) printf("DoBranchFalseNoPopIM:\n");

DoBranchFalseNoPopSP:
  if (_trace) printf("DoBranchFalseNoPopSP:\n");

DoBranchFalseNoPopLP:
  if (_trace) printf("DoBranchFalseNoPopLP:\n");

DoBranchFalseNoPopFP:
  if (_trace) printf("DoBranchFalseNoPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto NEXTINSTRUCTION;
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseNoPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseNoPop */
/* start callgeneric */

  /*  */
  /*  */
  /* Fullword instruction - callgeneric */
  /* ======================= */

callgeneric:
  if (_trace) printf("callgeneric:\n");

callgenericprefetch:
  if (_trace) printf("callgenericprefetch:\n");
  t3 = *(u64 *)&(processor->trapvecbase);   
  arg4 = arg3;		// Get operand 
  arg3 = Type_GenericFunction;
  arg5 = Type_EvenPC;
  arg6 = t3 + 2636;   
  goto startcallcompiledmerge;   

/* end callgeneric */
  /* End of Fullword instruction - callgeneric */
  /* ============================== */
  /*  */
/* start callcompiledeven */

  /*  */
  /*  */
  /* Fullword instruction - callcompiledeven */
  /* ======================= */

callcompiledeven:
  if (_trace) printf("callcompiledeven:\n");

callcompiledevenprefetch:
  if (_trace) printf("callcompiledevenprefetch:\n");
  arg6 = arg3;		// Get operand 
  arg5 = Type_EvenPC;
  arg3 = zero;		// No extra arg 
  goto startcallcompiledmerge;   

/* end callcompiledeven */
  /* End of Fullword instruction - callcompiledeven */
  /* ============================== */
  /*  */
/* start DoStartCall */

  /* Halfword operand from stack instruction - DoStartCall */
  /* arg2 has the preloaded 8 bit operand. */

dostartcall:
  if (_trace) printf("dostartcall:\n");

DoStartCallSP:
  if (_trace) printf("DoStartCallSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoStartCallLP:
  if (_trace) printf("DoStartCallLP:\n");

DoStartCallFP:
  if (_trace) printf("DoStartCallFP:\n");

begindostartcall:
  if (_trace) printf("begindostartcall:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg5 = *(s32 *)(arg1 + 4);   
  arg6 = *(s32 *)arg1;   

startcallagain:
  if (_trace) printf("startcallagain:\n");

start_call_dispatch30728:
  if (_trace) printf("start_call_dispatch30728:\n");
  t1 = *(u64 *)&(processor->trapvecbase);   
  t2 = arg5 & 63;		// Strip off any CDR code bits. 
  t3 = (t2 == Type_CompiledFunction) ? 1 : 0;   

force_alignment30776:
  if (_trace) printf("force_alignment30776:\n");
  if (t3 == 0) 
    goto basic_dispatch30733;
  /* Here if argument TypeCompiledFunction */

start_call_dispatch30729:
  if (_trace) printf("start_call_dispatch30729:\n");
  arg3 = zero;		// No extra argument 

start_call_dispatch30730:
  if (_trace) printf("start_call_dispatch30730:\n");
  arg5 = Type_EvenPC;

startcallcompiledmerge:
  if (_trace) printf("startcallcompiledmerge:\n");
  t7 = *((s32 *)(&processor->continuation)+1);   
  iSP = iSP + 16;		// prepare to push continuation/control register 
  t3 = *(s32 *)&processor->control;   
  t6 = Type_Fixnum+0xC0;
  t8 = *(s32 *)&processor->continuation;   
  t5 = (64) << 16;   
  t7 = t7 | 192;		// Set CDR code 3 
  *(u32 *)(iSP + -8) = t8;   		// push continuation 
  *(u32 *)(iSP + -4) = t7;   		// write the stack cache 
  t8 = t3 | t5;		// Set call started bit in CR 
  t5 = zero + 256;   
  *(u32 *)iSP = t3;   		// Push control register 
  *(u32 *)(iSP + 4) = t6;   		// write the stack cache 
  t8 = t8 & ~t5;		// Clear the extra arg bit 
  *(u32 *)&processor->control = t8;   		// Save control with new state 
  /* End of push-frame */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)&processor->continuation = arg6;   
  *((u32 *)(&processor->continuation)+1) = arg5;   
  *(u64 *)&processor->continuationcp = zero;   
  if (arg3 != 0)   
    goto start_call_dispatch30731;
  goto cachevalid;   

start_call_dispatch30731:
  if (_trace) printf("start_call_dispatch30731:\n");
  t1 = *(s32 *)&processor->control;   
  t2 = zero + 256;   
  t3 = arg3 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg4;   		// Push the extra arg. 
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  iSP = iSP + 8;
  t1 = t1 | t2;		// Set the extra arg bit 
  *(u32 *)&processor->control = t1;   		// Save control with new state 
  goto cachevalid;   

basic_dispatch30733:
  if (_trace) printf("basic_dispatch30733:\n");
  t3 = (t2 == Type_GenericFunction) ? 1 : 0;   

force_alignment30777:
  if (_trace) printf("force_alignment30777:\n");
  if (t3 == 0) 
    goto basic_dispatch30734;
  /* Here if argument TypeGenericFunction */
  arg3 = arg5;
  arg4 = (u32)arg6;   
  arg6 = t1 + 2636;   
  goto start_call_dispatch30730;   

basic_dispatch30734:
  if (_trace) printf("basic_dispatch30734:\n");
  t3 = (t2 == Type_Instance) ? 1 : 0;   

force_alignment30778:
  if (_trace) printf("force_alignment30778:\n");
  if (t3 == 0) 
    goto basic_dispatch30735;
  /* Here if argument TypeInstance */
  arg3 = arg5;
  arg4 = (u32)arg6;   
  arg6 = t1 + 2638;   
  goto start_call_dispatch30730;   

basic_dispatch30735:
  if (_trace) printf("basic_dispatch30735:\n");
  t3 = (t2 == Type_Symbol) ? 1 : 0;   

force_alignment30779:
  if (_trace) printf("force_alignment30779:\n");
  if (t3 == 0) 
    goto basic_dispatch30736;
  /* Here if argument TypeSymbol */
  arg6 = (u32)arg6;   
  arg3 = zero;		// No extra argument 
  arg2 = arg6 + 2;		// Get to the function cell 
  goto startcallindirect;   

basic_dispatch30736:
  if (_trace) printf("basic_dispatch30736:\n");
  t3 = (t2 == Type_LexicalClosure) ? 1 : 0;   

force_alignment30780:
  if (_trace) printf("force_alignment30780:\n");
  if (t3 == 0) 
    goto basic_dispatch30737;
  /* Here if argument TypeLexicalClosure */
  arg2 = (u32)arg6;   
  /* Memory Read Internal */

vma_memory_read30738:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30740;

vma_memory_read30739:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30742;

vma_memory_read30749:
  arg3 = arg5;
  arg4 = arg6;
  arg2 = arg2 + 1;

startcallindirect:
  if (_trace) printf("startcallindirect:\n");
  /* Memory Read Internal */

vma_memory_read30750:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30752;

vma_memory_read30751:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30754;

vma_memory_read30761:
  t5 = arg5 - Type_CompiledFunction;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto start_call_dispatch30728;
  goto start_call_dispatch30730;   

basic_dispatch30737:
  if (_trace) printf("basic_dispatch30737:\n");
  /* Here for all other cases */

start_call_dispatch30726:
  if (_trace) printf("start_call_dispatch30726:\n");
  arg3 = arg5;
  arg4 = arg6;
  t3 = t1 + 2304;   
  /* TagType. */
  arg5 = arg5 & 63;
  arg2 = arg5 + t3;
  /* Memory Read Internal */

vma_memory_read30763:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30765;

vma_memory_read30764:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30767;

vma_memory_read30774:
  t3 = arg5 - Type_EvenPC;   
  t3 = t3 & 63;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto start_call_dispatch30727;
  goto start_call_dispatch30730;   

basic_dispatch30732:
  if (_trace) printf("basic_dispatch30732:\n");

start_call_dispatch30727:
  if (_trace) printf("start_call_dispatch30727:\n");
  arg5 = t1;
  arg2 = 51;
  goto illegaloperand;

vma_memory_read30767:
  if (_trace) printf("vma_memory_read30767:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30766;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30763;   

vma_memory_read30766:
  if (_trace) printf("vma_memory_read30766:\n");

vma_memory_read30765:
  if (_trace) printf("vma_memory_read30765:\n");
  r0 = (u64)&&return0018;
  goto memoryreaddatadecode;
return0018:
  goto vma_memory_read30774;   

vma_memory_read30754:
  if (_trace) printf("vma_memory_read30754:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30753;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30750;   

vma_memory_read30753:
  if (_trace) printf("vma_memory_read30753:\n");

vma_memory_read30752:
  if (_trace) printf("vma_memory_read30752:\n");
  r0 = (u64)&&return0019;
  goto memoryreaddatadecode;
return0019:
  goto vma_memory_read30761;   

vma_memory_read30742:
  if (_trace) printf("vma_memory_read30742:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30741;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30738;   

vma_memory_read30741:
  if (_trace) printf("vma_memory_read30741:\n");

vma_memory_read30740:
  if (_trace) printf("vma_memory_read30740:\n");
  r0 = (u64)&&return0020;
  goto memoryreaddatadecode;
return0020:
  goto vma_memory_read30749;   

DoStartCallIM:
  goto doistageerror;

/* end DoStartCall */
  /* End of Halfword operand from stack instruction - DoStartCall */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifuncom1.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifuncom2.as
 ************************************************************************/

  /* The most commonly used instructions, part 2. */
/* start DoPushInstanceVariable */

  /* Halfword 10 bit immediate instruction - DoPushInstanceVariable */

dopushinstancevariable:
  if (_trace) printf("dopushinstancevariable:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPushInstanceVariableIM:
  if (_trace) printf("DoPushInstanceVariableIM:\n");

DoPushInstanceVariableSP:
  if (_trace) printf("DoPushInstanceVariableSP:\n");

DoPushInstanceVariableLP:
  if (_trace) printf("DoPushInstanceVariableLP:\n");

DoPushInstanceVariableFP:
  if (_trace) printf("DoPushInstanceVariableFP:\n");
  /* arg1 has operand preloaded. */
  arg1 = arg2;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Mapped */
  arg2 = *(s32 *)(iFP + 16);   		// Map 
  arg5 = *(s32 *)(iFP + 20);   
  arg2 = (u32)arg2;   
  t2 = arg5 - Type_Array;   
  t2 = t2 & 63;		// Strip CDR code 
  if (t2 != 0)   
    goto ivbadmap;
  /* Memory Read Internal */

vma_memory_read30784:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30786;

vma_memory_read30785:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30788;

vma_memory_read30793:
  arg6 = arg6 & Array_LengthMask;
  t3 = arg6 - arg1;   
  if ((s64)t3 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg2 = arg2 + arg1;
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read30794:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30796;

vma_memory_read30795:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30798;

vma_memory_read30805:
  t1 = arg6;
  t4 = arg5 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto pushivexception;
  arg2 = *(s32 *)(iFP + 24);   		// Self 
  t4 = *(s32 *)(iFP + 28);   
  arg2 = (u32)arg2;   
  t3 = t4 - Type_Instance;   
  t3 = t3 & 60;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto ivbadinst;
  t3 = t4 & 192;		// Unshifted cdr code 
  t3 = t3 - 64;   		// Check for CDR code 1 
  if (t3 != 0)   		// J. if CDR code is not 1 
    goto locate_instance0variable_mapped30783;

locate_instance0variable_mapped30782:
  if (_trace) printf("locate_instance0variable_mapped30782:\n");
  arg2 = arg2 + t1;

locate_instance0variable_mapped30781:
  if (_trace) printf("locate_instance0variable_mapped30781:\n");
  /* Memory Read Internal */

vma_memory_read30806:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30808;

vma_memory_read30807:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30810;

vma_memory_read30817:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;   
  *(u32 *)(iSP + 12) = t7;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

vma_memory_read30810:
  if (_trace) printf("vma_memory_read30810:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30809;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30806;   

vma_memory_read30809:
  if (_trace) printf("vma_memory_read30809:\n");

vma_memory_read30808:
  if (_trace) printf("vma_memory_read30808:\n");
  r0 = (u64)&&return0021;
  goto memoryreaddatadecode;
return0021:
  goto vma_memory_read30817;   

vma_memory_read30798:
  if (_trace) printf("vma_memory_read30798:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30797;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30794;   

vma_memory_read30797:
  if (_trace) printf("vma_memory_read30797:\n");

vma_memory_read30796:
  if (_trace) printf("vma_memory_read30796:\n");
  r0 = (u64)&&return0022;
  goto memoryreaddatadecode;
return0022:
  goto vma_memory_read30805;   

vma_memory_read30788:
  if (_trace) printf("vma_memory_read30788:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30787;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30784;   

vma_memory_read30787:
  if (_trace) printf("vma_memory_read30787:\n");

vma_memory_read30786:
  if (_trace) printf("vma_memory_read30786:\n");
  r0 = (u64)&&return0023;
  goto memoryreadheaderdecode;
return0023:
  goto vma_memory_read30793;   

locate_instance0variable_mapped30783:
  if (_trace) printf("locate_instance0variable_mapped30783:\n");
  t3 = arg2;
  /* Memory Read Internal */

vma_memory_read30818:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30820;

vma_memory_read30819:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30822;

vma_memory_read30827:
  t3 = t3 - arg2;   
  if (t3 != 0)   
    goto locate_instance0variable_mapped30782;
  /* TagType. */
  t4 = t4 & 63;
  t4 = t4 | 64;		// Set CDR code to 1 
  *(u32 *)(iFP + 24) = arg2;   		// Update self 
  *(u32 *)(iFP + 28) = t4;   		// write the stack cache 
  goto locate_instance0variable_mapped30782;   

vma_memory_read30822:
  if (_trace) printf("vma_memory_read30822:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30821;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30818;   

vma_memory_read30821:
  if (_trace) printf("vma_memory_read30821:\n");

vma_memory_read30820:
  if (_trace) printf("vma_memory_read30820:\n");
  r0 = (u64)&&return0024;
  goto memoryreadheaderdecode;
return0024:
  goto vma_memory_read30827;   

/* end DoPushInstanceVariable */
  /* End of Halfword operand from stack instruction - DoPushInstanceVariable */
/* start DoAdd */

  /* Halfword operand from stack instruction - DoAdd */
  /* arg2 has the preloaded 8 bit operand. */

doadd:
  if (_trace) printf("doadd:\n");

DoAddSP:
  if (_trace) printf("DoAddSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoadd;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoAddLP:
  if (_trace) printf("DoAddLP:\n");

DoAddFP:
  if (_trace) printf("DoAddFP:\n");

begindoadd:
  if (_trace) printf("begindoadd:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  LDS(1, f1, *(u32 *)iSP );   
  t1 = (u32)(arg6 >> ((4&7)*8));   		// ARG1 tag 
  t3 = *(s32 *)(arg1 + 4);   		// ARG2 tag 
  t2 = (s32)arg6;		// ARG1 data 
  t4 = *(s32 *)arg1;   		// ARG2 data 
  LDS(2, f2, *(u32 *)arg1 );   
  /* NIL */
  t9 = t1 & 63;		// Strip off any CDR code bits. 
  t11 = t3 & 63;		// Strip off any CDR code bits. 
  t10 = (t9 == Type_Fixnum) ? 1 : 0;   

force_alignment30867:
  if (_trace) printf("force_alignment30867:\n");
  if (t10 == 0) 
    goto basic_dispatch30838;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment30844:
  if (_trace) printf("force_alignment30844:\n");
  if (t12 == 0) 
    goto basic_dispatch30840;
  /* Here if argument TypeFixnum */
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = (u64)((s32)t2 + (s64)(s32)t4); 		// compute 64-bit result 
  if (t5 >> 31) exception(1, t5);  /* addl/v */     // WARNING !!! THIS SHOULD REFLECT THE DIFF FILE
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  *(u32 *)(iSP + 4) = t9;   		// Semi-cheat, we know temp2 has CDRNext/TypeFixnum 
  iPC = t6;
  *(u32 *)iSP = t5;   
  iCP = t7;
  goto cachevalid;   

basic_dispatch30840:
  if (_trace) printf("basic_dispatch30840:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment30845:
  if (_trace) printf("force_alignment30845:\n");
  if (t12 == 0) 
    goto basic_dispatch30841;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation30828;   

basic_dispatch30841:
  if (_trace) printf("basic_dispatch30841:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment30846:
  if (_trace) printf("force_alignment30846:\n");
  if (t12 == 0) 
    goto binary_type_dispatch30835;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation30831;   

basic_dispatch30839:
  if (_trace) printf("basic_dispatch30839:\n");

basic_dispatch30838:
  if (_trace) printf("basic_dispatch30838:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment30868:
  if (_trace) printf("force_alignment30868:\n");
  if (t10 == 0) 
    goto basic_dispatch30847;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment30853:
  if (_trace) printf("force_alignment30853:\n");
  if (t12 == 0) 
    goto basic_dispatch30849;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation30828:
  if (_trace) printf("simple_binary_arithmetic_operation30828:\n");
  ADDS(0, f0, 1, f1, 2, f2); /* adds */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  goto cachevalid;   

basic_dispatch30849:
  if (_trace) printf("basic_dispatch30849:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment30854:
  if (_trace) printf("force_alignment30854:\n");
  if (t12 == 0) 
    goto basic_dispatch30850;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation30828;   

basic_dispatch30850:
  if (_trace) printf("basic_dispatch30850:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment30855:
  if (_trace) printf("force_alignment30855:\n");
  if (t12 == 0) 
    goto binary_type_dispatch30835;
  /* Here if argument TypeDoubleFloat */

simple_binary_arithmetic_operation30831:
  if (_trace) printf("simple_binary_arithmetic_operation30831:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto simple_binary_arithmetic_operation30832;   

basic_dispatch30848:
  if (_trace) printf("basic_dispatch30848:\n");

basic_dispatch30847:
  if (_trace) printf("basic_dispatch30847:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment30869:
  if (_trace) printf("force_alignment30869:\n");
  if (t10 == 0) 
    goto basic_dispatch30856;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment30862:
  if (_trace) printf("force_alignment30862:\n");
  if (t12 == 0) 
    goto basic_dispatch30858;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0025;
  goto fetchdoublefloat;
return0025:
  LDT(1, f1, processor->fp0);   

simple_binary_arithmetic_operation30832:
  if (_trace) printf("simple_binary_arithmetic_operation30832:\n");
  arg2 = (u32)t4;   
  r0 = (u64)&&return0026;
  goto fetchdoublefloat;
return0026:
  LDT(2, f2, processor->fp0);   

simple_binary_arithmetic_operation30829:
  if (_trace) printf("simple_binary_arithmetic_operation30829:\n");
  ADDT(0, f0, 1, f1, 2, f2); /* addt */   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0027;
  goto consdoublefloat;
return0027:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;   
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch30858:
  if (_trace) printf("basic_dispatch30858:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment30863:
  if (_trace) printf("force_alignment30863:\n");
  if (t12 == 0) 
    goto basic_dispatch30859;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation30830:
  if (_trace) printf("simple_binary_arithmetic_operation30830:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0028;
  goto fetchdoublefloat;
return0028:
  LDT(1, f1, processor->fp0);   
  goto simple_binary_arithmetic_operation30829;   

basic_dispatch30859:
  if (_trace) printf("basic_dispatch30859:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment30864:
  if (_trace) printf("force_alignment30864:\n");
  if (t12 == 0) 
    goto binary_type_dispatch30835;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation30830;   

basic_dispatch30857:
  if (_trace) printf("basic_dispatch30857:\n");

basic_dispatch30856:
  if (_trace) printf("basic_dispatch30856:\n");
  /* Here for all other cases */

binary_type_dispatch30834:
  if (_trace) printf("binary_type_dispatch30834:\n");

doaddovfl:
  if (_trace) printf("doaddovfl:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch30836;   

binary_type_dispatch30835:
  if (_trace) printf("binary_type_dispatch30835:\n");
  t1 = t3;
  goto doaddovfl;   

binary_type_dispatch30836:
  if (_trace) printf("binary_type_dispatch30836:\n");

basic_dispatch30837:
  if (_trace) printf("basic_dispatch30837:\n");

DoAddIM:
  if (_trace) printf("DoAddIM:\n");
  t1 = (u32)(arg6 >> ((4&7)*8));   
  t2 = (s32)arg6;		// get ARG1 tag/data 
  t11 = t1 & 63;		// Strip off any CDR code bits. 
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment30874:
  if (_trace) printf("force_alignment30874:\n");
  if (t12 == 0) 
    goto basic_dispatch30871;
  /* Here if argument TypeFixnum */
  t3 = t2 + arg2;		// compute 64-bit result 
  t4 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t10 = (s32)t3;		// compute 32-bit sign-extended result 
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t10 = (t3 == t10) ? 1 : 0;   		// is it the same as the 64-bit result? 
  if (t10 == 0) 		// if not, we overflowed 
    goto doaddovfl;
  *(u32 *)(iSP + 4) = t11;   		// Semi-cheat, we know temp2 has CDRNext/TypeFixnum 
  iPC = t4;
  *(u32 *)iSP = t3;   
  iCP = t5;
  goto cachevalid;   

basic_dispatch30871:
  if (_trace) printf("basic_dispatch30871:\n");
  /* Here for all other cases */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindoadd;   

basic_dispatch30870:
  if (_trace) printf("basic_dispatch30870:\n");

/* end DoAdd */
  /* End of Halfword operand from stack instruction - DoAdd */
/* start DoBlock3Write */

  /* Halfword operand from stack instruction - DoBlock3Write */

doblock3write:
  if (_trace) printf("doblock3write:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoBlock3WriteIM:
  if (_trace) printf("DoBlock3WriteIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment30875:
  if (_trace) printf("force_alignment30875:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoblock3write;   

DoBlock3WriteSP:
  if (_trace) printf("DoBlock3WriteSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock3WriteLP:
  if (_trace) printf("DoBlock3WriteLP:\n");

DoBlock3WriteFP:
  if (_trace) printf("DoBlock3WriteFP:\n");

headdoblock3write:
  if (_trace) printf("headdoblock3write:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoblock3write:
  if (_trace) printf("begindoblock3write:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg3 = *(s32 *)&processor->bar3;   
  arg2 = (u64)&processor->bar3;   
  goto blockwrite;   

/* end DoBlock3Write */
  /* End of Halfword operand from stack instruction - DoBlock3Write */
/* start DoAset1 */

  /* Halfword operand from stack instruction - DoAset1 */
  /* arg2 has the preloaded 8 bit operand. */

doaset1:
  if (_trace) printf("doaset1:\n");

DoAset1SP:
  if (_trace) printf("DoAset1SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAset1LP:
  if (_trace) printf("DoAset1LP:\n");

DoAset1FP:
  if (_trace) printf("DoAset1FP:\n");

headdoaset1:
  if (_trace) printf("headdoaset1:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoaset1:
  if (_trace) printf("begindoaset1:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// Get the array tag/data 
  arg3 = *(s32 *)(iSP + 4);   		// Get the array tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  t6 = *(s32 *)iSP;   		// Get the new value tag/data 
  t5 = *(s32 *)(iSP + 4);   		// Get the new value tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  t6 = (u32)t6;   
  arg2 = (s32)arg1 + (s32)0;		// (sign-extended, for fast bounds check) Index Data 
  t8 = zero + AutoArrayRegMask;   
  t8 = arg4 & t8;
  arg1 = arg1 >> 32;   		// Index Tag 
  t7 = (u64)&processor->ac0array;   
  t7 = t7 + t8;		// This is the address if the array register block. 
  t1 = arg1 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto aset1illegal;

aset1merge:
  if (_trace) printf("aset1merge:\n");
  if (arg4 == 0) 
    goto aset1regset;
  t8 = *(u64 *)&(((ARRAYCACHEP)t7)->array);   		// Cached array object. 
  t1 = arg3 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto reallyaset1exc;
  t8 = (arg4 == t8) ? 1 : 0;   		// t8==1 iff cached array is ours. 
  if (t8 == 0) 		// Go and setup the array register. 
    goto aset1regset;
#ifdef SLOWARRAYS
  goto aset1regset;   
#endif
  arg6 = *(u64 *)&(((ARRAYCACHEP)t7)->arword);   
  t9 = *(u64 *)&(((ARRAYCACHEP)t7)->locat);   		// high order bits all zero 
  t3 = *(u64 *)&(((ARRAYCACHEP)t7)->length);   		// high order bits all zero 
  t11 = arg6 << 42;   
  t4 = *(u64 *)&(processor->areventcount);   
  t11 = t11 >> 42;   
  t2 = ((u64)arg2 < (u64)t3) ? 1 : 0;   
  t12 = t4 - t11;   
  if (t12 != 0)   		// J. if event count ticked. 
    goto aset1regset;
  if (t2 == 0) 
    goto aset1bounds;
  arg5 = arg6 >> (Array_RegisterBytePackingPos & 63);   
  t8 = arg6 >> (Array_RegisterElementTypePos & 63);   
  arg4 = arg6 >> (Array_RegisterByteOffsetPos & 63);   
  arg5 = arg5 & Array_RegisterBytePackingMask;
  arg4 = arg4 & Array_RegisterByteOffsetMask;
  arg6 = t8 & Array_RegisterElementTypeMask;

aset1restart:
  if (_trace) printf("aset1restart:\n");
  /* Element checking and foreplay. */
  /* TagType. */
  t1 = t5 & 63;
  t8 = (arg6 == Array_ElementTypeCharacter) ? 1 : 0;   

force_alignment30886:
  if (_trace) printf("force_alignment30886:\n");
  if (t8 == 0) 
    goto basic_dispatch30882;
  /* Here if argument ArrayElementTypeCharacter */
  t2 = t1 - Type_Character;   
  if (t2 == 0) 
    goto aset_1_internal30877;
  arg5 = 0;
  arg2 = 29;
  goto illegaloperand;

aset_1_internal30877:
  if (_trace) printf("aset_1_internal30877:\n");
  if (arg5 == 0) 		// Certainly will fit if not packed! 
    goto aset_1_internal30876;
  t2 = 32;
  t2 = t2 >> (arg5 & 63);   		// Compute size of byte 
  t1 = ~zero;   
  t1 = t1 << (t2 & 63);   
  t1 = ~t1;   		// Compute mask for byte 
  t1 = t6 & t1;
  t1 = t6 - t1;   
  if (t1 == 0) 		// J. if character fits. 
    goto aset_1_internal30876;
  arg5 = 0;
  arg2 = 62;
  goto illegaloperand;

basic_dispatch30882:
  if (_trace) printf("basic_dispatch30882:\n");
  t8 = (arg6 == Array_ElementTypeFixnum) ? 1 : 0;   

force_alignment30887:
  if (_trace) printf("force_alignment30887:\n");
  if (t8 == 0) 
    goto basic_dispatch30883;
  /* Here if argument ArrayElementTypeFixnum */
  t2 = t1 - Type_Fixnum;   
  if (t2 == 0) 
    goto aset_1_internal30876;
  arg5 = 0;
  arg2 = 33;
  goto illegaloperand;

basic_dispatch30883:
  if (_trace) printf("basic_dispatch30883:\n");
  t8 = (arg6 == Array_ElementTypeBoolean) ? 1 : 0;   

force_alignment30888:
  if (_trace) printf("force_alignment30888:\n");
  if (t8 == 0) 
    goto basic_dispatch30881;
  /* Here if argument ArrayElementTypeBoolean */
  t6 = 1;
  t1 = t1 - Type_NIL;   
  if (t1 != 0)   		// J. if True 
    goto aset_1_internal30876;
  t6 = zero;
  goto aset_1_internal30876;   		// J. if False 

basic_dispatch30881:
  if (_trace) printf("basic_dispatch30881:\n");
  /* Shove it in. */

aset_1_internal30876:
  if (_trace) printf("aset_1_internal30876:\n");
  if (arg5 != 0)   		// J. if packed 
    goto aset_1_internal30878;
  t1 = arg6 - Array_ElementTypeObject;   
  if (t1 != 0)   
    goto aset_1_internal30878;
  /* Here for the simple non packed case */
  t1 = t9 + arg2;
  /* Memory Read Internal */

vma_memory_read30889:
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t8 = t1 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t3 = (t8 * 4);   
  t2 = LDQ_U(t8);   
  t4 = t1 - t4;   		// Stack cache offset 
  arg1 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t4 < (u64)t7) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read30891;

vma_memory_read30890:
  t8 = zero + 240;   
  arg1 = arg1 >> (t2 & 63);   
  t8 = t8 >> (t2 & 63);   
  if (arg1 & 1)   
    goto vma_memory_read30893;

vma_memory_read30899:
  /* Merge cdr-code */
  t3 = t5 & 63;
  t2 = t2 & 192;
  t2 = t2 | t3;
  t7 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t1 + ivory;
  arg1 = *(s32 *)&processor->scovlimit;   
  t3 = (t4 * 4);   
  t8 = LDQ_U(t4);   
  t7 = t1 - t7;   		// Stack cache offset 
  arg1 = ((u64)t7 < (u64)arg1) ? 1 : 0;   		// In range? 
  t7 = (t2 & 0xff) << ((t4&7)*8);   
  t8 = t8 & ~(0xffL << (t4&7)*8);   

force_alignment30901:
  if (_trace) printf("force_alignment30901:\n");
  t8 = t8 | t7;
  STQ_U(t4, t8);   
  *(u32 *)t3 = t6;   
  if (arg1 != 0)   		// J. if in cache 
    goto vma_memory_write30900;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   
  /* Here for the slow packed version */

aset_1_internal30878:
  if (_trace) printf("aset_1_internal30878:\n");
  arg2 = arg4 + arg2;
  t1 = arg2 >> (arg5 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  /* Memory Read Internal */

vma_memory_read30902:
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t4 = t1 + ivory;
  t3 = *(s32 *)&processor->scovlimit;   
  t9 = (t4 * 4);   
  arg3 = LDQ_U(t4);   
  t2 = t1 - t2;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->dataread_mask);   
  t3 = ((u64)t2 < (u64)t3) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  arg3 = (u8)(arg3 >> ((t4&7)*8));   
  if (t3 != 0)   
    goto vma_memory_read30904;

vma_memory_read30903:
  t4 = zero + 240;   
  t7 = t7 >> (arg3 & 63);   
  t4 = t4 >> (arg3 & 63);   
  t9 = (u32)t9;   
  if (t7 & 1)   
    goto vma_memory_read30906;

vma_memory_read30913:
  /* Check fixnum element type */
  /* TagType. */
  t2 = arg3 & 63;
  t2 = t2 - Type_Fixnum;   
  if (t2 != 0)   		// J. if element type not fixnum. 
    goto aset_1_internal30879;
  if (arg5 == 0) 		// J. if unpacked fixnum element type. 
    goto aset_1_internal30880;
  t8 = ~zero;   
  t8 = t8 << (arg5 & 63);   
  t2 = zero - arg5;   
  t8 = arg2 & ~t8;		// Compute subword index 
  t2 = t2 + 5;
  t2 = t8 << (t2 & 63);   		// Compute shift to get byte 
  t8 = 32;
  t8 = t8 >> (arg5 & 63);   		// Compute size of byte 
  t3 = ~zero;   
  t3 = t3 << (t8 & 63);   
  t4 = ~t3;   		// Compute mask for byte 
  if (t2 == 0) 		// inserting into the low byte is easy 
    goto array_element_dpb30914;
  /* Inserting the byte into any byte other than the low byte */
  t7 = 64;
  t8 = t7 - t2;   		// = the left shift rotate amount 
  t7 = t9 >> (t2 & 63);   		// shift selected byte into low end of word. 
  t9 = t9 << (t8 & 63);   		// rotate low bits into high end of word. 
  t7 = t3 & t7;		// Remove unwanted bits 
  t9 = t9 >> (t8 & 63);   		// rotate low bits back into place. 
  t8 = t6 & t4;		// Strip any extra bits from element 
  t7 = t8 | t7;		// Insert new bits. 
  t7 = t7 << (t2 & 63);   		// reposition bits 
  t9 = t9 | t7;		// Replace low order bits 
  goto array_element_dpb30915;   

array_element_dpb30914:
  if (_trace) printf("array_element_dpb30914:\n");
  /* Inserting the byte into the low byte */
  t9 = t9 & t3;		// Remove the old low byte 
  t8 = t6 & t4;		// Remove unwanted bits from the new byte 
  t9 = t9 | t8;		// Insert the new byte in place of the old byte 

array_element_dpb30915:
  if (_trace) printf("array_element_dpb30915:\n");
  t6 = t9;

aset_1_internal30880:
  if (_trace) printf("aset_1_internal30880:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t7 = (t2 * 4);   
  t4 = LDQ_U(t2);   
  t3 = t1 - t3;   		// Stack cache offset 
  t8 = ((u64)t3 < (u64)t8) ? 1 : 0;   		// In range? 
  t3 = (arg3 & 0xff) << ((t2&7)*8);   
  t4 = t4 & ~(0xffL << (t2&7)*8);   

force_alignment30917:
  if (_trace) printf("force_alignment30917:\n");
  t4 = t4 | t3;
  STQ_U(t2, t4);   
  *(u32 *)t7 = t6;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write30916;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

aset_1_internal30879:
  if (_trace) printf("aset_1_internal30879:\n");
  arg5 = t1;
  arg2 = 25;
  goto illegaloperand;

DoAset1IM:
  if (_trace) printf("DoAset1IM:\n");
  t8 = zero + AutoArrayRegMask;   
  arg4 = *(s32 *)iSP;   		// Get the array tag/data 
  arg3 = *(s32 *)(iSP + 4);   		// Get the array tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  t7 = (u64)&processor->ac0array;   
  t8 = arg4 & t8;
  t7 = t7 + t8;		// This is the address of the array register block. 
  t6 = *(s32 *)iSP;   		// Get the new value tag/data 
  t5 = *(s32 *)(iSP + 4);   		// Get the new value tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  t6 = (u32)t6;   
  goto aset1merge;   

vma_memory_write30916:
  if (_trace) printf("vma_memory_write30916:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment30918:
  if (_trace) printf("force_alignment30918:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t3 = t1 - t3;   		// Stack cache offset 
  t2 = (t3 * 8) + t2;  		// reconstruct SCA 
  *(u32 *)t2 = t6;   		// Store in stack 
  *(u32 *)(t2 + 4) = arg3;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read30904:
  if (_trace) printf("vma_memory_read30904:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg3 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read30903;   

vma_memory_read30906:
  if (_trace) printf("vma_memory_read30906:\n");
  if ((t4 & 1) == 0)   
    goto vma_memory_read30905;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read30902;   

vma_memory_read30905:
  if (_trace) printf("vma_memory_read30905:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t4 = arg3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t4 = (t4 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t4;   		// Get the memory action 

vma_memory_read30910:
  if (_trace) printf("vma_memory_read30910:\n");
  t4 = t7 & MemoryActionTransform;
  if (t4 == 0) 
    goto vma_memory_read30909;
  arg3 = arg3 & ~63L;
  arg3 = arg3 | Type_ExternalValueCellPointer;
  goto vma_memory_read30913;   

vma_memory_read30909:

vma_memory_read30908:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_write30900:
  if (_trace) printf("vma_memory_write30900:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment30919:
  if (_trace) printf("force_alignment30919:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - t7;   		// Stack cache offset 
  t4 = (t7 * 8) + t4;  		// reconstruct SCA 
  *(u32 *)t4 = t6;   		// Store in stack 
  *(u32 *)(t4 + 4) = t2;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read30891:
  if (_trace) printf("vma_memory_read30891:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read30890;   

vma_memory_read30893:
  if (_trace) printf("vma_memory_read30893:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read30892;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read30889;   

vma_memory_read30892:
  if (_trace) printf("vma_memory_read30892:\n");
  arg1 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + arg1;   		// Adjust for a longword load 
  arg1 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read30896:

vma_memory_read30895:
  /* Perform memory action */
  arg1 = arg1;
  arg2 = 1;
  goto performmemoryaction;

/* end DoAset1 */
  /* End of Halfword operand from stack instruction - DoAset1 */
/* start DoFastAref1 */

  /* Halfword operand from stack instruction - DoFastAref1 */
  /* arg2 has the preloaded 8 bit operand. */

dofastaref1:
  if (_trace) printf("dofastaref1:\n");

DoFastAref1SP:
  if (_trace) printf("DoFastAref1SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindofastaref1;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoFastAref1LP:
  if (_trace) printf("DoFastAref1LP:\n");

DoFastAref1FP:
  if (_trace) printf("DoFastAref1FP:\n");

begindofastaref1:
  if (_trace) printf("begindofastaref1:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (s32)arg6;
  t1 = arg3 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto fastaref1iop;

fastaref1retry:
  if (_trace) printf("fastaref1retry:\n");
  arg6 = *(s32 *)arg1;   
  t9 = *(s32 *)(arg1 + 8);   
  t3 = *(s32 *)(arg1 + 16);   
  arg6 = (u32)arg6;   
  t9 = (u32)t9;   
  t5 = arg6 << 42;   
  t3 = (u32)t3;   
  t4 = *(u64 *)&(processor->areventcount);   
  t5 = t5 >> 42;   
  t2 = ((u64)arg4 < (u64)t3) ? 1 : 0;   
  if (t2 == 0) 
    goto fastaref1bounds;
  t6 = t4 - t5;   
  if (t6 != 0)   
    goto aref1recomputearrayregister;
  t6 = arg6 >> (Array_RegisterBytePackingPos & 63);   
  t7 = arg6 >> (Array_RegisterByteOffsetPos & 63);   
  t8 = arg6 >> (Array_RegisterElementTypePos & 63);   
  t6 = t6 & Array_RegisterBytePackingMask;
  t7 = t7 & Array_RegisterByteOffsetMask;
  t8 = t8 & Array_RegisterElementTypeMask;
  if (t6 != 0)   
    goto new_aref_1_internal30920;
  t1 = t9 + arg4;

new_aref_1_internal30921:
  if (_trace) printf("new_aref_1_internal30921:\n");
  /* Memory Read Internal */

vma_memory_read30928:
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t4 = t1 + ivory;
  t3 = *(s32 *)&processor->scovlimit;   
  t9 = (t4 * 4);   
  arg5 = LDQ_U(t4);   
  t2 = t1 - t2;   		// Stack cache offset 
  t5 = *(u64 *)&(processor->dataread_mask);   
  t3 = ((u64)t2 < (u64)t3) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  arg5 = (u8)(arg5 >> ((t4&7)*8));   
  if (t3 != 0)   
    goto vma_memory_read30930;

vma_memory_read30929:
  t4 = zero + 240;   
  t5 = t5 >> (arg5 & 63);   
  t4 = t4 >> (arg5 & 63);   
  t9 = (u32)t9;   
  if (t5 & 1)   
    goto vma_memory_read30932;

vma_memory_read30939:
  if (t6 != 0)   
    goto new_aref_1_internal30922;

new_aref_1_internal30923:
  if (_trace) printf("new_aref_1_internal30923:\n");
  r31 = r31 | r31;
  t1 = t8 - 2;   
  if ((s64)t1 <= 0)  
    goto new_aref_1_internal30924;
  /* TagType. */
  arg5 = arg5 & 63;

new_aref_1_internal30925:
  if (_trace) printf("new_aref_1_internal30925:\n");
  *(u32 *)(iSP + 4) = arg5;   
  t4 = (t6 == 0) ? 1 : 0;   
  if (t4 == 0) 
    goto case_others_189;

case_0_183:
  if (_trace) printf("case_0_183:\n");
  r31 = r31 | r31;
  if (t1 == 0) 
    goto new_aref_1_internal30926;
  *(u32 *)iSP = t9;   
  goto NEXTINSTRUCTION;   

case_2_184:
  if (_trace) printf("case_2_184:\n");
  /* AREF1-8B */
  r31 = r31 | r31;
  t4 = arg4 & 3;
  t5 = (u8)(t9 >> ((t4&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal30926;
  *(u32 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

case_3_185:
  if (_trace) printf("case_3_185:\n");
  /* AREF1-4B */
  r31 = r31 | r31;
  t4 = arg4 & 7;		// byte-index 
  t4 = t4 << 2;   		// byte-position 
  t5 = t9 >> (t4 & 63);   		// byte in position 
  t5 = t5 & 15;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal30926;
  *(u32 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

case_5_186:
  if (_trace) printf("case_5_186:\n");
  /* AREF1-1B */
  r31 = r31 | r31;
  t4 = arg4 & 31;		// byte-index 
  r31 = r31 | r31;
  t5 = t9 >> (t4 & 63);   		// byte in position 
  t5 = t5 & 1;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal30926;
  *(u32 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

case_1_187:
  if (_trace) printf("case_1_187:\n");
  /* AREF1-16B */
  t4 = arg4 & 1;
  t4 = t4 + t4;		// Bletch, it's a byte ref 
  t5 = (u16)(t9 >> ((t4&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal30926;
  *(u32 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

case_others_189:
  if (_trace) printf("case_others_189:\n");
  r31 = r31 | r31;
  t4 = (t6 == 2) ? 1 : 0;   
  t5 = (t6 == 3) ? 1 : 0;   
  if (t4 != 0)   
    goto case_2_184;
  t4 = (t6 == 5) ? 1 : 0;   
  if (t5 != 0)   
    goto case_3_185;
  t5 = (t6 == 1) ? 1 : 0;   
  if (t4 != 0)   
    goto case_5_186;
  if (t5 != 0)   
    goto case_1_187;

case_4_188:
  if (_trace) printf("case_4_188:\n");
  /* AREF1-2B */
  r31 = r31 | r31;
  t4 = arg4 & 15;		// byte-index 
  t4 = t4 << 1;   		// byte-position 
  t5 = t9 >> (t4 & 63);   		// byte in position 
  t5 = t5 & 3;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal30926;
  *(u32 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

new_aref_1_internal30920:
  if (_trace) printf("new_aref_1_internal30920:\n");
  arg4 = t7 + arg4;
  t1 = arg4 >> (t6 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  goto new_aref_1_internal30921;   

new_aref_1_internal30922:
  if (_trace) printf("new_aref_1_internal30922:\n");
  t1 = arg5 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto new_aref_1_internal30927;
  goto new_aref_1_internal30923;   

new_aref_1_internal30924:
  if (_trace) printf("new_aref_1_internal30924:\n");
  arg5 = Type_Character;
  if (t8 & 1)   
    goto new_aref_1_internal30925;
  arg5 = Type_Fixnum;
  if (t8 == 0) 
    goto new_aref_1_internal30925;
  t2 = *(u64 *)&(processor->niladdress);   
  t3 = *(u64 *)&(processor->taddress);   
  goto new_aref_1_internal30925;   

new_aref_1_internal30926:
  if (_trace) printf("new_aref_1_internal30926:\n");
  if (t5)   
    t2 = t3;
  *(u64 *)iSP = t2;   
  goto NEXTINSTRUCTION;   

new_aref_1_internal30927:
  if (_trace) printf("new_aref_1_internal30927:\n");
  arg5 = t1;
  arg2 = 25;
  goto illegaloperand;

fastaref1iop:
  if (_trace) printf("fastaref1iop:\n");
  arg5 = 0;
  arg2 = 32;
  goto illegaloperand;

fastaref1bounds:
  if (_trace) printf("fastaref1bounds:\n");
  arg5 = 0;
  arg2 = 13;
  goto illegaloperand;

vma_memory_read30930:
  if (_trace) printf("vma_memory_read30930:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg5 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read30929;   

vma_memory_read30932:
  if (_trace) printf("vma_memory_read30932:\n");
  if ((t4 & 1) == 0)   
    goto vma_memory_read30931;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read30928;   

vma_memory_read30931:
  if (_trace) printf("vma_memory_read30931:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t4 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t4 = (t4 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t4;   		// Get the memory action 

vma_memory_read30936:
  if (_trace) printf("vma_memory_read30936:\n");
  t4 = t5 & MemoryActionTransform;
  if (t4 == 0) 
    goto vma_memory_read30935;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read30939;   

vma_memory_read30935:

vma_memory_read30934:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

DoFastAref1IM:
  goto doistageerror;

/* end DoFastAref1 */
  /* End of Halfword operand from stack instruction - DoFastAref1 */
/* start DoRplaca */

  /* Halfword operand from stack instruction - DoRplaca */

dorplaca:
  if (_trace) printf("dorplaca:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoRplacaIM:
  if (_trace) printf("DoRplacaIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment30953:
  if (_trace) printf("force_alignment30953:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindorplaca;   

DoRplacaSP:
  if (_trace) printf("DoRplacaSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto headdorplaca;
  arg1 = arg6;		// SP-pop mode, TOS->arg1 
  arg6 = *(u64 *)arg4;   		// Reload TOS 
  iSP = arg4;		// Adjust SP 
  goto begindorplaca;   

DoRplacaLP:
  if (_trace) printf("DoRplacaLP:\n");

DoRplacaFP:
  if (_trace) printf("DoRplacaFP:\n");

headdorplaca:
  if (_trace) printf("headdorplaca:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindorplaca:
  if (_trace) printf("begindorplaca:\n");
  /* arg1 has the operand, sign extended if immediate. */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t1 = (u32)(arg6 >> ((4&7)*8));   
  arg2 = (u32)arg6;   		// Read ARG1, the list 
  iSP = iSP - 8;   		// Pop Stack. 
  /* TagType. */
  t3 = t1 & 63;
  t4 = t3 - Type_List;   
  t4 = t4 & ~4L;
  if (t4 != 0)   
    goto rplacaexception;

rplacstore:
  if (_trace) printf("rplacstore:\n");
  t2 = arg1 >> 32;   		// Tag for t2 
  arg1 = (u32)arg1;   		// data for t2 
  /* Memory Read Internal */

vma_memory_read30940:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30942;

vma_memory_read30941:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30944;

vma_memory_read30950:
  /* Merge cdr-code */
  arg6 = t2 & 63;
  arg5 = arg5 & 192;
  arg5 = arg5 | arg6;
  t5 = arg2 + ivory;
  arg6 = (t5 * 4);   
  t7 = LDQ_U(t5);   
  t6 = arg2 - t11;   		// Stack cache offset 
  t8 = ((u64)t6 < (u64)t12) ? 1 : 0;   		// In range? 
  t6 = (arg5 & 0xff) << ((t5&7)*8);   
  t7 = t7 & ~(0xffL << (t5&7)*8);   

force_alignment30952:
  if (_trace) printf("force_alignment30952:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)arg6 = arg1;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write30951;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write30951:
  if (_trace) printf("vma_memory_write30951:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t6 = arg2 - t11;   		// Stack cache offset 
  t5 = (t6 * 8) + t5;  		// reconstruct SCA 
  *(u32 *)t5 = arg1;   		// Store in stack 
  *(u32 *)(t5 + 4) = arg5;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read30942:
  if (_trace) printf("vma_memory_read30942:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read30941;   

vma_memory_read30944:
  if (_trace) printf("vma_memory_read30944:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30943;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30940;   

vma_memory_read30943:
  if (_trace) printf("vma_memory_read30943:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read30947:

vma_memory_read30946:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 1;
  goto performmemoryaction;

/* end DoRplaca */
  /* End of Halfword operand from stack instruction - DoRplaca */
/* start MemoryReadWrite */


memoryreadwrite:
  if (_trace) printf("memoryreadwrite:\n");
  /* Memory Read Internal */

vma_memory_read30954:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30956;

vma_memory_read30955:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read30958;

vma_memory_read30963:
  goto *r0; /* ret */

memoryreadwritedecode:
  if (_trace) printf("memoryreadwritedecode:\n");
  if (t6 == 0) 
    goto vma_memory_read30957;

vma_memory_read30956:
  if (_trace) printf("vma_memory_read30956:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read30955;   

vma_memory_read30958:
  if (_trace) printf("vma_memory_read30958:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30957;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30954;   

vma_memory_read30957:
  if (_trace) printf("vma_memory_read30957:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read30960:

vma_memory_read30959:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 1;
  goto performmemoryaction;

/* end MemoryReadWrite */
/* start DoRplacd */

  /* Halfword operand from stack instruction - DoRplacd */

dorplacd:
  if (_trace) printf("dorplacd:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoRplacdIM:
  if (_trace) printf("DoRplacdIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment30974:
  if (_trace) printf("force_alignment30974:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindorplacd;   

DoRplacdSP:
  if (_trace) printf("DoRplacdSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto headdorplacd;
  arg1 = arg6;		// SP-pop mode, TOS->arg1 
  arg6 = *(u64 *)arg4;   		// Reload TOS 
  iSP = arg4;		// Adjust SP 
  goto begindorplacd;   

DoRplacdLP:
  if (_trace) printf("DoRplacdLP:\n");

DoRplacdFP:
  if (_trace) printf("DoRplacdFP:\n");

headdorplacd:
  if (_trace) printf("headdorplacd:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindorplacd:
  if (_trace) printf("begindorplacd:\n");
  /* arg1 has the operand, sign extended if immediate. */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t1 = (u32)(arg6 >> ((4&7)*8));   
  arg2 = (u32)arg6;   		// Read ARG1, the list 
  iSP = iSP - 8;   		// Pop Stack. 
  /* TagType. */
  t3 = t1 & 63;
  t4 = t3 - Type_Locative;   
  if (t4 == 0) 
    goto rplacstore;
  t4 = t3 - Type_List;   
  if (t4 != 0)   
    goto rplacdexception;
  /* Memory Read Internal */

vma_memory_read30964:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read30966;

vma_memory_read30965:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read30968;

vma_memory_read30973:
  /* TagCdr. */
  arg5 = arg5 >> 6;   
  arg5 = arg5 - Cdr_Normal;   
  if (arg5 != 0)   		// J. if CDR coded 
    goto rplacdexception;
  arg2 = arg2 + 1;		// address of CDR 
  goto rplacstore;   

vma_memory_read30968:
  if (_trace) printf("vma_memory_read30968:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read30967;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read30964;   

vma_memory_read30967:
  if (_trace) printf("vma_memory_read30967:\n");

vma_memory_read30966:
  if (_trace) printf("vma_memory_read30966:\n");
  r0 = (u64)&&return0029;
  goto memoryreadcdrdecode;
return0029:
  goto vma_memory_read30973;   

/* end DoRplacd */
  /* End of Halfword operand from stack instruction - DoRplacd */
/* start DoBranchTrueAndExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueAndExtraPop */

dobranchtrueandextrapop:
  if (_trace) printf("dobranchtrueandextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueAndExtraPopIM:
  if (_trace) printf("DoBranchTrueAndExtraPopIM:\n");

DoBranchTrueAndExtraPopSP:
  if (_trace) printf("DoBranchTrueAndExtraPopSP:\n");

DoBranchTrueAndExtraPopLP:
  if (_trace) printf("DoBranchTrueAndExtraPopLP:\n");

DoBranchTrueAndExtraPopFP:
  if (_trace) printf("DoBranchTrueAndExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto dobrpopextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrpopextrapop:
  if (_trace) printf("dobrpopextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 16;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueAndExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueAndExtraPop */
/* start DoBranchFalseAndExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseAndExtraPop */

dobranchfalseandextrapop:
  if (_trace) printf("dobranchfalseandextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseAndExtraPopIM:
  if (_trace) printf("DoBranchFalseAndExtraPopIM:\n");

DoBranchFalseAndExtraPopSP:
  if (_trace) printf("DoBranchFalseAndExtraPopSP:\n");

DoBranchFalseAndExtraPopLP:
  if (_trace) printf("DoBranchFalseAndExtraPopLP:\n");

DoBranchFalseAndExtraPopFP:
  if (_trace) printf("DoBranchFalseAndExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto dobrnpopextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrnpopextrapop:
  if (_trace) printf("dobrnpopextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 16;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseAndExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseAndExtraPop */
/* start DoBranchTrueAndNoPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueAndNoPop */

dobranchtrueandnopop:
  if (_trace) printf("dobranchtrueandnopop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueAndNoPopIM:
  if (_trace) printf("DoBranchTrueAndNoPopIM:\n");

DoBranchTrueAndNoPopSP:
  if (_trace) printf("DoBranchTrueAndNoPopSP:\n");

DoBranchTrueAndNoPopLP:
  if (_trace) printf("DoBranchTrueAndNoPopLP:\n");

DoBranchTrueAndNoPopFP:
  if (_trace) printf("DoBranchTrueAndNoPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto dobrelsepop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrelsepop:
  if (_trace) printf("dobrelsepop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueAndNoPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueAndNoPop */
/* start DoBranchFalseAndNoPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseAndNoPop */

dobranchfalseandnopop:
  if (_trace) printf("dobranchfalseandnopop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseAndNoPopIM:
  if (_trace) printf("DoBranchFalseAndNoPopIM:\n");

DoBranchFalseAndNoPopSP:
  if (_trace) printf("DoBranchFalseAndNoPopSP:\n");

DoBranchFalseAndNoPopLP:
  if (_trace) printf("DoBranchFalseAndNoPopLP:\n");

DoBranchFalseAndNoPopFP:
  if (_trace) printf("DoBranchFalseAndNoPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto dobrnelsepop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrnelsepop:
  if (_trace) printf("dobrnelsepop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseAndNoPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseAndNoPop */
/* start DoBranchFalseElseNoPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseElseNoPop */

dobranchfalseelsenopop:
  if (_trace) printf("dobranchfalseelsenopop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseElseNoPopIM:
  if (_trace) printf("DoBranchFalseElseNoPopIM:\n");

DoBranchFalseElseNoPopSP:
  if (_trace) printf("DoBranchFalseElseNoPopSP:\n");

DoBranchFalseElseNoPopLP:
  if (_trace) printf("DoBranchFalseElseNoPopLP:\n");

DoBranchFalseElseNoPopFP:
  if (_trace) printf("DoBranchFalseElseNoPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto NEXTINSTRUCTION;
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseElseNoPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseElseNoPop */
/* start DoEqualNumber */

  /* Halfword operand from stack instruction - DoEqualNumber */
  /* arg2 has the preloaded 8 bit operand. */

doequalnumber:
  if (_trace) printf("doequalnumber:\n");

DoEqualNumberSP:
  if (_trace) printf("DoEqualNumberSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoequalnumber;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoEqualNumberLP:
  if (_trace) printf("DoEqualNumberLP:\n");

DoEqualNumberFP:
  if (_trace) printf("DoEqualNumberFP:\n");

begindoequalnumber:
  if (_trace) printf("begindoequalnumber:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t7 = arg3 >> 12;   
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   		// Get ARG1 tag 
  t1 = *(s32 *)(arg1 + 4);   		// t1 is tag of arg2 
  LDS(1, f1, *(u32 *)iSP );   
  t7 = t7 & 1;
  arg2 = *(s32 *)arg1;   
  arg4 = (s32)arg6;
  LDS(2, f2, *(u32 *)arg1 );   
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment30992:
  if (_trace) printf("force_alignment30992:\n");
  if (t6 == 0) 
    goto basic_dispatch30980;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment30984:
  if (_trace) printf("force_alignment30984:\n");
  if (t3 == 0) 
    goto binary_type_dispatch30975;
  /* Here if argument TypeFixnum */
  t2 = (s32)arg4 - (s32)arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch30981:
  if (_trace) printf("basic_dispatch30981:\n");

basic_dispatch30980:
  if (_trace) printf("basic_dispatch30980:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment30993:
  if (_trace) printf("force_alignment30993:\n");
  if (t6 == 0) 
    goto basic_dispatch30985;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment30989:
  if (_trace) printf("force_alignment30989:\n");
  if (t3 == 0) 
    goto binary_type_dispatch30975;
  /* Here if argument TypeSingleFloat */

equalnumbermmexcfltflt:
  if (_trace) printf("equalnumbermmexcfltflt:\n");
  SETFLTT(3,f3, FLTU64(1,f1) == FLTU64(2,f2) ? 2.0:0);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)iSP = t12;   
  if (FLTU64(3, f3) != 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch30986:
  if (_trace) printf("basic_dispatch30986:\n");

basic_dispatch30985:
  if (_trace) printf("basic_dispatch30985:\n");
  /* Here for all other cases */

binary_type_dispatch30975:
  if (_trace) printf("binary_type_dispatch30975:\n");
  goto equalnumbermmexc;   

basic_dispatch30979:
  if (_trace) printf("basic_dispatch30979:\n");

DoEqualNumberIM:
  if (_trace) printf("DoEqualNumberIM:\n");
  t11 = *(u64 *)&(processor->niladdress);   
  arg2 = arg2 << 56;   		// First half of sign extension 
  t12 = *(u64 *)&(processor->taddress);   
  t7 = arg3 >> 12;   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (s32)arg6;
  arg2 = (s64)arg2 >> 56;   		// Second half of sign extension 
  t7 = t7 & 1;
  t3 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = (t3 == Type_Fixnum) ? 1 : 0;   

force_alignment30998:
  if (_trace) printf("force_alignment30998:\n");
  if (t4 == 0) 
    goto basic_dispatch30995;
  /* Here if argument TypeFixnum */
  t2 = (s32)arg4 - (s32)arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch30995:
  if (_trace) printf("basic_dispatch30995:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch30994:
  if (_trace) printf("basic_dispatch30994:\n");

/* end DoEqualNumber */
  /* End of Halfword operand from stack instruction - DoEqualNumber */
/* start DoSetToCdrPushCar */

  /* Halfword operand from stack instruction - DoSetToCdrPushCar */
  /* arg2 has the preloaded 8 bit operand. */

dosettocdrpushcar:
  if (_trace) printf("dosettocdrpushcar:\n");

DoSetToCdrPushCarSP:
  if (_trace) printf("DoSetToCdrPushCarSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetToCdrPushCarLP:
  if (_trace) printf("DoSetToCdrPushCarLP:\n");

DoSetToCdrPushCarFP:
  if (_trace) printf("DoSetToCdrPushCarFP:\n");

begindosettocdrpushcar:
  if (_trace) printf("begindosettocdrpushcar:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t2 = *(s32 *)arg1;   		// Get the operand from the stack. 
  t1 = *(s32 *)(arg1 + 4);   
  t2 = (u32)t2;   
  t3 = t1 & 192;		// Save the old CDR code 
  t5 = t1 - Type_Locative;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 == 0) 
    goto settocdrpushcarlocative;
  r0 = (u64)&&return0030;
  goto carcdrinternal;
return0030:
  /* TagType. */
  arg5 = arg5 & 63;
  arg5 = arg5 | t3;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;   
  *(u32 *)(arg1 + 4) = arg5;   		// write the stack cache 
  t5 = t1 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t2;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

DoSetToCdrPushCarIM:
  goto doistageerror;

/* end DoSetToCdrPushCar */
  /* End of Halfword operand from stack instruction - DoSetToCdrPushCar */
/* start DoSub */

  /* Halfword operand from stack instruction - DoSub */
  /* arg2 has the preloaded 8 bit operand. */

dosub:
  if (_trace) printf("dosub:\n");

DoSubSP:
  if (_trace) printf("DoSubSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindosub;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoSubLP:
  if (_trace) printf("DoSubLP:\n");

DoSubFP:
  if (_trace) printf("DoSubFP:\n");

begindosub:
  if (_trace) printf("begindosub:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  LDS(1, f1, *(u32 *)iSP );   
  t1 = (u32)(arg6 >> ((4&7)*8));   		// ARG1 tag 
  t3 = *(s32 *)(arg1 + 4);   		// ARG2 tag 
  t2 = (s32)arg6;		// ARG1 data 
  t4 = *(s32 *)arg1;   		// ARG2 data 
  LDS(2, f2, *(u32 *)arg1 );   
  /* NIL */
  t9 = t1 & 63;		// Strip off any CDR code bits. 
  t11 = t3 & 63;		// Strip off any CDR code bits. 
  t10 = (t9 == Type_Fixnum) ? 1 : 0;   

force_alignment31038:
  if (_trace) printf("force_alignment31038:\n");
  if (t10 == 0) 
    goto basic_dispatch31009;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31015:
  if (_trace) printf("force_alignment31015:\n");
  if (t12 == 0) 
    goto basic_dispatch31011;
  /* Here if argument TypeFixnum */
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = (s64)((s32)t2 - (s64)(s32)t4); 		// compute 64-bit result 
  if (t5 >> 31) exception(2, t5);  /* subl/v */     // WARNING !!! THIS SHOULD REFLECT THE DIFF FILE 
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  *(u32 *)(iSP + 4) = t9;   		// Semi-cheat, we know temp2 has CDRNext/TypeFixnum 
  iPC = t6;
  *(u32 *)iSP = t5;   
  iCP = t7;
  goto cachevalid;   

basic_dispatch31011:
  if (_trace) printf("basic_dispatch31011:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31016:
  if (_trace) printf("force_alignment31016:\n");
  if (t12 == 0) 
    goto basic_dispatch31012;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation30999;   

basic_dispatch31012:
  if (_trace) printf("basic_dispatch31012:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31017:
  if (_trace) printf("force_alignment31017:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31006;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation31002;   

basic_dispatch31010:
  if (_trace) printf("basic_dispatch31010:\n");

basic_dispatch31009:
  if (_trace) printf("basic_dispatch31009:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment31039:
  if (_trace) printf("force_alignment31039:\n");
  if (t10 == 0) 
    goto basic_dispatch31018;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31024:
  if (_trace) printf("force_alignment31024:\n");
  if (t12 == 0) 
    goto basic_dispatch31020;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation30999:
  if (_trace) printf("simple_binary_arithmetic_operation30999:\n");
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  goto cachevalid;   

basic_dispatch31020:
  if (_trace) printf("basic_dispatch31020:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31025:
  if (_trace) printf("force_alignment31025:\n");
  if (t12 == 0) 
    goto basic_dispatch31021;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation30999;   

basic_dispatch31021:
  if (_trace) printf("basic_dispatch31021:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31026:
  if (_trace) printf("force_alignment31026:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31006;
  /* Here if argument TypeDoubleFloat */

simple_binary_arithmetic_operation31002:
  if (_trace) printf("simple_binary_arithmetic_operation31002:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto simple_binary_arithmetic_operation31003;   

basic_dispatch31019:
  if (_trace) printf("basic_dispatch31019:\n");

basic_dispatch31018:
  if (_trace) printf("basic_dispatch31018:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31040:
  if (_trace) printf("force_alignment31040:\n");
  if (t10 == 0) 
    goto basic_dispatch31027;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31033:
  if (_trace) printf("force_alignment31033:\n");
  if (t12 == 0) 
    goto basic_dispatch31029;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0031;
  goto fetchdoublefloat;
return0031:
  LDT(1, f1, processor->fp0);   

simple_binary_arithmetic_operation31003:
  if (_trace) printf("simple_binary_arithmetic_operation31003:\n");
  arg2 = (u32)t4;   
  r0 = (u64)&&return0032;
  goto fetchdoublefloat;
return0032:
  LDT(2, f2, processor->fp0);   

simple_binary_arithmetic_operation31000:
  if (_trace) printf("simple_binary_arithmetic_operation31000:\n");
  SUBT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0033;
  goto consdoublefloat;
return0033:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;   
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch31029:
  if (_trace) printf("basic_dispatch31029:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31034:
  if (_trace) printf("force_alignment31034:\n");
  if (t12 == 0) 
    goto basic_dispatch31030;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation31001:
  if (_trace) printf("simple_binary_arithmetic_operation31001:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0034;
  goto fetchdoublefloat;
return0034:
  LDT(1, f1, processor->fp0);   
  goto simple_binary_arithmetic_operation31000;   

basic_dispatch31030:
  if (_trace) printf("basic_dispatch31030:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31035:
  if (_trace) printf("force_alignment31035:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31006;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation31001;   

basic_dispatch31028:
  if (_trace) printf("basic_dispatch31028:\n");

basic_dispatch31027:
  if (_trace) printf("basic_dispatch31027:\n");
  /* Here for all other cases */

binary_type_dispatch31005:
  if (_trace) printf("binary_type_dispatch31005:\n");

dosubovfl:
  if (_trace) printf("dosubovfl:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch31007;   

binary_type_dispatch31006:
  if (_trace) printf("binary_type_dispatch31006:\n");
  t1 = t3;
  goto dosubovfl;   

binary_type_dispatch31007:
  if (_trace) printf("binary_type_dispatch31007:\n");

basic_dispatch31008:
  if (_trace) printf("basic_dispatch31008:\n");

DoSubIM:
  if (_trace) printf("DoSubIM:\n");
  t1 = (u32)(arg6 >> ((4&7)*8));   
  t2 = (s32)arg6;		// get ARG1 tag/data 
  t11 = t1 & 63;		// Strip off any CDR code bits. 
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31045:
  if (_trace) printf("force_alignment31045:\n");
  if (t12 == 0) 
    goto basic_dispatch31042;
  /* Here if argument TypeFixnum */
  t3 = t2 - arg2;   		// compute 64-bit result 
  t4 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t10 = (s32)t3;		// compute 32-bit sign-extended result 
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t10 = (t3 == t10) ? 1 : 0;   		// is it the same as the 64-bit result? 
  if (t10 == 0) 		// if not, we overflowed 
    goto dosubovfl;
  *(u32 *)(iSP + 4) = t11;   		// Semi-cheat, we know temp2 has CDRNext/TypeFixnum 
  iPC = t4;
  *(u32 *)iSP = t3;   
  iCP = t5;
  goto cachevalid;   

basic_dispatch31042:
  if (_trace) printf("basic_dispatch31042:\n");
  /* Here for all other cases */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindosub;   

basic_dispatch31041:
  if (_trace) printf("basic_dispatch31041:\n");

/* end DoSub */
  /* End of Halfword operand from stack instruction - DoSub */
/* start DoTag */

  /* Halfword operand from stack instruction - DoTag */
  /* arg2 has the preloaded 8 bit operand. */

dotag:
  if (_trace) printf("dotag:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoTagIM:
  if (_trace) printf("DoTagIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindotag;   

DoTagSP:
  if (_trace) printf("DoTagSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoTagLP:
  if (_trace) printf("DoTagLP:\n");

DoTagFP:
  if (_trace) printf("DoTagFP:\n");

begindotag:
  if (_trace) printf("begindotag:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  arg1 = *(s32 *)(arg1 + 4);   		// Get the tag of the operand 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t3 = Type_Fixnum;
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  *(u32 *)(iSP + 8) = arg1;   
  iSP = iSP + 8;
  goto cachevalid;   

/* end DoTag */
  /* End of Halfword operand from stack instruction - DoTag */
/* start DoEndp */

  /* Halfword operand from stack instruction - DoEndp */
  /* arg2 has the preloaded 8 bit operand. */

doendp:
  if (_trace) printf("doendp:\n");

DoEndpSP:
  if (_trace) printf("DoEndpSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoEndpLP:
  if (_trace) printf("DoEndpLP:\n");

DoEndpFP:
  if (_trace) printf("DoEndpFP:\n");

begindoendp:
  if (_trace) printf("begindoendp:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(u64 *)&(processor->niladdress);   
  arg2 = *(s32 *)(arg1 + 4);   		// Get tag. 
  t2 = *(u64 *)&(processor->taddress);   
  /* TagType. */
  arg2 = arg2 & 63;
  t6 = arg2 - Type_NIL;   		// Compare 
  if (t6 != 0)   
    goto endpnotnil;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)(iSP + 8) = t2;   
  iSP = iSP + 8;
  goto cachevalid;   

endpnil:
  if (_trace) printf("endpnil:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)(iSP + 8) = t1;   
  iSP = iSP + 8;
  goto cachevalid;   

endpnotnil:
  if (_trace) printf("endpnotnil:\n");
  t6 = t6 - 1;   		// Now check for list 
  if (t6 == 0) 
    goto endpnil;
  t6 = arg2 - Type_ListInstance;   
  if (t6 == 0) 
    goto endpnil;

DoEndpIM:
  if (_trace) printf("DoEndpIM:\n");
  arg5 = 0;
  arg2 = 64;
  goto illegaloperand;

/* end DoEndp */
  /* End of Halfword operand from stack instruction - DoEndp */
/* start DoMinusp */

  /* Halfword operand from stack instruction - DoMinusp */
  /* arg2 has the preloaded 8 bit operand. */

dominusp:
  if (_trace) printf("dominusp:\n");

DoMinuspSP:
  if (_trace) printf("DoMinuspSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoMinuspLP:
  if (_trace) printf("DoMinuspLP:\n");

DoMinuspFP:
  if (_trace) printf("DoMinuspFP:\n");

begindominusp:
  if (_trace) printf("begindominusp:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = *(s32 *)(arg1 + 4);   
  t12 = *(u64 *)&(processor->taddress);   
  t2 = *(s32 *)arg1;   
  LDS(1, f1, *(u32 *)arg1 );   
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t5 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment31051:
  if (_trace) printf("force_alignment31051:\n");
  if (t5 == 0) 
    goto basic_dispatch31047;
  /* Here if argument TypeFixnum */
  iPC = t6;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 < 0)   		// T if predicate succeeds 
    t11 = t12;
  *(u64 *)(iSP + 8) = t11;   
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch31047:
  if (_trace) printf("basic_dispatch31047:\n");
  t5 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment31052:
  if (_trace) printf("force_alignment31052:\n");
  if (t5 == 0) 
    goto basic_dispatch31048;
  /* Here if argument TypeSingleFloat */
  iPC = t6;
  *(u64 *)(iSP + 8) = t12;   
  iSP = iSP + 8;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (FLTU64(1, f1) < 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch31048:
  if (_trace) printf("basic_dispatch31048:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch31046:
  if (_trace) printf("basic_dispatch31046:\n");

DoMinuspIM:
  if (_trace) printf("DoMinuspIM:\n");
  t1 = *(u64 *)&(processor->niladdress);   
  arg2 = arg2 << 56;   		// Turned into a signed number 
  t2 = *(u64 *)&(processor->taddress);   
  iSP = iSP + 8;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)arg2 < 0)   		// stall 2 then di 
    t1 = t2;
  *(u64 *)iSP = t1;   		// yes Virginia, we dual issue with above yahoo 
  goto cachevalid;   

/* end DoMinusp */
  /* End of Halfword operand from stack instruction - DoMinusp */
/* start DoPlusp */

  /* Halfword operand from stack instruction - DoPlusp */
  /* arg2 has the preloaded 8 bit operand. */

doplusp:
  if (_trace) printf("doplusp:\n");

DoPluspSP:
  if (_trace) printf("DoPluspSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPluspLP:
  if (_trace) printf("DoPluspLP:\n");

DoPluspFP:
  if (_trace) printf("DoPluspFP:\n");

begindoplusp:
  if (_trace) printf("begindoplusp:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = *(s32 *)(arg1 + 4);   
  t12 = *(u64 *)&(processor->taddress);   
  t2 = *(s32 *)arg1;   
  LDS(1, f1, *(u32 *)arg1 );   
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t5 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment31058:
  if (_trace) printf("force_alignment31058:\n");
  if (t5 == 0) 
    goto basic_dispatch31054;
  /* Here if argument TypeFixnum */
  iPC = t6;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 > 0)   		// T if predicate succeeds 
    t11 = t12;
  *(u64 *)(iSP + 8) = t11;   
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch31054:
  if (_trace) printf("basic_dispatch31054:\n");
  t5 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment31059:
  if (_trace) printf("force_alignment31059:\n");
  if (t5 == 0) 
    goto basic_dispatch31055;
  /* Here if argument TypeSingleFloat */
  iPC = t6;
  *(u64 *)(iSP + 8) = t12;   
  iSP = iSP + 8;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (FLTU64(1, f1) > 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch31055:
  if (_trace) printf("basic_dispatch31055:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch31053:
  if (_trace) printf("basic_dispatch31053:\n");

DoPluspIM:
  if (_trace) printf("DoPluspIM:\n");
  t1 = *(u64 *)&(processor->niladdress);   
  arg2 = arg2 << 56;   		// Turned into a signed number 
  t2 = *(u64 *)&(processor->taddress);   
  iSP = iSP + 8;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)arg2 > 0)   		// stall 2 then di 
    t1 = t2;
  *(u64 *)iSP = t1;   		// yes Virginia, we dual issue with above yahoo 
  goto cachevalid;   

/* end DoPlusp */
  /* End of Halfword operand from stack instruction - DoPlusp */
/* start DoLessp */

  /* Halfword operand from stack instruction - DoLessp */
  /* arg2 has the preloaded 8 bit operand. */

dolessp:
  if (_trace) printf("dolessp:\n");

DoLesspSP:
  if (_trace) printf("DoLesspSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindolessp;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoLesspLP:
  if (_trace) printf("DoLesspLP:\n");

DoLesspFP:
  if (_trace) printf("DoLesspFP:\n");

begindolessp:
  if (_trace) printf("begindolessp:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t7 = arg3 >> 12;   
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   		// Get ARG1 tag 
  t1 = *(s32 *)(arg1 + 4);   		// t1 is tag of arg2 
  LDS(1, f1, *(u32 *)iSP );   
  t7 = t7 & 1;
  arg2 = *(s32 *)arg1;   
  arg4 = (s32)arg6;
  LDS(2, f2, *(u32 *)arg1 );   
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment31077:
  if (_trace) printf("force_alignment31077:\n");
  if (t6 == 0) 
    goto basic_dispatch31065;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment31069:
  if (_trace) printf("force_alignment31069:\n");
  if (t3 == 0) 
    goto binary_type_dispatch31060;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 < 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch31066:
  if (_trace) printf("basic_dispatch31066:\n");

basic_dispatch31065:
  if (_trace) printf("basic_dispatch31065:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment31078:
  if (_trace) printf("force_alignment31078:\n");
  if (t6 == 0) 
    goto basic_dispatch31070;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment31074:
  if (_trace) printf("force_alignment31074:\n");
  if (t3 == 0) 
    goto binary_type_dispatch31060;
  /* Here if argument TypeSingleFloat */

lesspmmexcfltflt:
  if (_trace) printf("lesspmmexcfltflt:\n");
  SETFLTT(3,f3, FLTU64(1,f1) < FLTU64(2,f2) ? 2.0:0);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)iSP = t12;   
  if (FLTU64(3, f3) != 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch31071:
  if (_trace) printf("basic_dispatch31071:\n");

basic_dispatch31070:
  if (_trace) printf("basic_dispatch31070:\n");
  /* Here for all other cases */

binary_type_dispatch31060:
  if (_trace) printf("binary_type_dispatch31060:\n");
  goto lesspmmexc;   

basic_dispatch31064:
  if (_trace) printf("basic_dispatch31064:\n");

DoLesspIM:
  if (_trace) printf("DoLesspIM:\n");
  t11 = *(u64 *)&(processor->niladdress);   
  arg2 = arg2 << 56;   		// First half of sign extension 
  t12 = *(u64 *)&(processor->taddress);   
  t7 = arg3 >> 12;   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (s32)arg6;
  arg2 = (s64)arg2 >> 56;   		// Second half of sign extension 
  t7 = t7 & 1;
  t3 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = (t3 == Type_Fixnum) ? 1 : 0;   

force_alignment31083:
  if (_trace) printf("force_alignment31083:\n");
  if (t4 == 0) 
    goto basic_dispatch31080;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 < 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch31080:
  if (_trace) printf("basic_dispatch31080:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch31079:
  if (_trace) printf("basic_dispatch31079:\n");

/* end DoLessp */
  /* End of Halfword operand from stack instruction - DoLessp */
/* start DoDecrement */

  /* Halfword operand from stack instruction - DoDecrement */
  /* arg2 has the preloaded 8 bit operand. */

dodecrement:
  if (_trace) printf("dodecrement:\n");

DoDecrementSP:
  if (_trace) printf("DoDecrementSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoDecrementLP:
  if (_trace) printf("DoDecrementLP:\n");

DoDecrementFP:
  if (_trace) printf("DoDecrementFP:\n");

begindodecrement:
  if (_trace) printf("begindodecrement:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg3 = *(s32 *)arg1;   		// read tag/data of arg1 
  arg2 = *(s32 *)(arg1 + 4);   
  arg3 = (u32)arg3;   
  t1 = arg2 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment31089:
  if (_trace) printf("force_alignment31089:\n");
  if (t2 == 0) 
    goto basic_dispatch31085;
  /* Here if argument TypeFixnum */
  t2 = *(u64 *)&(processor->mostnegativefixnum);   
  t3 = arg3 - 1;   
  t2 = (arg3 == t2) ? 1 : 0;   
  if (t2 != 0)   
    goto decrementexception;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)arg1 = t3;   
  *(u32 *)(arg1 + 4) = arg2;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch31085:
  if (_trace) printf("basic_dispatch31085:\n");
  t2 = (t1 == Type_SingleFloat) ? 1 : 0;   

force_alignment31090:
  if (_trace) printf("force_alignment31090:\n");
  if (t2 == 0) 
    goto basic_dispatch31086;
  /* Here if argument TypeSingleFloat */
  /* NIL */
  LDS(1, f1, *(u32 *)arg1 );   		// Get the floating data 
  LDS(2, f2, processor->sfp1);   		// constant 1.0 
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  STS( (u32 *)arg1, 0, f0 );   		// Put the floating result 
  goto cachevalid;   

basic_dispatch31086:
  if (_trace) printf("basic_dispatch31086:\n");
  /* Here for all other cases */
  goto decrementexception;   

basic_dispatch31084:
  if (_trace) printf("basic_dispatch31084:\n");

DoDecrementIM:
  goto doistageerror;

/* end DoDecrement */
  /* End of Halfword operand from stack instruction - DoDecrement */
/* start DoMergeCdrNoPop */

  /* Halfword operand from stack instruction - DoMergeCdrNoPop */
  /* arg2 has the preloaded 8 bit operand. */

domergecdrnopop:
  if (_trace) printf("domergecdrnopop:\n");

DoMergeCdrNoPopSP:
  if (_trace) printf("DoMergeCdrNoPopSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindomergecdrnopop;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoMergeCdrNoPopLP:
  if (_trace) printf("DoMergeCdrNoPopLP:\n");

DoMergeCdrNoPopFP:
  if (_trace) printf("DoMergeCdrNoPopFP:\n");

begindomergecdrnopop:
  if (_trace) printf("begindomergecdrnopop:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t1 = *(s32 *)(arg1 + 4);   		// Get the CDR CODE/TAG of arg2 
  t2 = (u32)(arg6 >> ((4&7)*8));   		// Get the CDR CODE/TAG of arg1 

force_alignment31091:
  if (_trace) printf("force_alignment31091:\n");
  t2 = t2 & 192;		// Get Just the CDR code in position 
  t1 = t1 & 63;		// Get the TAG of arg1 
  t3 = t1 | t2;		// Merge the tag of arg2 with the cdr code of arg1 
  *(u32 *)(arg1 + 4) = t3;   		// Replace tag/cdr code no pop 
  goto cachevalid;   

DoMergeCdrNoPopIM:
  goto doistageerror;

/* end DoMergeCdrNoPop */
  /* End of Halfword operand from stack instruction - DoMergeCdrNoPop */
/* start DoEqImmediateHandler */


doeqimmediatehandler:
  if (_trace) printf("doeqimmediatehandler:\n");

DoEqIM:
  if (_trace) printf("DoEqIM:\n");
  arg2 = arg2 << 56;   
  t4 = *(s32 *)(iSP + 4);   		// t4=tag t3=data 
  t3 = *(s32 *)iSP;   
  arg3 = arg3 >> 12;   
  t11 = *(u64 *)&(processor->niladdress);   
  arg2 = (s64)arg2 >> 56;   		// Sign extension of arg2 is complete 
  /* TagType. */
  t4 = t4 & 63;
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = arg3 & 1;		// 1 if no-pop, 0 if pop 
  arg2 = (s32)t3 - (s32)arg2;   
  t4 = t4 ^ Type_Fixnum;   
  iSP = (arg3 * 8) + iSP;  		// Either a stack-push or a stack-write 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t4 = arg2 | t4;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t4 == 0)   
    t11 = t12;
  *(u64 *)iSP = t11;   		// Yes Virginia, this does dual issue with above 
  goto cachevalid;   

/* end DoEqImmediateHandler */
/* start DoIncrement */

  /* Halfword operand from stack instruction - DoIncrement */
  /* arg2 has the preloaded 8 bit operand. */

doincrement:
  if (_trace) printf("doincrement:\n");

DoIncrementSP:
  if (_trace) printf("DoIncrementSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoIncrementLP:
  if (_trace) printf("DoIncrementLP:\n");

DoIncrementFP:
  if (_trace) printf("DoIncrementFP:\n");

begindoincrement:
  if (_trace) printf("begindoincrement:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg3 = *(s32 *)arg1;   		// read tag/data of arg1 
  arg2 = *(s32 *)(arg1 + 4);   
  arg3 = (u32)arg3;   
  t1 = arg2 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment31097:
  if (_trace) printf("force_alignment31097:\n");
  if (t2 == 0) 
    goto basic_dispatch31093;
  /* Here if argument TypeFixnum */
  t2 = *(u64 *)&(processor->mostpositivefixnum);   
  t3 = arg3 + 1;
  t2 = (arg3 == t2) ? 1 : 0;   
  if (t2 != 0)   
    goto incrementexception;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)arg1 = t3;   
  *(u32 *)(arg1 + 4) = arg2;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch31093:
  if (_trace) printf("basic_dispatch31093:\n");
  t2 = (t1 == Type_SingleFloat) ? 1 : 0;   

force_alignment31098:
  if (_trace) printf("force_alignment31098:\n");
  if (t2 == 0) 
    goto basic_dispatch31094;
  /* Here if argument TypeSingleFloat */
  /* NIL */
  LDS(1, f1, *(u32 *)arg1 );   		// Get the floating data 
  LDS(2, f2, processor->sfp1);   		// constant 1.0 
  ADDS(0, f0, 1, f1, 2, f2); /* adds */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  STS( (u32 *)arg1, 0, f0 );   		// Put the floating result 
  goto cachevalid;   

basic_dispatch31094:
  if (_trace) printf("basic_dispatch31094:\n");
  /* Here for all other cases */
  goto incrementexception;   

basic_dispatch31092:
  if (_trace) printf("basic_dispatch31092:\n");

DoIncrementIM:
  goto doistageerror;

/* end DoIncrement */
  /* End of Halfword operand from stack instruction - DoIncrement */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifuncom2.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifungene.as
 ************************************************************************/

  /* Generic dispatching an method lookup */
/* start DoMessageDispatch */

  /* Halfword operand from stack instruction - DoMessageDispatch */
  /* arg2 has the preloaded 8 bit operand. */

domessagedispatch:
  if (_trace) printf("domessagedispatch:\n");

DoMessageDispatchSP:
  if (_trace) printf("DoMessageDispatchSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoMessageDispatchLP:
  if (_trace) printf("DoMessageDispatchLP:\n");

DoMessageDispatchFP:
  if (_trace) printf("DoMessageDispatchFP:\n");

begindomessagedispatch:
  if (_trace) printf("begindomessagedispatch:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg2 = *(s32 *)&processor->control;   
  arg1 = *(s32 *)(iFP + 28);   		// get message tag and data 
  t1 = *(s32 *)(iFP + 24);   
  arg5 = arg2 & 255;		// get number of arguments 
  arg3 = *(s32 *)(iFP + 20);   		// get instance tag and data 
  arg4 = *(s32 *)(iFP + 16);   
  arg5 = arg5 - 4;   		// done if 2 or more arguments (plus 2 extra words) 
  if ((s64)arg5 < 0)   
    goto verifygenericarity;
  t1 = (u32)t1;   
  arg4 = (u32)arg4;   
  r0 = (u64)&&return0035;
  goto lookuphandler;
return0035:
  arg4 = *(u64 *)(iFP + 16);   		// clobbered by |LookupHandler| 
  t3 = t4 - Type_EvenPC;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto message_dispatch31101;
  t3 = t6 & 63;		// Strip CDR code 
  t3 = t3 - Type_NIL;   
  if (t3 == 0) 
    goto message_dispatch31099;
  *(u32 *)(iFP + 16) = t7;   
  *(u32 *)(iFP + 20) = t6;   		// write the stack cache 
  goto message_dispatch31100;   

message_dispatch31099:
  if (_trace) printf("message_dispatch31099:\n");
  *(u32 *)(iFP + 16) = t1;   		// swap message/instance in the frame 
  *(u32 *)(iFP + 20) = arg1;   		// write the stack cache 

message_dispatch31100:
  if (_trace) printf("message_dispatch31100:\n");
  *(u64 *)(iFP + 24) = arg4;   
  /* Convert real continuation to PC. */
  iPC = t4 & 1;
  iPC = t9 + iPC;
  iPC = t9 + iPC;
  goto interpretinstructionforjump;   

message_dispatch31101:
  if (_trace) printf("message_dispatch31101:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = iSP - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t3 = t2 + t3;		// reconstruct VMA 
  arg5 = t3;
  arg2 = 37;
  goto illegaloperand;

DoMessageDispatchIM:
  goto doistageerror;

/* end DoMessageDispatch */
  /* End of Halfword operand from stack instruction - DoMessageDispatch */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifungene.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunfcal.as
 ************************************************************************/

  /* Function calling. */
  /* Start call. */
  /* Finish call. */
/* start DoFinishCallTos */

  /* Halfword 10 bit immediate instruction - DoFinishCallTos */

dofinishcalltos:
  if (_trace) printf("dofinishcalltos:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoFinishCallTosIM:
  if (_trace) printf("DoFinishCallTosIM:\n");

DoFinishCallTosSP:
  if (_trace) printf("DoFinishCallTosSP:\n");

DoFinishCallTosLP:
  if (_trace) printf("DoFinishCallTosLP:\n");

DoFinishCallTosFP:
  if (_trace) printf("DoFinishCallTosFP:\n");
  /* arg1 has operand preloaded. */
  arg1 = (u8)(arg3 >> ((5&7)*8));   		// arg1 contains the disposition (two bits) 
  arg2 = *(s32 *)iSP;   		// Get the number of args 
  iSP = iSP - 8;   		// Pop stack 
  arg2 = (arg2 * 8) + 8;  		// Add 1 and convert to stacked word address 
  goto finishcallmerge;   

/* end DoFinishCallTos */
  /* End of Halfword operand from stack instruction - DoFinishCallTos */
  /* Function entry. */
/* start DoEntryRestAccepted */

  /* Field Extraction instruction - DoEntryRestAccepted */

doentryrestaccepted:
  if (_trace) printf("doentryrestaccepted:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoEntryRestAcceptedIM:
  if (_trace) printf("DoEntryRestAcceptedIM:\n");

DoEntryRestAcceptedSP:
  if (_trace) printf("DoEntryRestAcceptedSP:\n");

DoEntryRestAcceptedLP:
  if (_trace) printf("DoEntryRestAcceptedLP:\n");

DoEntryRestAcceptedFP:
  if (_trace) printf("DoEntryRestAcceptedFP:\n");
  arg5 = *(s32 *)&processor->control;   		// The control register 
  arg4 = arg3 >> 18;   		// Pull down the number of optionals 
  arg1 = (u8)(arg3 >> ((5&7)*8));   		// Extract the 'ptr' field while we are waiting 
  arg4 = arg4 & 255;
  /* arg1=ptr field, arg2=required, arg3=instn, arg4=optionals arg5=control-register */
  t2 = arg5 >> 27;   		// Get the cr.trace-pending bit 
  t1 = arg5 & 255;		// The supplied args 
  if (t2 & 1)   
    goto tracetrap;
  t3 = arg5 >> 17;   
  t4 = *(s32 *)(iSP + 4);   		// Get the tag of the stack top. 

force_alignment31104:
  if (_trace) printf("force_alignment31104:\n");
  if (t3 & 1)   		// J. if apply args 
    goto b_apply_argument_supplied31102;

b_apply_argument_supplied31103:
  t2 = t1 - arg2;   		// t2=supplied-minimum 
  if ((s64)t2 < 0)   		// B. if too few args. 
    goto retryeratoofew;
  arg1 = arg4 - t1;   		// maximum-supplied 
  if ((s64)arg1 < 0)   		// B. rest args. 
    goto retryerarest;
  /* Compute entry position and advance PC/CP accordingly. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   		// get the next PC 
  t3 = t2 << 1;   		// Adjust index to halfword 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0) 		// J. if index zero, no adjustment. 
    goto INTERPRETINSTRUCTION;
  iPC = iPC + t3;		// Compute the new address 
  iPC = iPC & ~1L;		// Make it an DTP-EVEN-PC 
  goto interpretinstructionforjump;   

applysuppra:
  if (_trace) printf("applysuppra:\n");
  arg1 = arg4 - t1;   		// maximum-supplied 
  if ((s64)arg1 < 0)   		// B. rest args. 
    goto retryerarest;
  if ((s64)arg1 > 0)   		// try pulling from applied args. 
    goto pullapplyargs;
  t6 = *(s32 *)(iSP + 4);   		// get tag 
  t6 = t6 & 63;
  t6 = t6 | 64;
  *(u32 *)(iSP + 4) = t6;   		// set tag 
  t2 = t1 - arg2;   		// t2=supplied-minimum 
  t2 = t2 + 1;
  /* Compute entry position and advance PC/CP accordingly. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   		// get the next PC 
  t3 = t2 << 1;   		// Adjust index to halfword 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0) 		// J. if index zero, no adjustment. 
    goto INTERPRETINSTRUCTION;
  iPC = iPC + t3;		// Compute the new address 
  iPC = iPC & ~1L;		// Make it an DTP-EVEN-PC 
  goto interpretinstructionforjump;   

retryeratoofew:
  if (_trace) printf("retryeratoofew:\n");
  arg5 = 0;
  arg2 = 77;
  goto illegaloperand;

retryerarest:
  if (_trace) printf("retryerarest:\n");
  t1 = *(s32 *)(iSP + 4);   		// get tag 
  t1 = t1 & 63;
  t1 = t1 | 64;
  *(u32 *)(iSP + 4) = t1;   		// set tag 
  t2 = arg5 >> 17;   
  t3 = *(s32 *)(iSP + 4);   		// Get the tag of the stack top. 

force_alignment31109:
  if (_trace) printf("force_alignment31109:\n");
  if (t2 & 1)   		// J. if apply args 
    goto b_apply_argument_supplied31107;

b_apply_argument_supplied31108:
  t1 = (arg4 * 8) + iFP;  
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  t1 = Type_List;
  *(u32 *)(iSP + 8) = t2;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto push_apply_args31106;   

push_apply_args31105:
  if (_trace) printf("push_apply_args31105:\n");
  t1 = iSP - 8;   
  t3 = *(s32 *)(t1 + 4);   		// get tag 
  t3 = t3 & 63;
  t3 = t3 | 128;
  *(u32 *)(t1 + 4) = t3;   		// set tag 
  t1 = (arg4 * 8) + iFP;  
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  t1 = Type_List;
  *(u32 *)(iSP + 8) = t2;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  iLP = iLP + 8;
  arg5 = arg5 + 1;
  *(u32 *)&processor->control = arg5;   

push_apply_args31106:
  if (_trace) printf("push_apply_args31106:\n");
  t1 = arg4 - arg2;   
  t1 = t1 + 1;
  /* Compute entry position and advance PC/CP accordingly. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   		// get the next PC 
  t2 = t1 << 1;   		// Adjust index to halfword 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t1 == 0) 		// J. if index zero, no adjustment. 
    goto INTERPRETINSTRUCTION;
  iPC = iPC + t2;		// Compute the new address 
  iPC = iPC & ~1L;		// Make it an DTP-EVEN-PC 
  goto interpretinstructionforjump;   

b_apply_argument_supplied31107:
  if (_trace) printf("b_apply_argument_supplied31107:\n");
  t3 = t3 & 63;
  t3 = t3 - Type_NIL;   
  if (t3 != 0)   		// J. if apply args supplied not nil. 
    goto push_apply_args31105;
  t2 = t2 & 1;		// keep just the apply bit! 
  t2 = t2 << 17;   		// reposition the apply bit 
  iSP = iSP - 8;   		// Pop off the null applied arg. 
  arg5 = arg5 & ~t2;		// Blast the apply arg bit away 
  *(u32 *)&processor->control = arg5;   		// Reset the stored cr bit 
  goto b_apply_argument_supplied31108;   

b_apply_argument_supplied31102:
  if (_trace) printf("b_apply_argument_supplied31102:\n");
  t4 = t4 & 63;
  t4 = t4 - Type_NIL;   
  if (t4 != 0)   		// J. if apply args supplied not nil. 
    goto applysuppra;
  t3 = t3 & 1;		// keep just the apply bit! 
  t3 = t3 << 17;   		// reposition the apply bit 
  iSP = iSP - 8;   		// Pop off the null applied arg. 
  arg5 = arg5 & ~t3;		// Blast the apply arg bit away 
  *(u32 *)&processor->control = arg5;   		// Reset the stored cr bit 
  goto b_apply_argument_supplied31103;   

/* end DoEntryRestAccepted */
  /* End of Halfword operand from stack instruction - DoEntryRestAccepted */
/* start CarCdrInternal */


carcdrinternal:
  if (_trace) printf("carcdrinternal:\n");
  sp = sp + -8;   
  arg2 = (u32)(t2 >> ((zero&7)*8));   
  t5 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_List) ? 1 : 0;   

force_alignment31157:
  if (_trace) printf("force_alignment31157:\n");
  if (t6 == 0) 
    goto basic_dispatch31114;
  /* Here if argument TypeList */
  /* Memory Read Internal */

vma_memory_read31115:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31117;

vma_memory_read31116:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read31119;

vma_memory_read31126:
  t5 = (s32)arg2 - (s32)t2;   
  if (t5 != 0)   		// CAR forwarded, must CDR the hard way 
    goto carcdr_internal31110;
  t1 = arg5;
  t2 = arg6;

carcdr_internal31112:
  if (_trace) printf("carcdr_internal31112:\n");
  t5 = arg5 & 192;		// Extract CDR code. 
  if (t5 != 0)   
    goto basic_dispatch31128;
  /* Here if argument 0 */
  arg6 = arg2 + 1;		// Address of next position is CDR 
  arg5 = Type_List;

basic_dispatch31127:
  if (_trace) printf("basic_dispatch31127:\n");

basic_dispatch31113:
  if (_trace) printf("basic_dispatch31113:\n");

carcdr_internal31111:
  if (_trace) printf("carcdr_internal31111:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch31114:
  if (_trace) printf("basic_dispatch31114:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment31158:
  if (_trace) printf("force_alignment31158:\n");
  if (t6 == 0) 
    goto basic_dispatch31144;
  /* Here if argument TypeNIL */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto basic_dispatch31113;   

basic_dispatch31144:
  if (_trace) printf("basic_dispatch31144:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

carcdr_internal31110:
  if (_trace) printf("carcdr_internal31110:\n");
  arg2 = (u32)(t2 >> ((zero&7)*8));   
  t1 = arg5;
  t2 = arg6;
  /* Memory Read Internal */

vma_memory_read31146:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31148;

vma_memory_read31147:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31150;

vma_memory_read31155:
  goto carcdr_internal31112;   

vma_memory_read31150:
  if (_trace) printf("vma_memory_read31150:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31149;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31146;   

vma_memory_read31149:
  if (_trace) printf("vma_memory_read31149:\n");

vma_memory_read31148:
  if (_trace) printf("vma_memory_read31148:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0036;
  goto memoryreadcdrdecode;
return0036:
  r0 = *(u64 *)sp;   
  goto vma_memory_read31155;   

basic_dispatch31128:
  if (_trace) printf("basic_dispatch31128:\n");
  t6 = (t5 == 128) ? 1 : 0;   

force_alignment31159:
  if (_trace) printf("force_alignment31159:\n");
  if (t6 == 0) 
    goto basic_dispatch31129;
  /* Here if argument 128 */
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read31130:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31132;

vma_memory_read31131:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read31134;
  goto carcdr_internal31111;   

basic_dispatch31129:
  if (_trace) printf("basic_dispatch31129:\n");
  t6 = (t5 == 64) ? 1 : 0;   

force_alignment31160:
  if (_trace) printf("force_alignment31160:\n");
  if (t6 == 0) 
    goto basic_dispatch31141;
  /* Here if argument 64 */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto carcdr_internal31111;   

basic_dispatch31141:
  if (_trace) printf("basic_dispatch31141:\n");
  /* Here for all other cases */
  arg5 = arg2;
  arg2 = 15;
  goto illegaloperand;

vma_memory_read31134:
  if (_trace) printf("vma_memory_read31134:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31133;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31130;   

vma_memory_read31133:
  if (_trace) printf("vma_memory_read31133:\n");

vma_memory_read31132:
  if (_trace) printf("vma_memory_read31132:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0037;
  goto memoryreaddatadecode;
return0037:
  r0 = *(u64 *)sp;   
  goto carcdr_internal31111;   

vma_memory_read31119:
  if (_trace) printf("vma_memory_read31119:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31118;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31115;   

vma_memory_read31118:
  if (_trace) printf("vma_memory_read31118:\n");

vma_memory_read31117:
  if (_trace) printf("vma_memory_read31117:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0038;
  goto memoryreaddatadecode;
return0038:
  r0 = *(u64 *)sp;   
  goto vma_memory_read31126;   

/* end CarCdrInternal */
/* start PullApplyArgsSlowly */


pullapplyargsslowly:
  if (_trace) printf("pullapplyargsslowly:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg4 = *(s32 *)iSP;   		// Get the rest arg 
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  t2 = (u32)(arg4 >> ((zero&7)*8));   
  t3 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = (t3 == Type_List) ? 1 : 0;   

force_alignment31208:
  if (_trace) printf("force_alignment31208:\n");
  if (t4 == 0) 
    goto basic_dispatch31165;
  /* Here if argument TypeList */
  /* Memory Read Internal */

vma_memory_read31166:
  t5 = t2 + ivory;
  arg6 = (t5 * 4);   
  arg5 = LDQ_U(t5);   
  t3 = t2 - t11;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->dataread_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read31168;

vma_memory_read31167:
  t5 = zero + 240;   
  t6 = t6 >> (arg5 & 63);   
  t5 = t5 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t6 & 1)   
    goto vma_memory_read31170;

vma_memory_read31177:
  t3 = (s32)t2 - (s32)arg4;   
  if (t3 != 0)   		// CAR forwarded, must CDR the hard way 
    goto carcdr_internal31161;
  arg3 = arg5;
  arg4 = arg6;

carcdr_internal31163:
  if (_trace) printf("carcdr_internal31163:\n");
  t3 = arg5 & 192;		// Extract CDR code. 
  if (t3 != 0)   
    goto basic_dispatch31179;
  /* Here if argument 0 */
  arg6 = t2 + 1;		// Address of next position is CDR 
  arg5 = Type_List;

basic_dispatch31178:
  if (_trace) printf("basic_dispatch31178:\n");

basic_dispatch31164:
  if (_trace) printf("basic_dispatch31164:\n");

carcdr_internal31162:
  if (_trace) printf("carcdr_internal31162:\n");
  *(u32 *)iSP = arg4;   		// Push the pulled argument 
  *(u32 *)(iSP + 4) = arg3;   		// write the stack cache 
  t1 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;   		// Push the new rest arg 
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  arg2 = *(s32 *)&processor->control;   
  t2 = arg2 & 255;		// Get current arg size. 
  arg2 = arg2 & ~255L;
  t2 = t2 + 1;
  arg2 = t2 + arg2;		// Update the arg size 
  *(u32 *)&processor->control = arg2;   
  iLP = iLP + 8;
  goto INTERPRETINSTRUCTION;   

basic_dispatch31165:
  if (_trace) printf("basic_dispatch31165:\n");
  t4 = (t3 == Type_NIL) ? 1 : 0;   

force_alignment31209:
  if (_trace) printf("force_alignment31209:\n");
  if (t4 == 0) 
    goto basic_dispatch31195;
  /* Here if argument TypeNIL */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto basic_dispatch31164;   

basic_dispatch31195:
  if (_trace) printf("basic_dispatch31195:\n");
  /* Here for all other cases */
  arg1 = arg1;
  goto pullapplyargstrap;

carcdr_internal31161:
  if (_trace) printf("carcdr_internal31161:\n");
  t2 = (u32)(arg4 >> ((zero&7)*8));   
  arg3 = arg5;
  arg4 = arg6;
  /* Memory Read Internal */

vma_memory_read31197:
  t5 = t2 + ivory;
  arg6 = (t5 * 4);   
  arg5 = LDQ_U(t5);   
  t3 = t2 - t11;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->cdr_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read31199;

vma_memory_read31198:
  t5 = zero + 192;   
  t6 = t6 >> (arg5 & 63);   
  t5 = t5 >> (arg5 & 63);   
  if (t6 & 1)   
    goto vma_memory_read31201;

vma_memory_read31206:
  goto carcdr_internal31163;   

vma_memory_read31199:
  if (_trace) printf("vma_memory_read31199:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg6 = *(s32 *)t3;   
  arg5 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read31198;   

vma_memory_read31201:
  if (_trace) printf("vma_memory_read31201:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read31200;
  t2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31197;   

vma_memory_read31200:
  if (_trace) printf("vma_memory_read31200:\n");
  t6 = *(u64 *)&(processor->cdr);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read31203:
  /* Perform memory action */
  arg1 = t6;
  arg2 = 9;
  goto performmemoryaction;

basic_dispatch31179:
  if (_trace) printf("basic_dispatch31179:\n");
  t4 = (t3 == 128) ? 1 : 0;   

force_alignment31210:
  if (_trace) printf("force_alignment31210:\n");
  if (t4 == 0) 
    goto basic_dispatch31180;
  /* Here if argument 128 */
  t2 = t2 + 1;
  /* Memory Read Internal */

vma_memory_read31181:
  t5 = t2 + ivory;
  arg6 = (t5 * 4);   
  arg5 = LDQ_U(t5);   
  t3 = t2 - t11;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->dataread_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read31183;

vma_memory_read31182:
  t5 = zero + 240;   
  t6 = t6 >> (arg5 & 63);   
  t5 = t5 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t6 & 1)   
    goto vma_memory_read31185;
  goto carcdr_internal31162;   

basic_dispatch31180:
  if (_trace) printf("basic_dispatch31180:\n");
  t4 = (t3 == 64) ? 1 : 0;   

force_alignment31211:
  if (_trace) printf("force_alignment31211:\n");
  if (t4 == 0) 
    goto basic_dispatch31192;
  /* Here if argument 64 */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto carcdr_internal31162;   

basic_dispatch31192:
  if (_trace) printf("basic_dispatch31192:\n");
  /* Here for all other cases */
  arg5 = t2;
  arg2 = 15;
  goto illegaloperand;

vma_memory_read31183:
  if (_trace) printf("vma_memory_read31183:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg6 = *(s32 *)t3;   
  arg5 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read31182;   

vma_memory_read31185:
  if (_trace) printf("vma_memory_read31185:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read31184;
  t2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31181;   

vma_memory_read31184:
  if (_trace) printf("vma_memory_read31184:\n");
  t6 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read31189:
  if (_trace) printf("vma_memory_read31189:\n");
  t5 = t6 & MemoryActionTransform;
  if (t5 == 0) 
    goto vma_memory_read31188;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto carcdr_internal31162;   

vma_memory_read31188:

vma_memory_read31187:
  /* Perform memory action */
  arg1 = t6;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31168:
  if (_trace) printf("vma_memory_read31168:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg6 = *(s32 *)t3;   
  arg5 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read31167;   

vma_memory_read31170:
  if (_trace) printf("vma_memory_read31170:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read31169;
  t2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31166;   

vma_memory_read31169:
  if (_trace) printf("vma_memory_read31169:\n");
  t6 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read31174:
  if (_trace) printf("vma_memory_read31174:\n");
  t5 = t6 & MemoryActionTransform;
  if (t5 == 0) 
    goto vma_memory_read31173;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read31177;   

vma_memory_read31173:

vma_memory_read31172:
  /* Perform memory action */
  arg1 = t6;
  arg2 = 0;
  goto performmemoryaction;

/* end PullApplyArgsSlowly */
/* start DoLocateLocals */

  /* Halfword operand from stack instruction - DoLocateLocals */
  /* arg2 has the preloaded 8 bit operand. */

dolocatelocals:
  if (_trace) printf("dolocatelocals:\n");

DoLocateLocalsSP:
  if (_trace) printf("DoLocateLocalsSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoLocateLocalsLP:
  if (_trace) printf("DoLocateLocalsLP:\n");

DoLocateLocalsFP:
  if (_trace) printf("DoLocateLocalsFP:\n");

begindolocatelocals:
  if (_trace) printf("begindolocatelocals:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)&processor->control;   		// The control register 
  iLP = iSP;
  t3 = iLP - iFP;   		// arg size including the fudge 2 
  t3 = t3 >> 3;   		// adjust arg size to words 
  t2 = t1 & 255;		// argument size 
  t2 = t2 - 2;   		// corrected arg size 
  t1 = t1 & ~255L;
  t1 = t1 | t3;		// replace the arg size 
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t2;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  *(u32 *)&processor->control = t1;   
  goto NEXTINSTRUCTION;   

DoLocateLocalsIM:
  goto doistageerror;

/* end DoLocateLocals */
  /* End of Halfword operand from stack instruction - DoLocateLocals */
  /* Returning. */
/* start DoReturnMultiple */

  /* Halfword operand from stack instruction - DoReturnMultiple */
  /* arg2 has the preloaded 8 bit operand. */

doreturnmultiple:
  if (_trace) printf("doreturnmultiple:\n");

DoReturnMultipleSP:
  if (_trace) printf("DoReturnMultipleSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoReturnMultipleLP:
  if (_trace) printf("DoReturnMultipleLP:\n");

DoReturnMultipleFP:
  if (_trace) printf("DoReturnMultipleFP:\n");

begindoreturnmultiple:
  if (_trace) printf("begindoreturnmultiple:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)(arg1 + 4);   		// Fetch the tag for type-check 
  arg1 = *(s32 *)arg1;   		// Fetch the data 
  t2 = t1 - Type_Fixnum;   
  t2 = t2 & 63;		// Strip CDR code 
  if (t2 != 0)   
    goto returnmultipleio;
  arg1 = (u32)arg1;   		// Discard dtp-fixnum tag word 

returnmultipletop:
  if (_trace) printf("returnmultipletop:\n");
  arg5 = *(s32 *)&processor->control;   
  t3 = (12) << 16;   
  t2 = iSP + 8;
  t1 = arg1 << 3;   		// Value bytes 
  t3 = t3 & arg5;		// Mask 
  t3 = t3 >> 18;   		// Shift disposition bits into place. 
  arg3 = t2 - t1;   		// Compute position of value(s) 
  arg6 = *(u64 *)&(processor->stackcachedata);   
  arg4 = t3 - 2;   		// arg4 -2=effect -1=value 0=return 1=multiple 
  if ((s64)arg4 < 0)   
    goto returnmultiplesingle;
  /* Restore machine state from frame header. */
  t3 = *(s32 *)iFP;   
  t1 = (1792) << 16;   
  t5 = *(s32 *)&processor->continuation;   
  t1 = arg5 & t1;		// Mask 
  t2 = *(s32 *)(iFP + 4);   
  t7 = iCP;
  if (t1 != 0)   		// Need to cleanup frame first 
    goto handleframecleanup;
  t3 = (u32)t3;   
  t4 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t6 = *(s32 *)(iFP + 8);   		// Get saved control register 
  /* TagType. */
  t2 = t2 & 63;
  /* Restore the PC. */
  if (arg4 == 0) 
    goto abandon_frame_simple31213;
  iPC = t5 << 1;   		// Assume even PC 
  t1 = t4 & 1;
  t7 = *(u64 *)&(processor->continuationcp);   
  iPC = iPC + t1;

abandon_frame_simple31213:
  if (_trace) printf("abandon_frame_simple31213:\n");
  /* Restore the saved continuation */
  *((u32 *)(&processor->continuation)+1) = t2;   
  t1 = arg5 >> 9;   		// Get the caller frame size into place 
  *(u32 *)&processor->continuation = t3;   
  iSP = iFP - 8;   		// Restore the stack pointer. 
  *(u64 *)&processor->continuationcp = zero;   
  t1 = t1 & 255;		// Mask just the caller frame size. 
  t1 = (t1 * 8) + 0;  		// *8 
  t2 = (2048) << 16;   
  t2 = t2 & arg5;
  t3 = *(s32 *)&processor->interruptreg;   		// Get the preempt-pending bit 
  t6 = t2 | t6;		// Sticky trace pending bit. 
  t4 = *(u64 *)&(processor->please_stop);   		// Get the trap/suspend bits 
  iFP = iFP - t1;   		// Restore the frame pointer. 
  *(u32 *)&processor->control = t6;   		// Restore the control register 
  t1 = t6 & 255;		// extract the argument size 
  t3 = t3 & 1;
  t3 = t4 | t3;
  *(u64 *)&processor->stop_interpreter = t3;   
  iLP = (t1 * 8) + iFP;  		// Restore the local pointer. 
  arg6 = ((u64)iFP < (u64)arg6) ? 1 : 0;   		// ARG6 = stack-cache underflow 
  t4 = iSP + 8;		// Compute destination of copy 
  t3 = arg1;		// Values 
  t1 = *(u64 *)&(processor->cdrcodemask);   		// mask for CDR codes 
  goto stack_block_copy31214;   

stack_block_copy31215:
  if (_trace) printf("stack_block_copy31215:\n");
  t3 = t3 - 1;   
  t2 = *(u64 *)arg3;   		// Get a word from source 
  arg3 = arg3 + 8;		// advance from position 
  t2 = t2 & ~t1;		// Strip off CDR code 
  *(u64 *)t4 = t2;   		// Put word in destination 
  t4 = t4 + 8;		// advance to position 

stack_block_copy31214:
  if ((s64)t3 > 0)   
    goto stack_block_copy31215;
  iSP = (arg1 * 8) + iSP;  		// Adjust iSP over returned values 
  /* arg4 -2=effect -1=value 0=return 1=multiple */
  if (arg4 == 0) 
    goto returnmultiplereturn;

returnmultiplemultiple:
  if (_trace) printf("returnmultiplemultiple:\n");
  t1 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg1;   		// push the MV return count 
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;

returnmultipledone:
  if (_trace) printf("returnmultipledone:\n");
  if (arg6 != 0)   
    goto returnmultipleunderflow;
  arg2 = t7;
  if (t7 != 0)   
    goto interpretinstructionpredicted;
  if (arg4 != 0)   
    goto interpretinstructionforbranch;
  goto INTERPRETINSTRUCTION;   		// Return-multiple done 

returnmultipleunderflow:
  if (_trace) printf("returnmultipleunderflow:\n");
  goto stackcacheunderflowcheck;

returnmultiplesingle:
  if (_trace) printf("returnmultiplesingle:\n");
  arg3 = *(u64 *)arg3;   
  t1 = *(u64 *)&(processor->niladdress);   
  arg3 = arg3 << 26;   		// Clear cdr 
  arg3 = arg3 >> 26;   		// Clear cdr 
  if (arg1 == 0)   
    arg3 = t1;
  goto returncommontail;   

returnmultiplereturn:
  if (_trace) printf("returnmultiplereturn:\n");
  if (arg2 != 0)   
    goto returnmultipledone;
  t1 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg1;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto returnmultipledone;   

DoReturnMultipleIM:
  if (_trace) printf("DoReturnMultipleIM:\n");
  arg1 = arg2;
  arg2 = zero + 1;   
  goto returnmultipletop;   

returnmultipleio:
  if (_trace) printf("returnmultipleio:\n");
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

/* end DoReturnMultiple */
  /* End of Halfword operand from stack instruction - DoReturnMultiple */
/* start HANDLEFRAMECLEANUP */


handleframecleanup:
  if (_trace) printf("handleframecleanup:\n");
  iSP = *(u64 *)&(processor->restartsp);   		// Restore SP to instruction start 
  arg5 = *(s32 *)&processor->control;   		// Get control register 

cleanup_frame31218:
  if (_trace) printf("cleanup_frame31218:\n");
  t1 = (1024) << 16;   
  t4 = *(s32 *)&processor->catchblock;   
  t4 = (u32)t4;   
  t2 = t1 & arg5;
  if (t2 == 0) 		// J. if cr.cleanup-catch is 0 
    goto cleanup_frame31217;
  /* Convert VMA to stack cache address */
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = t4 - t2;   		// stack cache base relative offset 
  t3 = (t2 * 8) + t3;  		// reconstruct SCA 
  t6 = *(s32 *)(t3 + 16);   
  t5 = *(s32 *)(t3 + 20);   
  t6 = (u32)t6;   
  t2 = *(s32 *)(t3 + 8);   
  t1 = *(s32 *)(t3 + 12);   
  t2 = (u32)t2;   
  t12 = t1 & 64;
  if (t12 != 0)   		// J. if catch block is UWP variety. 
    goto handleunwindprotect;
  t3 = (1024) << 16;   
  t2 = t5 & 64;		// Extract the catchcleanup bit 
  t2 = t2 << 20;   		// Shift into place for CR 
  t3 = arg5 & ~t3;
  arg5 = t3 | t2;
  *(u32 *)&processor->control = arg5;   
  /* TagType. */
  t5 = t5 & 63;
  t5 = t5 << 32;   
  t6 = t6 | t5;
  *(u64 *)&processor->catchblock = t6;   
  goto cleanup_frame31218;   

cleanup_frame31217:
  if (_trace) printf("cleanup_frame31217:\n");
  t1 = (512) << 16;   
  t2 = t1 & arg5;
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  if (t2 == 0) 		// J. if cr.cleanup-bindings is 0. 
    goto cleanup_frame31216;

cleanup_frame31219:
  if (_trace) printf("cleanup_frame31219:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  t2 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & t2;
  t4 = t4 & ~t2;		// Turn off the bit 
  if (t3 != 0)   
    goto g31220;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g31220:
  if (_trace) printf("g31220:\n");
  /* Memory Read Internal */

vma_memory_read31221:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t6 = (t10 * 4);   
  t7 = LDQ_U(t10);   
  t8 = t1 - t8;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->bindread_mask);   
  t9 = ((u64)t8 < (u64)t9) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((t10&7)*8));   
  if (t9 != 0)   
    goto vma_memory_read31223;

vma_memory_read31222:
  t10 = zero + 224;   
  t11 = t11 >> (t7 & 63);   
  t10 = t10 >> (t7 & 63);   
  if (t11 & 1)   
    goto vma_memory_read31225;

vma_memory_read31230:
  /* Memory Read Internal */

vma_memory_read31231:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t5 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t2 = (t10 * 4);   
  t3 = LDQ_U(t10);   
  t8 = t5 - t8;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->bindread_mask);   
  t9 = ((u64)t8 < (u64)t9) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t3 = (u8)(t3 >> ((t10&7)*8));   
  if (t9 != 0)   
    goto vma_memory_read31233;

vma_memory_read31232:
  t10 = zero + 224;   
  t11 = t11 >> (t3 & 63);   
  t10 = t10 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (t11 & 1)   
    goto vma_memory_read31235;

vma_memory_read31240:
  /* Memory Read Internal */

vma_memory_read31241:
  t10 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t12 = t2 + ivory;
  t11 = *(s32 *)&processor->scovlimit;   
  t9 = (t12 * 4);   
  t8 = LDQ_U(t12);   
  t10 = t2 - t10;   		// Stack cache offset 
  t11 = ((u64)t10 < (u64)t11) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  t8 = (u8)(t8 >> ((t12&7)*8));   
  if (t11 != 0)   
    goto vma_memory_read31243;

vma_memory_read31242:
  t10 = *(u64 *)&(processor->bindwrite_mask);   
  t12 = zero + 224;   
  t10 = t10 >> (t8 & 63);   
  t12 = t12 >> (t8 & 63);   
  if (t10 & 1)   
    goto vma_memory_read31245;

vma_memory_read31250:
  /* Merge cdr-code */
  t9 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | t9;
  t10 = t2 + ivory;
  t9 = (t10 * 4);   
  t12 = LDQ_U(t10);   
  t11 = (t8 & 0xff) << ((t10&7)*8);   
  t12 = t12 & ~(0xffL << (t10&7)*8);   

force_alignment31253:
  if (_trace) printf("force_alignment31253:\n");
  t12 = t12 | t11;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  STQ_U(t10, t12);   
  t10 = *(s32 *)&processor->scovlimit;   
  t11 = t2 - t11;   		// Stack cache offset 
  t10 = ((u64)t11 < (u64)t10) ? 1 : 0;   		// In range? 
  *(u32 *)t9 = t6;   
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write31252;

vma_memory_write31251:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
  *(u32 *)&processor->bindingstackpointer = t1;   		// vma only 
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   
  arg5 = *(s32 *)&processor->control;   
  t1 = (512) << 16;   
  t2 = t1 & arg5;
  if (t2 != 0)   		// J. if cr.cleanup-bindings is 0. 
    goto cleanup_frame31219;
  t2 = *(s32 *)&processor->interruptreg;   
  t3 = t2 & 2;
  t3 = (t3 == 2) ? 1 : 0;   
  t2 = t2 | t3;
  *(u32 *)&processor->interruptreg = t2;   
  if (t2 == 0) 
    goto check_preempt_request31254;
  *(u64 *)&processor->stop_interpreter = t2;   

check_preempt_request31254:
  if (_trace) printf("check_preempt_request31254:\n");

cleanup_frame31216:
  if (_trace) printf("cleanup_frame31216:\n");
  t3 = (256) << 16;   
  t2 = t3 & arg5;
  if (t2 == 0) 
    goto INTERPRETINSTRUCTION;
  arg5 = zero;
  arg2 = 79;
  goto illegaloperand;
  goto INTERPRETINSTRUCTION;   		// Retry the instruction 

vma_memory_write31252:
  if (_trace) printf("vma_memory_write31252:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t11 * 8) + t10;  		// reconstruct SCA 
  *(u32 *)t10 = t6;   		// Store in stack 
  *(u32 *)(t10 + 4) = t8;   		// write the stack cache 
  goto vma_memory_write31251;   

vma_memory_read31243:
  if (_trace) printf("vma_memory_read31243:\n");
  t11 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t10 * 8) + t11;  		// reconstruct SCA 
  t9 = *(s32 *)t10;   
  t8 = *(s32 *)(t10 + 4);   		// Read from stack cache 
  goto vma_memory_read31242;   

vma_memory_read31245:
  if (_trace) printf("vma_memory_read31245:\n");
  if ((t12 & 1) == 0)   
    goto vma_memory_read31244;
  t2 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read31241;   

vma_memory_read31244:
  if (_trace) printf("vma_memory_read31244:\n");
  t10 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t12 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t12 = (t12 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t12;   		// Get the memory action 

vma_memory_read31247:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read31233:
  if (_trace) printf("vma_memory_read31233:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read31232;   

vma_memory_read31235:
  if (_trace) printf("vma_memory_read31235:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31234;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31231;   

vma_memory_read31234:
  if (_trace) printf("vma_memory_read31234:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31237:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read31223:
  if (_trace) printf("vma_memory_read31223:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read31222;   

vma_memory_read31225:
  if (_trace) printf("vma_memory_read31225:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31224;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read31221;   

vma_memory_read31224:
  if (_trace) printf("vma_memory_read31224:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31227:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 2;
  goto performmemoryaction;

/* end HANDLEFRAMECLEANUP */
/* start StackCacheUnderflowCheck */


stackcacheunderflowcheck:
  if (_trace) printf("stackcacheunderflowcheck:\n");
  t1 = *(u64 *)&(processor->stackcachedata);   
  t4 = *(u64 *)&(processor->restartsp);   		// Preserve through instruction's original SP 
  t3 = t1 - iFP;   		// Number of words*8 to fill iff positive 
  if ((s64)t3 <= 0)  
    goto interpretinstructionforbranch;
  t3 = (s64)t3 >> 3;   		// Convert to a word count 
  t4 = t4 + 8;		// Account for the inclusive limit 
  if ((s64)t3 <= 0)  		// in case only low three bits nonzero 
    goto interpretinstructionforbranch;
  r0 = (u64)&&return0039;
  goto stackcacheunderflow;
return0039:
  goto interpretinstructionforbranch;   

/* end StackCacheUnderflowCheck */
/* start StackCacheUnderflow */


stackcacheunderflow:
  if (_trace) printf("stackcacheunderflow:\n");
  t2 = (t3 * 8) + t1;  		// Compute target address for shift 
  t5 = t4 - t1;   		// Compute number of elements to preserve 
  t5 = (s64)t5 >> 3;   		// Convert to word count 
  /* Shove everything up */
  t1 = (t5 * 8) + t1;  		// Adjust to end of source block 
  t2 = (t5 * 8) + t2;  		// Adjust to end of target block 
  goto stack_block_copy31255;   

stack_block_copy31256:
  if (_trace) printf("stack_block_copy31256:\n");
  t1 = t1 - 8;   		// advance from position 
  t5 = t5 - 1;   
  t7 = *(u64 *)t1;   		// Get a word from source 
  t2 = t2 - 8;   		// advance to position 
  *(u64 *)t2 = t7;   		// Put word in destination 

stack_block_copy31255:
  if ((s64)t5 > 0)   
    goto stack_block_copy31256;
  /* Adjust stack cache relative registers */
  iFP = (t3 * 8) + iFP;  
  t4 = *(u64 *)&(processor->restartsp);   
  iSP = (t3 * 8) + iSP;  
  iLP = (t3 * 8) + iLP;  
  t4 = (t3 * 8) + t4;  
  /* Fill freshly opened slots of stack cache from memory */
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = *(u64 *)&(processor->stackcachedata);   
  *(u64 *)&processor->restartsp = t4;   
  t1 = t1 - t3;   		// Compute new base address of stack cache 
  t4 = *(u64 *)&(processor->stackcachetopvma);   		// Top of cache 
  *(u64 *)&processor->stackcachebasevma = t1;   
  t4 = t4 - t3;   		// Adjust top of cache 
  *(u64 *)&processor->stackcachetopvma = t4;   
  t7 = t1 + ivory;
  t5 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  goto stack_fill31257;   

stack_fill31258:
  if (_trace) printf("stack_fill31258:\n");
  t7 = t1 + ivory;
  t5 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  t3 = t3 - 1;   
  t1 = t1 + 1;		// advance vma position 
  *(u32 *)t2 = t5;   
  *(u32 *)(t2 + 4) = t4;   		// write the stack cache 
  t2 = t2 + 8;		// advance sca position 

stack_fill31257:
  if ((s64)t3 > 0)   
    goto stack_fill31258;
  goto *r0; /* ret */

/* end StackCacheUnderflow */
/* start StackCacheOverflowHandler */


stackcacheoverflowhandler:
  if (_trace) printf("stackcacheoverflowhandler:\n");
  /* Stack cache overflow detected */
  t1 = zero + 256;   
  t1 = t1 + arg2;		// Account for what we're about to push 
  t1 = (t1 * 8) + iSP;  		// SCA of desired end of cache 
  iSP = *(u64 *)&(processor->restartsp);   
  t4 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t4 = t1 - t4;   		// New limit*8 
  t4 = t4 >> 3;   
  *(u32 *)&processor->scovlimit = t4;   		// Update stack cache limit 
  /* Check that the page underlying the end of the stack cache is accessible */
  /* Convert stack cache address to VMA */
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t1 - t4;   		// stack cache base relative offset 
  t4 = t4 >> 3;   		// convert byte address to word address 
  t3 = t4 + t3;		// reconstruct VMA 
  t5 = *(u64 *)&(processor->vmattributetable);   		// Per-page attributes table 
  t4 = t3 >> (MemoryPage_AddressShift & 63);   		// Index into the attributes table 
  t5 = t4 + t5;		// Address of the page's attributes 
  t4 = LDQ_U(t5);   		// Get the quadword with the page's attributes 
  *(u64 *)&processor->vma = t3;   		// Stash the VMA 
  t4 = (u8)(t4 >> ((t5&7)*8));   		// Extract the page's attributes 
  if (t4 == 0) 		// Non-existent page 
    goto pagenotresident;
  t5 = t4 & VMAttribute_AccessFault;
  if (t5 != 0)   		// Access fault 
    goto pagefaultrequesthandler;
  t5 = t4 & VMAttribute_WriteFault;
  if (t5 != 0)   		// Write fault 
    goto pagewritefault;
  /* Check if we must dump the cache */
  t4 = *(s32 *)&processor->scovlimit;   		// New stack cache limit (words) 
  t5 = *(u64 *)&(processor->stackcachesize);   		// Absolute size of the cache (words) 
  t5 = ((s64)t4 <= (s64)t5) ? 1 : 0;   
  if (t5 != 0)   		// We're done if new limit is less than absolute limit 
    goto INTERPRETINSTRUCTION;
  /* Dump the stack cache to make room */
  t1 = zero + 896;   
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Stack cache base VMA 
  t3 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  *(u32 *)&processor->scovdumpcount = t1;   		// Will be destructively modified 
  t5 = t2 + ivory;		// Starting address of tags 
  t2 = (t5 * 4);   		// Starting address of data 
  /* Dump the data */
  goto stack_dump31259;   

stack_dump31260:
  if (_trace) printf("stack_dump31260:\n");
  t4 = *(s32 *)t3;   		// Get data word 
  t1 = t1 - 1;   
  t3 = t3 + 8;		// Advance SCA position 
  *(u32 *)t2 = t4;   		// Save data word 
  t2 = t2 + 4;		// Advance VMA position 

stack_dump31259:
  if ((s64)t1 > 0)   
    goto stack_dump31260;
  /* Dump the tags */
  t1 = *(s32 *)&processor->scovdumpcount;   		// Restore the count 
  t2 = t5;		// Restore tag VMA 
  t4 = t1 << 3;   
  t3 = t3 - t4;   		// Restore orginal SCA 
  goto stack_dump31261;   

stack_dump31262:
  if (_trace) printf("stack_dump31262:\n");
  t1 = t1 - 1;   
  t4 = *(s32 *)(t3 + 4);   		// Get tag word 
  t3 = t3 + 8;		// Advance SCA position 
  t5 = LDQ_U(t2);   		// Get packed tags word 
  t4 = (t4 & 0xff) << ((t2&7)*8);   		// Position the new tag 
  t5 = t5 & ~(0xffL << (t2&7)*8);   		// Remove old tag 
  t5 = t4 | t5;		// Put in new byte 
  STQ_U(t2, t5);   		// Save packed tags word 
  t2 = t2 + 1;		// Advance VMA position 

stack_dump31261:
  if ((s64)t1 > 0)   
    goto stack_dump31262;
  t1 = zero + 896;   
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Stack cache base VMA 
  t4 = *(u64 *)&(processor->stackcachetopvma);   		// Top of cache 
  t5 = *(s32 *)&processor->scovlimit;   		// Cache limit in words 
  t2 = t2 + t1;		// Adjust cache base VMA 
  t4 = t4 + t1;		// Adjust top of cache 
  t5 = t5 - t1;   		// Adjust limit 
  *(u64 *)&processor->stackcachebasevma = t2;   		// Save update 
  *(u64 *)&processor->stackcachetopvma = t4;   
  *(u32 *)&processor->scovlimit = t5;   
  /* Move the cache down */
  t3 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t2 = (t1 * 8) + t3;  		// SCA of first word of new base 
  goto stack_block_copy31263;   

stack_block_copy31264:
  if (_trace) printf("stack_block_copy31264:\n");
  t1 = t1 - 1;   
  t5 = *(u64 *)t2;   		// Get a word from source 
  t2 = t2 + 8;		// advance from position 
  *(u64 *)t3 = t5;   		// Put word in destination 
  t3 = t3 + 8;		// advance to position 

stack_block_copy31263:
  if ((s64)t1 > 0)   
    goto stack_block_copy31264;
  /* Adjust stack cache relative registers */
  t1 = zero + 896;   
  t1 = t1 << 3;   		// Convert to SCA adjustment 
  iSP = iSP - t1;   
  iFP = iFP - t1;   
  iLP = iLP - t1;   
  *(u64 *)&processor->restartsp = iSP;   
  goto INTERPRETINSTRUCTION;   

/* end StackCacheOverflowHandler */
/* start DoReturnKludge */

  /* Halfword operand from stack instruction - DoReturnKludge */
  /* arg2 has the preloaded 8 bit operand. */

doreturnkludge:
  if (_trace) printf("doreturnkludge:\n");

DoReturnKludgeSP:
  if (_trace) printf("DoReturnKludgeSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoReturnKludgeLP:
  if (_trace) printf("DoReturnKludgeLP:\n");

DoReturnKludgeFP:
  if (_trace) printf("DoReturnKludgeFP:\n");

begindoreturnkludge:
  if (_trace) printf("begindoreturnkludge:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)(arg1 + 4);   
  arg2 = *(s32 *)arg1;   
  t2 = t1 - Type_Fixnum;   
  t2 = t2 & 63;		// Strip CDR code 
  if (t2 != 0)   
    goto returnkludgeio;
  arg2 = (u32)arg2;   

DoReturnKludgeIM:
  if (_trace) printf("DoReturnKludgeIM:\n");
  arg6 = *(u64 *)&(processor->stackcachedata);   
  t1 = (arg2 * 8) - 8;   
  t2 = *(s32 *)&processor->control;   
  t1 = iSP - t1;   		// t1 is the values block 
  /* Restore machine state from frame header. */
  t5 = *(s32 *)iFP;   
  t3 = (1792) << 16;   
  t7 = *(s32 *)&processor->continuation;   
  t3 = t2 & t3;		// Mask 
  t4 = *(s32 *)(iFP + 4);   
  t9 = iCP;
  if (t3 != 0)   		// Need to cleanup frame first 
    goto returnkludgecleanup;
  t5 = (u32)t5;   
  t6 = *((s32 *)(&processor->continuation)+1);   
  t7 = (u32)t7;   
  t8 = *(s32 *)(iFP + 8);   		// Get saved control register 
  /* TagType. */
  t4 = t4 & 63;
  /* Restore the PC. */
  iPC = t7 << 1;   		// Assume even PC 
  t3 = t6 & 1;
  t9 = *(u64 *)&(processor->continuationcp);   
  iPC = iPC + t3;

abandon_frame_simple31266:
  if (_trace) printf("abandon_frame_simple31266:\n");
  /* Restore the saved continuation */
  *((u32 *)(&processor->continuation)+1) = t4;   
  t3 = t2 >> 9;   		// Get the caller frame size into place 
  *(u32 *)&processor->continuation = t5;   
  iSP = iFP - 8;   		// Restore the stack pointer. 
  *(u64 *)&processor->continuationcp = zero;   
  t3 = t3 & 255;		// Mask just the caller frame size. 
  t3 = (t3 * 8) + 0;  		// *8 
  t4 = (2048) << 16;   
  t4 = t4 & t2;
  t5 = *(s32 *)&processor->interruptreg;   		// Get the preempt-pending bit 
  t8 = t4 | t8;		// Sticky trace pending bit. 
  t6 = *(u64 *)&(processor->please_stop);   		// Get the trap/suspend bits 
  iFP = iFP - t3;   		// Restore the frame pointer. 
  *(u32 *)&processor->control = t8;   		// Restore the control register 
  t3 = t8 & 255;		// extract the argument size 
  t5 = t5 & 1;
  t5 = t6 | t5;
  *(u64 *)&processor->stop_interpreter = t5;   
  iLP = (t3 * 8) + iFP;  		// Restore the local pointer. 
  arg6 = ((u64)iFP < (u64)arg6) ? 1 : 0;   		// ARG6 = stack-cache underflow 
  if (arg2 == 0) 
    goto rkloopdone;

rklooptop:
  if (_trace) printf("rklooptop:\n");
  t4 = *(u64 *)t1;   		// Read a 40 bit word from the values block 
  arg2 = arg2 - 1;   
  *(u64 *)(iSP + 8) = t4;   		// Push value onto stack cdr codes and all 
  t1 = t1 + 8;
  iSP = iSP + 8;
  if ((s64)arg2 > 0)   
    goto rklooptop;

rkloopdone:
  if (_trace) printf("rkloopdone:\n");
  if (arg6 != 0)   
    goto returnkludgeunderflow;
  if (t9 == 0) 		// No prediction, validate cache 
    goto interpretinstructionforbranch;
  iCP = t9;
  goto INTERPRETINSTRUCTION;   

returnkludgeio:
  if (_trace) printf("returnkludgeio:\n");
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

returnkludgecleanup:
  if (_trace) printf("returnkludgecleanup:\n");
  goto handleframecleanup;

returnkludgeunderflow:
  if (_trace) printf("returnkludgeunderflow:\n");
  goto stackcacheunderflowcheck;

/* end DoReturnKludge */
  /* End of Halfword operand from stack instruction - DoReturnKludge */
/* start DoTakeValues */

  /* Halfword operand from stack instruction - DoTakeValues */
  /* arg2 has the preloaded 8 bit operand. */

dotakevalues:
  if (_trace) printf("dotakevalues:\n");

DoTakeValuesIM:
  if (_trace) printf("DoTakeValuesIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindotakevalues;   

DoTakeValuesSP:
  if (_trace) printf("DoTakeValuesSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoTakeValuesLP:
  if (_trace) printf("DoTakeValuesLP:\n");

DoTakeValuesFP:
  if (_trace) printf("DoTakeValuesFP:\n");

headdotakevalues:
  if (_trace) printf("headdotakevalues:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindotakevalues:
  if (_trace) printf("begindotakevalues:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg6 = *(u64 *)&(processor->niladdress);   
  arg1 = (u32)arg1;   		// Number of values expected 
  arg4 = *(s32 *)iSP;   		// Number of values provided 
  arg3 = *(s32 *)(iSP + 4);   		// Number of values provided 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = arg1 - arg4;   
  if ((s64)arg2 < 0)   		// J. if too many args supplied 
    goto takevalueslose;
  if ((s64)arg2 > 0)   		// J. if too few values supplied 
    goto takevaluespad;
  goto NEXTINSTRUCTION;   

takevalueslose:
  if (_trace) printf("takevalueslose:\n");
  iSP = (arg2 * 8) + iSP;  		// Remove the unwanted values 
  goto NEXTINSTRUCTION;   

takevaluespad:
  if (_trace) printf("takevaluespad:\n");
  t4 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t1 = zero + 128;   
  t2 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t1 = t1 + arg2;		// Account for what we're about to push 
  t1 = (t1 * 8) + iSP;  		// SCA of desired end of cache 
  t2 = (t4 * 8) + t2;  		// SCA of current end of cache 
  t4 = ((s64)t1 <= (s64)t2) ? 1 : 0;   
  if (t4 == 0) 		// We're done if new SCA is within bounds 
    goto stackcacheoverflowhandler;

takevaluespadloop:
  if (_trace) printf("takevaluespadloop:\n");
  *(u64 *)(iSP + 8) = arg6;   		// Push NIL 
  iSP = iSP + 8;
  arg2 = arg2 - 1;   
  if ((s64)arg2 > 0)   
    goto takevaluespadloop;
  goto NEXTINSTRUCTION;   

/* end DoTakeValues */
  /* End of Halfword operand from stack instruction - DoTakeValues */
  /* Catch Instructions */
/* start DoCatchOpen */

  /* Halfword 10 bit immediate instruction - DoCatchOpen */

docatchopen:
  if (_trace) printf("docatchopen:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoCatchOpenIM:
  if (_trace) printf("DoCatchOpenIM:\n");

DoCatchOpenSP:
  if (_trace) printf("DoCatchOpenSP:\n");

DoCatchOpenLP:
  if (_trace) printf("DoCatchOpenLP:\n");

DoCatchOpenFP:
  if (_trace) printf("DoCatchOpenFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  t10 = arg1 & 1;		// t10=1 if unwind-protect, t10=0 if catch 
  t3 = *((s32 *)(&processor->catchblock)+1);   		// tag 
  t10 = t10 << 38;   
  t4 = *(s32 *)&processor->catchblock;   		// data 
  t2 = *(u64 *)&(processor->bindingstackpointer);   
  /* Convert stack cache address to VMA */
  t1 = *(u64 *)&(processor->stackcachedata);   
  t9 = *(u64 *)&(processor->stackcachebasevma);   
  t1 = iSP - t1;   		// stack cache base relative offset 
  t1 = t1 >> 3;   		// convert byte address to word address 
  t9 = t1 + t9;		// reconstruct VMA 
  t1 = t10 | t2;
  *(u64 *)(iSP + 8) = t1;   
  iSP = iSP + 8;
  t11 = *(s32 *)&processor->control;   
  t2 = t11 >> 20;   		// Get old cleanup catch bit 
  t2 = t2 & 64;
  t1 = t11 >> 1;   		// Get old extra arg bit 
  t1 = t1 & 128;
  t1 = t1 | t2;
  /* TagType. */
  t2 = t3 & 63;
  t1 = t1 | t2;		// T1 now has new tag 
  *(u32 *)(iSP + 8) = t4;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  if (t10 != 0)   
    goto catchopen2;
  t2 = *(s32 *)&processor->continuation;   
  t1 = *((s32 *)(&processor->continuation)+1);   
  t2 = (u32)t2;   
  /* TagType. */
  t1 = t1 & 63;
  t3 = arg1 & 192;		// T3 has the disposition bits in place 
  t1 = t1 | t3;
  *(u32 *)(iSP + 8) = t2;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;

catchopen2:
  if (_trace) printf("catchopen2:\n");
  t1 = Type_Locative;
  *((u32 *)(&processor->catchblock)+1) = t1;   		// tag 
  *(u32 *)&processor->catchblock = t9;   		// data 
  t1 = (1024) << 16;   
  t1 = t1 | t11;		// set it 
  *(u32 *)&processor->control = t1;   
  goto NEXTINSTRUCTION;   

/* end DoCatchOpen */
  /* End of Halfword operand from stack instruction - DoCatchOpen */
/* start DoCatchClose */

  /* Halfword operand from stack instruction - DoCatchClose */
  /* arg2 has the preloaded 8 bit operand. */

docatchclose:
  if (_trace) printf("docatchclose:\n");

DoCatchCloseSP:
  if (_trace) printf("DoCatchCloseSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoCatchCloseLP:
  if (_trace) printf("DoCatchCloseLP:\n");

DoCatchCloseFP:
  if (_trace) printf("DoCatchCloseFP:\n");

begindocatchclose:
  if (_trace) printf("begindocatchclose:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)&processor->catchblock;   		// data 
  t1 = (u32)t1;   
  /* Convert VMA to stack cache address */
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t10 = *(u64 *)&(processor->stackcachedata);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t10 = (t3 * 8) + t10;  		// reconstruct SCA 
  arg4 = *(s32 *)(t10 + 8);   		// bstag bsdata 
  arg3 = *(s32 *)(t10 + 12);   
  arg4 = (u32)arg4;   
  t4 = *(u64 *)&(processor->bindingstackpointer);   
  arg6 = *(s32 *)(t10 + 16);   		// prtag prdata 
  arg5 = *(s32 *)(t10 + 20);   
  arg6 = (u32)arg6;   
  t3 = t4 >> 32;   
  t5 = (s32)arg4 - (s32)t4;   
  if (t5 == 0) 
    goto catchcloseld;
  t1 = t3 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto catchclosedbt;

catchcloselt:
  if (_trace) printf("catchcloselt:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  t2 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & t2;
  t4 = t4 & ~t2;		// Turn off the bit 
  if (t3 != 0)   
    goto g31268;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g31268:
  if (_trace) printf("g31268:\n");
  /* Memory Read Internal */

vma_memory_read31269:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  arg1 = t1 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t6 = (arg1 * 4);   
  t7 = LDQ_U(arg1);   
  t8 = t1 - t8;   		// Stack cache offset 
  arg2 = *(u64 *)&(processor->bindread_mask);   
  t9 = ((u64)t8 < (u64)t9) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((arg1&7)*8));   
  if (t9 != 0)   
    goto vma_memory_read31271;

vma_memory_read31270:
  arg1 = zero + 224;   
  arg2 = arg2 >> (t7 & 63);   
  arg1 = arg1 >> (t7 & 63);   
  if (arg2 & 1)   
    goto vma_memory_read31273;

vma_memory_read31278:
  /* Memory Read Internal */

vma_memory_read31279:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  arg1 = t5 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t2 = (arg1 * 4);   
  t3 = LDQ_U(arg1);   
  t8 = t5 - t8;   		// Stack cache offset 
  arg2 = *(u64 *)&(processor->bindread_mask);   
  t9 = ((u64)t8 < (u64)t9) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t3 = (u8)(t3 >> ((arg1&7)*8));   
  if (t9 != 0)   
    goto vma_memory_read31281;

vma_memory_read31280:
  arg1 = zero + 224;   
  arg2 = arg2 >> (t3 & 63);   
  arg1 = arg1 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (arg2 & 1)   
    goto vma_memory_read31283;

vma_memory_read31288:
  /* Memory Read Internal */

vma_memory_read31289:
  arg1 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t11 = t2 + ivory;
  arg2 = *(s32 *)&processor->scovlimit;   
  t9 = (t11 * 4);   
  t8 = LDQ_U(t11);   
  arg1 = t2 - arg1;   		// Stack cache offset 
  arg2 = ((u64)arg1 < (u64)arg2) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  t8 = (u8)(t8 >> ((t11&7)*8));   
  if (arg2 != 0)   
    goto vma_memory_read31291;

vma_memory_read31290:
  arg1 = *(u64 *)&(processor->bindwrite_mask);   
  t11 = zero + 224;   
  arg1 = arg1 >> (t8 & 63);   
  t11 = t11 >> (t8 & 63);   
  if (arg1 & 1)   
    goto vma_memory_read31293;

vma_memory_read31298:
  /* Merge cdr-code */
  t9 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | t9;
  arg1 = t2 + ivory;
  t9 = (arg1 * 4);   
  t11 = LDQ_U(arg1);   
  arg2 = (t8 & 0xff) << ((arg1&7)*8);   
  t11 = t11 & ~(0xffL << (arg1&7)*8);   

force_alignment31301:
  if (_trace) printf("force_alignment31301:\n");
  t11 = t11 | arg2;
  arg2 = *(u64 *)&(processor->stackcachebasevma);   
  STQ_U(arg1, t11);   
  arg1 = *(s32 *)&processor->scovlimit;   
  arg2 = t2 - arg2;   		// Stack cache offset 
  arg1 = ((u64)arg2 < (u64)arg1) ? 1 : 0;   		// In range? 
  *(u32 *)t9 = t6;   
  if (arg1 != 0)   		// J. if in cache 
    goto vma_memory_write31300;

vma_memory_write31299:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
  *(u32 *)&processor->bindingstackpointer = t1;   		// vma only 
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   
  t5 = (s32)arg4 - (s32)t1;   
  if (t5 != 0)   
    goto catchcloselt;
  t3 = *(s32 *)&processor->interruptreg;   
  t4 = t3 & 2;
  t4 = (t4 == 2) ? 1 : 0;   
  t3 = t3 | t4;
  *(u32 *)&processor->interruptreg = t3;   
  if (t3 == 0) 
    goto check_preempt_request31302;
  *(u64 *)&processor->stop_interpreter = t3;   

check_preempt_request31302:
  if (_trace) printf("check_preempt_request31302:\n");

catchcloseld:
  if (_trace) printf("catchcloseld:\n");
  /* TagType. */
  t1 = arg5 & 63;
  *((u32 *)(&processor->catchblock)+1) = t1;   		// tag 
  t2 = arg5 & 128;		// extra argument bit 
  t6 = *(u64 *)&(processor->extraandcatch);   		// mask for two bits 
  t2 = t2 << 1;   		// position in place for control register. 
  *(u32 *)&processor->catchblock = arg6;   		// data 
  t3 = arg5 & 64;		// cleanup catch bit 
  t3 = t3 << 20;   		// position in place for cr 
  t4 = *(s32 *)&processor->control;   
  t5 = t2 | t3;		// coalesce the two bits 
  t4 = t4 & ~t6;		// Turn off extra-arg and cleanup-catch 
  t4 = t4 | t5;		// Maybe turn them back on 
  *(u32 *)&processor->control = t4;   
  t6 = arg3 & 64;		// uwp bit 
  if (t6 == 0) 
    goto NEXTINSTRUCTION;
  /* Handle unwind-protect cleanup here */
  arg2 = *(s32 *)t10;   		// pctag pcdata 
  arg1 = *(s32 *)(t10 + 4);   
  arg2 = (u32)arg2;   
  t8 = t4 >> 17;   		// Cleanup in progress bit into cdr code pos 
  t7 = iPC + 1;		// Next PC 
  /* Convert PC to a real continuation. */
  t8 = t7 & 1;
  t10 = t7 >> 1;   		// convert PC to a real word address. 
  t8 = t8 + Type_EvenPC;   
  /* TagType. */
  t7 = t8 & 63;
  t8 = t8 & 64;
  t9 = (128) << 16;   
  t8 = t8 | 128;
  t7 = t7 | t8;
  *(u32 *)(iSP + 8) = t10;   
  *(u32 *)(iSP + 12) = t7;   		// write the stack cache 
  iSP = iSP + 8;
  t4 = t4 | t9;		// set cr.cleanup-in-progress 
  *(u32 *)&processor->control = t4;   
  /* Convert real continuation to PC. */
  iPC = arg1 & 1;
  iPC = arg2 + iPC;
  iPC = arg2 + iPC;
  goto interpretinstructionforjump;   

catchclosedbt:
  if (_trace) printf("catchclosedbt:\n");
  goto dbunwindcatchtrap;

vma_memory_write31300:
  if (_trace) printf("vma_memory_write31300:\n");
  arg1 = *(u64 *)&(processor->stackcachedata);   
  arg1 = (arg2 * 8) + arg1;  		// reconstruct SCA 
  *(u32 *)arg1 = t6;   		// Store in stack 
  *(u32 *)(arg1 + 4) = t8;   		// write the stack cache 
  goto vma_memory_write31299;   

vma_memory_read31291:
  if (_trace) printf("vma_memory_read31291:\n");
  arg2 = *(u64 *)&(processor->stackcachedata);   
  arg1 = (arg1 * 8) + arg2;  		// reconstruct SCA 
  t9 = *(s32 *)arg1;   
  t8 = *(s32 *)(arg1 + 4);   		// Read from stack cache 
  goto vma_memory_read31290;   

vma_memory_read31293:
  if (_trace) printf("vma_memory_read31293:\n");
  if ((t11 & 1) == 0)   
    goto vma_memory_read31292;
  t2 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read31289;   

vma_memory_read31292:
  if (_trace) printf("vma_memory_read31292:\n");
  arg1 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t11 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + arg1;   		// Adjust for a longword load 
  arg1 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read31295:
  /* Perform memory action */
  arg1 = arg1;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read31281:
  if (_trace) printf("vma_memory_read31281:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read31280;   

vma_memory_read31283:
  if (_trace) printf("vma_memory_read31283:\n");
  if ((arg1 & 1) == 0)   
    goto vma_memory_read31282;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31279;   

vma_memory_read31282:
  if (_trace) printf("vma_memory_read31282:\n");
  arg2 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg1 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  arg1 = (arg1 * 4) + arg2;   		// Adjust for a longword load 
  arg2 = *(s32 *)arg1;   		// Get the memory action 

vma_memory_read31285:
  /* Perform memory action */
  arg1 = arg2;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read31271:
  if (_trace) printf("vma_memory_read31271:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read31270;   

vma_memory_read31273:
  if (_trace) printf("vma_memory_read31273:\n");
  if ((arg1 & 1) == 0)   
    goto vma_memory_read31272;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read31269;   

vma_memory_read31272:
  if (_trace) printf("vma_memory_read31272:\n");
  arg2 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg1 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  arg1 = (arg1 * 4) + arg2;   		// Adjust for a longword load 
  arg2 = *(s32 *)arg1;   		// Get the memory action 

vma_memory_read31275:
  /* Perform memory action */
  arg1 = arg2;
  arg2 = 2;
  goto performmemoryaction;

DoCatchCloseIM:
  goto doistageerror;

/* end DoCatchClose */
  /* End of Halfword operand from stack instruction - DoCatchClose */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunfcal.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunloop.as
 ************************************************************************/

  /* Branch and loop instructions. */
/* start DoBranchTrueElseNoPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueElseNoPop */

dobranchtrueelsenopop:
  if (_trace) printf("dobranchtrueelsenopop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueElseNoPopIM:
  if (_trace) printf("DoBranchTrueElseNoPopIM:\n");

DoBranchTrueElseNoPopSP:
  if (_trace) printf("DoBranchTrueElseNoPopSP:\n");

DoBranchTrueElseNoPopLP:
  if (_trace) printf("DoBranchTrueElseNoPopLP:\n");

DoBranchTrueElseNoPopFP:
  if (_trace) printf("DoBranchTrueElseNoPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto NEXTINSTRUCTION;
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueElseNoPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueElseNoPop */
/* start DoBranchTrueElseExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueElseExtraPop */

dobranchtrueelseextrapop:
  if (_trace) printf("dobranchtrueelseextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueElseExtraPopIM:
  if (_trace) printf("DoBranchTrueElseExtraPopIM:\n");

DoBranchTrueElseExtraPopSP:
  if (_trace) printf("DoBranchTrueElseExtraPopSP:\n");

DoBranchTrueElseExtraPopLP:
  if (_trace) printf("DoBranchTrueElseExtraPopLP:\n");

DoBranchTrueElseExtraPopFP:
  if (_trace) printf("DoBranchTrueElseExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto dobrelsepopextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 16;   
  goto cachevalid;   

dobrelsepopextrapop:
  if (_trace) printf("dobrelsepopextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueElseExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueElseExtraPop */
/* start DoBranchFalseElseExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseElseExtraPop */

dobranchfalseelseextrapop:
  if (_trace) printf("dobranchfalseelseextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseElseExtraPopIM:
  if (_trace) printf("DoBranchFalseElseExtraPopIM:\n");

DoBranchFalseElseExtraPopSP:
  if (_trace) printf("DoBranchFalseElseExtraPopSP:\n");

DoBranchFalseElseExtraPopLP:
  if (_trace) printf("DoBranchFalseElseExtraPopLP:\n");

DoBranchFalseElseExtraPopFP:
  if (_trace) printf("DoBranchFalseElseExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto dobrnelsepopextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 16;   
  goto cachevalid;   

dobrnelsepopextrapop:
  if (_trace) printf("dobrnelsepopextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseElseExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseElseExtraPop */
/* start DoBranchFalseExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseExtraPop */

dobranchfalseextrapop:
  if (_trace) printf("dobranchfalseextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseExtraPopIM:
  if (_trace) printf("DoBranchFalseExtraPopIM:\n");

DoBranchFalseExtraPopSP:
  if (_trace) printf("DoBranchFalseExtraPopSP:\n");

DoBranchFalseExtraPopLP:
  if (_trace) printf("DoBranchFalseExtraPopLP:\n");

DoBranchFalseExtraPopFP:
  if (_trace) printf("DoBranchFalseExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto dobrnpopelsepopextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 16;   
  goto cachevalid;   

dobrnpopelsepopextrapop:
  if (_trace) printf("dobrnpopelsepopextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 16;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseExtraPop */
/* start DoLoopDecrementTos */

  /* Halfword 10 bit immediate instruction - DoLoopDecrementTos */

doloopdecrementtos:
  if (_trace) printf("doloopdecrementtos:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoLoopDecrementTosIM:
  if (_trace) printf("DoLoopDecrementTosIM:\n");

DoLoopDecrementTosSP:
  if (_trace) printf("DoLoopDecrementTosSP:\n");

DoLoopDecrementTosLP:
  if (_trace) printf("DoLoopDecrementTosLP:\n");

DoLoopDecrementTosFP:
  if (_trace) printf("DoLoopDecrementTosFP:\n");
  arg1 = (s64)arg3 >> 48;   
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  t2 = (u32)arg6;   
  t3 = t1 - Type_Fixnum;   
  t3 = t3 & 63;		// Strip CDR code 
  if (t3 != 0)   
    goto iloop_decrement_tos31303;
  t3 = (s32)t2 - (s32)1;   
  t4 = ((s64)t3 < (s64)t2) ? 1 : 0;   
  if (t4 == 0) 
    goto iloop_decrement_tos31305;
  t6 = Type_Fixnum;
  *(u32 *)iSP = t3;   
  *(u32 *)(iSP + 4) = t6;   		// write the stack cache 
  if ((s64)t3 <= 0)  
    goto NEXTINSTRUCTION;
  /* Here if branch taken. */
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

iloop_decrement_tos31303:
  if (_trace) printf("iloop_decrement_tos31303:\n");
  t3 = t1 - Type_Fixnum;   
  t3 = t3 & 56;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto iloop_decrement_tos31304;

iloop_decrement_tos31305:
  if (_trace) printf("iloop_decrement_tos31305:\n");
  arg5 = iPC + arg1;		// Compute next-pc 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto loopexception;

iloop_decrement_tos31304:
  if (_trace) printf("iloop_decrement_tos31304:\n");
  arg5 = 0;
  arg2 = 81;
  goto illegaloperand;

/* end DoLoopDecrementTos */
  /* End of Halfword operand from stack instruction - DoLoopDecrementTos */
/* start DoLoopIncrementTosLessThan */

  /* Halfword 10 bit immediate instruction - DoLoopIncrementTosLessThan */

doloopincrementtoslessthan:
  if (_trace) printf("doloopincrementtoslessthan:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoLoopIncrementTosLessThanIM:
  if (_trace) printf("DoLoopIncrementTosLessThanIM:\n");

DoLoopIncrementTosLessThanSP:
  if (_trace) printf("DoLoopIncrementTosLessThanSP:\n");

DoLoopIncrementTosLessThanLP:
  if (_trace) printf("DoLoopIncrementTosLessThanLP:\n");

DoLoopIncrementTosLessThanFP:
  if (_trace) printf("DoLoopIncrementTosLessThanFP:\n");
  arg1 = (s64)arg3 >> 48;   
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  t2 = (u32)arg6;   
  t5 = t1 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than31306;
  t4 = *(s32 *)(iSP + -8);   		// Get arg1. 
  t3 = *(s32 *)(iSP + -4);   
  t4 = (u32)t4;   
  t5 = t3 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than31307;
  t5 = (s32)t2 + (s32)1;
  t6 = ((s64)t2 <= (s64)t5) ? 1 : 0;   
  if (t6 == 0) 
    goto iloop_increment_tos_less_than31308;
  t6 = Type_Fixnum;
  *(u32 *)iSP = t5;   
  *(u32 *)(iSP + 4) = t6;   		// write the stack cache 
  t6 = ((s64)t5 <= (s64)t4) ? 1 : 0;   
  if (t6 == 0) 
    goto NEXTINSTRUCTION;
  /* Here if branch taken. */

force_alignment31310:
  if (_trace) printf("force_alignment31310:\n");
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

iloop_increment_tos_less_than31306:
  if (_trace) printf("iloop_increment_tos_less_than31306:\n");
  t5 = t1 - Type_Fixnum;   
  t5 = t5 & 56;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than31309;

iloop_increment_tos_less_than31307:
  if (_trace) printf("iloop_increment_tos_less_than31307:\n");
  t5 = t3 - Type_Fixnum;   
  t5 = t5 & 56;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than31309;

iloop_increment_tos_less_than31308:
  if (_trace) printf("iloop_increment_tos_less_than31308:\n");
  arg5 = iPC + arg1;		// Compute next-pc 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto loopexception;

iloop_increment_tos_less_than31309:
  if (_trace) printf("iloop_increment_tos_less_than31309:\n");
  arg5 = 0;
  arg2 = 16;
  goto illegaloperand;

/* end DoLoopIncrementTosLessThan */
  /* End of Halfword operand from stack instruction - DoLoopIncrementTosLessThan */
/* start DoBranchTrueExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueExtraPop */

dobranchtrueextrapop:
  if (_trace) printf("dobranchtrueextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueExtraPopIM:
  if (_trace) printf("DoBranchTrueExtraPopIM:\n");

DoBranchTrueExtraPopSP:
  if (_trace) printf("DoBranchTrueExtraPopSP:\n");

DoBranchTrueExtraPopLP:
  if (_trace) printf("DoBranchTrueExtraPopLP:\n");

DoBranchTrueExtraPopFP:
  if (_trace) printf("DoBranchTrueExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto dobrpopelsepopextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 16;   
  goto cachevalid;   

dobrpopelsepopextrapop:
  if (_trace) printf("dobrpopelsepopextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 16;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueExtraPop */
/* start DoBranchTrueAndNoPopElseNoPopExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchTrueAndNoPopElseNoPopExtraPop */

dobranchtrueandnopopelsenopopextrapop:
  if (_trace) printf("dobranchtrueandnopopelsenopopextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueAndNoPopElseNoPopExtraPopIM:
  if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopIM:\n");

DoBranchTrueAndNoPopElseNoPopExtraPopSP:
  if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopSP:\n");

DoBranchTrueAndNoPopElseNoPopExtraPopLP:
  if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopLP:\n");

DoBranchTrueAndNoPopElseNoPopExtraPopFP:
  if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 != 0)   
    goto dobrextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrextrapop:
  if (_trace) printf("dobrextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchTrueAndNoPopElseNoPopExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchTrueAndNoPopElseNoPopExtraPop */
/* start DoBranchFalseAndNoPopElseNoPopExtraPop */

  /* Halfword 10 bit immediate instruction - DoBranchFalseAndNoPopElseNoPopExtraPop */

dobranchfalseandnopopelsenopopextrapop:
  if (_trace) printf("dobranchfalseandnopopelsenopopextrapop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseAndNoPopElseNoPopExtraPopIM:
  if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopIM:\n");

DoBranchFalseAndNoPopElseNoPopExtraPopSP:
  if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopSP:\n");

DoBranchFalseAndNoPopElseNoPopExtraPopLP:
  if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopLP:\n");

DoBranchFalseAndNoPopElseNoPopExtraPopFP:
  if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopFP:\n");
  /* arg1 has signed operand preloaded. */
  t1 = (u32)(arg6 >> ((4&7)*8));   		// Check tag of word in TOS. 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  arg1 = (s64)arg3 >> 48;   		// Get signed 10-bit immediate arg 
  /* TagType. */
  t1 = t1 & 63;		// strip the cdr code off. 
  t1 = t1 - Type_NIL;   		// Compare to NIL 
  if (t1 == 0) 
    goto dobrnextrapop;
  /* Here if branch not taken.  Pop the argument. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  iSP = iSP - 8;   
  goto cachevalid;   

dobrnextrapop:
  if (_trace) printf("dobrnextrapop:\n");
  if (arg1 == 0) 		// Can't branch to ourself 
    goto branchexception;
  iSP = iSP - 8;   
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end DoBranchFalseAndNoPopElseNoPopExtraPop */
  /* End of Halfword operand from stack instruction - DoBranchFalseAndNoPopElseNoPopExtraPop */
/* start BranchException */


branchexception:
  if (_trace) printf("branchexception:\n");
  arg5 = 0;
  arg2 = 24;
  goto illegaloperand;

/* end BranchException */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunloop.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunlist.as
 ************************************************************************/

  /* List Operations. */
/* start DoSetToCar */

  /* Halfword operand from stack instruction - DoSetToCar */
  /* arg2 has the preloaded 8 bit operand. */

dosettocar:
  if (_trace) printf("dosettocar:\n");

DoSetToCarSP:
  if (_trace) printf("DoSetToCarSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetToCarLP:
  if (_trace) printf("DoSetToCarLP:\n");

DoSetToCarFP:
  if (_trace) printf("DoSetToCarFP:\n");

begindosettocar:
  if (_trace) printf("begindosettocar:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg5 = *(s32 *)(arg1 + 4);   		// Get the operand from the stack. 
  arg6 = *(s32 *)arg1;   
  t2 = arg5 & 192;		// Save the old CDR code 
  r0 = (u64)&&return0040;
  goto carinternal;
return0040:
  /* TagType. */
  arg5 = arg5 & 63;
  arg5 = arg5 | t2;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;   
  *(u32 *)(arg1 + 4) = arg5;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

DoSetToCarIM:
  goto doistageerror;

/* end DoSetToCar */
  /* End of Halfword operand from stack instruction - DoSetToCar */
/* start DoSetToCdr */

  /* Halfword operand from stack instruction - DoSetToCdr */
  /* arg2 has the preloaded 8 bit operand. */

dosettocdr:
  if (_trace) printf("dosettocdr:\n");

DoSetToCdrSP:
  if (_trace) printf("DoSetToCdrSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetToCdrLP:
  if (_trace) printf("DoSetToCdrLP:\n");

DoSetToCdrFP:
  if (_trace) printf("DoSetToCdrFP:\n");

begindosettocdr:
  if (_trace) printf("begindosettocdr:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg5 = *(s32 *)(arg1 + 4);   		// Get the operand from the stack. 
  arg6 = *(s32 *)arg1;   
  t2 = arg5 & 192;		// Save the old CDR code 
  r0 = (u64)&&return0041;
  goto cdrinternal;
return0041:
  /* TagType. */
  arg5 = arg5 & 63;
  arg5 = arg5 | t2;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;   
  *(u32 *)(arg1 + 4) = arg5;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

DoSetToCdrIM:
  goto doistageerror;

/* end DoSetToCdr */
  /* End of Halfword operand from stack instruction - DoSetToCdr */
/* start SetToCdrPushCarLocative */


SetToCdrPushCarLocative:
  if (_trace) printf("SetToCdrPushCarLocative:\n");

settocdrpushcarlocative:
  if (_trace) printf("settocdrpushcarlocative:\n");
  arg2 = t2;
  /* Memory Read Internal */

vma_memory_read31311:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31313;

vma_memory_read31312:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31315;

vma_memory_read31322:
  /* TagType. */
  t1 = t1 & 63;
  *(u32 *)(iSP + 8) = arg6;   
  *(u32 *)(iSP + 12) = arg5;   		// write the stack cache 
  iSP = iSP + 8;
  t1 = t1 | t3;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;   
  *(u32 *)(arg1 + 4) = arg5;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read31315:
  if (_trace) printf("vma_memory_read31315:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31314;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31311;   

vma_memory_read31314:
  if (_trace) printf("vma_memory_read31314:\n");

vma_memory_read31313:
  if (_trace) printf("vma_memory_read31313:\n");
  r0 = (u64)&&return0042;
  goto memoryreaddatadecode;
return0042:
  goto vma_memory_read31322;   

/* end SetToCdrPushCarLocative */
/* start DoAssoc */

  /* Halfword operand from stack instruction - DoAssoc */
  /* arg2 has the preloaded 8 bit operand. */

doassoc:
  if (_trace) printf("doassoc:\n");

DoAssocSP:
  if (_trace) printf("DoAssocSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoassoc;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoAssocLP:
  if (_trace) printf("DoAssocLP:\n");

DoAssocFP:
  if (_trace) printf("DoAssocFP:\n");

begindoassoc:
  if (_trace) printf("begindoassoc:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t5 = zero + -2048;   
  t5 = t5 + ((1) << 16);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (u32)arg6;   
  t1 = *(s32 *)(arg1 + 4);   
  t2 = *(s32 *)arg1;   
  /* TagType. */
  arg3 = arg3 & 63;		// Get the object type bits 
  t5 = t5 >> (arg3 & 63);   		// Low bit will set iff EQ-NOT-EQL 
  /* TagType. */
  t1 = t1 & 63;		// Strip cdr code 
  t2 = (u32)t2;   		// Remove sign-extension 
  if (t5 & 1)   
    goto assocexc;
  t6 = zero;
  goto carcdrloop31324;   

assoccdr:
  if (_trace) printf("assoccdr:\n");
  t6 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop or trap? 
  /* Move cdr to car for next carcdr-internal */
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6;

carcdrloop31324:
  if (_trace) printf("carcdrloop31324:\n");
  t5 = t1 - Type_NIL;   
  if (t6 != 0)   		// Asked to stop, check for sequence break 
    goto carcdrloop31323;
  if (t5 == 0) 
    goto carcdrloop31325;
  r0 = (u64)&&return0043;
  goto carcdrinternal;
return0043:
  t7 = t1 & 63;		// Strip off any CDR code bits. 
  t8 = (t7 == Type_List) ? 1 : 0;   

force_alignment31343:
  if (_trace) printf("force_alignment31343:\n");
  if (t8 == 0) 
    goto basic_dispatch31327;
  /* Here if argument TypeList */
  arg2 = t2;
  t3 = arg5;
  arg1 = arg6;
  /* Memory Read Internal */

vma_memory_read31328:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31330;

vma_memory_read31329:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31332;

vma_memory_read31339:
  /* TagType. */
  t5 = arg5 & 63;
  arg5 = t3;
  t6 = (s32)arg4 - (s32)arg6;   		// t6=0 if data same 
  arg6 = arg1;
  if (t6 != 0)   		// J. if different 
    goto assoccdr;
  t5 = arg3 - t5;   		// t5 zero if same tag 
  if (t5 != 0)   		// J. if tags different 
    goto assoccdr;
  /* we found a match! */
  /* TagType. */
  t1 = t1 & 63;
  *(u32 *)iSP = t2;   
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

basic_dispatch31327:
  if (_trace) printf("basic_dispatch31327:\n");
  t8 = (t7 == Type_NIL) ? 1 : 0;   

force_alignment31344:
  if (_trace) printf("force_alignment31344:\n");
  if (t8 == 0) 
    goto basic_dispatch31340;
  /* Here if argument TypeNIL */
  goto assoccdr;   

basic_dispatch31340:
  if (_trace) printf("basic_dispatch31340:\n");
  /* Here for all other cases */
  /* SetTag. */
  t1 = arg4 << 32;   
  t1 = arg5 | t1;
  arg5 = t1;
  arg2 = 14;
  goto illegaloperand;

basic_dispatch31326:
  if (_trace) printf("basic_dispatch31326:\n");

carcdrloop31325:
  if (_trace) printf("carcdrloop31325:\n");
  t1 = *(u64 *)&(processor->niladdress);   		// Return NIL 
  *(u64 *)iSP = t1;   		// push the data 
  goto NEXTINSTRUCTION;   

assocexc:
  if (_trace) printf("assocexc:\n");
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

vma_memory_read31332:
  if (_trace) printf("vma_memory_read31332:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31331;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31328;   

vma_memory_read31331:
  if (_trace) printf("vma_memory_read31331:\n");

vma_memory_read31330:
  if (_trace) printf("vma_memory_read31330:\n");
  r0 = (u64)&&return0044;
  goto memoryreaddatadecode;
return0044:
  goto vma_memory_read31339;   

carcdrloop31323:
  if (_trace) printf("carcdrloop31323:\n");
  iSP = *(u64 *)&(processor->restartsp);   
  goto INTERPRETINSTRUCTION;   

DoAssocIM:
  goto doistageerror;

/* end DoAssoc */
  /* End of Halfword operand from stack instruction - DoAssoc */
/* start DoMember */

  /* Halfword operand from stack instruction - DoMember */
  /* arg2 has the preloaded 8 bit operand. */

domember:
  if (_trace) printf("domember:\n");

DoMemberSP:
  if (_trace) printf("DoMemberSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindomember;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoMemberLP:
  if (_trace) printf("DoMemberLP:\n");

DoMemberFP:
  if (_trace) printf("DoMemberFP:\n");

begindomember:
  if (_trace) printf("begindomember:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t5 = zero + -2048;   
  t5 = t5 + ((1) << 16);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (u32)arg6;   
  t1 = *(s32 *)(arg1 + 4);   
  t2 = *(s32 *)arg1;   
  /* TagType. */
  arg3 = arg3 & 63;		// Get the object type bits 
  t5 = t5 >> (arg3 & 63);   		// Low bit will set iff EQ-NOT-EQL 
  /* TagType. */
  t1 = t1 & 63;		// Strip cdr code 
  t2 = (u32)t2;   		// Remove sign-extension 
  if (t5 & 1)   
    goto memberexc;
  t6 = zero;
  goto carcdrloop31346;   

membercdr:
  if (_trace) printf("membercdr:\n");
  t6 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop or trap? 
  /* Move cdr to car for next carcdr-internal */
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6;

carcdrloop31346:
  if (_trace) printf("carcdrloop31346:\n");
  /* TagType. */
  t3 = t1 & 63;
  arg1 = t2;
  t5 = t1 - Type_NIL;   
  if (t6 != 0)   		// Asked to stop, check for sequence break 
    goto carcdrloop31345;
  if (t5 == 0) 
    goto carcdrloop31347;
  r0 = (u64)&&return0045;
  goto carcdrinternal;
return0045:
  /* TagType. */
  t5 = t1 & 63;
  t7 = arg4 - t2;   		// t7=0 if data same 
  if (t7 != 0)   		// J. if different 
    goto membercdr;
  t6 = arg3 - t5;   		// t6 zero if same tag 
  if (t6 != 0)   		// J. if tags different 
    goto membercdr;
  /* we found a match! */
  *(u32 *)iSP = arg1;   
  *(u32 *)(iSP + 4) = t3;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

carcdrloop31347:
  if (_trace) printf("carcdrloop31347:\n");
  t1 = *(u64 *)&(processor->niladdress);   		// Return NIL 
  *(u64 *)iSP = t1;   		// push the data 
  goto NEXTINSTRUCTION;   

memberexc:
  if (_trace) printf("memberexc:\n");
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

carcdrloop31345:
  if (_trace) printf("carcdrloop31345:\n");
  iSP = *(u64 *)&(processor->restartsp);   
  goto INTERPRETINSTRUCTION;   

DoMemberIM:
  goto doistageerror;

/* end DoMember */
  /* End of Halfword operand from stack instruction - DoMember */
/* start DoRgetf */

  /* Halfword operand from stack instruction - DoRgetf */
  /* arg2 has the preloaded 8 bit operand. */

dorgetf:
  if (_trace) printf("dorgetf:\n");

DoRgetfSP:
  if (_trace) printf("DoRgetfSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindorgetf;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoRgetfLP:
  if (_trace) printf("DoRgetfLP:\n");

DoRgetfFP:
  if (_trace) printf("DoRgetfFP:\n");

begindorgetf:
  if (_trace) printf("begindorgetf:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t5 = zero + -2048;   
  t5 = t5 + ((1) << 16);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (u32)arg6;   
  t1 = *(s32 *)(arg1 + 4);   
  t2 = *(s32 *)arg1;   
  /* TagType. */
  arg3 = arg3 & 63;		// Get the object type bits 
  t5 = t5 >> (arg3 & 63);   		// Low bit will set iff EQ-NOT-EQL 
  /* TagType. */
  t1 = t1 & 63;		// Strip cdr code 
  t2 = (u32)t2;   		// Remove sign-extension 
  if (t5 & 1)   
    goto rgetfexc;
  t6 = zero;
  goto carcdrloop31349;   

rgetfcdr:
  if (_trace) printf("rgetfcdr:\n");
  r0 = (u64)&&return0046;
  goto cdrinternal;
return0046:
  t6 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop or trap? 
  /* Move cdr to car for next carcdr-internal */
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6;

carcdrloop31349:
  if (_trace) printf("carcdrloop31349:\n");
  t5 = t1 - Type_NIL;   
  if (t6 != 0)   		// Asked to stop, check for sequence break 
    goto carcdrloop31348;
  if (t5 == 0) 
    goto carcdrloop31350;
  r0 = (u64)&&return0047;
  goto carcdrinternal;
return0047:
  /* TagType. */
  t5 = t1 & 63;
  t7 = arg4 - t2;   		// t7=0 if data same 
  if (t7 != 0)   		// J. if different 
    goto rgetfcdr;
  t6 = arg3 - t5;   		// t6 zero if same tag 
  if (t6 != 0)   		// J. if tags different 
    goto rgetfcdr;
  /* we found a match! */
  /* TagType. */
  t1 = arg5 & 63;		// Strip CDR code 
  t5 = t1 - Type_NIL;   		// t5=0 if end of list 
  if (t5 == 0) 		// after all this effort we lose! 
    goto rgetfexc;
  t2 = arg6;
  r0 = (u64)&&return0048;
  goto carinternal;
return0048:
  /* TagType. */
  arg5 = arg5 & 63;		// Strip the CDR code 
  *(u32 *)iSP = arg6;   
  *(u32 *)(iSP + 4) = arg5;   		// write the stack cache 
  arg2 = t1 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t2;   		// Push the second result 
  *(u32 *)(iSP + 12) = arg2;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

carcdrloop31350:
  if (_trace) printf("carcdrloop31350:\n");
  arg2 = *(u64 *)&(processor->niladdress);   		// Return NIL 
  *(u64 *)iSP = arg2;   
  *(u64 *)(iSP + 8) = arg2;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

rgetfexc:
  if (_trace) printf("rgetfexc:\n");
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

carcdrloop31348:
  if (_trace) printf("carcdrloop31348:\n");
  iSP = *(u64 *)&(processor->restartsp);   
  goto INTERPRETINSTRUCTION;   

DoRgetfIM:
  goto doistageerror;

/* end DoRgetf */
  /* End of Halfword operand from stack instruction - DoRgetf */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunlist.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifuninst.as
 ************************************************************************/

  /* Instance variable accessors.. */
/* start DoPopInstanceVariable */

  /* Halfword 10 bit immediate instruction - DoPopInstanceVariable */

dopopinstancevariable:
  if (_trace) printf("dopopinstancevariable:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPopInstanceVariableIM:
  if (_trace) printf("DoPopInstanceVariableIM:\n");

DoPopInstanceVariableSP:
  if (_trace) printf("DoPopInstanceVariableSP:\n");

DoPopInstanceVariableLP:
  if (_trace) printf("DoPopInstanceVariableLP:\n");

DoPopInstanceVariableFP:
  if (_trace) printf("DoPopInstanceVariableFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Mapped */
  arg1 = *(s32 *)(iFP + 16);   		// Map 
  t1 = *(s32 *)(iFP + 20);   
  arg1 = (u32)arg1;   
  t4 = t1 - Type_Array;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto ivbadmap;
  /* Memory Read Internal */

vma_memory_read31354:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31356;

vma_memory_read31355:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  if (t10 & 1)   
    goto vma_memory_read31358;

vma_memory_read31363:
  t2 = t2 & Array_LengthMask;
  t5 = t2 - arg2;   
  if ((s64)t5 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg1 = arg1 + arg2;
  arg1 = arg1 + 1;
  /* Memory Read Internal */

vma_memory_read31364:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31366;

vma_memory_read31365:
  t9 = zero + 240;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read31368;

vma_memory_read31375:
  t3 = t2;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto popiviex;
  arg1 = *(s32 *)(iFP + 24);   		// Self 
  t6 = *(s32 *)(iFP + 28);   
  arg1 = (u32)arg1;   
  t5 = t6 - Type_Instance;   
  t5 = t5 & 60;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto ivbadinst;
  t5 = t6 & 192;		// Unshifted cdr code 
  t5 = t5 - 64;   		// Check for CDR code 1 
  if (t5 != 0)   		// J. if CDR code is not 1 
    goto locate_instance0variable_mapped31353;

locate_instance0variable_mapped31352:
  if (_trace) printf("locate_instance0variable_mapped31352:\n");
  arg1 = arg1 + t3;

locate_instance0variable_mapped31351:
  if (_trace) printf("locate_instance0variable_mapped31351:\n");
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read31376:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read31378;

vma_memory_read31377:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read31380;

vma_memory_read31386:
  /* Merge cdr-code */
  t5 = t2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t6 = arg1 + ivory;
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment31388:
  if (_trace) printf("force_alignment31388:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write31387;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

popiviex:
  if (_trace) printf("popiviex:\n");
  t1 = zero + 8;   
  /* SetTag. */
  t1 = t1 << 32;   
  t1 = arg2 | t1;
  arg6 = t2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

vma_memory_write31387:
  if (_trace) printf("vma_memory_write31387:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read31378:
  if (_trace) printf("vma_memory_read31378:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read31377;   

vma_memory_read31380:
  if (_trace) printf("vma_memory_read31380:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read31379;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read31376;   

vma_memory_read31379:
  if (_trace) printf("vma_memory_read31379:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read31383:

vma_memory_read31382:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read31366:
  if (_trace) printf("vma_memory_read31366:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31365;   

vma_memory_read31368:
  if (_trace) printf("vma_memory_read31368:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31367;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31364;   

vma_memory_read31367:
  if (_trace) printf("vma_memory_read31367:\n");
  t10 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31372:
  if (_trace) printf("vma_memory_read31372:\n");
  t9 = t10 & MemoryActionTransform;
  if (t9 == 0) 
    goto vma_memory_read31371;
  t1 = t1 & ~63L;
  t1 = t1 | Type_ExternalValueCellPointer;
  goto vma_memory_read31375;   

vma_memory_read31371:

vma_memory_read31370:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31356:
  if (_trace) printf("vma_memory_read31356:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31355;   

vma_memory_read31358:
  if (_trace) printf("vma_memory_read31358:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31357;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31354;   

vma_memory_read31357:
  if (_trace) printf("vma_memory_read31357:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31360:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

locate_instance0variable_mapped31353:
  if (_trace) printf("locate_instance0variable_mapped31353:\n");
  t5 = arg1;
  /* Memory Read Internal */

vma_memory_read31389:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31391;

vma_memory_read31390:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read31393;

vma_memory_read31398:
  t5 = t5 - arg1;   
  if (t5 != 0)   
    goto locate_instance0variable_mapped31352;
  /* TagType. */
  t6 = t6 & 63;
  t6 = t6 | 64;		// Set CDR code to 1 
  *(u32 *)(iFP + 24) = arg1;   		// Update self 
  *(u32 *)(iFP + 28) = t6;   		// write the stack cache 
  goto locate_instance0variable_mapped31352;   

vma_memory_read31391:
  if (_trace) printf("vma_memory_read31391:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31390;   

vma_memory_read31393:
  if (_trace) printf("vma_memory_read31393:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31392;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31389;   

vma_memory_read31392:
  if (_trace) printf("vma_memory_read31392:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31395:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

/* end DoPopInstanceVariable */
  /* End of Halfword operand from stack instruction - DoPopInstanceVariable */
/* start DoMovemInstanceVariable */

  /* Halfword 10 bit immediate instruction - DoMovemInstanceVariable */

domoveminstancevariable:
  if (_trace) printf("domoveminstancevariable:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoMovemInstanceVariableIM:
  if (_trace) printf("DoMovemInstanceVariableIM:\n");

DoMovemInstanceVariableSP:
  if (_trace) printf("DoMovemInstanceVariableSP:\n");

DoMovemInstanceVariableLP:
  if (_trace) printf("DoMovemInstanceVariableLP:\n");

DoMovemInstanceVariableFP:
  if (_trace) printf("DoMovemInstanceVariableFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Mapped */
  arg1 = *(s32 *)(iFP + 16);   		// Map 
  t1 = *(s32 *)(iFP + 20);   
  arg1 = (u32)arg1;   
  t4 = t1 - Type_Array;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto ivbadmap;
  /* Memory Read Internal */

vma_memory_read31402:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31404;

vma_memory_read31403:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  if (t10 & 1)   
    goto vma_memory_read31406;

vma_memory_read31411:
  t2 = t2 & Array_LengthMask;
  t5 = t2 - arg2;   
  if ((s64)t5 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg1 = arg1 + arg2;
  arg1 = arg1 + 1;
  /* Memory Read Internal */

vma_memory_read31412:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31414;

vma_memory_read31413:
  t9 = zero + 240;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read31416;

vma_memory_read31423:
  t3 = t2;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto movemiviex;
  arg1 = *(s32 *)(iFP + 24);   		// Self 
  t6 = *(s32 *)(iFP + 28);   
  arg1 = (u32)arg1;   
  t5 = t6 - Type_Instance;   
  t5 = t5 & 60;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto ivbadinst;
  t5 = t6 & 192;		// Unshifted cdr code 
  t5 = t5 - 64;   		// Check for CDR code 1 
  if (t5 != 0)   		// J. if CDR code is not 1 
    goto locate_instance0variable_mapped31401;

locate_instance0variable_mapped31400:
  if (_trace) printf("locate_instance0variable_mapped31400:\n");
  arg1 = arg1 + t3;

locate_instance0variable_mapped31399:
  if (_trace) printf("locate_instance0variable_mapped31399:\n");
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  t1 = (u32)t1;   
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read31424:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read31426;

vma_memory_read31425:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read31428;

vma_memory_read31434:
  /* Merge cdr-code */
  t5 = t2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t6 = arg1 + ivory;
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment31436:
  if (_trace) printf("force_alignment31436:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write31435;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

movemiviex:
  if (_trace) printf("movemiviex:\n");
  t1 = zero + 8;   
  /* SetTag. */
  t1 = t1 << 32;   
  t1 = arg2 | t1;
  arg6 = t2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

vma_memory_write31435:
  if (_trace) printf("vma_memory_write31435:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read31426:
  if (_trace) printf("vma_memory_read31426:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read31425;   

vma_memory_read31428:
  if (_trace) printf("vma_memory_read31428:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read31427;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read31424;   

vma_memory_read31427:
  if (_trace) printf("vma_memory_read31427:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read31431:

vma_memory_read31430:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read31414:
  if (_trace) printf("vma_memory_read31414:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31413;   

vma_memory_read31416:
  if (_trace) printf("vma_memory_read31416:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31415;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31412;   

vma_memory_read31415:
  if (_trace) printf("vma_memory_read31415:\n");
  t10 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31420:
  if (_trace) printf("vma_memory_read31420:\n");
  t9 = t10 & MemoryActionTransform;
  if (t9 == 0) 
    goto vma_memory_read31419;
  t1 = t1 & ~63L;
  t1 = t1 | Type_ExternalValueCellPointer;
  goto vma_memory_read31423;   

vma_memory_read31419:

vma_memory_read31418:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31404:
  if (_trace) printf("vma_memory_read31404:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31403;   

vma_memory_read31406:
  if (_trace) printf("vma_memory_read31406:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31405;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31402;   

vma_memory_read31405:
  if (_trace) printf("vma_memory_read31405:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31408:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

locate_instance0variable_mapped31401:
  if (_trace) printf("locate_instance0variable_mapped31401:\n");
  t5 = arg1;
  /* Memory Read Internal */

vma_memory_read31437:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31439;

vma_memory_read31438:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read31441;

vma_memory_read31446:
  t5 = t5 - arg1;   
  if (t5 != 0)   
    goto locate_instance0variable_mapped31400;
  /* TagType. */
  t6 = t6 & 63;
  t6 = t6 | 64;		// Set CDR code to 1 
  *(u32 *)(iFP + 24) = arg1;   		// Update self 
  *(u32 *)(iFP + 28) = t6;   		// write the stack cache 
  goto locate_instance0variable_mapped31400;   

vma_memory_read31439:
  if (_trace) printf("vma_memory_read31439:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31438;   

vma_memory_read31441:
  if (_trace) printf("vma_memory_read31441:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31440;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31437;   

vma_memory_read31440:
  if (_trace) printf("vma_memory_read31440:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31443:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

/* end DoMovemInstanceVariable */
  /* End of Halfword operand from stack instruction - DoMovemInstanceVariable */
/* start DoPushAddressInstanceVariable */

  /* Halfword 10 bit immediate instruction - DoPushAddressInstanceVariable */

dopushaddressinstancevariable:
  if (_trace) printf("dopushaddressinstancevariable:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPushAddressInstanceVariableIM:
  if (_trace) printf("DoPushAddressInstanceVariableIM:\n");

DoPushAddressInstanceVariableSP:
  if (_trace) printf("DoPushAddressInstanceVariableSP:\n");

DoPushAddressInstanceVariableLP:
  if (_trace) printf("DoPushAddressInstanceVariableLP:\n");

DoPushAddressInstanceVariableFP:
  if (_trace) printf("DoPushAddressInstanceVariableFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Mapped */
  arg1 = *(s32 *)(iFP + 16);   		// Map 
  t1 = *(s32 *)(iFP + 20);   
  arg1 = (u32)arg1;   
  t4 = t1 - Type_Array;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto ivbadmap;
  /* Memory Read Internal */

vma_memory_read31450:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31452;

vma_memory_read31451:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  if (t10 & 1)   
    goto vma_memory_read31454;

vma_memory_read31459:
  t2 = t2 & Array_LengthMask;
  t5 = t2 - arg2;   
  if ((s64)t5 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg1 = arg1 + arg2;
  arg1 = arg1 + 1;
  /* Memory Read Internal */

vma_memory_read31460:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31462;

vma_memory_read31461:
  t9 = zero + 240;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read31464;

vma_memory_read31471:
  t3 = t2;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto pushadiviex;
  arg1 = *(s32 *)(iFP + 24);   		// Self 
  t6 = *(s32 *)(iFP + 28);   
  arg1 = (u32)arg1;   
  t5 = t6 - Type_Instance;   
  t5 = t5 & 60;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto ivbadinst;
  t5 = t6 & 192;		// Unshifted cdr code 
  t5 = t5 - 64;   		// Check for CDR code 1 
  if (t5 != 0)   		// J. if CDR code is not 1 
    goto locate_instance0variable_mapped31449;

locate_instance0variable_mapped31448:
  if (_trace) printf("locate_instance0variable_mapped31448:\n");
  arg1 = arg1 + t3;

locate_instance0variable_mapped31447:
  if (_trace) printf("locate_instance0variable_mapped31447:\n");
  t7 = Type_Locative;
  *(u32 *)(iSP + 8) = arg1;   
  *(u32 *)(iSP + 12) = t7;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

pushadiviex:
  if (_trace) printf("pushadiviex:\n");
  t1 = zero + 8;   
  /* SetTag. */
  t1 = t1 << 32;   
  t1 = arg2 | t1;
  arg6 = t2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

vma_memory_read31462:
  if (_trace) printf("vma_memory_read31462:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31461;   

vma_memory_read31464:
  if (_trace) printf("vma_memory_read31464:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31463;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31460;   

vma_memory_read31463:
  if (_trace) printf("vma_memory_read31463:\n");
  t10 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31468:
  if (_trace) printf("vma_memory_read31468:\n");
  t9 = t10 & MemoryActionTransform;
  if (t9 == 0) 
    goto vma_memory_read31467;
  t1 = t1 & ~63L;
  t1 = t1 | Type_ExternalValueCellPointer;
  goto vma_memory_read31471;   

vma_memory_read31467:

vma_memory_read31466:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31452:
  if (_trace) printf("vma_memory_read31452:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31451;   

vma_memory_read31454:
  if (_trace) printf("vma_memory_read31454:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31453;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31450;   

vma_memory_read31453:
  if (_trace) printf("vma_memory_read31453:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31456:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

locate_instance0variable_mapped31449:
  if (_trace) printf("locate_instance0variable_mapped31449:\n");
  t5 = arg1;
  /* Memory Read Internal */

vma_memory_read31472:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31474;

vma_memory_read31473:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read31476;

vma_memory_read31481:
  t5 = t5 - arg1;   
  if (t5 != 0)   
    goto locate_instance0variable_mapped31448;
  /* TagType. */
  t6 = t6 & 63;
  t6 = t6 | 64;		// Set CDR code to 1 
  *(u32 *)(iFP + 24) = arg1;   		// Update self 
  *(u32 *)(iFP + 28) = t6;   		// write the stack cache 
  goto locate_instance0variable_mapped31448;   

vma_memory_read31474:
  if (_trace) printf("vma_memory_read31474:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31473;   

vma_memory_read31476:
  if (_trace) printf("vma_memory_read31476:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read31475;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31472;   

vma_memory_read31475:
  if (_trace) printf("vma_memory_read31475:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read31478:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

/* end DoPushAddressInstanceVariable */
  /* End of Halfword operand from stack instruction - DoPushAddressInstanceVariable */
/* start DoPushInstanceVariableOrdered */

  /* Halfword 10 bit immediate instruction - DoPushInstanceVariableOrdered */

dopushinstancevariableordered:
  if (_trace) printf("dopushinstancevariableordered:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPushInstanceVariableOrderedIM:
  if (_trace) printf("DoPushInstanceVariableOrderedIM:\n");

DoPushInstanceVariableOrderedSP:
  if (_trace) printf("DoPushInstanceVariableOrderedSP:\n");

DoPushInstanceVariableOrderedLP:
  if (_trace) printf("DoPushInstanceVariableOrderedLP:\n");

DoPushInstanceVariableOrderedFP:
  if (_trace) printf("DoPushInstanceVariableOrderedFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Unmapped */
  t2 = *(s32 *)(iFP + 24);   		// self 
  t1 = *(s32 *)(iFP + 28);   
  t2 = (u32)t2;   
  t3 = t1 - Type_Instance;   
  t3 = t3 & 60;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto ivbadinst;
  arg1 = t2 + arg2;
  /* Memory Read Internal */

vma_memory_read31482:
  t6 = arg1 + ivory;
  t1 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = arg1 - arg5;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->dataread_mask);   
  t5 = ((u64)t4 < (u64)arg6) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read31484;

vma_memory_read31483:
  t6 = zero + 240;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read31486;

vma_memory_read31493:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = t2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t1;   
  *(u32 *)(iSP + 12) = t7;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

vma_memory_read31484:
  if (_trace) printf("vma_memory_read31484:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read31483;   

vma_memory_read31486:
  if (_trace) printf("vma_memory_read31486:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read31485;
  arg1 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read31482;   

vma_memory_read31485:
  if (_trace) printf("vma_memory_read31485:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read31490:
  if (_trace) printf("vma_memory_read31490:\n");
  t6 = t7 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read31489;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read31493;   

vma_memory_read31489:

vma_memory_read31488:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 0;
  goto performmemoryaction;

/* end DoPushInstanceVariableOrdered */
  /* End of Halfword operand from stack instruction - DoPushInstanceVariableOrdered */
/* start DoPopInstanceVariableOrdered */

  /* Halfword 10 bit immediate instruction - DoPopInstanceVariableOrdered */

dopopinstancevariableordered:
  if (_trace) printf("dopopinstancevariableordered:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPopInstanceVariableOrderedIM:
  if (_trace) printf("DoPopInstanceVariableOrderedIM:\n");

DoPopInstanceVariableOrderedSP:
  if (_trace) printf("DoPopInstanceVariableOrderedSP:\n");

DoPopInstanceVariableOrderedLP:
  if (_trace) printf("DoPopInstanceVariableOrderedLP:\n");

DoPopInstanceVariableOrderedFP:
  if (_trace) printf("DoPopInstanceVariableOrderedFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Unmapped */
  t2 = *(s32 *)(iFP + 24);   		// self 
  t1 = *(s32 *)(iFP + 28);   
  t2 = (u32)t2;   
  t3 = t1 - Type_Instance;   
  t3 = t3 & 60;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto ivbadinst;
  arg1 = t2 + arg2;
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  /* Memory Read Internal */

vma_memory_read31494:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read31496;

vma_memory_read31495:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read31498;

vma_memory_read31504:
  /* Merge cdr-code */
  t5 = t2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t6 = arg1 + ivory;
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment31506:
  if (_trace) printf("force_alignment31506:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write31505;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write31505:
  if (_trace) printf("vma_memory_write31505:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read31496:
  if (_trace) printf("vma_memory_read31496:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read31495;   

vma_memory_read31498:
  if (_trace) printf("vma_memory_read31498:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read31497;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read31494;   

vma_memory_read31497:
  if (_trace) printf("vma_memory_read31497:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read31501:

vma_memory_read31500:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

/* end DoPopInstanceVariableOrdered */
  /* End of Halfword operand from stack instruction - DoPopInstanceVariableOrdered */
/* start DoMovemInstanceVariableOrdered */

  /* Halfword 10 bit immediate instruction - DoMovemInstanceVariableOrdered */

domoveminstancevariableordered:
  if (_trace) printf("domoveminstancevariableordered:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoMovemInstanceVariableOrderedIM:
  if (_trace) printf("DoMovemInstanceVariableOrderedIM:\n");

DoMovemInstanceVariableOrderedSP:
  if (_trace) printf("DoMovemInstanceVariableOrderedSP:\n");

DoMovemInstanceVariableOrderedLP:
  if (_trace) printf("DoMovemInstanceVariableOrderedLP:\n");

DoMovemInstanceVariableOrderedFP:
  if (_trace) printf("DoMovemInstanceVariableOrderedFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Instance Variable Unmapped */
  t2 = *(s32 *)(iFP + 24);   		// self 
  t1 = *(s32 *)(iFP + 28);   
  t2 = (u32)t2;   
  t3 = t1 - Type_Instance;   
  t3 = t3 & 60;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto ivbadinst;
  arg1 = t2 + arg2;
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  t1 = (u32)t1;   
  /* Memory Read Internal */

vma_memory_read31507:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read31509;

vma_memory_read31508:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read31511;

vma_memory_read31517:
  /* Merge cdr-code */
  t5 = t2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t6 = arg1 + ivory;
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment31519:
  if (_trace) printf("force_alignment31519:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write31518;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write31518:
  if (_trace) printf("vma_memory_write31518:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read31509:
  if (_trace) printf("vma_memory_read31509:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read31508;   

vma_memory_read31511:
  if (_trace) printf("vma_memory_read31511:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read31510;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read31507;   

vma_memory_read31510:
  if (_trace) printf("vma_memory_read31510:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read31514:

vma_memory_read31513:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

/* end DoMovemInstanceVariableOrdered */
  /* End of Halfword operand from stack instruction - DoMovemInstanceVariableOrdered */
/* start DoPushAddressInstanceVariableOrdered */

  /* Halfword 10 bit immediate instruction - DoPushAddressInstanceVariableOrdered */

dopushaddressinstancevariableordered:
  if (_trace) printf("dopushaddressinstancevariableordered:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPushAddressInstanceVariableOrderedIM:
  if (_trace) printf("DoPushAddressInstanceVariableOrderedIM:\n");

DoPushAddressInstanceVariableOrderedSP:
  if (_trace) printf("DoPushAddressInstanceVariableOrderedSP:\n");

DoPushAddressInstanceVariableOrderedLP:
  if (_trace) printf("DoPushAddressInstanceVariableOrderedLP:\n");

DoPushAddressInstanceVariableOrderedFP:
  if (_trace) printf("DoPushAddressInstanceVariableOrderedFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  /* Locate Instance Variable Unmapped */
  t2 = *(s32 *)(iFP + 24);   		// self 
  t1 = *(s32 *)(iFP + 28);   
  t2 = (u32)t2;   
  t3 = t1 - Type_Instance;   
  t3 = t3 & 60;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto ivbadinst;
  arg1 = t2 + arg2;
  t7 = Type_Locative;
  *(u32 *)(iSP + 8) = arg1;   
  *(u32 *)(iSP + 12) = t7;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

ivbadmap:
  if (_trace) printf("ivbadmap:\n");
  arg5 = 0;
  arg2 = 68;
  goto illegaloperand;

ivbadindex:
  if (_trace) printf("ivbadindex:\n");
  arg5 = 0;
  arg2 = 53;
  goto illegaloperand;

ivbadinst:
  if (_trace) printf("ivbadinst:\n");
  arg5 = 0;
  arg2 = 69;
  goto illegaloperand;

/* end DoPushAddressInstanceVariableOrdered */
  /* End of Halfword operand from stack instruction - DoPushAddressInstanceVariableOrdered */
/* start DoInstanceRef */

  /* Halfword operand from stack instruction - DoInstanceRef */
  /* arg2 has the preloaded 8 bit operand. */

doinstanceref:
  if (_trace) printf("doinstanceref:\n");

DoInstanceRefIM:
  if (_trace) printf("DoInstanceRefIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoinstanceref;   

DoInstanceRefSP:
  if (_trace) printf("DoInstanceRefSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoInstanceRefLP:
  if (_trace) printf("DoInstanceRefLP:\n");

DoInstanceRefFP:
  if (_trace) printf("DoInstanceRefFP:\n");

headdoinstanceref:
  if (_trace) printf("headdoinstanceref:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoinstanceref:
  if (_trace) printf("begindoinstanceref:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Arbitrary Instance Variable */
  t1 = arg3 - Type_Instance;   
  t1 = t1 & 60;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto ivrefbadinst;
  t1 = arg2 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto ivrefbadoffset;
  /* Memory Read Internal */

vma_memory_read31520:
  t7 = arg4 + ivory;
  t1 = (t7 * 4);   
  t2 = LDQ_U(t7);   
  t5 = arg4 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31522;

vma_memory_read31521:
  t7 = zero + 64;   
  t8 = t8 >> (t2 & 63);   
  t7 = t7 >> (t2 & 63);   
  t1 = (u32)t1;   
  if (t8 & 1)   
    goto vma_memory_read31524;

vma_memory_read31529:
  t1 = t1 - 1;   
  /* Memory Read Internal */

vma_memory_read31530:
  t7 = t1 + ivory;
  t2 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = t1 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31532;

vma_memory_read31531:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31534;

vma_memory_read31541:
  t5 = t4 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto ivrefbadoffset;
  if ((s64)arg1 < 0)   		// J. if offset <0 
    goto ivrefbadoffset;
  t4 = arg1 - t2;   
  if ((s64)t4 >= 0)   		// J. if offset out of bounds 
    goto ivrefbadoffset;
  arg5 = arg1 + arg4;
  /* Memory Read Internal */

vma_memory_read31542:
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t6 = arg5 + ivory;
  t5 = *(s32 *)&processor->scovlimit;   
  t1 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = arg5 - t4;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->dataread_mask);   
  t5 = ((u64)t4 < (u64)t5) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read31544;

vma_memory_read31543:
  t6 = zero + 240;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read31546;

vma_memory_read31553:
  t2 = t2 & 63;		// set CDR-NEXT 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)iSP = t1;   
  *(u32 *)(iSP + 4) = t2;   		// write the stack cache 
  goto cachevalid;   

vma_memory_read31544:
  if (_trace) printf("vma_memory_read31544:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read31543;   

vma_memory_read31546:
  if (_trace) printf("vma_memory_read31546:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read31545;
  arg5 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read31542;   

vma_memory_read31545:
  if (_trace) printf("vma_memory_read31545:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg5;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read31550:
  if (_trace) printf("vma_memory_read31550:\n");
  t6 = t7 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read31549;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read31553;   

vma_memory_read31549:

vma_memory_read31548:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31532:
  if (_trace) printf("vma_memory_read31532:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t2 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31531;   

vma_memory_read31534:
  if (_trace) printf("vma_memory_read31534:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31533;
  t1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31530;   

vma_memory_read31533:
  if (_trace) printf("vma_memory_read31533:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31538:
  if (_trace) printf("vma_memory_read31538:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read31537;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read31541;   

vma_memory_read31537:

vma_memory_read31536:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31522:
  if (_trace) printf("vma_memory_read31522:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t1 = *(s32 *)t5;   
  t2 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31521;   

vma_memory_read31524:
  if (_trace) printf("vma_memory_read31524:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31523;
  arg4 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read31520;   

vma_memory_read31523:
  if (_trace) printf("vma_memory_read31523:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31526:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end DoInstanceRef */
  /* End of Halfword operand from stack instruction - DoInstanceRef */
/* start DoInstanceSet */

  /* Halfword operand from stack instruction - DoInstanceSet */
  /* arg2 has the preloaded 8 bit operand. */

doinstanceset:
  if (_trace) printf("doinstanceset:\n");

DoInstanceSetIM:
  if (_trace) printf("DoInstanceSetIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoinstanceset;   

DoInstanceSetSP:
  if (_trace) printf("DoInstanceSetSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoInstanceSetLP:
  if (_trace) printf("DoInstanceSetLP:\n");

DoInstanceSetFP:
  if (_trace) printf("DoInstanceSetFP:\n");

headdoinstanceset:
  if (_trace) printf("headdoinstanceset:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoinstanceset:
  if (_trace) printf("begindoinstanceset:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   
  arg3 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Arbitrary Instance Variable */
  t1 = arg3 - Type_Instance;   
  t1 = t1 & 60;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto ivrefbadinst3;
  t1 = arg2 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto ivrefbadoffset;
  /* Memory Read Internal */

vma_memory_read31554:
  t7 = arg4 + ivory;
  t1 = (t7 * 4);   
  t2 = LDQ_U(t7);   
  t5 = arg4 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31556;

vma_memory_read31555:
  t7 = zero + 64;   
  t8 = t8 >> (t2 & 63);   
  t7 = t7 >> (t2 & 63);   
  t1 = (u32)t1;   
  if (t8 & 1)   
    goto vma_memory_read31558;

vma_memory_read31563:
  t1 = t1 - 1;   
  /* Memory Read Internal */

vma_memory_read31564:
  t7 = t1 + ivory;
  t2 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = t1 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31566;

vma_memory_read31565:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31568;

vma_memory_read31575:
  t5 = t4 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto ivrefbadoffset;
  if ((s64)arg1 < 0)   		// J. if offset <0 
    goto ivrefbadoffset;
  t4 = arg1 - t2;   
  if ((s64)t4 >= 0)   		// J. if offset out of bounds 
    goto ivrefbadoffset;
  arg5 = arg1 + arg4;
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read31576:
  t7 = arg5 + ivory;
  t4 = (t7 * 4);   
  t3 = LDQ_U(t7);   
  t5 = arg5 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t4 = *(s32 *)t4;   
  t3 = (u8)(t3 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31578;

vma_memory_read31577:
  t7 = zero + 240;   
  t8 = t8 >> (t3 & 63);   
  t7 = t7 >> (t3 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31580;

vma_memory_read31586:
  /* Merge cdr-code */
  t4 = t2 & 63;
  t3 = t3 & 192;
  t3 = t3 | t4;
  t5 = arg5 + ivory;
  t4 = (t5 * 4);   
  t7 = LDQ_U(t5);   
  t6 = arg5 - t11;   		// Stack cache offset 
  t8 = ((u64)t6 < (u64)t12) ? 1 : 0;   		// In range? 
  t6 = (t3 & 0xff) << ((t5&7)*8);   
  t7 = t7 & ~(0xffL << (t5&7)*8);   

force_alignment31588:
  if (_trace) printf("force_alignment31588:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)t4 = t1;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write31587;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

ivrefbadinst3:
  if (_trace) printf("ivrefbadinst3:\n");
  arg5 = 0;
  arg2 = 4;
  goto illegaloperand;

vma_memory_write31587:
  if (_trace) printf("vma_memory_write31587:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t6 = arg5 - t11;   		// Stack cache offset 
  t5 = (t6 * 8) + t5;  		// reconstruct SCA 
  *(u32 *)t5 = t1;   		// Store in stack 
  *(u32 *)(t5 + 4) = t3;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read31578:
  if (_trace) printf("vma_memory_read31578:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t4 = *(s32 *)t5;   
  t3 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31577;   

vma_memory_read31580:
  if (_trace) printf("vma_memory_read31580:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31579;
  arg5 = (u32)t4;   		// Do the indirect thing 
  goto vma_memory_read31576;   

vma_memory_read31579:
  if (_trace) printf("vma_memory_read31579:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg5;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31583:

vma_memory_read31582:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read31566:
  if (_trace) printf("vma_memory_read31566:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t2 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31565;   

vma_memory_read31568:
  if (_trace) printf("vma_memory_read31568:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31567;
  t1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31564;   

vma_memory_read31567:
  if (_trace) printf("vma_memory_read31567:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31572:
  if (_trace) printf("vma_memory_read31572:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read31571;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read31575;   

vma_memory_read31571:

vma_memory_read31570:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31556:
  if (_trace) printf("vma_memory_read31556:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t1 = *(s32 *)t5;   
  t2 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31555;   

vma_memory_read31558:
  if (_trace) printf("vma_memory_read31558:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31557;
  arg4 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read31554;   

vma_memory_read31557:
  if (_trace) printf("vma_memory_read31557:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31560:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end DoInstanceSet */
  /* End of Halfword operand from stack instruction - DoInstanceSet */
/* start DoInstanceLoc */

  /* Halfword operand from stack instruction - DoInstanceLoc */
  /* arg2 has the preloaded 8 bit operand. */

doinstanceloc:
  if (_trace) printf("doinstanceloc:\n");

DoInstanceLocIM:
  if (_trace) printf("DoInstanceLocIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoinstanceloc;   

DoInstanceLocSP:
  if (_trace) printf("DoInstanceLocSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoInstanceLocLP:
  if (_trace) printf("DoInstanceLocLP:\n");

DoInstanceLocFP:
  if (_trace) printf("DoInstanceLocFP:\n");

headdoinstanceloc:
  if (_trace) printf("headdoinstanceloc:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoinstanceloc:
  if (_trace) printf("begindoinstanceloc:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Locate Arbitrary Instance Variable */
  t1 = arg3 - Type_Instance;   
  t1 = t1 & 60;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto ivrefbadinst;
  t1 = arg2 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto ivrefbadoffset;
  /* Memory Read Internal */

vma_memory_read31589:
  t7 = arg4 + ivory;
  t1 = (t7 * 4);   
  t2 = LDQ_U(t7);   
  t5 = arg4 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31591;

vma_memory_read31590:
  t7 = zero + 64;   
  t8 = t8 >> (t2 & 63);   
  t7 = t7 >> (t2 & 63);   
  t1 = (u32)t1;   
  if (t8 & 1)   
    goto vma_memory_read31593;

vma_memory_read31598:
  t1 = t1 - 1;   
  /* Memory Read Internal */

vma_memory_read31599:
  t7 = t1 + ivory;
  t2 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = t1 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31601;

vma_memory_read31600:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read31603;

vma_memory_read31610:
  t5 = t4 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto ivrefbadoffset;
  if ((s64)arg1 < 0)   		// J. if offset <0 
    goto ivrefbadoffset;
  t4 = arg1 - t2;   
  if ((s64)t4 >= 0)   		// J. if offset out of bounds 
    goto ivrefbadoffset;
  arg5 = arg1 + arg4;
  t7 = Type_Locative;
  *(u32 *)iSP = arg5;   
  *(u32 *)(iSP + 4) = t7;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

ivrefbadinst:
  if (_trace) printf("ivrefbadinst:\n");
  arg5 = 0;
  arg2 = 3;
  goto illegaloperand;

ivrefbadoffset:
  if (_trace) printf("ivrefbadoffset:\n");
  arg5 = 0;
  arg2 = 49;
  goto illegaloperand;

vma_memory_read31601:
  if (_trace) printf("vma_memory_read31601:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t2 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31600;   

vma_memory_read31603:
  if (_trace) printf("vma_memory_read31603:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31602;
  t1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read31599;   

vma_memory_read31602:
  if (_trace) printf("vma_memory_read31602:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31607:
  if (_trace) printf("vma_memory_read31607:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read31606;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read31610;   

vma_memory_read31606:

vma_memory_read31605:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31591:
  if (_trace) printf("vma_memory_read31591:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t1 = *(s32 *)t5;   
  t2 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31590;   

vma_memory_read31593:
  if (_trace) printf("vma_memory_read31593:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31592;
  arg4 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read31589;   

vma_memory_read31592:
  if (_trace) printf("vma_memory_read31592:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31595:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end DoInstanceLoc */
  /* End of Halfword operand from stack instruction - DoInstanceLoc */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifuninst.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunmath.as
 ************************************************************************/

  /* Arithmetic. */
/* start DoUnaryMinus */

  /* Halfword operand from stack instruction - DoUnaryMinus */
  /* arg2 has the preloaded 8 bit operand. */

dounaryminus:
  if (_trace) printf("dounaryminus:\n");

DoUnaryMinusSP:
  if (_trace) printf("DoUnaryMinusSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoUnaryMinusLP:
  if (_trace) printf("DoUnaryMinusLP:\n");

DoUnaryMinusFP:
  if (_trace) printf("DoUnaryMinusFP:\n");

begindounaryminus:
  if (_trace) printf("begindounaryminus:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  arg5 = *(s32 *)(arg1 + 4);   		// tag of ARG2 
  arg6 = *(s32 *)arg1;   
  t2 = *(u64 *)&(processor->mostnegativefixnum);   
  LDS(1, f1, *(u32 *)arg1 );   
  t5 = arg5 & 63;		// Strip off any CDR code bits. 
  t4 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment31616:
  if (_trace) printf("force_alignment31616:\n");
  if (t4 == 0) 
    goto basic_dispatch31612;
  /* Here if argument TypeFixnum */
  t2 = (s32)arg6 - (s32)t2;   
  arg2 = (s32)zero - (s32)arg6;   
  if (t2 == 0) 
    goto unaryminusexc;
  iPC = t6;
  *(u32 *)(iSP + 12) = t5;   		// Semi-cheat, we know t5 has CDRNext/TypeFixnum 
  iCP = t7;
  *(u32 *)(iSP + 8) = arg2;   		// Push the data 
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch31612:
  if (_trace) printf("basic_dispatch31612:\n");
  t4 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment31617:
  if (_trace) printf("force_alignment31617:\n");
  if (t4 == 0) 
    goto basic_dispatch31613;
  /* Here if argument TypeSingleFloat */
  /* NIL */
  SUBS(0, f0, 3, f31, 1, f1); /* subs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = t6;
  *(u32 *)(iSP + 12) = t5;   		// Semi-cheat, we know t5 has CDRNext/TypeSingleFloat 
  iCP = t7;
  STS( (u32 *)(iSP + 8), 0, f0 );   		// Push the data 
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch31613:
  if (_trace) printf("basic_dispatch31613:\n");
  /* Here for all other cases */

unaryminusexc:
  if (_trace) printf("unaryminusexc:\n");
  arg6 = arg5;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch31611:
  if (_trace) printf("basic_dispatch31611:\n");

DoUnaryMinusIM:
  if (_trace) printf("DoUnaryMinusIM:\n");
  arg2 = (s32)zero - (s32)arg2;   		// Negate the 8 bit immediate operand 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t7;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

/* end DoUnaryMinus */
  /* End of Halfword operand from stack instruction - DoUnaryMinus */
/* start DoMultiply */

  /* Halfword operand from stack instruction - DoMultiply */
  /* arg2 has the preloaded 8 bit operand. */

domultiply:
  if (_trace) printf("domultiply:\n");

DoMultiplySP:
  if (_trace) printf("DoMultiplySP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindomultiply;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoMultiplyLP:
  if (_trace) printf("DoMultiplyLP:\n");

DoMultiplyFP:
  if (_trace) printf("DoMultiplyFP:\n");

begindomultiply:
  if (_trace) printf("begindomultiply:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  LDS(1, f1, *(u32 *)iSP );   
  t1 = (u32)(arg6 >> ((4&7)*8));   		// ARG1 tag 
  t3 = *(s32 *)(arg1 + 4);   		// ARG2 tag 
  t2 = (s32)arg6;		// ARG1 data 
  t4 = *(s32 *)arg1;   		// ARG2 data 
  LDS(2, f2, *(u32 *)arg1 );   
  /* NIL */
  t9 = t1 & 63;		// Strip off any CDR code bits. 
  t11 = t3 & 63;		// Strip off any CDR code bits. 
  t10 = (t9 == Type_Fixnum) ? 1 : 0;   

force_alignment31657:
  if (_trace) printf("force_alignment31657:\n");
  if (t10 == 0) 
    goto basic_dispatch31628;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31634:
  if (_trace) printf("force_alignment31634:\n");
  if (t12 == 0) 
    goto basic_dispatch31630;
  /* Here if argument TypeFixnum */
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = (s64)((s32)t2 * (s64)(s32)t4); /* mull/v */   		// compute 64-bit result 
  if (t5 >> 31) exception(3, t5);  // WARNING !!! THIS SHOULD REFLECT THE DIFF FILE
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  *(u32 *)(iSP + 4) = t9;   		// Semi-cheat, we know temp2 has CDRNext/TypeFixnum 
  iPC = t6;
  *(u32 *)iSP = t5;   
  iCP = t7;
  goto cachevalid;   

basic_dispatch31630:
  if (_trace) printf("basic_dispatch31630:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31635:
  if (_trace) printf("force_alignment31635:\n");
  if (t12 == 0) 
    goto basic_dispatch31631;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation31618;   

basic_dispatch31631:
  if (_trace) printf("basic_dispatch31631:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31636:
  if (_trace) printf("force_alignment31636:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31625;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation31621;   

basic_dispatch31629:
  if (_trace) printf("basic_dispatch31629:\n");

basic_dispatch31628:
  if (_trace) printf("basic_dispatch31628:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment31658:
  if (_trace) printf("force_alignment31658:\n");
  if (t10 == 0) 
    goto basic_dispatch31637;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31643:
  if (_trace) printf("force_alignment31643:\n");
  if (t12 == 0) 
    goto basic_dispatch31639;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation31618:
  if (_trace) printf("simple_binary_arithmetic_operation31618:\n");
  MULS(0, f0, 1, f1, 2, f2); /* muls */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  goto cachevalid;   

basic_dispatch31639:
  if (_trace) printf("basic_dispatch31639:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31644:
  if (_trace) printf("force_alignment31644:\n");
  if (t12 == 0) 
    goto basic_dispatch31640;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation31618;   

basic_dispatch31640:
  if (_trace) printf("basic_dispatch31640:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31645:
  if (_trace) printf("force_alignment31645:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31625;
  /* Here if argument TypeDoubleFloat */

simple_binary_arithmetic_operation31621:
  if (_trace) printf("simple_binary_arithmetic_operation31621:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto simple_binary_arithmetic_operation31622;   

basic_dispatch31638:
  if (_trace) printf("basic_dispatch31638:\n");

basic_dispatch31637:
  if (_trace) printf("basic_dispatch31637:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31659:
  if (_trace) printf("force_alignment31659:\n");
  if (t10 == 0) 
    goto basic_dispatch31646;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31652:
  if (_trace) printf("force_alignment31652:\n");
  if (t12 == 0) 
    goto basic_dispatch31648;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0049;
  goto fetchdoublefloat;
return0049:
  LDT(1, f1, processor->fp0);   

simple_binary_arithmetic_operation31622:
  if (_trace) printf("simple_binary_arithmetic_operation31622:\n");
  arg2 = (u32)t4;   
  r0 = (u64)&&return0050;
  goto fetchdoublefloat;
return0050:
  LDT(2, f2, processor->fp0);   

simple_binary_arithmetic_operation31619:
  if (_trace) printf("simple_binary_arithmetic_operation31619:\n");
  MULT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0051;
  goto consdoublefloat;
return0051:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;   
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch31648:
  if (_trace) printf("basic_dispatch31648:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31653:
  if (_trace) printf("force_alignment31653:\n");
  if (t12 == 0) 
    goto basic_dispatch31649;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation31620:
  if (_trace) printf("simple_binary_arithmetic_operation31620:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0052;
  goto fetchdoublefloat;
return0052:
  LDT(1, f1, processor->fp0);   
  goto simple_binary_arithmetic_operation31619;   

basic_dispatch31649:
  if (_trace) printf("basic_dispatch31649:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31654:
  if (_trace) printf("force_alignment31654:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31625;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation31620;   

basic_dispatch31647:
  if (_trace) printf("basic_dispatch31647:\n");

basic_dispatch31646:
  if (_trace) printf("basic_dispatch31646:\n");
  /* Here for all other cases */

binary_type_dispatch31624:
  if (_trace) printf("binary_type_dispatch31624:\n");

domulovfl:
  if (_trace) printf("domulovfl:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch31626;   

binary_type_dispatch31625:
  if (_trace) printf("binary_type_dispatch31625:\n");
  t1 = t3;
  goto domulovfl;   

binary_type_dispatch31626:
  if (_trace) printf("binary_type_dispatch31626:\n");

basic_dispatch31627:
  if (_trace) printf("basic_dispatch31627:\n");

DoMultiplyIM:
  if (_trace) printf("DoMultiplyIM:\n");
  arg2 = arg2 << 56;   
  t1 = (u32)(arg6 >> ((4&7)*8));   
  t2 = (s32)arg6;		// get ARG1 tag/data 
  arg2 = (s64)arg2 >> 56;   
  t11 = t1 & 63;		// Strip off any CDR code bits. 
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31664:
  if (_trace) printf("force_alignment31664:\n");
  if (t12 == 0) 
    goto basic_dispatch31661;
  /* Here if argument TypeFixnum */
  t3 = t2 * arg2;   		// compute 64-bit result 
  t4 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t10 = (s32)t3;		// compute 32-bit sign-extended result 
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t10 = (t3 == t10) ? 1 : 0;   		// is it the same as the 64-bit result? 
  if (t10 == 0) 		// if not, we overflowed 
    goto domulovfl;
  *(u32 *)(iSP + 4) = t11;   		// Semi-cheat, we know temp2 has CDRNext/TypeFixnum 
  iPC = t4;
  *(u32 *)iSP = t3;   
  iCP = t5;
  goto cachevalid;   

basic_dispatch31661:
  if (_trace) printf("basic_dispatch31661:\n");
  /* Here for all other cases */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindomultiply;   

basic_dispatch31660:
  if (_trace) printf("basic_dispatch31660:\n");

/* end DoMultiply */
  /* End of Halfword operand from stack instruction - DoMultiply */
/* start BinaryArithmeticDivisionPrelude */


binaryarithmeticdivisionprelude:
  if (_trace) printf("binaryarithmeticdivisionprelude:\n");
  sp = sp + -8;   
  LDS(1, f1, *(u32 *)iSP );   
  t2 = (s32)arg6;		// ARG1 data 
  t4 = *(s32 *)arg1;   		// ARG2 data 
  t1 = (u32)(arg6 >> ((4&7)*8));   		// ARG1 tag 
  t3 = *(s32 *)(arg1 + 4);   		// ARG2 tag 
  LDS(2, f2, *(u32 *)arg1 );   
  t9 = t1 & 63;		// Strip off any CDR code bits. 
  t11 = t3 & 63;		// Strip off any CDR code bits. 
  t10 = (t9 == Type_Fixnum) ? 1 : 0;   

force_alignment31702:
  if (_trace) printf("force_alignment31702:\n");
  if (t10 == 0) 
    goto basic_dispatch31675;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31681:
  if (_trace) printf("force_alignment31681:\n");
  if (t12 == 0) 
    goto basic_dispatch31677;
  /* Here if argument TypeFixnum */
  CVTLQ(1, f1, f31, 1, f1);
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(1, f1, f31, 1, f1);
  CVTQT(2, f2, f31, 2, f2);

basic_dispatch31676:
  if (_trace) printf("basic_dispatch31676:\n");

basic_dispatch31674:
  if (_trace) printf("basic_dispatch31674:\n");

binary_arithmetic_division_prelude31665:
  if (_trace) printf("binary_arithmetic_division_prelude31665:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch31675:
  if (_trace) printf("basic_dispatch31675:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment31703:
  if (_trace) printf("force_alignment31703:\n");
  if (t10 == 0) 
    goto basic_dispatch31682;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31688:
  if (_trace) printf("force_alignment31688:\n");
  if (t12 != 0)   
    goto binary_arithmetic_division_prelude31665;

basic_dispatch31684:
  if (_trace) printf("basic_dispatch31684:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31689:
  if (_trace) printf("force_alignment31689:\n");
  if (t12 == 0) 
    goto basic_dispatch31685;
  /* Here if argument TypeFixnum */
  t3 = t1;		// contagion 
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto binary_arithmetic_division_prelude31665;   

basic_dispatch31685:
  if (_trace) printf("basic_dispatch31685:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31690:
  if (_trace) printf("force_alignment31690:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31672;
  /* Here if argument TypeDoubleFloat */

binary_arithmetic_division_prelude31667:
  if (_trace) printf("binary_arithmetic_division_prelude31667:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto binary_arithmetic_division_prelude31668;   

basic_dispatch31683:
  if (_trace) printf("basic_dispatch31683:\n");

basic_dispatch31682:
  if (_trace) printf("basic_dispatch31682:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31704:
  if (_trace) printf("force_alignment31704:\n");
  if (t10 == 0) 
    goto basic_dispatch31691;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31697:
  if (_trace) printf("force_alignment31697:\n");
  if (t12 == 0) 
    goto basic_dispatch31693;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0053;
  goto fetchdoublefloat;
return0053:
  r0 = *(u64 *)sp;   
  LDT(1, f1, processor->fp0);   

binary_arithmetic_division_prelude31668:
  if (_trace) printf("binary_arithmetic_division_prelude31668:\n");
  arg2 = (u32)t4;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0054;
  goto fetchdoublefloat;
return0054:
  r0 = *(u64 *)sp;   
  LDT(2, f2, processor->fp0);   
  goto binary_arithmetic_division_prelude31665;   

basic_dispatch31693:
  if (_trace) printf("basic_dispatch31693:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31698:
  if (_trace) printf("force_alignment31698:\n");
  if (t12 == 0) 
    goto basic_dispatch31694;
  /* Here if argument TypeSingleFloat */

binary_arithmetic_division_prelude31666:
  if (_trace) printf("binary_arithmetic_division_prelude31666:\n");
  t3 = t1;		// contagion 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0055;
  goto fetchdoublefloat;
return0055:
  r0 = *(u64 *)sp;   
  LDT(1, f1, processor->fp0);   
  goto binary_arithmetic_division_prelude31665;   

basic_dispatch31694:
  if (_trace) printf("basic_dispatch31694:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31699:
  if (_trace) printf("force_alignment31699:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31672;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto binary_arithmetic_division_prelude31666;   

basic_dispatch31692:
  if (_trace) printf("basic_dispatch31692:\n");

basic_dispatch31691:
  if (_trace) printf("basic_dispatch31691:\n");
  /* Here for all other cases */

binary_type_dispatch31671:
  if (_trace) printf("binary_type_dispatch31671:\n");

binary_arithmetic_division_prelude31669:
  if (_trace) printf("binary_arithmetic_division_prelude31669:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch31673;   

binary_type_dispatch31672:
  if (_trace) printf("binary_type_dispatch31672:\n");
  t1 = t3;
  goto binary_arithmetic_division_prelude31669;   

binary_type_dispatch31673:
  if (_trace) printf("binary_type_dispatch31673:\n");

basic_dispatch31677:
  if (_trace) printf("basic_dispatch31677:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31705:
  if (_trace) printf("force_alignment31705:\n");
  if (t12 == 0) 
    goto basic_dispatch31678;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto binary_arithmetic_division_prelude31665;   

basic_dispatch31678:
  if (_trace) printf("basic_dispatch31678:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31706:
  if (_trace) printf("force_alignment31706:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31672;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto binary_arithmetic_division_prelude31667;   

/* end BinaryArithmeticDivisionPrelude */
/* start DoQuotient */

  /* Halfword operand from stack instruction - DoQuotient */
  /* arg2 has the preloaded 8 bit operand. */

doquotient:
  if (_trace) printf("doquotient:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoQuotientIM:
  if (_trace) printf("DoQuotientIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindoquotient;   

DoQuotientSP:
  if (_trace) printf("DoQuotientSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoquotient;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoQuotientLP:
  if (_trace) printf("DoQuotientLP:\n");

DoQuotientFP:
  if (_trace) printf("DoQuotientFP:\n");

begindoquotient:
  if (_trace) printf("begindoquotient:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  r0 = (u64)&&return0056;
  goto binaryarithmeticdivisionprelude;
return0056:
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31712:
  if (_trace) printf("force_alignment31712:\n");
  if (t9 == 0) 
    goto basic_dispatch31708;
  /* Here if argument TypeFixnum */
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVC(0, f0, f31, 0, f0);
  CVTQLV(0, f0, f31, 0, f0);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   

basic_dispatch31707:
  if (_trace) printf("basic_dispatch31707:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch31708:
  if (_trace) printf("basic_dispatch31708:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment31713:
  if (_trace) printf("force_alignment31713:\n");
  if (t9 == 0) 
    goto basic_dispatch31709;
  /* Here if argument TypeSingleFloat */
  DIVS(0, f0, 1, f1, 2, f2); /* divs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  goto basic_dispatch31707;   

basic_dispatch31709:
  if (_trace) printf("basic_dispatch31709:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31714:
  if (_trace) printf("force_alignment31714:\n");
  if (t9 == 0) 
    goto basic_dispatch31707;
  /* Here if argument TypeDoubleFloat */
  DIVT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0057;
  goto consdoublefloat;
return0057:
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;   
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  goto basic_dispatch31707;   

/* end DoQuotient */
  /* End of Halfword operand from stack instruction - DoQuotient */
/* start DoRationalQuotient */

  /* Halfword operand from stack instruction - DoRationalQuotient */
  /* arg2 has the preloaded 8 bit operand. */

dorationalquotient:
  if (_trace) printf("dorationalquotient:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoRationalQuotientIM:
  if (_trace) printf("DoRationalQuotientIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindorationalquotient;   

DoRationalQuotientSP:
  if (_trace) printf("DoRationalQuotientSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindorationalquotient;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoRationalQuotientLP:
  if (_trace) printf("DoRationalQuotientLP:\n");

DoRationalQuotientFP:
  if (_trace) printf("DoRationalQuotientFP:\n");

begindorationalquotient:
  if (_trace) printf("begindorationalquotient:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  r0 = (u64)&&return0058;
  goto binaryarithmeticdivisionprelude;
return0058:
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31720:
  if (_trace) printf("force_alignment31720:\n");
  if (t9 == 0) 
    goto basic_dispatch31716;
  /* Here if argument TypeFixnum */
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQSVI(0, f0, f31, 0, f0);
  CVTQLV(0, f0, f31, 0, f0);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   

basic_dispatch31715:
  if (_trace) printf("basic_dispatch31715:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch31716:
  if (_trace) printf("basic_dispatch31716:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment31721:
  if (_trace) printf("force_alignment31721:\n");
  if (t9 == 0) 
    goto basic_dispatch31717;
  /* Here if argument TypeSingleFloat */
  DIVS(0, f0, 1, f1, 2, f2); /* divs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  goto basic_dispatch31715;   

basic_dispatch31717:
  if (_trace) printf("basic_dispatch31717:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31722:
  if (_trace) printf("force_alignment31722:\n");
  if (t9 == 0) 
    goto basic_dispatch31715;
  /* Here if argument TypeDoubleFloat */
  DIVT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0059;
  goto consdoublefloat;
return0059:
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;   
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  goto basic_dispatch31715;   

/* end DoRationalQuotient */
  /* End of Halfword operand from stack instruction - DoRationalQuotient */
/* start DoFloor */

  /* Halfword operand from stack instruction - DoFloor */
  /* arg2 has the preloaded 8 bit operand. */

dofloor:
  if (_trace) printf("dofloor:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoFloorIM:
  if (_trace) printf("DoFloorIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindofloor;   

DoFloorSP:
  if (_trace) printf("DoFloorSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindofloor;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoFloorLP:
  if (_trace) printf("DoFloorLP:\n");

DoFloorFP:
  if (_trace) printf("DoFloorFP:\n");

begindofloor:
  if (_trace) printf("begindofloor:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  r0 = (u64)&&return0060;
  goto binaryarithmeticdivisionprelude;
return0060:
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVM(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31728:
  if (_trace) printf("force_alignment31728:\n");
  if (t9 == 0) 
    goto basic_dispatch31724;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch31723:
  if (_trace) printf("basic_dispatch31723:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch31724:
  if (_trace) printf("basic_dispatch31724:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment31729:
  if (_trace) printf("force_alignment31729:\n");
  if (t9 == 0) 
    goto basic_dispatch31725;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch31723;   

basic_dispatch31725:
  if (_trace) printf("basic_dispatch31725:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31730:
  if (_trace) printf("force_alignment31730:\n");
  if (t9 == 0) 
    goto basic_dispatch31723;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0061;
  goto consdoublefloat;
return0061:
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto basic_dispatch31723;   

/* end DoFloor */
  /* End of Halfword operand from stack instruction - DoFloor */
/* start DoCeiling */

  /* Halfword operand from stack instruction - DoCeiling */
  /* arg2 has the preloaded 8 bit operand. */

doceiling:
  if (_trace) printf("doceiling:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoCeilingIM:
  if (_trace) printf("DoCeilingIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindoceiling;   

DoCeilingSP:
  if (_trace) printf("DoCeilingSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoceiling;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoCeilingLP:
  if (_trace) printf("DoCeilingLP:\n");

DoCeilingFP:
  if (_trace) printf("DoCeilingFP:\n");

begindoceiling:
  if (_trace) printf("begindoceiling:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  r0 = (u64)&&return0062;
  goto binaryarithmeticdivisionprelude;
return0062:
  CPYSN(2, f2, 2, f2, 2, f2);   
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVM(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  CPYSN(0, f0, 3, f3, 3, f3);   
  CVTTQ(0, f0, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31736:
  if (_trace) printf("force_alignment31736:\n");
  if (t9 == 0) 
    goto basic_dispatch31732;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch31731:
  if (_trace) printf("basic_dispatch31731:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch31732:
  if (_trace) printf("basic_dispatch31732:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment31737:
  if (_trace) printf("force_alignment31737:\n");
  if (t9 == 0) 
    goto basic_dispatch31733;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch31731;   

basic_dispatch31733:
  if (_trace) printf("basic_dispatch31733:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31738:
  if (_trace) printf("force_alignment31738:\n");
  if (t9 == 0) 
    goto basic_dispatch31731;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0063;
  goto consdoublefloat;
return0063:
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto basic_dispatch31731;   

/* end DoCeiling */
  /* End of Halfword operand from stack instruction - DoCeiling */
/* start DoTruncate */

  /* Halfword operand from stack instruction - DoTruncate */
  /* arg2 has the preloaded 8 bit operand. */

dotruncate:
  if (_trace) printf("dotruncate:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoTruncateIM:
  if (_trace) printf("DoTruncateIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindotruncate;   

DoTruncateSP:
  if (_trace) printf("DoTruncateSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindotruncate;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoTruncateLP:
  if (_trace) printf("DoTruncateLP:\n");

DoTruncateFP:
  if (_trace) printf("DoTruncateFP:\n");

begindotruncate:
  if (_trace) printf("begindotruncate:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  r0 = (u64)&&return0064;
  goto binaryarithmeticdivisionprelude;
return0064:
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVC(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31744:
  if (_trace) printf("force_alignment31744:\n");
  if (t9 == 0) 
    goto basic_dispatch31740;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch31739:
  if (_trace) printf("basic_dispatch31739:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch31740:
  if (_trace) printf("basic_dispatch31740:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment31745:
  if (_trace) printf("force_alignment31745:\n");
  if (t9 == 0) 
    goto basic_dispatch31741;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch31739;   

basic_dispatch31741:
  if (_trace) printf("basic_dispatch31741:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31746:
  if (_trace) printf("force_alignment31746:\n");
  if (t9 == 0) 
    goto basic_dispatch31739;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0065;
  goto consdoublefloat;
return0065:
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto basic_dispatch31739;   

/* end DoTruncate */
  /* End of Halfword operand from stack instruction - DoTruncate */
/* start DoRound */

  /* Halfword operand from stack instruction - DoRound */
  /* arg2 has the preloaded 8 bit operand. */

doround:
  if (_trace) printf("doround:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoRoundIM:
  if (_trace) printf("DoRoundIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindoround;   

DoRoundSP:
  if (_trace) printf("DoRoundSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindoround;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoRoundLP:
  if (_trace) printf("DoRoundLP:\n");

DoRoundFP:
  if (_trace) printf("DoRoundFP:\n");

begindoround:
  if (_trace) printf("begindoround:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  r0 = (u64)&&return0066;
  goto binaryarithmeticdivisionprelude;
return0066:
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQV(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31752:
  if (_trace) printf("force_alignment31752:\n");
  if (t9 == 0) 
    goto basic_dispatch31748;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch31747:
  if (_trace) printf("basic_dispatch31747:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch31748:
  if (_trace) printf("basic_dispatch31748:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment31753:
  if (_trace) printf("force_alignment31753:\n");
  if (t9 == 0) 
    goto basic_dispatch31749;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch31747;   

basic_dispatch31749:
  if (_trace) printf("basic_dispatch31749:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment31754:
  if (_trace) printf("force_alignment31754:\n");
  if (t9 == 0) 
    goto basic_dispatch31747;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0067;
  goto consdoublefloat;
return0067:
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto basic_dispatch31747;   

/* end DoRound */
  /* End of Halfword operand from stack instruction - DoRound */
  /* Other arithmetic. */
/* start DoMax */

  /* Halfword operand from stack instruction - DoMax */
  /* arg2 has the preloaded 8 bit operand. */

domax:
  if (_trace) printf("domax:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoMaxIM:
  if (_trace) printf("DoMaxIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindomax;   

DoMaxSP:
  if (_trace) printf("DoMaxSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindomax;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoMaxLP:
  if (_trace) printf("DoMaxLP:\n");

DoMaxFP:
  if (_trace) printf("DoMaxFP:\n");

begindomax:
  if (_trace) printf("begindomax:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  LDS(1, f1, *(u32 *)iSP );   
  t1 = (u32)(arg6 >> ((4&7)*8));   		// ARG1 tag 
  t3 = *(s32 *)(arg1 + 4);   		// ARG2 tag 
  t2 = (s32)arg6;		// ARG1 data 
  t4 = *(s32 *)arg1;   		// ARG2 data 
  LDS(2, f2, *(u32 *)arg1 );   
  t9 = t1 & 63;		// Strip off any CDR code bits. 
  t11 = t3 & 63;		// Strip off any CDR code bits. 
  t10 = (t9 == Type_Fixnum) ? 1 : 0;   

force_alignment31778:
  if (_trace) printf("force_alignment31778:\n");
  if (t10 == 0) 
    goto basic_dispatch31762;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31767:
  if (_trace) printf("force_alignment31767:\n");
  if (t12 == 0) 
    goto basic_dispatch31764;
  /* Here if argument TypeFixnum */
  t5 = t2 - t4;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if ((s64)t5 > 0)   
    t4 = t2;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)iSP = t4;   		// We know temp2 has CDRNext/TypeFixnum 
  *(u32 *)(iSP + 4) = t9;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch31764:
  if (_trace) printf("basic_dispatch31764:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31768:
  if (_trace) printf("force_alignment31768:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31759;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto simple_binary_minmax31756;   

basic_dispatch31763:
  if (_trace) printf("basic_dispatch31763:\n");

basic_dispatch31762:
  if (_trace) printf("basic_dispatch31762:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment31779:
  if (_trace) printf("force_alignment31779:\n");
  if (t10 == 0) 
    goto basic_dispatch31769;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31774:
  if (_trace) printf("force_alignment31774:\n");
  if (t12 == 0) 
    goto basic_dispatch31771;
  /* Here if argument TypeSingleFloat */

simple_binary_minmax31756:
  if (_trace) printf("simple_binary_minmax31756:\n");
  /* NIL */
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (FLTU64(0, f0) > 0.0)   
    f2 = f1;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 2, f2 );   
  goto cachevalid;   

basic_dispatch31771:
  if (_trace) printf("basic_dispatch31771:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31775:
  if (_trace) printf("force_alignment31775:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31759;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto simple_binary_minmax31756;   

basic_dispatch31770:
  if (_trace) printf("basic_dispatch31770:\n");

basic_dispatch31769:
  if (_trace) printf("basic_dispatch31769:\n");
  /* Here for all other cases */

binary_type_dispatch31758:
  if (_trace) printf("binary_type_dispatch31758:\n");

simple_binary_minmax31755:
  if (_trace) printf("simple_binary_minmax31755:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch31760;   

binary_type_dispatch31759:
  if (_trace) printf("binary_type_dispatch31759:\n");
  t1 = t3;
  goto simple_binary_minmax31755;   

binary_type_dispatch31760:
  if (_trace) printf("binary_type_dispatch31760:\n");

basic_dispatch31761:
  if (_trace) printf("basic_dispatch31761:\n");

/* end DoMax */
  /* End of Halfword operand from stack instruction - DoMax */
/* start DoMin */

  /* Halfword operand from stack instruction - DoMin */
  /* arg2 has the preloaded 8 bit operand. */

domin:
  if (_trace) printf("domin:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoMinIM:
  if (_trace) printf("DoMinIM:\n");
  /* This sequence only sucks a moderate amount */
  arg1 = arg2 << 56;   		// sign extend the byte argument. 
  arg2 = zero;
  arg1 = (s64)arg1 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg1;   
  arg1 = (u64)&processor->immediate_arg;   
  goto begindomin;   

DoMinSP:
  if (_trace) printf("DoMinSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindomin;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoMinLP:
  if (_trace) printf("DoMinLP:\n");

DoMinFP:
  if (_trace) printf("DoMinFP:\n");

begindomin:
  if (_trace) printf("begindomin:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  LDS(1, f1, *(u32 *)iSP );   
  t1 = (u32)(arg6 >> ((4&7)*8));   		// ARG1 tag 
  t3 = *(s32 *)(arg1 + 4);   		// ARG2 tag 
  t2 = (s32)arg6;		// ARG1 data 
  t4 = *(s32 *)arg1;   		// ARG2 data 
  LDS(2, f2, *(u32 *)arg1 );   
  t9 = t1 & 63;		// Strip off any CDR code bits. 
  t11 = t3 & 63;		// Strip off any CDR code bits. 
  t10 = (t9 == Type_Fixnum) ? 1 : 0;   

force_alignment31803:
  if (_trace) printf("force_alignment31803:\n");
  if (t10 == 0) 
    goto basic_dispatch31787;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31792:
  if (_trace) printf("force_alignment31792:\n");
  if (t12 == 0) 
    goto basic_dispatch31789;
  /* Here if argument TypeFixnum */
  t5 = t2 - t4;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if ((s64)t5 < 0)   
    t4 = t2;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)iSP = t4;   		// We know temp2 has CDRNext/TypeFixnum 
  *(u32 *)(iSP + 4) = t9;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch31789:
  if (_trace) printf("basic_dispatch31789:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31793:
  if (_trace) printf("force_alignment31793:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31784;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto simple_binary_minmax31781;   

basic_dispatch31788:
  if (_trace) printf("basic_dispatch31788:\n");

basic_dispatch31787:
  if (_trace) printf("basic_dispatch31787:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment31804:
  if (_trace) printf("force_alignment31804:\n");
  if (t10 == 0) 
    goto basic_dispatch31794;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment31799:
  if (_trace) printf("force_alignment31799:\n");
  if (t12 == 0) 
    goto basic_dispatch31796;
  /* Here if argument TypeSingleFloat */

simple_binary_minmax31781:
  if (_trace) printf("simple_binary_minmax31781:\n");
  /* NIL */
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (FLTU64(0, f0) < 0.0)   
    f2 = f1;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
  *(u32 *)(iSP + 4) = t8;   		// write the stack cache 
  STS( (u32 *)iSP, 2, f2 );   
  goto cachevalid;   

basic_dispatch31796:
  if (_trace) printf("basic_dispatch31796:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment31800:
  if (_trace) printf("force_alignment31800:\n");
  if (t12 == 0) 
    goto binary_type_dispatch31784;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto simple_binary_minmax31781;   

basic_dispatch31795:
  if (_trace) printf("basic_dispatch31795:\n");

basic_dispatch31794:
  if (_trace) printf("basic_dispatch31794:\n");
  /* Here for all other cases */

binary_type_dispatch31783:
  if (_trace) printf("binary_type_dispatch31783:\n");

simple_binary_minmax31780:
  if (_trace) printf("simple_binary_minmax31780:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch31785;   

binary_type_dispatch31784:
  if (_trace) printf("binary_type_dispatch31784:\n");
  t1 = t3;
  goto simple_binary_minmax31780;   

binary_type_dispatch31785:
  if (_trace) printf("binary_type_dispatch31785:\n");

basic_dispatch31786:
  if (_trace) printf("basic_dispatch31786:\n");

/* end DoMin */
  /* End of Halfword operand from stack instruction - DoMin */
/* start DoMultiplyDouble */

  /* Halfword operand from stack instruction - DoMultiplyDouble */

domultiplydouble:
  if (_trace) printf("domultiplydouble:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoMultiplyDoubleIM:
  if (_trace) printf("DoMultiplyDoubleIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment31805:
  if (_trace) printf("force_alignment31805:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindomultiplydouble;   

DoMultiplyDoubleSP:
  if (_trace) printf("DoMultiplyDoubleSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoMultiplyDoubleLP:
  if (_trace) printf("DoMultiplyDoubleLP:\n");

DoMultiplyDoubleFP:
  if (_trace) printf("DoMultiplyDoubleFP:\n");

headdomultiplydouble:
  if (_trace) printf("headdomultiplydouble:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindomultiplydouble:
  if (_trace) printf("begindomultiplydouble:\n");
  /* arg1 has the operand, sign extended if immediate. */
  t2 = arg1 >> 32;   		// ARG2 tag 
  t3 = *(s32 *)iSP;   		// ARG1 data, sign extended 
  t4 = (s32)arg1 + (s32)0;		// ARG2 data, sign extended 
  t1 = *(s32 *)(iSP + 4);   		// ARG1 tag 
  /* TagType. */
  t1 = t1 & 63;		// Strip CDR code if any. 
  t1 = t1 - Type_Fixnum;   
  /* TagType. */
  t2 = t2 & 63;		// Strip CDR code if any. 
  t5 = t3 * t4;   		// Perform the 63 bit multiply. 
  t2 = t2 - Type_Fixnum;   
  if (t1 != 0)   
    goto muldexc;
  if (t2 != 0)   
    goto muldexc;
  t6 = (u32)t5;   		// Get the low 32 bit half. 
  t5 = (u32)(t5 >> ((4&7)*8));   		// Get the high 32 bit half. 
  *(u32 *)iSP = t6;   		// Put the result back on the stack 
  t1 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t5;   		// Push high order half 
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

muldexc:
  if (_trace) printf("muldexc:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

/* end DoMultiplyDouble */
  /* End of Halfword operand from stack instruction - DoMultiplyDouble */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunmath.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunarra.as
 ************************************************************************/

  /* Array operations. */
/* start Aref1Regset */


aref1regset:
  if (_trace) printf("aref1regset:\n");
  t12 = arg4;
  /* Memory Read Internal */

vma_memory_read31806:
  t1 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = arg4 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  arg6 = (t3 * 4);   
  arg5 = LDQ_U(t3);   
  t1 = arg4 - t1;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t2) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read31808;

vma_memory_read31807:
  t3 = zero + 64;   
  t4 = t4 >> (arg5 & 63);   
  t3 = t3 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t4 & 1)   
    goto vma_memory_read31810;

vma_memory_read31815:
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6 >> (Array_LongPrefixBitPos & 63);   
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto aref1illegal;
  if (t2 & 1)   
    goto aref1exception;
  *(u32 *)&((ARRAYCACHEP)t7)->array = t12;   		// store the array 
  t2 = zero + Array_LengthMask;   
  t1 = arg6 & t2;
  t2 = ((u64)arg2 < (u64)t1) ? 1 : 0;   
  if (t2 == 0) 
    goto aref1bounds;
  *(u64 *)&((ARRAYCACHEP)t7)->length = t1;   		// store the array length [implicit fixnum] 
  t10 = arg6 >> (Array_RegisterBytePackingPos & 63);   
  t8 = *(u64 *)&(processor->areventcount);   
  t10 = t10 << (Array_RegisterBytePackingPos & 63);   
  t9 = arg4 + 1;
  t10 = t10 + t8;		// Construct the array register word 
  *(u32 *)&((ARRAYCACHEP)t7)->arword = t10;   		// store the array register word [implicit fixnum] 
  *(u64 *)&((ARRAYCACHEP)t7)->locat = t9;   		// store the storage [implicit locative] 
  arg5 = arg6 >> (Array_BytePackingPos & 63);   		// get BP into arg5 
  arg6 = arg6 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg5 = arg5 & Array_BytePackingMask;
  arg4 = zero;
  arg6 = arg6 & Array_ElementTypeMask;
  goto aref1restart;   

vma_memory_read31808:
  if (_trace) printf("vma_memory_read31808:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg6 = *(s32 *)t1;   
  arg5 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read31807;   

vma_memory_read31810:
  if (_trace) printf("vma_memory_read31810:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read31809;
  arg4 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31806;   

vma_memory_read31809:
  if (_trace) printf("vma_memory_read31809:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read31812:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 6;
  goto performmemoryaction;

/* end Aref1Regset */
/* start Aref1RecomputeArrayRegister */


aref1recomputearrayregister:
  if (_trace) printf("aref1recomputearrayregister:\n");
  t5 = *(s32 *)(arg1 + -8);   
  t4 = *(s32 *)(arg1 + -4);   
  t5 = (u32)t5;   
  t6 = t4 - Type_Array;   
  t6 = t6 & 62;		// Strip CDR code, low bits 
  if (t6 != 0)   
    goto recompute_array_register31817;
  /* Memory Read Internal */

vma_memory_read31819:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = t5 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  t6 = (t3 * 4);   
  t7 = LDQ_U(t3);   
  t8 = t5 - t8;   		// Stack cache offset 
  t1 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t8 < (u64)t2) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read31821;

vma_memory_read31820:
  t3 = zero + 64;   
  t1 = t1 >> (t7 & 63);   
  t3 = t3 >> (t7 & 63);   
  t6 = (u32)t6;   
  if (t1 & 1)   
    goto vma_memory_read31823;

vma_memory_read31828:
  /* TagType. */
  t8 = t7 & 63;
  t2 = t6 >> (Array_LongPrefixBitPos & 63);   
  t8 = t8 - Type_HeaderI;   
  if (t8 != 0)   
    goto recompute_array_register31816;
  if (t2 & 1)   
    goto recompute_array_register31818;
  t1 = t6 >> (Array_BytePackingPos & 63);   
  t4 = *(u64 *)&(processor->areventcount);   
  t1 = t1 << (Array_RegisterBytePackingPos & 63);   
  t2 = t5 + 1;
  t1 = t1 + t4;		// Construct the array register word 
  *(u32 *)(arg1 + 8) = t2;   
  t3 = zero + Array_LengthMask;   
  t3 = t6 & t3;
  *(u32 *)arg1 = t1;   
  *(u32 *)(arg1 + 16) = t3;   
  goto fastaref1retry;   

recompute_array_register31818:
  if (_trace) printf("recompute_array_register31818:\n");
  *(u64 *)&processor->asrf5 = arg1;   		// Just a place to save these values 
  *(u64 *)&processor->asrf4 = t10;   		// Just a place to save these values 
  *(u64 *)&processor->asrf3 = t11;   		// Just a place to save these values 
  *(u64 *)&processor->asrf6 = arg1;   		// Just a place to save these values 
  *(u64 *)&processor->asrf7 = arg2;   		// Just a place to save these values 
  *(u64 *)&processor->asrf8 = arg3;   		// Just a place to save these values 
  *(u64 *)&processor->asrf9 = arg4;   		// Just a place to save these values 
  t9 = *(s32 *)(arg1 + -8);   
  arg2 = *(s32 *)(arg1 + -4);   
  t9 = (u32)t9;   
  arg1 = t5;
  t4 = t7;
  t3 = t6;
  t2 = 1;
  iSP = iSP + 24;
  r0 = (u64)&&return0068;
  goto setup1dlongarray;
return0068:
  t4 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t4 != 0)   
    goto recompute_array_register31817;
  arg1 = *(u64 *)&(processor->asrf5);   		// Just a place to save these values 
  t10 = *(u64 *)&(processor->asrf4);   		// Just a place to save these values 
  t11 = *(u64 *)&(processor->asrf3);   		// Just a place to save these values 
  arg1 = *(u64 *)&(processor->asrf6);   		// Just a place to save these values 
  arg2 = *(u64 *)&(processor->asrf7);   		// Just a place to save these values 
  arg3 = *(u64 *)&(processor->asrf8);   		// Just a place to save these values 
  arg4 = *(u64 *)&(processor->asrf9);   		// Just a place to save these values 
  t3 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  t2 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  t4 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  iSP = iSP - 24;   
  *(u32 *)arg1 = t1;   
  *(u32 *)(arg1 + 8) = t2;   
  *(u32 *)(arg1 + 16) = t3;   
  goto fastaref1retry;   

recompute_array_register31817:
  if (_trace) printf("recompute_array_register31817:\n");
  arg6 = t4;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 12;
  goto arrayexception;

recompute_array_register31816:
  if (_trace) printf("recompute_array_register31816:\n");
  arg5 = 0;
  arg2 = 12;
  goto illegaloperand;

vma_memory_read31821:
  if (_trace) printf("vma_memory_read31821:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t2;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read31820;   

vma_memory_read31823:
  if (_trace) printf("vma_memory_read31823:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read31822;
  t5 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read31819;   

vma_memory_read31822:
  if (_trace) printf("vma_memory_read31822:\n");
  t1 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t1;   		// Adjust for a longword load 
  t1 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read31825:
  /* Perform memory action */
  arg1 = t1;
  arg2 = 6;
  goto performmemoryaction;

/* end Aref1RecomputeArrayRegister */
/* start Aref1Exception */


aref1exception:
  if (_trace) printf("aref1exception:\n");
  *(u64 *)&processor->asrf4 = arg2;   		// Just a place to save these values 
  *(u64 *)&processor->asrf5 = t7;   		// Just a place to save these values 
  t9 = t12;
  arg2 = arg3;
  arg1 = arg4;
  t4 = arg5;
  t3 = arg6;
  t2 = zero;
  iSP = iSP + 24;
  r0 = (u64)&&return0069;
  goto setup1dlongarray;
return0069:
  arg2 = *(s32 *)&processor->asrf4;   		// Just a place to save these values 
  t7 = *(u64 *)&(processor->asrf5);   		// Just a place to save these values 
  t1 = *(s32 *)iSP;   		// Length 
  t5 = *(s32 *)(iSP + 4);   		// Length 
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  t5 = *(u64 *)iSP;   		// base 
  iSP = iSP - 8;   		// Pop Stack. 
  t3 = *(u64 *)iSP;   		// control 
  iSP = iSP - 8;   		// Pop Stack. 
  t9 = *(s32 *)iSP;   		// The original array 
  arg3 = *(s32 *)(iSP + 4);   		// The original array 
  iSP = iSP - 8;   		// Pop Stack. 
  t9 = (u32)t9;   
  iSP = iSP - 24;   
  *(u64 *)&((ARRAYCACHEP)t7)->length = t1;   
  *(u32 *)&((ARRAYCACHEP)t7)->arword = t3;   
  *(u32 *)&((ARRAYCACHEP)t7)->locat = t5;   
  *(u32 *)&((ARRAYCACHEP)t7)->array = t9;   		// store the array 
  t9 = (u32)t5;   
  t2 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t2 != 0)   
    goto reallyaref1exc;
  t5 = ((u64)arg2 < (u64)t1) ? 1 : 0;   
  if (t5 == 0) 
    goto aref1bounds;
  arg5 = t3 >> (Array_BytePackingPos & 63);   		// get BP into arg5 
  arg6 = t3 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg4 = t3 >> (Array_RegisterByteOffsetPos & 63);   
  arg5 = arg5 & Array_BytePackingMask;
  arg4 = arg4 & Array_RegisterByteOffsetMask;
  arg6 = arg6 & Array_ElementTypeMask;
  goto aref1restart;   

reallyaref1exc:
  if (_trace) printf("reallyaref1exc:\n");
  arg1 = Type_Fixnum;
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 8;
  goto arrayexception;

aref1illegal:
  if (_trace) printf("aref1illegal:\n");
  arg5 = 0;
  arg2 = 8;
  goto illegaloperand;

aref1bounds:
  if (_trace) printf("aref1bounds:\n");
  *(u64 *)&((ARRAYCACHEP)t7)->array = zero;   
  arg5 = 0;
  arg2 = 74;
  goto illegaloperand;

/* end Aref1Exception */
/* start Aset1Regset */


aset1regset:
  if (_trace) printf("aset1regset:\n");
  t12 = arg4;
  /* Memory Read Internal */

vma_memory_read31829:
  t1 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = arg4 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  arg6 = (t3 * 4);   
  arg5 = LDQ_U(t3);   
  t1 = arg4 - t1;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t2) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read31831;

vma_memory_read31830:
  t3 = zero + 64;   
  t4 = t4 >> (arg5 & 63);   
  t3 = t3 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t4 & 1)   
    goto vma_memory_read31833;

vma_memory_read31838:
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6 >> (Array_LongPrefixBitPos & 63);   
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto aset1illegal;
  if (t2 & 1)   
    goto aset1exception;
  *(u32 *)&((ARRAYCACHEP)t7)->array = t12;   		// store the array 
  t2 = zero + Array_LengthMask;   
  t1 = arg6 & t2;
  t2 = ((u64)arg2 < (u64)t1) ? 1 : 0;   
  if (t2 == 0) 
    goto aset1bounds;
  *(u64 *)&((ARRAYCACHEP)t7)->length = t1;   		// store the array length [implicit fixnum] 
  t10 = arg6 >> (Array_RegisterBytePackingPos & 63);   
  t8 = *(u64 *)&(processor->areventcount);   
  t10 = t10 << (Array_RegisterBytePackingPos & 63);   
  t9 = arg4 + 1;
  t10 = t10 + t8;		// Construct the array register word 
  *(u32 *)&((ARRAYCACHEP)t7)->arword = t10;   		// store the array register word [implicit fixnum] 
  *(u64 *)&((ARRAYCACHEP)t7)->locat = t9;   		// store the storage [implicit locative] 
  arg5 = arg6 >> (Array_BytePackingPos & 63);   		// get BP into arg5 
  arg6 = arg6 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg5 = arg5 & Array_BytePackingMask;
  arg4 = zero;
  arg6 = arg6 & Array_ElementTypeMask;
  goto aset1restart;   

vma_memory_read31831:
  if (_trace) printf("vma_memory_read31831:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg6 = *(s32 *)t1;   
  arg5 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read31830;   

vma_memory_read31833:
  if (_trace) printf("vma_memory_read31833:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read31832;
  arg4 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31829;   

vma_memory_read31832:
  if (_trace) printf("vma_memory_read31832:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read31835:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 6;
  goto performmemoryaction;

/* end Aset1Regset */
/* start Aset1RecomputeArrayRegister */


aset1recomputearrayregister:
  if (_trace) printf("aset1recomputearrayregister:\n");
  t5 = *(s32 *)(arg1 + -8);   
  t4 = *(s32 *)(arg1 + -4);   
  t5 = (u32)t5;   
  t6 = t4 - Type_Array;   
  t6 = t6 & 62;		// Strip CDR code, low bits 
  if (t6 != 0)   
    goto recompute_array_register31840;
  /* Memory Read Internal */

vma_memory_read31842:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = t5 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  t6 = (t3 * 4);   
  t7 = LDQ_U(t3);   
  t8 = t5 - t8;   		// Stack cache offset 
  t1 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t8 < (u64)t2) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read31844;

vma_memory_read31843:
  t3 = zero + 64;   
  t1 = t1 >> (t7 & 63);   
  t3 = t3 >> (t7 & 63);   
  t6 = (u32)t6;   
  if (t1 & 1)   
    goto vma_memory_read31846;

vma_memory_read31851:
  /* TagType. */
  t8 = t7 & 63;
  t2 = t6 >> (Array_LongPrefixBitPos & 63);   
  t8 = t8 - Type_HeaderI;   
  if (t8 != 0)   
    goto recompute_array_register31839;
  if (t2 & 1)   
    goto recompute_array_register31841;
  t1 = t6 >> (Array_BytePackingPos & 63);   
  t4 = *(u64 *)&(processor->areventcount);   
  t1 = t1 << (Array_RegisterBytePackingPos & 63);   
  t2 = t5 + 1;
  t1 = t1 + t4;		// Construct the array register word 
  *(u32 *)(arg1 + 8) = t2;   
  t3 = zero + Array_LengthMask;   
  t3 = t6 & t3;
  *(u32 *)arg1 = t1;   
  *(u32 *)(arg1 + 16) = t3;   
  goto fastaset1retry;   

recompute_array_register31841:
  if (_trace) printf("recompute_array_register31841:\n");
  *(u64 *)&processor->asrf5 = arg1;   		// Just a place to save these values 
  *(u64 *)&processor->asrf4 = t10;   		// Just a place to save these values 
  *(u64 *)&processor->asrf3 = t11;   		// Just a place to save these values 
  *(u64 *)&processor->asrf6 = arg1;   		// Just a place to save these values 
  *(u64 *)&processor->asrf7 = arg2;   		// Just a place to save these values 
  *(u64 *)&processor->asrf8 = arg3;   		// Just a place to save these values 
  *(u64 *)&processor->asrf9 = arg4;   		// Just a place to save these values 
  t9 = *(s32 *)(arg1 + -8);   
  arg2 = *(s32 *)(arg1 + -4);   
  t9 = (u32)t9;   
  arg1 = t5;
  t4 = t7;
  t3 = t6;
  t2 = 1;
  iSP = iSP + 24;
  r0 = (u64)&&return0070;
  goto setup1dlongarray;
return0070:
  t4 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t4 != 0)   
    goto recompute_array_register31840;
  arg1 = *(u64 *)&(processor->asrf5);   		// Just a place to save these values 
  t10 = *(u64 *)&(processor->asrf4);   		// Just a place to save these values 
  t11 = *(u64 *)&(processor->asrf3);   		// Just a place to save these values 
  arg1 = *(u64 *)&(processor->asrf6);   		// Just a place to save these values 
  arg2 = *(u64 *)&(processor->asrf7);   		// Just a place to save these values 
  arg3 = *(u64 *)&(processor->asrf8);   		// Just a place to save these values 
  arg4 = *(u64 *)&(processor->asrf9);   		// Just a place to save these values 
  t3 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  t2 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  t4 = *(u64 *)iSP;   
  iSP = iSP - 8;   		// Pop Stack. 
  iSP = iSP - 24;   
  *(u32 *)arg1 = t1;   
  *(u32 *)(arg1 + 8) = t2;   
  *(u32 *)(arg1 + 16) = t3;   
  goto fastaset1retry;   

recompute_array_register31840:
  if (_trace) printf("recompute_array_register31840:\n");
  arg6 = t4;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 3;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 12;
  goto arrayexception;

recompute_array_register31839:
  if (_trace) printf("recompute_array_register31839:\n");
  arg5 = 0;
  arg2 = 12;
  goto illegaloperand;

vma_memory_read31844:
  if (_trace) printf("vma_memory_read31844:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t2;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read31843;   

vma_memory_read31846:
  if (_trace) printf("vma_memory_read31846:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read31845;
  t5 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read31842;   

vma_memory_read31845:
  if (_trace) printf("vma_memory_read31845:\n");
  t1 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t1;   		// Adjust for a longword load 
  t1 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read31848:
  /* Perform memory action */
  arg1 = t1;
  arg2 = 6;
  goto performmemoryaction;

/* end Aset1RecomputeArrayRegister */
/* start Aset1Exception */


aset1exception:
  if (_trace) printf("aset1exception:\n");
  *(u64 *)&processor->asrf4 = arg2;   		// Just a place to save these values 
  *(u64 *)&processor->asrf3 = t5;   		// Just a place to save these values 
  *(u64 *)&processor->asrf6 = t6;   		// Just a place to save these values 
  *(u64 *)&processor->asrf5 = t7;   		// Just a place to save these values 
  t9 = t12;
  arg2 = arg3;
  arg1 = arg4;
  t4 = arg5;
  t3 = arg6;
  t2 = zero;
  iSP = iSP + 24;
  r0 = (u64)&&return0071;
  goto setup1dlongarray;
return0071:
  t1 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t1 != 0)   
    goto reallyaset1exc;
  arg2 = *(s32 *)&processor->asrf4;   		// Just a place to save these values 
  t5 = *(u64 *)&(processor->asrf3);   		// Just a place to save these values 
  t6 = *(u64 *)&(processor->asrf6);   		// Just a place to save these values 
  t7 = *(u64 *)&(processor->asrf5);   		// Just a place to save these values 
  t1 = *(s32 *)iSP;   		// Length 
  t2 = *(s32 *)(iSP + 4);   		// Length 
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  t2 = *(u64 *)iSP;   		// base 
  iSP = iSP - 8;   		// Pop Stack. 
  t3 = *(u64 *)iSP;   		// control 
  iSP = iSP - 8;   		// Pop Stack. 
  t9 = *(s32 *)iSP;   		// The original array 
  arg3 = *(s32 *)(iSP + 4);   		// The original array 
  iSP = iSP - 8;   		// Pop Stack. 
  t9 = (u32)t9;   
  iSP = iSP - 24;   
  *(u64 *)&((ARRAYCACHEP)t7)->length = t1;   
  *(u32 *)&((ARRAYCACHEP)t7)->arword = t3;   
  *(u32 *)&((ARRAYCACHEP)t7)->locat = t2;   
  *(u32 *)&((ARRAYCACHEP)t7)->array = t9;   		// store the array 
  t9 = (u32)t2;   
  t2 = ((u64)arg2 < (u64)t1) ? 1 : 0;   
  if (t2 == 0) 
    goto aset1bounds;
  arg5 = t3 >> (Array_BytePackingPos & 63);   		// get BP into arg5 
  arg6 = t3 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg4 = t3 >> (Array_RegisterByteOffsetPos & 63);   
  arg5 = arg5 & Array_BytePackingMask;
  arg4 = arg4 & Array_RegisterByteOffsetMask;
  arg6 = arg6 & Array_ElementTypeMask;
  goto aset1restart;   

reallyaset1exc:
  if (_trace) printf("reallyaset1exc:\n");
  arg1 = Type_Fixnum;
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 3;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 9;
  goto arrayexception;

aset1illegal:
  if (_trace) printf("aset1illegal:\n");
  arg5 = 0;
  arg2 = 9;
  goto illegaloperand;

aset1bounds:
  if (_trace) printf("aset1bounds:\n");
  *(u64 *)&((ARRAYCACHEP)t7)->array = zero;   
  arg5 = 0;
  arg2 = 74;
  goto illegaloperand;

/* end Aset1Exception */
/* start DoAloc1 */

  /* Halfword operand from stack instruction - DoAloc1 */
  /* arg2 has the preloaded 8 bit operand. */

doaloc1:
  if (_trace) printf("doaloc1:\n");

DoAloc1SP:
  if (_trace) printf("DoAloc1SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAloc1LP:
  if (_trace) printf("DoAloc1LP:\n");

DoAloc1FP:
  if (_trace) printf("DoAloc1FP:\n");

headdoaloc1:
  if (_trace) printf("headdoaloc1:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoaloc1:
  if (_trace) printf("begindoaloc1:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// Get the array tag/data 
  arg3 = *(s32 *)(iSP + 4);   		// Get the array tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = (u32)arg1;   		// Index Data 
  arg1 = arg1 >> 32;   		// Index Tag 
  t1 = arg1 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto aloc1illegal;

aloc1merge:
  if (_trace) printf("aloc1merge:\n");
  t1 = arg3 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto aloc1exception;
  /* Memory Read Internal */

vma_memory_read31852:
  t1 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = arg4 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  arg6 = (t3 * 4);   
  arg5 = LDQ_U(t3);   
  t1 = arg4 - t1;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t2) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read31854;

vma_memory_read31853:
  t3 = zero + 64;   
  t4 = t4 >> (arg5 & 63);   
  t3 = t3 >> (arg5 & 63);   
  if (t4 & 1)   
    goto vma_memory_read31856;

vma_memory_read31861:
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6 >> (Array_LongPrefixBitPos & 63);   
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto aloc1illegal;
  if (t2 & 1)   
    goto aloc1exception;
  t2 = zero + Array_LengthMask;   
  t1 = arg6 & t2;
  t3 = ((u64)arg2 < (u64)t1) ? 1 : 0;   
  if (t3 == 0) 
    goto aloc1illegal;
  arg6 = arg6 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg4 = arg4 + 1;
  arg4 = arg4 + arg2;
  arg6 = arg6 & Array_ElementTypeMask;
  arg6 = arg6 - Array_ElementTypeObject;   
  if (arg6 != 0)   
    goto aloc1notobject;
  t1 = Type_Locative;
  *(u32 *)(iSP + 8) = arg4;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

aloc1exception:
  if (_trace) printf("aloc1exception:\n");
  arg1 = Type_Fixnum;
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 8;
  goto arrayexception;

aloc1illegal:
  if (_trace) printf("aloc1illegal:\n");
  arg5 = 0;
  arg2 = 8;
  goto illegaloperand;

aloc1bounds:
  if (_trace) printf("aloc1bounds:\n");
  arg5 = 0;
  arg2 = 74;
  goto illegaloperand;

aloc1notobject:
  if (_trace) printf("aloc1notobject:\n");
  arg5 = 0;
  arg2 = 7;
  goto illegaloperand;

DoAloc1IM:
  if (_trace) printf("DoAloc1IM:\n");
  arg4 = *(s32 *)iSP;   		// Get the array tag/data 
  arg3 = *(s32 *)(iSP + 4);   		// Get the array tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  goto aloc1merge;   

vma_memory_read31854:
  if (_trace) printf("vma_memory_read31854:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg6 = *(s32 *)t1;   
  arg5 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read31853;   

vma_memory_read31856:
  if (_trace) printf("vma_memory_read31856:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read31855;
  arg4 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31852;   

vma_memory_read31855:
  if (_trace) printf("vma_memory_read31855:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read31858:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 6;
  goto performmemoryaction;

/* end DoAloc1 */
  /* End of Halfword operand from stack instruction - DoAloc1 */
  /* Array register operations. */
/* start DoSetup1DArray */

  /* Halfword operand from stack instruction - DoSetup1DArray */

dosetup1darray:
  if (_trace) printf("dosetup1darray:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoSetup1DArrayIM:
  if (_trace) printf("DoSetup1DArrayIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment31875:
  if (_trace) printf("force_alignment31875:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindosetup1darray;   

DoSetup1DArraySP:
  if (_trace) printf("DoSetup1DArraySP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetup1DArrayLP:
  if (_trace) printf("DoSetup1DArrayLP:\n");

DoSetup1DArrayFP:
  if (_trace) printf("DoSetup1DArrayFP:\n");

headdosetup1darray:
  if (_trace) printf("headdosetup1darray:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindosetup1darray:
  if (_trace) printf("begindosetup1darray:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = arg1 >> 32;   		// Get the tag 
  arg1 = (u32)arg1;   		// and the data 
  t2 = 0;		// Indicate not forcing 1d 
  t9 = arg1;
  t3 = arg2 - Type_Array;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto setup_array_register31863;
  /* Memory Read Internal */

vma_memory_read31865:
  t5 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t7 = arg1 + ivory;
  t6 = *(s32 *)&processor->scovlimit;   
  t3 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = arg1 - t5;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31867;

vma_memory_read31866:
  t7 = zero + 64;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  t3 = (u32)t3;   
  if (t8 & 1)   
    goto vma_memory_read31869;

vma_memory_read31874:
  /* TagType. */
  t5 = t4 & 63;
  t6 = t3 >> (Array_LongPrefixBitPos & 63);   
  t5 = t5 - Type_HeaderI;   
  if (t5 != 0)   
    goto setup_array_register31862;
  if (t6 & 1)   
    goto setup_array_register31864;
  t5 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = t3 >> (Array_RegisterBytePackingPos & 63);   
  t7 = Type_Fixnum;
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t5 = arg1 + 1;
  t8 = t8 + t1;		// Construct the array register word 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;   
  *(u32 *)(iSP + 12) = t6;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  t6 = zero + Array_LengthMask;   
  t6 = t3 & t6;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t6;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

setup_array_register31863:
  if (_trace) printf("setup_array_register31863:\n");
  /* SetTag. */
  t6 = arg2 << 32;   
  t6 = t9 | t6;
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 71;
  goto arrayexception;

setup_array_register31862:
  if (_trace) printf("setup_array_register31862:\n");
  arg5 = 0;
  arg2 = 71;
  goto illegaloperand;

setup_array_register31864:
  if (_trace) printf("setup_array_register31864:\n");
  r0 = (u64)&&return0072;
  goto setup1dlongarray;
return0072:
  t1 = (t2 == ReturnValue_Normal) ? 1 : 0;   
  if (t1 != 0)   
    goto NEXTINSTRUCTION;
  t1 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register31863;
  t1 = (t2 == ReturnValue_IllegalOperand) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register31862;
  goto NEXTINSTRUCTION;   

vma_memory_read31867:
  if (_trace) printf("vma_memory_read31867:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t3 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31866;   

vma_memory_read31869:
  if (_trace) printf("vma_memory_read31869:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31868;
  arg1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read31865;   

vma_memory_read31868:
  if (_trace) printf("vma_memory_read31868:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31871:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end DoSetup1DArray */
  /* End of Halfword operand from stack instruction - DoSetup1DArray */
/* start DoSetupForce1DArray */

  /* Halfword operand from stack instruction - DoSetupForce1DArray */

dosetupforce1darray:
  if (_trace) printf("dosetupforce1darray:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoSetupForce1DArrayIM:
  if (_trace) printf("DoSetupForce1DArrayIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment31889:
  if (_trace) printf("force_alignment31889:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindosetupforce1darray;   

DoSetupForce1DArraySP:
  if (_trace) printf("DoSetupForce1DArraySP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetupForce1DArrayLP:
  if (_trace) printf("DoSetupForce1DArrayLP:\n");

DoSetupForce1DArrayFP:
  if (_trace) printf("DoSetupForce1DArrayFP:\n");

headdosetupforce1darray:
  if (_trace) printf("headdosetupforce1darray:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindosetupforce1darray:
  if (_trace) printf("begindosetupforce1darray:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = arg1 >> 32;   		// Get the tag 
  arg1 = (u32)arg1;   		// and the data 
  t2 = 1;		// Indicate forcing 1d 
  t9 = arg1;
  t3 = arg2 - Type_Array;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto setup_array_register31877;
  /* Memory Read Internal */

vma_memory_read31879:
  t5 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t7 = arg1 + ivory;
  t6 = *(s32 *)&processor->scovlimit;   
  t3 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = arg1 - t5;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read31881;

vma_memory_read31880:
  t7 = zero + 64;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  t3 = (u32)t3;   
  if (t8 & 1)   
    goto vma_memory_read31883;

vma_memory_read31888:
  /* TagType. */
  t5 = t4 & 63;
  t6 = t3 >> (Array_LongPrefixBitPos & 63);   
  t5 = t5 - Type_HeaderI;   
  if (t5 != 0)   
    goto setup_array_register31876;
  if (t6 & 1)   
    goto setup_array_register31878;
  t5 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = t3 >> (Array_RegisterBytePackingPos & 63);   
  t7 = Type_Fixnum;
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t5 = arg1 + 1;
  t8 = t8 + t1;		// Construct the array register word 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;   
  *(u32 *)(iSP + 12) = t6;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  t6 = zero + Array_LengthMask;   
  t6 = t3 & t6;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t6;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

setup_array_register31877:
  if (_trace) printf("setup_array_register31877:\n");
  /* SetTag. */
  t6 = arg2 << 32;   
  t6 = t9 | t6;
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 71;
  goto arrayexception;

setup_array_register31876:
  if (_trace) printf("setup_array_register31876:\n");
  arg5 = 0;
  arg2 = 71;
  goto illegaloperand;

setup_array_register31878:
  if (_trace) printf("setup_array_register31878:\n");
  r0 = (u64)&&return0073;
  goto setup1dlongarray;
return0073:
  t1 = (t2 == ReturnValue_Normal) ? 1 : 0;   
  if (t1 != 0)   
    goto NEXTINSTRUCTION;
  t1 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register31877;
  t1 = (t2 == ReturnValue_IllegalOperand) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register31876;
  goto NEXTINSTRUCTION;   

vma_memory_read31881:
  if (_trace) printf("vma_memory_read31881:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t3 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read31880;   

vma_memory_read31883:
  if (_trace) printf("vma_memory_read31883:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read31882;
  arg1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read31879;   

vma_memory_read31882:
  if (_trace) printf("vma_memory_read31882:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read31885:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end DoSetupForce1DArray */
  /* End of Halfword operand from stack instruction - DoSetupForce1DArray */
/* start Setup1DLongArray */


setup1dlongarray:
  if (_trace) printf("setup1dlongarray:\n");
  /* Read data from the header: alength offset indirect lengths&mults */
  t1 = arg1 + 1;   		// length=array+1 
  /* Memory Read Internal */

vma_memory_read31899:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  arg4 = (t10 * 4);   
  t6 = LDQ_U(t10);   
  t7 = t1 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  arg4 = *(s32 *)arg4;   
  t6 = (u8)(t6 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31901;

vma_memory_read31900:
  t10 = zero + 240;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  arg4 = (u32)arg4;   
  if (t11 & 1)   
    goto vma_memory_read31903;

vma_memory_read31910:
  t8 = t6 - Type_Fixnum;   
  t8 = t8 & 63;		// Strip CDR code 
  if (t8 != 0)   
    goto setup_long_array_register31890;
  t1 = t1 + 1;   		// Offset is adata+2 
  /* Memory Read Internal */

vma_memory_read31911:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  arg3 = (t10 * 4);   
  t6 = LDQ_U(t10);   
  t7 = t1 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  arg3 = *(s32 *)arg3;   
  t6 = (u8)(t6 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31913;

vma_memory_read31912:
  t10 = zero + 240;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  arg3 = (u32)arg3;   
  if (t11 & 1)   
    goto vma_memory_read31915;

vma_memory_read31922:
  t8 = t6 - Type_Fixnum;   
  t8 = t8 & 63;		// Strip CDR code 
  if (t8 != 0)   
    goto setup_long_array_register31890;
  t1 = t1 + 1;   		// Indirect is adata+3 
  /* Memory Read Internal */

vma_memory_read31923:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t5 = (t10 * 4);   
  t6 = LDQ_U(t10);   
  t7 = t1 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t6 = (u8)(t6 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31925;

vma_memory_read31924:
  t10 = zero + 240;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  t5 = (u32)t5;   
  if (t11 & 1)   
    goto vma_memory_read31927;

vma_memory_read31934:
  t10 = t6 & 63;		// Strip off any CDR code bits. 
  t11 = (t10 == Type_Locative) ? 1 : 0;   

force_alignment31999:
  if (_trace) printf("force_alignment31999:\n");
  if (t11 == 0) 
    goto basic_dispatch31936;
  /* Here if argument TypeLocative */

setup_long_array_register31893:
  if (_trace) printf("setup_long_array_register31893:\n");
  t10 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t10;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = t3 >> (Array_BytePackingPos & 63);   
  t7 = Type_Fixnum;
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t8 = t8 + t1;		// Construct the array register word 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;   
  *(u32 *)(iSP + 12) = t6;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg4;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto setup_long_array_register31898;   

basic_dispatch31936:
  if (_trace) printf("basic_dispatch31936:\n");
  t11 = (t10 == Type_Fixnum) ? 1 : 0;   

force_alignment32000:
  if (_trace) printf("force_alignment32000:\n");
  if (t11 == 0) 
    goto basic_dispatch31937;
  /* Here if argument TypeFixnum */
  goto setup_long_array_register31893;   

basic_dispatch31937:
  if (_trace) printf("basic_dispatch31937:\n");
  t11 = (t10 == Type_Array) ? 1 : 0;   

force_alignment32001:
  if (_trace) printf("force_alignment32001:\n");
  if (t11 == 0) 
    goto basic_dispatch31938;
  /* Here if argument TypeArray */

setup_long_array_register31897:
  if (_trace) printf("setup_long_array_register31897:\n");
  t1 = t3 & 7;
  t1 = (t1 == 1) ? 1 : 0;   
  t1 = t1 | t2;		// Force true if FORCE 
  if (t1 == 0) 
    goto setup_long_array_register31890;
  t12 = t3 >> (Array_BytePackingPos & 63);   
  t12 = t12 & Array_BytePackingMask;
  t2 = arg3;

setup_long_array_register31892:
  if (_trace) printf("setup_long_array_register31892:\n");
  /* Memory Read Internal */

vma_memory_read31939:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t5 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t4 = (t10 * 4);   
  t6 = LDQ_U(t10);   
  t7 = t5 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  t4 = *(s32 *)t4;   
  t6 = (u8)(t6 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31941;

vma_memory_read31940:
  t10 = zero + 64;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  t4 = (u32)t4;   
  if (t11 & 1)   
    goto vma_memory_read31943;

vma_memory_read31948:
  t10 = t4 >> (Array_BytePackingPos & 63);   
  t10 = t10 & Array_BytePackingMask;
  arg1 = t12 - t10;   
  t7 = t4 >> (Array_LongPrefixBitPos & 63);   
  if (t7 & 1)   
    goto setup_long_array_register31894;
  t5 = t5 + 1;		// increment beyond header 
  t8 = zero + 32767;   
  t8 = t4 & t8;
  t10 = zero - arg1;   
  t10 = t8 >> (t10 & 63);   
  t8 = t8 << (arg1 & 63);   
  if ((s64)arg1 <= 0)   
    t8 = t10;
  t10 = arg4 + arg3;
  t7 = t10 - t8;   
  if ((s64)t7 <= 0)   
    t8 = t10;
  arg4 = t8;

setup_long_array_register31891:
  if (_trace) printf("setup_long_array_register31891:\n");
  arg4 = arg4 - t2;   
  t10 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;   
  *(u32 *)(iSP + 12) = t10;   		// write the stack cache 
  iSP = iSP + 8;
  t7 = Type_Fixnum;
  t8 = t3 >> (Array_RegisterBytePackingPos & 63);   
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t11 = zero - 1;   		// -1 
  t11 = t11 << (t12 & 63);   		// (LSH -1 byte-packing) 
  t11 = t2 & ~t11;
  t11 = t11 << (Array_RegisterByteOffsetPos & 63);   
  t8 = t8 + t1;		// Construct the array register word 
  t8 = t11 + t8;		// Add in the byte offset 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;   
  *(u32 *)(iSP + 12) = t6;   		// write the stack cache 
  iSP = iSP + 8;
  if ((s64)arg4 <= 0)   
    arg4 = zero;
  if (arg4 == 0) 
    goto setup_long_array_register31895;
  t1 = zero - t12;   
  t1 = t2 << (t1 & 63);   
  t2 = t2 >> (t12 & 63);   
  if ((s64)t12 <= 0)   
    t2 = t1;
  t5 = t2 + t5;

setup_long_array_register31895:
  if (_trace) printf("setup_long_array_register31895:\n");
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg4;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto setup_long_array_register31898;   

setup_long_array_register31894:
  if (_trace) printf("setup_long_array_register31894:\n");
  t1 = t5 + 1;		// length=array+1 
  /* Memory Read Internal */

vma_memory_read31949:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  arg6 = (t10 * 4);   
  t4 = LDQ_U(t10);   
  t7 = t1 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  t4 = (u8)(t4 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31951;

vma_memory_read31950:
  t10 = zero + 240;   
  t11 = t11 >> (t4 & 63);   
  t10 = t10 >> (t4 & 63);   
  arg6 = (u32)arg6;   
  if (t11 & 1)   
    goto vma_memory_read31953;

vma_memory_read31960:
  t1 = t4 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto setup_long_array_register31890;
  t1 = t5 + 2;		// offset=array+2 
  /* Memory Read Internal */

vma_memory_read31961:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  arg5 = (t10 * 4);   
  t4 = LDQ_U(t10);   
  t7 = t1 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  t4 = (u8)(t4 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31963;

vma_memory_read31962:
  t10 = zero + 240;   
  t11 = t11 >> (t4 & 63);   
  t10 = t10 >> (t4 & 63);   
  arg5 = (u32)arg5;   
  if (t11 & 1)   
    goto vma_memory_read31965;

vma_memory_read31972:
  t1 = t4 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto setup_long_array_register31890;
  t1 = t5 + 3;		// next=array+3 
  /* Memory Read Internal */

vma_memory_read31973:
  t7 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t5 = (t10 * 4);   
  t4 = LDQ_U(t10);   
  t7 = t1 - t7;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)t8) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t10&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read31975;

vma_memory_read31974:
  t10 = zero + 240;   
  t11 = t11 >> (t4 & 63);   
  t10 = t10 >> (t4 & 63);   
  t5 = (u32)t5;   
  if (t11 & 1)   
    goto vma_memory_read31977;

vma_memory_read31984:
  t8 = zero - arg1;   
  t8 = arg6 >> (t8 & 63);   
  t10 = arg6 << (arg1 & 63);   
  if ((s64)arg1 <= 0)   
    t10 = t8;
  t8 = arg4 + arg3;
  if ((s64)t10 <= 0)   
    t10 = t8;
  t7 = t10 - t8;   
  if ((s64)t7 <= 0)   
    t8 = t10;
  arg4 = t8;
  t8 = t4 & 63;		// Strip off any CDR code bits. 
  t10 = (t8 == Type_Locative) ? 1 : 0;   

force_alignment31992:
  if (_trace) printf("force_alignment31992:\n");
  if (t10 == 0) 
    goto basic_dispatch31986;
  /* Here if argument TypeLocative */
  goto setup_long_array_register31891;   

basic_dispatch31986:
  if (_trace) printf("basic_dispatch31986:\n");
  t10 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment31993:
  if (_trace) printf("force_alignment31993:\n");
  if (t10 == 0) 
    goto basic_dispatch31987;
  /* Here if argument TypeFixnum */
  goto setup_long_array_register31891;   

basic_dispatch31987:
  if (_trace) printf("basic_dispatch31987:\n");
  t10 = (t8 == Type_Array) ? 1 : 0;   

force_alignment31994:
  if (_trace) printf("force_alignment31994:\n");
  if (t10 == 0) 
    goto basic_dispatch31988;
  /* Here if argument TypeArray */

setup_long_array_register31896:
  if (_trace) printf("setup_long_array_register31896:\n");
  t7 = zero - arg1;   
  t7 = arg5 >> (t7 & 63);   
  arg3 = arg5 << (arg1 & 63);   
  if ((s64)arg1 <= 0)   
    arg3 = t7;
  t2 = t2 + arg3;
  goto setup_long_array_register31892;   

basic_dispatch31988:
  if (_trace) printf("basic_dispatch31988:\n");
  t10 = (t8 == Type_String) ? 1 : 0;   

force_alignment31995:
  if (_trace) printf("force_alignment31995:\n");
  if (t10 == 0) 
    goto basic_dispatch31989;
  /* Here if argument TypeString */
  goto setup_long_array_register31896;   

basic_dispatch31989:
  if (_trace) printf("basic_dispatch31989:\n");
  /* Here for all other cases */
  goto setup_long_array_register31890;   

basic_dispatch31985:
  if (_trace) printf("basic_dispatch31985:\n");

basic_dispatch31938:
  if (_trace) printf("basic_dispatch31938:\n");
  t11 = (t10 == Type_String) ? 1 : 0;   

force_alignment32002:
  if (_trace) printf("force_alignment32002:\n");
  if (t11 == 0) 
    goto basic_dispatch31996;
  /* Here if argument TypeString */
  goto setup_long_array_register31897;   

basic_dispatch31996:
  if (_trace) printf("basic_dispatch31996:\n");
  /* Here for all other cases */
  goto setup_long_array_register31890;   

basic_dispatch31935:
  if (_trace) printf("basic_dispatch31935:\n");

setup_long_array_register31890:
  if (_trace) printf("setup_long_array_register31890:\n");
  t2 = ReturnValue_Exception;
  goto *r0; /* ret */

setup_long_array_register31898:
  if (_trace) printf("setup_long_array_register31898:\n");
  t2 = ReturnValue_Normal;
  goto *r0; /* ret */

vma_memory_read31975:
  if (_trace) printf("vma_memory_read31975:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t5 = *(s32 *)t7;   
  t4 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31974;   

vma_memory_read31977:
  if (_trace) printf("vma_memory_read31977:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31976;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read31973;   

vma_memory_read31976:
  if (_trace) printf("vma_memory_read31976:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31981:
  if (_trace) printf("vma_memory_read31981:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read31980;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read31984;   

vma_memory_read31980:

vma_memory_read31979:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31963:
  if (_trace) printf("vma_memory_read31963:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg5 = *(s32 *)t7;   
  t4 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31962;   

vma_memory_read31965:
  if (_trace) printf("vma_memory_read31965:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31964;
  t1 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read31961;   

vma_memory_read31964:
  if (_trace) printf("vma_memory_read31964:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31969:
  if (_trace) printf("vma_memory_read31969:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read31968;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read31972;   

vma_memory_read31968:

vma_memory_read31967:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31951:
  if (_trace) printf("vma_memory_read31951:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg6 = *(s32 *)t7;   
  t4 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31950;   

vma_memory_read31953:
  if (_trace) printf("vma_memory_read31953:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31952;
  t1 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read31949;   

vma_memory_read31952:
  if (_trace) printf("vma_memory_read31952:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31957:
  if (_trace) printf("vma_memory_read31957:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read31956;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read31960;   

vma_memory_read31956:

vma_memory_read31955:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31941:
  if (_trace) printf("vma_memory_read31941:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t4 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31940;   

vma_memory_read31943:
  if (_trace) printf("vma_memory_read31943:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31942;
  t5 = (u32)t4;   		// Do the indirect thing 
  goto vma_memory_read31939;   

vma_memory_read31942:
  if (_trace) printf("vma_memory_read31942:\n");
  t11 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31945:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 6;
  goto performmemoryaction;

vma_memory_read31925:
  if (_trace) printf("vma_memory_read31925:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t5 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31924;   

vma_memory_read31927:
  if (_trace) printf("vma_memory_read31927:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31926;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read31923;   

vma_memory_read31926:
  if (_trace) printf("vma_memory_read31926:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31931:
  if (_trace) printf("vma_memory_read31931:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read31930;
  t6 = t6 & ~63L;
  t6 = t6 | Type_ExternalValueCellPointer;
  goto vma_memory_read31934;   

vma_memory_read31930:

vma_memory_read31929:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31913:
  if (_trace) printf("vma_memory_read31913:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg3 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31912;   

vma_memory_read31915:
  if (_trace) printf("vma_memory_read31915:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31914;
  t1 = (u32)arg3;   		// Do the indirect thing 
  goto vma_memory_read31911;   

vma_memory_read31914:
  if (_trace) printf("vma_memory_read31914:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31919:
  if (_trace) printf("vma_memory_read31919:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read31918;
  t6 = t6 & ~63L;
  t6 = t6 | Type_ExternalValueCellPointer;
  goto vma_memory_read31922;   

vma_memory_read31918:

vma_memory_read31917:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read31901:
  if (_trace) printf("vma_memory_read31901:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg4 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read31900;   

vma_memory_read31903:
  if (_trace) printf("vma_memory_read31903:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read31902;
  t1 = (u32)arg4;   		// Do the indirect thing 
  goto vma_memory_read31899;   

vma_memory_read31902:
  if (_trace) printf("vma_memory_read31902:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read31907:
  if (_trace) printf("vma_memory_read31907:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read31906;
  t6 = t6 & ~63L;
  t6 = t6 | Type_ExternalValueCellPointer;
  goto vma_memory_read31910;   

vma_memory_read31906:

vma_memory_read31905:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

/* end Setup1DLongArray */
/* start DoFastAset1 */

  /* Halfword operand from stack instruction - DoFastAset1 */
  /* arg2 has the preloaded 8 bit operand. */

dofastaset1:
  if (_trace) printf("dofastaset1:\n");

DoFastAset1SP:
  if (_trace) printf("DoFastAset1SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoFastAset1LP:
  if (_trace) printf("DoFastAset1LP:\n");

DoFastAset1FP:
  if (_trace) printf("DoFastAset1FP:\n");

begindofastaset1:
  if (_trace) printf("begindofastaset1:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg4 = *(s32 *)iSP;   		// Index 
  arg3 = *(s32 *)(iSP + 4);   		// Index 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  t11 = *(s32 *)iSP;   		// value 
  t10 = *(s32 *)(iSP + 4);   		// value 
  iSP = iSP - 8;   		// Pop Stack. 
  t11 = (u32)t11;   
  t1 = arg3 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto fastaset1iop;

fastaset1retry:
  if (_trace) printf("fastaset1retry:\n");
  arg6 = *(s32 *)arg1;   
  t9 = *(s32 *)(arg1 + 8);   
  t3 = *(s32 *)(arg1 + 16);   
  arg6 = (u32)arg6;   
  t9 = (u32)t9;   
  t5 = arg6 << 42;   
  t3 = (u32)t3;   
  t4 = *(u64 *)&(processor->areventcount);   
  t5 = t5 >> 42;   
  t2 = ((u64)arg4 < (u64)t3) ? 1 : 0;   
  if (t2 == 0) 
    goto fastaset1bounds;
  t6 = t4 - t5;   
  if (t6 != 0)   
    goto aset1recomputearrayregister;
  t6 = arg6 >> (Array_RegisterBytePackingPos & 63);   
  t7 = arg6 >> (Array_RegisterByteOffsetPos & 63);   
  t8 = arg6 >> (Array_RegisterElementTypePos & 63);   
  t6 = t6 & Array_RegisterBytePackingMask;
  t7 = t7 & Array_RegisterByteOffsetMask;
  t8 = t8 & Array_RegisterElementTypeMask;
  /* Element checking and foreplay. */
  /* TagType. */
  t1 = t10 & 63;
  t12 = (t8 == Array_ElementTypeCharacter) ? 1 : 0;   

force_alignment32013:
  if (_trace) printf("force_alignment32013:\n");
  if (t12 == 0) 
    goto basic_dispatch32009;
  /* Here if argument ArrayElementTypeCharacter */
  t2 = t1 - Type_Character;   
  if (t2 == 0) 
    goto aset_1_internal32004;
  arg5 = 0;
  arg2 = 29;
  goto illegaloperand;

aset_1_internal32004:
  if (_trace) printf("aset_1_internal32004:\n");
  if (t6 == 0) 		// Certainly will fit if not packed! 
    goto aset_1_internal32003;
  t2 = 32;
  t2 = t2 >> (t6 & 63);   		// Compute size of byte 
  t1 = ~zero;   
  t1 = t1 << (t2 & 63);   
  t1 = ~t1;   		// Compute mask for byte 
  t1 = t11 & t1;
  t1 = t11 - t1;   
  if (t1 == 0) 		// J. if character fits. 
    goto aset_1_internal32003;
  arg5 = 0;
  arg2 = 62;
  goto illegaloperand;

basic_dispatch32009:
  if (_trace) printf("basic_dispatch32009:\n");
  t12 = (t8 == Array_ElementTypeFixnum) ? 1 : 0;   

force_alignment32014:
  if (_trace) printf("force_alignment32014:\n");
  if (t12 == 0) 
    goto basic_dispatch32010;
  /* Here if argument ArrayElementTypeFixnum */
  t2 = t1 - Type_Fixnum;   
  if (t2 == 0) 
    goto aset_1_internal32003;
  arg5 = 0;
  arg2 = 33;
  goto illegaloperand;

basic_dispatch32010:
  if (_trace) printf("basic_dispatch32010:\n");
  t12 = (t8 == Array_ElementTypeBoolean) ? 1 : 0;   

force_alignment32015:
  if (_trace) printf("force_alignment32015:\n");
  if (t12 == 0) 
    goto basic_dispatch32008;
  /* Here if argument ArrayElementTypeBoolean */
  t11 = 1;
  t1 = t1 - Type_NIL;   
  if (t1 != 0)   		// J. if True 
    goto aset_1_internal32003;
  t11 = zero;
  goto aset_1_internal32003;   		// J. if False 

basic_dispatch32008:
  if (_trace) printf("basic_dispatch32008:\n");
  /* Shove it in. */

aset_1_internal32003:
  if (_trace) printf("aset_1_internal32003:\n");
  if (t6 != 0)   		// J. if packed 
    goto aset_1_internal32005;
  t1 = t8 - Array_ElementTypeObject;   
  if (t1 != 0)   
    goto aset_1_internal32005;
  /* Here for the simple non packed case */
  t1 = t9 + arg4;
  /* Memory Read Internal */

vma_memory_read32016:
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t12 = t1 + ivory;
  t5 = *(s32 *)&processor->scovlimit;   
  t3 = (t12 * 4);   
  t2 = LDQ_U(t12);   
  t4 = t1 - t4;   		// Stack cache offset 
  arg3 = *(u64 *)&(processor->datawrite_mask);   
  t5 = ((u64)t4 < (u64)t5) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t12&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read32018;

vma_memory_read32017:
  t12 = zero + 240;   
  arg3 = arg3 >> (t2 & 63);   
  t12 = t12 >> (t2 & 63);   
  if (arg3 & 1)   
    goto vma_memory_read32020;

vma_memory_read32026:
  /* Merge cdr-code */
  t3 = t10 & 63;
  t2 = t2 & 192;
  t2 = t2 | t3;
  t5 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t1 + ivory;
  arg3 = *(s32 *)&processor->scovlimit;   
  t3 = (t4 * 4);   
  t12 = LDQ_U(t4);   
  t5 = t1 - t5;   		// Stack cache offset 
  arg3 = ((u64)t5 < (u64)arg3) ? 1 : 0;   		// In range? 
  t5 = (t2 & 0xff) << ((t4&7)*8);   
  t12 = t12 & ~(0xffL << (t4&7)*8);   

force_alignment32028:
  if (_trace) printf("force_alignment32028:\n");
  t12 = t12 | t5;
  STQ_U(t4, t12);   
  *(u32 *)t3 = t11;   
  if (arg3 != 0)   		// J. if in cache 
    goto vma_memory_write32027;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   
  /* Here for the slow packed version */

aset_1_internal32005:
  if (_trace) printf("aset_1_internal32005:\n");
  arg4 = t7 + arg4;
  t1 = arg4 >> (t6 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  /* Memory Read Internal */

vma_memory_read32029:
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t4 = t1 + ivory;
  t3 = *(s32 *)&processor->scovlimit;   
  t9 = (t4 * 4);   
  arg5 = LDQ_U(t4);   
  t2 = t1 - t2;   		// Stack cache offset 
  t5 = *(u64 *)&(processor->dataread_mask);   
  t3 = ((u64)t2 < (u64)t3) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  arg5 = (u8)(arg5 >> ((t4&7)*8));   
  if (t3 != 0)   
    goto vma_memory_read32031;

vma_memory_read32030:
  t4 = zero + 240;   
  t5 = t5 >> (arg5 & 63);   
  t4 = t4 >> (arg5 & 63);   
  t9 = (u32)t9;   
  if (t5 & 1)   
    goto vma_memory_read32033;

vma_memory_read32040:
  /* Check fixnum element type */
  /* TagType. */
  t2 = arg5 & 63;
  t2 = t2 - Type_Fixnum;   
  if (t2 != 0)   		// J. if element type not fixnum. 
    goto aset_1_internal32006;
  if (t6 == 0) 		// J. if unpacked fixnum element type. 
    goto aset_1_internal32007;
  t12 = ~zero;   
  t12 = t12 << (t6 & 63);   
  t2 = zero - t6;   
  t12 = arg4 & ~t12;		// Compute subword index 
  t2 = t2 + 5;
  t2 = t12 << (t2 & 63);   		// Compute shift to get byte 
  t12 = 32;
  t12 = t12 >> (t6 & 63);   		// Compute size of byte 
  t3 = ~zero;   
  t3 = t3 << (t12 & 63);   
  t4 = ~t3;   		// Compute mask for byte 
  if (t2 == 0) 		// inserting into the low byte is easy 
    goto array_element_dpb32041;
  /* Inserting the byte into any byte other than the low byte */
  t5 = 64;
  t12 = t5 - t2;   		// = the left shift rotate amount 
  t5 = t9 >> (t2 & 63);   		// shift selected byte into low end of word. 
  t9 = t9 << (t12 & 63);   		// rotate low bits into high end of word. 
  t5 = t3 & t5;		// Remove unwanted bits 
  t9 = t9 >> (t12 & 63);   		// rotate low bits back into place. 
  t12 = t11 & t4;		// Strip any extra bits from element 
  t5 = t12 | t5;		// Insert new bits. 
  t5 = t5 << (t2 & 63);   		// reposition bits 
  t9 = t9 | t5;		// Replace low order bits 
  goto array_element_dpb32042;   

array_element_dpb32041:
  if (_trace) printf("array_element_dpb32041:\n");
  /* Inserting the byte into the low byte */
  t9 = t9 & t3;		// Remove the old low byte 
  t12 = t11 & t4;		// Remove unwanted bits from the new byte 
  t9 = t9 | t12;		// Insert the new byte in place of the old byte 

array_element_dpb32042:
  if (_trace) printf("array_element_dpb32042:\n");
  t11 = t9;

aset_1_internal32007:
  if (_trace) printf("aset_1_internal32007:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = t1 + ivory;
  t12 = *(s32 *)&processor->scovlimit;   
  t5 = (t2 * 4);   
  t4 = LDQ_U(t2);   
  t3 = t1 - t3;   		// Stack cache offset 
  t12 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  t3 = (arg5 & 0xff) << ((t2&7)*8);   
  t4 = t4 & ~(0xffL << (t2&7)*8);   

force_alignment32044:
  if (_trace) printf("force_alignment32044:\n");
  t4 = t4 | t3;
  STQ_U(t2, t4);   
  *(u32 *)t5 = t11;   
  if (t12 != 0)   		// J. if in cache 
    goto vma_memory_write32043;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

aset_1_internal32006:
  if (_trace) printf("aset_1_internal32006:\n");
  arg5 = t1;
  arg2 = 25;
  goto illegaloperand;

fastaset1iop:
  if (_trace) printf("fastaset1iop:\n");
  arg5 = 0;
  arg2 = 32;
  goto illegaloperand;

fastaset1bounds:
  if (_trace) printf("fastaset1bounds:\n");
  arg5 = 0;
  arg2 = 13;
  goto illegaloperand;

vma_memory_write32043:
  if (_trace) printf("vma_memory_write32043:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32045:
  if (_trace) printf("force_alignment32045:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t3 = t1 - t3;   		// Stack cache offset 
  t2 = (t3 * 8) + t2;  		// reconstruct SCA 
  *(u32 *)t2 = t11;   		// Store in stack 
  *(u32 *)(t2 + 4) = arg5;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32031:
  if (_trace) printf("vma_memory_read32031:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg5 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read32030;   

vma_memory_read32033:
  if (_trace) printf("vma_memory_read32033:\n");
  if ((t4 & 1) == 0)   
    goto vma_memory_read32032;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read32029;   

vma_memory_read32032:
  if (_trace) printf("vma_memory_read32032:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t4 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t4 = (t4 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t4;   		// Get the memory action 

vma_memory_read32037:
  if (_trace) printf("vma_memory_read32037:\n");
  t4 = t5 & MemoryActionTransform;
  if (t4 == 0) 
    goto vma_memory_read32036;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read32040;   

vma_memory_read32036:

vma_memory_read32035:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_write32027:
  if (_trace) printf("vma_memory_write32027:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32046:
  if (_trace) printf("force_alignment32046:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t1 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  *(u32 *)t4 = t11;   		// Store in stack 
  *(u32 *)(t4 + 4) = t2;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32018:
  if (_trace) printf("vma_memory_read32018:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t3 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read32017;   

vma_memory_read32020:
  if (_trace) printf("vma_memory_read32020:\n");
  if ((t12 & 1) == 0)   
    goto vma_memory_read32019;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32016;   

vma_memory_read32019:
  if (_trace) printf("vma_memory_read32019:\n");
  arg3 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t12 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t12 = (t12 * 4) + arg3;   		// Adjust for a longword load 
  arg3 = *(s32 *)t12;   		// Get the memory action 

vma_memory_read32023:

vma_memory_read32022:
  /* Perform memory action */
  arg1 = arg3;
  arg2 = 1;
  goto performmemoryaction;

DoFastAset1IM:
  goto doistageerror;

/* end DoFastAset1 */
  /* End of Halfword operand from stack instruction - DoFastAset1 */
  /* Array leaders. */
/* start DoArrayLeader */

  /* Halfword operand from stack instruction - DoArrayLeader */
  /* arg2 has the preloaded 8 bit operand. */

doarrayleader:
  if (_trace) printf("doarrayleader:\n");

DoArrayLeaderSP:
  if (_trace) printf("DoArrayLeaderSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoArrayLeaderLP:
  if (_trace) printf("DoArrayLeaderLP:\n");

DoArrayLeaderFP:
  if (_trace) printf("DoArrayLeaderFP:\n");

headdoarrayleader:
  if (_trace) printf("headdoarrayleader:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoarrayleader:
  if (_trace) printf("begindoarrayleader:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// arg3=arraytag, arg4=arraydata 
  arg3 = *(s32 *)(iSP + 4);   		// arg3=arraytag, arg4=arraydata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = (u32)arg1;   		// index data 
  arg1 = arg1 >> 32;   		// index tag 
  t1 = arg1 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto arrayleaderiop;

arrayleadermerge:
  if (_trace) printf("arrayleadermerge:\n");
  t1 = arg3 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto arrayleaderexception;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read32047:
  t3 = arg4 + ivory;
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg4 - t11;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t12) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read32049;

vma_memory_read32048:
  t3 = zero + 64;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read32051;

vma_memory_read32056:
  /* TagType. */
  t1 = arg6 & 63;
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto arrayleaderiop;
  t8 = arg5 >> (Array_LeaderLengthFieldPos & 63);   
  t8 = t8 & Array_LeaderLengthFieldMask;
  t1 = ((u64)arg2 < (u64)t8) ? 1 : 0;   
  if (t1 == 0) 
    goto arrayleaderbounds;
  arg2 = arg4 - arg2;   
  arg2 = arg2 - 1;   
  /* Memory Read Internal */

vma_memory_read32057:
  t3 = arg2 + ivory;
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg2 - t11;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->dataread_mask);   
  t2 = ((u64)t1 < (u64)t12) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read32059;

vma_memory_read32058:
  t3 = zero + 240;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read32061;

vma_memory_read32068:
  t1 = arg6 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg5;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

arrayleaderexception:
  if (_trace) printf("arrayleaderexception:\n");
  arg1 = Type_Fixnum;
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 10;
  goto arrayexception;

arrayleaderiop:
  if (_trace) printf("arrayleaderiop:\n");
  arg5 = 0;
  arg2 = 10;
  goto illegaloperand;

arrayleaderbounds:
  if (_trace) printf("arrayleaderbounds:\n");
  arg5 = 0;
  arg2 = 74;
  goto illegaloperand;

DoArrayLeaderIM:
  if (_trace) printf("DoArrayLeaderIM:\n");
  arg4 = *(s32 *)iSP;   		// arg3=arraytag, arg4=arraydata 
  arg3 = *(s32 *)(iSP + 4);   		// arg3=arraytag, arg4=arraydata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  goto arrayleadermerge;   

vma_memory_read32059:
  if (_trace) printf("vma_memory_read32059:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read32058;   

vma_memory_read32061:
  if (_trace) printf("vma_memory_read32061:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read32060;
  arg2 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read32057;   

vma_memory_read32060:
  if (_trace) printf("vma_memory_read32060:\n");
  t4 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read32065:
  if (_trace) printf("vma_memory_read32065:\n");
  t3 = t4 & MemoryActionTransform;
  if (t3 == 0) 
    goto vma_memory_read32064;
  arg6 = arg6 & ~63L;
  arg6 = arg6 | Type_ExternalValueCellPointer;
  goto vma_memory_read32068;   

vma_memory_read32064:

vma_memory_read32063:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read32049:
  if (_trace) printf("vma_memory_read32049:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read32048;   

vma_memory_read32051:
  if (_trace) printf("vma_memory_read32051:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read32050;
  arg4 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read32047;   

vma_memory_read32050:
  if (_trace) printf("vma_memory_read32050:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read32053:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 6;
  goto performmemoryaction;

/* end DoArrayLeader */
  /* End of Halfword operand from stack instruction - DoArrayLeader */
/* start DoStoreArrayLeader */

  /* Halfword operand from stack instruction - DoStoreArrayLeader */
  /* arg2 has the preloaded 8 bit operand. */

dostorearrayleader:
  if (_trace) printf("dostorearrayleader:\n");

DoStoreArrayLeaderSP:
  if (_trace) printf("DoStoreArrayLeaderSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoStoreArrayLeaderLP:
  if (_trace) printf("DoStoreArrayLeaderLP:\n");

DoStoreArrayLeaderFP:
  if (_trace) printf("DoStoreArrayLeaderFP:\n");

headdostorearrayleader:
  if (_trace) printf("headdostorearrayleader:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindostorearrayleader:
  if (_trace) printf("begindostorearrayleader:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// arg3=arraytag, arg4=arraydata 
  arg3 = *(s32 *)(iSP + 4);   		// arg3=arraytag, arg4=arraydata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  t7 = *(s32 *)iSP;   		// t6=valuetag, t7=valuedata 
  t6 = *(s32 *)(iSP + 4);   		// t6=valuetag, t7=valuedata 
  iSP = iSP - 8;   		// Pop Stack. 
  t7 = (u32)t7;   
  arg2 = (u32)arg1;   		// index data 
  arg1 = arg1 >> 32;   		// index tag 
  t1 = arg1 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto storearrayleaderiop;

storearrayleadermerge:
  if (_trace) printf("storearrayleadermerge:\n");
  t1 = arg3 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto storearrayleaderexception;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read32069:
  t3 = arg4 + ivory;
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg4 - t11;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t12) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read32071;

vma_memory_read32070:
  t3 = zero + 64;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read32073;

vma_memory_read32078:
  /* TagType. */
  t1 = arg6 & 63;
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto storearrayleaderiop;
  t2 = arg5 >> (Array_LeaderLengthFieldPos & 63);   
  t2 = t2 & Array_LeaderLengthFieldMask;
  t1 = ((u64)arg2 < (u64)t2) ? 1 : 0;   
  if (t1 == 0) 
    goto storearrayleaderbounds;
  arg2 = arg4 - arg2;   
  arg2 = arg2 - 1;   
  /* Memory Read Internal */

vma_memory_read32079:
  t5 = arg2 + ivory;
  t2 = (t5 * 4);   
  t1 = LDQ_U(t5);   
  t3 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read32081;

vma_memory_read32080:
  t5 = zero + 240;   
  t8 = t8 >> (t1 & 63);   
  t5 = t5 >> (t1 & 63);   
  if (t8 & 1)   
    goto vma_memory_read32083;

vma_memory_read32089:
  /* Merge cdr-code */
  t2 = t6 & 63;
  t1 = t1 & 192;
  t1 = t1 | t2;
  t3 = arg2 + ivory;
  t2 = (t3 * 4);   
  t5 = LDQ_U(t3);   
  t4 = arg2 - t11;   		// Stack cache offset 
  t8 = ((u64)t4 < (u64)t12) ? 1 : 0;   		// In range? 
  t4 = (t1 & 0xff) << ((t3&7)*8);   
  t5 = t5 & ~(0xffL << (t3&7)*8);   

force_alignment32091:
  if (_trace) printf("force_alignment32091:\n");
  t5 = t5 | t4;
  STQ_U(t3, t5);   
  *(u32 *)t2 = t7;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write32090;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

storearrayleaderexception:
  if (_trace) printf("storearrayleaderexception:\n");
  arg1 = Type_Fixnum;
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 3;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 11;
  goto arrayexception;

storearrayleaderiop:
  if (_trace) printf("storearrayleaderiop:\n");
  arg5 = 0;
  arg2 = 11;
  goto illegaloperand;

storearrayleaderbounds:
  if (_trace) printf("storearrayleaderbounds:\n");
  arg5 = 0;
  arg2 = 74;
  goto illegaloperand;

DoStoreArrayLeaderIM:
  if (_trace) printf("DoStoreArrayLeaderIM:\n");
  arg4 = *(s32 *)iSP;   		// arg3=arraytag, arg4=arraydata 
  arg3 = *(s32 *)(iSP + 4);   		// arg3=arraytag, arg4=arraydata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  t7 = *(s32 *)iSP;   		// t6=valuetag, t7=valuedata 
  t6 = *(s32 *)(iSP + 4);   		// t6=valuetag, t7=valuedata 
  iSP = iSP - 8;   		// Pop Stack. 
  t7 = (u32)t7;   
  goto storearrayleadermerge;   

vma_memory_write32090:
  if (_trace) printf("vma_memory_write32090:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = arg2 - t11;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
  *(u32 *)t3 = t7;   		// Store in stack 
  *(u32 *)(t3 + 4) = t1;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32081:
  if (_trace) printf("vma_memory_read32081:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t2 = *(s32 *)t3;   
  t1 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read32080;   

vma_memory_read32083:
  if (_trace) printf("vma_memory_read32083:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read32082;
  arg2 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read32079;   

vma_memory_read32082:
  if (_trace) printf("vma_memory_read32082:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read32086:

vma_memory_read32085:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read32071:
  if (_trace) printf("vma_memory_read32071:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read32070;   

vma_memory_read32073:
  if (_trace) printf("vma_memory_read32073:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read32072;
  arg4 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read32069;   

vma_memory_read32072:
  if (_trace) printf("vma_memory_read32072:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read32075:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 6;
  goto performmemoryaction;

/* end DoStoreArrayLeader */
  /* End of Halfword operand from stack instruction - DoStoreArrayLeader */
/* start DoAlocLeader */

  /* Halfword operand from stack instruction - DoAlocLeader */
  /* arg2 has the preloaded 8 bit operand. */

doalocleader:
  if (_trace) printf("doalocleader:\n");

DoAlocLeaderSP:
  if (_trace) printf("DoAlocLeaderSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAlocLeaderLP:
  if (_trace) printf("DoAlocLeaderLP:\n");

DoAlocLeaderFP:
  if (_trace) printf("DoAlocLeaderFP:\n");

headdoalocleader:
  if (_trace) printf("headdoalocleader:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoalocleader:
  if (_trace) printf("begindoalocleader:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// arg3=arraytag, arg4=arraydata 
  arg3 = *(s32 *)(iSP + 4);   		// arg3=arraytag, arg4=arraydata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = (u32)arg1;   		// index data 
  arg1 = arg1 >> 32;   		// index tag 
  t1 = arg1 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto alocleaderiop;

alocleadermerge:
  if (_trace) printf("alocleadermerge:\n");
  t1 = arg3 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto alocleaderexception;
  /* Memory Read Internal */

vma_memory_read32092:
  t1 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = arg4 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg4 - t1;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t2) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read32094;

vma_memory_read32093:
  t3 = zero + 64;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read32096;

vma_memory_read32101:
  /* TagType. */
  t1 = arg6 & 63;
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto alocleaderiop;
  t9 = arg5 >> (Array_LeaderLengthFieldPos & 63);   
  t9 = t9 & Array_LeaderLengthFieldMask;
  t1 = ((u64)arg2 < (u64)t9) ? 1 : 0;   
  if (t1 == 0) 
    goto alocleaderbounds;
  arg2 = arg4 - arg2;   
  arg2 = arg2 - 1;   
  t1 = Type_Locative;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

alocleaderexception:
  if (_trace) printf("alocleaderexception:\n");
  arg1 = Type_Fixnum;
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 10;
  goto arrayexception;

alocleaderiop:
  if (_trace) printf("alocleaderiop:\n");
  arg5 = 0;
  arg2 = 10;
  goto illegaloperand;

alocleaderbounds:
  if (_trace) printf("alocleaderbounds:\n");
  arg5 = 0;
  arg2 = 74;
  goto illegaloperand;

DoAlocLeaderIM:
  if (_trace) printf("DoAlocLeaderIM:\n");
  arg4 = *(s32 *)iSP;   		// arg3=arraytag, arg4=arraydata 
  arg3 = *(s32 *)(iSP + 4);   		// arg3=arraytag, arg4=arraydata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  goto alocleadermerge;   

vma_memory_read32094:
  if (_trace) printf("vma_memory_read32094:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read32093;   

vma_memory_read32096:
  if (_trace) printf("vma_memory_read32096:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read32095;
  arg4 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read32092;   

vma_memory_read32095:
  if (_trace) printf("vma_memory_read32095:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read32098:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 6;
  goto performmemoryaction;

/* end DoAlocLeader */
  /* End of Halfword operand from stack instruction - DoAlocLeader */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunarra.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunmove.as
 ************************************************************************/

  /* Data movement. */
/* start DoPushNNils */

  /* Halfword operand from stack instruction - DoPushNNils */
  /* arg2 has the preloaded 8 bit operand. */

dopushnnils:
  if (_trace) printf("dopushnnils:\n");

DoPushNNilsSP:
  if (_trace) printf("DoPushNNilsSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushNNilsLP:
  if (_trace) printf("DoPushNNilsLP:\n");

DoPushNNilsFP:
  if (_trace) printf("DoPushNNilsFP:\n");

headdopushnnils:
  if (_trace) printf("headdopushnnils:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindopushnnils:
  if (_trace) printf("begindopushnnils:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = (u32)arg1;   		// Get the data 
  t1 = arg1 >> 32;   		// and the tag 
  t5 = t1 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto pushnnbadop;

DoPushNNilsIM:
  if (_trace) printf("DoPushNNilsIM:\n");
  t4 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t1 = zero + 128;   
  t2 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t1 = t1 + arg2;		// Account for what we're about to push 
  t1 = (t1 * 8) + iSP;  		// SCA of desired end of cache 
  t2 = (t4 * 8) + t2;  		// SCA of current end of cache 
  t4 = ((s64)t1 <= (s64)t2) ? 1 : 0;   
  if (t4 == 0) 		// We're done if new SCA is within bounds 
    goto stackcacheoverflowhandler;
  arg6 = *(u64 *)&(processor->niladdress);   
  goto pushnnilsl2;   

pushnnilsl1:
  if (_trace) printf("pushnnilsl1:\n");
  *(u64 *)(iSP + 8) = arg6;   		// Push NIL 
  iSP = iSP + 8;
  arg2 = arg2 - 1;   

pushnnilsl2:
  if ((s64)arg2 > 0)   
    goto pushnnilsl1;
  goto NEXTINSTRUCTION;   

pushnnbadop:
  if (_trace) printf("pushnnbadop:\n");
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

/* end DoPushNNils */
  /* End of Halfword operand from stack instruction - DoPushNNils */
/* start DoPushAddressSpRelative */

  /* Halfword operand from stack instruction - DoPushAddressSpRelative */
  /* arg2 has the preloaded 8 bit operand. */

dopushaddresssprelative:
  if (_trace) printf("dopushaddresssprelative:\n");

DoPushAddressSpRelativeIM:
  if (_trace) printf("DoPushAddressSpRelativeIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindopushaddresssprelative;   

DoPushAddressSpRelativeSP:
  if (_trace) printf("DoPushAddressSpRelativeSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushAddressSpRelativeLP:
  if (_trace) printf("DoPushAddressSpRelativeLP:\n");

DoPushAddressSpRelativeFP:
  if (_trace) printf("DoPushAddressSpRelativeFP:\n");

headdopushaddresssprelative:
  if (_trace) printf("headdopushaddresssprelative:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindopushaddresssprelative:
  if (_trace) printf("begindopushaddresssprelative:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t4 = *(u64 *)&(processor->restartsp);   		// SP before any popping 
  t1 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t6 = *(u64 *)&(processor->stackcachebasevma);   		// Base of the stack cache 
  t7 = *(u64 *)&(processor->stackcachedata);   		// THe stack cache data block 
  t2 = t1 & 63;		// Strip off any CDR code bits. 
  t3 = (t2 == Type_Fixnum) ? 1 : 0;   

force_alignment32107:
  if (_trace) printf("force_alignment32107:\n");
  if (t3 == 0) 
    goto basic_dispatch32104;
  /* Here if argument TypeFixnum */
  arg1 = (arg1 * 8) + 8;  
  t5 = t4 - arg1;   		// Compute stack relative pointer 
  t5 = t5 - t7;   		// Index into stack data 
  t5 = t5 >> 3;   		// Convert to word index 
  t5 = t6 + t5;		// Convert to an ivory word address 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t6 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;   
  *(u32 *)(iSP + 12) = t6;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch32104:
  if (_trace) printf("basic_dispatch32104:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

basic_dispatch32103:
  if (_trace) printf("basic_dispatch32103:\n");

/* end DoPushAddressSpRelative */
  /* End of Halfword operand from stack instruction - DoPushAddressSpRelative */
/* start DoStackBlt */

  /* Halfword operand from stack instruction - DoStackBlt */
  /* arg2 has the preloaded 8 bit operand. */

dostackblt:
  if (_trace) printf("dostackblt:\n");

DoStackBltIM:
  if (_trace) printf("DoStackBltIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindostackblt;   

DoStackBltSP:
  if (_trace) printf("DoStackBltSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoStackBltLP:
  if (_trace) printf("DoStackBltLP:\n");

DoStackBltFP:
  if (_trace) printf("DoStackBltFP:\n");

headdostackblt:
  if (_trace) printf("headdostackblt:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindostackblt:
  if (_trace) printf("begindostackblt:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t3 = *(s32 *)iSP;   		// Destination locative 
  t2 = *(s32 *)(iSP + 4);   		// Destination locative 
  iSP = iSP - 8;   		// Pop Stack. 
  t3 = (u32)t3;   
  t1 = (u32)arg1;   
  /* Convert VMA to stack cache address */
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  arg1 = *(u64 *)&(processor->stackcachedata);   
  t4 = t1 - t4;   		// stack cache base relative offset 
  arg1 = (t4 * 8) + arg1;  		// reconstruct SCA 
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of the stack cache 
  t5 = *(u64 *)&(processor->stackcachetopvma);   		// End ofthe stack cache 
  t1 = *(u64 *)&(processor->stackcachedata);   		// THe stack cache data block 
  t6 = t3 - t4;   		// BAse of Stack Cache. 
  t7 = t3 - t5;   		// Top of Stack Cache. 
  if ((s64)t6 < 0)   		// J. if vma below stack cache 
    goto stkbltexc;
  if ((s64)t7 >= 0)   		// J. if vma above stack cache 
    goto stkbltexc;
  t6 = (t6 * 8) + t1;  		// Compute the stackcache address 
  goto stkbltloopend;   

stkbltloop:
  if (_trace) printf("stkbltloop:\n");
  arg1 = arg1 + 8;		// Advance Source 
  t6 = t6 + 8;		// Advance destination 

stkbltloopend:
  t1 = *(u64 *)arg1;   		// Read a word from the source 
  t4 = arg1 - iSP;   
  *(u64 *)t6 = t1;   		// copy the word 
  if (t4 != 0)   		// J. if sourse not stack top 
    goto stkbltloop;
  iSP = t6;		// Update the SP to point at the last written location 
  goto NEXTINSTRUCTION;   

stkbltexc:
  if (_trace) printf("stkbltexc:\n");
  arg5 = 0;
  arg2 = 73;
  goto illegaloperand;

/* end DoStackBlt */
  /* End of Halfword operand from stack instruction - DoStackBlt */
/* start DoStackBltAddress */

  /* Halfword operand from stack instruction - DoStackBltAddress */
  /* arg2 has the preloaded 8 bit operand. */

dostackbltaddress:
  if (_trace) printf("dostackbltaddress:\n");

DoStackBltAddressSP:
  if (_trace) printf("DoStackBltAddressSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoStackBltAddressLP:
  if (_trace) printf("DoStackBltAddressLP:\n");

DoStackBltAddressFP:
  if (_trace) printf("DoStackBltAddressFP:\n");

begindostackbltaddress:
  if (_trace) printf("begindostackbltaddress:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t3 = *(s32 *)iSP;   		// Destination locative 
  t2 = *(s32 *)(iSP + 4);   		// Destination locative 
  iSP = iSP - 8;   		// Pop Stack. 
  t3 = (u32)t3;   
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of the stack cache 
  t5 = *(u64 *)&(processor->stackcachetopvma);   		// End ofthe stack cache 
  t1 = *(u64 *)&(processor->stackcachedata);   		// THe stack cache data block 
  t6 = t3 - t4;   		// Base of Stack Cache. 
  t7 = t3 - t5;   		// Top of Stack Cache. 
  if ((s64)t6 < 0)   		// J. if vma below stack cache 
    goto stkbltadrexc;
  if ((s64)t7 >= 0)   		// J. if vma above stack cache 
    goto stkbltadrexc;
  t6 = (t6 * 8) + t1;  		// Compute the stackcache address 
  goto stkbltaddloopend;   

stkbltaddloop:
  if (_trace) printf("stkbltaddloop:\n");
  arg1 = arg1 + 8;		// Advance Source 
  t6 = t6 + 8;		// Advance destination 

stkbltaddloopend:
  t1 = *(u64 *)arg1;   		// Read a word from the source 
  t4 = arg1 - iSP;   
  *(u64 *)t6 = t1;   		// copy the word 
  if (t4 != 0)   		// J. if sourse not stack top 
    goto stkbltaddloop;
  iSP = t6;		// Update the SP to point at the last written location 
  goto NEXTINSTRUCTION;   

stkbltadrexc:
  if (_trace) printf("stkbltadrexc:\n");
  arg5 = 0;
  arg2 = 73;
  goto illegaloperand;

DoStackBltAddressIM:
  goto doistageerror;

/* end DoStackBltAddress */
  /* End of Halfword operand from stack instruction - DoStackBltAddress */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunmove.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunpred.as
 ************************************************************************/

  /* Predicates. */
/* start DoEql */

  /* Halfword operand from stack instruction - DoEql */
  /* arg2 has the preloaded 8 bit operand. */

doeql:
  if (_trace) printf("doeql:\n");

DoEqlSP:
  if (_trace) printf("DoEqlSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoEqlLP:
  if (_trace) printf("DoEqlLP:\n");

DoEqlFP:
  if (_trace) printf("DoEqlFP:\n");

headdoeql:
  if (_trace) printf("headdoeql:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoeql:
  if (_trace) printf("begindoeql:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg6 = arg3 >> 12;   
  t3 = *(u64 *)iSP;   		// Load arg1 into t3 
  t4 = zero + -2048;   		// Low part of EQ-NOT-EQL mask 
  t11 = *(u64 *)&(processor->niladdress);   
  t4 = t4 + ((1) << 16);   		// High part of EQ-NOT-EQL mask 
  t12 = *(u64 *)&(processor->taddress);   		// Assume result will be T 
  t5 = arg1 ^ t3;   
  t5 = t5 << 26;   		// Shift left to lose CDRCODE. 
  arg6 = arg6 & 1;		// 1 if no-pop, 0 if pop 
  if (t5 == 0) 
    goto eqldone;
  /* They are not EQ, if types different or not numeric return nil */
  t5 = t5 >> 58;   		// Get the tag alone 
  t12 = t11;		// Now assume result will be NIL 
  if (t5 != 0)   		// Return NIL if tags different 
    goto eqldone;
  t3 = t3 >> 32;   		// Get tag, check for numeric 
  /* TagType. */
  t3 = t3 & 63;
  t4 = t4 >> (t3 & 63);   		// Type is now a bit mask 
  if (t4 & 1)   		// If funny numeric type, exception 
    goto eqlexc;

eqldone:
  if (_trace) printf("eqldone:\n");
  iSP = (arg6 * 8) + iSP;  		// Either a stack-push or a stack-write 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)iSP = t12;   
  goto cachevalid;   

DoEqlIM:
  if (_trace) printf("DoEqlIM:\n");
  arg2 = arg2 << 56;   
  t4 = *(s32 *)(iSP + 4);   		// t4=tag t3=data 
  t3 = *(s32 *)iSP;   
  arg6 = arg3 >> 12;   
  arg2 = (s64)arg2 >> 56;   		// Sign extension of arg2 is complete 
  t3 = (u32)t3;   
  t11 = *(u64 *)&(processor->niladdress);   
  /* TagType. */
  t4 = t4 & 63;
  t12 = *(u64 *)&(processor->taddress);   
  arg2 = (s32)t3 - (s32)arg2;   
  t4 = t4 ^ Type_Fixnum;   
  arg6 = arg6 & 1;		// 1 if no-pop, 0 if pop 
  t4 = arg2 | t4;
  iSP = (arg6 * 8) + iSP;  		// Either a stack-push or a stack-write 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t4 == 0)   
    t11 = t12;
  *(u64 *)iSP = t11;   		// Yes Virginia, this does dual issue with above 
  goto cachevalid;   

eqlexc:
  if (_trace) printf("eqlexc:\n");
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto exception;

/* end DoEql */
  /* End of Halfword operand from stack instruction - DoEql */
/* start DoGreaterp */

  /* Halfword operand from stack instruction - DoGreaterp */
  /* arg2 has the preloaded 8 bit operand. */

dogreaterp:
  if (_trace) printf("dogreaterp:\n");

DoGreaterpSP:
  if (_trace) printf("DoGreaterpSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindogreaterp;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoGreaterpLP:
  if (_trace) printf("DoGreaterpLP:\n");

DoGreaterpFP:
  if (_trace) printf("DoGreaterpFP:\n");

begindogreaterp:
  if (_trace) printf("begindogreaterp:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t7 = arg3 >> 12;   
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   		// Get ARG1 tag 
  t1 = *(s32 *)(arg1 + 4);   		// t1 is tag of arg2 
  LDS(1, f1, *(u32 *)iSP );   
  t7 = t7 & 1;
  arg2 = *(s32 *)arg1;   
  arg4 = (s32)arg6;
  LDS(2, f2, *(u32 *)arg1 );   
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment32125:
  if (_trace) printf("force_alignment32125:\n");
  if (t6 == 0) 
    goto basic_dispatch32113;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment32117:
  if (_trace) printf("force_alignment32117:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32108;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 > 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch32114:
  if (_trace) printf("basic_dispatch32114:\n");

basic_dispatch32113:
  if (_trace) printf("basic_dispatch32113:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment32126:
  if (_trace) printf("force_alignment32126:\n");
  if (t6 == 0) 
    goto basic_dispatch32118;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment32122:
  if (_trace) printf("force_alignment32122:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32108;
  /* Here if argument TypeSingleFloat */

greaterpmmexcfltflt:
  if (_trace) printf("greaterpmmexcfltflt:\n");
  SETFLTT(3,f3, FLTU64(1,f1) <= FLTU64(2,f2) ? 2.0:0);  
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)iSP = t12;   
  if (FLTU64(3, f3) == 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch32119:
  if (_trace) printf("basic_dispatch32119:\n");

basic_dispatch32118:
  if (_trace) printf("basic_dispatch32118:\n");
  /* Here for all other cases */

binary_type_dispatch32108:
  if (_trace) printf("binary_type_dispatch32108:\n");
  goto greaterpmmexc;   

basic_dispatch32112:
  if (_trace) printf("basic_dispatch32112:\n");

DoGreaterpIM:
  if (_trace) printf("DoGreaterpIM:\n");
  t11 = *(u64 *)&(processor->niladdress);   
  arg2 = arg2 << 56;   		// First half of sign extension 
  t12 = *(u64 *)&(processor->taddress);   
  t7 = arg3 >> 12;   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (s32)arg6;
  arg2 = (s64)arg2 >> 56;   		// Second half of sign extension 
  t7 = t7 & 1;
  t3 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = (t3 == Type_Fixnum) ? 1 : 0;   

force_alignment32131:
  if (_trace) printf("force_alignment32131:\n");
  if (t4 == 0) 
    goto basic_dispatch32128;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 > 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch32128:
  if (_trace) printf("basic_dispatch32128:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch32127:
  if (_trace) printf("basic_dispatch32127:\n");

/* end DoGreaterp */
  /* End of Halfword operand from stack instruction - DoGreaterp */
/* start DoLogtest */

  /* Halfword operand from stack instruction - DoLogtest */
  /* arg2 has the preloaded 8 bit operand. */

dologtest:
  if (_trace) printf("dologtest:\n");

DoLogtestSP:
  if (_trace) printf("DoLogtestSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindologtest;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoLogtestLP:
  if (_trace) printf("DoLogtestLP:\n");

DoLogtestFP:
  if (_trace) printf("DoLogtestFP:\n");

begindologtest:
  if (_trace) printf("begindologtest:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t11 = *(u64 *)&(processor->niladdress);   
  t7 = arg3 >> 12;   
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = (u32)(arg6 >> ((4&7)*8));   		// Get ARG1 tag 
  arg2 = *(s32 *)arg1;   
  LDS(1, f1, *(u32 *)iSP );   
  t7 = t7 & 1;
  t1 = *(s32 *)(arg1 + 4);   		// t1 is tag of arg2 
  arg4 = (u32)arg6;   
  arg2 = (u32)arg2;   
  LDS(2, f2, *(u32 *)arg1 );   
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment32144:
  if (_trace) printf("force_alignment32144:\n");
  if (t6 == 0) 
    goto basic_dispatch32137;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment32141:
  if (_trace) printf("force_alignment32141:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32134;
  /* Here if argument TypeFixnum */
  t2 = arg4 & arg2;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch32138:
  if (_trace) printf("basic_dispatch32138:\n");

basic_dispatch32137:
  if (_trace) printf("basic_dispatch32137:\n");
  /* Here for all other cases */

binary_type_dispatch32133:
  if (_trace) printf("binary_type_dispatch32133:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch32135;   

binary_type_dispatch32134:
  if (_trace) printf("binary_type_dispatch32134:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch32135:
  if (_trace) printf("binary_type_dispatch32135:\n");

basic_dispatch32136:
  if (_trace) printf("basic_dispatch32136:\n");

DoLogtestIM:
  if (_trace) printf("DoLogtestIM:\n");
  t11 = *(u64 *)&(processor->niladdress);   
  arg2 = arg2 << 56;   		// First half of sign extension 
  t12 = *(u64 *)&(processor->taddress);   
  t7 = arg3 >> 12;   
  arg3 = (u32)(arg6 >> ((4&7)*8));   
  arg4 = (s32)arg6;
  arg2 = (s64)arg2 >> 56;   		// Second half of sign extension 
  t7 = t7 & 1;
  t3 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = (t3 == Type_Fixnum) ? 1 : 0;   

force_alignment32149:
  if (_trace) printf("force_alignment32149:\n");
  if (t4 == 0) 
    goto basic_dispatch32146;
  /* Here if argument TypeFixnum */
  t2 = arg4 & arg2;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch32146:
  if (_trace) printf("basic_dispatch32146:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch32145:
  if (_trace) printf("basic_dispatch32145:\n");

/* end DoLogtest */
  /* End of Halfword operand from stack instruction - DoLogtest */
/* start EqualNumberMMExc */


equalnumbermmexc:
  if (_trace) printf("equalnumbermmexc:\n");
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment32167:
  if (_trace) printf("force_alignment32167:\n");
  if (t6 == 0) 
    goto basic_dispatch32155;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment32159:
  if (_trace) printf("force_alignment32159:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32152;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto equalnumbermmexcfltflt;   

basic_dispatch32156:
  if (_trace) printf("basic_dispatch32156:\n");

basic_dispatch32155:
  if (_trace) printf("basic_dispatch32155:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment32168:
  if (_trace) printf("force_alignment32168:\n");
  if (t6 == 0) 
    goto basic_dispatch32160;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment32164:
  if (_trace) printf("force_alignment32164:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32152;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto equalnumbermmexcfltflt;   

basic_dispatch32161:
  if (_trace) printf("basic_dispatch32161:\n");

basic_dispatch32160:
  if (_trace) printf("basic_dispatch32160:\n");
  /* Here for all other cases */

binary_type_dispatch32151:
  if (_trace) printf("binary_type_dispatch32151:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch32153;   

binary_type_dispatch32152:
  if (_trace) printf("binary_type_dispatch32152:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch32153:
  if (_trace) printf("binary_type_dispatch32153:\n");

basic_dispatch32154:
  if (_trace) printf("basic_dispatch32154:\n");

/* end EqualNumberMMExc */
/* start LesspMMExc */


lesspmmexc:
  if (_trace) printf("lesspmmexc:\n");
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment32186:
  if (_trace) printf("force_alignment32186:\n");
  if (t6 == 0) 
    goto basic_dispatch32174;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment32178:
  if (_trace) printf("force_alignment32178:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32171;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto lesspmmexcfltflt;   

basic_dispatch32175:
  if (_trace) printf("basic_dispatch32175:\n");

basic_dispatch32174:
  if (_trace) printf("basic_dispatch32174:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment32187:
  if (_trace) printf("force_alignment32187:\n");
  if (t6 == 0) 
    goto basic_dispatch32179;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment32183:
  if (_trace) printf("force_alignment32183:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32171;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto lesspmmexcfltflt;   

basic_dispatch32180:
  if (_trace) printf("basic_dispatch32180:\n");

basic_dispatch32179:
  if (_trace) printf("basic_dispatch32179:\n");
  /* Here for all other cases */

binary_type_dispatch32170:
  if (_trace) printf("binary_type_dispatch32170:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch32172;   

binary_type_dispatch32171:
  if (_trace) printf("binary_type_dispatch32171:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch32172:
  if (_trace) printf("binary_type_dispatch32172:\n");

basic_dispatch32173:
  if (_trace) printf("basic_dispatch32173:\n");

/* end LesspMMExc */
/* start GreaterpMMExc */


greaterpmmexc:
  if (_trace) printf("greaterpmmexc:\n");
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment32205:
  if (_trace) printf("force_alignment32205:\n");
  if (t6 == 0) 
    goto basic_dispatch32193;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment32197:
  if (_trace) printf("force_alignment32197:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32190;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto greaterpmmexcfltflt;   

basic_dispatch32194:
  if (_trace) printf("basic_dispatch32194:\n");

basic_dispatch32193:
  if (_trace) printf("basic_dispatch32193:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment32206:
  if (_trace) printf("force_alignment32206:\n");
  if (t6 == 0) 
    goto basic_dispatch32198;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment32202:
  if (_trace) printf("force_alignment32202:\n");
  if (t3 == 0) 
    goto binary_type_dispatch32190;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto greaterpmmexcfltflt;   

basic_dispatch32199:
  if (_trace) printf("basic_dispatch32199:\n");

basic_dispatch32198:
  if (_trace) printf("basic_dispatch32198:\n");
  /* Here for all other cases */

binary_type_dispatch32189:
  if (_trace) printf("binary_type_dispatch32189:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch32191;   

binary_type_dispatch32190:
  if (_trace) printf("binary_type_dispatch32190:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch32191:
  if (_trace) printf("binary_type_dispatch32191:\n");

basic_dispatch32192:
  if (_trace) printf("basic_dispatch32192:\n");

/* end GreaterpMMExc */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunpred.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunsubp.as
 ************************************************************************/

  /* Subprimitives. */
/* start DoEphemeralp */

  /* Halfword operand from stack instruction - DoEphemeralp */

doephemeralp:
  if (_trace) printf("doephemeralp:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoEphemeralpIM:
  if (_trace) printf("DoEphemeralpIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32207:
  if (_trace) printf("force_alignment32207:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoephemeralp;   

DoEphemeralpSP:
  if (_trace) printf("DoEphemeralpSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoEphemeralpLP:
  if (_trace) printf("DoEphemeralpLP:\n");

DoEphemeralpFP:
  if (_trace) printf("DoEphemeralpFP:\n");

headdoephemeralp:
  if (_trace) printf("headdoephemeralp:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoephemeralp:
  if (_trace) printf("begindoephemeralp:\n");
  /* arg1 has the operand, sign extended if immediate. */
  t1 = *(u64 *)&(processor->ptrtype);   		// ptr type array 
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  /* TagType. */
  arg2 = arg2 & 63;
  t2 = (arg2 * 4) + t1;   
  arg1 = arg1 >> 27;   
  t3 = *(s32 *)t2;   		// =0 if not a pointer 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (arg1 != 0)   		// J. if zone not ephemeral 
    goto nonephem;
  if (t3 == 0) 		// J. if not a pointer 
    goto nonephem;
  t6 = *(u64 *)&(processor->taddress);   
  *(u64 *)(iSP + 8) = t6;   		// push the data 
  iSP = iSP + 8;
  goto cachevalid;   

nonephem:
  if (_trace) printf("nonephem:\n");
  t6 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t6;   		// push the data 
  iSP = iSP + 8;
  goto cachevalid;   

/* end DoEphemeralp */
  /* End of Halfword operand from stack instruction - DoEphemeralp */
/* start DoUnsignedLessp */

  /* Halfword operand from stack instruction - DoUnsignedLessp */
  /* arg2 has the preloaded 8 bit operand. */

dounsignedlessp:
  if (_trace) printf("dounsignedlessp:\n");

DoUnsignedLesspSP:
  if (_trace) printf("DoUnsignedLesspSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoUnsignedLesspLP:
  if (_trace) printf("DoUnsignedLesspLP:\n");

DoUnsignedLesspFP:
  if (_trace) printf("DoUnsignedLesspFP:\n");

headdounsignedlessp:
  if (_trace) printf("headdounsignedlessp:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindounsignedlessp:
  if (_trace) printf("begindounsignedlessp:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t2 = *(s32 *)iSP;   		// Get data from arg1 
  arg3 = arg3 >> 12;   
  t11 = *(u64 *)&(processor->niladdress);   
  t4 = (u32)arg1;   		// Get unsigned data from arg2 
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = arg3 & 1;		// 1 if no-pop, 0 if pop 
  t2 = (u32)t2;   		// Unsigned arg1 
  iSP = (arg3 * 8) + iSP;  		// Either a stack-push or a stack-write 
  t6 = t4 - t2;   		// t6:=arg2-arg1 unsigned 
  if ((s64)t6 > 0)   
    t11 = t12;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)iSP = t11;   
  goto cachevalid;   

DoUnsignedLesspIM:
  if (_trace) printf("DoUnsignedLesspIM:\n");
  t2 = *(s32 *)iSP;   		// Get data from arg1 
  arg3 = arg3 >> 12;   
  t11 = *(u64 *)&(processor->niladdress);   
  t2 = (u32)t2;   		// ... 
  t12 = *(u64 *)&(processor->taddress);   
  arg3 = arg3 & 1;		// 1 if no-pop, 0 if pop 
  t6 = arg2 - t2;   		// t6:=arg2-arg1 unsigned 
  iSP = (arg3 * 8) + iSP;  		// Either a stack-push or a stack-write 
  if ((s64)t6 > 0)   
    t11 = t12;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u64 *)iSP = t11;   
  goto cachevalid;   

/* end DoUnsignedLessp */
  /* End of Halfword operand from stack instruction - DoUnsignedLessp */
/* start DoAllocateListBlock */

  /* Halfword operand from stack instruction - DoAllocateListBlock */
  /* arg2 has the preloaded 8 bit operand. */

doallocatelistblock:
  if (_trace) printf("doallocatelistblock:\n");

DoAllocateListBlockIM:
  if (_trace) printf("DoAllocateListBlockIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoallocatelistblock;   

DoAllocateListBlockSP:
  if (_trace) printf("DoAllocateListBlockSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAllocateListBlockLP:
  if (_trace) printf("DoAllocateListBlockLP:\n");

DoAllocateListBlockFP:
  if (_trace) printf("DoAllocateListBlockFP:\n");

headdoallocatelistblock:
  if (_trace) printf("headdoallocatelistblock:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoallocatelistblock:
  if (_trace) printf("begindoallocatelistblock:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t1 = *(u64 *)&(processor->lcarea);   
  arg3 = *(u64 *)iSP;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t5 = arg2 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto i_allocate_block32208;
  t4 = *(s32 *)&processor->lclength;   
  t2 = (arg3 == t1) ? 1 : 0;   
  if (t2 == 0) 		// Wrong area 
    goto i_allocate_block32209;
  t2 = t4 - arg1;   		// Effectively an unsigned 32-bit compare 
  if ((s64)t2 < 0)   		// Insufficient cache 
    goto i_allocate_block32209;
  t1 = *(u64 *)&(processor->lcaddress);   		// Fetch address 
  t3 = (-16384) << 16;   
  t3 = (u32)t3;   
  *(u32 *)&processor->lclength = t2;   		// Store remaining length 
  *(u64 *)iSP = t1;   		// Cache address/tag -> TOS 
  *(u32 *)&processor->bar1 = t1;   		// Cache address -> BAR1 
  t1 = (u32)t1;   
  t4 = *(s32 *)&processor->control;   		// Verify trap mode 
  t1 = t1 + arg1;		// Increment address 
  *(u32 *)&processor->lcaddress = t1;   		// Store updated address 
  t3 = t3 & t4;
  if (t3 != 0)   		// Already above emulator mode 
    goto NEXTINSTRUCTION;
  t3 = (16384) << 16;   
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   
  goto NEXTINSTRUCTION;   

i_allocate_block32208:
  if (_trace) printf("i_allocate_block32208:\n");
  arg5 = 0;
  arg2 = 1;
  goto illegaloperand;

i_allocate_block32209:
  if (_trace) printf("i_allocate_block32209:\n");
  /* SetTag. */
  t1 = arg2 << 32;   
  t1 = arg1 | t1;
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

/* end DoAllocateListBlock */
  /* End of Halfword operand from stack instruction - DoAllocateListBlock */
/* start DoAllocateStructureBlock */

  /* Halfword operand from stack instruction - DoAllocateStructureBlock */
  /* arg2 has the preloaded 8 bit operand. */

doallocatestructureblock:
  if (_trace) printf("doallocatestructureblock:\n");

DoAllocateStructureBlockIM:
  if (_trace) printf("DoAllocateStructureBlockIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoallocatestructureblock;   

DoAllocateStructureBlockSP:
  if (_trace) printf("DoAllocateStructureBlockSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAllocateStructureBlockLP:
  if (_trace) printf("DoAllocateStructureBlockLP:\n");

DoAllocateStructureBlockFP:
  if (_trace) printf("DoAllocateStructureBlockFP:\n");

headdoallocatestructureblock:
  if (_trace) printf("headdoallocatestructureblock:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoallocatestructureblock:
  if (_trace) printf("begindoallocatestructureblock:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t1 = *(u64 *)&(processor->scarea);   
  arg3 = *(u64 *)iSP;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t5 = arg2 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto i_allocate_block32210;
  t4 = *(s32 *)&processor->sclength;   
  t2 = (arg3 == t1) ? 1 : 0;   
  if (t2 == 0) 		// Wrong area 
    goto i_allocate_block32211;
  t2 = t4 - arg1;   		// Effectively an unsigned 32-bit compare 
  if ((s64)t2 < 0)   		// Insufficient cache 
    goto i_allocate_block32211;
  t1 = *(u64 *)&(processor->scaddress);   		// Fetch address 
  t3 = (-16384) << 16;   
  t3 = (u32)t3;   
  *(u32 *)&processor->sclength = t2;   		// Store remaining length 
  *(u64 *)iSP = t1;   		// Cache address/tag -> TOS 
  *(u32 *)&processor->bar1 = t1;   		// Cache address -> BAR1 
  t1 = (u32)t1;   
  t4 = *(s32 *)&processor->control;   		// Verify trap mode 
  t1 = t1 + arg1;		// Increment address 
  *(u32 *)&processor->scaddress = t1;   		// Store updated address 
  t3 = t3 & t4;
  if (t3 != 0)   		// Already above emulator mode 
    goto NEXTINSTRUCTION;
  t3 = (16384) << 16;   
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   
  goto NEXTINSTRUCTION;   

i_allocate_block32210:
  if (_trace) printf("i_allocate_block32210:\n");
  arg5 = 0;
  arg2 = 1;
  goto illegaloperand;

i_allocate_block32211:
  if (_trace) printf("i_allocate_block32211:\n");
  /* SetTag. */
  t1 = arg2 << 32;   
  t1 = arg1 | t1;
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

/* end DoAllocateStructureBlock */
  /* End of Halfword operand from stack instruction - DoAllocateStructureBlock */
/* start DoPointerDifference */

  /* Halfword operand from stack instruction - DoPointerDifference */
  /* arg2 has the preloaded 8 bit operand. */

dopointerdifference:
  if (_trace) printf("dopointerdifference:\n");

DoPointerDifferenceSP:
  if (_trace) printf("DoPointerDifferenceSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPointerDifferenceLP:
  if (_trace) printf("DoPointerDifferenceLP:\n");

DoPointerDifferenceFP:
  if (_trace) printf("DoPointerDifferenceFP:\n");

headdopointerdifference:
  if (_trace) printf("headdopointerdifference:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindopointerdifference:
  if (_trace) printf("begindopointerdifference:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t1 = *(s32 *)iSP;   		// Get the data of ARG1 
  t2 = (u32)arg1;   		// Get the data of ARG2 
  t3 = (s32)t1 - (s32)t2;   		// (%32-bit-difference (data arg1) (data arg2)) 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Fixnum;
  *(u32 *)iSP = t3;   		// Save result and coerce to a FIXNUM 
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

DoPointerDifferenceIM:
  if (_trace) printf("DoPointerDifferenceIM:\n");
  t2 = arg2 << 56;   
  t1 = *(s32 *)iSP;   		// Get the data of arg1 
  t2 = (s64)t2 >> 56;   
  t3 = (s32)t1 - (s32)t2;   		// (%32-bit-difference (data arg1) (data arg2)) 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Fixnum;
  *(u32 *)iSP = t3;   		// Save result and coerce to a FIXNUM 
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

/* end DoPointerDifference */
  /* End of Halfword operand from stack instruction - DoPointerDifference */
/* start DoPointerIncrement */

  /* Halfword operand from stack instruction - DoPointerIncrement */
  /* arg2 has the preloaded 8 bit operand. */

dopointerincrement:
  if (_trace) printf("dopointerincrement:\n");

DoPointerIncrementSP:
  if (_trace) printf("DoPointerIncrementSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPointerIncrementLP:
  if (_trace) printf("DoPointerIncrementLP:\n");

DoPointerIncrementFP:
  if (_trace) printf("DoPointerIncrementFP:\n");

begindopointerincrement:
  if (_trace) printf("begindopointerincrement:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t2 = *(s32 *)arg1;   		// Get the data of arg2 
  t3 = (s32)t2 + (s32)1;		// (%32-bit-plus (data arg1) 1) 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)arg1 = t3;   		// Put result back 
  goto cachevalid;   

DoPointerIncrementIM:
  goto doistageerror;

/* end DoPointerIncrement */
  /* End of Halfword operand from stack instruction - DoPointerIncrement */
/* start DoStoreConditional */

  /* Halfword operand from stack instruction - DoStoreConditional */

dostoreconditional:
  if (_trace) printf("dostoreconditional:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoStoreConditionalIM:
  if (_trace) printf("DoStoreConditionalIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32228:
  if (_trace) printf("force_alignment32228:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindostoreconditional;   

DoStoreConditionalSP:
  if (_trace) printf("DoStoreConditionalSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoStoreConditionalLP:
  if (_trace) printf("DoStoreConditionalLP:\n");

DoStoreConditionalFP:
  if (_trace) printf("DoStoreConditionalFP:\n");

headdostoreconditional:
  if (_trace) printf("headdostoreconditional:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindostoreconditional:
  if (_trace) printf("begindostoreconditional:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = arg1 >> 32;   
  arg4 = *(s32 *)iSP;   		// old tag and data 
  arg3 = *(s32 *)(iSP + 4);   		// old tag and data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg1 = (u32)arg1;   
  arg6 = *(s32 *)iSP;   		// address tag and data 
  arg5 = *(s32 *)(iSP + 4);   		// address tag and data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg6 = (u32)arg6;   
  /* TagType. */
  t1 = arg5 & 63;
  t2 = t1 - Type_Locative;   
  t2 = t2 & 63;		// Strip CDR code 
  if (t2 != 0)   
    goto storecondiop;
  /* Read the location, checking write access */
  /* Memory Read Internal */

vma_memory_read32212:
  t1 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t3 = arg6 + ivory;
  t2 = *(s32 *)&processor->scovlimit;   
  t5 = (t3 * 4);   
  t4 = LDQ_U(t3);   
  t1 = arg6 - t1;   		// Stack cache offset 
  t2 = ((u64)t1 < (u64)t2) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read32214;

vma_memory_read32213:
  t1 = *(u64 *)&(processor->dataread_mask);   
  t3 = zero + 240;   
  t1 = t1 >> (t4 & 63);   
  t3 = t3 >> (t4 & 63);   
  if (t1 & 1)   
    goto vma_memory_read32216;

vma_memory_read32223:
  t1 = (s32)arg4 - (s32)t5;   		// Check for data match - NOT 
  t2 = arg3 ^ t4;   		// Zero if tags match 
  if (t1 != 0)   		// Jump if data didn't match 
    goto storecondnil;
  /* TagType. */
  t2 = t2 & 63;		// Stip result of comparing CDR-CODEs 
  if (t2 != 0)   		// Jump if tags don't match 
    goto storecondnil;
  t1 = arg2 & 63;		// Strip CDR-CODE 
  t4 = t4 & 192;		// Retain CDR-CODE 
  t4 = t1 | t4;		// Merge new tag with old CDR-CODE 
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t1 = arg6 + ivory;
  t6 = *(s32 *)&processor->scovlimit;   
  t5 = (t1 * 4);   
  t3 = LDQ_U(t1);   
  t2 = arg6 - t2;   		// Stack cache offset 
  t6 = ((u64)t2 < (u64)t6) ? 1 : 0;   		// In range? 
  t2 = (t4 & 0xff) << ((t1&7)*8);   
  t3 = t3 & ~(0xffL << (t1&7)*8);   

force_alignment32226:
  if (_trace) printf("force_alignment32226:\n");
  t3 = t3 | t2;
  STQ_U(t1, t3);   
  *(u32 *)t5 = arg1;   
  if (t6 != 0)   		// J. if in cache 
    goto vma_memory_write32225;

vma_memory_write32224:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t6 = *(u64 *)&(processor->taddress);   
  *(u64 *)(iSP + 8) = t6;   		// push the data 
  iSP = iSP + 8;
  goto cachevalid;   

storecondnil:
  if (_trace) printf("storecondnil:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t6 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t6;   		// push the data 
  iSP = iSP + 8;
  goto cachevalid;   

storecondiop:
  if (_trace) printf("storecondiop:\n");
  arg5 = 0;
  arg2 = 65;
  goto illegaloperand;

vma_memory_write32225:
  if (_trace) printf("vma_memory_write32225:\n");
  t2 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32227:
  if (_trace) printf("force_alignment32227:\n");
  t1 = *(u64 *)&(processor->stackcachedata);   
  t2 = arg6 - t2;   		// Stack cache offset 
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
  *(u32 *)t1 = arg1;   		// Store in stack 
  *(u32 *)(t1 + 4) = t4;   		// write the stack cache 
  goto vma_memory_write32224;   

vma_memory_read32214:
  if (_trace) printf("vma_memory_read32214:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  t5 = *(s32 *)t1;   
  t4 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read32213;   

vma_memory_read32216:
  if (_trace) printf("vma_memory_read32216:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read32215;
  arg6 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read32212;   

vma_memory_read32215:
  if (_trace) printf("vma_memory_read32215:\n");
  t1 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg6;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t1;   		// Adjust for a longword load 
  t1 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read32220:
  if (_trace) printf("vma_memory_read32220:\n");
  t3 = t1 & MemoryActionTransform;
  if (t3 == 0) 
    goto vma_memory_read32219;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read32223;   

vma_memory_read32219:

vma_memory_read32218:
  /* Perform memory action */
  arg1 = t1;
  arg2 = 0;
  goto performmemoryaction;

/* end DoStoreConditional */
  /* End of Halfword operand from stack instruction - DoStoreConditional */
/* start DoMemoryWrite */

  /* Halfword operand from stack instruction - DoMemoryWrite */

domemorywrite:
  if (_trace) printf("domemorywrite:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoMemoryWriteIM:
  if (_trace) printf("DoMemoryWriteIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32232:
  if (_trace) printf("force_alignment32232:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindomemorywrite;   

DoMemoryWriteSP:
  if (_trace) printf("DoMemoryWriteSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoMemoryWriteLP:
  if (_trace) printf("DoMemoryWriteLP:\n");

DoMemoryWriteFP:
  if (_trace) printf("DoMemoryWriteFP:\n");

headdomemorywrite:
  if (_trace) printf("headdomemorywrite:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindomemorywrite:
  if (_trace) printf("begindomemorywrite:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg4 = *(s32 *)iSP;   
  arg3 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t1 = arg4 + ivory;
  t5 = *(s32 *)&processor->scovlimit;   
  t4 = (t1 * 4);   
  t3 = LDQ_U(t1);   
  t2 = arg4 - t2;   		// Stack cache offset 
  t5 = ((u64)t2 < (u64)t5) ? 1 : 0;   		// In range? 
  t2 = (arg2 & 0xff) << ((t1&7)*8);   
  t3 = t3 & ~(0xffL << (t1&7)*8);   

force_alignment32230:
  if (_trace) printf("force_alignment32230:\n");
  t3 = t3 | t2;
  STQ_U(t1, t3);   
  *(u32 *)t4 = arg1;   
  if (t5 != 0)   		// J. if in cache 
    goto vma_memory_write32229;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write32229:
  if (_trace) printf("vma_memory_write32229:\n");
  t2 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32231:
  if (_trace) printf("force_alignment32231:\n");
  t1 = *(u64 *)&(processor->stackcachedata);   
  t2 = arg4 - t2;   		// Stack cache offset 
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
  *(u32 *)t1 = arg1;   		// Store in stack 
  *(u32 *)(t1 + 4) = arg2;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

/* end DoMemoryWrite */
  /* End of Halfword operand from stack instruction - DoMemoryWrite */
/* start DoPStoreContents */

  /* Halfword operand from stack instruction - DoPStoreContents */

dopstorecontents:
  if (_trace) printf("dopstorecontents:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoPStoreContentsIM:
  if (_trace) printf("DoPStoreContentsIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32245:
  if (_trace) printf("force_alignment32245:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindopstorecontents;   

DoPStoreContentsSP:
  if (_trace) printf("DoPStoreContentsSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPStoreContentsLP:
  if (_trace) printf("DoPStoreContentsLP:\n");

DoPStoreContentsFP:
  if (_trace) printf("DoPStoreContentsFP:\n");

headdopstorecontents:
  if (_trace) printf("headdopstorecontents:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindopstorecontents:
  if (_trace) printf("begindopstorecontents:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// address tag and data 
  arg3 = *(s32 *)(iSP + 4);   		// address tag and data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  /* Memory Read Internal */

vma_memory_read32233:
  t6 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t8 = arg4 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg4 - t6;   		// Stack cache offset 
  t7 = ((u64)t6 < (u64)t7) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32235;

vma_memory_read32234:

vma_memory_read32241:
  /* Merge cdr-code */
  t5 = arg2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t7 = *(u64 *)&(processor->stackcachebasevma);   
  t6 = arg4 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = arg4 - t7;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)t9) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment32243:
  if (_trace) printf("force_alignment32243:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = arg1;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write32242;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write32242:
  if (_trace) printf("vma_memory_write32242:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32244:
  if (_trace) printf("force_alignment32244:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg4 - t7;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = arg1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32235:
  if (_trace) printf("vma_memory_read32235:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read32234;   

/* end DoPStoreContents */
  /* End of Halfword operand from stack instruction - DoPStoreContents */
/* start DoSetCdrCode1 */

  /* Halfword operand from stack instruction - DoSetCdrCode1 */
  /* arg2 has the preloaded 8 bit operand. */

dosetcdrcode1:
  if (_trace) printf("dosetcdrcode1:\n");

DoSetCdrCode1SP:
  if (_trace) printf("DoSetCdrCode1SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetCdrCode1LP:
  if (_trace) printf("DoSetCdrCode1LP:\n");

DoSetCdrCode1FP:
  if (_trace) printf("DoSetCdrCode1FP:\n");

begindosetcdrcode1:
  if (_trace) printf("begindosetcdrcode1:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)(arg1 + 4);   		// Get CDR CODE/TAG of operand 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t1 = t1 & 63;		// Strip off any existing CDR code bits 
  t1 = t1 | 64;		// OR in the CDR 
  *(u32 *)(arg1 + 4) = t1;   		// Replace the CDE CODE/TAG 
  goto cachevalid;   

DoSetCdrCode1IM:
  goto doistageerror;

/* end DoSetCdrCode1 */
  /* End of Halfword operand from stack instruction - DoSetCdrCode1 */
/* start DoSetCdrCode2 */

  /* Halfword operand from stack instruction - DoSetCdrCode2 */
  /* arg2 has the preloaded 8 bit operand. */

dosetcdrcode2:
  if (_trace) printf("dosetcdrcode2:\n");

DoSetCdrCode2SP:
  if (_trace) printf("DoSetCdrCode2SP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSetCdrCode2LP:
  if (_trace) printf("DoSetCdrCode2LP:\n");

DoSetCdrCode2FP:
  if (_trace) printf("DoSetCdrCode2FP:\n");

begindosetcdrcode2:
  if (_trace) printf("begindosetcdrcode2:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t1 = *(s32 *)(arg1 + 4);   		// Get CDR CODE/TAG of operand 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t1 = t1 & 63;		// Strip off any existing CDR code bits 
  t1 = t1 | 128;		// OR in the CDR 
  *(u32 *)(arg1 + 4) = t1;   		// Replace the CDE CODE/TAG 
  goto cachevalid;   

DoSetCdrCode2IM:
  goto doistageerror;

/* end DoSetCdrCode2 */
  /* End of Halfword operand from stack instruction - DoSetCdrCode2 */
/* start DoJump */

  /* Halfword operand from stack instruction - DoJump */
  /* arg2 has the preloaded 8 bit operand. */

dojump:
  if (_trace) printf("dojump:\n");

DoJumpSP:
  if (_trace) printf("DoJumpSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoJumpLP:
  if (_trace) printf("DoJumpLP:\n");

DoJumpFP:
  if (_trace) printf("DoJumpFP:\n");

begindojump:
  if (_trace) printf("begindojump:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t4 = *(s32 *)arg1;   		// Read address and even/odd PC tag. 
  t3 = *(s32 *)(arg1 + 4);   
  t4 = (u32)t4;   
  t5 = t3 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto jexc;
  t4 = t4 << 1;   
  iPC = t3 & 1;
  iPC = iPC + t4;
  t5 = t3 & 128;
  if (t5 == 0) 
    goto interpretinstructionforjump;
  /* Bit 39=1 indicates we need to update control reg */
  t6 = t3 & 64;		// Get the cleanup bit 
  t5 = *(u64 *)&(processor->control);   		// Processor control register. 
  t6 = t6 << 17;   		// shift into cleanup-in-progress place 
  t7 = (128) << 16;   
  t5 = t5 & ~t7;		// Mask 
  t5 = t5 | t6;		// Set 
  *(u64 *)&processor->control = t5;   
  goto interpretinstructionforjump;   

jexc:
  if (_trace) printf("jexc:\n");
  arg3 = 1;		// arg3 = stackp 
  arg1 = 0;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

DoJumpIM:
  goto doistageerror;

/* end DoJump */
  /* End of Halfword operand from stack instruction - DoJump */
/* start DoCheckPreemptRequest */

  /* Halfword 10 bit immediate instruction - DoCheckPreemptRequest */

docheckpreemptrequest:
  if (_trace) printf("docheckpreemptrequest:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoCheckPreemptRequestIM:
  if (_trace) printf("DoCheckPreemptRequestIM:\n");

DoCheckPreemptRequestSP:
  if (_trace) printf("DoCheckPreemptRequestSP:\n");

DoCheckPreemptRequestLP:
  if (_trace) printf("DoCheckPreemptRequestLP:\n");

DoCheckPreemptRequestFP:
  if (_trace) printf("DoCheckPreemptRequestFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  t1 = *(s32 *)&processor->interruptreg;   
  t2 = t1 & 2;
  t2 = (t2 == 2) ? 1 : 0;   
  t1 = t1 | t2;
  *(u32 *)&processor->interruptreg = t1;   
  if (t1 == 0) 
    goto NEXTINSTRUCTION;
  *(u64 *)&processor->stop_interpreter = t1;   
  goto NEXTINSTRUCTION;   

/* end DoCheckPreemptRequest */
  /* End of Halfword operand from stack instruction - DoCheckPreemptRequest */
/* start DoHalt */

  /* Halfword 10 bit immediate instruction - DoHalt */

dohalt:
  if (_trace) printf("dohalt:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoHaltIM:
  if (_trace) printf("DoHaltIM:\n");

DoHaltSP:
  if (_trace) printf("DoHaltSP:\n");

DoHaltLP:
  if (_trace) printf("DoHaltLP:\n");

DoHaltFP:
  if (_trace) printf("DoHaltFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  t1 = *(s32 *)&processor->control;   
  t1 = t1 >> 30;   		// Isolate current trap mode (FEP mode = -1) 
  t1 = (s32)t1 + (s32)1;		// t1 is zero iff we're in trap mode FEP 
  if (t1 != 0)   
    goto haltexc;
  goto haltmachine;

haltexc:
  if (_trace) printf("haltexc:\n");
  arg3 = 1;		// arg3 = stackp 
  arg1 = 0;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

/* end DoHalt */
  /* End of Halfword operand from stack instruction - DoHalt */
/* start DoNoOp */

  /* Halfword 10 bit immediate instruction - DoNoOp */

donoop:
  if (_trace) printf("donoop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoNoOpIM:
  if (_trace) printf("DoNoOpIM:\n");

DoNoOpSP:
  if (_trace) printf("DoNoOpSP:\n");

DoNoOpLP:
  if (_trace) printf("DoNoOpLP:\n");

DoNoOpFP:
  if (_trace) printf("DoNoOpFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  goto NEXTINSTRUCTION;   

/* end DoNoOp */
  /* End of Halfword operand from stack instruction - DoNoOp */
/* start DoAlu */

  /* Halfword operand from stack instruction - DoAlu */

doalu:
  if (_trace) printf("doalu:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoAluIM:
  if (_trace) printf("DoAluIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32316:
  if (_trace) printf("force_alignment32316:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoalu;   

DoAluSP:
  if (_trace) printf("DoAluSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAluLP:
  if (_trace) printf("DoAluLP:\n");

DoAluFP:
  if (_trace) printf("DoAluFP:\n");

headdoalu:
  if (_trace) printf("headdoalu:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoalu:
  if (_trace) printf("begindoalu:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = arg1 >> 32;   		// Get tag of ARG2 
  arg1 = (u32)arg1;   		// Get data of ARG2 
  arg4 = *(s32 *)iSP;   		// Get ARG1 
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  t1 = arg2 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto aluexc;
  t1 = arg3 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto aluexc;
  arg5 = *(u64 *)&(processor->aluop);   
  *(u64 *)&processor->aluoverflow = zero;   
  arg6 = *(u64 *)&(processor->aluandrotatecontrol);   
  t1 = (arg5 == ALUFunction_Boolean) ? 1 : 0;   

force_alignment32306:
  if (_trace) printf("force_alignment32306:\n");
  if (t1 == 0) 
    goto basic_dispatch32247;
  /* Here if argument ALUFunctionBoolean */
  t10 = arg6 >> 10;   
  t10 = t10 & 15;		// Extract the ALU boolean function 
  t1 = (t10 == Boole_Clear) ? 1 : 0;   

force_alignment32266:
  if (_trace) printf("force_alignment32266:\n");
  if (t1 != 0)   
    goto basic_dispatch32248;

basic_dispatch32249:
  if (_trace) printf("basic_dispatch32249:\n");
  t1 = (t10 == Boole_And) ? 1 : 0;   

force_alignment32267:
  if (_trace) printf("force_alignment32267:\n");
  if (t1 == 0) 
    goto basic_dispatch32250;
  /* Here if argument BooleAnd */
  t10 = arg4 & arg1;
  goto basic_dispatch32248;   

basic_dispatch32250:
  if (_trace) printf("basic_dispatch32250:\n");
  t1 = (t10 == Boole_AndC1) ? 1 : 0;   

force_alignment32268:
  if (_trace) printf("force_alignment32268:\n");
  if (t1 == 0) 
    goto basic_dispatch32251;
  /* Here if argument BooleAndC1 */
  t10 = arg1 & ~arg4;
  goto basic_dispatch32248;   

basic_dispatch32251:
  if (_trace) printf("basic_dispatch32251:\n");
  t1 = (t10 == Boole_2) ? 1 : 0;   

force_alignment32269:
  if (_trace) printf("force_alignment32269:\n");
  if (t1 == 0) 
    goto basic_dispatch32252;
  /* Here if argument Boole2 */
  t10 = arg1;
  goto basic_dispatch32248;   

basic_dispatch32252:
  if (_trace) printf("basic_dispatch32252:\n");
  t1 = (t10 == Boole_AndC2) ? 1 : 0;   

force_alignment32270:
  if (_trace) printf("force_alignment32270:\n");
  if (t1 == 0) 
    goto basic_dispatch32253;
  /* Here if argument BooleAndC2 */
  t10 = arg4 & ~arg1;
  goto basic_dispatch32248;   

basic_dispatch32253:
  if (_trace) printf("basic_dispatch32253:\n");
  t1 = (t10 == Boole_1) ? 1 : 0;   

force_alignment32271:
  if (_trace) printf("force_alignment32271:\n");
  if (t1 == 0) 
    goto basic_dispatch32254;
  /* Here if argument Boole1 */
  t10 = arg4;
  goto basic_dispatch32248;   

basic_dispatch32254:
  if (_trace) printf("basic_dispatch32254:\n");
  t1 = (t10 == Boole_Xor) ? 1 : 0;   

force_alignment32272:
  if (_trace) printf("force_alignment32272:\n");
  if (t1 == 0) 
    goto basic_dispatch32255;
  /* Here if argument BooleXor */
  t10 = arg4 ^ arg1;   
  goto basic_dispatch32248;   

basic_dispatch32255:
  if (_trace) printf("basic_dispatch32255:\n");
  t1 = (t10 == Boole_Ior) ? 1 : 0;   

force_alignment32273:
  if (_trace) printf("force_alignment32273:\n");
  if (t1 == 0) 
    goto basic_dispatch32256;
  /* Here if argument BooleIor */
  t10 = arg4 | arg1;
  goto basic_dispatch32248;   

basic_dispatch32256:
  if (_trace) printf("basic_dispatch32256:\n");
  t1 = (t10 == Boole_Nor) ? 1 : 0;   

force_alignment32274:
  if (_trace) printf("force_alignment32274:\n");
  if (t1 == 0) 
    goto basic_dispatch32257;
  /* Here if argument BooleNor */
  t10 = arg4 | arg1;
  t10 = ~t10;   
  goto basic_dispatch32248;   

basic_dispatch32257:
  if (_trace) printf("basic_dispatch32257:\n");
  t1 = (t10 == Boole_Equiv) ? 1 : 0;   

force_alignment32275:
  if (_trace) printf("force_alignment32275:\n");
  if (t1 == 0) 
    goto basic_dispatch32258;
  /* Here if argument BooleEquiv */
  t10 = arg4 ^ arg1;   
  t10 = ~t10;   
  goto basic_dispatch32248;   

basic_dispatch32258:
  if (_trace) printf("basic_dispatch32258:\n");
  t1 = (t10 == Boole_C1) ? 1 : 0;   

force_alignment32276:
  if (_trace) printf("force_alignment32276:\n");
  if (t1 == 0) 
    goto basic_dispatch32259;
  /* Here if argument BooleC1 */
  t10 = ~arg4;   
  goto basic_dispatch32248;   

basic_dispatch32259:
  if (_trace) printf("basic_dispatch32259:\n");
  t1 = (t10 == Boole_OrC1) ? 1 : 0;   

force_alignment32277:
  if (_trace) printf("force_alignment32277:\n");
  if (t1 == 0) 
    goto basic_dispatch32260;
  /* Here if argument BooleOrC1 */
  t10 = arg1 | ~(arg4);   
  goto basic_dispatch32248;   

basic_dispatch32260:
  if (_trace) printf("basic_dispatch32260:\n");
  t1 = (t10 == Boole_C2) ? 1 : 0;   

force_alignment32278:
  if (_trace) printf("force_alignment32278:\n");
  if (t1 == 0) 
    goto basic_dispatch32261;
  /* Here if argument BooleC2 */
  t10 = ~arg1;   
  goto basic_dispatch32248;   

basic_dispatch32261:
  if (_trace) printf("basic_dispatch32261:\n");
  t1 = (t10 == Boole_OrC2) ? 1 : 0;   

force_alignment32279:
  if (_trace) printf("force_alignment32279:\n");
  if (t1 == 0) 
    goto basic_dispatch32262;
  /* Here if argument BooleOrC2 */
  t10 = arg4 & ~arg1;
  goto basic_dispatch32248;   

basic_dispatch32262:
  if (_trace) printf("basic_dispatch32262:\n");
  t1 = (t10 == Boole_Nand) ? 1 : 0;   

force_alignment32280:
  if (_trace) printf("force_alignment32280:\n");
  if (t1 == 0) 
    goto basic_dispatch32263;
  /* Here if argument BooleNand */
  t10 = arg4 & arg1;
  goto basic_dispatch32248;   

basic_dispatch32263:
  if (_trace) printf("basic_dispatch32263:\n");
  t1 = (t10 == Boole_Set) ? 1 : 0;   

force_alignment32281:
  if (_trace) printf("force_alignment32281:\n");
  if (t1 == 0) 
    goto basic_dispatch32248;
  /* Here if argument BooleSet */
  t10 = ~zero;   

basic_dispatch32248:
  if (_trace) printf("basic_dispatch32248:\n");
  *(u32 *)iSP = t10;   
  goto NEXTINSTRUCTION;   

basic_dispatch32247:
  if (_trace) printf("basic_dispatch32247:\n");
  t1 = (arg5 == ALUFunction_Byte) ? 1 : 0;   

force_alignment32307:
  if (_trace) printf("force_alignment32307:\n");
  if (t1 == 0) 
    goto basic_dispatch32282;
  /* Here if argument ALUFunctionByte */
  t2 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t3 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t1 = arg6 >> 10;   
  t1 = t1 & 3;		// Extract the byte background 
  t4 = (t1 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment32289:
  if (_trace) printf("force_alignment32289:\n");
  if (t4 == 0) 
    goto basic_dispatch32285;
  /* Here if argument ALUByteBackgroundOp1 */
  t1 = arg4;

basic_dispatch32284:
  if (_trace) printf("basic_dispatch32284:\n");
  t5 = arg6 >> 12;   
  t5 = t5 & 1;		// Extractthe byte rotate latch 
  t10 = arg1 << (t2 & 63);   
  t4 = (u32)(t10 >> ((4&7)*8));   
  t10 = (u32)t10;   
  t10 = t10 | t4;		// OP2 rotated 
  if (t5 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte32283;
  *(u64 *)&processor->rotatelatch = t10;   

alu_function_byte32283:
  if (_trace) printf("alu_function_byte32283:\n");
  t5 = zero + -2;   
  t5 = t5 << (t3 & 63);   
  t5 = ~t5;   		// Compute mask 
  /* Get byte function */
  t4 = arg6 >> 13;   
  t4 = t4 & 1;
  t3 = (t4 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment32294:
  if (_trace) printf("force_alignment32294:\n");
  if (t3 == 0) 
    goto basic_dispatch32291;
  /* Here if argument ALUByteFunctionDpb */
  t5 = t5 << (t2 & 63);   		// Position mask 

basic_dispatch32290:
  if (_trace) printf("basic_dispatch32290:\n");
  t10 = t10 & t5;		// rotated&mask 
  t1 = t1 & ~t5;		// background&~mask 
  t10 = t10 | t1;
  *(u32 *)iSP = t10;   
  goto NEXTINSTRUCTION;   

basic_dispatch32282:
  if (_trace) printf("basic_dispatch32282:\n");
  t1 = (arg5 == ALUFunction_Adder) ? 1 : 0;   

force_alignment32308:
  if (_trace) printf("force_alignment32308:\n");
  if (t1 == 0) 
    goto basic_dispatch32295;
  /* Here if argument ALUFunctionAdder */
  t3 = arg6 >> 11;   
  t3 = t3 & 3;		// Extract the op2 
  t2 = arg6 >> 10;   
  t2 = t2 & 1;		// Extract the adder carry in 
  t4 = (t3 == ALUAdderOp2_Op2) ? 1 : 0;   

force_alignment32303:
  if (_trace) printf("force_alignment32303:\n");
  if (t4 == 0) 
    goto basic_dispatch32298;
  /* Here if argument ALUAdderOp2Op2 */
  t1 = arg1;

basic_dispatch32297:
  if (_trace) printf("basic_dispatch32297:\n");
  t10 = arg4 + t1;
  t10 = t10 + t2;
  t3 = t10 >> 31;   		// Sign bit 
  t4 = t10 >> 32;   		// Next bit 
  t3 = t3 ^ t4;   		// Low bit is now overflow indicator 
  t4 = arg6 >> 24;   		// Get the load-carry-in bit 
  *(u64 *)&processor->aluoverflow = t3;   
  if ((t4 & 1) == 0)   
    goto alu_function_adder32296;
  t3 = (u32)(t10 >> ((4&7)*8));   		// Get the carry 
  t4 = zero + 1024;   
  arg6 = arg6 & ~t4;
  t4 = t3 & 1;
  t4 = t4 << 10;   
  arg6 = arg6 | t4;		// Set the adder carry in 
  *(u64 *)&processor->aluandrotatecontrol = arg6;   

alu_function_adder32296:
  if (_trace) printf("alu_function_adder32296:\n");
  t3 = ((s64)arg4 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->aluborrow = t3;   
  arg4 = (s32)arg4;
  arg1 = (s32)arg1;
  t3 = ((s64)arg4 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->alulessthan = t3;   
  *(u32 *)iSP = t10;   
  goto NEXTINSTRUCTION;   

basic_dispatch32295:
  if (_trace) printf("basic_dispatch32295:\n");
  t1 = (arg5 == ALUFunction_MultiplyDivide) ? 1 : 0;   

force_alignment32309:
  if (_trace) printf("force_alignment32309:\n");
  if (t1 == 0) 
    goto basic_dispatch32246;
  /* Here if argument ALUFunctionMultiplyDivide */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;
  *(u32 *)iSP = t10;   
  goto NEXTINSTRUCTION;   

basic_dispatch32246:
  if (_trace) printf("basic_dispatch32246:\n");

aluexc:
  if (_trace) printf("aluexc:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

basic_dispatch32298:
  if (_trace) printf("basic_dispatch32298:\n");
  t4 = (t3 == ALUAdderOp2_Zero) ? 1 : 0;   

force_alignment32310:
  if (_trace) printf("force_alignment32310:\n");
  if (t4 == 0) 
    goto basic_dispatch32299;
  /* Here if argument ALUAdderOp2Zero */
  t1 = zero;
  goto basic_dispatch32297;   

basic_dispatch32299:
  if (_trace) printf("basic_dispatch32299:\n");
  t4 = (t3 == ALUAdderOp2_Invert) ? 1 : 0;   

force_alignment32311:
  if (_trace) printf("force_alignment32311:\n");
  if (t4 == 0) 
    goto basic_dispatch32300;
  /* Here if argument ALUAdderOp2Invert */
  t1 = (s32)arg1;
  t1 = zero - t1;   
  t1 = (u32)t1;   
  goto basic_dispatch32297;   

basic_dispatch32300:
  if (_trace) printf("basic_dispatch32300:\n");
  t4 = (t3 == ALUAdderOp2_MinusOne) ? 1 : 0;   

force_alignment32312:
  if (_trace) printf("force_alignment32312:\n");
  if (t4 == 0) 
    goto basic_dispatch32297;
  /* Here if argument ALUAdderOp2MinusOne */
  t1 = ~zero;   
  t1 = (u32)t1;   
  goto basic_dispatch32297;   

basic_dispatch32291:
  if (_trace) printf("basic_dispatch32291:\n");
  t3 = (t4 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment32313:
  if (_trace) printf("force_alignment32313:\n");
  if (t3 != 0)   
    goto basic_dispatch32290;
  goto basic_dispatch32290;   

basic_dispatch32285:
  if (_trace) printf("basic_dispatch32285:\n");
  t4 = (t1 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment32314:
  if (_trace) printf("force_alignment32314:\n");
  if (t4 == 0) 
    goto basic_dispatch32286;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t1 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch32284;   

basic_dispatch32286:
  if (_trace) printf("basic_dispatch32286:\n");
  t4 = (t1 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment32315:
  if (_trace) printf("force_alignment32315:\n");
  if (t4 == 0) 
    goto basic_dispatch32284;
  /* Here if argument ALUByteBackgroundZero */
  t1 = zero;
  goto basic_dispatch32284;   

/* end DoAlu */
  /* End of Halfword operand from stack instruction - DoAlu */
/* start DoSpareOp */

  /* Halfword 10 bit immediate instruction - DoSpareOp */

dospareop:
  if (_trace) printf("dospareop:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoSpareOpIM:
  if (_trace) printf("DoSpareOpIM:\n");

DoSpareOpSP:
  if (_trace) printf("DoSpareOpSP:\n");

DoSpareOpLP:
  if (_trace) printf("DoSpareOpLP:\n");

DoSpareOpFP:
  if (_trace) printf("DoSpareOpFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  t1 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// Get the instruction 
  t1 = t1 >> 10;   		// Position the opcode 
  t1 = t1 & 255;		// Extract it 
  arg1 = 0;		// arg1 = instruction arity 
  arg2 = t1;		// arg2 = instruction opcode 
  arg3 = 1;		// arg3 = stackp 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;		// when not stackp arg5=the arg 
  arg6 = 0;		// arg6=tag to dispatch on 
  goto exception;
  goto NEXTINSTRUCTION;   

/* end DoSpareOp */
  /* End of Halfword operand from stack instruction - DoSpareOp */
  /* Reading and writing internal registers */
/* start ReadRegisterFP */


ReadRegisterFP:
  if (_trace) printf("ReadRegisterFP:\n");
  /* Convert stack cache address to VMA */
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t5 = iFP - t5;   		// stack cache base relative offset 
  t5 = t5 >> 3;   		// convert byte address to word address 
  t4 = t5 + t4;		// reconstruct VMA 
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t4;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterFP */
/* start ReadRegisterLP */


ReadRegisterLP:
  if (_trace) printf("ReadRegisterLP:\n");
  /* Convert stack cache address to VMA */
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t5 = iLP - t5;   		// stack cache base relative offset 
  t5 = t5 >> 3;   		// convert byte address to word address 
  t4 = t5 + t4;		// reconstruct VMA 
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t4;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterLP */
/* start ReadRegisterSP */


ReadRegisterSP:
  if (_trace) printf("ReadRegisterSP:\n");
  /* Convert stack cache address to VMA */
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t5 = iSP - t5;   		// stack cache base relative offset 
  t5 = t5 >> 3;   		// convert byte address to word address 
  t4 = t5 + t4;		// reconstruct VMA 
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t4;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterSP */
/* start ReadRegisterStackCacheLowerBound */


ReadRegisterStackCacheLowerBound:
  if (_trace) printf("ReadRegisterStackCacheLowerBound:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStackCacheLowerBound */
/* start ReadRegisterBARx */


ReadRegisterBARx:
  if (_trace) printf("ReadRegisterBARx:\n");
  t2 = arg1 >> 7;   		// BAR number into T2 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = (u64)&processor->bar0;   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t1 = (t2 * 8) + t1;  		// Now T1 points to the BAR 
  t3 = *(u64 *)t1;   
  t4 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

/* end ReadRegisterBARx */
/* start ReadRegisterContinuation */


ReadRegisterContinuation:
  if (_trace) printf("ReadRegisterContinuation:\n");
  t3 = *(u64 *)&(processor->continuation);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterContinuation */
/* start ReadRegisterAluAndRotateControl */


ReadRegisterAluAndRotateControl:
  if (_trace) printf("ReadRegisterAluAndRotateControl:\n");
  t3 = *(u64 *)&(processor->aluandrotatecontrol);   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterAluAndRotateControl */
/* start ReadRegisterControlRegister */


ReadRegisterControlRegister:
  if (_trace) printf("ReadRegisterControlRegister:\n");
  t3 = *(s32 *)&processor->control;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterControlRegister */
/* start ReadRegisterCRArgumentSize */


ReadRegisterCRArgumentSize:
  if (_trace) printf("ReadRegisterCRArgumentSize:\n");
  t3 = *(s32 *)&processor->control;   
  t3 = t3 & 255;		// Get the argument size field 
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterCRArgumentSize */
/* start ReadRegisterEphemeralOldspaceRegister */


ReadRegisterEphemeralOldspaceRegister:
  if (_trace) printf("ReadRegisterEphemeralOldspaceRegister:\n");
  t3 = *(s32 *)&processor->ephemeraloldspace;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterEphemeralOldspaceRegister */
/* start ReadRegisterZoneOldspaceRegister */


ReadRegisterZoneOldspaceRegister:
  if (_trace) printf("ReadRegisterZoneOldspaceRegister:\n");
  t3 = *(s32 *)&processor->zoneoldspace;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterZoneOldspaceRegister */
/* start ReadRegisterChipRevision */


ReadRegisterChipRevision:
  if (_trace) printf("ReadRegisterChipRevision:\n");
  t3 = 5;
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterChipRevision */
/* start ReadRegisterFPCoprocessorPresent */


ReadRegisterFPCoprocessorPresent:
  if (_trace) printf("ReadRegisterFPCoprocessorPresent:\n");
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterFPCoprocessorPresent */
/* start ReadRegisterPreemptRegister */


ReadRegisterPreemptRegister:
  if (_trace) printf("ReadRegisterPreemptRegister:\n");
  t3 = *(s32 *)&processor->interruptreg;   
  t3 = t3 & 3;
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPreemptRegister */
/* start ReadRegisterIcacheControl */


ReadRegisterIcacheControl:
  if (_trace) printf("ReadRegisterIcacheControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterIcacheControl */
/* start ReadRegisterPrefetcherControl */


ReadRegisterPrefetcherControl:
  if (_trace) printf("ReadRegisterPrefetcherControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPrefetcherControl */
/* start ReadRegisterMapCacheControl */


ReadRegisterMapCacheControl:
  if (_trace) printf("ReadRegisterMapCacheControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterMapCacheControl */
/* start ReadRegisterMemoryControl */


ReadRegisterMemoryControl:
  if (_trace) printf("ReadRegisterMemoryControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterMemoryControl */
/* start ReadRegisterStackCacheOverflowLimit */


ReadRegisterStackCacheOverflowLimit:
  if (_trace) printf("ReadRegisterStackCacheOverflowLimit:\n");
  t3 = *(s32 *)&processor->scovlimit;   
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t3 + t4;
  t4 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStackCacheOverflowLimit */
/* start ReadRegisterMicrosecondClock */


ReadRegisterMicrosecondClock:
  if (_trace) printf("ReadRegisterMicrosecondClock:\n");
  t1 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterMicrosecondClock */
/* start ReadRegisterTOS */


ReadRegisterTOS:
  if (_trace) printf("ReadRegisterTOS:\n");
  t1 = *(u64 *)iSP;   
  iSP = iSP + 8;
  t2 = t1 << 26;   
  t2 = t2 >> 26;   
  *(u64 *)iSP = t2;   		// Push CDR-NEXT TOS 
  goto NEXTINSTRUCTION;   

/* end ReadRegisterTOS */
/* start ReadRegisterEventCount */


ReadRegisterEventCount:
  if (_trace) printf("ReadRegisterEventCount:\n");
  t3 = *(u64 *)&(processor->areventcount);   
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterEventCount */
/* start ReadRegisterBindingStackPointer */


ReadRegisterBindingStackPointer:
  if (_trace) printf("ReadRegisterBindingStackPointer:\n");
  t3 = *(u64 *)&(processor->bindingstackpointer);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterBindingStackPointer */
/* start ReadRegisterCatchBlockList */


ReadRegisterCatchBlockList:
  if (_trace) printf("ReadRegisterCatchBlockList:\n");
  t3 = *(u64 *)&(processor->catchblock);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterCatchBlockList */
/* start ReadRegisterControlStackLimit */


ReadRegisterControlStackLimit:
  if (_trace) printf("ReadRegisterControlStackLimit:\n");
  t3 = *(s32 *)&processor->cslimit;   
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterControlStackLimit */
/* start ReadRegisterControlStackExtraLimit */


ReadRegisterControlStackExtraLimit:
  if (_trace) printf("ReadRegisterControlStackExtraLimit:\n");
  t3 = *(s32 *)&processor->csextralimit;   
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterControlStackExtraLimit */
/* start ReadRegisterBindingStackLimit */


ReadRegisterBindingStackLimit:
  if (_trace) printf("ReadRegisterBindingStackLimit:\n");
  t3 = *(u64 *)&(processor->bindingstacklimit);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterBindingStackLimit */
/* start ReadRegisterPHTBase */


ReadRegisterPHTBase:
  if (_trace) printf("ReadRegisterPHTBase:\n");
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPHTBase */
/* start ReadRegisterPHTMask */


ReadRegisterPHTMask:
  if (_trace) printf("ReadRegisterPHTMask:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPHTMask */
/* start ReadRegisterCountMapReloads */


ReadRegisterCountMapReloads:
  if (_trace) printf("ReadRegisterCountMapReloads:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterCountMapReloads */
/* start ReadRegisterListCacheArea */


ReadRegisterListCacheArea:
  if (_trace) printf("ReadRegisterListCacheArea:\n");
  t3 = *(u64 *)&(processor->lcarea);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterListCacheArea */
/* start ReadRegisterListCacheAddress */


ReadRegisterListCacheAddress:
  if (_trace) printf("ReadRegisterListCacheAddress:\n");
  t3 = *(u64 *)&(processor->lcaddress);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterListCacheAddress */
/* start ReadRegisterListCacheLength */


ReadRegisterListCacheLength:
  if (_trace) printf("ReadRegisterListCacheLength:\n");
  t3 = *(s32 *)&processor->lclength;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterListCacheLength */
/* start ReadRegisterStructureCacheArea */


ReadRegisterStructureCacheArea:
  if (_trace) printf("ReadRegisterStructureCacheArea:\n");
  t3 = *(u64 *)&(processor->scarea);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStructureCacheArea */
/* start ReadRegisterStructureCacheAddress */


ReadRegisterStructureCacheAddress:
  if (_trace) printf("ReadRegisterStructureCacheAddress:\n");
  t3 = *(u64 *)&(processor->scaddress);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStructureCacheAddress */
/* start ReadRegisterStructureCacheLength */


ReadRegisterStructureCacheLength:
  if (_trace) printf("ReadRegisterStructureCacheLength:\n");
  t3 = *(s32 *)&processor->sclength;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStructureCacheLength */
/* start ReadRegisterDynamicBindingCacheBase */


ReadRegisterDynamicBindingCacheBase:
  if (_trace) printf("ReadRegisterDynamicBindingCacheBase:\n");
  t3 = *(u64 *)&(processor->dbcbase);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterDynamicBindingCacheBase */
/* start ReadRegisterDynamicBindingCacheMask */


ReadRegisterDynamicBindingCacheMask:
  if (_trace) printf("ReadRegisterDynamicBindingCacheMask:\n");
  t3 = *(u64 *)&(processor->dbcmask);   
  iSP = iSP + 8;
  t5 = t3 << 26;   
  t5 = t5 >> 26;   
  *(u64 *)iSP = t5;   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterDynamicBindingCacheMask */
/* start ReadRegisterChoicePointer */


ReadRegisterChoicePointer:
  if (_trace) printf("ReadRegisterChoicePointer:\n");
  t3 = *(s32 *)&processor->choiceptr;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterChoicePointer */
/* start ReadRegisterStructureStackChoicePointer */


ReadRegisterStructureStackChoicePointer:
  if (_trace) printf("ReadRegisterStructureStackChoicePointer:\n");
  t3 = *(s32 *)&processor->sstkchoiceptr;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStructureStackChoicePointer */
/* start ReadRegisterFEPModeTrapVectorAddress */


ReadRegisterFEPModeTrapVectorAddress:
  if (_trace) printf("ReadRegisterFEPModeTrapVectorAddress:\n");
  t3 = *(u64 *)&(processor->fepmodetrapvecaddress);   
  goto NEXTINSTRUCTION;   

/* end ReadRegisterFEPModeTrapVectorAddress */
/* start ReadRegisterStackFrameMaximumSize */


ReadRegisterStackFrameMaximumSize:
  if (_trace) printf("ReadRegisterStackFrameMaximumSize:\n");
  t3 = zero + 128;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStackFrameMaximumSize */
/* start ReadRegisterStackCacheDumpQuantum */


ReadRegisterStackCacheDumpQuantum:
  if (_trace) printf("ReadRegisterStackCacheDumpQuantum:\n");
  t3 = zero + 896;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStackCacheDumpQuantum */
/* start ReadRegisterConstantNIL */


ReadRegisterConstantNIL:
  if (_trace) printf("ReadRegisterConstantNIL:\n");
  t5 = *(u64 *)&(processor->taddress);   
  *(u64 *)(iSP + 8) = t5;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterConstantNIL */
/* start ReadRegisterConstantT */


ReadRegisterConstantT:
  if (_trace) printf("ReadRegisterConstantT:\n");
  t5 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t5;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterConstantT */
/* start ReadRegisterError */


ReadRegisterError:
  if (_trace) printf("ReadRegisterError:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end ReadRegisterError */
/* start WriteRegisterFP */


WriteRegisterFP:
  if (_trace) printf("WriteRegisterFP:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end WriteRegisterFP */
/* start WriteRegisterLP */


WriteRegisterLP:
  if (_trace) printf("WriteRegisterLP:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end WriteRegisterLP */
/* start WriteRegisterSP */


WriteRegisterSP:
  if (_trace) printf("WriteRegisterSP:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end WriteRegisterSP */
/* start WriteRegisterStackCacheLowerBound */


WriteRegisterStackCacheLowerBound:
  if (_trace) printf("WriteRegisterStackCacheLowerBound:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end WriteRegisterStackCacheLowerBound */
/* start WriteRegisterContinuation */


WriteRegisterContinuation:
  if (_trace) printf("WriteRegisterContinuation:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->continuation = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterContinuation */
/* start WriteRegisterAluAndRotateControl */


WriteRegisterAluAndRotateControl:
  if (_trace) printf("WriteRegisterAluAndRotateControl:\n");
  t1 = arg3 >> 14;   
  t1 = t1 & 3;		// Extract the function class bits 
  *(u64 *)&processor->aluandrotatecontrol = arg3;   
  t2 = arg3 >> 5;   
  t2 = t2 & 31;		// Extract the byte size 
  *(u64 *)&processor->aluop = t1;   
  t3 = arg3 & 31;		// Extract the Byte Rotate 
  *(u64 *)&processor->bytesize = t2;   
  *(u64 *)&processor->byterotate = t3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterAluAndRotateControl */
/* start WriteRegisterControlRegister */


WriteRegisterControlRegister:
  if (_trace) printf("WriteRegisterControlRegister:\n");
  *(u32 *)&processor->control = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterControlRegister */
/* start WriteRegisterEphemeralOldspaceRegister */


WriteRegisterEphemeralOldspaceRegister:
  if (_trace) printf("WriteRegisterEphemeralOldspaceRegister:\n");
  *(u64 *)&processor->ac0array = zero;   
  *(u64 *)&processor->ac1array = zero;   
  *(u64 *)&processor->ac2array = zero;   
  *(u64 *)&processor->ac3array = zero;   
  *(u64 *)&processor->ac4array = zero;   
  *(u64 *)&processor->ac5array = zero;   
  *(u64 *)&processor->ac6array = zero;   
  *(u64 *)&processor->ac7array = zero;   
  *(u32 *)&processor->ephemeraloldspace = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterEphemeralOldspaceRegister */
/* start WriteRegisterZoneOldspaceRegister */


WriteRegisterZoneOldspaceRegister:
  if (_trace) printf("WriteRegisterZoneOldspaceRegister:\n");
  *(u32 *)&processor->zoneoldspace = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterZoneOldspaceRegister */
/* start WriteRegisterFPCoprocessorPresent */


WriteRegisterFPCoprocessorPresent:
  if (_trace) printf("WriteRegisterFPCoprocessorPresent:\n");
  goto NEXTINSTRUCTION;   

/* end WriteRegisterFPCoprocessorPresent */
/* start WriteRegisterPreemptRegister */


WriteRegisterPreemptRegister:
  if (_trace) printf("WriteRegisterPreemptRegister:\n");
  t3 = *(s32 *)&processor->interruptreg;   
  t3 = t3 & ~3L;
  arg3 = arg3 & 3;
  t3 = t3 | arg3;
  *(u32 *)&processor->interruptreg = t3;   
  if ((t3 & 1) == 0)   
    goto NEXTINSTRUCTION;
  *(u64 *)&processor->stop_interpreter = t3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterPreemptRegister */
/* start WriteRegisterStackCacheOverflowLimit */


WriteRegisterStackCacheOverflowLimit:
  if (_trace) printf("WriteRegisterStackCacheOverflowLimit:\n");
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t1 = (u32)t1;   
  t1 = arg3 - t1;   
  *(u32 *)&processor->scovlimit = t1;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterStackCacheOverflowLimit */
/* start WriteRegisterTOS */


WriteRegisterTOS:
  if (_trace) printf("WriteRegisterTOS:\n");
  goto NEXTINSTRUCTION;   

/* end WriteRegisterTOS */
/* start WriteRegisterEventCount */


WriteRegisterEventCount:
  if (_trace) printf("WriteRegisterEventCount:\n");
  *(u64 *)&processor->areventcount = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterEventCount */
/* start WriteRegisterBindingStackPointer */


WriteRegisterBindingStackPointer:
  if (_trace) printf("WriteRegisterBindingStackPointer:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->bindingstackpointer = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterBindingStackPointer */
/* start WriteRegisterCatchBlockList */


WriteRegisterCatchBlockList:
  if (_trace) printf("WriteRegisterCatchBlockList:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->catchblock = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterCatchBlockList */
/* start WriteRegisterControlStackLimit */


WriteRegisterControlStackLimit:
  if (_trace) printf("WriteRegisterControlStackLimit:\n");
  *(u32 *)&processor->cslimit = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterControlStackLimit */
/* start WriteRegisterControlStackExtraLimit */


WriteRegisterControlStackExtraLimit:
  if (_trace) printf("WriteRegisterControlStackExtraLimit:\n");
  *(u32 *)&processor->csextralimit = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterControlStackExtraLimit */
/* start WriteRegisterBindingStackLimit */


WriteRegisterBindingStackLimit:
  if (_trace) printf("WriteRegisterBindingStackLimit:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->bindingstacklimit = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterBindingStackLimit */
/* start WriteRegisterListCacheArea */


WriteRegisterListCacheArea:
  if (_trace) printf("WriteRegisterListCacheArea:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->lcarea = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterListCacheArea */
/* start WriteRegisterListCacheAddress */


WriteRegisterListCacheAddress:
  if (_trace) printf("WriteRegisterListCacheAddress:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->lcaddress = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterListCacheAddress */
/* start WriteRegisterListCacheLength */


WriteRegisterListCacheLength:
  if (_trace) printf("WriteRegisterListCacheLength:\n");
  *(u32 *)&processor->lclength = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterListCacheLength */
/* start WriteRegisterStructureCacheArea */


WriteRegisterStructureCacheArea:
  if (_trace) printf("WriteRegisterStructureCacheArea:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->scarea = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterStructureCacheArea */
/* start WriteRegisterStructureCacheAddress */


WriteRegisterStructureCacheAddress:
  if (_trace) printf("WriteRegisterStructureCacheAddress:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->scaddress = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterStructureCacheAddress */
/* start WriteRegisterStructureCacheLength */


WriteRegisterStructureCacheLength:
  if (_trace) printf("WriteRegisterStructureCacheLength:\n");
  *(u32 *)&processor->sclength = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterStructureCacheLength */
/* start WriteRegisterDynamicBindingCacheBase */


WriteRegisterDynamicBindingCacheBase:
  if (_trace) printf("WriteRegisterDynamicBindingCacheBase:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->dbcbase = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterDynamicBindingCacheBase */
/* start WriteRegisterDynamicBindingCacheMask */


WriteRegisterDynamicBindingCacheMask:
  if (_trace) printf("WriteRegisterDynamicBindingCacheMask:\n");
  arg4 = arg2 << 32;   
  arg4 = arg4 | arg3;		// construct the combined word 
  *(u64 *)&processor->dbcmask = arg4;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterDynamicBindingCacheMask */
/* start WriteRegisterChoicePointer */


WriteRegisterChoicePointer:
  if (_trace) printf("WriteRegisterChoicePointer:\n");
  *(u32 *)&processor->choiceptr = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterChoicePointer */
/* start WriteRegisterStructureStackChoicePointer */


WriteRegisterStructureStackChoicePointer:
  if (_trace) printf("WriteRegisterStructureStackChoicePointer:\n");
  *(u32 *)&processor->sstkchoiceptr = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterStructureStackChoicePointer */
/* start WriteRegisterFEPModeTrapVectorAddress */


WriteRegisterFEPModeTrapVectorAddress:
  if (_trace) printf("WriteRegisterFEPModeTrapVectorAddress:\n");
  *(u32 *)&processor->fepmodetrapvecaddress = arg3;   
  goto NEXTINSTRUCTION;   

/* end WriteRegisterFEPModeTrapVectorAddress */
/* start WriteRegisterMappingTableCache */


WriteRegisterMappingTableCache:
  if (_trace) printf("WriteRegisterMappingTableCache:\n");
  goto NEXTINSTRUCTION;   

/* end WriteRegisterMappingTableCache */
/* start WriteRegisterError */


WriteRegisterError:
  if (_trace) printf("WriteRegisterError:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end WriteRegisterError */
  /* Coprocessor read and write are implemented in C in order to */
  /* encourage creativity!  The hooks are in aicoproc.c */
/* start DoCoprocessorRead */

  /* Halfword 10 bit immediate instruction - DoCoprocessorRead */

docoprocessorread:
  if (_trace) printf("docoprocessorread:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoCoprocessorReadIM:
  if (_trace) printf("DoCoprocessorReadIM:\n");

DoCoprocessorReadSP:
  if (_trace) printf("DoCoprocessorReadSP:\n");

DoCoprocessorReadLP:
  if (_trace) printf("DoCoprocessorReadLP:\n");

DoCoprocessorReadFP:
  if (_trace) printf("DoCoprocessorReadFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  r0 = *(u64 *)&(processor->coprocessorreadhook);   
  *(u64 *)&processor->cp = iCP;   
  *(u64 *)&processor->epc = iPC;   
  *(u64 *)&processor->sp = iSP;   
  *(u64 *)&processor->fp = iFP;   
  *(u64 *)&processor->lp = iLP;   
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  pv = r0;
    r0 = (*( u64 (*)(u64, u64) )r0)(arg1, arg2); /* jsr */  
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  iCP = *(u64 *)&(processor->cp);   
  iPC = *(u64 *)&(processor->epc);   
  iSP = *(u64 *)&(processor->sp);   
  iFP = *(u64 *)&(processor->fp);   
  iLP = *(u64 *)&(processor->lp);   
  /* Long -1 is never a valid LISP value */
  t1 = zero + -1;   
  t1 = (r0 == t1) ? 1 : 0;   
  if (t1 != 0)   		// J. if CoprocessorRead exception return 
    goto cpreadexc;
  iSP = iSP + 8;
  t1 = r0 << 26;   
  t1 = t1 >> 26;   
  *(u64 *)iSP = t1;   		// Push the result of coprocessor read! 
  goto NEXTINSTRUCTION;   

cpreadexc:
  if (_trace) printf("cpreadexc:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end DoCoprocessorRead */
  /* End of Halfword operand from stack instruction - DoCoprocessorRead */
/* start DoCoprocessorWrite */

  /* Halfword 10 bit immediate instruction - DoCoprocessorWrite */

docoprocessorwrite:
  if (_trace) printf("docoprocessorwrite:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoCoprocessorWriteIM:
  if (_trace) printf("DoCoprocessorWriteIM:\n");

DoCoprocessorWriteSP:
  if (_trace) printf("DoCoprocessorWriteSP:\n");

DoCoprocessorWriteLP:
  if (_trace) printf("DoCoprocessorWriteLP:\n");

DoCoprocessorWriteFP:
  if (_trace) printf("DoCoprocessorWriteFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = *(u64 *)iSP;   		// The value to be written 
  iSP = iSP - 8;   		// Pop Stack. 
  t2 = zero + CoprocessorRegister_UnwindStackForRestartOrApply;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch32318;
  /* Here if argument CoprocessorRegisterUnwindStackForRestartOrApply */
  t1 = *(s32 *)iSP;   		// peek at new continuation to look at tag 
  t2 = *(s32 *)(iSP + 4);   
  t1 = (u32)t1;   
  t3 = t2 - Type_EvenPC;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto unwindillegalcontinuation;
  t1 = *(u64 *)iSP;   		// Get new continuation 
  iSP = iSP - 8;   		// Pop Stack. 
  *(u64 *)&processor->continuation = t1;   		// Update continuation register 
  *(u64 *)&processor->continuationcp = zero;   
  t1 = *(s32 *)iSP;   		// Get new FP 
  t2 = *(s32 *)(iSP + 4);   		// Get new FP 
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  t3 = t2 - Type_Locative;   
  t3 = t3 & 63;		// Strip CDR code 
  if (t3 != 0)   
    goto unwindillegalfp;
  /* Convert VMA to stack cache address */
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  iFP = *(u64 *)&(processor->stackcachedata);   
  t2 = t1 - t2;   		// stack cache base relative offset 
  iFP = (t2 * 8) + iFP;  		// reconstruct SCA 
  t1 = *(s32 *)iSP;   		// Get new LP 
  t2 = *(s32 *)(iSP + 4);   		// Get new LP 
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  t3 = t2 - Type_Locative;   
  t3 = t3 & 63;		// Strip CDR code 
  if (t3 != 0)   
    goto unwindillegallp;
  /* Convert VMA to stack cache address */
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  iLP = *(u64 *)&(processor->stackcachedata);   
  t2 = t1 - t2;   		// stack cache base relative offset 
  iLP = (t2 * 8) + iLP;  		// reconstruct SCA 
  /* Update CDR-CODEs to make it a legitimate frame */
  t1 = *(s32 *)(iFP + 4);   		// Tag of saved continuation register 
  t2 = *(s32 *)(iFP + 12);   		// Tag of saved control register 
  t1 = t1 | 192;		// Set CDR-CODE to 3 
  *(u32 *)(iFP + 4) = t1;   		// Put it back 
  t2 = t2 | 192;		// Set CDR-CODE to 3 
  *(u32 *)(iFP + 12) = t2;   		// Put it back 
  /* Copy the current trap-on-exit bit into the saved control register */
  t1 = *(s32 *)&processor->control;   		// Get control register 
  t2 = *(s32 *)(iFP + 8);   		// Get saved control register 
  t2 = (u32)t2;   
  t3 = (256) << 16;   
  t2 = t2 & ~t3;		// Remove saved control register's trap-on-exit bit 
  t1 = t1 & t3;		// Extract control register's trap-on-exit bit 
  t2 = t2 | t1;		// Copy it into saved control register 
  *(u32 *)(iFP + 8) = t2;   		// Update saved control register 
  /* Restore the new control register with proper trap mode */
  t1 = *(s32 *)iSP;   		// peek at new control register to look at tag 
  t2 = *(s32 *)(iSP + 4);   
  t1 = (u32)t1;   
  t3 = t2 - Type_Fixnum;   
  t3 = t3 & 63;		// Strip CDR code 
  if (t3 != 0)   
    goto unwindillegalcontrol;
  t1 = *(s32 *)iSP;   		// Get new control register 
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  *(u32 *)&processor->control = t1;   
  goto mondo_dispatch32317;   

mondo_dispatch32318:
  if (_trace) printf("mondo_dispatch32318:\n");
  t2 = zero + CoprocessorRegister_FlushIDCaches;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch32319;
  /* Here if argument CoprocessorRegisterFlushIDCaches */
  /* We're about to flush the instruction cache so we can't rely */
  /* on ContinueToNextInstruction working.  Instead, we must load */
  /* the next PC now and explicitly fill the cache. */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = *(u64 *)&(processor->flushcaches_hook);   
  *(u64 *)&processor->cp = iCP;   
  *(u64 *)&processor->epc = iPC;   
  *(u64 *)&processor->sp = iSP;   
  *(u64 *)&processor->fp = iFP;   
  *(u64 *)&processor->lp = iLP;   
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  pv = t1;
    r0 = (*( u64 (*)(u64, u64) )t1)(arg1, arg2); /* jsr */  
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  iCP = *(u64 *)&(processor->cp);   
  iPC = *(u64 *)&(processor->epc);   
  iSP = *(u64 *)&(processor->sp);   
  iFP = *(u64 *)&(processor->fp);   
  iLP = *(u64 *)&(processor->lp);   
  /* Compute proper iCP after FlushCaches resets it. */
  goto ICACHEMISS;
  goto mondo_dispatch32317;   

mondo_dispatch32319:
  if (_trace) printf("mondo_dispatch32319:\n");
  t2 = zero + CoprocessorRegister_FlushCachesForVMA;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch32320;
  /* Here if argument CoprocessorRegisterFlushCachesForVMA */
  arg2 = (u32)arg2;   		// Extract the VMA 
  t1 = arg2 << 1;   		// convert continuation to an even pc 
  /* Convert a halfword address into a CP pointer. */
  t2 = t1 >> (CacheLine_RShift & 63);   		// Get third byte into bottom 
  t4 = *(u64 *)&(processor->icachebase);   		// get the base of the icache 
  t3 = zero + -1;   
  t3 = t3 + ((4) << 16);   
  t2 = t2 << (CacheLine_LShift & 63);   		// Now third byte is zero-shifted 
  t2 = t1 + t2;
  t2 = t2 & t3;
  t3 = t2 << 5;   		// temp=cpos*32 
  t2 = t2 << 4;   		// cpos=cpos*16 
  t4 = t4 + t3;		// temp2=base+cpos*32 
  t2 = t4 + t2;		// cpos=base+cpos*48 
  t3 = *(u64 *)&(((CACHELINEP)t2)->pcdata);   
  t3 = (t1 == t3) ? 1 : 0;   		// Is this VMA in the cache? 
  if (t3 == 0) 		// No. 
    goto dcwnotincache;
  *(u64 *)&((CACHELINEP)t2)->pcdata = zero;   		// Yes, flush it 
  *((u64 *)(&((CACHELINEP)t2)->pcdata)+CACHELINESIZE/8) = zero;   

dcwnotincache:
  if (_trace) printf("dcwnotincache:\n");
  goto mondo_dispatch32317;   

mondo_dispatch32320:
  if (_trace) printf("mondo_dispatch32320:\n");
  t2 = zero + CoprocessorRegister_FlushHiddenArrayRegisters;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch32321;
  /* Here if argument CoprocessorRegisterFlushHiddenArrayRegisters */
  arg2 = (u32)arg2;   		// Get the VMA of the new stack array 
  t8 = zero + AutoArrayRegMask;   
  t8 = arg2 & t8;
  t7 = (u64)&processor->ac0array;   
  t7 = t7 + t8;		// Here is our array register block 
  t8 = *(u64 *)&(((ARRAYCACHEP)t7)->array);   		// And here is the cached array 
  t8 = (arg2 == t8) ? 1 : 0;   		// t8==1 iff cached array is ours 
  if (t8 == 0) 
    goto arraynotincache;
  *(u64 *)&((ARRAYCACHEP)t7)->array = zero;   		// Flush it 

arraynotincache:
  if (_trace) printf("arraynotincache:\n");
  goto mondo_dispatch32317;   

mondo_dispatch32321:
  if (_trace) printf("mondo_dispatch32321:\n");
  /* Here for all other cases */
  /* Standard coprocessor register processing */
  r0 = *(u64 *)&(processor->coprocessorwritehook);   
  *(u64 *)&processor->cp = iCP;   
  *(u64 *)&processor->epc = iPC;   
  *(u64 *)&processor->sp = iSP;   
  *(u64 *)&processor->fp = iFP;   
  *(u64 *)&processor->lp = iLP;   
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  pv = r0;
    r0 = (*( u64 (*)(u64, u64) )r0)(arg1, arg2); /* jsr */  
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  iCP = *(u64 *)&(processor->cp);   
  iPC = *(u64 *)&(processor->epc);   
  iSP = *(u64 *)&(processor->sp);   
  iFP = *(u64 *)&(processor->fp);   
  iLP = *(u64 *)&(processor->lp);   
  if (r0 == 0) 		// J. if CoprocessorWrite exception return 
    goto cpreadexc;
  goto mondo_dispatch32317;   

mondo_dispatch32322:
  if (_trace) printf("mondo_dispatch32322:\n");

mondo_dispatch32317:
  if (_trace) printf("mondo_dispatch32317:\n");
  goto NEXTINSTRUCTION;   

unwindillegalcontinuation:
  if (_trace) printf("unwindillegalcontinuation:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

unwindillegalcontrol:
  if (_trace) printf("unwindillegalcontrol:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

unwindillegalfp:
  if (_trace) printf("unwindillegalfp:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

unwindillegallp:
  if (_trace) printf("unwindillegallp:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

cpwriteexc:
  if (_trace) printf("cpwriteexc:\n");
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;

/* end DoCoprocessorWrite */
  /* End of Halfword operand from stack instruction - DoCoprocessorWrite */
/* start GetRPCC */


getrpcc:
  if (_trace) printf("getrpcc:\n");
  r0 = RPCC();
  arg1 = r0 << 32;   
  arg1 = r0 + arg1;
  r0 = arg1 >> 32;   
  goto *ra; /* ret */

/* end GetRPCC */
/* start SpinWheels */


spinwheels:
  if (_trace) printf("spinwheels:\n");
  arg1 = 1;
  arg1 = arg1 << 25;   

spinwheelaxis:
  if (_trace) printf("spinwheelaxis:\n");
  arg1 = arg1 + -1;
  if ((s64)arg1 > 0)   
    goto spinwheelaxis;
  goto *ra; /* ret */

/* end SpinWheels */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunsubp.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunfext.as
 ************************************************************************/

  /* Field extraction instruction. */
/* start DoCharLdb */

  /* Field Extraction instruction - DoCharLdb */

docharldb:
  if (_trace) printf("docharldb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoCharLdbIM:
  if (_trace) printf("DoCharLdbIM:\n");

DoCharLdbSP:
  if (_trace) printf("DoCharLdbSP:\n");

DoCharLdbLP:
  if (_trace) printf("DoCharLdbLP:\n");

DoCharLdbFP:
  if (_trace) printf("DoCharLdbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t7 = zero - 1;   		// t7= -1 
  arg3 = *(s32 *)(iSP + 4);   		// get ARG1 tag/data 
  arg4 = *(s32 *)iSP;   
  arg1 = arg1 + 1;   		// Size of field 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  /* TagType. */
  t8 = arg3 & 63;
  t9 = t8 - Type_Character;   
  arg4 = (u32)arg4;   		// Clear sign extension now 
  if (t9 != 0)   		// Not a character 
    goto charldbexc;
  t4 = arg4 << (arg2 & 63);   		// T4= shifted value if PP==0 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = t4 >> 32;   		// T5= shifted value if PP<>0 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (arg2 == 0)   		// T5= shifted value 
    t5 = t4;
  t3 = t5 & ~t7;		// T3= masked value. 
  t4 = Type_Fixnum;
  *(u32 *)iSP = t3;   
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

charldbexc:
  if (_trace) printf("charldbexc:\n");
  arg5 = 0;
  arg2 = 28;
  goto illegaloperand;

/* end DoCharLdb */
  /* End of Halfword operand from stack instruction - DoCharLdb */
/* start DoPLdb */

  /* Field Extraction instruction - DoPLdb */

dopldb:
  if (_trace) printf("dopldb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPLdbIM:
  if (_trace) printf("DoPLdbIM:\n");

DoPLdbSP:
  if (_trace) printf("DoPLdbSP:\n");

DoPLdbLP:
  if (_trace) printf("DoPLdbLP:\n");

DoPLdbFP:
  if (_trace) printf("DoPLdbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t2 = *(s32 *)iSP;   		// get arg1 tag/data 
  t1 = *(s32 *)(iSP + 4);   
  t2 = (u32)t2;   
  t3 = t1 - Type_PhysicalAddress;   
  t3 = t3 & 63;
  if (t3 == 0) 
    goto pldbillop;
  /* Memory Read Internal */

vma_memory_read32324:
  t3 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t5 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;   
  arg4 = (t5 * 4);   
  arg3 = LDQ_U(t5);   
  t3 = t2 - t3;   		// Stack cache offset 
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;   		// In range? 
  arg4 = *(s32 *)arg4;   
  arg3 = (u8)(arg3 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read32326;

vma_memory_read32325:
  arg4 = (u32)arg4;   

vma_memory_read32332:
  t7 = zero - 1;   		// t7= -1 
  arg1 = arg1 + 1;		// Size of field 
  t4 = arg4 << (arg2 & 63);   		// T4= shifted value if PP==0 
  t5 = t4 >> 32;   		// T5= shifted value if PP<>0 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  if (arg2 == 0)   		// T5= shifted value 
    t5 = t4;
  t3 = t5 & ~t7;		// T3= masked value. 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Fixnum;
  *(u32 *)iSP = t3;   
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

pldbillop:
  if (_trace) printf("pldbillop:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = iSP - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

vma_memory_read32326:
  if (_trace) printf("vma_memory_read32326:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg4 = *(s32 *)t3;   
  arg3 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read32325;   

/* end DoPLdb */
  /* End of Halfword operand from stack instruction - DoPLdb */
/* start DoPTagLdb */

  /* Field Extraction instruction - DoPTagLdb */

doptagldb:
  if (_trace) printf("doptagldb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPTagLdbIM:
  if (_trace) printf("DoPTagLdbIM:\n");

DoPTagLdbSP:
  if (_trace) printf("DoPTagLdbSP:\n");

DoPTagLdbLP:
  if (_trace) printf("DoPTagLdbLP:\n");

DoPTagLdbFP:
  if (_trace) printf("DoPTagLdbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t2 = *(s32 *)iSP;   		// get arg1 tag/data 
  t1 = *(s32 *)(iSP + 4);   
  t2 = (u32)t2;   
  t3 = t1 - Type_PhysicalAddress;   
  t3 = t3 & 63;
  if (t3 == 0) 
    goto ptagldbillop;
  /* Memory Read Internal */

vma_memory_read32333:
  t3 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t5 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;   
  arg4 = (t5 * 4);   
  arg3 = LDQ_U(t5);   
  t3 = t2 - t3;   		// Stack cache offset 
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;   		// In range? 
  arg4 = *(s32 *)arg4;   
  arg3 = (u8)(arg3 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read32335;

vma_memory_read32334:

vma_memory_read32341:
  t7 = zero - 1;   		// t7= -1 
  arg1 = arg1 + 1;		// Size of field 
  t4 = arg3 << (arg2 & 63);   		// T4= shifted value if PP==0 
  t5 = t4 >> 32;   		// T5= shifted value if PP<>0 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  if (arg2 == 0)   		// T5= shifted value 
    t5 = t4;
  t3 = t5 & ~t7;		// T3= masked value. 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Fixnum;
  *(u32 *)iSP = t3;   
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

ptagldbillop:
  if (_trace) printf("ptagldbillop:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = iSP - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

vma_memory_read32335:
  if (_trace) printf("vma_memory_read32335:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg4 = *(s32 *)t3;   
  arg3 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read32334;   

/* end DoPTagLdb */
  /* End of Halfword operand from stack instruction - DoPTagLdb */
/* start DoDpb */

  /* Field Extraction instruction - DoDpb */

dodpb:
  if (_trace) printf("dodpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoDpbIM:
  if (_trace) printf("DoDpbIM:\n");

DoDpbSP:
  if (_trace) printf("DoDpbSP:\n");

DoDpbLP:
  if (_trace) printf("DoDpbLP:\n");

DoDpbFP:
  if (_trace) printf("DoDpbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t6 = *(s32 *)iSP;   		// Get arg2 tag/data 
  t5 = *(s32 *)(iSP + 4);   		// Get arg2 tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  t6 = (u32)t6;   
  arg4 = *(s32 *)iSP;   		// get arg1 tag/data 
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  t1 = t5 & 63;		// Strip off any CDR code bits. 
  arg6 = arg3 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment32354:
  if (_trace) printf("force_alignment32354:\n");
  if (t2 == 0) 
    goto basic_dispatch32347;
  /* Here if argument TypeFixnum */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;   

force_alignment32351:
  if (_trace) printf("force_alignment32351:\n");
  if (arg5 == 0) 
    goto binary_type_dispatch32344;
  /* Here if argument TypeFixnum */
  t7 = zero - 2;   		// t7= -2 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  t5 = ~t7;   		// reuse t5 as mask 
  t3 = arg4 & ~t7;		// T3= masked new value. 
  t5 = t5 << (arg2 & 63);   		// t5 is the inplace mask 
  t4 = t3 << (arg2 & 63);   		// t4 is the shifted field 
  t6 = t6 & ~t5;		// Clear out existing bits in arg2 field 
  t6 = t4 | t6;		// Put the new bits in 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Fixnum;
  *(u32 *)iSP = t6;   
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch32348:
  if (_trace) printf("basic_dispatch32348:\n");

basic_dispatch32347:
  if (_trace) printf("basic_dispatch32347:\n");
  /* Here for all other cases */

binary_type_dispatch32343:
  if (_trace) printf("binary_type_dispatch32343:\n");
  arg6 = t5;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch32345;   

binary_type_dispatch32344:
  if (_trace) printf("binary_type_dispatch32344:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch32345:
  if (_trace) printf("binary_type_dispatch32345:\n");

basic_dispatch32346:
  if (_trace) printf("basic_dispatch32346:\n");

/* end DoDpb */
  /* End of Halfword operand from stack instruction - DoDpb */
/* start DoCharDpb */

  /* Field Extraction instruction - DoCharDpb */

dochardpb:
  if (_trace) printf("dochardpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoCharDpbIM:
  if (_trace) printf("DoCharDpbIM:\n");

DoCharDpbSP:
  if (_trace) printf("DoCharDpbSP:\n");

DoCharDpbLP:
  if (_trace) printf("DoCharDpbLP:\n");

DoCharDpbFP:
  if (_trace) printf("DoCharDpbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t6 = *(s32 *)iSP;   		// Get arg2 tag/data 
  t5 = *(s32 *)(iSP + 4);   		// Get arg2 tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  t6 = (u32)t6;   
  arg4 = *(s32 *)iSP;   		// get arg1 tag/data 
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  t1 = t5 & 63;		// Strip off any CDR code bits. 
  arg6 = arg3 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_Character) ? 1 : 0;   

force_alignment32367:
  if (_trace) printf("force_alignment32367:\n");
  if (t2 == 0) 
    goto basic_dispatch32360;
  /* Here if argument TypeCharacter */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;   

force_alignment32364:
  if (_trace) printf("force_alignment32364:\n");
  if (arg5 == 0) 
    goto binary_type_dispatch32357;
  /* Here if argument TypeFixnum */
  t7 = zero - 2;   		// t7= -2 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  t5 = ~t7;   		// reuse t5 as mask 
  t3 = arg4 & ~t7;		// T3= masked new value. 
  t5 = t5 << (arg2 & 63);   		// t5 is the inplace mask 
  t4 = t3 << (arg2 & 63);   		// t4 is the shifted field 
  t6 = t6 & ~t5;		// Clear out existing bits in arg2 field 
  t6 = t4 | t6;		// Put the new bits in 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = Type_Character;
  *(u32 *)iSP = t6;   
  *(u32 *)(iSP + 4) = t4;   		// write the stack cache 
  goto cachevalid;   

basic_dispatch32361:
  if (_trace) printf("basic_dispatch32361:\n");

basic_dispatch32360:
  if (_trace) printf("basic_dispatch32360:\n");
  /* Here for all other cases */

binary_type_dispatch32356:
  if (_trace) printf("binary_type_dispatch32356:\n");
  arg6 = t5;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 27;
  goto spareexception;
  goto binary_type_dispatch32358;   

binary_type_dispatch32357:
  if (_trace) printf("binary_type_dispatch32357:\n");
  arg5 = 0;
  arg2 = 27;
  goto illegaloperand;

binary_type_dispatch32358:
  if (_trace) printf("binary_type_dispatch32358:\n");

basic_dispatch32359:
  if (_trace) printf("basic_dispatch32359:\n");

/* end DoCharDpb */
  /* End of Halfword operand from stack instruction - DoCharDpb */
/* start DoPDpb */

  /* Field Extraction instruction - DoPDpb */

dopdpb:
  if (_trace) printf("dopdpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPDpbIM:
  if (_trace) printf("DoPDpbIM:\n");

DoPDpbSP:
  if (_trace) printf("DoPDpbSP:\n");

DoPDpbLP:
  if (_trace) printf("DoPDpbLP:\n");

DoPDpbFP:
  if (_trace) printf("DoPDpbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t2 = *(s32 *)iSP;   		// Get arg2 tag/data 
  t1 = *(s32 *)(iSP + 4);   		// Get arg2 tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  t2 = (u32)t2;   
  t3 = t1 - Type_PhysicalAddress;   
  t3 = t3 & 63;
  if (t3 == 0) 
    goto pdpbillop;
  arg4 = *(s32 *)iSP;   		// get arg1 tag/data 
  arg3 = *(s32 *)(iSP + 4);   		// get arg1 tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  /* Memory Read Internal */

vma_memory_read32368:
  t3 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t1 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;   
  t6 = (t1 * 4);   
  t8 = LDQ_U(t1);   
  t3 = t2 - t3;   		// Stack cache offset 
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t8 = (u8)(t8 >> ((t1&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read32370;

vma_memory_read32369:
  t6 = (u32)t6;   

vma_memory_read32376:
  t6 = (u32)t6;   
  t1 = arg3 & 63;		// Strip off any CDR code bits. 
  t10 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment32383:
  if (_trace) printf("force_alignment32383:\n");
  if (t10 == 0) 
    goto basic_dispatch32378;
  /* Here if argument TypeFixnum */
  t7 = zero - 2;   		// t7= -2 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  t5 = ~t7;   		// reuse t5 as mask 
  t3 = arg4 & ~t7;		// T3= masked new value. 
  t5 = t5 << (arg2 & 63);   		// t5 is the inplace mask 
  t4 = t3 << (arg2 & 63);   		// t4 is the shifted field 
  t6 = t6 & ~t5;		// Clear out existing bits in arg2 field 
  t6 = t4 | t6;		// Put the new bits in 
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t2 + ivory;
  t10 = *(s32 *)&processor->scovlimit;   
  t5 = (t3 * 4);   
  t1 = LDQ_U(t3);   
  t4 = t2 - t4;   		// Stack cache offset 
  t10 = ((u64)t4 < (u64)t10) ? 1 : 0;   		// In range? 
  t4 = (t8 & 0xff) << ((t3&7)*8);   
  t1 = t1 & ~(0xffL << (t3&7)*8);   

force_alignment32380:
  if (_trace) printf("force_alignment32380:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);   
  *(u32 *)t5 = t6;   
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write32379;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

basic_dispatch32378:
  if (_trace) printf("basic_dispatch32378:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

basic_dispatch32377:
  if (_trace) printf("basic_dispatch32377:\n");

pdpbillop:
  if (_trace) printf("pdpbillop:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = iSP - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

vma_memory_write32379:
  if (_trace) printf("vma_memory_write32379:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32384:
  if (_trace) printf("force_alignment32384:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = t2 - t4;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
  *(u32 *)t3 = t6;   		// Store in stack 
  *(u32 *)(t3 + 4) = t8;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32370:
  if (_trace) printf("vma_memory_read32370:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t6 = *(s32 *)t3;   
  t8 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read32369;   

/* end DoPDpb */
  /* End of Halfword operand from stack instruction - DoPDpb */
/* start DoPTagDpb */

  /* Field Extraction instruction - DoPTagDpb */

doptagdpb:
  if (_trace) printf("doptagdpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoPTagDpbIM:
  if (_trace) printf("DoPTagDpbIM:\n");

DoPTagDpbSP:
  if (_trace) printf("DoPTagDpbSP:\n");

DoPTagDpbLP:
  if (_trace) printf("DoPTagDpbLP:\n");

DoPTagDpbFP:
  if (_trace) printf("DoPTagDpbFP:\n");
  arg1 = arg3 >> 37;   		// Shift the 'size-1' bits into place 
  arg2 = arg2 & 31;		// mask out the unwanted bits in arg2 
  arg1 = arg1 & 31;		// mask out the unwanted bits in arg1 
  /* arg1 has size-1, arg2 has position. */
  t2 = *(s32 *)iSP;   		// Get arg2 tag/data 
  t1 = *(s32 *)(iSP + 4);   		// Get arg2 tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  t2 = (u32)t2;   
  t3 = t1 - Type_PhysicalAddress;   
  t3 = t3 & 63;
  if (t3 == 0) 
    goto ptagdpbillop;
  arg4 = *(s32 *)iSP;   		// get arg1 tag/data 
  arg3 = *(s32 *)(iSP + 4);   		// get arg1 tag/data 
  iSP = iSP - 8;   		// Pop Stack. 
  arg4 = (u32)arg4;   
  /* Memory Read Internal */

vma_memory_read32385:
  t3 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t1 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;   
  t8 = (t1 * 4);   
  t6 = LDQ_U(t1);   
  t3 = t2 - t3;   		// Stack cache offset 
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;   		// In range? 
  t8 = *(s32 *)t8;   
  t6 = (u8)(t6 >> ((t1&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read32387;

vma_memory_read32386:

vma_memory_read32393:
  t1 = arg3 & 63;		// Strip off any CDR code bits. 
  t10 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment32400:
  if (_trace) printf("force_alignment32400:\n");
  if (t10 == 0) 
    goto basic_dispatch32395;
  /* Here if argument TypeFixnum */
  t7 = zero - 2;   		// t7= -2 
  t7 = t7 << (arg1 & 63);   		// Unmask 
  t5 = ~t7;   		// reuse t5 as mask 
  t3 = arg4 & ~t7;		// T3= masked new value. 
  t5 = t5 << (arg2 & 63);   		// t5 is the inplace mask 
  t4 = t3 << (arg2 & 63);   		// t4 is the shifted field 
  t6 = t6 & ~t5;		// Clear out existing bits in arg2 field 
  t6 = t4 | t6;		// Put the new bits in 
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t2 + ivory;
  t10 = *(s32 *)&processor->scovlimit;   
  t5 = (t3 * 4);   
  t1 = LDQ_U(t3);   
  t4 = t2 - t4;   		// Stack cache offset 
  t10 = ((u64)t4 < (u64)t10) ? 1 : 0;   		// In range? 
  t4 = (t6 & 0xff) << ((t3&7)*8);   
  t1 = t1 & ~(0xffL << (t3&7)*8);   

force_alignment32397:
  if (_trace) printf("force_alignment32397:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);   
  *(u32 *)t5 = t8;   
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write32396;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

basic_dispatch32395:
  if (_trace) printf("basic_dispatch32395:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

basic_dispatch32394:
  if (_trace) printf("basic_dispatch32394:\n");

ptagdpbillop:
  if (_trace) printf("ptagdpbillop:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = iSP - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

vma_memory_write32396:
  if (_trace) printf("vma_memory_write32396:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32401:
  if (_trace) printf("force_alignment32401:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = t2 - t4;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
  *(u32 *)t3 = t8;   		// Store in stack 
  *(u32 *)(t3 + 4) = t6;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32387:
  if (_trace) printf("vma_memory_read32387:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t8 = *(s32 *)t3;   
  t6 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read32386;   

/* end DoPTagDpb */
  /* End of Halfword operand from stack instruction - DoPTagDpb */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunfext.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunlexi.as
 ************************************************************************/

  /* Lexical variable accessors. */
/* start DoPushLexicalVarN */

  /* Halfword operand from stack instruction - DoPushLexicalVarN */
  /* arg2 has the preloaded 8 bit operand. */

dopushlexicalvarn:
  if (_trace) printf("dopushlexicalvarn:\n");

DoPushLexicalVarNSP:
  if (_trace) printf("DoPushLexicalVarNSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushLexicalVarNLP:
  if (_trace) printf("DoPushLexicalVarNLP:\n");

DoPushLexicalVarNFP:
  if (_trace) printf("DoPushLexicalVarNFP:\n");

begindopushlexicalvarn:
  if (_trace) printf("begindopushlexicalvarn:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t4 = arg3 >> 10;   		// Position the opcode 
  t1 = *(s32 *)arg1;   
  t2 = *(s32 *)(arg1 + 4);   
  t4 = t4 & 7;		// Get the lexical var number 
  t1 = (u32)t1;   
  /* TagType. */
  t3 = t2 & 63;
  t3 = t3 - Type_List;   
  t3 = t3 & ~4L;
  t1 = t1 + t4;		// Compute the address of the lexical variable. 
  if (t3 != 0)   
    goto pushlexvariop;
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read32402:
  t6 = t1 + ivory;
  t3 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = t1 - arg5;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->dataread_mask);   
  t5 = ((u64)t4 < (u64)arg6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read32404;

vma_memory_read32403:
  t6 = zero + 240;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read32406;

vma_memory_read32413:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = t2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

pushlexvariop:
  if (_trace) printf("pushlexvariop:\n");
  arg5 = 0;
  arg2 = 82;
  goto illegaloperand;

vma_memory_read32404:
  if (_trace) printf("vma_memory_read32404:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t3 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read32403;   

vma_memory_read32406:
  if (_trace) printf("vma_memory_read32406:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read32405;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32402;   

vma_memory_read32405:
  if (_trace) printf("vma_memory_read32405:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read32410:
  if (_trace) printf("vma_memory_read32410:\n");
  t6 = t7 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read32409;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read32413;   

vma_memory_read32409:

vma_memory_read32408:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 0;
  goto performmemoryaction;

DoPushLexicalVarNIM:
  goto doistageerror;

/* end DoPushLexicalVarN */
  /* End of Halfword operand from stack instruction - DoPushLexicalVarN */
/* start DoPopLexicalVarN */

  /* Halfword operand from stack instruction - DoPopLexicalVarN */
  /* arg2 has the preloaded 8 bit operand. */

dopoplexicalvarn:
  if (_trace) printf("dopoplexicalvarn:\n");

DoPopLexicalVarNSP:
  if (_trace) printf("DoPopLexicalVarNSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPopLexicalVarNLP:
  if (_trace) printf("DoPopLexicalVarNLP:\n");

DoPopLexicalVarNFP:
  if (_trace) printf("DoPopLexicalVarNFP:\n");

begindopoplexicalvarn:
  if (_trace) printf("begindopoplexicalvarn:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t4 = arg3 >> 10;   		// Position the opcode 
  t1 = *(s32 *)arg1;   
  t2 = *(s32 *)(arg1 + 4);   
  t4 = t4 & 7;		// Get the lexical var number 
  t1 = (u32)t1;   
  /* TagType. */
  t3 = t2 & 63;
  t3 = t3 - Type_List;   
  t3 = t3 & ~4L;
  t1 = t1 + t4;		// Compute the address of the lexical variable. 
  if (t3 != 0)   
    goto poplexvariop;
  t3 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  t3 = (u32)t3;   
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read32414:
  t8 = t1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = t1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32416;

vma_memory_read32415:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read32418;

vma_memory_read32424:
  /* Merge cdr-code */
  t5 = t2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t6 = t1 + ivory;
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = t1 - arg5;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment32426:
  if (_trace) printf("force_alignment32426:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t3;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write32425;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

poplexvariop:
  if (_trace) printf("poplexvariop:\n");
  arg5 = 0;
  arg2 = 17;
  goto illegaloperand;

vma_memory_write32425:
  if (_trace) printf("vma_memory_write32425:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t3;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32416:
  if (_trace) printf("vma_memory_read32416:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read32415;   

vma_memory_read32418:
  if (_trace) printf("vma_memory_read32418:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read32417;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read32414;   

vma_memory_read32417:
  if (_trace) printf("vma_memory_read32417:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read32421:

vma_memory_read32420:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

DoPopLexicalVarNIM:
  goto doistageerror;

/* end DoPopLexicalVarN */
  /* End of Halfword operand from stack instruction - DoPopLexicalVarN */
/* start DoMovemLexicalVarN */

  /* Halfword operand from stack instruction - DoMovemLexicalVarN */
  /* arg2 has the preloaded 8 bit operand. */

domovemlexicalvarn:
  if (_trace) printf("domovemlexicalvarn:\n");

DoMovemLexicalVarNSP:
  if (_trace) printf("DoMovemLexicalVarNSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoMovemLexicalVarNLP:
  if (_trace) printf("DoMovemLexicalVarNLP:\n");

DoMovemLexicalVarNFP:
  if (_trace) printf("DoMovemLexicalVarNFP:\n");

begindomovemlexicalvarn:
  if (_trace) printf("begindomovemlexicalvarn:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  t4 = arg3 >> 10;   		// Position the opcode 
  t1 = *(s32 *)arg1;   
  t2 = *(s32 *)(arg1 + 4);   
  t4 = t4 & 7;		// Get the lexical var number 
  t1 = (u32)t1;   
  /* TagType. */
  t3 = t2 & 63;
  t3 = t3 - Type_List;   
  t3 = t3 & ~4L;
  t1 = t1 + t4;		// Compute the address of the lexical variable. 
  if (t3 != 0)   
    goto movemlexvariop;
  t3 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  t3 = (u32)t3;   
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read32427:
  t8 = t1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = t1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32429;

vma_memory_read32428:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read32431;

vma_memory_read32437:
  /* Merge cdr-code */
  t5 = t2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t6 = t1 + ivory;
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = t1 - arg5;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment32439:
  if (_trace) printf("force_alignment32439:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t3;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write32438;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

movemlexvariop:
  if (_trace) printf("movemlexvariop:\n");
  arg5 = 0;
  arg2 = 17;
  goto illegaloperand;

vma_memory_write32438:
  if (_trace) printf("vma_memory_write32438:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t3;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

vma_memory_read32429:
  if (_trace) printf("vma_memory_read32429:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read32428;   

vma_memory_read32431:
  if (_trace) printf("vma_memory_read32431:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read32430;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read32427;   

vma_memory_read32430:
  if (_trace) printf("vma_memory_read32430:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read32434:

vma_memory_read32433:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

DoMovemLexicalVarNIM:
  goto doistageerror;

/* end DoMovemLexicalVarN */
  /* End of Halfword operand from stack instruction - DoMovemLexicalVarN */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunlexi.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunbits.as
 ************************************************************************/

  /* Bits. */
/* start DoLogand */

  /* Halfword operand from stack instruction - DoLogand */
  /* arg2 has the preloaded 8 bit operand. */

dologand:
  if (_trace) printf("dologand:\n");

DoLogandSP:
  if (_trace) printf("DoLogandSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoLogandLP:
  if (_trace) printf("DoLogandLP:\n");

DoLogandFP:
  if (_trace) printf("DoLogandFP:\n");

headdologand:
  if (_trace) printf("headdologand:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindologand:
  if (_trace) printf("begindologand:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t3 = *(s32 *)(iSP + 4);   		// Get tag from ARG1 
  t4 = *(s32 *)iSP;   		// Grab data for ARG1 
  t1 = (u8)(arg1 >> ((4&7)*8));   		// Get tag from ARG2 
  t6 = t3 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical32440;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical32441;
  /* Here we know that both args are fixnums! */
  t4 = t4 & arg1;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
  *(u32 *)iSP = t4;   		// Push result 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

ilogical32440:
  if (_trace) printf("ilogical32440:\n");
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

ilogical32441:
  if (_trace) printf("ilogical32441:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

DoLogandIM:
  if (_trace) printf("DoLogandIM:\n");
  t3 = *(s32 *)(iSP + 4);   		// Get tag from ARG1 
  arg2 = arg2 << 56;   
  t4 = *(s32 *)iSP;   		// Grab data for ARG1 
  arg2 = (s64)arg2 >> 56;   
  t6 = t3 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical_immediate32442;
  /* Here we know that both args are fixnums! */
  t4 = t4 & arg2;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
  *(u32 *)iSP = t4;   		// Push result 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

ilogical_immediate32442:
  if (_trace) printf("ilogical_immediate32442:\n");
  arg1 = Type_Fixnum;
  arg2 = (u32)arg2;   
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

/* end DoLogand */
  /* End of Halfword operand from stack instruction - DoLogand */
/* start DoLogior */

  /* Halfword operand from stack instruction - DoLogior */
  /* arg2 has the preloaded 8 bit operand. */

dologior:
  if (_trace) printf("dologior:\n");

DoLogiorSP:
  if (_trace) printf("DoLogiorSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoLogiorLP:
  if (_trace) printf("DoLogiorLP:\n");

DoLogiorFP:
  if (_trace) printf("DoLogiorFP:\n");

headdologior:
  if (_trace) printf("headdologior:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindologior:
  if (_trace) printf("begindologior:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t3 = *(s32 *)(iSP + 4);   		// Get tag from ARG1 
  t4 = *(s32 *)iSP;   		// Grab data for ARG1 
  t1 = (u8)(arg1 >> ((4&7)*8));   		// Get tag from ARG2 
  t6 = t3 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical32443;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical32444;
  /* Here we know that both args are fixnums! */
  t4 = t4 | arg1;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
  *(u32 *)iSP = t4;   		// Push result 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

ilogical32443:
  if (_trace) printf("ilogical32443:\n");
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

ilogical32444:
  if (_trace) printf("ilogical32444:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

DoLogiorIM:
  if (_trace) printf("DoLogiorIM:\n");
  t3 = *(s32 *)(iSP + 4);   		// Get tag from ARG1 
  arg2 = arg2 << 56;   
  t4 = *(s32 *)iSP;   		// Grab data for ARG1 
  arg2 = (s64)arg2 >> 56;   
  t6 = t3 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical_immediate32445;
  /* Here we know that both args are fixnums! */
  t4 = t4 | arg2;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
  *(u32 *)iSP = t4;   		// Push result 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

ilogical_immediate32445:
  if (_trace) printf("ilogical_immediate32445:\n");
  arg1 = Type_Fixnum;
  arg2 = (u32)arg2;   
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

/* end DoLogior */
  /* End of Halfword operand from stack instruction - DoLogior */
/* start DoLogxor */

  /* Halfword operand from stack instruction - DoLogxor */
  /* arg2 has the preloaded 8 bit operand. */

dologxor:
  if (_trace) printf("dologxor:\n");

DoLogxorSP:
  if (_trace) printf("DoLogxorSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoLogxorLP:
  if (_trace) printf("DoLogxorLP:\n");

DoLogxorFP:
  if (_trace) printf("DoLogxorFP:\n");

headdologxor:
  if (_trace) printf("headdologxor:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindologxor:
  if (_trace) printf("begindologxor:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  t3 = *(s32 *)(iSP + 4);   		// Get tag from ARG1 
  t4 = *(s32 *)iSP;   		// Grab data for ARG1 
  t1 = (u8)(arg1 >> ((4&7)*8));   		// Get tag from ARG2 
  t6 = t3 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical32446;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical32447;
  /* Here we know that both args are fixnums! */
  t4 = t4 ^ arg1;   		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
  *(u32 *)iSP = t4;   		// Push result 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

ilogical32446:
  if (_trace) printf("ilogical32446:\n");
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

ilogical32447:
  if (_trace) printf("ilogical32447:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

DoLogxorIM:
  if (_trace) printf("DoLogxorIM:\n");
  t3 = *(s32 *)(iSP + 4);   		// Get tag from ARG1 
  arg2 = arg2 << 56;   
  t4 = *(s32 *)iSP;   		// Grab data for ARG1 
  arg2 = (s64)arg2 >> 56;   
  t6 = t3 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical_immediate32448;
  /* Here we know that both args are fixnums! */
  t4 = t4 ^ arg2;   		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
  *(u32 *)iSP = t4;   		// Push result 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

ilogical_immediate32448:
  if (_trace) printf("ilogical_immediate32448:\n");
  arg1 = Type_Fixnum;
  arg2 = (u32)arg2;   
  /* SetTag. */
  t1 = arg1 << 32;   
  t1 = arg2 | t1;
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

/* end DoLogxor */
  /* End of Halfword operand from stack instruction - DoLogxor */
/* start DoAsh */

  /* Halfword operand from stack instruction - DoAsh */

doash:
  if (_trace) printf("doash:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoAshIM:
  if (_trace) printf("DoAshIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32462:
  if (_trace) printf("force_alignment32462:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoash;   

DoAshSP:
  if (_trace) printf("DoAshSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAshLP:
  if (_trace) printf("DoAshLP:\n");

DoAshFP:
  if (_trace) printf("DoAshFP:\n");

headdoash:
  if (_trace) printf("headdoash:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoash:
  if (_trace) printf("begindoash:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg4 = *(s32 *)iSP;   		// Get ARG1. 
  arg3 = *(s32 *)(iSP + 4);   
  arg4 = (u32)arg4;   
  arg2 = arg1 >> 32;   		// Get ARG2's tag. 
  arg1 = (s32)arg1;		// Sign extended the rotation amount. 
  t1 = arg2 & 63;		// Strip off any CDR code bits. 
  t3 = arg3 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment32461:
  if (_trace) printf("force_alignment32461:\n");
  if (t2 == 0) 
    goto basic_dispatch32454;
  /* Here if argument TypeFixnum */
  t4 = (t3 == Type_Fixnum) ? 1 : 0;   

force_alignment32458:
  if (_trace) printf("force_alignment32458:\n");
  if (t4 == 0) 
    goto binary_type_dispatch32451;
  /* Here if argument TypeFixnum */
  if (arg4 == 0) 		// B. if ash of zero -- trivial case 
    goto zerash;
  if ((s64)arg1 <= 0)  		// B. if negative ash. 
    goto negash;
  arg4 = (s32)arg4;		// Sign extend ARG1 before shifting. 
  arg5 = arg1 - 32;   
  if ((s64)arg5 > 0)   
    goto ashovexc;
  arg5 = arg4 << (arg1 & 63);   		// Shift Left 
  arg6 = arg4 ^ arg5;   
  arg6 = arg6 >> 31;   		// arg6<0>=1 if overflow, 0 otherwise 
  /* TagType. */
  arg2 = arg2 & 63;
  if (arg6 != 0)   		// J. if overflow 
    goto ashovexc;
  *(u32 *)iSP = arg5;   
  *(u32 *)(iSP + 4) = arg2;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

negash:
  if (_trace) printf("negash:\n");
  arg1 = zero - arg1;   
  arg4 = (s32)arg4;		// Sign extend ARG1 before shifting. 
  arg5 = (s64)arg4 >> $27(arg1 & 63);   		// Shift Right 
  /* TagType. */
  arg2 = arg2 & 63;
  *(u32 *)iSP = arg5;   
  *(u32 *)(iSP + 4) = arg2;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

zerash:
  if (_trace) printf("zerash:\n");
  arg5 = Type_Fixnum;
  *(u32 *)iSP = arg4;   
  *(u32 *)(iSP + 4) = arg5;   		// write the stack cache 
  goto NEXTINSTRUCTION;   

basic_dispatch32455:
  if (_trace) printf("basic_dispatch32455:\n");

basic_dispatch32454:
  if (_trace) printf("basic_dispatch32454:\n");
  /* Here for all other cases */

binary_type_dispatch32450:
  if (_trace) printf("binary_type_dispatch32450:\n");
  arg1 = (u32)arg1;   
  /* SetTag. */
  t2 = arg2 << 32;   
  t2 = arg1 | t2;
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch32452;   

binary_type_dispatch32451:
  if (_trace) printf("binary_type_dispatch32451:\n");
  arg1 = (u32)arg1;   
  /* SetTag. */
  t2 = arg2 << 32;   
  t2 = arg1 | t2;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch32452:
  if (_trace) printf("binary_type_dispatch32452:\n");

basic_dispatch32453:
  if (_trace) printf("basic_dispatch32453:\n");

ashovexc:
  if (_trace) printf("ashovexc:\n");
  arg1 = (u32)arg1;   
  /* SetTag. */
  t1 = arg2 << 32;   
  t1 = arg1 | t1;
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto exception;

/* end DoAsh */
  /* End of Halfword operand from stack instruction - DoAsh */
/* start DoRot */

  /* Halfword operand from stack instruction - DoRot */
  /* arg2 has the preloaded 8 bit operand. */

dorot:
  if (_trace) printf("dorot:\n");

DoRotSP:
  if (_trace) printf("DoRotSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindorot;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoRotLP:
  if (_trace) printf("DoRotLP:\n");

DoRotFP:
  if (_trace) printf("DoRotFP:\n");

begindorot:
  if (_trace) printf("begindorot:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 

with_simple_binary_fixnum_operation32464:
  if (_trace) printf("with_simple_binary_fixnum_operation32464:\n");
  t4 = (u32)(arg6 >> ((4&7)*8));   		// Arg1 on the stack 
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = (u32)arg6;   		// Arg1 on the stack 
  t5 = *(s32 *)(arg1 + 4);   		// Arg2 from operand 
  t4 = t4 & 63;		// Strip CDR code if any. 
  t2 = *(s32 *)arg1;   		// Arg2 from operand 
  t4 = t4 - Type_Fixnum;   
  t8 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t5 = t5 & 63;		// Strip CDR code if any. 
  if (t4 != 0)   
    goto with_simple_binary_fixnum_operation32463;
  t2 = (u32)t2;   
  t5 = t5 - Type_Fixnum;   

force_alignment32465:
  if (_trace) printf("force_alignment32465:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation32463;
  t2 = t2 & 31;		// Get low 5 bits of the rotation 
  t3 = t1 << (t2 & 63);   		// Shift left to get new high bits 
  t6 = (u32)(t3 >> ((4&7)*8));   		// Get new low bits 
  t3 = t3 | t6;		// Glue two parts of shifted operand together 

force_alignment32466:
  if (_trace) printf("force_alignment32466:\n");
  iPC = t7;
  *(u32 *)iSP = t3;   		// Put the result back on the stack 
  iCP = t8;
  goto cachevalid;   

DoRotIM:
  if (_trace) printf("DoRotIM:\n");
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation32464;   

with_simple_binary_fixnum_operation32463:
  if (_trace) printf("with_simple_binary_fixnum_operation32463:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

/* end DoRot */
  /* End of Halfword operand from stack instruction - DoRot */
/* start DoLsh */

  /* Halfword operand from stack instruction - DoLsh */
  /* arg2 has the preloaded 8 bit operand. */

dolsh:
  if (_trace) printf("dolsh:\n");

DoLshSP:
  if (_trace) printf("DoLshSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindolsh;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

DoLshLP:
  if (_trace) printf("DoLshLP:\n");

DoLshFP:
  if (_trace) printf("DoLshFP:\n");

begindolsh:
  if (_trace) printf("begindolsh:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 

with_simple_binary_fixnum_operation32468:
  if (_trace) printf("with_simple_binary_fixnum_operation32468:\n");
  t4 = (u32)(arg6 >> ((4&7)*8));   		// Arg1 on the stack 
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = (u32)arg6;   		// Arg1 on the stack 
  t5 = *(s32 *)(arg1 + 4);   		// Arg2 from operand 
  t4 = t4 & 63;		// Strip CDR code if any. 
  t2 = *(s32 *)arg1;   		// Arg2 from operand 
  t4 = t4 - Type_Fixnum;   
  t8 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t5 = t5 & 63;		// Strip CDR code if any. 
  if (t4 != 0)   
    goto with_simple_binary_fixnum_operation32467;
  t5 = t5 - Type_Fixnum;   

force_alignment32469:
  if (_trace) printf("force_alignment32469:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation32467;
  if ((s64)t2 < 0)   		// B. if negative lsh. 
    goto neglsh;
  t3 = t2 - 32;   
  if ((s64)t3 >= 0)   
    goto returnzero;
  t3 = t1 << (t2 & 63);   		// Shift Left 
  goto lshdone;   

neglsh:
  if (_trace) printf("neglsh:\n");
  t2 = zero - t2;   
  t3 = t2 - 32;   
  if ((s64)t3 >= 0)   
    goto returnzero;
  t3 = t1 >> (t2 & 63);   		// Shift Right 
  goto lshdone;   

returnzero:
  if (_trace) printf("returnzero:\n");
  t3 = t3 & ~t3;

lshdone:
  if (_trace) printf("lshdone:\n");

force_alignment32470:
  if (_trace) printf("force_alignment32470:\n");
  iPC = t7;
  *(u32 *)iSP = t3;   		// Put the result back on the stack 
  iCP = t8;
  goto cachevalid;   

DoLshIM:
  if (_trace) printf("DoLshIM:\n");
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32471:
  if (_trace) printf("force_alignment32471:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation32468;   

with_simple_binary_fixnum_operation32467:
  if (_trace) printf("with_simple_binary_fixnum_operation32467:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

/* end DoLsh */
  /* End of Halfword operand from stack instruction - DoLsh */
/* start Do32BitPlus */

  /* Halfword operand from stack instruction - Do32BitPlus */
  /* arg2 has the preloaded 8 bit operand. */

do32bitplus:
  if (_trace) printf("do32bitplus:\n");

Do32BitPlusSP:
  if (_trace) printf("Do32BitPlusSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindo32bitplus;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

Do32BitPlusLP:
  if (_trace) printf("Do32BitPlusLP:\n");

Do32BitPlusFP:
  if (_trace) printf("Do32BitPlusFP:\n");

begindo32bitplus:
  if (_trace) printf("begindo32bitplus:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 

with_simple_binary_fixnum_operation32473:
  if (_trace) printf("with_simple_binary_fixnum_operation32473:\n");
  t4 = (u32)(arg6 >> ((4&7)*8));   		// Arg1 on the stack 
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = (u32)arg6;   		// Arg1 on the stack 
  t5 = *(s32 *)(arg1 + 4);   		// Arg2 from operand 
  t4 = t4 & 63;		// Strip CDR code if any. 
  t2 = *(s32 *)arg1;   		// Arg2 from operand 
  t4 = t4 - Type_Fixnum;   
  t8 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t5 = t5 & 63;		// Strip CDR code if any. 
  if (t4 != 0)   
    goto with_simple_binary_fixnum_operation32472;
  t2 = (u32)t2;   
  t5 = t5 - Type_Fixnum;   

force_alignment32474:
  if (_trace) printf("force_alignment32474:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation32472;
  t3 = t1 + t2;		// Perform the 32 bit Add. 

force_alignment32475:
  if (_trace) printf("force_alignment32475:\n");
  iPC = t7;
  *(u32 *)iSP = t3;   		// Put the result back on the stack 
  iCP = t8;
  goto cachevalid;   

Do32BitPlusIM:
  if (_trace) printf("Do32BitPlusIM:\n");
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation32473;   

with_simple_binary_fixnum_operation32472:
  if (_trace) printf("with_simple_binary_fixnum_operation32472:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

/* end Do32BitPlus */
  /* End of Halfword operand from stack instruction - Do32BitPlus */
/* start Do32BitDifference */

  /* Halfword operand from stack instruction - Do32BitDifference */
  /* arg2 has the preloaded 8 bit operand. */

do32bitdifference:
  if (_trace) printf("do32bitdifference:\n");

Do32BitDifferenceSP:
  if (_trace) printf("Do32BitDifferenceSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 != 0)   
    goto begindo32bitdifference;
  arg6 = *(u64 *)arg4;   		// SP-pop, Reload TOS 
  arg1 = iSP;		// SP-pop mode 
  iSP = arg4;		// Adjust SP 

Do32BitDifferenceLP:
  if (_trace) printf("Do32BitDifferenceLP:\n");

Do32BitDifferenceFP:
  if (_trace) printf("Do32BitDifferenceFP:\n");

begindo32bitdifference:
  if (_trace) printf("begindo32bitdifference:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 

with_simple_binary_fixnum_operation32477:
  if (_trace) printf("with_simple_binary_fixnum_operation32477:\n");
  t4 = (u32)(arg6 >> ((4&7)*8));   		// Arg1 on the stack 
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t1 = (u32)arg6;   		// Arg1 on the stack 
  t5 = *(s32 *)(arg1 + 4);   		// Arg2 from operand 
  t4 = t4 & 63;		// Strip CDR code if any. 
  t2 = *(s32 *)arg1;   		// Arg2 from operand 
  t4 = t4 - Type_Fixnum;   
  t8 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t5 = t5 & 63;		// Strip CDR code if any. 
  if (t4 != 0)   
    goto with_simple_binary_fixnum_operation32476;
  t2 = (u32)t2;   
  t5 = t5 - Type_Fixnum;   

force_alignment32478:
  if (_trace) printf("force_alignment32478:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation32476;
  t3 = t1 - t2;   		// Perform the 32 bit Difference. 

force_alignment32479:
  if (_trace) printf("force_alignment32479:\n");
  iPC = t7;
  *(u32 *)iSP = t3;   		// Put the result back on the stack 
  iCP = t8;
  goto cachevalid;   

Do32BitDifferenceIM:
  if (_trace) printf("Do32BitDifferenceIM:\n");
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation32477;   

with_simple_binary_fixnum_operation32476:
  if (_trace) printf("with_simple_binary_fixnum_operation32476:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

/* end Do32BitDifference */
  /* End of Halfword operand from stack instruction - Do32BitDifference */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunbits.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunblok.as
 ************************************************************************/

  /* Block Instructions. */
/* start DoBlock0Read */

  /* Halfword 10 bit immediate instruction - DoBlock0Read */

doblock0read:
  if (_trace) printf("doblock0read:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock0ReadIM:
  if (_trace) printf("DoBlock0ReadIM:\n");

DoBlock0ReadSP:
  if (_trace) printf("DoBlock0ReadSP:\n");

DoBlock0ReadLP:
  if (_trace) printf("DoBlock0ReadLP:\n");

DoBlock0ReadFP:
  if (_trace) printf("DoBlock0ReadFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar0;   
  goto blockread;   

/* end DoBlock0Read */
  /* End of Halfword operand from stack instruction - DoBlock0Read */
/* start DoBlock0Write */

  /* Halfword operand from stack instruction - DoBlock0Write */

doblock0write:
  if (_trace) printf("doblock0write:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoBlock0WriteIM:
  if (_trace) printf("DoBlock0WriteIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32480:
  if (_trace) printf("force_alignment32480:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoblock0write;   

DoBlock0WriteSP:
  if (_trace) printf("DoBlock0WriteSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock0WriteLP:
  if (_trace) printf("DoBlock0WriteLP:\n");

DoBlock0WriteFP:
  if (_trace) printf("DoBlock0WriteFP:\n");

headdoblock0write:
  if (_trace) printf("headdoblock0write:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoblock0write:
  if (_trace) printf("begindoblock0write:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg3 = *(s32 *)&processor->bar0;   
  arg2 = (u64)&processor->bar0;   
  goto blockwrite;   

/* end DoBlock0Write */
  /* End of Halfword operand from stack instruction - DoBlock0Write */
/* start DoBlock0ReadShift */

  /* Halfword 10 bit immediate instruction - DoBlock0ReadShift */

doblock0readshift:
  if (_trace) printf("doblock0readshift:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock0ReadShiftIM:
  if (_trace) printf("DoBlock0ReadShiftIM:\n");

DoBlock0ReadShiftSP:
  if (_trace) printf("DoBlock0ReadShiftSP:\n");

DoBlock0ReadShiftLP:
  if (_trace) printf("DoBlock0ReadShiftLP:\n");

DoBlock0ReadShiftFP:
  if (_trace) printf("DoBlock0ReadShiftFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar0;   
  goto blockreadshift;   

/* end DoBlock0ReadShift */
  /* End of Halfword operand from stack instruction - DoBlock0ReadShift */
/* start DoBlock3ReadShift */

  /* Halfword 10 bit immediate instruction - DoBlock3ReadShift */

doblock3readshift:
  if (_trace) printf("doblock3readshift:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock3ReadShiftIM:
  if (_trace) printf("DoBlock3ReadShiftIM:\n");

DoBlock3ReadShiftSP:
  if (_trace) printf("DoBlock3ReadShiftSP:\n");

DoBlock3ReadShiftLP:
  if (_trace) printf("DoBlock3ReadShiftLP:\n");

DoBlock3ReadShiftFP:
  if (_trace) printf("DoBlock3ReadShiftFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar3;   
  goto blockreadshift;   

/* end DoBlock3ReadShift */
  /* End of Halfword operand from stack instruction - DoBlock3ReadShift */
/* start DoBlock2ReadShift */

  /* Halfword 10 bit immediate instruction - DoBlock2ReadShift */

doblock2readshift:
  if (_trace) printf("doblock2readshift:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock2ReadShiftIM:
  if (_trace) printf("DoBlock2ReadShiftIM:\n");

DoBlock2ReadShiftSP:
  if (_trace) printf("DoBlock2ReadShiftSP:\n");

DoBlock2ReadShiftLP:
  if (_trace) printf("DoBlock2ReadShiftLP:\n");

DoBlock2ReadShiftFP:
  if (_trace) printf("DoBlock2ReadShiftFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar2;   
  goto blockreadshift;   

/* end DoBlock2ReadShift */
  /* End of Halfword operand from stack instruction - DoBlock2ReadShift */
/* start DoBlock1ReadShift */

  /* Halfword 10 bit immediate instruction - DoBlock1ReadShift */

doblock1readshift:
  if (_trace) printf("doblock1readshift:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock1ReadShiftIM:
  if (_trace) printf("DoBlock1ReadShiftIM:\n");

DoBlock1ReadShiftSP:
  if (_trace) printf("DoBlock1ReadShiftSP:\n");

DoBlock1ReadShiftLP:
  if (_trace) printf("DoBlock1ReadShiftLP:\n");

DoBlock1ReadShiftFP:
  if (_trace) printf("DoBlock1ReadShiftFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar1;   

blockreadshift:
  if (_trace) printf("blockreadshift:\n");
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t2 = *(s32 *)arg2;   		// Get the vma 
  t1 = arg1 >> 6;   		// cycle type 
  t4 = arg1 & 4;		// =no-incrementp 
  t5 = arg1 & 16;		// =cdr-code-nextp 
  t6 = arg1 & 32;		// =fixnum onlyp 
  t2 = (u32)t2;   
  /* Memory Read Internal */

vma_memory_read32485:
  t11 = t2 + ivory;
  t12 = (t1 * 4);   		// Cycle-number -> table offset 
  t8 = LDQ_U(t11);   
  t12 = (t12 * 4) + ivory;   
  t7 = (t11 * 4);   
  t9 = t2 - arg5;   		// Stack cache offset 
  t12 = *(u64 *)(t12 + PROCESSORSTATE_DATAREAD_MASK);   
  t10 = ((u64)t9 < (u64)arg6) ? 1 : 0;   		// In range? 
  t7 = *(s32 *)t7;   
  t8 = (u8)(t8 >> ((t11&7)*8));   
  if (t10 != 0)   
    goto vma_memory_read32487;

vma_memory_read32486:
  t12 = t12 >> (t8 & 63);   
  t7 = (u32)t7;   
  if (t12 & 1)   
    goto vma_memory_read32489;

vma_memory_read32496:
  if (t6 == 0) 		// J. if we don't have to test for fixnump. 
    goto i_block_n_read_shift32481;
  t9 = t8 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_shift32484;

i_block_n_read_shift32481:
  if (_trace) printf("i_block_n_read_shift32481:\n");
  if (t4 != 0)   		// J. if we don't have to increment the address. 
    goto i_block_n_read_shift32482;
  t2 = t2 + 1;		// Increment the address 

i_block_n_read_shift32482:
  if (_trace) printf("i_block_n_read_shift32482:\n");
  *(u32 *)arg2 = t2;   		// Store updated vma in BAR 
  if (t5 == 0) 		// J. if we don't have to clear CDR codes. 
    goto i_block_n_read_shift32483;
  t8 = t8 & 63;

i_block_n_read_shift32483:
  if (_trace) printf("i_block_n_read_shift32483:\n");
  t1 = zero + 21504;   
  t3 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t4 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t2 = t1 >> 10;   
  t2 = t2 & 3;		// Extract the byte background 
  t5 = (t2 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment32503:
  if (_trace) printf("force_alignment32503:\n");
  if (t5 == 0) 
    goto basic_dispatch32499;
  /* Here if argument ALUByteBackgroundOp1 */
  t2 = t1;

basic_dispatch32498:
  if (_trace) printf("basic_dispatch32498:\n");
  t6 = t1 >> 12;   
  t6 = t6 & 1;		// Extractthe byte rotate latch 
  t7 = t7 << (t3 & 63);   
  t5 = (u32)(t7 >> ((4&7)*8));   
  t7 = (u32)t7;   
  t7 = t7 | t5;		// OP2 rotated 
  if (t6 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte32497;
  *(u64 *)&processor->rotatelatch = t7;   

alu_function_byte32497:
  if (_trace) printf("alu_function_byte32497:\n");
  t6 = zero + -2;   
  t6 = t6 << (t4 & 63);   
  t6 = ~t6;   		// Compute mask 
  /* Get byte function */
  t5 = t1 >> 13;   
  t5 = t5 & 1;
  t4 = (t5 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment32508:
  if (_trace) printf("force_alignment32508:\n");
  if (t4 == 0) 
    goto basic_dispatch32505;
  /* Here if argument ALUByteFunctionDpb */
  t6 = t6 << (t3 & 63);   		// Position mask 

basic_dispatch32504:
  if (_trace) printf("basic_dispatch32504:\n");
  t7 = t7 & t6;		// rotated&mask 
  t2 = t2 & ~t6;		// background&~mask 
  t7 = t7 | t2;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + 8) = t7;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

i_block_n_read_shift32484:
  if (_trace) printf("i_block_n_read_shift32484:\n");
  arg5 = t2;
  arg2 = 23;
  goto illegaloperand;

basic_dispatch32505:
  if (_trace) printf("basic_dispatch32505:\n");
  t4 = (t5 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment32509:
  if (_trace) printf("force_alignment32509:\n");
  if (t4 != 0)   
    goto basic_dispatch32504;
  goto basic_dispatch32504;   

basic_dispatch32499:
  if (_trace) printf("basic_dispatch32499:\n");
  t5 = (t2 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment32510:
  if (_trace) printf("force_alignment32510:\n");
  if (t5 == 0) 
    goto basic_dispatch32500;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t2 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch32498;   

basic_dispatch32500:
  if (_trace) printf("basic_dispatch32500:\n");
  t5 = (t2 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment32511:
  if (_trace) printf("force_alignment32511:\n");
  if (t5 == 0) 
    goto basic_dispatch32498;
  /* Here if argument ALUByteBackgroundZero */
  t2 = zero;
  goto basic_dispatch32498;   

vma_memory_read32487:
  if (_trace) printf("vma_memory_read32487:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t10;  		// reconstruct SCA 
  t7 = *(s32 *)t9;   
  t8 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read32486;   

vma_memory_read32489:
  if (_trace) printf("vma_memory_read32489:\n");

vma_memory_read32488:
  if (_trace) printf("vma_memory_read32488:\n");
  t12 = (t1 * 4);   		// Cycle-number -> table offset 
  t12 = (t12 * 4) + ivory;   
  t12 = *(u64 *)(t12 + PROCESSORSTATE_DATAREAD);   
  /* TagType. */
  t11 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + t12;   		// Adjust for a longword load 
  t12 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read32494:
  if (_trace) printf("vma_memory_read32494:\n");
  t10 = t12 & MemoryActionIndirect;
  if (t10 == 0) 
    goto vma_memory_read32493;
  t2 = (u32)t7;   		// Do the indirect thing 
  goto vma_memory_read32485;   

vma_memory_read32493:
  if (_trace) printf("vma_memory_read32493:\n");
  t11 = t12 & MemoryActionTransform;
  if (t11 == 0) 
    goto vma_memory_read32492;
  t8 = t8 & ~63L;
  t8 = t8 | Type_ExternalValueCellPointer;
  goto vma_memory_read32496;   

vma_memory_read32492:

vma_memory_read32491:
  /* Perform memory action */
  arg1 = t12;
  arg2 = t1;
  goto performmemoryaction;

/* end DoBlock1ReadShift */
  /* End of Halfword operand from stack instruction - DoBlock1ReadShift */
/* start DoBlock0ReadAlu */

  /* Halfword operand from stack instruction - DoBlock0ReadAlu */
  /* arg2 has the preloaded 8 bit operand. */

doblock0readalu:
  if (_trace) printf("doblock0readalu:\n");

DoBlock0ReadAluSP:
  if (_trace) printf("DoBlock0ReadAluSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock0ReadAluLP:
  if (_trace) printf("DoBlock0ReadAluLP:\n");

DoBlock0ReadAluFP:
  if (_trace) printf("DoBlock0ReadAluFP:\n");

begindoblock0readalu:
  if (_trace) printf("begindoblock0readalu:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg2 = (u64)&processor->bar0;   
  goto blockreadalu;   

DoBlock0ReadAluIM:
  goto doistageerror;

/* end DoBlock0ReadAlu */
  /* End of Halfword operand from stack instruction - DoBlock0ReadAlu */
/* start DoBlock3ReadAlu */

  /* Halfword operand from stack instruction - DoBlock3ReadAlu */
  /* arg2 has the preloaded 8 bit operand. */

doblock3readalu:
  if (_trace) printf("doblock3readalu:\n");

DoBlock3ReadAluSP:
  if (_trace) printf("DoBlock3ReadAluSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock3ReadAluLP:
  if (_trace) printf("DoBlock3ReadAluLP:\n");

DoBlock3ReadAluFP:
  if (_trace) printf("DoBlock3ReadAluFP:\n");

begindoblock3readalu:
  if (_trace) printf("begindoblock3readalu:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg2 = (u64)&processor->bar3;   
  goto blockreadalu;   

DoBlock3ReadAluIM:
  goto doistageerror;

/* end DoBlock3ReadAlu */
  /* End of Halfword operand from stack instruction - DoBlock3ReadAlu */
/* start DoBlock2ReadAlu */

  /* Halfword operand from stack instruction - DoBlock2ReadAlu */
  /* arg2 has the preloaded 8 bit operand. */

doblock2readalu:
  if (_trace) printf("doblock2readalu:\n");

DoBlock2ReadAluSP:
  if (_trace) printf("DoBlock2ReadAluSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock2ReadAluLP:
  if (_trace) printf("DoBlock2ReadAluLP:\n");

DoBlock2ReadAluFP:
  if (_trace) printf("DoBlock2ReadAluFP:\n");

begindoblock2readalu:
  if (_trace) printf("begindoblock2readalu:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg2 = (u64)&processor->bar2;   
  goto blockreadalu;   

DoBlock2ReadAluIM:
  goto doistageerror;

/* end DoBlock2ReadAlu */
  /* End of Halfword operand from stack instruction - DoBlock2ReadAlu */
/* start DoBlock1ReadAlu */

  /* Halfword operand from stack instruction - DoBlock1ReadAlu */
  /* arg2 has the preloaded 8 bit operand. */

doblock1readalu:
  if (_trace) printf("doblock1readalu:\n");

DoBlock1ReadAluSP:
  if (_trace) printf("DoBlock1ReadAluSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBlock1ReadAluLP:
  if (_trace) printf("DoBlock1ReadAluLP:\n");

DoBlock1ReadAluFP:
  if (_trace) printf("DoBlock1ReadAluFP:\n");

begindoblock1readalu:
  if (_trace) printf("begindoblock1readalu:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg2 = (u64)&processor->bar1;   

blockreadalu:
  if (_trace) printf("blockreadalu:\n");
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t1 = *(s32 *)arg2;   		// Get the vma 
  t5 = *(s32 *)arg1;   
  t4 = *(s32 *)(arg1 + 4);   
  t5 = (u32)t5;   
  t9 = t4 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_alu32512;
  t1 = (u32)t1;   
  /* Memory Read Internal */

vma_memory_read32514:
  t11 = t1 + ivory;
  t3 = (t11 * 4);   
  t2 = LDQ_U(t11);   
  t9 = t1 - arg5;   		// Stack cache offset 
  t12 = *(u64 *)&(processor->dataread_mask);   
  t10 = ((u64)t9 < (u64)arg6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t11&7)*8));   
  if (t10 != 0)   
    goto vma_memory_read32516;

vma_memory_read32515:
  t11 = zero + 240;   
  t12 = t12 >> (t2 & 63);   
  t11 = t11 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t12 & 1)   
    goto vma_memory_read32518;

vma_memory_read32525:
  t9 = t2 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_alu32513;
  t1 = t1 + 1;		// Increment the address 
  *(u32 *)arg2 = t1;   		// Store updated vma in BAR 
  t6 = *(u64 *)&(processor->aluop);   
  *(u64 *)&processor->aluoverflow = zero;   
  t7 = *(u64 *)&(processor->aluandrotatecontrol);   
  t1 = (t6 == ALUFunction_Boolean) ? 1 : 0;   

force_alignment32586:
  if (_trace) printf("force_alignment32586:\n");
  if (t1 == 0) 
    goto basic_dispatch32527;
  /* Here if argument ALUFunctionBoolean */
  t8 = t7 >> 10;   
  t8 = t8 & 15;		// Extract the ALU boolean function 
  t1 = (t8 == Boole_Clear) ? 1 : 0;   

force_alignment32546:
  if (_trace) printf("force_alignment32546:\n");
  if (t1 != 0)   
    goto basic_dispatch32528;

basic_dispatch32529:
  if (_trace) printf("basic_dispatch32529:\n");
  t1 = (t8 == Boole_And) ? 1 : 0;   

force_alignment32547:
  if (_trace) printf("force_alignment32547:\n");
  if (t1 == 0) 
    goto basic_dispatch32530;
  /* Here if argument BooleAnd */
  t8 = t3 & t5;
  goto basic_dispatch32528;   

basic_dispatch32530:
  if (_trace) printf("basic_dispatch32530:\n");
  t1 = (t8 == Boole_AndC1) ? 1 : 0;   

force_alignment32548:
  if (_trace) printf("force_alignment32548:\n");
  if (t1 == 0) 
    goto basic_dispatch32531;
  /* Here if argument BooleAndC1 */
  t8 = t5 & ~t3;
  goto basic_dispatch32528;   

basic_dispatch32531:
  if (_trace) printf("basic_dispatch32531:\n");
  t1 = (t8 == Boole_2) ? 1 : 0;   

force_alignment32549:
  if (_trace) printf("force_alignment32549:\n");
  if (t1 == 0) 
    goto basic_dispatch32532;
  /* Here if argument Boole2 */
  t8 = t5;
  goto basic_dispatch32528;   

basic_dispatch32532:
  if (_trace) printf("basic_dispatch32532:\n");
  t1 = (t8 == Boole_AndC2) ? 1 : 0;   

force_alignment32550:
  if (_trace) printf("force_alignment32550:\n");
  if (t1 == 0) 
    goto basic_dispatch32533;
  /* Here if argument BooleAndC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch32528;   

basic_dispatch32533:
  if (_trace) printf("basic_dispatch32533:\n");
  t1 = (t8 == Boole_1) ? 1 : 0;   

force_alignment32551:
  if (_trace) printf("force_alignment32551:\n");
  if (t1 == 0) 
    goto basic_dispatch32534;
  /* Here if argument Boole1 */
  t8 = t3;
  goto basic_dispatch32528;   

basic_dispatch32534:
  if (_trace) printf("basic_dispatch32534:\n");
  t1 = (t8 == Boole_Xor) ? 1 : 0;   

force_alignment32552:
  if (_trace) printf("force_alignment32552:\n");
  if (t1 == 0) 
    goto basic_dispatch32535;
  /* Here if argument BooleXor */
  t8 = t3 ^ t5;   
  goto basic_dispatch32528;   

basic_dispatch32535:
  if (_trace) printf("basic_dispatch32535:\n");
  t1 = (t8 == Boole_Ior) ? 1 : 0;   

force_alignment32553:
  if (_trace) printf("force_alignment32553:\n");
  if (t1 == 0) 
    goto basic_dispatch32536;
  /* Here if argument BooleIor */
  t8 = t3 | t5;
  goto basic_dispatch32528;   

basic_dispatch32536:
  if (_trace) printf("basic_dispatch32536:\n");
  t1 = (t8 == Boole_Nor) ? 1 : 0;   

force_alignment32554:
  if (_trace) printf("force_alignment32554:\n");
  if (t1 == 0) 
    goto basic_dispatch32537;
  /* Here if argument BooleNor */
  t8 = t3 | t5;
  t8 = ~t8;   
  goto basic_dispatch32528;   

basic_dispatch32537:
  if (_trace) printf("basic_dispatch32537:\n");
  t1 = (t8 == Boole_Equiv) ? 1 : 0;   

force_alignment32555:
  if (_trace) printf("force_alignment32555:\n");
  if (t1 == 0) 
    goto basic_dispatch32538;
  /* Here if argument BooleEquiv */
  t8 = t3 ^ t5;   
  t8 = ~t8;   
  goto basic_dispatch32528;   

basic_dispatch32538:
  if (_trace) printf("basic_dispatch32538:\n");
  t1 = (t8 == Boole_C1) ? 1 : 0;   

force_alignment32556:
  if (_trace) printf("force_alignment32556:\n");
  if (t1 == 0) 
    goto basic_dispatch32539;
  /* Here if argument BooleC1 */
  t8 = ~t3;   
  goto basic_dispatch32528;   

basic_dispatch32539:
  if (_trace) printf("basic_dispatch32539:\n");
  t1 = (t8 == Boole_OrC1) ? 1 : 0;   

force_alignment32557:
  if (_trace) printf("force_alignment32557:\n");
  if (t1 == 0) 
    goto basic_dispatch32540;
  /* Here if argument BooleOrC1 */
  t8 = t5 | ~(t3);   
  goto basic_dispatch32528;   

basic_dispatch32540:
  if (_trace) printf("basic_dispatch32540:\n");
  t1 = (t8 == Boole_C2) ? 1 : 0;   

force_alignment32558:
  if (_trace) printf("force_alignment32558:\n");
  if (t1 == 0) 
    goto basic_dispatch32541;
  /* Here if argument BooleC2 */
  t8 = ~t5;   
  goto basic_dispatch32528;   

basic_dispatch32541:
  if (_trace) printf("basic_dispatch32541:\n");
  t1 = (t8 == Boole_OrC2) ? 1 : 0;   

force_alignment32559:
  if (_trace) printf("force_alignment32559:\n");
  if (t1 == 0) 
    goto basic_dispatch32542;
  /* Here if argument BooleOrC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch32528;   

basic_dispatch32542:
  if (_trace) printf("basic_dispatch32542:\n");
  t1 = (t8 == Boole_Nand) ? 1 : 0;   

force_alignment32560:
  if (_trace) printf("force_alignment32560:\n");
  if (t1 == 0) 
    goto basic_dispatch32543;
  /* Here if argument BooleNand */
  t8 = t3 & t5;
  goto basic_dispatch32528;   

basic_dispatch32543:
  if (_trace) printf("basic_dispatch32543:\n");
  t1 = (t8 == Boole_Set) ? 1 : 0;   

force_alignment32561:
  if (_trace) printf("force_alignment32561:\n");
  if (t1 == 0) 
    goto basic_dispatch32528;
  /* Here if argument BooleSet */
  t8 = ~zero;   

basic_dispatch32528:
  if (_trace) printf("basic_dispatch32528:\n");
  *(u32 *)arg1 = t8;   
  goto NEXTINSTRUCTION;   

basic_dispatch32527:
  if (_trace) printf("basic_dispatch32527:\n");
  t1 = (t6 == ALUFunction_Byte) ? 1 : 0;   

force_alignment32587:
  if (_trace) printf("force_alignment32587:\n");
  if (t1 == 0) 
    goto basic_dispatch32562;
  /* Here if argument ALUFunctionByte */
  t9 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t10 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t1 = t7 >> 10;   
  t1 = t1 & 3;		// Extract the byte background 
  t11 = (t1 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment32569:
  if (_trace) printf("force_alignment32569:\n");
  if (t11 == 0) 
    goto basic_dispatch32565;
  /* Here if argument ALUByteBackgroundOp1 */
  t1 = t3;

basic_dispatch32564:
  if (_trace) printf("basic_dispatch32564:\n");
  t12 = t7 >> 12;   
  t12 = t12 & 1;		// Extractthe byte rotate latch 
  t8 = t5 << (t9 & 63);   
  t11 = (u32)(t8 >> ((4&7)*8));   
  t8 = (u32)t8;   
  t8 = t8 | t11;		// OP2 rotated 
  if (t12 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte32563;
  *(u64 *)&processor->rotatelatch = t8;   

alu_function_byte32563:
  if (_trace) printf("alu_function_byte32563:\n");
  t12 = zero + -2;   
  t12 = t12 << (t10 & 63);   
  t12 = ~t12;   		// Compute mask 
  /* Get byte function */
  t11 = t7 >> 13;   
  t11 = t11 & 1;
  t10 = (t11 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment32574:
  if (_trace) printf("force_alignment32574:\n");
  if (t10 == 0) 
    goto basic_dispatch32571;
  /* Here if argument ALUByteFunctionDpb */
  t12 = t12 << (t9 & 63);   		// Position mask 

basic_dispatch32570:
  if (_trace) printf("basic_dispatch32570:\n");
  t8 = t8 & t12;		// rotated&mask 
  t1 = t1 & ~t12;		// background&~mask 
  t8 = t8 | t1;
  *(u32 *)arg1 = t8;   
  goto NEXTINSTRUCTION;   

basic_dispatch32562:
  if (_trace) printf("basic_dispatch32562:\n");
  t1 = (t6 == ALUFunction_Adder) ? 1 : 0;   

force_alignment32588:
  if (_trace) printf("force_alignment32588:\n");
  if (t1 == 0) 
    goto basic_dispatch32575;
  /* Here if argument ALUFunctionAdder */
  t10 = t7 >> 11;   
  t10 = t10 & 3;		// Extract the op2 
  t9 = t7 >> 10;   
  t9 = t9 & 1;		// Extract the adder carry in 
  t11 = (t10 == ALUAdderOp2_Op2) ? 1 : 0;   

force_alignment32583:
  if (_trace) printf("force_alignment32583:\n");
  if (t11 == 0) 
    goto basic_dispatch32578;
  /* Here if argument ALUAdderOp2Op2 */
  t1 = t5;

basic_dispatch32577:
  if (_trace) printf("basic_dispatch32577:\n");
  t8 = t3 + t1;
  t8 = t8 + t9;
  t10 = t8 >> 31;   		// Sign bit 
  t11 = t8 >> 32;   		// Next bit 
  t10 = t10 ^ t11;   		// Low bit is now overflow indicator 
  t11 = t7 >> 24;   		// Get the load-carry-in bit 
  *(u64 *)&processor->aluoverflow = t10;   
  if ((t11 & 1) == 0)   
    goto alu_function_adder32576;
  t10 = (u32)(t8 >> ((4&7)*8));   		// Get the carry 
  t11 = zero + 1024;   
  t7 = t7 & ~t11;
  t11 = t10 & 1;
  t11 = t11 << 10;   
  t7 = t7 | t11;		// Set the adder carry in 
  *(u64 *)&processor->aluandrotatecontrol = t7;   

alu_function_adder32576:
  if (_trace) printf("alu_function_adder32576:\n");
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->aluborrow = t10;   
  t3 = (s32)t3;
  t5 = (s32)t5;
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->alulessthan = t10;   
  *(u32 *)arg1 = t8;   
  goto NEXTINSTRUCTION;   

basic_dispatch32575:
  if (_trace) printf("basic_dispatch32575:\n");
  t1 = (t6 == ALUFunction_MultiplyDivide) ? 1 : 0;   

force_alignment32589:
  if (_trace) printf("force_alignment32589:\n");
  if (t1 == 0) 
    goto basic_dispatch32526;
  /* Here if argument ALUFunctionMultiplyDivide */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;
  *(u32 *)arg1 = t8;   
  goto NEXTINSTRUCTION;   

basic_dispatch32526:
  if (_trace) printf("basic_dispatch32526:\n");

i_block_n_read_alu32512:
  if (_trace) printf("i_block_n_read_alu32512:\n");
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  t9 = arg1 - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  t1 = t9 + arg5;		// reconstruct VMA 
  arg5 = t1;
  arg2 = 23;
  goto illegaloperand;

i_block_n_read_alu32513:
  if (_trace) printf("i_block_n_read_alu32513:\n");
  arg5 = t1;
  arg2 = 23;
  goto illegaloperand;

basic_dispatch32578:
  if (_trace) printf("basic_dispatch32578:\n");
  t11 = (t10 == ALUAdderOp2_Zero) ? 1 : 0;   

force_alignment32590:
  if (_trace) printf("force_alignment32590:\n");
  if (t11 == 0) 
    goto basic_dispatch32579;
  /* Here if argument ALUAdderOp2Zero */
  t1 = zero;
  goto basic_dispatch32577;   

basic_dispatch32579:
  if (_trace) printf("basic_dispatch32579:\n");
  t11 = (t10 == ALUAdderOp2_Invert) ? 1 : 0;   

force_alignment32591:
  if (_trace) printf("force_alignment32591:\n");
  if (t11 == 0) 
    goto basic_dispatch32580;
  /* Here if argument ALUAdderOp2Invert */
  t1 = (s32)t5;
  t1 = zero - t1;   
  t1 = (u32)t1;   
  goto basic_dispatch32577;   

basic_dispatch32580:
  if (_trace) printf("basic_dispatch32580:\n");
  t11 = (t10 == ALUAdderOp2_MinusOne) ? 1 : 0;   

force_alignment32592:
  if (_trace) printf("force_alignment32592:\n");
  if (t11 == 0) 
    goto basic_dispatch32577;
  /* Here if argument ALUAdderOp2MinusOne */
  t1 = ~zero;   
  t1 = (u32)t1;   
  goto basic_dispatch32577;   

basic_dispatch32571:
  if (_trace) printf("basic_dispatch32571:\n");
  t10 = (t11 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment32593:
  if (_trace) printf("force_alignment32593:\n");
  if (t10 != 0)   
    goto basic_dispatch32570;
  goto basic_dispatch32570;   

basic_dispatch32565:
  if (_trace) printf("basic_dispatch32565:\n");
  t11 = (t1 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment32594:
  if (_trace) printf("force_alignment32594:\n");
  if (t11 == 0) 
    goto basic_dispatch32566;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t1 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch32564;   

basic_dispatch32566:
  if (_trace) printf("basic_dispatch32566:\n");
  t11 = (t1 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment32595:
  if (_trace) printf("force_alignment32595:\n");
  if (t11 == 0) 
    goto basic_dispatch32564;
  /* Here if argument ALUByteBackgroundZero */
  t1 = zero;
  goto basic_dispatch32564;   

vma_memory_read32516:
  if (_trace) printf("vma_memory_read32516:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t10;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read32515;   

vma_memory_read32518:
  if (_trace) printf("vma_memory_read32518:\n");
  if ((t11 & 1) == 0)   
    goto vma_memory_read32517;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32514;   

vma_memory_read32517:
  if (_trace) printf("vma_memory_read32517:\n");
  t12 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t11 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + t12;   		// Adjust for a longword load 
  t12 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read32522:
  if (_trace) printf("vma_memory_read32522:\n");
  t11 = t12 & MemoryActionTransform;
  if (t11 == 0) 
    goto vma_memory_read32521;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read32525;   

vma_memory_read32521:

vma_memory_read32520:
  /* Perform memory action */
  arg1 = t12;
  arg2 = 0;
  goto performmemoryaction;

DoBlock1ReadAluIM:
  goto doistageerror;

/* end DoBlock1ReadAlu */
  /* End of Halfword operand from stack instruction - DoBlock1ReadAlu */
/* start DoBlock0ReadTest */

  /* Halfword 10 bit immediate instruction - DoBlock0ReadTest */

doblock0readtest:
  if (_trace) printf("doblock0readtest:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock0ReadTestIM:
  if (_trace) printf("DoBlock0ReadTestIM:\n");

DoBlock0ReadTestSP:
  if (_trace) printf("DoBlock0ReadTestSP:\n");

DoBlock0ReadTestLP:
  if (_trace) printf("DoBlock0ReadTestLP:\n");

DoBlock0ReadTestFP:
  if (_trace) printf("DoBlock0ReadTestFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar0;   
  goto blockreadtest;   

/* end DoBlock0ReadTest */
  /* End of Halfword operand from stack instruction - DoBlock0ReadTest */
/* start DoBlock3ReadTest */

  /* Halfword 10 bit immediate instruction - DoBlock3ReadTest */

doblock3readtest:
  if (_trace) printf("doblock3readtest:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock3ReadTestIM:
  if (_trace) printf("DoBlock3ReadTestIM:\n");

DoBlock3ReadTestSP:
  if (_trace) printf("DoBlock3ReadTestSP:\n");

DoBlock3ReadTestLP:
  if (_trace) printf("DoBlock3ReadTestLP:\n");

DoBlock3ReadTestFP:
  if (_trace) printf("DoBlock3ReadTestFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar3;   
  goto blockreadtest;   

/* end DoBlock3ReadTest */
  /* End of Halfword operand from stack instruction - DoBlock3ReadTest */
/* start DoBlock2ReadTest */

  /* Halfword 10 bit immediate instruction - DoBlock2ReadTest */

doblock2readtest:
  if (_trace) printf("doblock2readtest:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock2ReadTestIM:
  if (_trace) printf("DoBlock2ReadTestIM:\n");

DoBlock2ReadTestSP:
  if (_trace) printf("DoBlock2ReadTestSP:\n");

DoBlock2ReadTestLP:
  if (_trace) printf("DoBlock2ReadTestLP:\n");

DoBlock2ReadTestFP:
  if (_trace) printf("DoBlock2ReadTestFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar2;   
  goto blockreadtest;   

/* end DoBlock2ReadTest */
  /* End of Halfword operand from stack instruction - DoBlock2ReadTest */
/* start DoBlock1ReadTest */

  /* Halfword 10 bit immediate instruction - DoBlock1ReadTest */

doblock1readtest:
  if (_trace) printf("doblock1readtest:\n");
  /* Actually only one entry point, but simulate others for dispatch */

DoBlock1ReadTestIM:
  if (_trace) printf("DoBlock1ReadTestIM:\n");

DoBlock1ReadTestSP:
  if (_trace) printf("DoBlock1ReadTestSP:\n");

DoBlock1ReadTestLP:
  if (_trace) printf("DoBlock1ReadTestLP:\n");

DoBlock1ReadTestFP:
  if (_trace) printf("DoBlock1ReadTestFP:\n");
  arg1 = (u16)(arg3 >> ((4&7)*8));   
  /* arg1 has operand preloaded. */
  arg2 = (u64)&processor->bar1;   

blockreadtest:
  if (_trace) printf("blockreadtest:\n");
  arg3 = *(s32 *)arg2;   		// Get the vma 
  t1 = arg1 >> 6;   		// cycle type 
  t5 = *(s32 *)iSP;   
  t4 = *(s32 *)(iSP + 4);   
  t5 = (u32)t5;   
  arg3 = (u32)arg3;   
  /* Memory Read Internal */

vma_memory_read32602:
  t9 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t11 = arg3 + ivory;
  t10 = *(s32 *)&processor->scovlimit;   
  t12 = (t1 * 4);   		// Cycle-number -> table offset 
  t2 = LDQ_U(t11);   
  t12 = (t12 * 4) + ivory;   
  t3 = (t11 * 4);   
  t9 = arg3 - t9;   		// Stack cache offset 
  t12 = *(u64 *)(t12 + PROCESSORSTATE_DATAREAD_MASK);   
  t10 = ((u64)t9 < (u64)t10) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t11&7)*8));   
  if (t10 != 0)   
    goto vma_memory_read32604;

vma_memory_read32603:
  t12 = t12 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t12 & 1)   
    goto vma_memory_read32606;

vma_memory_read32613:
  t1 = arg1 & 32;		// =fixnum onlyp 
  if (t1 == 0) 		// J. if we don't have to test for fixnump. 
    goto i_block_n_read_test32596;
  t9 = t2 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_test32599;
  t9 = t4 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_test32600;

i_block_n_read_test32596:
  if (_trace) printf("i_block_n_read_test32596:\n");
  t1 = arg1 & 16;		// =cdr-code-nextp 
  if (t1 == 0) 		// J. if we don't have to clear CDR codes. 
    goto i_block_n_read_test32598;
  /* TagType. */
  t2 = t2 & 63;

i_block_n_read_test32598:
  if (_trace) printf("i_block_n_read_test32598:\n");
  t6 = *(u64 *)&(processor->aluop);   
  *(u64 *)&processor->aluoverflow = zero;   
  t7 = *(u64 *)&(processor->aluandrotatecontrol);   
  t1 = (t6 == ALUFunction_Boolean) ? 1 : 0;   

force_alignment32674:
  if (_trace) printf("force_alignment32674:\n");
  if (t1 == 0) 
    goto basic_dispatch32615;
  /* Here if argument ALUFunctionBoolean */
  t8 = t7 >> 10;   
  t8 = t8 & 15;		// Extract the ALU boolean function 
  t1 = (t8 == Boole_Clear) ? 1 : 0;   

force_alignment32634:
  if (_trace) printf("force_alignment32634:\n");
  if (t1 != 0)   
    goto basic_dispatch32616;

basic_dispatch32617:
  if (_trace) printf("basic_dispatch32617:\n");
  t1 = (t8 == Boole_And) ? 1 : 0;   

force_alignment32635:
  if (_trace) printf("force_alignment32635:\n");
  if (t1 == 0) 
    goto basic_dispatch32618;
  /* Here if argument BooleAnd */
  t8 = t3 & t5;
  goto basic_dispatch32616;   

basic_dispatch32618:
  if (_trace) printf("basic_dispatch32618:\n");
  t1 = (t8 == Boole_AndC1) ? 1 : 0;   

force_alignment32636:
  if (_trace) printf("force_alignment32636:\n");
  if (t1 == 0) 
    goto basic_dispatch32619;
  /* Here if argument BooleAndC1 */
  t8 = t5 & ~t3;
  goto basic_dispatch32616;   

basic_dispatch32619:
  if (_trace) printf("basic_dispatch32619:\n");
  t1 = (t8 == Boole_2) ? 1 : 0;   

force_alignment32637:
  if (_trace) printf("force_alignment32637:\n");
  if (t1 == 0) 
    goto basic_dispatch32620;
  /* Here if argument Boole2 */
  t8 = t5;
  goto basic_dispatch32616;   

basic_dispatch32620:
  if (_trace) printf("basic_dispatch32620:\n");
  t1 = (t8 == Boole_AndC2) ? 1 : 0;   

force_alignment32638:
  if (_trace) printf("force_alignment32638:\n");
  if (t1 == 0) 
    goto basic_dispatch32621;
  /* Here if argument BooleAndC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch32616;   

basic_dispatch32621:
  if (_trace) printf("basic_dispatch32621:\n");
  t1 = (t8 == Boole_1) ? 1 : 0;   

force_alignment32639:
  if (_trace) printf("force_alignment32639:\n");
  if (t1 == 0) 
    goto basic_dispatch32622;
  /* Here if argument Boole1 */
  t8 = t3;
  goto basic_dispatch32616;   

basic_dispatch32622:
  if (_trace) printf("basic_dispatch32622:\n");
  t1 = (t8 == Boole_Xor) ? 1 : 0;   

force_alignment32640:
  if (_trace) printf("force_alignment32640:\n");
  if (t1 == 0) 
    goto basic_dispatch32623;
  /* Here if argument BooleXor */
  t8 = t3 ^ t5;   
  goto basic_dispatch32616;   

basic_dispatch32623:
  if (_trace) printf("basic_dispatch32623:\n");
  t1 = (t8 == Boole_Ior) ? 1 : 0;   

force_alignment32641:
  if (_trace) printf("force_alignment32641:\n");
  if (t1 == 0) 
    goto basic_dispatch32624;
  /* Here if argument BooleIor */
  t8 = t3 | t5;
  goto basic_dispatch32616;   

basic_dispatch32624:
  if (_trace) printf("basic_dispatch32624:\n");
  t1 = (t8 == Boole_Nor) ? 1 : 0;   

force_alignment32642:
  if (_trace) printf("force_alignment32642:\n");
  if (t1 == 0) 
    goto basic_dispatch32625;
  /* Here if argument BooleNor */
  t8 = t3 | t5;
  t8 = ~t8;   
  goto basic_dispatch32616;   

basic_dispatch32625:
  if (_trace) printf("basic_dispatch32625:\n");
  t1 = (t8 == Boole_Equiv) ? 1 : 0;   

force_alignment32643:
  if (_trace) printf("force_alignment32643:\n");
  if (t1 == 0) 
    goto basic_dispatch32626;
  /* Here if argument BooleEquiv */
  t8 = t3 ^ t5;   
  t8 = ~t8;   
  goto basic_dispatch32616;   

basic_dispatch32626:
  if (_trace) printf("basic_dispatch32626:\n");
  t1 = (t8 == Boole_C1) ? 1 : 0;   

force_alignment32644:
  if (_trace) printf("force_alignment32644:\n");
  if (t1 == 0) 
    goto basic_dispatch32627;
  /* Here if argument BooleC1 */
  t8 = ~t3;   
  goto basic_dispatch32616;   

basic_dispatch32627:
  if (_trace) printf("basic_dispatch32627:\n");
  t1 = (t8 == Boole_OrC1) ? 1 : 0;   

force_alignment32645:
  if (_trace) printf("force_alignment32645:\n");
  if (t1 == 0) 
    goto basic_dispatch32628;
  /* Here if argument BooleOrC1 */
  t8 = t5 | ~(t3);   
  goto basic_dispatch32616;   

basic_dispatch32628:
  if (_trace) printf("basic_dispatch32628:\n");
  t1 = (t8 == Boole_C2) ? 1 : 0;   

force_alignment32646:
  if (_trace) printf("force_alignment32646:\n");
  if (t1 == 0) 
    goto basic_dispatch32629;
  /* Here if argument BooleC2 */
  t8 = ~t5;   
  goto basic_dispatch32616;   

basic_dispatch32629:
  if (_trace) printf("basic_dispatch32629:\n");
  t1 = (t8 == Boole_OrC2) ? 1 : 0;   

force_alignment32647:
  if (_trace) printf("force_alignment32647:\n");
  if (t1 == 0) 
    goto basic_dispatch32630;
  /* Here if argument BooleOrC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch32616;   

basic_dispatch32630:
  if (_trace) printf("basic_dispatch32630:\n");
  t1 = (t8 == Boole_Nand) ? 1 : 0;   

force_alignment32648:
  if (_trace) printf("force_alignment32648:\n");
  if (t1 == 0) 
    goto basic_dispatch32631;
  /* Here if argument BooleNand */
  t8 = t3 & t5;
  goto basic_dispatch32616;   

basic_dispatch32631:
  if (_trace) printf("basic_dispatch32631:\n");
  t1 = (t8 == Boole_Set) ? 1 : 0;   

force_alignment32649:
  if (_trace) printf("force_alignment32649:\n");
  if (t1 == 0) 
    goto basic_dispatch32616;
  /* Here if argument BooleSet */
  t8 = ~zero;   

basic_dispatch32616:
  if (_trace) printf("basic_dispatch32616:\n");

basic_dispatch32614:
  if (_trace) printf("basic_dispatch32614:\n");
  t1 = t7 >> 16;   
  t1 = t1 & 31;		// Extract ALU condition 
  t10 = *(u64 *)&(processor->aluoverflow);   
  t11 = *(u64 *)&(processor->aluborrow);   
  t12 = *(u64 *)&(processor->alulessthan);   
  t9 = (t1 == ALUCondition_SignedLessThanOrEqual) ? 1 : 0;   

force_alignment32706:
  if (_trace) printf("force_alignment32706:\n");
  if (t9 == 0) 
    goto basic_dispatch32679;
  /* Here if argument ALUConditionSignedLessThanOrEqual */
  if (t12 != 0)   
    goto alu_compute_condition32675;
  if (t8 == 0) 
    goto alu_compute_condition32675;

basic_dispatch32678:
  if (_trace) printf("basic_dispatch32678:\n");

alu_compute_condition32676:
  if (_trace) printf("alu_compute_condition32676:\n");
  t1 = zero;
  goto alu_compute_condition32677;   

alu_compute_condition32675:
  if (_trace) printf("alu_compute_condition32675:\n");
  t1 = 1;

alu_compute_condition32677:
  if (_trace) printf("alu_compute_condition32677:\n");
  t9 = t7 >> 21;   
  t9 = t9 & 1;		// Extract the condition sense 
  t1 = t1 ^ t9;   
  if (t1 != 0)   
    goto i_block_n_read_test32601;
  t1 = arg1 & 4;		// =no-incrementp 
  if (t1 != 0)   		// J. if we don't have to increment the address. 
    goto i_block_n_read_test32597;
  arg3 = arg3 + 1;		// Increment the address 

i_block_n_read_test32597:
  if (_trace) printf("i_block_n_read_test32597:\n");
  *(u32 *)arg2 = arg3;   		// Store updated vma in BAR 
  goto NEXTINSTRUCTION;   

i_block_n_read_test32601:
  if (_trace) printf("i_block_n_read_test32601:\n");
  t10 = *(s32 *)(iSP + -8);   
  t9 = *(s32 *)(iSP + -4);   
  t10 = (u32)t10;   
  t10 = t10 << 1;   
  iPC = t9 & 1;
  iPC = iPC + t10;
  goto interpretinstructionforjump;   

i_block_n_read_test32600:
  if (_trace) printf("i_block_n_read_test32600:\n");
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  arg3 = *(u64 *)&(processor->stackcachebasevma);   
  t9 = iSP - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  arg3 = t9 + arg3;		// reconstruct VMA 
  arg5 = arg3;
  arg2 = 23;
  goto illegaloperand;

i_block_n_read_test32599:
  if (_trace) printf("i_block_n_read_test32599:\n");
  arg5 = arg3;
  arg2 = 23;
  goto illegaloperand;

basic_dispatch32679:
  if (_trace) printf("basic_dispatch32679:\n");
  t9 = (t1 == ALUCondition_SignedLessThan) ? 1 : 0;   

force_alignment32707:
  if (_trace) printf("force_alignment32707:\n");
  if (t9 == 0) 
    goto basic_dispatch32680;
  /* Here if argument ALUConditionSignedLessThan */
  if (t12 != 0)   
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32680:
  if (_trace) printf("basic_dispatch32680:\n");
  t9 = (t1 == ALUCondition_Negative) ? 1 : 0;   

force_alignment32708:
  if (_trace) printf("force_alignment32708:\n");
  if (t9 == 0) 
    goto basic_dispatch32681;
  /* Here if argument ALUConditionNegative */
  if ((s64)t8 < 0)   
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32681:
  if (_trace) printf("basic_dispatch32681:\n");
  t9 = (t1 == ALUCondition_SignedOverflow) ? 1 : 0;   

force_alignment32709:
  if (_trace) printf("force_alignment32709:\n");
  if (t9 == 0) 
    goto basic_dispatch32682;
  /* Here if argument ALUConditionSignedOverflow */
  if (t10 != 0)   
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32682:
  if (_trace) printf("basic_dispatch32682:\n");
  t9 = (t1 == ALUCondition_UnsignedLessThanOrEqual) ? 1 : 0;   

force_alignment32710:
  if (_trace) printf("force_alignment32710:\n");
  if (t9 == 0) 
    goto basic_dispatch32683;
  /* Here if argument ALUConditionUnsignedLessThanOrEqual */
  if (t11 != 0)   
    goto alu_compute_condition32675;
  if (t8 == 0) 
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32683:
  if (_trace) printf("basic_dispatch32683:\n");
  t9 = (t1 == ALUCondition_UnsignedLessThan) ? 1 : 0;   

force_alignment32711:
  if (_trace) printf("force_alignment32711:\n");
  if (t9 == 0) 
    goto basic_dispatch32684;
  /* Here if argument ALUConditionUnsignedLessThan */
  if (t11 != 0)   
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32684:
  if (_trace) printf("basic_dispatch32684:\n");
  t9 = (t1 == ALUCondition_Zero) ? 1 : 0;   

force_alignment32712:
  if (_trace) printf("force_alignment32712:\n");
  if (t9 == 0) 
    goto basic_dispatch32685;
  /* Here if argument ALUConditionZero */
  if (t8 == 0) 
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32685:
  if (_trace) printf("basic_dispatch32685:\n");
  t9 = (t1 == ALUCondition_High25Zero) ? 1 : 0;   

force_alignment32713:
  if (_trace) printf("force_alignment32713:\n");
  if (t9 == 0) 
    goto basic_dispatch32686;
  /* Here if argument ALUConditionHigh25Zero */
  t1 = t8 >> 7;   
  if (t1 == 0) 
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32686:
  if (_trace) printf("basic_dispatch32686:\n");
  t9 = (t1 == ALUCondition_Eq) ? 1 : 0;   

force_alignment32714:
  if (_trace) printf("force_alignment32714:\n");
  if (t9 == 0) 
    goto basic_dispatch32687;
  /* Here if argument ALUConditionEq */
  if (t8 != 0)   
    goto alu_compute_condition32676;
  t9 = t2 ^ t4;   
  /* TagType. */
  t9 = t9 & 63;
  if (t9 == 0) 
    goto alu_compute_condition32675;
  goto basic_dispatch32678;   

basic_dispatch32687:
  if (_trace) printf("basic_dispatch32687:\n");
  t9 = (t1 == ALUCondition_Op1Ephemeralp) ? 1 : 0;   

force_alignment32715:
  if (_trace) printf("force_alignment32715:\n");
  if (t9 == 0) 
    goto basic_dispatch32688;
  /* Here if argument ALUConditionOp1Ephemeralp */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32688:
  if (_trace) printf("basic_dispatch32688:\n");
  t9 = (t1 == ALUCondition_ResultTypeNil) ? 1 : 0;   

force_alignment32716:
  if (_trace) printf("force_alignment32716:\n");
  if (t9 == 0) 
    goto basic_dispatch32689;
  /* Here if argument ALUConditionResultTypeNil */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32689:
  if (_trace) printf("basic_dispatch32689:\n");
  t9 = (t1 == ALUCondition_Op2Fixnum) ? 1 : 0;   

force_alignment32717:
  if (_trace) printf("force_alignment32717:\n");
  if (t9 == 0) 
    goto basic_dispatch32690;
  /* Here if argument ALUConditionOp2Fixnum */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32690:
  if (_trace) printf("basic_dispatch32690:\n");
  t9 = (t1 == ALUCondition_False) ? 1 : 0;   

force_alignment32718:
  if (_trace) printf("force_alignment32718:\n");
  if (t9 == 0) 
    goto basic_dispatch32691;
  /* Here if argument ALUConditionFalse */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32691:
  if (_trace) printf("basic_dispatch32691:\n");
  t9 = (t1 == ALUCondition_ResultCdrLow) ? 1 : 0;   

force_alignment32719:
  if (_trace) printf("force_alignment32719:\n");
  if (t9 == 0) 
    goto basic_dispatch32692;
  /* Here if argument ALUConditionResultCdrLow */
  /* TagCdr. */
  t9 = t2 >> 6;   
  t1 = t9 & 1;
  goto alu_compute_condition32677;   

basic_dispatch32692:
  if (_trace) printf("basic_dispatch32692:\n");
  t9 = (t1 == ALUCondition_CleanupBitsSet) ? 1 : 0;   

force_alignment32720:
  if (_trace) printf("force_alignment32720:\n");
  if (t9 == 0) 
    goto basic_dispatch32693;
  /* Here if argument ALUConditionCleanupBitsSet */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32693:
  if (_trace) printf("basic_dispatch32693:\n");
  t9 = (t1 == ALUCondition_AddressInStackCache) ? 1 : 0;   

force_alignment32721:
  if (_trace) printf("force_alignment32721:\n");
  if (t9 == 0) 
    goto basic_dispatch32694;
  /* Here if argument ALUConditionAddressInStackCache */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32694:
  if (_trace) printf("basic_dispatch32694:\n");
  t9 = (t1 == ALUCondition_ExtraStackMode) ? 1 : 0;   

force_alignment32722:
  if (_trace) printf("force_alignment32722:\n");
  if (t9 == 0) 
    goto basic_dispatch32695;
  /* Here if argument ALUConditionExtraStackMode */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32695:
  if (_trace) printf("basic_dispatch32695:\n");
  t9 = (t1 == ALUCondition_FepMode) ? 1 : 0;   

force_alignment32723:
  if (_trace) printf("force_alignment32723:\n");
  if (t9 == 0) 
    goto basic_dispatch32696;
  /* Here if argument ALUConditionFepMode */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32696:
  if (_trace) printf("basic_dispatch32696:\n");
  t9 = (t1 == ALUCondition_FpCoprocessorPresent) ? 1 : 0;   

force_alignment32724:
  if (_trace) printf("force_alignment32724:\n");
  if (t9 == 0) 
    goto basic_dispatch32697;
  /* Here if argument ALUConditionFpCoprocessorPresent */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32697:
  if (_trace) printf("basic_dispatch32697:\n");
  t9 = (t1 == ALUCondition_Op1Oldspacep) ? 1 : 0;   

force_alignment32725:
  if (_trace) printf("force_alignment32725:\n");
  if (t9 == 0) 
    goto basic_dispatch32698;
  /* Here if argument ALUConditionOp1Oldspacep */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32698:
  if (_trace) printf("basic_dispatch32698:\n");
  t9 = (t1 == ALUCondition_PendingSequenceBreakEnabled) ? 1 : 0;   

force_alignment32726:
  if (_trace) printf("force_alignment32726:\n");
  if (t9 == 0) 
    goto basic_dispatch32699;
  /* Here if argument ALUConditionPendingSequenceBreakEnabled */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32699:
  if (_trace) printf("basic_dispatch32699:\n");
  t9 = (t1 == ALUCondition_Op1TypeAcceptable) ? 1 : 0;   

force_alignment32727:
  if (_trace) printf("force_alignment32727:\n");
  if (t9 == 0) 
    goto basic_dispatch32700;
  /* Here if argument ALUConditionOp1TypeAcceptable */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32700:
  if (_trace) printf("basic_dispatch32700:\n");
  t9 = (t1 == ALUCondition_Op1TypeCondition) ? 1 : 0;   

force_alignment32728:
  if (_trace) printf("force_alignment32728:\n");
  if (t9 == 0) 
    goto basic_dispatch32701;
  /* Here if argument ALUConditionOp1TypeCondition */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32701:
  if (_trace) printf("basic_dispatch32701:\n");
  t9 = (t1 == ALUCondition_StackCacheOverflow) ? 1 : 0;   

force_alignment32729:
  if (_trace) printf("force_alignment32729:\n");
  if (t9 == 0) 
    goto basic_dispatch32702;
  /* Here if argument ALUConditionStackCacheOverflow */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32702:
  if (_trace) printf("basic_dispatch32702:\n");
  t9 = (t1 == ALUCondition_OrLogicVariable) ? 1 : 0;   

force_alignment32730:
  if (_trace) printf("force_alignment32730:\n");
  if (t9 == 0) 
    goto basic_dispatch32703;
  /* Here if argument ALUConditionOrLogicVariable */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32703:
  if (_trace) printf("basic_dispatch32703:\n");
  /* Here for all other cases */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32615:
  if (_trace) printf("basic_dispatch32615:\n");
  t1 = (t6 == ALUFunction_Byte) ? 1 : 0;   

force_alignment32731:
  if (_trace) printf("force_alignment32731:\n");
  if (t1 == 0) 
    goto basic_dispatch32650;
  /* Here if argument ALUFunctionByte */
  t9 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t10 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t1 = t7 >> 10;   
  t1 = t1 & 3;		// Extract the byte background 
  t11 = (t1 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment32657:
  if (_trace) printf("force_alignment32657:\n");
  if (t11 == 0) 
    goto basic_dispatch32653;
  /* Here if argument ALUByteBackgroundOp1 */
  t1 = t3;

basic_dispatch32652:
  if (_trace) printf("basic_dispatch32652:\n");
  t12 = t7 >> 12;   
  t12 = t12 & 1;		// Extractthe byte rotate latch 
  t8 = t5 << (t9 & 63);   
  t11 = (u32)(t8 >> ((4&7)*8));   
  t8 = (u32)t8;   
  t8 = t8 | t11;		// OP2 rotated 
  if (t12 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte32651;
  *(u64 *)&processor->rotatelatch = t8;   

alu_function_byte32651:
  if (_trace) printf("alu_function_byte32651:\n");
  t12 = zero + -2;   
  t12 = t12 << (t10 & 63);   
  t12 = ~t12;   		// Compute mask 
  /* Get byte function */
  t11 = t7 >> 13;   
  t11 = t11 & 1;
  t10 = (t11 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment32662:
  if (_trace) printf("force_alignment32662:\n");
  if (t10 == 0) 
    goto basic_dispatch32659;
  /* Here if argument ALUByteFunctionDpb */
  t12 = t12 << (t9 & 63);   		// Position mask 

basic_dispatch32658:
  if (_trace) printf("basic_dispatch32658:\n");
  t8 = t8 & t12;		// rotated&mask 
  t1 = t1 & ~t12;		// background&~mask 
  t8 = t8 | t1;
  goto basic_dispatch32614;   

basic_dispatch32650:
  if (_trace) printf("basic_dispatch32650:\n");
  t1 = (t6 == ALUFunction_Adder) ? 1 : 0;   

force_alignment32732:
  if (_trace) printf("force_alignment32732:\n");
  if (t1 == 0) 
    goto basic_dispatch32663;
  /* Here if argument ALUFunctionAdder */
  t10 = t7 >> 11;   
  t10 = t10 & 3;		// Extract the op2 
  t9 = t7 >> 10;   
  t9 = t9 & 1;		// Extract the adder carry in 
  t11 = (t10 == ALUAdderOp2_Op2) ? 1 : 0;   

force_alignment32671:
  if (_trace) printf("force_alignment32671:\n");
  if (t11 == 0) 
    goto basic_dispatch32666;
  /* Here if argument ALUAdderOp2Op2 */
  t1 = t5;

basic_dispatch32665:
  if (_trace) printf("basic_dispatch32665:\n");
  t8 = t3 + t1;
  t8 = t8 + t9;
  t10 = t8 >> 31;   		// Sign bit 
  t11 = t8 >> 32;   		// Next bit 
  t10 = t10 ^ t11;   		// Low bit is now overflow indicator 
  t11 = t7 >> 24;   		// Get the load-carry-in bit 
  *(u64 *)&processor->aluoverflow = t10;   
  if ((t11 & 1) == 0)   
    goto alu_function_adder32664;
  t10 = (u32)(t8 >> ((4&7)*8));   		// Get the carry 
  t11 = zero + 1024;   
  t7 = t7 & ~t11;
  t11 = t10 & 1;
  t11 = t11 << 10;   
  t7 = t7 | t11;		// Set the adder carry in 
  *(u64 *)&processor->aluandrotatecontrol = t7;   

alu_function_adder32664:
  if (_trace) printf("alu_function_adder32664:\n");
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->aluborrow = t10;   
  t3 = (s32)t3;
  t5 = (s32)t5;
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->alulessthan = t10;   
  goto basic_dispatch32614;   

basic_dispatch32663:
  if (_trace) printf("basic_dispatch32663:\n");
  t1 = (t6 == ALUFunction_MultiplyDivide) ? 1 : 0;   

force_alignment32733:
  if (_trace) printf("force_alignment32733:\n");
  if (t1 == 0) 
    goto basic_dispatch32614;
  /* Here if argument ALUFunctionMultiplyDivide */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch32666:
  if (_trace) printf("basic_dispatch32666:\n");
  t11 = (t10 == ALUAdderOp2_Zero) ? 1 : 0;   

force_alignment32734:
  if (_trace) printf("force_alignment32734:\n");
  if (t11 == 0) 
    goto basic_dispatch32667;
  /* Here if argument ALUAdderOp2Zero */
  t1 = zero;
  goto basic_dispatch32665;   

basic_dispatch32667:
  if (_trace) printf("basic_dispatch32667:\n");
  t11 = (t10 == ALUAdderOp2_Invert) ? 1 : 0;   

force_alignment32735:
  if (_trace) printf("force_alignment32735:\n");
  if (t11 == 0) 
    goto basic_dispatch32668;
  /* Here if argument ALUAdderOp2Invert */
  t1 = (s32)t5;
  t1 = zero - t1;   
  t1 = (u32)t1;   
  goto basic_dispatch32665;   

basic_dispatch32668:
  if (_trace) printf("basic_dispatch32668:\n");
  t11 = (t10 == ALUAdderOp2_MinusOne) ? 1 : 0;   

force_alignment32736:
  if (_trace) printf("force_alignment32736:\n");
  if (t11 == 0) 
    goto basic_dispatch32665;
  /* Here if argument ALUAdderOp2MinusOne */
  t1 = ~zero;   
  t1 = (u32)t1;   
  goto basic_dispatch32665;   

basic_dispatch32659:
  if (_trace) printf("basic_dispatch32659:\n");
  t10 = (t11 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment32737:
  if (_trace) printf("force_alignment32737:\n");
  if (t10 != 0)   
    goto basic_dispatch32658;
  goto basic_dispatch32658;   

basic_dispatch32653:
  if (_trace) printf("basic_dispatch32653:\n");
  t11 = (t1 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment32738:
  if (_trace) printf("force_alignment32738:\n");
  if (t11 == 0) 
    goto basic_dispatch32654;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t1 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch32652;   

basic_dispatch32654:
  if (_trace) printf("basic_dispatch32654:\n");
  t11 = (t1 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment32739:
  if (_trace) printf("force_alignment32739:\n");
  if (t11 == 0) 
    goto basic_dispatch32652;
  /* Here if argument ALUByteBackgroundZero */
  t1 = zero;
  goto basic_dispatch32652;   

vma_memory_read32604:
  if (_trace) printf("vma_memory_read32604:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t10;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read32603;   

vma_memory_read32606:
  if (_trace) printf("vma_memory_read32606:\n");

vma_memory_read32605:
  if (_trace) printf("vma_memory_read32605:\n");
  t12 = (t1 * 4);   		// Cycle-number -> table offset 
  t12 = (t12 * 4) + ivory;   
  t12 = *(u64 *)(t12 + PROCESSORSTATE_DATAREAD);   
  /* TagType. */
  t11 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg3;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + t12;   		// Adjust for a longword load 
  t12 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read32611:
  if (_trace) printf("vma_memory_read32611:\n");
  t10 = t12 & MemoryActionIndirect;
  if (t10 == 0) 
    goto vma_memory_read32610;
  arg3 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32602;   

vma_memory_read32610:
  if (_trace) printf("vma_memory_read32610:\n");
  t11 = t12 & MemoryActionTransform;
  if (t11 == 0) 
    goto vma_memory_read32609;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read32613;   

vma_memory_read32609:

vma_memory_read32608:
  /* Perform memory action */
  arg1 = t12;
  arg2 = t1;
  goto performmemoryaction;

/* end DoBlock1ReadTest */
  /* End of Halfword operand from stack instruction - DoBlock1ReadTest */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunblok.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunbind.as
 ************************************************************************/

  /* Binding Instructions. */
/* start DoBindLocativeToValue */

  /* Halfword operand from stack instruction - DoBindLocativeToValue */

dobindlocativetovalue:
  if (_trace) printf("dobindlocativetovalue:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoBindLocativeToValueIM:
  if (_trace) printf("DoBindLocativeToValueIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32772:
  if (_trace) printf("force_alignment32772:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindobindlocativetovalue;   

DoBindLocativeToValueSP:
  if (_trace) printf("DoBindLocativeToValueSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBindLocativeToValueLP:
  if (_trace) printf("DoBindLocativeToValueLP:\n");

DoBindLocativeToValueFP:
  if (_trace) printf("DoBindLocativeToValueFP:\n");

headdobindlocativetovalue:
  if (_trace) printf("headdobindlocativetovalue:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindobindlocativetovalue:
  if (_trace) printf("begindobindlocativetovalue:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg6 = *(s32 *)iSP;   		// ltag/ldata 
  arg5 = *(s32 *)(iSP + 4);   		// ltag/ldata 
  iSP = iSP - 8;   		// Pop Stack. 
  arg6 = (u32)arg6;   
  arg3 = *(u64 *)&(processor->bindingstackpointer);   
  arg2 = arg1 >> 32;   		// new tag 
  arg4 = *(u64 *)&(processor->bindingstacklimit);   
  arg1 = (u32)arg1;   		// new data 
  t1 = arg5 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto bindloctovaliop;
  arg3 = (u32)arg3;   
  arg4 = (u32)arg4;   
  t1 = arg3 - arg4;   
  if ((s64)t1 >= 0)   		// J. if binding stack overflow 
    goto bindloctovalov;
  t3 = arg3 + 1;
  t9 = *(s32 *)&processor->control;   
  t8 = arg6;
  /* Memory Read Internal */

vma_memory_read32740:
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t6 = t8 + ivory;
  t5 = *(s32 *)&processor->scovlimit;   
  t1 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = t8 - t4;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->bindread_mask);   
  t5 = ((u64)t4 < (u64)t5) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read32742;

vma_memory_read32741:
  t6 = zero + 224;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read32744;

vma_memory_read32749:
  t10 = t9 >> 19;   
  /* TagType. */
  t8 = arg5 & 63;
  t10 = t10 & 64;		// Extract the CR.cleanup-bindings bit 
  t11 = t10 | t8;
  t5 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t7 = (t4 * 4);   
  t6 = LDQ_U(t4);   
  t5 = t3 - t5;   		// Stack cache offset 
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;   		// In range? 
  t5 = (t11 & 0xff) << ((t4&7)*8);   
  t6 = t6 & ~(0xffL << (t4&7)*8);   

force_alignment32752:
  if (_trace) printf("force_alignment32752:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = arg6;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write32751;

vma_memory_write32750:
  t3 = arg3 + 2;
  t5 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t7 = (t4 * 4);   
  t6 = LDQ_U(t4);   
  t5 = t3 - t5;   		// Stack cache offset 
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;   		// In range? 
  t5 = (t2 & 0xff) << ((t4&7)*8);   
  t6 = t6 & ~(0xffL << (t4&7)*8);   

force_alignment32755:
  if (_trace) printf("force_alignment32755:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = t1;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write32754;

vma_memory_write32753:
  t1 = (512) << 16;   
  /* Memory Read Internal */

vma_memory_read32756:
  t6 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t8 = arg6 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg6 - t6;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->bindwrite_mask);   
  t7 = ((u64)t6 < (u64)t7) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32758;

vma_memory_read32757:
  t8 = zero + 224;   
  t10 = t10 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t10 & 1)   
    goto vma_memory_read32760;

vma_memory_read32765:
  /* Merge cdr-code */
  t5 = arg2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t7 = *(u64 *)&(processor->stackcachebasevma);   
  t6 = arg6 + ivory;
  t10 = *(s32 *)&processor->scovlimit;   
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = arg6 - t7;   		// Stack cache offset 
  t10 = ((u64)t7 < (u64)t10) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment32768:
  if (_trace) printf("force_alignment32768:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = arg1;   
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write32767;

vma_memory_write32766:
  t9 = t1 | t9;		// Set cr.cleanup-bindings bit 
  *(u32 *)&processor->control = t9;   
  *(u32 *)&processor->bindingstackpointer = t3;   		// vma only 
  goto NEXTINSTRUCTION;   

bindloctovalov:
  if (_trace) printf("bindloctovalov:\n");
  arg5 = 0;
  arg2 = 19;
  goto illegaloperand;

bindloctovaliop:
  if (_trace) printf("bindloctovaliop:\n");
  arg5 = 0;
  arg2 = 18;
  goto illegaloperand;

bindloctovaldeep:
  if (_trace) printf("bindloctovaldeep:\n");
  t1 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 72;
  goto illegaloperand;

vma_memory_write32767:
  if (_trace) printf("vma_memory_write32767:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32769:
  if (_trace) printf("force_alignment32769:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg6 - t7;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = arg1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto vma_memory_write32766;   

vma_memory_read32758:
  if (_trace) printf("vma_memory_read32758:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read32757;   

vma_memory_read32760:
  if (_trace) printf("vma_memory_read32760:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read32759;
  arg6 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read32756;   

vma_memory_read32759:
  if (_trace) printf("vma_memory_read32759:\n");
  t10 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg6;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read32762:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_write32754:
  if (_trace) printf("vma_memory_write32754:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32770:
  if (_trace) printf("force_alignment32770:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  *(u32 *)t4 = t1;   		// Store in stack 
  *(u32 *)(t4 + 4) = t2;   		// write the stack cache 
  goto vma_memory_write32753;   

vma_memory_write32751:
  if (_trace) printf("vma_memory_write32751:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32771:
  if (_trace) printf("force_alignment32771:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  *(u32 *)t4 = arg6;   		// Store in stack 
  *(u32 *)(t4 + 4) = t11;   		// write the stack cache 
  goto vma_memory_write32750;   

vma_memory_read32742:
  if (_trace) printf("vma_memory_read32742:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read32741;   

vma_memory_read32744:
  if (_trace) printf("vma_memory_read32744:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read32743;
  t8 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read32740;   

vma_memory_read32743:
  if (_trace) printf("vma_memory_read32743:\n");
  t7 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read32746:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 2;
  goto performmemoryaction;

/* end DoBindLocativeToValue */
  /* End of Halfword operand from stack instruction - DoBindLocativeToValue */
/* start DoBindLocative */

  /* Halfword operand from stack instruction - DoBindLocative */
  /* arg2 has the preloaded 8 bit operand. */

dobindlocative:
  if (_trace) printf("dobindlocative:\n");

DoBindLocativeSP:
  if (_trace) printf("DoBindLocativeSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoBindLocativeLP:
  if (_trace) printf("DoBindLocativeLP:\n");

DoBindLocativeFP:
  if (_trace) printf("DoBindLocativeFP:\n");

begindobindlocative:
  if (_trace) printf("begindobindlocative:\n");
  /* arg1 has the operand address. */
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 
  arg3 = *(u64 *)&(processor->bindingstackpointer);   
  arg5 = arg1 >> 32;   		// tag 
  arg4 = *(u64 *)&(processor->bindingstacklimit);   
  arg6 = (u32)arg1;   		// data 
  t1 = arg5 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto bindlociop;
  arg3 = (u32)arg3;   
  arg4 = (u32)arg4;   
  t1 = arg3 - arg4;   
  if ((s64)t1 >= 0)   		// J. if binding stack overflow 
    goto bindlocov;
  t3 = arg3 + 1;
  t9 = *(s32 *)&processor->control;   
  t8 = arg6;
  /* Memory Read Internal */

vma_memory_read32773:
  t4 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t6 = t8 + ivory;
  t5 = *(s32 *)&processor->scovlimit;   
  t1 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = t8 - t4;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->bindread_mask);   
  t5 = ((u64)t4 < (u64)t5) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read32775;

vma_memory_read32774:
  t6 = zero + 224;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read32777;

vma_memory_read32782:
  t10 = t9 >> 19;   
  /* TagType. */
  t8 = arg5 & 63;
  t10 = t10 & 64;		// Extract the CR.cleanup-bindings bit 
  t11 = t10 | t8;
  t5 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t7 = (t4 * 4);   
  t6 = LDQ_U(t4);   
  t5 = t3 - t5;   		// Stack cache offset 
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;   		// In range? 
  t5 = (t11 & 0xff) << ((t4&7)*8);   
  t6 = t6 & ~(0xffL << (t4&7)*8);   

force_alignment32785:
  if (_trace) printf("force_alignment32785:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = arg6;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write32784;

vma_memory_write32783:
  t3 = arg3 + 2;
  t5 = *(u64 *)&(processor->stackcachebasevma);   
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t7 = (t4 * 4);   
  t6 = LDQ_U(t4);   
  t5 = t3 - t5;   		// Stack cache offset 
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;   		// In range? 
  t5 = (t2 & 0xff) << ((t4&7)*8);   
  t6 = t6 & ~(0xffL << (t4&7)*8);   

force_alignment32788:
  if (_trace) printf("force_alignment32788:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = t1;   
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write32787;

vma_memory_write32786:
  t1 = (512) << 16;   
  t9 = t1 | t9;		// Set cr.cleanup-bindings bit 
  *(u32 *)&processor->control = t9;   
  *(u32 *)&processor->bindingstackpointer = t3;   		// vma only 
  goto NEXTINSTRUCTION;   

bindlocov:
  if (_trace) printf("bindlocov:\n");
  arg5 = 0;
  arg2 = 19;
  goto illegaloperand;

bindlociop:
  if (_trace) printf("bindlociop:\n");
  arg5 = 0;
  arg2 = 18;
  goto illegaloperand;

bindlocdeep:
  if (_trace) printf("bindlocdeep:\n");
  t1 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 72;
  goto illegaloperand;

vma_memory_write32787:
  if (_trace) printf("vma_memory_write32787:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32789:
  if (_trace) printf("force_alignment32789:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  *(u32 *)t4 = t1;   		// Store in stack 
  *(u32 *)(t4 + 4) = t2;   		// write the stack cache 
  goto vma_memory_write32786;   

vma_memory_write32784:
  if (_trace) printf("vma_memory_write32784:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment32790:
  if (_trace) printf("force_alignment32790:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  *(u32 *)t4 = arg6;   		// Store in stack 
  *(u32 *)(t4 + 4) = t11;   		// write the stack cache 
  goto vma_memory_write32783;   

vma_memory_read32775:
  if (_trace) printf("vma_memory_read32775:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read32774;   

vma_memory_read32777:
  if (_trace) printf("vma_memory_read32777:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read32776;
  t8 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read32773;   

vma_memory_read32776:
  if (_trace) printf("vma_memory_read32776:\n");
  t7 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read32779:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 2;
  goto performmemoryaction;

DoBindLocativeIM:
  goto doistageerror;

/* end DoBindLocative */
  /* End of Halfword operand from stack instruction - DoBindLocative */
/* start DoUnbindN */

  /* Halfword operand from stack instruction - DoUnbindN */
  /* arg2 has the preloaded 8 bit operand. */

dounbindn:
  if (_trace) printf("dounbindn:\n");

DoUnbindNIM:
  if (_trace) printf("DoUnbindNIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindounbindn;   

DoUnbindNSP:
  if (_trace) printf("DoUnbindNSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoUnbindNLP:
  if (_trace) printf("DoUnbindNLP:\n");

DoUnbindNFP:
  if (_trace) printf("DoUnbindNFP:\n");

headdounbindn:
  if (_trace) printf("headdounbindn:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindounbindn:
  if (_trace) printf("begindounbindn:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t1 = arg2 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto unbindniop;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto unbindnendloop;   

unbindntoploop:
  if (_trace) printf("unbindntoploop:\n");
  arg1 = arg1 - 1;   
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  t2 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & t2;
  t4 = t4 & ~t2;		// Turn off the bit 
  if (t3 != 0)   
    goto g32791;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g32791:
  if (_trace) printf("g32791:\n");
  /* Memory Read Internal */

vma_memory_read32792:
  arg4 = t1 + ivory;
  t6 = (arg4 * 4);   
  t7 = LDQ_U(arg4);   
  t8 = t1 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read32794;

vma_memory_read32793:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t7 & 63);   
  arg4 = arg4 >> (t7 & 63);   
  if (arg5 & 1)   
    goto vma_memory_read32796;

vma_memory_read32801:
  /* Memory Read Internal */

vma_memory_read32802:
  arg4 = t5 + ivory;
  t2 = (arg4 * 4);   
  t3 = LDQ_U(arg4);   
  t8 = t5 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t3 = (u8)(t3 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read32804;

vma_memory_read32803:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t3 & 63);   
  arg4 = arg4 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (arg5 & 1)   
    goto vma_memory_read32806;

vma_memory_read32811:
  /* Memory Read Internal */

vma_memory_read32812:
  arg6 = t2 + ivory;
  arg3 = (arg6 * 4);   
  t8 = LDQ_U(arg6);   
  arg4 = t2 - t11;   		// Stack cache offset 
  arg5 = ((u64)arg4 < (u64)t12) ? 1 : 0;   		// In range? 
  arg3 = *(s32 *)arg3;   
  t8 = (u8)(t8 >> ((arg6&7)*8));   
  if (arg5 != 0)   
    goto vma_memory_read32814;

vma_memory_read32813:
  arg4 = *(u64 *)&(processor->bindwrite_mask);   
  arg6 = zero + 224;   
  arg4 = arg4 >> (t8 & 63);   
  arg6 = arg6 >> (t8 & 63);   
  if (arg4 & 1)   
    goto vma_memory_read32816;

vma_memory_read32821:
  /* Merge cdr-code */
  arg3 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | arg3;
  arg4 = t2 + ivory;
  arg3 = (arg4 * 4);   
  arg6 = LDQ_U(arg4);   
  arg5 = (t8 & 0xff) << ((arg4&7)*8);   
  arg6 = arg6 & ~(0xffL << (arg4&7)*8);   

force_alignment32824:
  if (_trace) printf("force_alignment32824:\n");
  arg6 = arg6 | arg5;
  STQ_U(arg4, arg6);   
  arg4 = *(s32 *)&processor->scovlimit;   
  arg5 = t2 - t11;   		// Stack cache offset 
  arg4 = ((u64)arg5 < (u64)arg4) ? 1 : 0;   		// In range? 
  *(u32 *)arg3 = t6;   
  if (arg4 != 0)   		// J. if in cache 
    goto vma_memory_write32823;

vma_memory_write32822:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
  *(u32 *)&processor->bindingstackpointer = t1;   		// vma only 
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   

unbindnendloop:
  if (_trace) printf("unbindnendloop:\n");
  if ((s64)arg1 > 0)   
    goto unbindntoploop;
  t3 = *(s32 *)&processor->interruptreg;   
  t4 = t3 & 2;
  t4 = (t4 == 2) ? 1 : 0;   
  t3 = t3 | t4;
  *(u32 *)&processor->interruptreg = t3;   
  if (t3 == 0) 
    goto NEXTINSTRUCTION;
  *(u64 *)&processor->stop_interpreter = t3;   
  goto NEXTINSTRUCTION;   

unbindniop:
  if (_trace) printf("unbindniop:\n");
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

vma_memory_write32823:
  if (_trace) printf("vma_memory_write32823:\n");
  arg4 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg5 * 8) + arg4;  		// reconstruct SCA 
  *(u32 *)arg4 = t6;   		// Store in stack 
  *(u32 *)(arg4 + 4) = t8;   		// write the stack cache 
  goto vma_memory_write32822;   

vma_memory_read32814:
  if (_trace) printf("vma_memory_read32814:\n");
  arg5 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg4 * 8) + arg5;  		// reconstruct SCA 
  arg3 = *(s32 *)arg4;   
  t8 = *(s32 *)(arg4 + 4);   		// Read from stack cache 
  goto vma_memory_read32813;   

vma_memory_read32816:
  if (_trace) printf("vma_memory_read32816:\n");
  if ((arg6 & 1) == 0)   
    goto vma_memory_read32815;
  t2 = (u32)arg3;   		// Do the indirect thing 
  goto vma_memory_read32812;   

vma_memory_read32815:
  if (_trace) printf("vma_memory_read32815:\n");
  arg4 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  arg6 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  arg6 = (arg6 * 4) + arg4;   		// Adjust for a longword load 
  arg4 = *(s32 *)arg6;   		// Get the memory action 

vma_memory_read32818:
  /* Perform memory action */
  arg1 = arg4;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read32804:
  if (_trace) printf("vma_memory_read32804:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read32803;   

vma_memory_read32806:
  if (_trace) printf("vma_memory_read32806:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read32805;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read32802;   

vma_memory_read32805:
  if (_trace) printf("vma_memory_read32805:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read32808:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read32794:
  if (_trace) printf("vma_memory_read32794:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read32793;   

vma_memory_read32796:
  if (_trace) printf("vma_memory_read32796:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read32795;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read32792;   

vma_memory_read32795:
  if (_trace) printf("vma_memory_read32795:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read32798:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

/* end DoUnbindN */
  /* End of Halfword operand from stack instruction - DoUnbindN */
/* start DoRestoreBindingStack */

  /* Halfword operand from stack instruction - DoRestoreBindingStack */
  /* arg2 has the preloaded 8 bit operand. */

dorestorebindingstack:
  if (_trace) printf("dorestorebindingstack:\n");

DoRestoreBindingStackIM:
  if (_trace) printf("DoRestoreBindingStackIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindorestorebindingstack;   

DoRestoreBindingStackSP:
  if (_trace) printf("DoRestoreBindingStackSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoRestoreBindingStackLP:
  if (_trace) printf("DoRestoreBindingStackLP:\n");

DoRestoreBindingStackFP:
  if (_trace) printf("DoRestoreBindingStackFP:\n");

headdorestorebindingstack:
  if (_trace) printf("headdorestorebindingstack:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindorestorebindingstack:
  if (_trace) printf("begindorestorebindingstack:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t1 = arg2 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto restorebsiop;
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto restorebsendloop;   

restorebstoploop:
  if (_trace) printf("restorebstoploop:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  t2 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & t2;
  t4 = t4 & ~t2;		// Turn off the bit 
  if (t3 != 0)   
    goto g32825;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g32825:
  if (_trace) printf("g32825:\n");
  /* Memory Read Internal */

vma_memory_read32826:
  arg4 = t1 + ivory;
  t6 = (arg4 * 4);   
  t7 = LDQ_U(arg4);   
  t8 = t1 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read32828;

vma_memory_read32827:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t7 & 63);   
  arg4 = arg4 >> (t7 & 63);   
  if (arg5 & 1)   
    goto vma_memory_read32830;

vma_memory_read32835:
  /* Memory Read Internal */

vma_memory_read32836:
  arg4 = t5 + ivory;
  t2 = (arg4 * 4);   
  t3 = LDQ_U(arg4);   
  t8 = t5 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t3 = (u8)(t3 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read32838;

vma_memory_read32837:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t3 & 63);   
  arg4 = arg4 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (arg5 & 1)   
    goto vma_memory_read32840;

vma_memory_read32845:
  /* Memory Read Internal */

vma_memory_read32846:
  arg6 = t2 + ivory;
  arg3 = (arg6 * 4);   
  t8 = LDQ_U(arg6);   
  arg4 = t2 - t11;   		// Stack cache offset 
  arg5 = ((u64)arg4 < (u64)t12) ? 1 : 0;   		// In range? 
  arg3 = *(s32 *)arg3;   
  t8 = (u8)(t8 >> ((arg6&7)*8));   
  if (arg5 != 0)   
    goto vma_memory_read32848;

vma_memory_read32847:
  arg4 = *(u64 *)&(processor->bindwrite_mask);   
  arg6 = zero + 224;   
  arg4 = arg4 >> (t8 & 63);   
  arg6 = arg6 >> (t8 & 63);   
  if (arg4 & 1)   
    goto vma_memory_read32850;

vma_memory_read32855:
  /* Merge cdr-code */
  arg3 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | arg3;
  arg4 = t2 + ivory;
  arg3 = (arg4 * 4);   
  arg6 = LDQ_U(arg4);   
  arg5 = (t8 & 0xff) << ((arg4&7)*8);   
  arg6 = arg6 & ~(0xffL << (arg4&7)*8);   

force_alignment32858:
  if (_trace) printf("force_alignment32858:\n");
  arg6 = arg6 | arg5;
  STQ_U(arg4, arg6);   
  arg4 = *(s32 *)&processor->scovlimit;   
  arg5 = t2 - t11;   		// Stack cache offset 
  arg4 = ((u64)arg5 < (u64)arg4) ? 1 : 0;   		// In range? 
  *(u32 *)arg3 = t6;   
  if (arg4 != 0)   		// J. if in cache 
    goto vma_memory_write32857;

vma_memory_write32856:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
  *(u32 *)&processor->bindingstackpointer = t1;   		// vma only 
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   

restorebsendloop:
  if (_trace) printf("restorebsendloop:\n");
  arg4 = (s32)t1 - (s32)arg1;   
  if ((s64)arg4 > 0)   
    goto restorebstoploop;
  t3 = *(s32 *)&processor->interruptreg;   
  t4 = t3 & 2;
  t4 = (t4 == 2) ? 1 : 0;   
  t3 = t3 | t4;
  *(u32 *)&processor->interruptreg = t3;   
  if (t3 == 0) 
    goto NEXTINSTRUCTION;
  *(u64 *)&processor->stop_interpreter = t3;   
  goto NEXTINSTRUCTION;   

restorebsiop:
  if (_trace) printf("restorebsiop:\n");
  arg5 = 0;
  arg2 = 66;
  goto illegaloperand;

vma_memory_write32857:
  if (_trace) printf("vma_memory_write32857:\n");
  arg4 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg5 * 8) + arg4;  		// reconstruct SCA 
  *(u32 *)arg4 = t6;   		// Store in stack 
  *(u32 *)(arg4 + 4) = t8;   		// write the stack cache 
  goto vma_memory_write32856;   

vma_memory_read32848:
  if (_trace) printf("vma_memory_read32848:\n");
  arg5 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg4 * 8) + arg5;  		// reconstruct SCA 
  arg3 = *(s32 *)arg4;   
  t8 = *(s32 *)(arg4 + 4);   		// Read from stack cache 
  goto vma_memory_read32847;   

vma_memory_read32850:
  if (_trace) printf("vma_memory_read32850:\n");
  if ((arg6 & 1) == 0)   
    goto vma_memory_read32849;
  t2 = (u32)arg3;   		// Do the indirect thing 
  goto vma_memory_read32846;   

vma_memory_read32849:
  if (_trace) printf("vma_memory_read32849:\n");
  arg4 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  arg6 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  arg6 = (arg6 * 4) + arg4;   		// Adjust for a longword load 
  arg4 = *(s32 *)arg6;   		// Get the memory action 

vma_memory_read32852:
  /* Perform memory action */
  arg1 = arg4;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read32838:
  if (_trace) printf("vma_memory_read32838:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read32837;   

vma_memory_read32840:
  if (_trace) printf("vma_memory_read32840:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read32839;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read32836;   

vma_memory_read32839:
  if (_trace) printf("vma_memory_read32839:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read32842:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read32828:
  if (_trace) printf("vma_memory_read32828:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read32827;   

vma_memory_read32830:
  if (_trace) printf("vma_memory_read32830:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read32829;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read32826;   

vma_memory_read32829:
  if (_trace) printf("vma_memory_read32829:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read32832:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

/* end DoRestoreBindingStack */
  /* End of Halfword operand from stack instruction - DoRestoreBindingStack */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunbind.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunfull.as
 ************************************************************************/

  /* The full word instructions */
/* start DoIStageError */

  /*  */
  /*  */
  /* Fullword instruction - DoIStageError */
  /* ======================= */

doistageerror:
  if (_trace) printf("doistageerror:\n");
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

/* end DoIStageError */
  /* End of Fullword instruction - DoIStageError */
  /* ============================== */
  /*  */
/* start nullfw */

  /*  */
  /*  */
  /* Fullword instruction - nullfw */
  /* ======================= */

nullfw:
  if (_trace) printf("nullfw:\n");
  arg5 = 0;
  arg2 = 47;
  goto illegaloperand;

/* end nullfw */
  /* End of Fullword instruction - nullfw */
  /* ============================== */
  /*  */
/* start monitorforwardfw */

  /*  */
  /*  */
  /* Fullword instruction - monitorforwardfw */
  /* ======================= */

monitorforwardfw:
  if (_trace) printf("monitorforwardfw:\n");
  arg5 = 0;
  arg2 = 46;
  goto illegaloperand;

/* end monitorforwardfw */
  /* End of Fullword instruction - monitorforwardfw */
  /* ============================== */
  /*  */
/* start headerpfw */

  /*  */
  /*  */
  /* Fullword instruction - headerpfw */
  /* ======================= */

headerpfw:
  if (_trace) printf("headerpfw:\n");
  arg5 = 0;
  arg2 = 44;
  goto illegaloperand;

/* end headerpfw */
  /* End of Fullword instruction - headerpfw */
  /* ============================== */
  /*  */
/* start headerifw */

  /*  */
  /*  */
  /* Fullword instruction - headerifw */
  /* ======================= */

headerifw:
  if (_trace) printf("headerifw:\n");
  arg5 = 0;
  arg2 = 43;
  goto illegaloperand;

/* end headerifw */
  /* End of Fullword instruction - headerifw */
  /* ============================== */
  /*  */
/* start oneqforwardfw */

  /*  */
  /*  */
  /* Fullword instruction - oneqforwardfw */
  /* ======================= */

oneqforwardfw:
  if (_trace) printf("oneqforwardfw:\n");
  arg5 = 0;
  arg2 = 48;
  goto illegaloperand;

/* end oneqforwardfw */
  /* End of Fullword instruction - oneqforwardfw */
  /* ============================== */
  /*  */
/* start headerforwardfw */

  /*  */
  /*  */
  /* Fullword instruction - headerforwardfw */
  /* ======================= */

headerforwardfw:
  if (_trace) printf("headerforwardfw:\n");
  arg5 = 0;
  arg2 = 42;
  goto illegaloperand;

/* end headerforwardfw */
  /* End of Fullword instruction - headerforwardfw */
  /* ============================== */
  /*  */
/* start elementforwardfw */

  /*  */
  /*  */
  /* Fullword instruction - elementforwardfw */
  /* ======================= */

elementforwardfw:
  if (_trace) printf("elementforwardfw:\n");
  arg5 = 0;
  arg2 = 40;
  goto illegaloperand;

/* end elementforwardfw */
  /* End of Fullword instruction - elementforwardfw */
  /* ============================== */
  /*  */
/* start gcforwardfw */

  /*  */
  /*  */
  /* Fullword instruction - gcforwardfw */
  /* ======================= */

gcforwardfw:
  if (_trace) printf("gcforwardfw:\n");
  arg5 = 0;
  arg2 = 41;
  goto illegaloperand;

/* end gcforwardfw */
  /* End of Fullword instruction - gcforwardfw */
  /* ============================== */
  /*  */
/* start boundlocationfw */

  /*  */
  /*  */
  /* Fullword instruction - boundlocationfw */
  /* ======================= */

boundlocationfw:
  if (_trace) printf("boundlocationfw:\n");
  arg5 = 0;
  arg2 = 39;
  goto illegaloperand;

/* end boundlocationfw */
  /* End of Fullword instruction - boundlocationfw */
  /* ============================== */
  /*  */
/* start logicvariablefw */

  /*  */
  /*  */
  /* Fullword instruction - logicvariablefw */
  /* ======================= */

logicvariablefw:
  if (_trace) printf("logicvariablefw:\n");
  arg5 = 0;
  arg2 = 45;
  goto illegaloperand;

/* end logicvariablefw */
  /* End of Fullword instruction - logicvariablefw */
  /* ============================== */
  /*  */
/* start pushsparepointer3 */

  /*  */
  /*  */
  /* Fullword instruction - pushsparepointer3 */
  /* ======================= */

pushsparepointer3:
  if (_trace) printf("pushsparepointer3:\n");
  arg1 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// Get operand 
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

/* end pushsparepointer3 */
  /* End of Fullword instruction - pushsparepointer3 */
  /* ============================== */
  /*  */
/* start pushsparepointer4 */

  /*  */
  /*  */
  /* Fullword instruction - pushsparepointer4 */
  /* ======================= */

pushsparepointer4:
  if (_trace) printf("pushsparepointer4:\n");
  arg1 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// Get operand 
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

/* end pushsparepointer4 */
  /* End of Fullword instruction - pushsparepointer4 */
  /* ============================== */
  /*  */
/* start callcompiledodd */

  /*  */
  /*  */
  /* Fullword instruction - callcompiledodd */
  /* ======================= */

callcompiledodd:
  if (_trace) printf("callcompiledodd:\n");

callcompiledoddprefetch:
  if (_trace) printf("callcompiledoddprefetch:\n");
  arg6 = arg3;		// Get operand 
  arg5 = Type_OddPC;
  arg3 = zero;		// No extra arg 
  goto startcallcompiledmerge;   

/* end callcompiledodd */
  /* End of Fullword instruction - callcompiledodd */
  /* ============================== */
  /*  */
/* start nativeinstruction */

  /*  */
  /*  */
  /* Fullword instruction - nativeinstruction */
  /* ======================= */

nativeinstruction:
  if (_trace) printf("nativeinstruction:\n");
  arg1 = iPC & ~1L;		// arg1 is instruction address*2 here 
  arg1 = arg1 + arg1;		// Select the DATA address 
  arg1 = (ivory * 4) + arg1;   		// Add in the memory base 
    r0 = (*( u64 (*)(u64, u64) )arg1)(arg1, arg2); /* jsr */  		// Jump into the Ivory code 

/* end nativeinstruction */
  /* End of Fullword instruction - nativeinstruction */
  /* ============================== */
  /*  */
/* start resumeemulated */


resumeemulated:
  if (_trace) printf("resumeemulated:\n");
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->annotation);   
  iPC = (ivory * 4) - arg1;   
  iPC = zero - iPC;   
  iPC = iPC >> 1;   
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

/* end resumeemulated */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunfull.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunbnum.as
 ************************************************************************/

  /* Bignums. */
/* start DoAddBignumStep */

  /* Halfword operand from stack instruction - DoAddBignumStep */
  /* arg2 has the preloaded 8 bit operand. */

doaddbignumstep:
  if (_trace) printf("doaddbignumstep:\n");

DoAddBignumStepIM:
  if (_trace) printf("DoAddBignumStepIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindoaddbignumstep;   

DoAddBignumStepSP:
  if (_trace) printf("DoAddBignumStepSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoAddBignumStepLP:
  if (_trace) printf("DoAddBignumStepLP:\n");

DoAddBignumStepFP:
  if (_trace) printf("DoAddBignumStepFP:\n");

headdoaddbignumstep:
  if (_trace) printf("headdoaddbignumstep:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindoaddbignumstep:
  if (_trace) printf("begindoaddbignumstep:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = *(s32 *)iSP;   		// Get arg2 
  t2 = *(s32 *)(iSP + 4);   		// and its tag 
  t3 = arg1 >> 32;   
  arg1 = (u32)arg1;   		// Strip type from arg3 
  t4 = t3 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto addbignumsteplose;
  arg3 = *(s32 *)(iSP + -8);   		// Get arg1 
  t1 = *(s32 *)(iSP + -4);   		// and its tag 
  arg2 = (u32)arg2;   		// Clear sign extension from arg2 
  t4 = t2 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto addbignumsteplose;
  arg3 = (u32)arg3;   		// Clear sign extension 
  t4 = t1 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto addbignumsteplose;
  arg4 = arg1 + arg2;
  arg5 = arg3 + arg4;
  arg6 = arg5 >> 32;   		// Shift the carry into arg6 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + -8) = arg5;   		// Store fixnum result 
  *(u32 *)(iSP + -4) = t1;   		// write the stack cache 
  *(u32 *)iSP = arg6;   		// Store the carry if any 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

addbignumsteplose:
  if (_trace) printf("addbignumsteplose:\n");
  arg5 = 0;
  arg2 = 76;
  goto illegaloperand;

/* end DoAddBignumStep */
  /* End of Halfword operand from stack instruction - DoAddBignumStep */
/* start DoSubBignumStep */

  /* Halfword operand from stack instruction - DoSubBignumStep */
  /* arg2 has the preloaded 8 bit operand. */

dosubbignumstep:
  if (_trace) printf("dosubbignumstep:\n");

DoSubBignumStepIM:
  if (_trace) printf("DoSubBignumStepIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindosubbignumstep;   

DoSubBignumStepSP:
  if (_trace) printf("DoSubBignumStepSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoSubBignumStepLP:
  if (_trace) printf("DoSubBignumStepLP:\n");

DoSubBignumStepFP:
  if (_trace) printf("DoSubBignumStepFP:\n");

headdosubbignumstep:
  if (_trace) printf("headdosubbignumstep:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindosubbignumstep:
  if (_trace) printf("begindosubbignumstep:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = *(s32 *)iSP;   		// Get arg2 
  t2 = *(s32 *)(iSP + 4);   		// and its tag 
  t3 = arg1 >> 32;   
  arg1 = (u32)arg1;   		// Strip type from arg3 
  t4 = t3 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto subbignumsteplose;
  arg3 = *(s32 *)(iSP + -8);   		// Get arg1 
  t1 = *(s32 *)(iSP + -4);   		// and its tag 
  arg2 = (u32)arg2;   		// Clear sign extension from arg2 
  t4 = t2 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto subbignumsteplose;
  arg3 = (u32)arg3;   		// Clear sign extension 
  t4 = t1 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto subbignumsteplose;
  arg4 = arg3 - arg2;   		// arg1-arg2 
  arg6 = ((s64)arg4 < (s64)zero) ? 1 : 0;   		// arg6=1 if we borrowed in 1st step 
  arg4 = (u32)arg4;   		// Truncate 1st step to 32-bits 
  arg5 = arg4 - arg1;   		// (arg1-arg2)-arg3 
  t6 = ((s64)arg5 < (s64)zero) ? 1 : 0;   		// t6=1 if we borrowed in 2nd step 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + -8) = arg5;   		// Store fixnum result 
  *(u32 *)(iSP + -4) = t1;   		// write the stack cache 
  arg6 = arg6 + t6;		// Compute borrow 
  *(u32 *)iSP = arg6;   		// Store the borrow if any 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

subbignumsteplose:
  if (_trace) printf("subbignumsteplose:\n");
  arg5 = 0;
  arg2 = 76;
  goto illegaloperand;

/* end DoSubBignumStep */
  /* End of Halfword operand from stack instruction - DoSubBignumStep */
/* start DoMultiplyBignumStep */

  /* Halfword operand from stack instruction - DoMultiplyBignumStep */
  /* arg2 has the preloaded 8 bit operand. */

domultiplybignumstep:
  if (_trace) printf("domultiplybignumstep:\n");

DoMultiplyBignumStepIM:
  if (_trace) printf("DoMultiplyBignumStepIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindomultiplybignumstep;   

DoMultiplyBignumStepSP:
  if (_trace) printf("DoMultiplyBignumStepSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoMultiplyBignumStepLP:
  if (_trace) printf("DoMultiplyBignumStepLP:\n");

DoMultiplyBignumStepFP:
  if (_trace) printf("DoMultiplyBignumStepFP:\n");

headdomultiplybignumstep:
  if (_trace) printf("headdomultiplybignumstep:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindomultiplybignumstep:
  if (_trace) printf("begindomultiplybignumstep:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = *(s32 *)iSP;   		// Get arg1 
  t1 = *(s32 *)(iSP + 4);   
  t2 = arg1 >> 32;   
  arg1 = (u32)arg1;   		// Strip type from arg2 
  t4 = t2 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto multbignumsteplose;
  arg2 = (u32)arg2;   
  t4 = t1 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto multbignumsteplose;
  arg3 = arg2 * arg1;   		// arg1*arg2 
  arg6 = (u32)(arg3 >> ((4&7)*8));   		// arg6=high order word 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)iSP = arg3;   		// Store fixnum result ls word 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  *(u32 *)(iSP + 8) = arg6;   		// Store ms word 
  *(u32 *)(iSP + 12) = t1;   		// write the stack cache 
  iSP = iSP + 8;
  goto cachevalid;   

multbignumsteplose:
  if (_trace) printf("multbignumsteplose:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

/* end DoMultiplyBignumStep */
  /* End of Halfword operand from stack instruction - DoMultiplyBignumStep */
/* start DoDivideBignumStep */

  /* Halfword operand from stack instruction - DoDivideBignumStep */
  /* arg2 has the preloaded 8 bit operand. */

dodividebignumstep:
  if (_trace) printf("dodividebignumstep:\n");

DoDivideBignumStepIM:
  if (_trace) printf("DoDivideBignumStepIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindodividebignumstep;   

DoDivideBignumStepSP:
  if (_trace) printf("DoDivideBignumStepSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoDivideBignumStepLP:
  if (_trace) printf("DoDivideBignumStepLP:\n");

DoDivideBignumStepFP:
  if (_trace) printf("DoDivideBignumStepFP:\n");

headdodividebignumstep:
  if (_trace) printf("headdodividebignumstep:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindodividebignumstep:
  if (_trace) printf("begindodividebignumstep:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg2 = *(s32 *)iSP;   		// Get arg2 
  t1 = *(s32 *)(iSP + 4);   
  t2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t4 = t2 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto divbignumsteplose1;
  if (arg1 == 0) 		// J. if division by zero 
    goto divbignumsteplose2;
  arg2 = (u32)arg2;   
  arg3 = *(s32 *)(iSP + -8);   		// Get arg1 
  t3 = *(s32 *)(iSP + -4);   
  t4 = t1 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto divbignumsteplose1;
  arg2 = arg2 << 32;   		// arg2=(ash arg2 32) 
  arg3 = (u32)arg3;   
  t4 = t3 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto divbignumsteplose1;
  arg4 = arg3 | arg2;		// arg1+(ash arg2 32) 
  t1 = arg4 / arg1;   		// t1 is now the quotient 
  t2 = t1 * arg1;   
  t2 = arg4 - t2;   		// t2 is now the remainder 
  *(u32 *)(iSP + -8) = t1;   		// store quotient (already fixnum) 
  *(u32 *)iSP = t2;   		// store remainder (already fixnum) 
  goto NEXTINSTRUCTION;   

divbignumsteplose1:
  if (_trace) printf("divbignumsteplose1:\n");
  arg5 = 0;
  arg2 = 76;
  goto illegaloperand;

divbignumsteplose2:
  if (_trace) printf("divbignumsteplose2:\n");
  arg5 = 0;
  arg2 = 2;
  goto illegaloperand;

/* end DoDivideBignumStep */
  /* End of Halfword operand from stack instruction - DoDivideBignumStep */
/* start DoLshcBignumStep */

  /* Halfword operand from stack instruction - DoLshcBignumStep */

dolshcbignumstep:
  if (_trace) printf("dolshcbignumstep:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoLshcBignumStepIM:
  if (_trace) printf("DoLshcBignumStepIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment32859:
  if (_trace) printf("force_alignment32859:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindolshcbignumstep;   

DoLshcBignumStepSP:
  if (_trace) printf("DoLshcBignumStepSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoLshcBignumStepLP:
  if (_trace) printf("DoLshcBignumStepLP:\n");

DoLshcBignumStepFP:
  if (_trace) printf("DoLshcBignumStepFP:\n");

headdolshcbignumstep:
  if (_trace) printf("headdolshcbignumstep:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindolshcbignumstep:
  if (_trace) printf("begindolshcbignumstep:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = *(s32 *)iSP;   		// Get arg2 
  t2 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack 
  t3 = arg1 >> 32;   
  arg1 = (u32)arg1;   		// Strip type from arg3 
  t4 = t3 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto lshcbignumsteplose;
  arg2 = (u32)arg2;   
  arg3 = *(s32 *)iSP;   		// Get arg1 
  t1 = *(s32 *)(iSP + 4);   
  t4 = t2 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto lshcbignumsteplose;
  arg2 = arg2 << 32;   		// arg2=(ash arg2 32) 
  arg3 = (u32)arg3;   
  t4 = t1 - Type_Fixnum;   
  t4 = t4 & 63;		// Strip CDR code 
  if (t4 != 0)   
    goto lshcbignumsteplose;
  arg4 = arg3 | arg2;		// arg1+(ash arg2 32) 
  arg5 = arg4 << (arg1 & 63);   
  arg6 = (s64)arg5 >> 32;   		// Extract the result 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)iSP = arg6;   		// Store the result as a fixnum 
  *(u32 *)(iSP + 4) = t1;   		// write the stack cache 
  goto cachevalid;   

lshcbignumsteplose:
  if (_trace) printf("lshcbignumsteplose:\n");
  arg5 = 0;
  arg2 = 76;
  goto illegaloperand;

/* end DoLshcBignumStep */
  /* End of Halfword operand from stack instruction - DoLshcBignumStep */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunbnum.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifuntrap.as
 ************************************************************************/

/* start DECODEFAULT */


decodefault:
  if (_trace) printf("decodefault:\n");
  /* We come here when a memory access faults to figure out why */
  t1 = *(u64 *)&(processor->vma);   		// retrieve the trapping VMA 
  t3 = *(u64 *)&(processor->vmattributetable);   		// Per-page attributes table 
  t2 = t1 >> (MemoryPage_AddressShift & 63);   		// Index into the attributes table 
  t3 = t2 + t3;		// Address of the page's attributes 
  t2 = LDQ_U(t3);   		// Get the quadword with the page's attributes 
  *(u64 *)&processor->vma = t1;   		// Stash the VMA 
  t2 = (u8)(t2 >> ((t3&7)*8));   		// Extract the page's attributes 
  if (t2 == 0) 		// Non-existent page 
    goto pagenotresident;
  t3 = t2 & VMAttribute_AccessFault;
  if (t3 != 0)   		// Access fault 
    goto pagefaultrequesthandler;
  t3 = t2 & VMAttribute_TransportFault;
  if (t3 != 0)   		// Transport fault 
    goto transporttrap;
  t3 = t2 & VMAttribute_WriteFault;
  if (t3 != 0)   		// Write fault 
    goto pagewritefault;
  goto buserror;

/* end DECODEFAULT */
/* start HANDLEUNWINDPROTECT */


handleunwindprotect:
  if (_trace) printf("handleunwindprotect:\n");
  t4 = *(s32 *)&processor->catchblock;   
  t4 = (u32)t4;   
  /* Convert VMA to stack cache address */
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = t4 - t2;   		// stack cache base relative offset 
  t3 = (t2 * 8) + t3;  		// reconstruct SCA 
  t6 = *(s32 *)(t3 + 16);   
  t5 = *(s32 *)(t3 + 20);   
  t6 = (u32)t6;   
  t2 = *(s32 *)(t3 + 8);   
  t1 = *(s32 *)(t3 + 12);   
  t2 = (u32)t2;   
  iSP = *(u64 *)&(processor->restartsp);   		// Restore SP 
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t3 = (s32)t1 - (s32)t2;   
  if (t3 == 0) 		// J. if binding level= binding stack 
    goto do_unwind_protect32860;

do_unwind_protect32861:
  if (_trace) printf("do_unwind_protect32861:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  arg1 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & arg1;
  t4 = t4 & ~arg1;		// Turn off the bit 
  if (t3 != 0)   
    goto g32862;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g32862:
  if (_trace) printf("g32862:\n");
  /* Memory Read Internal */

vma_memory_read32863:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t1 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t6 = (t10 * 4);   
  t7 = LDQ_U(t10);   
  t8 = t1 - t8;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->bindread_mask);   
  t9 = ((u64)t8 < (u64)t9) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((t10&7)*8));   
  if (t9 != 0)   
    goto vma_memory_read32865;

vma_memory_read32864:
  t10 = zero + 224;   
  t11 = t11 >> (t7 & 63);   
  t10 = t10 >> (t7 & 63);   
  if (t11 & 1)   
    goto vma_memory_read32867;

vma_memory_read32872:
  /* Memory Read Internal */

vma_memory_read32873:
  t8 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t10 = t5 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  arg1 = (t10 * 4);   
  t3 = LDQ_U(t10);   
  t8 = t5 - t8;   		// Stack cache offset 
  t11 = *(u64 *)&(processor->bindread_mask);   
  t9 = ((u64)t8 < (u64)t9) ? 1 : 0;   		// In range? 
  arg1 = *(s32 *)arg1;   
  t3 = (u8)(t3 >> ((t10&7)*8));   
  if (t9 != 0)   
    goto vma_memory_read32875;

vma_memory_read32874:
  t10 = zero + 224;   
  t11 = t11 >> (t3 & 63);   
  t10 = t10 >> (t3 & 63);   
  arg1 = (u32)arg1;   
  if (t11 & 1)   
    goto vma_memory_read32877;

vma_memory_read32882:
  /* Memory Read Internal */

vma_memory_read32883:
  t10 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t12 = arg1 + ivory;
  t11 = *(s32 *)&processor->scovlimit;   
  t9 = (t12 * 4);   
  t8 = LDQ_U(t12);   
  t10 = arg1 - t10;   		// Stack cache offset 
  t11 = ((u64)t10 < (u64)t11) ? 1 : 0;   		// In range? 
  t9 = *(s32 *)t9;   
  t8 = (u8)(t8 >> ((t12&7)*8));   
  if (t11 != 0)   
    goto vma_memory_read32885;

vma_memory_read32884:
  t10 = *(u64 *)&(processor->bindwrite_mask);   
  t12 = zero + 224;   
  t10 = t10 >> (t8 & 63);   
  t12 = t12 >> (t8 & 63);   
  if (t10 & 1)   
    goto vma_memory_read32887;

vma_memory_read32892:
  /* Merge cdr-code */
  t9 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | t9;
  t10 = arg1 + ivory;
  t9 = (t10 * 4);   
  t12 = LDQ_U(t10);   
  t11 = (t8 & 0xff) << ((t10&7)*8);   
  t12 = t12 & ~(0xffL << (t10&7)*8);   

force_alignment32895:
  if (_trace) printf("force_alignment32895:\n");
  t12 = t12 | t11;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  STQ_U(t10, t12);   
  t10 = *(s32 *)&processor->scovlimit;   
  t11 = arg1 - t11;   		// Stack cache offset 
  t10 = ((u64)t11 < (u64)t10) ? 1 : 0;   		// In range? 
  *(u32 *)t9 = t6;   
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write32894;

vma_memory_write32893:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
  *(u32 *)&processor->bindingstackpointer = t1;   		// vma only 
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;   
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t3 = (s32)t1 - (s32)t2;   
  if (t3 != 0)   		// J. if binding level/= binding stack 
    goto do_unwind_protect32861;
  t2 = *(s32 *)&processor->interruptreg;   
  t3 = t2 & 2;
  t3 = (t3 == 2) ? 1 : 0;   
  t2 = t2 | t3;
  *(u32 *)&processor->interruptreg = t2;   
  if (t2 == 0) 
    goto do_unwind_protect32860;
  *(u64 *)&processor->stop_interpreter = t2;   

do_unwind_protect32860:
  if (_trace) printf("do_unwind_protect32860:\n");
  /* Convert PC to a real continuation. */
  t3 = iPC & 1;
  t1 = iPC >> 1;   		// convert PC to a real word address. 
  t3 = t3 + Type_EvenPC;   
  arg1 = *(s32 *)&processor->control;   
  t2 = arg1 >> 17;   
  t2 = t2 | 128;
  t2 = t2 & 192;
  /* TagType. */
  t3 = t3 & 63;
  t3 = t3 | t2;
  *(u32 *)(iSP + 8) = t1;   
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  iSP = iSP + 8;
  /* Load catch-block PC */
  t4 = *(s32 *)&processor->catchblock;   
  t4 = (u32)t4;   
  /* Convert VMA to stack cache address */
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = t4 - t2;   		// stack cache base relative offset 
  t3 = (t2 * 8) + t3;  		// reconstruct SCA 
  t6 = *(s32 *)t3;   
  t5 = *(s32 *)(t3 + 4);   
  t6 = (u32)t6;   
  /* Convert real continuation to PC. */
  iPC = t5 & 1;
  iPC = t6 + iPC;
  iPC = t6 + iPC;
  t1 = (128) << 16;   
  arg1 = arg1 | t1;
  t10 = *(s32 *)(t3 + 16);   
  t5 = *(s32 *)(t3 + 20);   
  t10 = (u32)t10;   
  t6 = t5 & 128;		// This is the  extra-arg bit 
  t8 = *(s32 *)&processor->extraandcatch;   
  t7 = t5 & 64;		// This is the  cleanup-catch bit 
  t6 = t6 << 1;   		// Shift bit into place for cr 
  t7 = t7 << 20;   		// Shift extra arg bit into place for cr 
  arg1 = arg1 & ~t8;
  t6 = t6 | t7;
  arg1 = arg1 | t6;		// update the bits extra-arg/cleanupcatch 
  *(u32 *)&processor->control = arg1;   
  /* TagType. */
  t5 = t5 & 63;
  t5 = t5 << 32;   
  t5 = t5 | t10;
  *(u64 *)&processor->catchblock = t5;   
  goto interpretinstructionforbranch;   		// Execute cleanup 

vma_memory_write32894:
  if (_trace) printf("vma_memory_write32894:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t11 * 8) + t10;  		// reconstruct SCA 
  *(u32 *)t10 = t6;   		// Store in stack 
  *(u32 *)(t10 + 4) = t8;   		// write the stack cache 
  goto vma_memory_write32893;   

vma_memory_read32885:
  if (_trace) printf("vma_memory_read32885:\n");
  t11 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t10 * 8) + t11;  		// reconstruct SCA 
  t9 = *(s32 *)t10;   
  t8 = *(s32 *)(t10 + 4);   		// Read from stack cache 
  goto vma_memory_read32884;   

vma_memory_read32887:
  if (_trace) printf("vma_memory_read32887:\n");
  if ((t12 & 1) == 0)   
    goto vma_memory_read32886;
  arg1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read32883;   

vma_memory_read32886:
  if (_trace) printf("vma_memory_read32886:\n");
  t10 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t12 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t12 = (t12 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t12;   		// Get the memory action 

vma_memory_read32889:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read32875:
  if (_trace) printf("vma_memory_read32875:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  arg1 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read32874;   

vma_memory_read32877:
  if (_trace) printf("vma_memory_read32877:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read32876;
  t5 = (u32)arg1;   		// Do the indirect thing 
  goto vma_memory_read32873;   

vma_memory_read32876:
  if (_trace) printf("vma_memory_read32876:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read32879:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read32865:
  if (_trace) printf("vma_memory_read32865:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read32864;   

vma_memory_read32867:
  if (_trace) printf("vma_memory_read32867:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read32866;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read32863;   

vma_memory_read32866:
  if (_trace) printf("vma_memory_read32866:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read32869:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 2;
  goto performmemoryaction;

/* end HANDLEUNWINDPROTECT */
/* start PERFORMMEMORYACTION */


performmemoryaction:
  if (_trace) printf("performmemoryaction:\n");
  /* We get here when a memory action that will trap is detected. */
  /* ARG1 contains the memory action code with the Transport bit removed. */
  /* ARG2 contains the memory cycle so we can generate the proper microstate. */
  t1 = (arg1 == MemoryActionTrap) ? 1 : 0;   

force_alignment32924:
  if (_trace) printf("force_alignment32924:\n");
  if (t1 == 0) 
    goto basic_dispatch32897;
  /* Here if argument MemoryActionTrap */
  t1 = *(u64 *)&(processor->vma);   		// Get the failing VMA 
  t2 = (arg2 == Cycle_DataRead) ? 1 : 0;   

force_alignment32911:
  if (_trace) printf("force_alignment32911:\n");
  if (t2 == 0) 
    goto basic_dispatch32899;
  /* Here if argument CycleDataRead */
  arg5 = t1;
  arg2 = 57;
  goto illegaloperand;

basic_dispatch32899:
  if (_trace) printf("basic_dispatch32899:\n");
  t2 = (arg2 == Cycle_DataWrite) ? 1 : 0;   

force_alignment32912:
  if (_trace) printf("force_alignment32912:\n");
  if (t2 == 0) 
    goto basic_dispatch32900;
  /* Here if argument CycleDataWrite */
  arg5 = t1;
  arg2 = 58;
  goto illegaloperand;

basic_dispatch32900:
  if (_trace) printf("basic_dispatch32900:\n");
  t2 = (arg2 == Cycle_BindRead) ? 1 : 0;   

force_alignment32913:
  if (_trace) printf("force_alignment32913:\n");
  if (t2 != 0)   
    goto basic_dispatch32902;
  t2 = (arg2 == Cycle_BindReadNoMonitor) ? 1 : 0;   

force_alignment32914:
  if (_trace) printf("force_alignment32914:\n");
  if (t2 == 0) 
    goto basic_dispatch32901;

basic_dispatch32902:
  if (_trace) printf("basic_dispatch32902:\n");
  /* Here if argument (CycleBindRead CycleBindReadNoMonitor) */
  arg5 = t1;
  arg2 = 54;
  goto illegaloperand;

basic_dispatch32901:
  if (_trace) printf("basic_dispatch32901:\n");
  t2 = (arg2 == Cycle_BindWrite) ? 1 : 0;   

force_alignment32915:
  if (_trace) printf("force_alignment32915:\n");
  if (t2 != 0)   
    goto basic_dispatch32904;
  t2 = (arg2 == Cycle_BindWriteNoMonitor) ? 1 : 0;   

force_alignment32916:
  if (_trace) printf("force_alignment32916:\n");
  if (t2 == 0) 
    goto basic_dispatch32903;

basic_dispatch32904:
  if (_trace) printf("basic_dispatch32904:\n");
  /* Here if argument (CycleBindWrite CycleBindWriteNoMonitor) */
  arg5 = t1;
  arg2 = 55;
  goto illegaloperand;

basic_dispatch32903:
  if (_trace) printf("basic_dispatch32903:\n");
  t2 = (arg2 == Cycle_Header) ? 1 : 0;   

force_alignment32917:
  if (_trace) printf("force_alignment32917:\n");
  if (t2 != 0)   
    goto basic_dispatch32906;
  t2 = (arg2 == Cycle_StructureOffset) ? 1 : 0;   

force_alignment32918:
  if (_trace) printf("force_alignment32918:\n");
  if (t2 == 0) 
    goto basic_dispatch32905;

basic_dispatch32906:
  if (_trace) printf("basic_dispatch32906:\n");
  /* Here if argument (CycleHeader CycleStructureOffset) */
  arg5 = t1;
  arg2 = 59;
  goto illegaloperand;

basic_dispatch32905:
  if (_trace) printf("basic_dispatch32905:\n");
  t2 = (arg2 == Cycle_Scavenge) ? 1 : 0;   

force_alignment32919:
  if (_trace) printf("force_alignment32919:\n");
  if (t2 != 0)   
    goto basic_dispatch32908;
  t2 = (arg2 == Cycle_GCCopy) ? 1 : 0;   

force_alignment32920:
  if (_trace) printf("force_alignment32920:\n");
  if (t2 == 0) 
    goto basic_dispatch32907;

basic_dispatch32908:
  if (_trace) printf("basic_dispatch32908:\n");
  /* Here if argument (CycleScavenge CycleGCCopy) */
  arg5 = t1;
  arg2 = 60;
  goto illegaloperand;

basic_dispatch32907:
  if (_trace) printf("basic_dispatch32907:\n");
  t2 = (arg2 == Cycle_Cdr) ? 1 : 0;   

force_alignment32921:
  if (_trace) printf("force_alignment32921:\n");
  if (t2 == 0) 
    goto basic_dispatch32898;
  /* Here if argument CycleCdr */
  arg5 = t1;
  arg2 = 56;
  goto illegaloperand;

basic_dispatch32898:
  if (_trace) printf("basic_dispatch32898:\n");

basic_dispatch32897:
  if (_trace) printf("basic_dispatch32897:\n");
  t1 = (arg1 == MemoryActionMonitor) ? 1 : 0;   

force_alignment32925:
  if (_trace) printf("force_alignment32925:\n");
  if (t1 == 0) 
    goto basic_dispatch32896;
  /* Here if argument MemoryActionMonitor */
  goto monitortrap;

basic_dispatch32896:
  if (_trace) printf("basic_dispatch32896:\n");

/* end PERFORMMEMORYACTION */
/* start OutOfLineExceptions */


outoflineexceptions:
  if (_trace) printf("outoflineexceptions:\n");

ldbexception:
  if (_trace) printf("ldbexception:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;

rplacaexception:
  if (_trace) printf("rplacaexception:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

rplacdexception:
  if (_trace) printf("rplacdexception:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

pushivexception:
  if (_trace) printf("pushivexception:\n");
  t1 = zero + 8;   
  /* SetTag. */
  t1 = t1 << 32;   
  t1 = arg2 | t1;
  arg6 = t2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

incrementexception:
  if (_trace) printf("incrementexception:\n");
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto unarynumericexception;

decrementexception:
  if (_trace) printf("decrementexception:\n");
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto unarynumericexception;

/* end OutOfLineExceptions */
/* start NUMERICEXCEPTION */


numericexception:
  if (_trace) printf("numericexception:\n");
  t1 = arg6 - Type_Fixnum;   
  t1 = t1 & 56;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto notnumeric;
  goto exception;

notnumeric:
  if (_trace) printf("notnumeric:\n");
  arg5 = 0;
  arg2 = 16;
  goto illegaloperand;

/* end NUMERICEXCEPTION */
/* start UNARYNUMERICEXCEPTION */


unarynumericexception:
  if (_trace) printf("unarynumericexception:\n");
  t1 = arg6 - Type_Fixnum;   
  t1 = t1 & 56;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto unarynotnumeric;
  goto exception;

unarynotnumeric:
  if (_trace) printf("unarynotnumeric:\n");
  arg5 = 0;
  arg2 = 81;
  goto illegaloperand;

/* end UNARYNUMERICEXCEPTION */
/* start LISTEXCEPTION */


listexception:
  if (_trace) printf("listexception:\n");
  t1 = arg6 - Type_List;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto notlist1;
  goto exception;

notlist1:
  if (_trace) printf("notlist1:\n");
  t1 = arg6 - Type_ListInstance;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto notlist2;
  goto exception;

notlist2:
  if (_trace) printf("notlist2:\n");
  arg5 = 0;
  arg2 = 26;
  goto illegaloperand;

/* end LISTEXCEPTION */
/* start ARRAYEXCEPTION */


arrayexception:
  if (_trace) printf("arrayexception:\n");
  t1 = arg6 - Type_Array;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto notarray1;
  goto exception;

notarray1:
  if (_trace) printf("notarray1:\n");
  t1 = arg6 - Type_ArrayInstance;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto notarray2;
  goto exception;

notarray2:
  if (_trace) printf("notarray2:\n");
  goto spareexception;

/* end ARRAYEXCEPTION */
/* start SPAREEXCEPTION */


spareexception:
  if (_trace) printf("spareexception:\n");
  t1 = arg6 - Type_SparePointer1;   
  t1 = t1 & 62;		// Strip CDR code, low bits 
  if (t1 != 0)   
    goto notspare1;
  goto exception;

notspare1:
  if (_trace) printf("notspare1:\n");

notspare2:
  if (_trace) printf("notspare2:\n");
  t1 = arg6 - Type_SpareNumber;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto notspare3;
  goto exception;

notspare3:
  if (_trace) printf("notspare3:\n");
  goto illegaloperand;

/* end SPAREEXCEPTION */
/* start EXCEPTION */


exception:
  if (_trace) printf("exception:\n");
  if (arg4 != 0)   		// J. if arithmetic exception 
    goto arithmeticexception;
  t2 = *(u64 *)&(processor->linkage);   
  iSP = *(u64 *)&(processor->restartsp);   		// fix the stack pointer 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// fetch the real opcode 
  if (t2 != 0)   
    goto nativeexception;
  if (arg3 != 0)   		// J. if arguments stacked 
    goto exception_handler32927;
  t1 = (u16)(arg2 >> ((4&7)*8));   		// Get original operand 
  t3 = (t1 == 512) ? 1 : 0;   		// t3 is non-zero iff SP|POP operand 
  if (t3 != 0)   		// SP|POP operand recovered by restoring SP 
    goto exception_handler32927;
  arg5 = iFP;   		// Assume FP mode 
  t3 = iSP + -2040;   		// SP mode constant 
  t4 = (u8)(arg2 >> ((5&7)*8));   		// Get the mode bits 
  t2 = (u8)(arg2 >> ((4&7)*8));   		// Extract (8-bit, unsigned) operand 
  t4 = t4 - 2;   		// t4 = -2 FP, -1 LP, 0 SP, 1 Imm 
  if (t4 & 1)   		// LP or Immediate mode 
   arg5 = iLP;
  if (t4 == 0)   		// SP mode 
    arg5 = t3;
  arg5 = (t2 * 8) + arg5;  		// Compute operand address 
  if ((s64)t4 <= 0)  		// Not immediate mode 
    goto exception_handler32928;
  t1 = t2 << 56;   
  t3 = arg2 >> 16;   
  t1 = (s64)t1 >> 56;   
  arg5 = (u64)&processor->immediate_arg;   		// Immediate mode constant 
  if ((t3 & 1) == 0)   		// Signed immediate 
   t2 = t1;
  *(u32 *)&processor->immediate_arg = t2;   

exception_handler32928:
  if (_trace) printf("exception_handler32928:\n");
  t1 = zero + -32768;   
  t1 = t1 + ((2) << 16);   
  t2 = arg2 & t1;
  t3 = (t1 == t2) ? 1 : 0;   
  if (t3 == 0) 		// J. if not address-format operand 
    goto exception_handler32929;
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = arg5 - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  t2 = Type_Locative;
  /* SetTag. */
  arg5 = t2 << 32;   
  arg5 = t1 | arg5;
  goto exception_handler32930;   

exception_handler32929:
  if (_trace) printf("exception_handler32929:\n");
  arg5 = *(u64 *)arg5;   		// Fetch the arg 

exception_handler32930:
  if (_trace) printf("exception_handler32930:\n");
  *(u64 *)(iSP + 8) = arg5;   
  iSP = iSP + 8;

exception_handler32927:
  if (_trace) printf("exception_handler32927:\n");
  arg2 = arg2 >> 10;   		// Shift opcode into position 
  arg2 = arg2 & 255;		// Just 8-bits of opcode 
  t11 = arg2 + TrapVector_InstructionException;   
  t12 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  goto handleexception;   

/* end EXCEPTION */
/* start ARITHMETICEXCEPTION */


arithmeticexception:
  if (_trace) printf("arithmeticexception:\n");
  t2 = *(u64 *)&(processor->linkage);   
  iSP = *(u64 *)&(processor->restartsp);   		// fix the stack pointer 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// fetch the real opcode 
  if (t2 != 0)   
    goto nativeexception;
  t1 = (u16)(arg2 >> ((4&7)*8));   		// Get original operand 
  t3 = (t1 == 512) ? 1 : 0;   		// t3 is non-zero iff SP|POP operand 
  if (t3 != 0)   		// SP|POP operand recovered by restoring SP 
    goto exception_handler32932;
  arg5 = iFP;   		// Assume FP mode 
  t3 = iSP + -2040;   		// SP mode constant 
  t4 = (u8)(arg2 >> ((5&7)*8));   		// Get the mode bits 
  t2 = (u8)(arg2 >> ((4&7)*8));   		// Extract (8-bit, unsigned) operand 
  t4 = t4 - 2;   		// t4 = -2 FP, -1 LP, 0 SP, 1 Imm 
  if (t4 & 1)   		// LP or Immediate mode 
   arg5 = iLP;
  if (t4 == 0)   		// SP mode 
    arg5 = t3;
  arg5 = (t2 * 8) + arg5;  		// Compute operand address 
  if ((s64)t4 <= 0)  		// Not immediate mode 
    goto exception_handler32933;
  t1 = t2 << 56;   
  t3 = arg2 >> 16;   
  t1 = (s64)t1 >> 56;   
  arg5 = (u64)&processor->immediate_arg;   		// Immediate mode constant 
  if ((t3 & 1) == 0)   		// Signed immediate 
   t2 = t1;
  *(u32 *)&processor->immediate_arg = t2;   

exception_handler32933:
  if (_trace) printf("exception_handler32933:\n");
  t1 = zero + -32768;   
  t1 = t1 + ((2) << 16);   
  t2 = arg2 & t1;
  t3 = (t1 == t2) ? 1 : 0;   
  if (t3 == 0) 		// J. if not address-format operand 
    goto exception_handler32934;
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = arg5 - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  t2 = Type_Locative;
  /* SetTag. */
  arg5 = t2 << 32;   
  arg5 = t1 | arg5;
  goto exception_handler32935;   

exception_handler32934:
  if (_trace) printf("exception_handler32934:\n");
  arg5 = *(u64 *)arg5;   		// Fetch the arg 

exception_handler32935:
  if (_trace) printf("exception_handler32935:\n");
  *(u64 *)(iSP + 8) = arg5;   
  iSP = iSP + 8;

exception_handler32932:
  if (_trace) printf("exception_handler32932:\n");
  t4 = arg2 >> 17;   		// Get unary/nary bit of opcode 
  arg1 = 1;		// Assume unary 
  t11 = zero;
  t2 = iSP;
  if ((t4 & 1) == 0)   		// J. if not binary arithmetic dispatch 
    goto exception_handler32931;
  arg1 = 2;		// Nary -> Binary 
  t11 = *(s32 *)(iSP + 4);   
  t2 = t2 - 8;   
  t11 = t11 & 7;		// low three bits has opcode tag for op2 

exception_handler32931:
  if (_trace) printf("exception_handler32931:\n");
  arg2 = arg2 >> 4;   		// Shift opcode into position 
  t2 = *(s32 *)(t2 + 4);   
  arg2 = arg2 & 1984;		// five bits from the opcode 
  t2 = t2 & 7;
  t11 = (t2 * 8) + t11;  
  t11 = arg2 | t11;
  t11 = t11 + TrapVector_ArithmeticInstructionException;   
  t12 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  goto handleexception;   

/* end ARITHMETICEXCEPTION */
/* start LOOPEXCEPTION */


loopexception:
  if (_trace) printf("loopexception:\n");
  t2 = *(u64 *)&(processor->linkage);   
  iSP = *(u64 *)&(processor->restartsp);   		// fix the stack pointer 
  arg2 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   		// fetch the real opcode 
  if (t2 != 0)   
    goto nativeexception;
  if (arg3 != 0)   		// J. if arguments stacked 
    goto exception_handler32937;
  t1 = (u16)(arg2 >> ((4&7)*8));   		// Get original operand 
  t3 = (t1 == 512) ? 1 : 0;   		// t3 is non-zero iff SP|POP operand 
  if (t3 != 0)   		// SP|POP operand recovered by restoring SP 
    goto exception_handler32937;
  arg5 = iFP;   		// Assume FP mode 
  t3 = iSP + -2040;   		// SP mode constant 
  t4 = (u8)(arg2 >> ((5&7)*8));   		// Get the mode bits 
  t2 = (u8)(arg2 >> ((4&7)*8));   		// Extract (8-bit, unsigned) operand 
  t4 = t4 - 2;   		// t4 = -2 FP, -1 LP, 0 SP, 1 Imm 
  if (t4 & 1)   		// LP or Immediate mode 
   arg5 = iLP;
  if (t4 == 0)   		// SP mode 
    arg5 = t3;
  arg5 = (t2 * 8) + arg5;  		// Compute operand address 
  if ((s64)t4 <= 0)  		// Not immediate mode 
    goto exception_handler32938;
  t1 = t2 << 56;   
  t3 = arg2 >> 16;   
  t1 = (s64)t1 >> 56;   
  arg5 = (u64)&processor->immediate_arg;   		// Immediate mode constant 
  if ((t3 & 1) == 0)   		// Signed immediate 
   t2 = t1;
  *(u32 *)&processor->immediate_arg = t2;   

exception_handler32938:
  if (_trace) printf("exception_handler32938:\n");
  t1 = zero + -32768;   
  t1 = t1 + ((2) << 16);   
  t2 = arg2 & t1;
  t3 = (t1 == t2) ? 1 : 0;   
  if (t3 == 0) 		// J. if not address-format operand 
    goto exception_handler32939;
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = arg5 - t2;   		// stack cache base relative offset 
  t2 = t2 >> 3;   		// convert byte address to word address 
  t1 = t2 + t1;		// reconstruct VMA 
  t2 = Type_Locative;
  /* SetTag. */
  arg5 = t2 << 32;   
  arg5 = t1 | arg5;
  goto exception_handler32940;   

exception_handler32939:
  if (_trace) printf("exception_handler32939:\n");
  arg5 = *(u64 *)arg5;   		// Fetch the arg 

exception_handler32940:
  if (_trace) printf("exception_handler32940:\n");
  *(u64 *)(iSP + 8) = arg5;   
  iSP = iSP + 8;

exception_handler32937:
  if (_trace) printf("exception_handler32937:\n");
  arg2 = arg2 >> 10;   		// Shift opcode into position 
  arg2 = arg2 & 255;		// Just 8-bits of opcode 
  t11 = arg2 + TrapVector_InstructionException;   
  t12 = arg5;
  goto handleexception;   

/* end LOOPEXCEPTION */
/* start HandleException */


handleexception:
  if (_trace) printf("handleexception:\n");
  t1 = iFP;		// save old frame pointer 
  t4 = *(s32 *)&processor->control;   
  t9 = *(u64 *)&(processor->fepmodetrapvecaddress);   
  t8 = *(u64 *)&(processor->trapvecbase);   
  t5 = (-16384) << 16;   
  t6 = t4 >> 30;   
  t5 = t4 | t5;		// Set trap mode to 3 
  t6 = t6 & 3;
  *(u32 *)&processor->control = t5;   
  t7 = t6 - 3;   
  t8 = t8 + t11;
  if (t7 == 0)   
    t8 = t9;
  *(u64 *)&processor->tvi = t8;   		// Record TVI for tracing (if enabled) 
  /* Memory Read Internal */

vma_memory_read32944:
  t9 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t6 = t8 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t3 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t9 = t8 - t9;   		// Stack cache offset 
  t5 = *(u64 *)&(processor->dataread_mask);   
  t7 = ((u64)t9 < (u64)t7) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32946;

vma_memory_read32945:
  t6 = zero + 240;   
  t5 = t5 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t5 & 1)   
    goto vma_memory_read32948;

vma_memory_read32955:
  t5 = t2 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto get_trap_vector_entry32943;
  *(u32 *)&processor->control = t4;   		// Restore the cr 
  t8 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t5 = zero + 128;   
  t6 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t5 = t5 + 8;		// Account for what we're about to push 
  t5 = (t5 * 8) + iSP;  		// SCA of desired end of cache 
  t6 = (t8 * 8) + t6;  		// SCA of current end of cache 
  t8 = ((s64)t5 <= (s64)t6) ? 1 : 0;   
  if (t8 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check32956;
  iFP = (arg1 * 8) + zero;  
  iFP = iSP - iFP;   
  iFP = iFP + 8;
  if (arg1 == 0) 
    goto take_post_trap32941;
  t5 = *(u64 *)iSP;   
  *(u64 *)(iSP + 32) = t5;   
  arg1 = arg1 - 1;   
  if (arg1 == 0) 
    goto take_post_trap32941;
  t5 = *(u64 *)(iSP + -8);   
  *(u64 *)(iSP + 24) = t5;   
  arg1 = arg1 - 1;   
  if (arg1 == 0) 
    goto take_post_trap32941;
  t5 = *(u64 *)(iSP + -16);   
  *(u64 *)(iSP + 16) = t5;   
  arg1 = arg1 - 1;   
  if (arg1 == 0) 
    goto take_post_trap32941;
  t5 = *(u64 *)(iSP + -24);   
  *(u64 *)(iSP + 8) = t5;   
  arg1 = arg1 - 1;   

take_post_trap32941:
  if (_trace) printf("take_post_trap32941:\n");
  iSP = iSP + 32;
  t5 = *(s32 *)&processor->continuation;   
  t7 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t8 = (8192) << 16;   
  t4 = (u32)t4;   
  t7 = t7 | 192;
  *(u32 *)iFP = t5;   
  *(u32 *)(iFP + 4) = t7;   		// write the stack cache 
  t8 = t4 & t8;
  t8 = t8 >> 2;   
  t6 = Type_Fixnum+0xC0;
  t8 = t4 | t8;
  *(u32 *)(iFP + 8) = t8;   
  *(u32 *)(iFP + 12) = t6;   		// write the stack cache 
  iLP = iSP + 8;
  t6 = Type_Fixnum;
  t8 = t11;
  *(u32 *)(iFP + 16) = t8;   
  *(u32 *)(iFP + 20) = t6;   		// write the stack cache 
  /* Convert PC to a real continuation. */
  t6 = iPC & 1;
  t8 = iPC >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *(u32 *)(iFP + 24) = t8;   
  *(u32 *)(iFP + 28) = t6;   		// write the stack cache 
  t7 = *(u64 *)&(processor->fccrtrapmask);   		// Get CR mask 
  t5 = (ValueDisposition_Value*4) << 16;   		// 1<<18! 
  t6 = iLP - iFP;   		// Arg size 
  t8 = iFP - t1;   		// Caller Frame Size 
  t6 = t6 >> 3;   		// Arg size in words 
  t8 = t8 << 6;   		// Caller Frame Size in words in place 
  t5 = t5 | t6;
  t5 = t5 | t8;
  /* TagCdr. */
  t9 = t2 >> 6;   
  t6 = t4 >> 30;   
  t8 = t9 - t6;   
  if ((s64)t8 >= 0)  
    t6 = t9;
  t6 = t6 << 30;   
  t4 = t4 & t7;		// Mask off unwanted bits 
  t4 = t4 | t6;		// Add trap mode 
  t4 = t4 | t5;		// Add argsize, apply, disposition, caller FS 
  *(u32 *)&processor->control = t4;   
  /* Convert PC to a real continuation. */
  t6 = t12 & 1;
  t8 = t12 >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *(u64 *)&processor->continuationcp = zero;   
  *((u32 *)(&processor->continuation)+1) = t6;   
  *(u32 *)&processor->continuation = t8;   
  /* Convert real continuation to PC. */
  iPC = t2 & 1;
  iPC = t3 + iPC;
  iPC = t3 + iPC;
  t6 = t4 >> 30;   		// Save current trap mode 
  t4 = t4 >> 30;   		// Isolate trap mode 
  t8 = *(s32 *)&processor->cslimit;   		// Limit for emulator mode 
  t9 = *(s32 *)&processor->csextralimit;   		// Limit for extra stack and higher modes 
  if (t4)   		// Get the right limit for the current trap mode 
    t8 = t9;
  t8 = (u32)t8;   		// Might have been sign extended 
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t9 = iSP - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  t4 = t9 + t4;		// reconstruct VMA 
  t9 = ((s64)t4 < (s64)t8) ? 1 : 0;   		// Check for overflow 
  if (t9 == 0) 		// Jump if overflow 
    goto take_post_trap32942;
  /* Convert a halfword address into a CP pointer. */
  iCP = iPC >> (CacheLine_RShift & 63);   		// Get third byte into bottom 
  t9 = *(u64 *)&(processor->icachebase);   		// get the base of the icache 
  t8 = zero + -1;   
  t8 = t8 + ((4) << 16);   
  iCP = iCP << (CacheLine_LShift & 63);   		// Now third byte is zero-shifted 
  iCP = iPC + iCP;
  iCP = iCP & t8;
  t8 = iCP << 5;   		// temp=cpos*32 
  iCP = iCP << 4;   		// cpos=cpos*16 
  t9 = t9 + t8;		// temp2=base+cpos*32 
  iCP = t9 + iCP;		// cpos=base+cpos*48 
  goto cachevalid;   

take_post_trap32942:
  if (_trace) printf("take_post_trap32942:\n");
  if (t6 == 0) 		// Take the overflow if in emulator mode 
    goto stackoverflow;
  goto fatalstackoverflow;

stack_cache_overflow_check32956:
  if (_trace) printf("stack_cache_overflow_check32956:\n");
  arg2 = 8;
  goto stackcacheoverflowhandler;   

vma_memory_read32946:
  if (_trace) printf("vma_memory_read32946:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read32945;   

vma_memory_read32948:
  if (_trace) printf("vma_memory_read32948:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read32947;
  t8 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32944;   

vma_memory_read32947:
  if (_trace) printf("vma_memory_read32947:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read32952:
  if (_trace) printf("vma_memory_read32952:\n");
  t6 = t5 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read32951;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read32955;   

vma_memory_read32951:

vma_memory_read32950:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

get_trap_vector_entry32943:
  if (_trace) printf("get_trap_vector_entry32943:\n");
  goto illegaltrapvector;

/* end HandleException */
/* start STACKOVERFLOW */


stackoverflow:
  if (_trace) printf("stackoverflow:\n");
  *(u64 *)&processor->restartsp = iSP;   
  t1 = iFP;		// save old frame pointer 
  t4 = *(s32 *)&processor->control;   
  t9 = *(u64 *)&(processor->fepmodetrapvecaddress);   
  t8 = *(u64 *)&(processor->trapvecbase);   
  t5 = (-16384) << 16;   
  t6 = t4 >> 30;   
  t5 = t4 | t5;		// Set trap mode to 3 
  t6 = t6 & 3;
  *(u32 *)&processor->control = t5;   
  t7 = t6 - 3;   
  t8 = t8 + TrapVector_StackOverflow;
  if (t7 == 0)   
    t8 = t9;
  *(u64 *)&processor->tvi = t8;   		// Record TVI for tracing (if enabled) 
  /* Memory Read Internal */

vma_memory_read32960:
  t9 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t6 = t8 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t3 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t9 = t8 - t9;   		// Stack cache offset 
  t5 = *(u64 *)&(processor->dataread_mask);   
  t7 = ((u64)t9 < (u64)t7) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32962;

vma_memory_read32961:
  t6 = zero + 240;   
  t5 = t5 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t5 & 1)   
    goto vma_memory_read32964;

vma_memory_read32971:
  t5 = t2 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto get_trap_vector_entry32959;
  *(u32 *)&processor->control = t4;   		// Restore the cr 
  t8 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t5 = zero + 128;   
  t6 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t5 = t5 + 8;		// Account for what we're about to push 
  t5 = (t5 * 8) + iSP;  		// SCA of desired end of cache 
  t6 = (t8 * 8) + t6;  		// SCA of current end of cache 
  t8 = ((s64)t5 <= (s64)t6) ? 1 : 0;   
  if (t8 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check32972;
  iFP = (zero * 8) + zero;  
  iFP = iSP - iFP;   
  iFP = iFP + 8;
  if (zero == 0) 
    goto take_post_trap32957;
  t5 = *(u64 *)iSP;   
  *(u64 *)(iSP + 32) = t5;   
  if (zero == 0) 
    goto take_post_trap32957;
  t5 = *(u64 *)(iSP + -8);   
  *(u64 *)(iSP + 24) = t5;   
  if (zero == 0) 
    goto take_post_trap32957;
  t5 = *(u64 *)(iSP + -16);   
  *(u64 *)(iSP + 16) = t5;   
  if (zero == 0) 
    goto take_post_trap32957;
  t5 = *(u64 *)(iSP + -24);   
  *(u64 *)(iSP + 8) = t5;   

take_post_trap32957:
  if (_trace) printf("take_post_trap32957:\n");
  iSP = iSP + 32;
  t5 = *(s32 *)&processor->continuation;   
  t7 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t8 = (8192) << 16;   
  t4 = (u32)t4;   
  t7 = t7 | 192;
  *(u32 *)iFP = t5;   
  *(u32 *)(iFP + 4) = t7;   		// write the stack cache 
  t8 = t4 & t8;
  t8 = t8 >> 2;   
  t6 = Type_Fixnum+0xC0;
  t8 = t4 | t8;
  *(u32 *)(iFP + 8) = t8;   
  *(u32 *)(iFP + 12) = t6;   		// write the stack cache 
  iLP = iSP + 8;
  t6 = Type_Fixnum;
  t8 = TrapVector_StackOverflow;
  *(u32 *)(iFP + 16) = t8;   
  *(u32 *)(iFP + 20) = t6;   		// write the stack cache 
  /* Convert PC to a real continuation. */
  t6 = iPC & 1;
  t8 = iPC >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *(u32 *)(iFP + 24) = t8;   
  *(u32 *)(iFP + 28) = t6;   		// write the stack cache 
  t7 = *(u64 *)&(processor->fccrtrapmask);   		// Get CR mask 
  t5 = (ValueDisposition_Value*4) << 16;   		// 1<<18! 
  t6 = iLP - iFP;   		// Arg size 
  t8 = iFP - t1;   		// Caller Frame Size 
  t6 = t6 >> 3;   		// Arg size in words 
  t8 = t8 << 6;   		// Caller Frame Size in words in place 
  t5 = t5 | t6;
  t5 = t5 | t8;
  /* TagCdr. */
  t9 = t2 >> 6;   
  t6 = t4 >> 30;   
  t8 = t9 - t6;   
  if ((s64)t8 >= 0)  
    t6 = t9;
  t6 = t6 << 30;   
  t4 = t4 & t7;		// Mask off unwanted bits 
  t4 = t4 | t6;		// Add trap mode 
  t4 = t4 | t5;		// Add argsize, apply, disposition, caller FS 
  *(u32 *)&processor->control = t4;   
  /* Convert PC to a real continuation. */
  t6 = iPC & 1;
  t8 = iPC >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *(u64 *)&processor->continuationcp = zero;   
  *((u32 *)(&processor->continuation)+1) = t6;   
  *(u32 *)&processor->continuation = t8;   
  /* Convert real continuation to PC. */
  iPC = t2 & 1;
  iPC = t3 + iPC;
  iPC = t3 + iPC;
  t6 = t4 >> 30;   		// Save current trap mode 
  t4 = t4 >> 30;   		// Isolate trap mode 
  t8 = *(s32 *)&processor->cslimit;   		// Limit for emulator mode 
  t9 = *(s32 *)&processor->csextralimit;   		// Limit for extra stack and higher modes 
  if (t4)   		// Get the right limit for the current trap mode 
    t8 = t9;
  t8 = (u32)t8;   		// Might have been sign extended 
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  t4 = *(u64 *)&(processor->stackcachebasevma);   
  t9 = iSP - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  t4 = t9 + t4;		// reconstruct VMA 
  t9 = ((s64)t4 < (s64)t8) ? 1 : 0;   		// Check for overflow 
  if (t9 == 0) 		// Jump if overflow 
    goto take_post_trap32958;
  /* Convert a halfword address into a CP pointer. */
  iCP = iPC >> (CacheLine_RShift & 63);   		// Get third byte into bottom 
  t9 = *(u64 *)&(processor->icachebase);   		// get the base of the icache 
  t8 = zero + -1;   
  t8 = t8 + ((4) << 16);   
  iCP = iCP << (CacheLine_LShift & 63);   		// Now third byte is zero-shifted 
  iCP = iPC + iCP;
  iCP = iCP & t8;
  t8 = iCP << 5;   		// temp=cpos*32 
  iCP = iCP << 4;   		// cpos=cpos*16 
  t9 = t9 + t8;		// temp2=base+cpos*32 
  iCP = t9 + iCP;		// cpos=base+cpos*48 
  goto cachevalid;   

take_post_trap32958:
  if (_trace) printf("take_post_trap32958:\n");
  if (t6 == 0) 		// Take the overflow if in emulator mode 
    goto stackoverflow;
  goto fatalstackoverflow;

stack_cache_overflow_check32972:
  if (_trace) printf("stack_cache_overflow_check32972:\n");
  arg2 = 8;
  goto stackcacheoverflowhandler;   

vma_memory_read32962:
  if (_trace) printf("vma_memory_read32962:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read32961;   

vma_memory_read32964:
  if (_trace) printf("vma_memory_read32964:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read32963;
  t8 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32960;   

vma_memory_read32963:
  if (_trace) printf("vma_memory_read32963:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read32968:
  if (_trace) printf("vma_memory_read32968:\n");
  t6 = t5 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read32967;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read32971;   

vma_memory_read32967:

vma_memory_read32966:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

get_trap_vector_entry32959:
  if (_trace) printf("get_trap_vector_entry32959:\n");
  goto illegaltrapvector;

/* end STACKOVERFLOW */
/* start StartPreTrap */


startpretrap:
  if (_trace) printf("startpretrap:\n");
  t2 = *(u64 *)&(processor->linkage);   
  if (t2 != 0)   
    goto nativeexception;
  t4 = *(s32 *)&processor->control;   
  t9 = *(u64 *)&(processor->fepmodetrapvecaddress);   
  t8 = *(u64 *)&(processor->trapvecbase);   
  t5 = (-16384) << 16;   
  t6 = t4 >> 30;   
  t5 = t4 | t5;		// Set trap mode to 3 
  t6 = t6 & 3;
  *(u32 *)&processor->control = t5;   
  t7 = t6 - 3;   
  t8 = t8 + t10;
  if (t7 == 0)   
    t8 = t9;
  *(u64 *)&processor->tvi = t8;   		// Record TVI for tracing (if enabled) 
  /* Memory Read Internal */

vma_memory_read32974:
  t9 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t6 = t8 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t3 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t9 = t8 - t9;   		// Stack cache offset 
  t5 = *(u64 *)&(processor->dataread_mask);   
  t7 = ((u64)t9 < (u64)t7) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read32976;

vma_memory_read32975:
  t6 = zero + 240;   
  t5 = t5 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t5 & 1)   
    goto vma_memory_read32978;

vma_memory_read32985:
  t5 = t2 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto get_trap_vector_entry32973;
  *(u32 *)&processor->control = t4;   		// Restore the cr 
  iSP = *(u64 *)&(processor->restartsp);   
  t7 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t4 = zero + 128;   
  t5 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t4 = t4 + 8;		// Account for what we're about to push 
  t4 = (t4 * 8) + iSP;  		// SCA of desired end of cache 
  t5 = (t7 * 8) + t5;  		// SCA of current end of cache 
  t7 = ((s64)t4 <= (s64)t5) ? 1 : 0;   
  if (t7 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check32986;
  t5 = *(s32 *)&processor->continuation;   
  t4 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t7 = *(s32 *)&processor->control;   
  t7 = (u32)t7;   
  t4 = t4 | 192;
  *(u32 *)(iSP + 8) = t5;   
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  t6 = Type_Fixnum+0xC0;
  *(u32 *)(iSP + 8) = t7;   
  *(u32 *)(iSP + 12) = t6;   		// write the stack cache 
  iSP = iSP + 8;
  t6 = t10;
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t6;   
  *(u32 *)(iSP + 12) = t8;   		// write the stack cache 
  iSP = iSP + 8;
  /* Convert PC to a real continuation. */
  t6 = iPC & 1;
  t8 = iPC >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *((u32 *)(&processor->continuation)+1) = t6;   
  *(u32 *)&processor->continuation = t8;   
  *(u64 *)&processor->continuationcp = iCP;   
  t9 = t6 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;   
  *(u32 *)(iSP + 12) = t9;   		// write the stack cache 
  iSP = iSP + 8;
  goto *r0; /* ret */

stack_cache_overflow_check32986:
  if (_trace) printf("stack_cache_overflow_check32986:\n");
  arg2 = 8;
  goto stackcacheoverflowhandler;   

vma_memory_read32976:
  if (_trace) printf("vma_memory_read32976:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read32975;   

vma_memory_read32978:
  if (_trace) printf("vma_memory_read32978:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read32977;
  t8 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read32974;   

vma_memory_read32977:
  if (_trace) printf("vma_memory_read32977:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read32982:
  if (_trace) printf("vma_memory_read32982:\n");
  t6 = t5 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read32981;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read32985;   

vma_memory_read32981:

vma_memory_read32980:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

get_trap_vector_entry32973:
  if (_trace) printf("get_trap_vector_entry32973:\n");
  goto illegaltrapvector;

/* end StartPreTrap */
/* start FinishPreTrap */


finishpretrap:
  if (_trace) printf("finishpretrap:\n");
  iFP = *(u64 *)&(processor->restartsp);   
  iFP = iFP + 8;		// iFP now points to the start of our new frame 
  iLP = iSP + 8;		// Points beyond the last argument 
  t4 = *(u64 *)&(processor->fccrtrapmask);   		// Get CR mask 
  t5 = (ValueDisposition_Value*4) << 16;   		// 1<<18! 
  t6 = iLP - iFP;   		// Arg size 
  t8 = iFP - t1;   		// Caller Frame Size 
  t6 = t6 >> 3;   		// Arg size in words 
  t8 = t8 << 6;   		// Caller Frame Size in words in place 
  t5 = t5 | t6;
  t5 = t5 | t8;
  /* TagCdr. */
  t9 = t2 >> 6;   
  t6 = t7 >> 30;   
  t8 = t9 - t6;   
  if ((s64)t8 >= 0)  
    t6 = t9;
  t6 = t6 << 30;   
  t7 = t7 & t4;		// Mask off unwanted bits 
  t7 = t7 | t6;		// Add trap mode 
  t7 = t7 | t5;		// Add argsize, apply, disposition, caller FS 
  *(u32 *)&processor->control = t7;   
  /* Convert real continuation to PC. */
  iPC = t2 & 1;
  iPC = t3 + iPC;
  iPC = t3 + iPC;
  /* Check for stack overflow */
  t7 = t7 >> 30;   		// Isolate trap mode 
  t8 = *(s32 *)&processor->cslimit;   		// Limit for emulator mode 
  t9 = *(s32 *)&processor->csextralimit;   		// Limit for extra stack and higher modes 
  if (t7)   		// Get the right limit for the current trap mode 
    t8 = t9;
  t8 = (u32)t8;   		// Might have been sign extended 
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  t7 = *(u64 *)&(processor->stackcachebasevma);   
  t9 = iSP - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  t7 = t9 + t7;		// reconstruct VMA 
  t9 = ((s64)t7 < (s64)t8) ? 1 : 0;   		// Check for overflow 
  if (t9 == 0) 		// Jump if overflow 
    goto stackoverflow;
  /* Convert a halfword address into a CP pointer. */
  iCP = iPC >> (CacheLine_RShift & 63);   		// Get third byte into bottom 
  t9 = *(u64 *)&(processor->icachebase);   		// get the base of the icache 
  t8 = zero + -1;   
  t8 = t8 + ((4) << 16);   
  iCP = iCP << (CacheLine_LShift & 63);   		// Now third byte is zero-shifted 
  iCP = iPC + iCP;
  iCP = iCP & t8;
  t8 = iCP << 5;   		// temp=cpos*32 
  iCP = iCP << 4;   		// cpos=cpos*16 
  t9 = t9 + t8;		// temp2=base+cpos*32 
  iCP = t9 + iCP;		// cpos=base+cpos*48 
  goto cachevalid;   

/* end FinishPreTrap */
/* start ILLEGALOPERAND */


illegaloperand:
  if (_trace) printf("illegaloperand:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Error;		// save the trap vector index 
  r0 = (u64)&&return0074;
  goto startpretrap;
return0074:
  t11 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg2;   
  *(u32 *)(iSP + 12) = t11;   		// write the stack cache 
  iSP = iSP + 8;
  t11 = Type_Locative;
  *(u32 *)(iSP + 8) = arg5;   
  *(u32 *)(iSP + 12) = t11;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end ILLEGALOPERAND */
/* start RESETTRAP */


resettrap:
  if (_trace) printf("resettrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Reset;		// save the trap vector index 
  r0 = (u64)&&return0075;
  goto startpretrap;
return0075:
  goto finishpretrap;   

/* end RESETTRAP */
/* start PULLAPPLYARGSTRAP */


pullapplyargstrap:
  if (_trace) printf("pullapplyargstrap:\n");
  t12 = *(s32 *)iSP;   
  t11 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  t12 = (u32)t12;   
  *(u64 *)&processor->restartsp = iSP;   
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PullApplyArgs;		// save the trap vector index 
  r0 = (u64)&&return0076;
  goto startpretrap;
return0076:
  arg2 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg1;   
  *(u32 *)(iSP + 12) = arg2;   		// write the stack cache 
  iSP = iSP + 8;
  arg2 = t11 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t12;   
  *(u32 *)(iSP + 12) = arg2;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PULLAPPLYARGSTRAP */
/* start TRACETRAP */


tracetrap:
  if (_trace) printf("tracetrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Trace;		// save the trap vector index 
  r0 = (u64)&&return0077;
  goto startpretrap;
return0077:
  goto finishpretrap;   

/* end TRACETRAP */
/* start PREEMPTREQUESTTRAP */


preemptrequesttrap:
  if (_trace) printf("preemptrequesttrap:\n");
#endif
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PreemptRequest;		// save the trap vector index 
  r0 = (u64)&&return0078;
  goto startpretrap;
return0078:
  goto finishpretrap;   

/* end PREEMPTREQUESTTRAP */
/* start HIGHPRIORITYSEQUENCEBREAK */


highprioritysequencebreak:
  if (_trace) printf("highprioritysequencebreak:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_HighPrioritySequenceBreak;		// save the trap vector index 
  r0 = (u64)&&return0079;
  goto startpretrap;
return0079:
  goto finishpretrap;   

/* end HIGHPRIORITYSEQUENCEBREAK */
/* start LOWPRIORITYSEQUENCEBREAK */


lowprioritysequencebreak:
  if (_trace) printf("lowprioritysequencebreak:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_LowPrioritySequenceBreak;		// save the trap vector index 
  r0 = (u64)&&return0080;
  goto startpretrap;
return0080:
  goto finishpretrap;   

/* end LOWPRIORITYSEQUENCEBREAK */
/* start DBUNWINDFRAMETRAP */


dbunwindframetrap:
  if (_trace) printf("dbunwindframetrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_DBUnwindFrame;		// save the trap vector index 
  r0 = (u64)&&return0081;
  goto startpretrap;
return0081:
  t11 = *(u64 *)&(processor->bindingstackpointer);   
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end DBUNWINDFRAMETRAP */
/* start DBUNWINDCATCHTRAP */


dbunwindcatchtrap:
  if (_trace) printf("dbunwindcatchtrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_DBUnwindCatch;		// save the trap vector index 
  r0 = (u64)&&return0082;
  goto startpretrap;
return0082:
  t11 = *(u64 *)&(processor->bindingstackpointer);   
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end DBUNWINDCATCHTRAP */
/* start TRANSPORTTRAP */


transporttrap:
  if (_trace) printf("transporttrap:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Transport;		// save the trap vector index 
  r0 = (u64)&&return0083;
  goto startpretrap;
return0083:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end TRANSPORTTRAP */
/* start MONITORTRAP */


monitortrap:
  if (_trace) printf("monitortrap:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Monitor;		// save the trap vector index 
  r0 = (u64)&&return0084;
  goto startpretrap;
return0084:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end MONITORTRAP */
/* start PAGENOTRESIDENT */


pagenotresident:
  if (_trace) printf("pagenotresident:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PageNotResident;		// save the trap vector index 
  r0 = (u64)&&return0085;
  goto startpretrap;
return0085:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PAGENOTRESIDENT */
/* start PAGEFAULTREQUESTHANDLER */


pagefaultrequesthandler:
  if (_trace) printf("pagefaultrequesthandler:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PageFaultRequest;		// save the trap vector index 
  r0 = (u64)&&return0086;
  goto startpretrap;
return0086:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PAGEFAULTREQUESTHANDLER */
/* start PAGEWRITEFAULT */


pagewritefault:
  if (_trace) printf("pagewritefault:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PageWriteFault;		// save the trap vector index 
  r0 = (u64)&&return0087;
  goto startpretrap;
return0087:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PAGEWRITEFAULT */
  /* The following handlers should never be invoked. */
/* start UNCORRECTABLEMEMORYERROR */


uncorrectablememoryerror:
  if (_trace) printf("uncorrectablememoryerror:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_UncorrectableMemoryError;		// save the trap vector index 
  r0 = (u64)&&return0088;
  goto startpretrap;
return0088:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end UNCORRECTABLEMEMORYERROR */
/* start BUSERROR */


buserror:
  if (_trace) printf("buserror:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_MemoryBusError;		// save the trap vector index 
  r0 = (u64)&&return0089;
  goto startpretrap;
return0089:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;   
  *(u32 *)(iSP + 12) = t12;   		// write the stack cache 
  iSP = iSP + 8;
  goto finishpretrap;   

/* end BUSERROR */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifuntrap.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ihalt.as
 ************************************************************************/

  /* This file implements the out-of-line parts of the instruction dispatch loop. */
/* start iOutOfLine */


ioutofline:
  if (_trace) printf("ioutofline:\n");

traporsuspendmachine:
  if (_trace) printf("traporsuspendmachine:\n");
  t4 = *(s32 *)&processor->control;   
  *(u64 *)&processor->restartsp = iSP;   		// Be sure this is up-to-date 
  r0 = *(u64 *)&(processor->please_stop); /* lock */     		// Has the spy asked us to stop or trap? 
  t5 = zero;
  *(u64 *)&processor->please_stop = t5; /* lock */   
  t5 = 1;
  if (t5 == 0) 
    goto collision;
  *(u64 *)&processor->stop_interpreter = zero;   

collision:
  t3 = CMPBGE(r0, HaltReason_IllInstn);  		// t3<0>=1 if we've been asked to stop 
  if (t3 & 1)   
    goto SUSPENDMACHINE;
  /* Here when someone wants the emulator to trap. */
  r0 = (u32)(r0 >> ((4&7)*8));   		// Extract PROCESSORSTATE_PLEASE_TRAP (ivory) 
  t4 = t4 >> 30;   		// Isolate current trap mode 
  t3 = (r0 == TrapReason_HighPrioritySequenceBreak) ? 1 : 0;   

force_alignment32992:
  if (_trace) printf("force_alignment32992:\n");
  if (t3 == 0) 
    goto basic_dispatch32988;
  /* Here if argument TrapReasonHighPrioritySequenceBreak */
  t4 = ((u64)t4 <= (u64)TrapMode_ExtraStack) ? 1 : 0;   		// Only interrupts EXTRA-STACK and EMULATOR 
  if (t4 == 0) 
    goto continuecurrentinstruction;
  goto highprioritysequencebreak;

basic_dispatch32988:
  if (_trace) printf("basic_dispatch32988:\n");
  t3 = (r0 == TrapReason_LowPrioritySequenceBreak) ? 1 : 0;   

force_alignment32993:
  if (_trace) printf("force_alignment32993:\n");
  if (t3 == 0) 
    goto basic_dispatch32989;
  /* Here if argument TrapReasonLowPrioritySequenceBreak */
  if (t4 != 0)   		// Only interrupts EMULATOR 
    goto continuecurrentinstruction;
  goto lowprioritysequencebreak;

basic_dispatch32989:
  if (_trace) printf("basic_dispatch32989:\n");
  /* Here for all other cases */
  /* Check for preempt-request trap */
  t5 = *(s32 *)&processor->interruptreg;   		// Get the preempt-pending bit 
  if (t4 != 0)   		// Don't take preempt trap unless in emulator mode 
    goto continuecurrentinstruction;
  if ((t5 & 1) == 0)   		// Jump if preempt request not pending 
    goto continuecurrentinstruction;
  goto preemptrequesttrap;

basic_dispatch32987:
  if (_trace) printf("basic_dispatch32987:\n");

SUSPENDMACHINE:
  if (_trace) printf("SUSPENDMACHINE:\n");
  t1 = (u32)r0;   		// Get the reason 
  goto stopinterp;   

ILLEGALINSTRUCTION:
  if (_trace) printf("ILLEGALINSTRUCTION:\n");
  t1 = HaltReason_IllInstn;
  goto stopinterp;   

haltmachine:
  if (_trace) printf("haltmachine:\n");
  t1 = HaltReason_Halted;
  goto stopinterp;   

fatalstackoverflow:
  if (_trace) printf("fatalstackoverflow:\n");
  t1 = HaltReason_FatalStackOverflow;
  goto stopinterp;   

illegaltrapvector:
  if (_trace) printf("illegaltrapvector:\n");
  t1 = HaltReason_IllegalTrapVector;
  goto stopinterp;   

stopinterp:
  if (_trace) printf("stopinterp:\n");
  r0 = t1;		// Return the halt reason 
  *(u32 *)&processor->please_stop = zero;   		// Clear the request flag 
  *(u64 *)&processor->cp = iCP;   
  *(u64 *)&processor->epc = iPC;   
  *(u64 *)&processor->sp = iSP;   
  *(u64 *)&processor->fp = iFP;   
  *(u64 *)&processor->lp = iLP;   
  *(u64 *)&processor->runningp = zero;   		// Stop the (emulated) chip 
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r26 = *(u64 *)&(processor->asrr26);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  r30 = *(u64 *)&(processor->asrr30);   
  r14 = *(u64 *)&(processor->asrr14);   
  goto *ra; /* ret */

/* end iOutOfLine */



/* End of file automatically generated from ../alpha-emulator/ihalt.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/idouble.as
 ************************************************************************/

  /* Support for double precision floating point. */
/* start FetchDoubleFloat */


fetchdoublefloat:
  if (_trace) printf("fetchdoublefloat:\n");
  sp = sp + -8;   
  /* Memory Read Internal */

vma_memory_read32995:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read32997;

vma_memory_read32996:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read32999;

vma_memory_read33006:
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto fetch_double_float_internal32994;
  *((u32 *)(&processor->fp0)+1) = arg6;   
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read33007:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read33009;

vma_memory_read33008:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read33011;

vma_memory_read33018:
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto fetch_double_float_internal32994;
  *(u32 *)&processor->fp0 = arg6;   
  sp = sp + 8;   
  goto *r0; /* ret */

vma_memory_read33011:
  if (_trace) printf("vma_memory_read33011:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read33010;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read33007;   

vma_memory_read33010:
  if (_trace) printf("vma_memory_read33010:\n");

vma_memory_read33009:
  if (_trace) printf("vma_memory_read33009:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0090;
  goto memoryreaddatadecode;
return0090:
  r0 = *(u64 *)sp;   
  goto vma_memory_read33018;   

vma_memory_read32999:
  if (_trace) printf("vma_memory_read32999:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read32998;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read32995;   

vma_memory_read32998:
  if (_trace) printf("vma_memory_read32998:\n");

vma_memory_read32997:
  if (_trace) printf("vma_memory_read32997:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0091;
  goto memoryreaddatadecode;
return0091:
  r0 = *(u64 *)sp;   
  goto vma_memory_read33006;   

fetch_double_float_internal32994:
  if (_trace) printf("fetch_double_float_internal32994:\n");
  arg6 = Type_DoubleFloat;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

/* end FetchDoubleFloat */
/* start ConsDoubleFloat */


consdoublefloat:
  if (_trace) printf("consdoublefloat:\n");
  sp = sp + -8;   
  arg6 = *(s32 *)&processor->fp0;   
  arg5 = *((s32 *)(&processor->fp0)+1);   
  t5 = *(u64 *)&(processor->lcarea);   
  t8 = *(u64 *)&(processor->niladdress);   
  t6 = *(s32 *)&processor->lclength;   
  arg2 = *(u64 *)&(processor->lcaddress);   		// Fetch address 
  t7 = (t5 == t8) ? 1 : 0;   
  if (t7 != 0)   		// Decached area 
    goto cons_double_float_internal33019;
  t7 = t6 - 2;   		// Effectively an unsigned 32-bit compare 
  if ((s64)t7 < 0)   		// Insufficient cache 
    goto cons_double_float_internal33019;
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  *(u32 *)&processor->lclength = t7;   		// Store remaining length 
  t8 = (u32)arg2;   
  t8 = t8 + 2;		// Increment address 
  *(u32 *)&processor->lcaddress = t8;   		// Store updated address 
  arg2 = (u32)arg2;   
  t9 = Type_Fixnum;
  t9 = t9 | 128;
  t5 = arg2 + ivory;
  t8 = (t5 * 4);   
  t7 = LDQ_U(t5);   
  t6 = (t9 & 0xff) << ((t5&7)*8);   
  t7 = t7 & ~(0xffL << (t5&7)*8);   

force_alignment33020:
  if (_trace) printf("force_alignment33020:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)t8 = arg5;   
  t10 = arg2 + 1;
  t9 = Type_Fixnum;
  t9 = t9 | 64;
  t5 = t10 + ivory;
  t8 = (t5 * 4);   
  t7 = LDQ_U(t5);   
  t6 = (t9 & 0xff) << ((t5&7)*8);   
  t7 = t7 & ~(0xffL << (t5&7)*8);   

force_alignment33021:
  if (_trace) printf("force_alignment33021:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)t8 = arg6;   
  sp = sp + 8;   
  goto *r0; /* ret */

cons_double_float_internal33019:
  if (_trace) printf("cons_double_float_internal33019:\n");
  arg6 = Type_DoubleFloat;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

/* end ConsDoubleFloat */
/* start DoDoubleFloatOp */

  /* Halfword operand from stack instruction - DoDoubleFloatOp */
  /* arg2 has the preloaded 8 bit operand. */

dodoublefloatop:
  if (_trace) printf("dodoublefloatop:\n");

DoDoubleFloatOpIM:
  if (_trace) printf("DoDoubleFloatOpIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindodoublefloatop;   

DoDoubleFloatOpSP:
  if (_trace) printf("DoDoubleFloatOpSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoDoubleFloatOpLP:
  if (_trace) printf("DoDoubleFloatOpLP:\n");

DoDoubleFloatOpFP:
  if (_trace) printf("DoDoubleFloatOpFP:\n");

headdodoublefloatop:
  if (_trace) printf("headdodoublefloatop:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindodoublefloatop:
  if (_trace) printf("begindodoublefloatop:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg3 = *(s32 *)(iSP + -24);   		// X high 
  arg4 = *(s32 *)(iSP + -16);   		// X low 
  arg5 = *(s32 *)(iSP + -8);   		// Y high 
  arg6 = *(s32 *)iSP;   		// Y low 
  arg3 = arg3 << 32;   		// Get high part up top 
  arg4 = (u32)arg4;   
  arg5 = arg5 << 32;   		// Get high part up top 
  arg6 = (u32)arg6;   
  arg3 = arg3 | arg4;		// ARG3 is now X 
  arg5 = arg5 | arg6;		// ARG5 is now Y 
  *(u64 *)&processor->fp0 = arg3;   
  *(u64 *)&processor->fp1 = arg5;   
  t2 = arg1 >> 32;   		// Immediate tag 
  t1 = (u32)arg1;   		// Immediate data 
  t3 = t2 - Type_Fixnum;   
  t3 = t3 & 63;		// Strip CDR code 
  if (t3 != 0)   
    goto doublefloatiop;
  LDT(1, f1, processor->fp0);   
  LDT(2, f2, processor->fp1);   
  /* NIL */
  t3 = zero + DoubleFloatOp_Add;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch33023;
  /* Here if argument DoubleFloatOpAdd */
  ADDT(1, f1, 1, f1, 2, f2); /* addt */   
  goto mondo_dispatch33022;   

mondo_dispatch33023:
  if (_trace) printf("mondo_dispatch33023:\n");
  t3 = zero + DoubleFloatOp_Sub;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch33024;
  /* Here if argument DoubleFloatOpSub */
  SUBT(1, f1, 1, f1, 2, f2);   
  goto mondo_dispatch33022;   

mondo_dispatch33024:
  if (_trace) printf("mondo_dispatch33024:\n");
  t3 = zero + DoubleFloatOp_Multiply;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch33025;
  /* Here if argument DoubleFloatOpMultiply */
  MULT(1, f1, 1, f1, 2, f2);   
  goto mondo_dispatch33022;   

mondo_dispatch33025:
  if (_trace) printf("mondo_dispatch33025:\n");
  t3 = zero + DoubleFloatOp_Divide;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch33026;
  /* Here if argument DoubleFloatOpDivide */
  DIVT(1, f1, 1, f1, 2, f2);   
  goto mondo_dispatch33022;   

mondo_dispatch33026:
  if (_trace) printf("mondo_dispatch33026:\n");

mondo_dispatch33022:
  if (_trace) printf("mondo_dispatch33022:\n");
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t3 = *(u64 *)&(processor->niladdress);   		// There was no FP exception 

doublefloatmerge:
  STT( (u64 *)&processor->fp0, 1, f1 );   
  t1 = *(s32 *)&processor->fp0;   
  t2 = *((s32 *)(&processor->fp0)+1);   
  iSP = iSP - 32;   		// Pop all the operands 
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t2;   		// Push high result 
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t1;   		// Push low result 
  *(u32 *)(iSP + 12) = t4;   		// write the stack cache 
  iSP = iSP + 8;
  iSP = iSP + 8;
  t4 = t3 << 26;   
  t4 = t4 >> 26;   
  *(u64 *)iSP = t4;   		// Push the exception predicate 
  goto NEXTINSTRUCTION;   

doublefloatexc:
  if (_trace) printf("doublefloatexc:\n");
  t3 = *(u64 *)&(processor->taddress);   		// Indicate an FP exception occurred 
  goto doublefloatmerge;   

doublefloatiop:
  if (_trace) printf("doublefloatiop:\n");
  arg5 = 0;
  arg2 = 85;
  goto illegaloperand;

/* end DoDoubleFloatOp */
  /* End of Halfword operand from stack instruction - DoDoubleFloatOp */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/idouble.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunjosh.as
 ************************************************************************/

  /* 'AI' instructions. */
/* start DoDereference */

  /* Halfword operand from stack instruction - DoDereference */

dodereference:
  if (_trace) printf("dodereference:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoDereferenceIM:
  if (_trace) printf("DoDereferenceIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment33051:
  if (_trace) printf("force_alignment33051:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindodereference;   

DoDereferenceSP:
  if (_trace) printf("DoDereferenceSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoDereferenceLP:
  if (_trace) printf("DoDereferenceLP:\n");

DoDereferenceFP:
  if (_trace) printf("DoDereferenceFP:\n");

headdodereference:
  if (_trace) printf("headdodereference:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindodereference:
  if (_trace) printf("begindodereference:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t1 = arg2 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_OneQForward) ? 1 : 0;   

force_alignment33046:
  if (_trace) printf("force_alignment33046:\n");
  if (t2 != 0)   
    goto basic_dispatch33042;
  t2 = (t1 == Type_ElementForward) ? 1 : 0;   

force_alignment33047:
  if (_trace) printf("force_alignment33047:\n");
  if (t2 != 0)   
    goto basic_dispatch33042;
  t2 = (t1 == Type_HeaderForward) ? 1 : 0;   

force_alignment33048:
  if (_trace) printf("force_alignment33048:\n");
  if (t2 != 0)   
    goto basic_dispatch33042;
  t2 = (t1 == Type_ExternalValueCellPointer) ? 1 : 0;   

force_alignment33049:
  if (_trace) printf("force_alignment33049:\n");
  if (t2 == 0) 
    goto basic_dispatch33029;

basic_dispatch33042:
  if (_trace) printf("basic_dispatch33042:\n");
  /* Here if argument (TypeOneQForward TypeElementForward TypeHeaderForward
                  TypeExternalValueCellPointer) */
  /* Memory Read Internal */

vma_memory_read33030:
  t5 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t7 = arg1 + ivory;
  t6 = *(s32 *)&processor->scovlimit;   
  t3 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = arg1 - t5;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read33032;

vma_memory_read33031:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read33034;

vma_memory_read33041:
  t5 = t4 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t3;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch33029:
  if (_trace) printf("basic_dispatch33029:\n");
  t2 = (t1 == Type_LogicVariable) ? 1 : 0;   

force_alignment33050:
  if (_trace) printf("force_alignment33050:\n");
  if (t2 == 0) 
    goto basic_dispatch33043;
  /* Here if argument TypeLogicVariable */
  t5 = Type_ExternalValueCellPointer;
  *(u32 *)(iSP + 8) = arg1;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch33043:
  if (_trace) printf("basic_dispatch33043:\n");
  /* Here for all other cases */
  t5 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg1;   
  *(u32 *)(iSP + 12) = t5;   		// write the stack cache 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch33028:
  if (_trace) printf("basic_dispatch33028:\n");

vma_memory_read33032:
  if (_trace) printf("vma_memory_read33032:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t3 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read33031;   

vma_memory_read33034:
  if (_trace) printf("vma_memory_read33034:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read33033;
  arg1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read33030;   

vma_memory_read33033:
  if (_trace) printf("vma_memory_read33033:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read33038:
  if (_trace) printf("vma_memory_read33038:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read33037;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read33041;   

vma_memory_read33037:

vma_memory_read33036:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

/* end DoDereference */
  /* End of Halfword operand from stack instruction - DoDereference */
/* start DoUnify */

  /* Halfword operand from stack instruction - DoUnify */

dounify:
  if (_trace) printf("dounify:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoUnifyIM:
  if (_trace) printf("DoUnifyIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment33052:
  if (_trace) printf("force_alignment33052:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindounify;   

DoUnifySP:
  if (_trace) printf("DoUnifySP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoUnifyLP:
  if (_trace) printf("DoUnifyLP:\n");

DoUnifyFP:
  if (_trace) printf("DoUnifyFP:\n");

headdounify:
  if (_trace) printf("headdounify:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindounify:
  if (_trace) printf("begindounify:\n");
  /* arg1 has the operand, sign extended if immediate. */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;
  goto NEXTINSTRUCTION;   

/* end DoUnify */
  /* End of Halfword operand from stack instruction - DoUnify */
/* start DoPushLocalLogicVariables */

  /* Halfword operand from stack instruction - DoPushLocalLogicVariables */
  /* arg2 has the preloaded 8 bit operand. */

dopushlocallogicvariables:
  if (_trace) printf("dopushlocallogicvariables:\n");

DoPushLocalLogicVariablesIM:
  if (_trace) printf("DoPushLocalLogicVariablesIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindopushlocallogicvariables;   

DoPushLocalLogicVariablesSP:
  if (_trace) printf("DoPushLocalLogicVariablesSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushLocalLogicVariablesLP:
  if (_trace) printf("DoPushLocalLogicVariablesLP:\n");

DoPushLocalLogicVariablesFP:
  if (_trace) printf("DoPushLocalLogicVariablesFP:\n");

headdopushlocallogicvariables:
  if (_trace) printf("headdopushlocallogicvariables:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindopushlocallogicvariables:
  if (_trace) printf("begindopushlocallogicvariables:\n");
  /* arg1 has the operand, not sign extended if immediate. */
  arg6 = Type_LogicVariable;
  t1 = arg1 >> 32;   
  arg2 = (u32)arg1;   
  t2 = t1 - Type_Fixnum;   
  t2 = t2 & 63;		// Strip CDR code 
  if (t2 != 0)   
    goto pllvillop;
  t4 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t1 = zero + 128;   
  t2 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t1 = t1 + arg2;		// Account for what we're about to push 
  t1 = (t1 * 8) + iSP;  		// SCA of desired end of cache 
  t2 = (t4 * 8) + t2;  		// SCA of current end of cache 
  t4 = ((s64)t1 <= (s64)t2) ? 1 : 0;   
  if (t4 == 0) 		// We're done if new SCA is within bounds 
    goto stackcacheoverflowhandler;
  goto pllvloopend;   

pllvlooptop:
  if (_trace) printf("pllvlooptop:\n");
  *(u32 *)(iSP + 8) = iSP;   
  *(u32 *)(iSP + 12) = arg6;   		// write the stack cache 
  iSP = iSP + 8;

pllvloopend:
  if (_trace) printf("pllvloopend:\n");
  arg2 = arg2 - 1;   
  if ((s64)arg2 >= 0)   		// J. If iterations to go. 
    goto pllvlooptop;
  goto NEXTINSTRUCTION;   

pllvillop:
  if (_trace) printf("pllvillop:\n");
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

/* end DoPushLocalLogicVariables */
  /* End of Halfword operand from stack instruction - DoPushLocalLogicVariables */
/* start DoPushGlobalLogicVariable */

  /* Halfword operand from stack instruction - DoPushGlobalLogicVariable */

dopushgloballogicvariable:
  if (_trace) printf("dopushgloballogicvariable:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoPushGlobalLogicVariableIM:
  if (_trace) printf("DoPushGlobalLogicVariableIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment33069:
  if (_trace) printf("force_alignment33069:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindopushgloballogicvariable;   

DoPushGlobalLogicVariableSP:
  if (_trace) printf("DoPushGlobalLogicVariableSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoPushGlobalLogicVariableLP:
  if (_trace) printf("DoPushGlobalLogicVariableLP:\n");

DoPushGlobalLogicVariableFP:
  if (_trace) printf("DoPushGlobalLogicVariableFP:\n");

headdopushgloballogicvariable:
  if (_trace) printf("headdopushgloballogicvariable:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindopushgloballogicvariable:
  if (_trace) printf("begindopushgloballogicvariable:\n");
  /* arg1 has the operand, sign extended if immediate. */
  t1 = *(s32 *)&processor->bar2;   		// Get the structure stack pointer 
  t3 = Type_ExternalValueCellPointer;
  *(u32 *)(iSP + 8) = t1;   
  *(u32 *)(iSP + 12) = t3;   		// write the stack cache 
  iSP = iSP + 8;
  /* Memory Read Internal */

vma_memory_read33054:
  t6 = *(u64 *)&(processor->stackcachebasevma);   		// Base of stack cache 
  t8 = t1 + ivory;
  t7 = *(s32 *)&processor->scovlimit;   
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = t1 - t6;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)t7) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read33056;

vma_memory_read33055:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read33058;

vma_memory_read33064:
  /* Merge cdr-code */
  t5 = t3 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t7 = *(u64 *)&(processor->stackcachebasevma);   
  t6 = t1 + ivory;
  t9 = *(s32 *)&processor->scovlimit;   
  t5 = (t6 * 4);   
  t8 = LDQ_U(t6);   
  t7 = t1 - t7;   		// Stack cache offset 
  t9 = ((u64)t7 < (u64)t9) ? 1 : 0;   		// In range? 
  t7 = (t4 & 0xff) << ((t6&7)*8);   
  t8 = t8 & ~(0xffL << (t6&7)*8);   

force_alignment33067:
  if (_trace) printf("force_alignment33067:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;   
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write33066;

vma_memory_write33065:
  t2 = t1 + 1;		// Increment the structure-stack-pointer 
  *(u32 *)&processor->bar2 = t2;   		// Set the structure stack pointer 
  goto NEXTINSTRUCTION;   

vma_memory_write33066:
  if (_trace) printf("vma_memory_write33066:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment33068:
  if (_trace) printf("force_alignment33068:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - t7;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
  *(u32 *)t6 = t1;   		// Store in stack 
  *(u32 *)(t6 + 4) = t4;   		// write the stack cache 
  goto vma_memory_write33065;   

vma_memory_read33056:
  if (_trace) printf("vma_memory_read33056:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read33055;   

vma_memory_read33058:
  if (_trace) printf("vma_memory_read33058:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read33057;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read33054;   

vma_memory_read33057:
  if (_trace) printf("vma_memory_read33057:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read33061:

vma_memory_read33060:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

/* end DoPushGlobalLogicVariable */
  /* End of Halfword operand from stack instruction - DoPushGlobalLogicVariable */
/* start DoLogicTailTest */

  /* Halfword operand from stack instruction - DoLogicTailTest */

dologictailtest:
  if (_trace) printf("dologictailtest:\n");
  /* arg2 has the preloaded 8 bit operand. */

DoLogicTailTestIM:
  if (_trace) printf("DoLogicTailTestIM:\n");
  /* This sequence only sucks a moderate amount */
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment33079:
  if (_trace) printf("force_alignment33079:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;   
  arg1 = *(u64 *)&(processor->immediate_arg);   
  goto begindologictailtest;   

DoLogicTailTestSP:
  if (_trace) printf("DoLogicTailTestSP:\n");
  arg1 = arg5;		// Assume SP mode 
  if (arg2 == 0)   		// SP-pop mode 
    arg1 = iSP;
  if (arg2 == 0)   		// Adjust SP if SP-pop mode 
    iSP = arg4;

DoLogicTailTestLP:
  if (_trace) printf("DoLogicTailTestLP:\n");

DoLogicTailTestFP:
  if (_trace) printf("DoLogicTailTestFP:\n");

headdologictailtest:
  if (_trace) printf("headdologictailtest:\n");
  arg1 = (arg2 * 8) + arg1;  		// Compute operand address 
  arg1 = *(u64 *)arg1;   		// Get the operand 

begindologictailtest:
  if (_trace) printf("begindologictailtest:\n");
  /* arg1 has the operand, sign extended if immediate. */
  arg2 = arg1 >> 32;   
  t1 = arg2 & 63;		// Strip off any CDR code bits. 
  t2 = (t1 == Type_List) ? 1 : 0;   

force_alignment33076:
  if (_trace) printf("force_alignment33076:\n");
  if (t2 == 0) 
    goto basic_dispatch33071;
  /* Here if argument TypeList */
  t3 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t3;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch33071:
  if (_trace) printf("basic_dispatch33071:\n");
  t2 = (t1 == Type_ExternalValueCellPointer) ? 1 : 0;   

force_alignment33077:
  if (_trace) printf("force_alignment33077:\n");
  if (t2 == 0) 
    goto basic_dispatch33072;
  /* Here if argument TypeExternalValueCellPointer */
  t3 = *(u64 *)&(processor->taddress);   
  *(u64 *)(iSP + 8) = t3;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch33072:
  if (_trace) printf("basic_dispatch33072:\n");
  t2 = (t1 == Type_ListInstance) ? 1 : 0;   

force_alignment33078:
  if (_trace) printf("force_alignment33078:\n");
  if (t2 == 0) 
    goto basic_dispatch33073;
  /* Here if argument TypeListInstance */
  t3 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t3;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch33073:
  if (_trace) printf("basic_dispatch33073:\n");
  /* Here for all other cases */
  arg6 = t2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

basic_dispatch33070:
  if (_trace) printf("basic_dispatch33070:\n");

/* end DoLogicTailTest */
  /* End of Halfword operand from stack instruction - DoLogicTailTest */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunjosh.as */
/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifuntran.as
 ************************************************************************/

/* start NativeException */


nativeexception:
  if (_trace) printf("nativeexception:\n");
  t1 = *(u64 *)&(processor->linkage);   		// Load linkage to escape block 
  r0 = *(u64 *)&(processor->resumeema);   		// Re-load resumemulator 
  iSP = *(u64 *)&(processor->restartsp);   		// Restore SP (Just in case?) 
  *(u64 *)&processor->linkage = zero;   
  goto *t1; /* ret */

/* end NativeException */
/* start PadPastAref1 */


padpastaref1:
  if (_trace) printf("padpastaref1:\n");
  t1 = *(u64 *)&(processor->linkage);   		// Load linkage to escape block 
  r0 = *(u64 *)&(processor->resumeema);   		// Re-load resumemulator 
  iSP = *(u64 *)&(processor->restartsp);   		// Restore SP (Just in case?) 
  *(u64 *)&processor->linkage = zero;   
  t1 = *(u64 *)&(processor->linkage);   		// Load linkage to escape block 
  r0 = *(u64 *)&(processor->resumeema);   		// Re-load resumemulator 
  iSP = *(u64 *)&(processor->restartsp);   		// Restore SP (Just in case?) 
  *(u64 *)&processor->linkage = zero;   
  goto *t1; /* ret */

/* end PadPastAref1 */
/* start CarSubroutine */


carsubroutine:
  if (_trace) printf("carsubroutine:\n");
  sp = sp + -8;   
  *(u64 *)&processor->linkage = r0;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  r0 = r0 + 4;
  *(u64 *)&processor->restartsp = iSP;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0092;
  goto carinternal;
return0092:
  r0 = *(u64 *)sp;   
  *(u64 *)&processor->linkage = zero;   
  sp = sp + 8;   
  goto *r0; /* ret */

/* end CarSubroutine */
/* start CdrSubroutine */


cdrsubroutine:
  if (_trace) printf("cdrsubroutine:\n");
  sp = sp + -8;   
  *(u64 *)&processor->linkage = r0;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  r0 = r0 + 4;
  *(u64 *)&processor->restartsp = iSP;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0093;
  goto cdrinternal;
return0093:
  r0 = *(u64 *)sp;   
  *(u64 *)&processor->linkage = zero;   
  sp = sp + 8;   
  goto *r0; /* ret */

/* end CdrSubroutine */
/* start CarCdrSubroutine */


carcdrsubroutine:
  if (_trace) printf("carcdrsubroutine:\n");
  sp = sp + -8;   
  *(u64 *)&processor->linkage = r0;   
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  r0 = r0 + 4;
  *(u64 *)&processor->restartsp = iSP;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0094;
  goto carcdrinternal;
return0094:
  r0 = *(u64 *)sp;   
  *(u64 *)&processor->linkage = zero;   
  sp = sp + 8;   
  goto *r0; /* ret */

/* end CarCdrSubroutine */



/* End of file automatically generated from ../alpha-emulator/ifuntran.as */
}

void SpinWheels()
{
    int i;
    for (i = 0; i < 0x2000000; i++)
        ;
}

#include "blanks.c"
