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

}/************************************************************************
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

vma_memory_read22302:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22304;

vma_memory_read22303:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22306;

vma_memory_read22312:
  goto *r0; /* ret */

memoryreaddatadecode:
  if (_trace) printf("memoryreaddatadecode:\n");
  if (t6 == 0) 
    goto vma_memory_read22305;

vma_memory_read22304:
  if (_trace) printf("vma_memory_read22304:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read22303;   

vma_memory_read22306:
  if (_trace) printf("vma_memory_read22306:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22305;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22302;   

vma_memory_read22305:
  if (_trace) printf("vma_memory_read22305:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read22309:
  if (_trace) printf("vma_memory_read22309:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read22308;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read22312;   

vma_memory_read22308:

vma_memory_read22307:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

/* end MemoryReadData */
/* start MemoryReadGeneral */


memoryreadgeneral:
  if (_trace) printf("memoryreadgeneral:\n");
  /* Memory Read Internal */

vma_memory_read22313:
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
    goto vma_memory_read22315;

vma_memory_read22314:
  t8 = t8 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22317;

vma_memory_read22323:
  goto *r0; /* ret */

memoryreadgeneraldecode:
  if (_trace) printf("memoryreadgeneraldecode:\n");
  if (t6 == 0) 
    goto vma_memory_read22316;

vma_memory_read22315:
  if (_trace) printf("vma_memory_read22315:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read22314;   

vma_memory_read22317:
  if (_trace) printf("vma_memory_read22317:\n");

vma_memory_read22316:
  if (_trace) printf("vma_memory_read22316:\n");
  t8 = (arg3 * 4);   		// Cycle-number -> table offset 
  t8 = (t8 * 4) + ivory;   
  t8 = *(u64 *)(t8 + PROCESSORSTATE_DATAREAD);   
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read22321:
  if (_trace) printf("vma_memory_read22321:\n");
  t6 = t8 & MemoryActionIndirect;
  if (t6 == 0) 
    goto vma_memory_read22320;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22313;   

vma_memory_read22320:
  if (_trace) printf("vma_memory_read22320:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read22319;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read22323;   

vma_memory_read22319:

vma_memory_read22318:
  /* Perform memory action */
  arg1 = t8;
  arg2 = arg3;
  goto performmemoryaction;

/* end MemoryReadGeneral */
/* start MemoryReadHeader */


memoryreadheader:
  if (_trace) printf("memoryreadheader:\n");
  /* Memory Read Internal */

vma_memory_read22324:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22326;

vma_memory_read22325:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22328;

vma_memory_read22332:
  goto *r0; /* ret */

memoryreadheaderdecode:
  if (_trace) printf("memoryreadheaderdecode:\n");
  if (t6 == 0) 
    goto vma_memory_read22327;

vma_memory_read22326:
  if (_trace) printf("vma_memory_read22326:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read22325;   

vma_memory_read22328:
  if (_trace) printf("vma_memory_read22328:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22327;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22324;   

vma_memory_read22327:
  if (_trace) printf("vma_memory_read22327:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read22329:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 6;
  goto performmemoryaction;

/* end MemoryReadHeader */
/* start MemoryReadCdr */


memoryreadcdr:
  if (_trace) printf("memoryreadcdr:\n");
  /* Memory Read Internal */

vma_memory_read22333:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22335;

vma_memory_read22334:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22337;

vma_memory_read22341:
  goto *r0; /* ret */

memoryreadcdrdecode:
  if (_trace) printf("memoryreadcdrdecode:\n");
  if (t6 == 0) 
    goto vma_memory_read22336;

vma_memory_read22335:
  if (_trace) printf("vma_memory_read22335:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read22334;   

vma_memory_read22337:
  if (_trace) printf("vma_memory_read22337:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22336;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22333;   

vma_memory_read22336:
  if (_trace) printf("vma_memory_read22336:\n");
  t8 = *(u64 *)&(processor->cdr);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read22338:
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
#ifdef CACHEMETERING
  /* Increment the fill count for both cache entries */
  t10 = *(s32 *)&((CACHELINEP)ecp)->annotation;   
  t11 = *(s32 *)&((CACHELINEP)ocp)->annotation;   
  t10 = (u32)t10;   
  t11 = (u32)t11;   
  t10 = t10 + 1;
  *(u32 *)&((CACHELINEP)ecp)->annotation = t10;
  t11 = t11 + 1;
  *(u32 *)&((CACHELINEP)ocp)->annotation = t11;
#endif
  *(u64 *)&((CACHELINEP)ecp)->pcdata = epc;   		// Set address of even cache posn. 
  arg1 = arg4 & 192;		// CDR code << 6 
  /* TagType. */
  arg4 = arg4 & 63;		// Strip cdr 
  *(u64 *)&((CACHELINEP)ocp)->pcdata = opc;   		// Set address of odd cache posn. 
  iword = (u32)iword;   		// Strip nasty bits out. 

force_alignment22342:
  if (_trace) printf("force_alignment22342:\n");
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
#ifdef CACHEMETERING
  arg1 = *(s32 *)&processor->metervalue;   
  arg4 = *(s32 *)&processor->metercount;   		// The number of remaining tokens. 
  t10 = *(u64 *)&(processor->meterdatabuff);   		// The cache miss meter buffer. 
  arg1 = arg1 + 1;		// count the miss. 
  t11 = *(s32 *)&processor->meterpos;   		// Position for new data. 
  *(u32 *)&processor->metervalue = arg1;
  if (arg4 != 0)   
    goto maybe_meter_miss22343;
  arg2 = *(s32 *)&processor->metermask;   
  t10 = (t11 * 4) + t10;   		// position of the current data item 
  t11 = t11 + 1;
  t11 = t11 & arg2;
  arg2 = *(s32 *)&processor->metermax;   
  t12 = arg1 - arg2;   
  if ((s64)t12 > 0)   
    arg2 = arg1;
  *(u32 *)&processor->metermax = arg2;
		/* store the datapoint */
  *(u32 *)t10 = arg1;
		/* Position for new data. */
  *(u32 *)&processor->meterpos = t11;
  *(u32 *)&processor->metervalue = zero;
  arg4 = *(s32 *)&processor->meterfreq;   

maybe_meter_miss22343:
  if (_trace) printf("maybe_meter_miss22343:\n");
  *(u32 *)&processor->metercount = arg4;
#endif
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
#ifndef CACHEMETERING
  *(u64 *)&((CACHELINEP)ecp)->annotation = zero;   
#endif
#ifdef CACHEMETERING
  epc = *(s32 *)&processor->metervalue;   
  t12 = *(s32 *)&processor->metercount;   		// The number of remaining tokens. 
  t11 = *(u64 *)&(processor->meterdatabuff);   		// The cache miss meter buffer. 
  epc = epc + 1;		// count the miss. 
  arg1 = *(s32 *)&processor->meterpos;   		// Position for new data. 
  *(u32 *)&processor->metervalue = epc;
  if (t12 != 0)   
    goto maybe_meter_miss22344;
  arg2 = *(s32 *)&processor->metermask;   
  t11 = (arg1 * 4) + t11;   		// position of the current data item 
  arg1 = arg1 + 1;
  arg1 = arg1 & arg2;
  arg2 = *(s32 *)&processor->metermax;   
  t10 = epc - arg2;   
  if ((s64)t10 > 0)   
    arg2 = epc;
  *(u32 *)&processor->metermax = arg2;
		/* store the datapoint */
  *(u32 *)t11 = epc;
		/* Position for new data. */
  *(u32 *)&processor->meterpos = arg1;
  *(u32 *)&processor->metervalue = zero;
  t12 = *(s32 *)&processor->meterfreq;   

maybe_meter_miss22344:
  if (_trace) printf("maybe_meter_miss22344:\n");
  *(u32 *)&processor->metercount = t12;
#endif
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

force_alignment22345:
  if (_trace) printf("force_alignment22345:\n");
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
		/* Store the data word */
  *(u32 *)iSP = t2;

force_alignment22352:
  if (_trace) printf("force_alignment22352:\n");
  /* TagType. */
  t1 = t1 & 63;		// make it CDR NEXT 
		/* Store the TAG - this *DOES* dual issue! */
  *(u32 *)(iSP + 4) = t1;

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
#ifdef TRACING
  /* Update the instruction count. */
  t4 = *(u64 *)&(processor->instruction_count);   
  t4 = t4 - 1;   		// Decrement the instruction count. 
  if (t4 != 0)   		// J. if not reached stop point. 
    goto maybe_icount22359;
  zero = zero;		// put a breakpoint here to catch stops 

maybe_icount22359:
  if (_trace) printf("maybe_icount22359:\n");
  *(u64 *)&processor->instruction_count = t4;   
  /* Trace instructions if requested. */
  t4 = *(u64 *)&(processor->trace_hook);   
  if (t4 == 0) 		// J. if not tracing. 
    goto maybe_trace22364;
  /* Record an instruction trace entry */
  t5 = *(s32 *)&t4->tracedata_recording_p;   
  t6 = *(u64 *)&(t4->tracedata_start_pc);   
  if (t5 != 0)   		// Jump if recording is on 
    goto maybe_trace22360;
  t6 = (t6 == iPC) ? 1 : 0;   		// Turn recording on if at the start PC 
  *(u32 *)&t4->tracedata_recording_p = t6;
  if (t6 == 0) 		// Jump if not at the start PC 
    goto maybe_trace22364;

maybe_trace22360:
  if (_trace) printf("maybe_trace22360:\n");
  t5 = *(u64 *)&(t4->tracedata_current_entry);   		// Get address of next trace record  
  t6 = *(u64 *)&(processor->instruction_count);   
  *(u64 *)&t5->tracerecord_epc = iPC;   		// Save current PC 
  *(u64 *)&t5->tracerecord_counter = t6;   		// Save instruction count 
  t6 = *(u64 *)iSP;   
  /* Convert stack cache address to VMA */
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = *(u64 *)&(processor->stackcachebasevma);   
  t8 = iSP - t8;   		// stack cache base relative offset 
  t8 = t8 >> 3;   		// convert byte address to word address 
  t7 = t8 + t7;		// reconstruct VMA 
  *(u64 *)&t5->tracerecord_tos = t6;   		// Save current value of TOS 
  *(u64 *)&t5->tracerecord_sp = t7;   		// Save current SP 
  t6 = *(s32 *)&((CACHELINEP)iCP)->operand;   
  t7 = *(u64 *)&(((CACHELINEP)iCP)->code);   
		/* Save current instruction's operand */
  *(u32 *)&t5->tracerecord_operand = t6;
  *(u64 *)&t5->tracerecord_instruction = t7;   		// Save pointer to current instruction code 
  t7 = *(u64 *)&(processor->control);   
  t8 = *(u64 *)&(((CACHELINEP)iCP)->instruction);   
		/* We don't yet record catch blocks */
  *(u32 *)&t5->tracerecord_catch_block_p = zero;
  *(u64 *)&t5->tracerecord_catch_block_0 = t7;   		// Save control register 
  t6 = *(u64 *)&(processor->tvi);   
  *(u64 *)&t5->tracerecord_instruction_data = t8;   		// Save full word instruction operand 
		/* Save trap indiciator */
  *(u32 *)&t5->tracerecord_trap_p = t6;
  if (t6 == 0) 		// Jump if didn't trap 
    goto maybe_trace22361;
  t6 = *(u64 *)(iFP + 16);   
  *(u64 *)&processor->tvi = zero;   		// Zero flag to avoid false trap entries 
  t7 = *(u64 *)(iFP + 24);   
  *(u64 *)&t5->tracerecord_trap_data_0 = t6;   		// Save trap vector index 
  t8 = *(u64 *)(iFP + 32);   
  *(u64 *)&t5->tracerecord_trap_data_1 = t7;   		// Save fault PC 
  t9 = *(u64 *)(iFP + 40);   
  *(u64 *)&t5->tracerecord_trap_data_2 = t8;   		// Save two additional arguments 
  *(u64 *)&t5->tracerecord_trap_data_3 = t9;   

maybe_trace22361:
  if (_trace) printf("maybe_trace22361:\n");
  t5 = t5 + tracerecordsize;		// Bump to next trace record 
  t6 = *(u64 *)&(t4->tracedata_records_start);   		// Get pointer to start of trace records 
  *(u64 *)&t4->tracedata_current_entry = t5;   		// Set record pointer to keep printer happy 
  t7 = *(u64 *)&(t4->tracedata_records_end);   		// Get pointer to end of trace record 
  t8 = *(u64 *)&(t4->tracedata_printer);   		// Function to print trace if non-zero 
  t7 = ((s64)t7 <= (s64)t5) ? 1 : 0;   		// Non-zero iff we're about to wrap the circular buffer 
  if (t7)   		// Update next record pointer iff we wrapped 
    t5 = t6;
  if (t7 == 0)   		// Don't print if we didn't wrap 
    t8 = zero;
  if (t8 == 0) 		// Jump if we don't need to print 
    goto maybe_trace22362;
  *(u64 *)&processor->cp = iCP;   
  *(u64 *)&processor->epc = iPC;   
  *(u64 *)&processor->sp = iSP;   
  *(u64 *)&processor->fp = iFP;   
  *(u64 *)&processor->lp = iLP;   
  *(u64 *)&processor->asrf2 = arg1;   
  *(u64 *)&processor->asrf3 = arg2;   
  *(u64 *)&processor->asrf4 = arg3;   
  *(u64 *)&processor->asrf5 = arg4;   
  *(u64 *)&processor->asrf6 = arg5;   
  *(u64 *)&processor->asrf7 = arg6;   
  *(u64 *)&processor->asrf8 = t4;   
  *(u64 *)&processor->asrf9 = t5;   
  *(u64 *)&processor->long_pad1 = t3;   
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  pv = t8;
    r0 = (*( u64 (*)(u64, u64) )t8)(arg1, arg2); /* jsr */  
  r9 = *(u64 *)&(processor->asrr9);   
  r10 = *(u64 *)&(processor->asrr10);   
  r11 = *(u64 *)&(processor->asrr11);   
  r12 = *(u64 *)&(processor->asrr12);   
  r13 = *(u64 *)&(processor->asrr13);   
  r15 = *(u64 *)&(processor->asrr15);   
  r27 = *(u64 *)&(processor->asrr27);   
  r29 = *(u64 *)&(processor->asrr29);   
  arg1 = *(u64 *)&(processor->asrf2);   
  arg2 = *(u64 *)&(processor->asrf3);   
  arg3 = *(u64 *)&(processor->asrf4);   
  arg4 = *(u64 *)&(processor->asrf5);   
  arg5 = *(u64 *)&(processor->asrf6);   
  arg6 = *(u64 *)&(processor->asrf7);   
  t4 = *(u64 *)&(processor->asrf8);   
  t5 = *(u64 *)&(processor->asrf9);   
  t3 = *(u64 *)&(processor->long_pad1);   
  iCP = *(u64 *)&(processor->cp);   
  iPC = *(u64 *)&(processor->epc);   
  iSP = *(u64 *)&(processor->sp);   
  iFP = *(u64 *)&(processor->fp);   
  iLP = *(u64 *)&(processor->lp);   
  t7 = zero;		// Claim we didn't wrap 

maybe_trace22362:
  if (_trace) printf("maybe_trace22362:\n");
  *(u64 *)&t4->tracedata_current_entry = t5;   		// Save next record pointer 
  if (t7 == 0) 		// Jump if we didn't wrap 
    goto maybe_trace22363;
		/* Set flag indicating that we wrapped */
  *(u32 *)&t4->tracedata_wrap_p = t7;

maybe_trace22363:
  if (_trace) printf("maybe_trace22363:\n");
  t5 = *(u64 *)&(t4->tracedata_stop_pc);   
  t5 = (t5 == iPC) ? 1 : 0;   		// Non-zero if at PC where we should stop tracing 
  t5 = (t5 == 0) ? 1 : 0;   		// Non-zero if not at the PC 
		/* Update recording flag */
  *(u32 *)&t4->tracedata_recording_p = t5;

maybe_trace22364:
  if (_trace) printf("maybe_trace22364:\n");
#endif
#ifdef STATISTICS
  t4 = *(u64 *)&(((CACHELINEP)iCP)->code);   		// The instruction. 
  t5 = *(u64 *)&(processor->statistics);   		// The usage statistics array 
  t9 = zero + 8191;   
  t6 = t4 >> 4;   
  t6 = t6 & t9;		// Extract the address 
  t7 = (t6 * 4) + t5;   		// Compute the index to the usage data for this instn. 
  t8 = *(s32 *)t7;   		// Get current usage data 
  t8 = t8 + 1;		// Increment 
		/* Set current usage data */
  *(u32 *)t7 = t8;
#endif
#ifdef CACHEMETERING
  t5 = *(s32 *)&processor->metercount;   		// The number of remaining tokens. 
  t4 = *(u64 *)&(processor->meterdatabuff);   		// The cache miss meter buffer. 
  t7 = *(s32 *)&processor->meterpos;   		// Position for new data. 
  t5 = t5 - 1;   		// record a cache hit 
  if (t5 != 0)   
    goto maybe_meter_hit22365;
  t8 = *(s32 *)&processor->metermask;   
  t4 = (t7 * 4) + t4;   		// position of the current data item 
  t9 = *(s32 *)&processor->metervalue;   
  t7 = t7 + 1;
  t7 = t7 & t8;
  t8 = *(s32 *)&processor->metermax;   
  t6 = t9 - t8;   
  if ((s64)t6 > 0)   
    t8 = t9;
  *(u32 *)&processor->metermax = t8;
		/* store the datapoint */
  *(u32 *)t4 = t9;
		/* Position for new data. */
  *(u32 *)&processor->meterpos = t7;
  *(u32 *)&processor->metervalue = zero;
  t5 = *(s32 *)&processor->meterfreq;   

maybe_meter_hit22365:
  if (_trace) printf("maybe_meter_hit22365:\n");
  *(u32 *)&processor->metercount = t5;
#endif
#ifdef DEBUGGING
  if (t3 == 0) 		// Just in case... 
    goto haltmachine;
#endif
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
		/* Push it with CDR-NEXT onto the stack */
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
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
    goto abandon_frame_simple22367;
  iPC = t5 << 1;   		// Assume even PC 
  t1 = t4 & 1;
  t7 = *(u64 *)&(processor->continuationcp);   
  iPC = iPC + t1;

abandon_frame_simple22367:
  if (_trace) printf("abandon_frame_simple22367:\n");
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
		/* Restore the control register */
  *(u32 *)&processor->control = t6;
  t1 = t6 & 255;		// extract the argument size 
  t3 = t3 & 1;
  t3 = t4 | t3;
  *(u64 *)&processor->stop_interpreter = t3;   
  iLP = (t1 * 8) + iFP;  		// Restore the local pointer. 

force_alignment22368:
  if (_trace) printf("force_alignment22368:\n");
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

vma_memory_read22369:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22371;

vma_memory_read22370:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22373;

vma_memory_read22380:
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
		/* push continuation */
  *(u32 *)(iSP + -8) = t8;
		/* write the stack cache */
  *(u32 *)(iSP + -4) = t7;
  t8 = t3 | t5;		// Set call started bit in CR 
  t5 = zero + 256;   
		/* Push control register */
  *(u32 *)iSP = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t6;
  t8 = t8 & ~t5;		// Clear the extra arg bit 
		/* Save control with new state */
  *(u32 *)&processor->control = t8;
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
		/* Push the extra arg. */
  *(u32 *)(iSP + 8) = arg4;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
  iSP = iSP + 8;
  t1 = t1 | t2;		// Set the extra arg bit 
		/* Save control with new state */
  *(u32 *)&processor->control = t1;
  goto cachevalid;   

vma_memory_read22373:
  if (_trace) printf("vma_memory_read22373:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22372;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22369;   

vma_memory_read22372:
  if (_trace) printf("vma_memory_read22372:\n");

vma_memory_read22371:
  if (_trace) printf("vma_memory_read22371:\n");
  r0 = (u64)&&return0097;
  goto memoryreaddatadecode;
return0097:
  goto vma_memory_read22380;   

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
    goto stack_cache_overflow_check22381;
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
		/* Set return address */
  *((u32 *)(&processor->continuation)+1) = t6;
  /* Update CP */
  t7 = (4096) << 16;   
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = t7 & t1;
  t7 = t7 >> 1;   		// Shift into trace pending place 
  *(u64 *)&processor->continuationcp = t5;   
  t1 = t1 | t7;		// Set the cr.trace pending if appropriate. 
		/* Set the control register */
  *(u32 *)&processor->control = t1;
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

stack_cache_overflow_check22381:
  if (_trace) printf("stack_cache_overflow_check22381:\n");
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

force_alignment22384:
  if (_trace) printf("force_alignment22384:\n");
  if (t3 & 1)   		// J. if apply args 
    goto b_apply_argument_supplied22382;

b_apply_argument_supplied22383:
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

b_apply_argument_supplied22382:
  if (_trace) printf("b_apply_argument_supplied22382:\n");
  t4 = t4 & 63;
  t4 = t4 - Type_NIL;   
  if (t4 != 0)   		// J. if apply args supplied not nil. 
    goto applysupprna;
  t3 = t3 & 1;		// keep just the apply bit! 
  t3 = t3 << 17;   		// reposition the apply bit 
  iSP = iSP - 8;   		// Pop off the null applied arg. 
  arg5 = arg5 & ~t3;		// Blast the apply arg bit away 
		/* Reset the stored cr bit */
  *(u32 *)&processor->control = arg5;
  goto b_apply_argument_supplied22383;   

/* end DoEntryRestNotAccepted */
  /* End of Halfword operand from stack instruction - DoEntryRestNotAccepted */
/* start VerifyGenericArity */


verifygenericarity:
  if (_trace) printf("verifygenericarity:\n");
  t11 = (2) << 16;   
  t11 = t11 & arg2;
  if (t11 == 0) 		// not applying 
    goto verify_generic_arity22385;
  arg1 = zero - arg5;   		// 4 - argsize 
  goto pullapplyargs;   

verify_generic_arity22385:
  if (_trace) printf("verify_generic_arity22385:\n");
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

force_alignment22414:
  if (_trace) printf("force_alignment22414:\n");
  if (t5 == 0) 
    goto basic_dispatch22388;
  /* Here if argument TypeList */
  t5 = t2 - arg5;   		// Stack cache offset 
  t6 = ((u64)t5 < (u64)arg6) ? 1 : 0;   		// In range? 
  t4 = *(u64 *)&(processor->stackcachedata);   
  if (t6 == 0) 		// J. if not in cache 
    goto pull_apply_args22386;
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  t7 = zero;
  t5 = zero + 128;   
  t6 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t5 = t5 + arg1;		// Account for what we're about to push 
  t5 = (t5 * 8) + iSP;  		// SCA of desired end of cache 
  t6 = (arg6 * 8) + t6;  		// SCA of current end of cache 
  t10 = ((s64)t5 <= (s64)t6) ? 1 : 0;   
  if (t10 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check22395;
  iSP = iSP - 8;   		// Pop Stack. 
  goto pull_apply_args_quickly22394;   

pull_apply_args_quickly22389:
  if (_trace) printf("pull_apply_args_quickly22389:\n");
  t9 = *(s32 *)t4;   
  t8 = *(s32 *)(t4 + 4);   
  t9 = (u32)t9;   
  t7 = t7 + 1;
  t4 = t4 + 8;
  t5 = t8 & 192;		// Extract CDR code. 
  if (t5 != 0)   
    goto basic_dispatch22397;
  /* Here if argument 0 */
  t5 = t8 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  t5 = (t7 == arg1) ? 1 : 0;   
  if (t5 == 0) 
    goto pull_apply_args_quickly22389;
  goto pull_apply_args_quickly22390;   

basic_dispatch22397:
  if (_trace) printf("basic_dispatch22397:\n");
  t6 = (t5 == 64) ? 1 : 0;   

force_alignment22409:
  if (_trace) printf("force_alignment22409:\n");
  if (t6 == 0) 
    goto basic_dispatch22398;
  /* Here if argument 64 */
  t5 = t8 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;

pull_apply_args_quickly22392:
  if (_trace) printf("pull_apply_args_quickly22392:\n");
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

basic_dispatch22398:
  if (_trace) printf("basic_dispatch22398:\n");
  t6 = (t5 == 128) ? 1 : 0;   

force_alignment22410:
  if (_trace) printf("force_alignment22410:\n");
  if (t6 == 0) 
    goto basic_dispatch22399;
  /* Here if argument 128 */
  t5 = t8 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  t11 = *(s32 *)t4;   
  t10 = *(s32 *)(t4 + 4);   
  t11 = (u32)t11;   
  t5 = t10 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_List) ? 1 : 0;   

force_alignment22405:
  if (_trace) printf("force_alignment22405:\n");
  if (t6 == 0) 
    goto basic_dispatch22401;
  /* Here if argument TypeList */
  t5 = t11 - arg5;   		// Stack cache offset 
  t6 = ((u64)t5 < (u64)arg6) ? 1 : 0;   		// In range? 
  t4 = *(u64 *)&(processor->stackcachedata);   
  if (t6 == 0) 		// J. if not in cache 
    goto pull_apply_args_quickly22391;
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
  goto pull_apply_args_quickly22394;   

basic_dispatch22401:
  if (_trace) printf("basic_dispatch22401:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment22406:
  if (_trace) printf("force_alignment22406:\n");
  if (t6 == 0) 
    goto basic_dispatch22402;
  /* Here if argument TypeNIL */
  goto pull_apply_args_quickly22392;   

basic_dispatch22402:
  if (_trace) printf("basic_dispatch22402:\n");
  /* Here for all other cases */

pull_apply_args_quickly22391:
  if (_trace) printf("pull_apply_args_quickly22391:\n");
  t5 = t10 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto pull_apply_args_quickly22393;   

basic_dispatch22400:
  if (_trace) printf("basic_dispatch22400:\n");

basic_dispatch22399:
  if (_trace) printf("basic_dispatch22399:\n");
  /* Here for all other cases */
  t7 = t7 - 1;   
  t4 = t4 - 8;   
  goto pull_apply_args_quickly22390;   

basic_dispatch22396:
  if (_trace) printf("basic_dispatch22396:\n");

pull_apply_args_quickly22394:
  t5 = (t7 == arg1) ? 1 : 0;   
  if (t5 == 0) 
    goto pull_apply_args_quickly22389;

pull_apply_args_quickly22390:
  if (_trace) printf("pull_apply_args_quickly22390:\n");
  /* Here if count=n, or bad cdr */
  /* Convert stack cache address to VMA */
  t5 = *(u64 *)&(processor->stackcachedata);   
  t5 = t4 - t5;   		// stack cache base relative offset 
  t5 = t5 >> 3;   		// convert byte address to word address 
  t9 = t5 + arg5;		// reconstruct VMA 
  t5 = Type_List;
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;

pull_apply_args_quickly22393:
  if (_trace) printf("pull_apply_args_quickly22393:\n");
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

basic_dispatch22388:
  if (_trace) printf("basic_dispatch22388:\n");
  t5 = (t4 == Type_NIL) ? 1 : 0;   

force_alignment22415:
  if (_trace) printf("force_alignment22415:\n");
  if (t5 == 0) 
    goto basic_dispatch22411;
  /* Here if argument TypeNIL */
  t6 = *(s32 *)&processor->control;   		// Get the control register 
  t7 = (2) << 16;   
  iSP = iSP - 8;   		// Discard that silly nil 
  t6 = t6 & ~t7;		// Blast away the apply arg bit. 
  *(u32 *)&processor->control = t6;
  goto INTERPRETINSTRUCTION;   

basic_dispatch22411:
  if (_trace) printf("basic_dispatch22411:\n");
  /* Here for all other cases */
  arg1 = arg1;		// Pull apply args trap needs nargs in ARG1 
  goto pullapplyargstrap;

pull_apply_args22386:
  if (_trace) printf("pull_apply_args22386:\n");
  arg1 = arg1;
  goto pullapplyargsslowly;

basic_dispatch22387:
  if (_trace) printf("basic_dispatch22387:\n");

stack_cache_overflow_check22395:
  if (_trace) printf("stack_cache_overflow_check22395:\n");
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

vma_memory_read22416:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22418;

vma_memory_read22417:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22420;

vma_memory_read22427:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t3 = arg5 & 63;		// set CDR-NEXT 
		/* Push the result */
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
  iSP = iSP + 8;
  goto cachevalid;   

vma_memory_read22420:
  if (_trace) printf("vma_memory_read22420:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22419;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22416;   

vma_memory_read22419:
  if (_trace) printf("vma_memory_read22419:\n");

vma_memory_read22418:
  if (_trace) printf("vma_memory_read22418:\n");
  r0 = (u64)&&return0098;
  goto memoryreaddatadecode;
return0098:
  goto vma_memory_read22427;   

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

force_alignment22433:
  if (_trace) printf("force_alignment22433:\n");
  if (t5 == 0) 
    goto basic_dispatch22429;
  /* Here if argument TypeFixnum */
  iPC = t6;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0)   		// T if predicate succeeds 
    t11 = t12;
  *(u64 *)(iSP + 8) = t11;   
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch22429:
  if (_trace) printf("basic_dispatch22429:\n");
  t5 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment22434:
  if (_trace) printf("force_alignment22434:\n");
  if (t5 == 0) 
    goto basic_dispatch22430;
  /* Here if argument TypeSingleFloat */
  iPC = t6;
  *(u64 *)(iSP + 8) = t12;   
  iSP = iSP + 8;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (FLTU64(1, f1) == 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch22430:
  if (_trace) printf("basic_dispatch22430:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch22428:
  if (_trace) printf("basic_dispatch22428:\n");

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
    goto new_aref_1_internal22435;
  t1 = t9 + arg2;

new_aref_1_internal22436:
  if (_trace) printf("new_aref_1_internal22436:\n");
  /* Memory Read Internal */

vma_memory_read22443:
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
    goto vma_memory_read22445;

vma_memory_read22444:
  t5 = zero + 240;   
  t6 = t6 >> (arg3 & 63);   
  t5 = t5 >> (arg3 & 63);   
  t9 = (u32)t9;   
  if (t6 & 1)   
    goto vma_memory_read22447;

vma_memory_read22454:
  if (arg5 != 0)   
    goto new_aref_1_internal22437;

new_aref_1_internal22438:
  if (_trace) printf("new_aref_1_internal22438:\n");
  r31 = r31 | r31;
  t1 = arg6 - 2;   
  if ((s64)t1 <= 0)  
    goto new_aref_1_internal22439;
  /* TagType. */
  arg3 = arg3 & 63;

new_aref_1_internal22440:
  if (_trace) printf("new_aref_1_internal22440:\n");
  *(u32 *)(iSP + 4) = arg3;
  t5 = (arg5 == 0) ? 1 : 0;   
  if (t5 == 0) 
    goto case_others_140;

case_0_134:
  if (_trace) printf("case_0_134:\n");
  r31 = r31 | r31;
  if (t1 == 0) 
    goto new_aref_1_internal22441;
  *(u32 *)iSP = t9;
  goto NEXTINSTRUCTION;   

case_2_135:
  if (_trace) printf("case_2_135:\n");
  /* AREF1-8B */
  r31 = r31 | r31;
  t5 = arg2 & 3;
  t6 = (u8)(t9 >> ((t5&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal22441;
  *(u32 *)iSP = t6;
  goto NEXTINSTRUCTION;   

case_3_136:
  if (_trace) printf("case_3_136:\n");
  /* AREF1-4B */
  r31 = r31 | r31;
  t5 = arg2 & 7;		// byte-index 
  t5 = t5 << 2;   		// byte-position 
  t6 = t9 >> (t5 & 63);   		// byte in position 
  t6 = t6 & 15;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal22441;
  *(u32 *)iSP = t6;
  goto NEXTINSTRUCTION;   

case_5_137:
  if (_trace) printf("case_5_137:\n");
  /* AREF1-1B */
  r31 = r31 | r31;
  t5 = arg2 & 31;		// byte-index 
  r31 = r31 | r31;
  t6 = t9 >> (t5 & 63);   		// byte in position 
  t6 = t6 & 1;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal22441;
  *(u32 *)iSP = t6;
  goto NEXTINSTRUCTION;   

case_1_138:
  if (_trace) printf("case_1_138:\n");
  /* AREF1-16B */
  t5 = arg2 & 1;
  t5 = t5 + t5;		// Bletch, it's a byte ref 
  t6 = (u16)(t9 >> ((t5&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal22441;
  *(u32 *)iSP = t6;
  goto NEXTINSTRUCTION;   

case_others_140:
  if (_trace) printf("case_others_140:\n");
  r31 = r31 | r31;
  t5 = (arg5 == 2) ? 1 : 0;   
  t6 = (arg5 == 3) ? 1 : 0;   
  if (t5 != 0)   
    goto case_2_135;
  t5 = (arg5 == 5) ? 1 : 0;   
  if (t6 != 0)   
    goto case_3_136;
  t6 = (arg5 == 1) ? 1 : 0;   
  if (t5 != 0)   
    goto case_5_137;
  if (t6 != 0)   
    goto case_1_138;

case_4_139:
  if (_trace) printf("case_4_139:\n");
  /* AREF1-2B */
  r31 = r31 | r31;
  t5 = arg2 & 15;		// byte-index 
  t5 = t5 << 1;   		// byte-position 
  t6 = t9 >> (t5 & 63);   		// byte in position 
  t6 = t6 & 3;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal22441;
  *(u32 *)iSP = t6;
  goto NEXTINSTRUCTION;   

new_aref_1_internal22435:
  if (_trace) printf("new_aref_1_internal22435:\n");
  arg2 = arg4 + arg2;
  t1 = arg2 >> (arg5 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  goto new_aref_1_internal22436;   

new_aref_1_internal22437:
  if (_trace) printf("new_aref_1_internal22437:\n");
  t1 = arg3 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto new_aref_1_internal22442;
  goto new_aref_1_internal22438;   

new_aref_1_internal22439:
  if (_trace) printf("new_aref_1_internal22439:\n");
  arg3 = Type_Character;
  if (arg6 & 1)   
    goto new_aref_1_internal22440;
  arg3 = Type_Fixnum;
  if (arg6 == 0) 
    goto new_aref_1_internal22440;
  t2 = *(u64 *)&(processor->niladdress);   
  t3 = *(u64 *)&(processor->taddress);   
  goto new_aref_1_internal22440;   

new_aref_1_internal22441:
  if (_trace) printf("new_aref_1_internal22441:\n");
  if (t6)   
    t2 = t3;
  *(u64 *)iSP = t2;   
  goto NEXTINSTRUCTION;   

new_aref_1_internal22442:
  if (_trace) printf("new_aref_1_internal22442:\n");
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

vma_memory_read22445:
  if (_trace) printf("vma_memory_read22445:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg3 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read22444;   

vma_memory_read22447:
  if (_trace) printf("vma_memory_read22447:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read22446;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read22443;   

vma_memory_read22446:
  if (_trace) printf("vma_memory_read22446:\n");
  t6 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read22451:
  if (_trace) printf("vma_memory_read22451:\n");
  t5 = t6 & MemoryActionTransform;
  if (t5 == 0) 
    goto vma_memory_read22450;
  arg3 = arg3 & ~63L;
  arg3 = arg3 | Type_ExternalValueCellPointer;
  goto vma_memory_read22454;   

vma_memory_read22450:

vma_memory_read22449:
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

force_alignment22455:
  if (_trace) printf("force_alignment22455:\n");
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
		/* Put result back on the stack */
  *(u32 *)iSP = t3;
  goto cachevalid;   

DoPointerPlusIM:
  if (_trace) printf("DoPointerPlusIM:\n");
  t2 = arg2 << 56;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t2 = (s64)t2 >> 56;   

force_alignment22456:
  if (_trace) printf("force_alignment22456:\n");
  t3 = (s32)arg6 + (s32)t2;		// (%32-bit-plus (data arg1) (data arg2)) 
		/* Put result back on the stack */
  *(u32 *)iSP = t3;
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
		/* T8 is TypeFixnum from above */
  *(u32 *)(iSP + 4) = t8;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
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

vma_memory_read22457:
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
    goto vma_memory_read22459;

vma_memory_read22458:
  t8 = t8 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22461;

vma_memory_read22468:
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = arg5;
  goto cachevalid;   

mrnotfixnum:
  if (_trace) printf("mrnotfixnum:\n");
  arg5 = 0;
  arg2 = 5;
  goto illegaloperand;

vma_memory_read22461:
  if (_trace) printf("vma_memory_read22461:\n");

vma_memory_read22459:
  if (_trace) printf("vma_memory_read22459:\n");
  r0 = (u64)&&return0099;
  goto memoryreadgeneraldecode;
return0099:
  goto vma_memory_read22468;   

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
  r0 = (u64)&&return0100;
  goto lookuphandler;
return0100:
  t3 = t4 - Type_EvenPC;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto generic_dispatch22470;
  t3 = t6 & 63;		// Strip CDR code 
  t3 = t3 - Type_NIL;   
  if (t3 == 0) 
    goto generic_dispatch22469;
  *(u32 *)(iFP + 16) = t7;
		/* write the stack cache */
  *(u32 *)(iFP + 20) = t6;

generic_dispatch22469:
  if (_trace) printf("generic_dispatch22469:\n");
  /* Convert real continuation to PC. */
  iPC = t4 & 1;
  iPC = t9 + iPC;
  iPC = t9 + iPC;
  goto interpretinstructionforjump;   

generic_dispatch22470:
  if (_trace) printf("generic_dispatch22470:\n");
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
    goto instance_descriptor_info22474;
  arg2 = arg4;		// Don't clobber instance if it's forwarded 
  /* Memory Read Internal */

vma_memory_read22475:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22477;

vma_memory_read22476:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22479;

vma_memory_read22484:

instance_descriptor_info22473:
  if (_trace) printf("instance_descriptor_info22473:\n");
  arg2 = arg6;
  /* Memory Read Internal */

vma_memory_read22485:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22487;

vma_memory_read22486:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22489;

vma_memory_read22496:
  t2 = arg6;
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto instance_descriptor_info22471;
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read22497:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22499;

vma_memory_read22498:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22501;

vma_memory_read22508:
  t3 = arg6;
  t5 = arg5 - Type_Locative;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto instance_descriptor_info22472;
  arg2 = t2 & t1;
  t5 = arg2 << 1;   
  arg4 = arg2 + t5;		// (* (logand mask data) 3) 
  /* TagType. */
  arg1 = arg1 & 63;

lookup_handler22510:
  if (_trace) printf("lookup_handler22510:\n");
  arg2 = t3 + arg4;
  arg4 = arg4 + 3;
  /* Read key */
  /* Memory Read Internal */

vma_memory_read22511:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22513;

vma_memory_read22512:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22515;

vma_memory_read22522:
  /* TagType. */
  arg5 = arg5 & 63;
  t5 = (arg5 == Type_NIL) ? 1 : 0;   
  if (t5 != 0)   
    goto lookup_handler22509;
  t5 = (arg1 == arg5) ? 1 : 0;   
  if (t5 == 0) 
    goto lookup_handler22510;
  t5 = (s32)t1 - (s32)arg6;   
  if (t5 != 0)   
    goto lookup_handler22510;

lookup_handler22509:
  if (_trace) printf("lookup_handler22509:\n");
  /* Read method */
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read22523:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22525;

vma_memory_read22524:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22527;

vma_memory_read22534:
  t4 = arg5;
  arg3 = arg6;
  /* Read parameter */
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read22535:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22537;

vma_memory_read22536:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22539;

vma_memory_read22546:
  t6 = arg5;
  t7 = arg6;
  t9 = arg3;
  sp = sp + 8;   
  goto *r0; /* ret */

vma_memory_read22539:
  if (_trace) printf("vma_memory_read22539:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22538;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22535;   

vma_memory_read22538:
  if (_trace) printf("vma_memory_read22538:\n");

vma_memory_read22537:
  if (_trace) printf("vma_memory_read22537:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0101;
  goto memoryreaddatadecode;
return0101:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22546;   

vma_memory_read22527:
  if (_trace) printf("vma_memory_read22527:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22526;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22523;   

vma_memory_read22526:
  if (_trace) printf("vma_memory_read22526:\n");

vma_memory_read22525:
  if (_trace) printf("vma_memory_read22525:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0102;
  goto memoryreaddatadecode;
return0102:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22534;   

vma_memory_read22515:
  if (_trace) printf("vma_memory_read22515:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22514;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22511;   

vma_memory_read22514:
  if (_trace) printf("vma_memory_read22514:\n");

vma_memory_read22513:
  if (_trace) printf("vma_memory_read22513:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0103;
  goto memoryreaddatadecode;
return0103:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22522;   

vma_memory_read22501:
  if (_trace) printf("vma_memory_read22501:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22500;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22497;   

vma_memory_read22500:
  if (_trace) printf("vma_memory_read22500:\n");

vma_memory_read22499:
  if (_trace) printf("vma_memory_read22499:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0104;
  goto memoryreaddatadecode;
return0104:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22508;   

vma_memory_read22489:
  if (_trace) printf("vma_memory_read22489:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22488;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22485;   

vma_memory_read22488:
  if (_trace) printf("vma_memory_read22488:\n");

vma_memory_read22487:
  if (_trace) printf("vma_memory_read22487:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0105;
  goto memoryreaddatadecode;
return0105:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22496;   

vma_memory_read22479:
  if (_trace) printf("vma_memory_read22479:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22478;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22475;   

vma_memory_read22478:
  if (_trace) printf("vma_memory_read22478:\n");

vma_memory_read22477:
  if (_trace) printf("vma_memory_read22477:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0106;
  goto memoryreadheaderdecode;
return0106:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22484;   

instance_descriptor_info22474:
  if (_trace) printf("instance_descriptor_info22474:\n");
  /* not an instance, flavor description comes from magic vector */
  arg2 = *(u64 *)&(processor->trapvecbase);   
  /* TagType. */
  t5 = arg3 & 63;
  arg2 = arg2 + 2560;   
  arg2 = t5 + arg2;
  /* Memory Read Internal */

vma_memory_read22547:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22549;

vma_memory_read22548:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22551;
  goto instance_descriptor_info22473;   

instance_descriptor_info22471:
  if (_trace) printf("instance_descriptor_info22471:\n");
  arg5 = arg2;
  arg2 = 34;
  goto illegaloperand;

instance_descriptor_info22472:
  if (_trace) printf("instance_descriptor_info22472:\n");
  arg5 = arg2;
  arg2 = 35;
  goto illegaloperand;

vma_memory_read22551:
  if (_trace) printf("vma_memory_read22551:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22550;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22547;   

vma_memory_read22550:
  if (_trace) printf("vma_memory_read22550:\n");

vma_memory_read22549:
  if (_trace) printf("vma_memory_read22549:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0107;
  goto memoryreaddatadecode;
return0107:
  r0 = *(u64 *)sp;   
  goto instance_descriptor_info22473;   

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
		/* Set TAG of op1 */
  *(u32 *)(iSP + 4) = arg2;
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
  r0 = (u64)&&return0108;
  goto carinternal;
return0108:
  t5 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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

force_alignment22578:
  if (_trace) printf("force_alignment22578:\n");
  if (t6 == 0) 
    goto basic_dispatch22561;
  /* Here if argument TypeList */

car_internal22558:
  /* Memory Read Internal */

vma_memory_read22562:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22564;

vma_memory_read22563:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22566;

vma_memory_read22573:

basic_dispatch22560:
  if (_trace) printf("basic_dispatch22560:\n");

car_internal22559:
  if (_trace) printf("car_internal22559:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch22561:
  if (_trace) printf("basic_dispatch22561:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment22579:
  if (_trace) printf("force_alignment22579:\n");
  if (t6 != 0)   
    goto basic_dispatch22560;

basic_dispatch22574:
  if (_trace) printf("basic_dispatch22574:\n");
  t6 = (t5 == Type_Locative) ? 1 : 0;   

force_alignment22580:
  if (_trace) printf("force_alignment22580:\n");
  if (t6 != 0)   
    goto car_internal22558;

basic_dispatch22575:
  if (_trace) printf("basic_dispatch22575:\n");
  /* Here for all other cases */
  arg6 = arg5;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

vma_memory_read22566:
  if (_trace) printf("vma_memory_read22566:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22565;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22562;   

vma_memory_read22565:
  if (_trace) printf("vma_memory_read22565:\n");

vma_memory_read22564:
  if (_trace) printf("vma_memory_read22564:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0109;
  goto memoryreaddatadecode;
return0109:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22573;   

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
  r0 = (u64)&&return0110;
  goto cdrinternal;
return0110:
  t5 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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

force_alignment22616:
  if (_trace) printf("force_alignment22616:\n");
  if (t6 == 0) 
    goto basic_dispatch22584;
  /* Here if argument TypeList */
  /* Memory Read Internal */

vma_memory_read22585:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22587;

vma_memory_read22586:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22589;

vma_memory_read22594:
  t5 = arg5 & 192;		// Extract CDR code. 
  if (t5 != 0)   
    goto basic_dispatch22596;
  /* Here if argument 0 */
  arg6 = arg2 + 1;		// Address of next position is CDR 
  arg5 = Type_List;

basic_dispatch22595:
  if (_trace) printf("basic_dispatch22595:\n");

basic_dispatch22583:
  if (_trace) printf("basic_dispatch22583:\n");

cdr_internal22582:
  if (_trace) printf("cdr_internal22582:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch22584:
  if (_trace) printf("basic_dispatch22584:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment22617:
  if (_trace) printf("force_alignment22617:\n");
  if (t6 != 0)   
    goto basic_dispatch22583;

basic_dispatch22612:
  if (_trace) printf("basic_dispatch22612:\n");
  t6 = (t5 == Type_Locative) ? 1 : 0;   

force_alignment22618:
  if (_trace) printf("force_alignment22618:\n");
  if (t6 != 0)   
    goto cdr_internal22581;

basic_dispatch22613:
  if (_trace) printf("basic_dispatch22613:\n");
  /* Here for all other cases */
  arg6 = arg5;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

basic_dispatch22596:
  if (_trace) printf("basic_dispatch22596:\n");
  t6 = (t5 == 128) ? 1 : 0;   

force_alignment22619:
  if (_trace) printf("force_alignment22619:\n");
  if (t6 == 0) 
    goto basic_dispatch22597;
  /* Here if argument 128 */
  arg2 = arg2 + 1;

cdr_internal22581:
  if (_trace) printf("cdr_internal22581:\n");
  /* Memory Read Internal */

vma_memory_read22598:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22600;

vma_memory_read22599:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22602;
  goto cdr_internal22582;   

basic_dispatch22597:
  if (_trace) printf("basic_dispatch22597:\n");
  t6 = (t5 == 64) ? 1 : 0;   

force_alignment22620:
  if (_trace) printf("force_alignment22620:\n");
  if (t6 == 0) 
    goto basic_dispatch22609;
  /* Here if argument 64 */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto cdr_internal22582;   

basic_dispatch22609:
  if (_trace) printf("basic_dispatch22609:\n");
  /* Here for all other cases */
  arg5 = arg2;
  arg2 = 15;
  goto illegaloperand;

vma_memory_read22602:
  if (_trace) printf("vma_memory_read22602:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22601;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22598;   

vma_memory_read22601:
  if (_trace) printf("vma_memory_read22601:\n");

vma_memory_read22600:
  if (_trace) printf("vma_memory_read22600:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0111;
  goto memoryreaddatadecode;
return0111:
  r0 = *(u64 *)sp;   
  goto cdr_internal22582;   

vma_memory_read22589:
  if (_trace) printf("vma_memory_read22589:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22588;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22585;   

vma_memory_read22588:
  if (_trace) printf("vma_memory_read22588:\n");

vma_memory_read22587:
  if (_trace) printf("vma_memory_read22587:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0112;
  goto memoryreadcdrdecode;
return0112:
  r0 = *(u64 *)sp;   
  goto vma_memory_read22594;   

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
    goto internal_register_dispatch22621;
  t3 = arg1 & 63;		// Keep only six bits 
  t2 = ((s64)t3 <= (s64)42) ? 1 : 0;   		// In range for the low registers? 
  t3 = (t3 * 8) + t1;  
  if (t2 == 0) 
    goto ReadRegisterError;
  t3 = *(u64 *)t3;   
    goto *t3; /* jmp */   		// Jump to the handler 

internal_register_dispatch22621:
  if (_trace) printf("internal_register_dispatch22621:\n");
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
    goto internal_register_dispatch22622;
  t3 = arg1 & 63;		// Keep only six bits 
  t2 = ((s64)t3 <= (s64)42) ? 1 : 0;   		// In range for the low registers? 
  t3 = (t3 * 8) + t1;  
  if (t2 == 0) 
    goto WriteRegisterError;
  t3 = *(u64 *)t3;   
    goto *t3; /* jmp */   		// Jump to the handler 

internal_register_dispatch22622:
  if (_trace) printf("internal_register_dispatch22622:\n");
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

vma_memory_read22626:
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
    goto vma_memory_read22628;

vma_memory_read22627:
  t8 = t8 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22630;

vma_memory_read22637:
  if (t4 != 0)   		// J. if we have to test for fixnump. 
    goto i_block_n_read22623;

i_block_n_read22624:
  t4 = arg2 + 1;		// Compute Incremented address 

force_alignment22638:
  if (_trace) printf("force_alignment22638:\n");
  if (t2 == 0)   		// Conditionally update address 
    arg2 = t4;
		/* Store updated vma in BAR */
  *(u32 *)arg4 = arg2;
  t2 = arg5 & 63;		// Compute CDR-NEXT 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (t3)   		// Conditionally Set CDR-NEXT 
    arg5 = t2;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = arg5;
  iSP = iSP + 8;
  goto cachevalid;   

i_block_n_read22625:
  if (_trace) printf("i_block_n_read22625:\n");
  arg5 = arg2;
  arg2 = 23;
  goto illegaloperand;

vma_memory_read22630:
  if (_trace) printf("vma_memory_read22630:\n");

vma_memory_read22628:
  if (_trace) printf("vma_memory_read22628:\n");
  r0 = (u64)&&return0113;
  goto memoryreadgeneraldecode;
return0113:
  goto vma_memory_read22637;   

i_block_n_read22623:
  if (_trace) printf("i_block_n_read22623:\n");
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto i_block_n_read22625;
  goto i_block_n_read22624;   

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

force_alignment22639:
  if (_trace) printf("force_alignment22639:\n");
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

force_alignment22643:
  if (_trace) printf("force_alignment22643:\n");
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

force_alignment22642:
  if (_trace) printf("force_alignment22642:\n");
  t5 = t5 | t4;
  STQ_U(t8, t5);   
  *(u32 *)t6 = t3;
  if (t7 != 0)   		// J. if in cache 
    goto vma_memory_write22641;

vma_memory_write22640:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  arg3 = arg3 + 1;		// Increment the address 
		/* Store updated vma in BAR */
  *(u32 *)arg2 = arg3;
  goto cachevalid;   

vma_memory_write22641:
  if (_trace) printf("vma_memory_write22641:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t4 = arg3 - t11;   		// Stack cache offset 
  t8 = (t4 * 8) + t8;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t8 = t3;
		/* write the stack cache */
  *(u32 *)(t8 + 4) = t2;
  goto vma_memory_write22640;   

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

start_call_dispatch22646:
  if (_trace) printf("start_call_dispatch22646:\n");
  t1 = *(u64 *)&(processor->trapvecbase);   
  t2 = arg5 & 63;		// Strip off any CDR code bits. 
  t3 = (t2 == Type_CompiledFunction) ? 1 : 0;   

force_alignment22694:
  if (_trace) printf("force_alignment22694:\n");
  if (t3 == 0) 
    goto basic_dispatch22651;
  /* Here if argument TypeCompiledFunction */

start_call_dispatch22647:
  if (_trace) printf("start_call_dispatch22647:\n");
  arg3 = zero;		// No extra argument 

start_call_dispatch22648:
  if (_trace) printf("start_call_dispatch22648:\n");
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
		/* push continuation */
  *(u32 *)(iSP + -8) = t8;
		/* write the stack cache */
  *(u32 *)(iSP + -4) = t7;
  t8 = t3 | t5;		// Set call started bit in CR 
  t5 = zero + 256;   
		/* Push control register */
  *(u32 *)iSP = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t6;
  t8 = t8 & ~t5;		// Clear the extra arg bit 
		/* Save control with new state */
  *(u32 *)&processor->control = t8;
  /* End of push-frame */
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)&processor->continuation = arg6;
  *((u32 *)(&processor->continuation)+1) = arg5;
  *(u64 *)&processor->continuationcp = zero;   
  if (arg3 != 0)   
    goto start_call_dispatch22649;
  goto cachevalid;   

start_call_dispatch22649:
  if (_trace) printf("start_call_dispatch22649:\n");
  t1 = *(s32 *)&processor->control;   
  t2 = zero + 256;   
  t3 = arg3 & 63;		// set CDR-NEXT 
		/* Push the extra arg. */
  *(u32 *)(iSP + 8) = arg4;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
  iSP = iSP + 8;
  t1 = t1 | t2;		// Set the extra arg bit 
		/* Save control with new state */
  *(u32 *)&processor->control = t1;
  goto cachevalid;   

basic_dispatch22651:
  if (_trace) printf("basic_dispatch22651:\n");
  t3 = (t2 == Type_GenericFunction) ? 1 : 0;   

force_alignment22695:
  if (_trace) printf("force_alignment22695:\n");
  if (t3 == 0) 
    goto basic_dispatch22652;
  /* Here if argument TypeGenericFunction */
  arg3 = arg5;
  arg4 = (u32)arg6;   
  arg6 = t1 + 2636;   
  goto start_call_dispatch22648;   

basic_dispatch22652:
  if (_trace) printf("basic_dispatch22652:\n");
  t3 = (t2 == Type_Instance) ? 1 : 0;   

force_alignment22696:
  if (_trace) printf("force_alignment22696:\n");
  if (t3 == 0) 
    goto basic_dispatch22653;
  /* Here if argument TypeInstance */
  arg3 = arg5;
  arg4 = (u32)arg6;   
  arg6 = t1 + 2638;   
  goto start_call_dispatch22648;   

basic_dispatch22653:
  if (_trace) printf("basic_dispatch22653:\n");
  t3 = (t2 == Type_Symbol) ? 1 : 0;   

force_alignment22697:
  if (_trace) printf("force_alignment22697:\n");
  if (t3 == 0) 
    goto basic_dispatch22654;
  /* Here if argument TypeSymbol */
  arg6 = (u32)arg6;   
  arg3 = zero;		// No extra argument 
  arg2 = arg6 + 2;		// Get to the function cell 
  goto startcallindirect;   

basic_dispatch22654:
  if (_trace) printf("basic_dispatch22654:\n");
  t3 = (t2 == Type_LexicalClosure) ? 1 : 0;   

force_alignment22698:
  if (_trace) printf("force_alignment22698:\n");
  if (t3 == 0) 
    goto basic_dispatch22655;
  /* Here if argument TypeLexicalClosure */
  arg2 = (u32)arg6;   
  /* Memory Read Internal */

vma_memory_read22656:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22658;

vma_memory_read22657:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22660;

vma_memory_read22667:
  arg3 = arg5;
  arg4 = arg6;
  arg2 = arg2 + 1;

startcallindirect:
  if (_trace) printf("startcallindirect:\n");
  /* Memory Read Internal */

vma_memory_read22668:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22670;

vma_memory_read22669:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22672;

vma_memory_read22679:
  t5 = arg5 - Type_CompiledFunction;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto start_call_dispatch22646;
  goto start_call_dispatch22648;   

basic_dispatch22655:
  if (_trace) printf("basic_dispatch22655:\n");
  /* Here for all other cases */

start_call_dispatch22644:
  if (_trace) printf("start_call_dispatch22644:\n");
  arg3 = arg5;
  arg4 = arg6;
  t3 = t1 + 2304;   
  /* TagType. */
  arg5 = arg5 & 63;
  arg2 = arg5 + t3;
  /* Memory Read Internal */

vma_memory_read22681:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22683;

vma_memory_read22682:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22685;

vma_memory_read22692:
  t3 = arg5 - Type_EvenPC;   
  t3 = t3 & 63;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto start_call_dispatch22645;
  goto start_call_dispatch22648;   

basic_dispatch22650:
  if (_trace) printf("basic_dispatch22650:\n");

start_call_dispatch22645:
  if (_trace) printf("start_call_dispatch22645:\n");
  arg5 = t1;
  arg2 = 51;
  goto illegaloperand;

vma_memory_read22685:
  if (_trace) printf("vma_memory_read22685:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22684;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22681;   

vma_memory_read22684:
  if (_trace) printf("vma_memory_read22684:\n");

vma_memory_read22683:
  if (_trace) printf("vma_memory_read22683:\n");
  r0 = (u64)&&return0114;
  goto memoryreaddatadecode;
return0114:
  goto vma_memory_read22692;   

vma_memory_read22672:
  if (_trace) printf("vma_memory_read22672:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22671;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22668;   

vma_memory_read22671:
  if (_trace) printf("vma_memory_read22671:\n");

vma_memory_read22670:
  if (_trace) printf("vma_memory_read22670:\n");
  r0 = (u64)&&return0115;
  goto memoryreaddatadecode;
return0115:
  goto vma_memory_read22679;   

vma_memory_read22660:
  if (_trace) printf("vma_memory_read22660:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22659;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22656;   

vma_memory_read22659:
  if (_trace) printf("vma_memory_read22659:\n");

vma_memory_read22658:
  if (_trace) printf("vma_memory_read22658:\n");
  r0 = (u64)&&return0116;
  goto memoryreaddatadecode;
return0116:
  goto vma_memory_read22667;   

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

vma_memory_read22702:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22704;

vma_memory_read22703:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22706;

vma_memory_read22711:
  arg6 = arg6 & Array_LengthMask;
  t3 = arg6 - arg1;   
  if ((s64)t3 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg2 = arg2 + arg1;
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read22712:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22714;

vma_memory_read22713:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22716;

vma_memory_read22723:
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
    goto locate_instance0variable_mapped22701;

locate_instance0variable_mapped22700:
  if (_trace) printf("locate_instance0variable_mapped22700:\n");
  arg2 = arg2 + t1;

locate_instance0variable_mapped22699:
  if (_trace) printf("locate_instance0variable_mapped22699:\n");
  /* Memory Read Internal */

vma_memory_read22724:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22726;

vma_memory_read22725:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22728;

vma_memory_read22735:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = arg5 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t7;
  iSP = iSP + 8;
  goto cachevalid;   

vma_memory_read22728:
  if (_trace) printf("vma_memory_read22728:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22727;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22724;   

vma_memory_read22727:
  if (_trace) printf("vma_memory_read22727:\n");

vma_memory_read22726:
  if (_trace) printf("vma_memory_read22726:\n");
  r0 = (u64)&&return0117;
  goto memoryreaddatadecode;
return0117:
  goto vma_memory_read22735;   

vma_memory_read22716:
  if (_trace) printf("vma_memory_read22716:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22715;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22712;   

vma_memory_read22715:
  if (_trace) printf("vma_memory_read22715:\n");

vma_memory_read22714:
  if (_trace) printf("vma_memory_read22714:\n");
  r0 = (u64)&&return0118;
  goto memoryreaddatadecode;
return0118:
  goto vma_memory_read22723;   

vma_memory_read22706:
  if (_trace) printf("vma_memory_read22706:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22705;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22702;   

vma_memory_read22705:
  if (_trace) printf("vma_memory_read22705:\n");

vma_memory_read22704:
  if (_trace) printf("vma_memory_read22704:\n");
  r0 = (u64)&&return0119;
  goto memoryreadheaderdecode;
return0119:
  goto vma_memory_read22711;   

locate_instance0variable_mapped22701:
  if (_trace) printf("locate_instance0variable_mapped22701:\n");
  t3 = arg2;
  /* Memory Read Internal */

vma_memory_read22736:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22738;

vma_memory_read22737:
  t7 = zero + 64;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22740;

vma_memory_read22745:
  t3 = t3 - arg2;   
  if (t3 != 0)   
    goto locate_instance0variable_mapped22700;
  /* TagType. */
  t4 = t4 & 63;
  t4 = t4 | 64;		// Set CDR code to 1 
		/* Update self */
  *(u32 *)(iFP + 24) = arg2;
		/* write the stack cache */
  *(u32 *)(iFP + 28) = t4;
  goto locate_instance0variable_mapped22700;   

vma_memory_read22740:
  if (_trace) printf("vma_memory_read22740:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22739;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22736;   

vma_memory_read22739:
  if (_trace) printf("vma_memory_read22739:\n");

vma_memory_read22738:
  if (_trace) printf("vma_memory_read22738:\n");
  r0 = (u64)&&return0120;
  goto memoryreadheaderdecode;
return0120:
  goto vma_memory_read22745;   

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

force_alignment22785:
  if (_trace) printf("force_alignment22785:\n");
  if (t10 == 0) 
    goto basic_dispatch22756;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22762:
  if (_trace) printf("force_alignment22762:\n");
  if (t12 == 0) 
    goto basic_dispatch22758;
  /* Here if argument TypeFixnum */
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = (u64)((s32)t2 + (s64)(s32)t4); 		// compute 64-bit result 
  if (t5 >> 32)
    exception();  /* addl/v */
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
		/* Semi-cheat, we know temp2 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 4) = t9;
  iPC = t6;
  *(u32 *)iSP = t5;
  iCP = t7;
  goto cachevalid;   

basic_dispatch22758:
  if (_trace) printf("basic_dispatch22758:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment22763:
  if (_trace) printf("force_alignment22763:\n");
  if (t12 == 0) 
    goto basic_dispatch22759;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation22746;   

basic_dispatch22759:
  if (_trace) printf("basic_dispatch22759:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22764:
  if (_trace) printf("force_alignment22764:\n");
  if (t12 == 0) 
    goto binary_type_dispatch22753;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation22749;   

basic_dispatch22757:
  if (_trace) printf("basic_dispatch22757:\n");

basic_dispatch22756:
  if (_trace) printf("basic_dispatch22756:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment22786:
  if (_trace) printf("force_alignment22786:\n");
  if (t10 == 0) 
    goto basic_dispatch22765;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment22771:
  if (_trace) printf("force_alignment22771:\n");
  if (t12 == 0) 
    goto basic_dispatch22767;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation22746:
  if (_trace) printf("simple_binary_arithmetic_operation22746:\n");
  ADDS(0, f0, 1, f1, 2, f2); /* adds */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  goto cachevalid;   

basic_dispatch22767:
  if (_trace) printf("basic_dispatch22767:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22772:
  if (_trace) printf("force_alignment22772:\n");
  if (t12 == 0) 
    goto basic_dispatch22768;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation22746;   

basic_dispatch22768:
  if (_trace) printf("basic_dispatch22768:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22773:
  if (_trace) printf("force_alignment22773:\n");
  if (t12 == 0) 
    goto binary_type_dispatch22753;
  /* Here if argument TypeDoubleFloat */

simple_binary_arithmetic_operation22749:
  if (_trace) printf("simple_binary_arithmetic_operation22749:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto simple_binary_arithmetic_operation22750;   

basic_dispatch22766:
  if (_trace) printf("basic_dispatch22766:\n");

basic_dispatch22765:
  if (_trace) printf("basic_dispatch22765:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22787:
  if (_trace) printf("force_alignment22787:\n");
  if (t10 == 0) 
    goto basic_dispatch22774;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22780:
  if (_trace) printf("force_alignment22780:\n");
  if (t12 == 0) 
    goto basic_dispatch22776;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0121;
  goto fetchdoublefloat;
return0121:
  LDT(1, f1, processor->fp0);   

simple_binary_arithmetic_operation22750:
  if (_trace) printf("simple_binary_arithmetic_operation22750:\n");
  arg2 = (u32)t4;   
  r0 = (u64)&&return0122;
  goto fetchdoublefloat;
return0122:
  LDT(2, f2, processor->fp0);   

simple_binary_arithmetic_operation22747:
  if (_trace) printf("simple_binary_arithmetic_operation22747:\n");
  ADDT(0, f0, 1, f1, 2, f2); /* addt */   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0123;
  goto consdoublefloat;
return0123:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  goto cachevalid;   

basic_dispatch22776:
  if (_trace) printf("basic_dispatch22776:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment22781:
  if (_trace) printf("force_alignment22781:\n");
  if (t12 == 0) 
    goto basic_dispatch22777;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation22748:
  if (_trace) printf("simple_binary_arithmetic_operation22748:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0124;
  goto fetchdoublefloat;
return0124:
  LDT(1, f1, processor->fp0);   
  goto simple_binary_arithmetic_operation22747;   

basic_dispatch22777:
  if (_trace) printf("basic_dispatch22777:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22782:
  if (_trace) printf("force_alignment22782:\n");
  if (t12 == 0) 
    goto binary_type_dispatch22753;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation22748;   

basic_dispatch22775:
  if (_trace) printf("basic_dispatch22775:\n");

basic_dispatch22774:
  if (_trace) printf("basic_dispatch22774:\n");
  /* Here for all other cases */

binary_type_dispatch22752:
  if (_trace) printf("binary_type_dispatch22752:\n");

doaddovfl:
  if (_trace) printf("doaddovfl:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch22754;   

binary_type_dispatch22753:
  if (_trace) printf("binary_type_dispatch22753:\n");
  t1 = t3;
  goto doaddovfl;   

binary_type_dispatch22754:
  if (_trace) printf("binary_type_dispatch22754:\n");

basic_dispatch22755:
  if (_trace) printf("basic_dispatch22755:\n");

DoAddIM:
  if (_trace) printf("DoAddIM:\n");
  t1 = (u32)(arg6 >> ((4&7)*8));   
  t2 = (s32)arg6;		// get ARG1 tag/data 
  t11 = t1 & 63;		// Strip off any CDR code bits. 
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22792:
  if (_trace) printf("force_alignment22792:\n");
  if (t12 == 0) 
    goto basic_dispatch22789;
  /* Here if argument TypeFixnum */
  t3 = t2 + arg2;		// compute 64-bit result 
  t4 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t10 = (s32)t3;		// compute 32-bit sign-extended result 
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t10 = (t3 == t10) ? 1 : 0;   		// is it the same as the 64-bit result? 
  if (t10 == 0) 		// if not, we overflowed 
    goto doaddovfl;
		/* Semi-cheat, we know temp2 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 4) = t11;
  iPC = t4;
  *(u32 *)iSP = t3;
  iCP = t5;
  goto cachevalid;   

basic_dispatch22789:
  if (_trace) printf("basic_dispatch22789:\n");
  /* Here for all other cases */
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindoadd;   

basic_dispatch22788:
  if (_trace) printf("basic_dispatch22788:\n");

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

force_alignment22793:
  if (_trace) printf("force_alignment22793:\n");
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

force_alignment22804:
  if (_trace) printf("force_alignment22804:\n");
  if (t8 == 0) 
    goto basic_dispatch22800;
  /* Here if argument ArrayElementTypeCharacter */
  t2 = t1 - Type_Character;   
  if (t2 == 0) 
    goto aset_1_internal22795;
  arg5 = 0;
  arg2 = 29;
  goto illegaloperand;

aset_1_internal22795:
  if (_trace) printf("aset_1_internal22795:\n");
  if (arg5 == 0) 		// Certainly will fit if not packed! 
    goto aset_1_internal22794;
  t2 = 32;
  t2 = t2 >> (arg5 & 63);   		// Compute size of byte 
  t1 = ~zero;   
  t1 = t1 << (t2 & 63);   
  t1 = ~t1;   		// Compute mask for byte 
  t1 = t6 & t1;
  t1 = t6 - t1;   
  if (t1 == 0) 		// J. if character fits. 
    goto aset_1_internal22794;
  arg5 = 0;
  arg2 = 62;
  goto illegaloperand;

basic_dispatch22800:
  if (_trace) printf("basic_dispatch22800:\n");
  t8 = (arg6 == Array_ElementTypeFixnum) ? 1 : 0;   

force_alignment22805:
  if (_trace) printf("force_alignment22805:\n");
  if (t8 == 0) 
    goto basic_dispatch22801;
  /* Here if argument ArrayElementTypeFixnum */
  t2 = t1 - Type_Fixnum;   
  if (t2 == 0) 
    goto aset_1_internal22794;
  arg5 = 0;
  arg2 = 33;
  goto illegaloperand;

basic_dispatch22801:
  if (_trace) printf("basic_dispatch22801:\n");
  t8 = (arg6 == Array_ElementTypeBoolean) ? 1 : 0;   

force_alignment22806:
  if (_trace) printf("force_alignment22806:\n");
  if (t8 == 0) 
    goto basic_dispatch22799;
  /* Here if argument ArrayElementTypeBoolean */
  t6 = 1;
  t1 = t1 - Type_NIL;   
  if (t1 != 0)   		// J. if True 
    goto aset_1_internal22794;
  t6 = zero;
  goto aset_1_internal22794;   		// J. if False 

basic_dispatch22799:
  if (_trace) printf("basic_dispatch22799:\n");
  /* Shove it in. */

aset_1_internal22794:
  if (_trace) printf("aset_1_internal22794:\n");
  if (arg5 != 0)   		// J. if packed 
    goto aset_1_internal22796;
  t1 = arg6 - Array_ElementTypeObject;   
  if (t1 != 0)   
    goto aset_1_internal22796;
  /* Here for the simple non packed case */
  t1 = t9 + arg2;
  /* Memory Read Internal */

vma_memory_read22807:
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
    goto vma_memory_read22809;

vma_memory_read22808:
  t8 = zero + 240;   
  arg1 = arg1 >> (t2 & 63);   
  t8 = t8 >> (t2 & 63);   
  if (arg1 & 1)   
    goto vma_memory_read22811;

vma_memory_read22817:
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

force_alignment22819:
  if (_trace) printf("force_alignment22819:\n");
  t8 = t8 | t7;
  STQ_U(t4, t8);   
  *(u32 *)t3 = t6;
  if (arg1 != 0)   		// J. if in cache 
    goto vma_memory_write22818;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   
  /* Here for the slow packed version */

aset_1_internal22796:
  if (_trace) printf("aset_1_internal22796:\n");
  arg2 = arg4 + arg2;
  t1 = arg2 >> (arg5 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  /* Memory Read Internal */

vma_memory_read22820:
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
    goto vma_memory_read22822;

vma_memory_read22821:
  t4 = zero + 240;   
  t7 = t7 >> (arg3 & 63);   
  t4 = t4 >> (arg3 & 63);   
  t9 = (u32)t9;   
  if (t7 & 1)   
    goto vma_memory_read22824;

vma_memory_read22831:
  /* Check fixnum element type */
  /* TagType. */
  t2 = arg3 & 63;
  t2 = t2 - Type_Fixnum;   
  if (t2 != 0)   		// J. if element type not fixnum. 
    goto aset_1_internal22797;
  if (arg5 == 0) 		// J. if unpacked fixnum element type. 
    goto aset_1_internal22798;
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
    goto array_element_dpb22832;
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
  goto array_element_dpb22833;   

array_element_dpb22832:
  if (_trace) printf("array_element_dpb22832:\n");
  /* Inserting the byte into the low byte */
  t9 = t9 & t3;		// Remove the old low byte 
  t8 = t6 & t4;		// Remove unwanted bits from the new byte 
  t9 = t9 | t8;		// Insert the new byte in place of the old byte 

array_element_dpb22833:
  if (_trace) printf("array_element_dpb22833:\n");
  t6 = t9;

aset_1_internal22798:
  if (_trace) printf("aset_1_internal22798:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = t1 + ivory;
  t8 = *(s32 *)&processor->scovlimit;   
  t7 = (t2 * 4);   
  t4 = LDQ_U(t2);   
  t3 = t1 - t3;   		// Stack cache offset 
  t8 = ((u64)t3 < (u64)t8) ? 1 : 0;   		// In range? 
  t3 = (arg3 & 0xff) << ((t2&7)*8);   
  t4 = t4 & ~(0xffL << (t2&7)*8);   

force_alignment22835:
  if (_trace) printf("force_alignment22835:\n");
  t4 = t4 | t3;
  STQ_U(t2, t4);   
  *(u32 *)t7 = t6;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write22834;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

aset_1_internal22797:
  if (_trace) printf("aset_1_internal22797:\n");
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

vma_memory_write22834:
  if (_trace) printf("vma_memory_write22834:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment22836:
  if (_trace) printf("force_alignment22836:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t3 = t1 - t3;   		// Stack cache offset 
  t2 = (t3 * 8) + t2;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t2 = t6;
		/* write the stack cache */
  *(u32 *)(t2 + 4) = arg3;
  goto NEXTINSTRUCTION;   

vma_memory_read22822:
  if (_trace) printf("vma_memory_read22822:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg3 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read22821;   

vma_memory_read22824:
  if (_trace) printf("vma_memory_read22824:\n");
  if ((t4 & 1) == 0)   
    goto vma_memory_read22823;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read22820;   

vma_memory_read22823:
  if (_trace) printf("vma_memory_read22823:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t4 = arg3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t4 = (t4 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t4;   		// Get the memory action 

vma_memory_read22828:
  if (_trace) printf("vma_memory_read22828:\n");
  t4 = t7 & MemoryActionTransform;
  if (t4 == 0) 
    goto vma_memory_read22827;
  arg3 = arg3 & ~63L;
  arg3 = arg3 | Type_ExternalValueCellPointer;
  goto vma_memory_read22831;   

vma_memory_read22827:

vma_memory_read22826:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_write22818:
  if (_trace) printf("vma_memory_write22818:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment22837:
  if (_trace) printf("force_alignment22837:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - t7;   		// Stack cache offset 
  t4 = (t7 * 8) + t4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t4 = t6;
		/* write the stack cache */
  *(u32 *)(t4 + 4) = t2;
  goto NEXTINSTRUCTION;   

vma_memory_read22809:
  if (_trace) printf("vma_memory_read22809:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read22808;   

vma_memory_read22811:
  if (_trace) printf("vma_memory_read22811:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read22810;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read22807;   

vma_memory_read22810:
  if (_trace) printf("vma_memory_read22810:\n");
  arg1 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + arg1;   		// Adjust for a longword load 
  arg1 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read22814:

vma_memory_read22813:
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
    goto new_aref_1_internal22838;
  t1 = t9 + arg4;

new_aref_1_internal22839:
  if (_trace) printf("new_aref_1_internal22839:\n");
  /* Memory Read Internal */

vma_memory_read22846:
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
    goto vma_memory_read22848;

vma_memory_read22847:
  t4 = zero + 240;   
  t5 = t5 >> (arg5 & 63);   
  t4 = t4 >> (arg5 & 63);   
  t9 = (u32)t9;   
  if (t5 & 1)   
    goto vma_memory_read22850;

vma_memory_read22857:
  if (t6 != 0)   
    goto new_aref_1_internal22840;

new_aref_1_internal22841:
  if (_trace) printf("new_aref_1_internal22841:\n");
  r31 = r31 | r31;
  t1 = t8 - 2;   
  if ((s64)t1 <= 0)  
    goto new_aref_1_internal22842;
  /* TagType. */
  arg5 = arg5 & 63;

new_aref_1_internal22843:
  if (_trace) printf("new_aref_1_internal22843:\n");
  *(u32 *)(iSP + 4) = arg5;
  t4 = (t6 == 0) ? 1 : 0;   
  if (t4 == 0) 
    goto case_others_147;

case_0_141:
  if (_trace) printf("case_0_141:\n");
  r31 = r31 | r31;
  if (t1 == 0) 
    goto new_aref_1_internal22844;
  *(u32 *)iSP = t9;
  goto NEXTINSTRUCTION;   

case_2_142:
  if (_trace) printf("case_2_142:\n");
  /* AREF1-8B */
  r31 = r31 | r31;
  t4 = arg4 & 3;
  t5 = (u8)(t9 >> ((t4&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal22844;
  *(u32 *)iSP = t5;
  goto NEXTINSTRUCTION;   

case_3_143:
  if (_trace) printf("case_3_143:\n");
  /* AREF1-4B */
  r31 = r31 | r31;
  t4 = arg4 & 7;		// byte-index 
  t4 = t4 << 2;   		// byte-position 
  t5 = t9 >> (t4 & 63);   		// byte in position 
  t5 = t5 & 15;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal22844;
  *(u32 *)iSP = t5;
  goto NEXTINSTRUCTION;   

case_5_144:
  if (_trace) printf("case_5_144:\n");
  /* AREF1-1B */
  r31 = r31 | r31;
  t4 = arg4 & 31;		// byte-index 
  r31 = r31 | r31;
  t5 = t9 >> (t4 & 63);   		// byte in position 
  t5 = t5 & 1;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal22844;
  *(u32 *)iSP = t5;
  goto NEXTINSTRUCTION;   

case_1_145:
  if (_trace) printf("case_1_145:\n");
  /* AREF1-16B */
  t4 = arg4 & 1;
  t4 = t4 + t4;		// Bletch, it's a byte ref 
  t5 = (u16)(t9 >> ((t4&7)*8));   
  if (t1 == 0) 
    goto new_aref_1_internal22844;
  *(u32 *)iSP = t5;
  goto NEXTINSTRUCTION;   

case_others_147:
  if (_trace) printf("case_others_147:\n");
  r31 = r31 | r31;
  t4 = (t6 == 2) ? 1 : 0;   
  t5 = (t6 == 3) ? 1 : 0;   
  if (t4 != 0)   
    goto case_2_142;
  t4 = (t6 == 5) ? 1 : 0;   
  if (t5 != 0)   
    goto case_3_143;
  t5 = (t6 == 1) ? 1 : 0;   
  if (t4 != 0)   
    goto case_5_144;
  if (t5 != 0)   
    goto case_1_145;

case_4_146:
  if (_trace) printf("case_4_146:\n");
  /* AREF1-2B */
  r31 = r31 | r31;
  t4 = arg4 & 15;		// byte-index 
  t4 = t4 << 1;   		// byte-position 
  t5 = t9 >> (t4 & 63);   		// byte in position 
  t5 = t5 & 3;		// byte masked 
  if (t1 == 0) 
    goto new_aref_1_internal22844;
  *(u32 *)iSP = t5;
  goto NEXTINSTRUCTION;   

new_aref_1_internal22838:
  if (_trace) printf("new_aref_1_internal22838:\n");
  arg4 = t7 + arg4;
  t1 = arg4 >> (t6 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  goto new_aref_1_internal22839;   

new_aref_1_internal22840:
  if (_trace) printf("new_aref_1_internal22840:\n");
  t1 = arg5 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto new_aref_1_internal22845;
  goto new_aref_1_internal22841;   

new_aref_1_internal22842:
  if (_trace) printf("new_aref_1_internal22842:\n");
  arg5 = Type_Character;
  if (t8 & 1)   
    goto new_aref_1_internal22843;
  arg5 = Type_Fixnum;
  if (t8 == 0) 
    goto new_aref_1_internal22843;
  t2 = *(u64 *)&(processor->niladdress);   
  t3 = *(u64 *)&(processor->taddress);   
  goto new_aref_1_internal22843;   

new_aref_1_internal22844:
  if (_trace) printf("new_aref_1_internal22844:\n");
  if (t5)   
    t2 = t3;
  *(u64 *)iSP = t2;   
  goto NEXTINSTRUCTION;   

new_aref_1_internal22845:
  if (_trace) printf("new_aref_1_internal22845:\n");
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

vma_memory_read22848:
  if (_trace) printf("vma_memory_read22848:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg5 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read22847;   

vma_memory_read22850:
  if (_trace) printf("vma_memory_read22850:\n");
  if ((t4 & 1) == 0)   
    goto vma_memory_read22849;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read22846;   

vma_memory_read22849:
  if (_trace) printf("vma_memory_read22849:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t4 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t4 = (t4 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t4;   		// Get the memory action 

vma_memory_read22854:
  if (_trace) printf("vma_memory_read22854:\n");
  t4 = t5 & MemoryActionTransform;
  if (t4 == 0) 
    goto vma_memory_read22853;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read22857;   

vma_memory_read22853:

vma_memory_read22852:
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

force_alignment22871:
  if (_trace) printf("force_alignment22871:\n");
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

vma_memory_read22858:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22860;

vma_memory_read22859:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22862;

vma_memory_read22868:
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

force_alignment22870:
  if (_trace) printf("force_alignment22870:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)arg6 = arg1;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write22869;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write22869:
  if (_trace) printf("vma_memory_write22869:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t6 = arg2 - t11;   		// Stack cache offset 
  t5 = (t6 * 8) + t5;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t5 = arg1;
		/* write the stack cache */
  *(u32 *)(t5 + 4) = arg5;
  goto NEXTINSTRUCTION;   

vma_memory_read22862:
  if (_trace) printf("vma_memory_read22862:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22861;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22858;   

vma_memory_read22861:
  if (_trace) printf("vma_memory_read22861:\n");

vma_memory_read22860:
  if (_trace) printf("vma_memory_read22860:\n");
  r0 = (u64)&&return0125;
  goto memoryreadwritedecode;
return0125:
  goto vma_memory_read22868;   

/* end DoRplaca */
  /* End of Halfword operand from stack instruction - DoRplaca */
/* start MemoryReadWrite */


memoryreadwrite:
  if (_trace) printf("memoryreadwrite:\n");
  /* Memory Read Internal */

vma_memory_read22872:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22874;

vma_memory_read22873:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read22876;

vma_memory_read22881:
  goto *r0; /* ret */

memoryreadwritedecode:
  if (_trace) printf("memoryreadwritedecode:\n");
  if (t6 == 0) 
    goto vma_memory_read22875;

vma_memory_read22874:
  if (_trace) printf("vma_memory_read22874:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  arg6 = *(s32 *)t5;   
  arg5 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read22873;   

vma_memory_read22876:
  if (_trace) printf("vma_memory_read22876:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22875;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22872;   

vma_memory_read22875:
  if (_trace) printf("vma_memory_read22875:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read22878:

vma_memory_read22877:
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

force_alignment22892:
  if (_trace) printf("force_alignment22892:\n");
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

vma_memory_read22882:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read22884;

vma_memory_read22883:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read22886;

vma_memory_read22891:
  /* TagCdr. */
  arg5 = arg5 >> 6;   
  arg5 = arg5 - Cdr_Normal;   
  if (arg5 != 0)   		// J. if CDR coded 
    goto rplacdexception;
  arg2 = arg2 + 1;		// address of CDR 
  goto rplacstore;   

vma_memory_read22886:
  if (_trace) printf("vma_memory_read22886:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read22885;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read22882;   

vma_memory_read22885:
  if (_trace) printf("vma_memory_read22885:\n");

vma_memory_read22884:
  if (_trace) printf("vma_memory_read22884:\n");
  r0 = (u64)&&return0126;
  goto memoryreadcdrdecode;
return0126:
  goto vma_memory_read22891;   

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

force_alignment22910:
  if (_trace) printf("force_alignment22910:\n");
  if (t6 == 0) 
    goto basic_dispatch22898;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment22902:
  if (_trace) printf("force_alignment22902:\n");
  if (t3 == 0) 
    goto binary_type_dispatch22893;
  /* Here if argument TypeFixnum */
  t2 = (s32)arg4 - (s32)arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch22899:
  if (_trace) printf("basic_dispatch22899:\n");

basic_dispatch22898:
  if (_trace) printf("basic_dispatch22898:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment22911:
  if (_trace) printf("force_alignment22911:\n");
  if (t6 == 0) 
    goto basic_dispatch22903;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment22907:
  if (_trace) printf("force_alignment22907:\n");
  if (t3 == 0) 
    goto binary_type_dispatch22893;
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

basic_dispatch22904:
  if (_trace) printf("basic_dispatch22904:\n");

basic_dispatch22903:
  if (_trace) printf("basic_dispatch22903:\n");
  /* Here for all other cases */

binary_type_dispatch22893:
  if (_trace) printf("binary_type_dispatch22893:\n");
  goto equalnumbermmexc;   

basic_dispatch22897:
  if (_trace) printf("basic_dispatch22897:\n");

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

force_alignment22916:
  if (_trace) printf("force_alignment22916:\n");
  if (t4 == 0) 
    goto basic_dispatch22913;
  /* Here if argument TypeFixnum */
  t2 = (s32)arg4 - (s32)arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2 == 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch22913:
  if (_trace) printf("basic_dispatch22913:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch22912:
  if (_trace) printf("basic_dispatch22912:\n");

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
  r0 = (u64)&&return0127;
  goto carcdrinternal;
return0127:
  /* TagType. */
  arg5 = arg5 & 63;
  arg5 = arg5 | t3;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = arg5;
  t5 = t1 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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

force_alignment22956:
  if (_trace) printf("force_alignment22956:\n");
  if (t10 == 0) 
    goto basic_dispatch22927;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22933:
  if (_trace) printf("force_alignment22933:\n");
  if (t12 == 0) 
    goto basic_dispatch22929;
  /* Here if argument TypeFixnum */
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = (s64)((s32)t2 - (s64)(s32)t4); 		// compute 64-bit result 
  if (t5 >> 32)
    exception();  /* subl/v */ 
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
		/* Semi-cheat, we know temp2 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 4) = t9;
  iPC = t6;
  *(u32 *)iSP = t5;
  iCP = t7;
  goto cachevalid;   

basic_dispatch22929:
  if (_trace) printf("basic_dispatch22929:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment22934:
  if (_trace) printf("force_alignment22934:\n");
  if (t12 == 0) 
    goto basic_dispatch22930;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation22917;   

basic_dispatch22930:
  if (_trace) printf("basic_dispatch22930:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22935:
  if (_trace) printf("force_alignment22935:\n");
  if (t12 == 0) 
    goto binary_type_dispatch22924;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation22920;   

basic_dispatch22928:
  if (_trace) printf("basic_dispatch22928:\n");

basic_dispatch22927:
  if (_trace) printf("basic_dispatch22927:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment22957:
  if (_trace) printf("force_alignment22957:\n");
  if (t10 == 0) 
    goto basic_dispatch22936;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment22942:
  if (_trace) printf("force_alignment22942:\n");
  if (t12 == 0) 
    goto basic_dispatch22938;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation22917:
  if (_trace) printf("simple_binary_arithmetic_operation22917:\n");
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  goto cachevalid;   

basic_dispatch22938:
  if (_trace) printf("basic_dispatch22938:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22943:
  if (_trace) printf("force_alignment22943:\n");
  if (t12 == 0) 
    goto basic_dispatch22939;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation22917;   

basic_dispatch22939:
  if (_trace) printf("basic_dispatch22939:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22944:
  if (_trace) printf("force_alignment22944:\n");
  if (t12 == 0) 
    goto binary_type_dispatch22924;
  /* Here if argument TypeDoubleFloat */

simple_binary_arithmetic_operation22920:
  if (_trace) printf("simple_binary_arithmetic_operation22920:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto simple_binary_arithmetic_operation22921;   

basic_dispatch22937:
  if (_trace) printf("basic_dispatch22937:\n");

basic_dispatch22936:
  if (_trace) printf("basic_dispatch22936:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22958:
  if (_trace) printf("force_alignment22958:\n");
  if (t10 == 0) 
    goto basic_dispatch22945;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment22951:
  if (_trace) printf("force_alignment22951:\n");
  if (t12 == 0) 
    goto basic_dispatch22947;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0128;
  goto fetchdoublefloat;
return0128:
  LDT(1, f1, processor->fp0);   

simple_binary_arithmetic_operation22921:
  if (_trace) printf("simple_binary_arithmetic_operation22921:\n");
  arg2 = (u32)t4;   
  r0 = (u64)&&return0129;
  goto fetchdoublefloat;
return0129:
  LDT(2, f2, processor->fp0);   

simple_binary_arithmetic_operation22918:
  if (_trace) printf("simple_binary_arithmetic_operation22918:\n");
  SUBT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0130;
  goto consdoublefloat;
return0130:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  goto cachevalid;   

basic_dispatch22947:
  if (_trace) printf("basic_dispatch22947:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment22952:
  if (_trace) printf("force_alignment22952:\n");
  if (t12 == 0) 
    goto basic_dispatch22948;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation22919:
  if (_trace) printf("simple_binary_arithmetic_operation22919:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0131;
  goto fetchdoublefloat;
return0131:
  LDT(1, f1, processor->fp0);   
  goto simple_binary_arithmetic_operation22918;   

basic_dispatch22948:
  if (_trace) printf("basic_dispatch22948:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22953:
  if (_trace) printf("force_alignment22953:\n");
  if (t12 == 0) 
    goto binary_type_dispatch22924;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation22919;   

basic_dispatch22946:
  if (_trace) printf("basic_dispatch22946:\n");

basic_dispatch22945:
  if (_trace) printf("basic_dispatch22945:\n");
  /* Here for all other cases */

binary_type_dispatch22923:
  if (_trace) printf("binary_type_dispatch22923:\n");

dosubovfl:
  if (_trace) printf("dosubovfl:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch22925;   

binary_type_dispatch22924:
  if (_trace) printf("binary_type_dispatch22924:\n");
  t1 = t3;
  goto dosubovfl;   

binary_type_dispatch22925:
  if (_trace) printf("binary_type_dispatch22925:\n");

basic_dispatch22926:
  if (_trace) printf("basic_dispatch22926:\n");

DoSubIM:
  if (_trace) printf("DoSubIM:\n");
  t1 = (u32)(arg6 >> ((4&7)*8));   
  t2 = (s32)arg6;		// get ARG1 tag/data 
  t11 = t1 & 63;		// Strip off any CDR code bits. 
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment22963:
  if (_trace) printf("force_alignment22963:\n");
  if (t12 == 0) 
    goto basic_dispatch22960;
  /* Here if argument TypeFixnum */
  t3 = t2 - arg2;   		// compute 64-bit result 
  t4 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t10 = (s32)t3;		// compute 32-bit sign-extended result 
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t10 = (t3 == t10) ? 1 : 0;   		// is it the same as the 64-bit result? 
  if (t10 == 0) 		// if not, we overflowed 
    goto dosubovfl;
		/* Semi-cheat, we know temp2 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 4) = t11;
  iPC = t4;
  *(u32 *)iSP = t3;
  iCP = t5;
  goto cachevalid;   

basic_dispatch22960:
  if (_trace) printf("basic_dispatch22960:\n");
  /* Here for all other cases */
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindosub;   

basic_dispatch22959:
  if (_trace) printf("basic_dispatch22959:\n");

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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
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

force_alignment22969:
  if (_trace) printf("force_alignment22969:\n");
  if (t5 == 0) 
    goto basic_dispatch22965;
  /* Here if argument TypeFixnum */
  iPC = t6;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 < 0)   		// T if predicate succeeds 
    t11 = t12;
  *(u64 *)(iSP + 8) = t11;   
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch22965:
  if (_trace) printf("basic_dispatch22965:\n");
  t5 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment22970:
  if (_trace) printf("force_alignment22970:\n");
  if (t5 == 0) 
    goto basic_dispatch22966;
  /* Here if argument TypeSingleFloat */
  iPC = t6;
  *(u64 *)(iSP + 8) = t12;   
  iSP = iSP + 8;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (FLTU64(1, f1) < 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch22966:
  if (_trace) printf("basic_dispatch22966:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch22964:
  if (_trace) printf("basic_dispatch22964:\n");

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

force_alignment22976:
  if (_trace) printf("force_alignment22976:\n");
  if (t5 == 0) 
    goto basic_dispatch22972;
  /* Here if argument TypeFixnum */
  iPC = t6;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 > 0)   		// T if predicate succeeds 
    t11 = t12;
  *(u64 *)(iSP + 8) = t11;   
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch22972:
  if (_trace) printf("basic_dispatch22972:\n");
  t5 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment22977:
  if (_trace) printf("force_alignment22977:\n");
  if (t5 == 0) 
    goto basic_dispatch22973;
  /* Here if argument TypeSingleFloat */
  iPC = t6;
  *(u64 *)(iSP + 8) = t12;   
  iSP = iSP + 8;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (FLTU64(1, f1) > 0.0)   
    goto cachevalid;
  *(u64 *)iSP = t11;   		// Didn't branch, answer is NIL 
  goto cachevalid;   

basic_dispatch22973:
  if (_trace) printf("basic_dispatch22973:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch22971:
  if (_trace) printf("basic_dispatch22971:\n");

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

force_alignment22995:
  if (_trace) printf("force_alignment22995:\n");
  if (t6 == 0) 
    goto basic_dispatch22983;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment22987:
  if (_trace) printf("force_alignment22987:\n");
  if (t3 == 0) 
    goto binary_type_dispatch22978;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 < 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch22984:
  if (_trace) printf("basic_dispatch22984:\n");

basic_dispatch22983:
  if (_trace) printf("basic_dispatch22983:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment22996:
  if (_trace) printf("force_alignment22996:\n");
  if (t6 == 0) 
    goto basic_dispatch22988;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment22992:
  if (_trace) printf("force_alignment22992:\n");
  if (t3 == 0) 
    goto binary_type_dispatch22978;
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

basic_dispatch22989:
  if (_trace) printf("basic_dispatch22989:\n");

basic_dispatch22988:
  if (_trace) printf("basic_dispatch22988:\n");
  /* Here for all other cases */

binary_type_dispatch22978:
  if (_trace) printf("binary_type_dispatch22978:\n");
  goto lesspmmexc;   

basic_dispatch22982:
  if (_trace) printf("basic_dispatch22982:\n");

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

force_alignment23001:
  if (_trace) printf("force_alignment23001:\n");
  if (t4 == 0) 
    goto basic_dispatch22998;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 < 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch22998:
  if (_trace) printf("basic_dispatch22998:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch22997:
  if (_trace) printf("basic_dispatch22997:\n");

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

force_alignment23007:
  if (_trace) printf("force_alignment23007:\n");
  if (t2 == 0) 
    goto basic_dispatch23003;
  /* Here if argument TypeFixnum */
  t2 = *(u64 *)&(processor->mostnegativefixnum);   
  t3 = arg3 - 1;   
  t2 = (arg3 == t2) ? 1 : 0;   
  if (t2 != 0)   
    goto decrementexception;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)arg1 = t3;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = arg2;
  goto cachevalid;   

basic_dispatch23003:
  if (_trace) printf("basic_dispatch23003:\n");
  t2 = (t1 == Type_SingleFloat) ? 1 : 0;   

force_alignment23008:
  if (_trace) printf("force_alignment23008:\n");
  if (t2 == 0) 
    goto basic_dispatch23004;
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

basic_dispatch23004:
  if (_trace) printf("basic_dispatch23004:\n");
  /* Here for all other cases */
  goto decrementexception;   

basic_dispatch23002:
  if (_trace) printf("basic_dispatch23002:\n");

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

force_alignment23009:
  if (_trace) printf("force_alignment23009:\n");
  t2 = t2 & 192;		// Get Just the CDR code in position 
  t1 = t1 & 63;		// Get the TAG of arg1 
  t3 = t1 | t2;		// Merge the tag of arg2 with the cdr code of arg1 
		/* Replace tag/cdr code no pop */
  *(u32 *)(arg1 + 4) = t3;
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

force_alignment23015:
  if (_trace) printf("force_alignment23015:\n");
  if (t2 == 0) 
    goto basic_dispatch23011;
  /* Here if argument TypeFixnum */
  t2 = *(u64 *)&(processor->mostpositivefixnum);   
  t3 = arg3 + 1;
  t2 = (arg3 == t2) ? 1 : 0;   
  if (t2 != 0)   
    goto incrementexception;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)arg1 = t3;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = arg2;
  goto cachevalid;   

basic_dispatch23011:
  if (_trace) printf("basic_dispatch23011:\n");
  t2 = (t1 == Type_SingleFloat) ? 1 : 0;   

force_alignment23016:
  if (_trace) printf("force_alignment23016:\n");
  if (t2 == 0) 
    goto basic_dispatch23012;
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

basic_dispatch23012:
  if (_trace) printf("basic_dispatch23012:\n");
  /* Here for all other cases */
  goto incrementexception;   

basic_dispatch23010:
  if (_trace) printf("basic_dispatch23010:\n");

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
  r0 = (u64)&&return0132;
  goto lookuphandler;
return0132:
  arg4 = *(u64 *)(iFP + 16);   		// clobbered by |LookupHandler| 
  t3 = t4 - Type_EvenPC;   
  t3 = t3 & 62;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto message_dispatch23019;
  t3 = t6 & 63;		// Strip CDR code 
  t3 = t3 - Type_NIL;   
  if (t3 == 0) 
    goto message_dispatch23017;
  *(u32 *)(iFP + 16) = t7;
		/* write the stack cache */
  *(u32 *)(iFP + 20) = t6;
  goto message_dispatch23018;   

message_dispatch23017:
  if (_trace) printf("message_dispatch23017:\n");
		/* swap message/instance in the frame */
  *(u32 *)(iFP + 16) = t1;
		/* write the stack cache */
  *(u32 *)(iFP + 20) = arg1;

message_dispatch23018:
  if (_trace) printf("message_dispatch23018:\n");
  *(u64 *)(iFP + 24) = arg4;   
  /* Convert real continuation to PC. */
  iPC = t4 & 1;
  iPC = t9 + iPC;
  iPC = t9 + iPC;
  goto interpretinstructionforjump;   

message_dispatch23019:
  if (_trace) printf("message_dispatch23019:\n");
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

force_alignment23022:
  if (_trace) printf("force_alignment23022:\n");
  if (t3 & 1)   		// J. if apply args 
    goto b_apply_argument_supplied23020;

b_apply_argument_supplied23021:
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
		/* set tag */
  *(u32 *)(iSP + 4) = t6;
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
		/* set tag */
  *(u32 *)(iSP + 4) = t1;
  t2 = arg5 >> 17;   
  t3 = *(s32 *)(iSP + 4);   		// Get the tag of the stack top. 

force_alignment23027:
  if (_trace) printf("force_alignment23027:\n");
  if (t2 & 1)   		// J. if apply args 
    goto b_apply_argument_supplied23025;

b_apply_argument_supplied23026:
  t1 = (arg4 * 8) + iFP;  
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  t1 = Type_List;
  *(u32 *)(iSP + 8) = t2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
  iSP = iSP + 8;
  goto push_apply_args23024;   

push_apply_args23023:
  if (_trace) printf("push_apply_args23023:\n");
  t1 = iSP - 8;   
  t3 = *(s32 *)(t1 + 4);   		// get tag 
  t3 = t3 & 63;
  t3 = t3 | 128;
		/* set tag */
  *(u32 *)(t1 + 4) = t3;
  t1 = (arg4 * 8) + iFP;  
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  t1 = Type_List;
  *(u32 *)(iSP + 8) = t2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
  iSP = iSP + 8;
  iLP = iLP + 8;
  arg5 = arg5 + 1;
  *(u32 *)&processor->control = arg5;

push_apply_args23024:
  if (_trace) printf("push_apply_args23024:\n");
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

b_apply_argument_supplied23025:
  if (_trace) printf("b_apply_argument_supplied23025:\n");
  t3 = t3 & 63;
  t3 = t3 - Type_NIL;   
  if (t3 != 0)   		// J. if apply args supplied not nil. 
    goto push_apply_args23023;
  t2 = t2 & 1;		// keep just the apply bit! 
  t2 = t2 << 17;   		// reposition the apply bit 
  iSP = iSP - 8;   		// Pop off the null applied arg. 
  arg5 = arg5 & ~t2;		// Blast the apply arg bit away 
		/* Reset the stored cr bit */
  *(u32 *)&processor->control = arg5;
  goto b_apply_argument_supplied23026;   

b_apply_argument_supplied23020:
  if (_trace) printf("b_apply_argument_supplied23020:\n");
  t4 = t4 & 63;
  t4 = t4 - Type_NIL;   
  if (t4 != 0)   		// J. if apply args supplied not nil. 
    goto applysuppra;
  t3 = t3 & 1;		// keep just the apply bit! 
  t3 = t3 << 17;   		// reposition the apply bit 
  iSP = iSP - 8;   		// Pop off the null applied arg. 
  arg5 = arg5 & ~t3;		// Blast the apply arg bit away 
		/* Reset the stored cr bit */
  *(u32 *)&processor->control = arg5;
  goto b_apply_argument_supplied23021;   

/* end DoEntryRestAccepted */
  /* End of Halfword operand from stack instruction - DoEntryRestAccepted */
/* start CarCdrInternal */


carcdrinternal:
  if (_trace) printf("carcdrinternal:\n");
  sp = sp + -8;   
  arg2 = (u32)(t2 >> ((zero&7)*8));   
  t5 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_List) ? 1 : 0;   

force_alignment23075:
  if (_trace) printf("force_alignment23075:\n");
  if (t6 == 0) 
    goto basic_dispatch23032;
  /* Here if argument TypeList */
  /* Memory Read Internal */

vma_memory_read23033:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23035;

vma_memory_read23034:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read23037;

vma_memory_read23044:
  t5 = (s32)arg2 - (s32)t2;   
  if (t5 != 0)   		// CAR forwarded, must CDR the hard way 
    goto carcdr_internal23028;
  t1 = arg5;
  t2 = arg6;

carcdr_internal23030:
  if (_trace) printf("carcdr_internal23030:\n");
  t5 = arg5 & 192;		// Extract CDR code. 
  if (t5 != 0)   
    goto basic_dispatch23046;
  /* Here if argument 0 */
  arg6 = arg2 + 1;		// Address of next position is CDR 
  arg5 = Type_List;

basic_dispatch23045:
  if (_trace) printf("basic_dispatch23045:\n");

basic_dispatch23031:
  if (_trace) printf("basic_dispatch23031:\n");

carcdr_internal23029:
  if (_trace) printf("carcdr_internal23029:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch23032:
  if (_trace) printf("basic_dispatch23032:\n");
  t6 = (t5 == Type_NIL) ? 1 : 0;   

force_alignment23076:
  if (_trace) printf("force_alignment23076:\n");
  if (t6 == 0) 
    goto basic_dispatch23062;
  /* Here if argument TypeNIL */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto basic_dispatch23031;   

basic_dispatch23062:
  if (_trace) printf("basic_dispatch23062:\n");
  /* Here for all other cases */
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto listexception;

carcdr_internal23028:
  if (_trace) printf("carcdr_internal23028:\n");
  arg2 = (u32)(t2 >> ((zero&7)*8));   
  t1 = arg5;
  t2 = arg6;
  /* Memory Read Internal */

vma_memory_read23064:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->cdr_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23066;

vma_memory_read23065:
  t7 = zero + 192;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23068;

vma_memory_read23073:
  goto carcdr_internal23030;   

vma_memory_read23068:
  if (_trace) printf("vma_memory_read23068:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23067;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23064;   

vma_memory_read23067:
  if (_trace) printf("vma_memory_read23067:\n");

vma_memory_read23066:
  if (_trace) printf("vma_memory_read23066:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0133;
  goto memoryreadcdrdecode;
return0133:
  r0 = *(u64 *)sp;   
  goto vma_memory_read23073;   

basic_dispatch23046:
  if (_trace) printf("basic_dispatch23046:\n");
  t6 = (t5 == 128) ? 1 : 0;   

force_alignment23077:
  if (_trace) printf("force_alignment23077:\n");
  if (t6 == 0) 
    goto basic_dispatch23047;
  /* Here if argument 128 */
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read23048:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23050;

vma_memory_read23049:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t8 & 1)   
    goto vma_memory_read23052;
  goto carcdr_internal23029;   

basic_dispatch23047:
  if (_trace) printf("basic_dispatch23047:\n");
  t6 = (t5 == 64) ? 1 : 0;   

force_alignment23078:
  if (_trace) printf("force_alignment23078:\n");
  if (t6 == 0) 
    goto basic_dispatch23059;
  /* Here if argument 64 */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto carcdr_internal23029;   

basic_dispatch23059:
  if (_trace) printf("basic_dispatch23059:\n");
  /* Here for all other cases */
  arg5 = arg2;
  arg2 = 15;
  goto illegaloperand;

vma_memory_read23052:
  if (_trace) printf("vma_memory_read23052:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23051;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23048;   

vma_memory_read23051:
  if (_trace) printf("vma_memory_read23051:\n");

vma_memory_read23050:
  if (_trace) printf("vma_memory_read23050:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0134;
  goto memoryreaddatadecode;
return0134:
  r0 = *(u64 *)sp;   
  goto carcdr_internal23029;   

vma_memory_read23037:
  if (_trace) printf("vma_memory_read23037:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23036;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23033;   

vma_memory_read23036:
  if (_trace) printf("vma_memory_read23036:\n");

vma_memory_read23035:
  if (_trace) printf("vma_memory_read23035:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0135;
  goto memoryreaddatadecode;
return0135:
  r0 = *(u64 *)sp;   
  goto vma_memory_read23044;   

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

force_alignment23126:
  if (_trace) printf("force_alignment23126:\n");
  if (t4 == 0) 
    goto basic_dispatch23083;
  /* Here if argument TypeList */
  /* Memory Read Internal */

vma_memory_read23084:
  t5 = t2 + ivory;
  arg6 = (t5 * 4);   
  arg5 = LDQ_U(t5);   
  t3 = t2 - t11;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->dataread_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read23086;

vma_memory_read23085:
  t5 = zero + 240;   
  t6 = t6 >> (arg5 & 63);   
  t5 = t5 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t6 & 1)   
    goto vma_memory_read23088;

vma_memory_read23095:
  t3 = (s32)t2 - (s32)arg4;   
  if (t3 != 0)   		// CAR forwarded, must CDR the hard way 
    goto carcdr_internal23079;
  arg3 = arg5;
  arg4 = arg6;

carcdr_internal23081:
  if (_trace) printf("carcdr_internal23081:\n");
  t3 = arg5 & 192;		// Extract CDR code. 
  if (t3 != 0)   
    goto basic_dispatch23097;
  /* Here if argument 0 */
  arg6 = t2 + 1;		// Address of next position is CDR 
  arg5 = Type_List;

basic_dispatch23096:
  if (_trace) printf("basic_dispatch23096:\n");

basic_dispatch23082:
  if (_trace) printf("basic_dispatch23082:\n");

carcdr_internal23080:
  if (_trace) printf("carcdr_internal23080:\n");
		/* Push the pulled argument */
  *(u32 *)iSP = arg4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = arg3;
  t1 = arg5 & 63;		// set CDR-NEXT 
		/* Push the new rest arg */
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
  iSP = iSP + 8;
  arg2 = *(s32 *)&processor->control;   
  t2 = arg2 & 255;		// Get current arg size. 
  arg2 = arg2 & ~255L;
  t2 = t2 + 1;
  arg2 = t2 + arg2;		// Update the arg size 
  *(u32 *)&processor->control = arg2;
  iLP = iLP + 8;
  goto INTERPRETINSTRUCTION;   

basic_dispatch23083:
  if (_trace) printf("basic_dispatch23083:\n");
  t4 = (t3 == Type_NIL) ? 1 : 0;   

force_alignment23127:
  if (_trace) printf("force_alignment23127:\n");
  if (t4 == 0) 
    goto basic_dispatch23113;
  /* Here if argument TypeNIL */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto basic_dispatch23082;   

basic_dispatch23113:
  if (_trace) printf("basic_dispatch23113:\n");
  /* Here for all other cases */
  arg1 = arg1;
  goto pullapplyargstrap;

carcdr_internal23079:
  if (_trace) printf("carcdr_internal23079:\n");
  t2 = (u32)(arg4 >> ((zero&7)*8));   
  arg3 = arg5;
  arg4 = arg6;
  /* Memory Read Internal */

vma_memory_read23115:
  t5 = t2 + ivory;
  arg6 = (t5 * 4);   
  arg5 = LDQ_U(t5);   
  t3 = t2 - t11;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->cdr_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read23117;

vma_memory_read23116:
  t5 = zero + 192;   
  t6 = t6 >> (arg5 & 63);   
  t5 = t5 >> (arg5 & 63);   
  if (t6 & 1)   
    goto vma_memory_read23119;

vma_memory_read23124:
  goto carcdr_internal23081;   

vma_memory_read23117:
  if (_trace) printf("vma_memory_read23117:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg6 = *(s32 *)t3;   
  arg5 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read23116;   

vma_memory_read23119:
  if (_trace) printf("vma_memory_read23119:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read23118;
  t2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23115;   

vma_memory_read23118:
  if (_trace) printf("vma_memory_read23118:\n");
  t6 = *(u64 *)&(processor->cdr);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read23121:
  /* Perform memory action */
  arg1 = t6;
  arg2 = 9;
  goto performmemoryaction;

basic_dispatch23097:
  if (_trace) printf("basic_dispatch23097:\n");
  t4 = (t3 == 128) ? 1 : 0;   

force_alignment23128:
  if (_trace) printf("force_alignment23128:\n");
  if (t4 == 0) 
    goto basic_dispatch23098;
  /* Here if argument 128 */
  t2 = t2 + 1;
  /* Memory Read Internal */

vma_memory_read23099:
  t5 = t2 + ivory;
  arg6 = (t5 * 4);   
  arg5 = LDQ_U(t5);   
  t3 = t2 - t11;   		// Stack cache offset 
  t6 = *(u64 *)&(processor->dataread_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read23101;

vma_memory_read23100:
  t5 = zero + 240;   
  t6 = t6 >> (arg5 & 63);   
  t5 = t5 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t6 & 1)   
    goto vma_memory_read23103;
  goto carcdr_internal23080;   

basic_dispatch23098:
  if (_trace) printf("basic_dispatch23098:\n");
  t4 = (t3 == 64) ? 1 : 0;   

force_alignment23129:
  if (_trace) printf("force_alignment23129:\n");
  if (t4 == 0) 
    goto basic_dispatch23110;
  /* Here if argument 64 */
  arg6 = *(s32 *)&processor->niladdress;   
  arg5 = *((s32 *)(&processor->niladdress)+1);   
  arg6 = (u32)arg6;   
  goto carcdr_internal23080;   

basic_dispatch23110:
  if (_trace) printf("basic_dispatch23110:\n");
  /* Here for all other cases */
  arg5 = t2;
  arg2 = 15;
  goto illegaloperand;

vma_memory_read23101:
  if (_trace) printf("vma_memory_read23101:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg6 = *(s32 *)t3;   
  arg5 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read23100;   

vma_memory_read23103:
  if (_trace) printf("vma_memory_read23103:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read23102;
  t2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23099;   

vma_memory_read23102:
  if (_trace) printf("vma_memory_read23102:\n");
  t6 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read23107:
  if (_trace) printf("vma_memory_read23107:\n");
  t5 = t6 & MemoryActionTransform;
  if (t5 == 0) 
    goto vma_memory_read23106;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto carcdr_internal23080;   

vma_memory_read23106:

vma_memory_read23105:
  /* Perform memory action */
  arg1 = t6;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23086:
  if (_trace) printf("vma_memory_read23086:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg6 = *(s32 *)t3;   
  arg5 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read23085;   

vma_memory_read23088:
  if (_trace) printf("vma_memory_read23088:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read23087;
  t2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23084;   

vma_memory_read23087:
  if (_trace) printf("vma_memory_read23087:\n");
  t6 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t6;   		// Adjust for a longword load 
  t6 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read23092:
  if (_trace) printf("vma_memory_read23092:\n");
  t5 = t6 & MemoryActionTransform;
  if (t5 == 0) 
    goto vma_memory_read23091;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read23095;   

vma_memory_read23091:

vma_memory_read23090:
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
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
    goto abandon_frame_simple23131;
  iPC = t5 << 1;   		// Assume even PC 
  t1 = t4 & 1;
  t7 = *(u64 *)&(processor->continuationcp);   
  iPC = iPC + t1;

abandon_frame_simple23131:
  if (_trace) printf("abandon_frame_simple23131:\n");
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
		/* Restore the control register */
  *(u32 *)&processor->control = t6;
  t1 = t6 & 255;		// extract the argument size 
  t3 = t3 & 1;
  t3 = t4 | t3;
  *(u64 *)&processor->stop_interpreter = t3;   
  iLP = (t1 * 8) + iFP;  		// Restore the local pointer. 
  arg6 = ((u64)iFP < (u64)arg6) ? 1 : 0;   		// ARG6 = stack-cache underflow 
  t4 = iSP + 8;		// Compute destination of copy 
  t3 = arg1;		// Values 
  t1 = *(u64 *)&(processor->cdrcodemask);   		// mask for CDR codes 
  goto stack_block_copy23132;   

stack_block_copy23133:
  if (_trace) printf("stack_block_copy23133:\n");
  t3 = t3 - 1;   
  t2 = *(u64 *)arg3;   		// Get a word from source 
  arg3 = arg3 + 8;		// advance from position 
  t2 = t2 & ~t1;		// Strip off CDR code 
  *(u64 *)t4 = t2;   		// Put word in destination 
  t4 = t4 + 8;		// advance to position 

stack_block_copy23132:
  if ((s64)t3 > 0)   
    goto stack_block_copy23133;
  iSP = (arg1 * 8) + iSP;  		// Adjust iSP over returned values 
  /* arg4 -2=effect -1=value 0=return 1=multiple */
  if (arg4 == 0) 
    goto returnmultiplereturn;

returnmultiplemultiple:
  if (_trace) printf("returnmultiplemultiple:\n");
  t1 = Type_Fixnum;
		/* push the MV return count */
  *(u32 *)(iSP + 8) = arg1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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

cleanup_frame23136:
  if (_trace) printf("cleanup_frame23136:\n");
  t1 = (1024) << 16;   
  t4 = *(s32 *)&processor->catchblock;   
  t4 = (u32)t4;   
  t2 = t1 & arg5;
  if (t2 == 0) 		// J. if cr.cleanup-catch is 0 
    goto cleanup_frame23135;
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
  goto cleanup_frame23136;   

cleanup_frame23135:
  if (_trace) printf("cleanup_frame23135:\n");
  t1 = (512) << 16;   
  t2 = t1 & arg5;
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  if (t2 == 0) 		// J. if cr.cleanup-bindings is 0. 
    goto cleanup_frame23134;

cleanup_frame23137:
  if (_trace) printf("cleanup_frame23137:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  t2 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & t2;
  t4 = t4 & ~t2;		// Turn off the bit 
  if (t3 != 0)   
    goto g23138;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g23138:
  if (_trace) printf("g23138:\n");
  /* Memory Read Internal */

vma_memory_read23139:
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
    goto vma_memory_read23141;

vma_memory_read23140:
  t10 = zero + 224;   
  t11 = t11 >> (t7 & 63);   
  t10 = t10 >> (t7 & 63);   
  if (t11 & 1)   
    goto vma_memory_read23143;

vma_memory_read23148:
  /* Memory Read Internal */

vma_memory_read23149:
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
    goto vma_memory_read23151;

vma_memory_read23150:
  t10 = zero + 224;   
  t11 = t11 >> (t3 & 63);   
  t10 = t10 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (t11 & 1)   
    goto vma_memory_read23153;

vma_memory_read23158:
  /* Memory Read Internal */

vma_memory_read23159:
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
    goto vma_memory_read23161;

vma_memory_read23160:
  t10 = *(u64 *)&(processor->bindwrite_mask);   
  t12 = zero + 224;   
  t10 = t10 >> (t8 & 63);   
  t12 = t12 >> (t8 & 63);   
  if (t10 & 1)   
    goto vma_memory_read23163;

vma_memory_read23168:
  /* Merge cdr-code */
  t9 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | t9;
  t10 = t2 + ivory;
  t9 = (t10 * 4);   
  t12 = LDQ_U(t10);   
  t11 = (t8 & 0xff) << ((t10&7)*8);   
  t12 = t12 & ~(0xffL << (t10&7)*8);   

force_alignment23171:
  if (_trace) printf("force_alignment23171:\n");
  t12 = t12 | t11;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  STQ_U(t10, t12);   
  t10 = *(s32 *)&processor->scovlimit;   
  t11 = t2 - t11;   		// Stack cache offset 
  t10 = ((u64)t11 < (u64)t10) ? 1 : 0;   		// In range? 
  *(u32 *)t9 = t6;
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write23170;

vma_memory_write23169:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t1;
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;
  arg5 = *(s32 *)&processor->control;   
  t1 = (512) << 16;   
  t2 = t1 & arg5;
  if (t2 != 0)   		// J. if cr.cleanup-bindings is 0. 
    goto cleanup_frame23137;
  t2 = *(s32 *)&processor->interruptreg;   
  t3 = t2 & 2;
  t3 = (t3 == 2) ? 1 : 0;   
  t2 = t2 | t3;
  *(u32 *)&processor->interruptreg = t2;
  if (t2 == 0) 
    goto check_preempt_request23172;
  *(u64 *)&processor->stop_interpreter = t2;   

check_preempt_request23172:
  if (_trace) printf("check_preempt_request23172:\n");

cleanup_frame23134:
  if (_trace) printf("cleanup_frame23134:\n");
  t3 = (256) << 16;   
  t2 = t3 & arg5;
  if (t2 == 0) 
    goto INTERPRETINSTRUCTION;
  arg5 = zero;
  arg2 = 79;
  goto illegaloperand;
  goto INTERPRETINSTRUCTION;   		// Retry the instruction 

vma_memory_write23170:
  if (_trace) printf("vma_memory_write23170:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t11 * 8) + t10;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t10 = t6;
		/* write the stack cache */
  *(u32 *)(t10 + 4) = t8;
  goto vma_memory_write23169;   

vma_memory_read23161:
  if (_trace) printf("vma_memory_read23161:\n");
  t11 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t10 * 8) + t11;  		// reconstruct SCA 
  t9 = *(s32 *)t10;   
  t8 = *(s32 *)(t10 + 4);   		// Read from stack cache 
  goto vma_memory_read23160;   

vma_memory_read23163:
  if (_trace) printf("vma_memory_read23163:\n");
  if ((t12 & 1) == 0)   
    goto vma_memory_read23162;
  t2 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read23159;   

vma_memory_read23162:
  if (_trace) printf("vma_memory_read23162:\n");
  t10 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t12 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t12 = (t12 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t12;   		// Get the memory action 

vma_memory_read23165:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read23151:
  if (_trace) printf("vma_memory_read23151:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read23150;   

vma_memory_read23153:
  if (_trace) printf("vma_memory_read23153:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23152;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23149;   

vma_memory_read23152:
  if (_trace) printf("vma_memory_read23152:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23155:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read23141:
  if (_trace) printf("vma_memory_read23141:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read23140;   

vma_memory_read23143:
  if (_trace) printf("vma_memory_read23143:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23142;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read23139;   

vma_memory_read23142:
  if (_trace) printf("vma_memory_read23142:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23145:
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
  r0 = (u64)&&return0136;
  goto stackcacheunderflow;
return0136:
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
  goto stack_block_copy23173;   

stack_block_copy23174:
  if (_trace) printf("stack_block_copy23174:\n");
  t1 = t1 - 8;   		// advance from position 
  t5 = t5 - 1;   
  t7 = *(u64 *)t1;   		// Get a word from source 
  t2 = t2 - 8;   		// advance to position 
  *(u64 *)t2 = t7;   		// Put word in destination 

stack_block_copy23173:
  if ((s64)t5 > 0)   
    goto stack_block_copy23174;
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
  goto stack_fill23175;   

stack_fill23176:
  if (_trace) printf("stack_fill23176:\n");
  t7 = t1 + ivory;
  t5 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  t3 = t3 - 1;   
  t1 = t1 + 1;		// advance vma position 
  *(u32 *)t2 = t5;
		/* write the stack cache */
  *(u32 *)(t2 + 4) = t4;
  t2 = t2 + 8;		// advance sca position 

stack_fill23175:
  if ((s64)t3 > 0)   
    goto stack_fill23176;
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
		/* Update stack cache limit */
  *(u32 *)&processor->scovlimit = t4;
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
		/* Will be destructively modified */
  *(u32 *)&processor->scovdumpcount = t1;
  t5 = t2 + ivory;		// Starting address of tags 
  t2 = (t5 * 4);   		// Starting address of data 
  /* Dump the data */
  goto stack_dump23177;   

stack_dump23178:
  if (_trace) printf("stack_dump23178:\n");
  t4 = *(s32 *)t3;   		// Get data word 
  t1 = t1 - 1;   
  t3 = t3 + 8;		// Advance SCA position 
		/* Save data word */
  *(u32 *)t2 = t4;
  t2 = t2 + 4;		// Advance VMA position 

stack_dump23177:
  if ((s64)t1 > 0)   
    goto stack_dump23178;
  /* Dump the tags */
  t1 = *(s32 *)&processor->scovdumpcount;   		// Restore the count 
  t2 = t5;		// Restore tag VMA 
  t4 = t1 << 3;   
  t3 = t3 - t4;   		// Restore orginal SCA 
  goto stack_dump23179;   

stack_dump23180:
  if (_trace) printf("stack_dump23180:\n");
  t1 = t1 - 1;   
  t4 = *(s32 *)(t3 + 4);   		// Get tag word 
  t3 = t3 + 8;		// Advance SCA position 
  t5 = LDQ_U(t2);   		// Get packed tags word 
  t4 = (t4 & 0xff) << ((t2&7)*8);   		// Position the new tag 
  t5 = t5 & ~(0xffL << (t2&7)*8);   		// Remove old tag 
  t5 = t4 | t5;		// Put in new byte 
  STQ_U(t2, t5);   		// Save packed tags word 
  t2 = t2 + 1;		// Advance VMA position 

stack_dump23179:
  if ((s64)t1 > 0)   
    goto stack_dump23180;
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
  goto stack_block_copy23181;   

stack_block_copy23182:
  if (_trace) printf("stack_block_copy23182:\n");
  t1 = t1 - 1;   
  t5 = *(u64 *)t2;   		// Get a word from source 
  t2 = t2 + 8;		// advance from position 
  *(u64 *)t3 = t5;   		// Put word in destination 
  t3 = t3 + 8;		// advance to position 

stack_block_copy23181:
  if ((s64)t1 > 0)   
    goto stack_block_copy23182;
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

abandon_frame_simple23184:
  if (_trace) printf("abandon_frame_simple23184:\n");
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
		/* Restore the control register */
  *(u32 *)&processor->control = t8;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
  iSP = iSP + 8;

catchopen2:
  if (_trace) printf("catchopen2:\n");
  t1 = Type_Locative;
		/* tag */
  *((u32 *)(&processor->catchblock)+1) = t1;
		/* data */
  *(u32 *)&processor->catchblock = t9;
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
    goto g23186;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g23186:
  if (_trace) printf("g23186:\n");
  /* Memory Read Internal */

vma_memory_read23187:
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
    goto vma_memory_read23189;

vma_memory_read23188:
  arg1 = zero + 224;   
  arg2 = arg2 >> (t7 & 63);   
  arg1 = arg1 >> (t7 & 63);   
  if (arg2 & 1)   
    goto vma_memory_read23191;

vma_memory_read23196:
  /* Memory Read Internal */

vma_memory_read23197:
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
    goto vma_memory_read23199;

vma_memory_read23198:
  arg1 = zero + 224;   
  arg2 = arg2 >> (t3 & 63);   
  arg1 = arg1 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (arg2 & 1)   
    goto vma_memory_read23201;

vma_memory_read23206:
  /* Memory Read Internal */

vma_memory_read23207:
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
    goto vma_memory_read23209;

vma_memory_read23208:
  arg1 = *(u64 *)&(processor->bindwrite_mask);   
  t11 = zero + 224;   
  arg1 = arg1 >> (t8 & 63);   
  t11 = t11 >> (t8 & 63);   
  if (arg1 & 1)   
    goto vma_memory_read23211;

vma_memory_read23216:
  /* Merge cdr-code */
  t9 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | t9;
  arg1 = t2 + ivory;
  t9 = (arg1 * 4);   
  t11 = LDQ_U(arg1);   
  arg2 = (t8 & 0xff) << ((arg1&7)*8);   
  t11 = t11 & ~(0xffL << (arg1&7)*8);   

force_alignment23219:
  if (_trace) printf("force_alignment23219:\n");
  t11 = t11 | arg2;
  arg2 = *(u64 *)&(processor->stackcachebasevma);   
  STQ_U(arg1, t11);   
  arg1 = *(s32 *)&processor->scovlimit;   
  arg2 = t2 - arg2;   		// Stack cache offset 
  arg1 = ((u64)arg2 < (u64)arg1) ? 1 : 0;   		// In range? 
  *(u32 *)t9 = t6;
  if (arg1 != 0)   		// J. if in cache 
    goto vma_memory_write23218;

vma_memory_write23217:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t1;
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
    goto check_preempt_request23220;
  *(u64 *)&processor->stop_interpreter = t3;   

check_preempt_request23220:
  if (_trace) printf("check_preempt_request23220:\n");

catchcloseld:
  if (_trace) printf("catchcloseld:\n");
  /* TagType. */
  t1 = arg5 & 63;
		/* tag */
  *((u32 *)(&processor->catchblock)+1) = t1;
  t2 = arg5 & 128;		// extra argument bit 
  t6 = *(u64 *)&(processor->extraandcatch);   		// mask for two bits 
  t2 = t2 << 1;   		// position in place for control register. 
		/* data */
  *(u32 *)&processor->catchblock = arg6;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t7;
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

vma_memory_write23218:
  if (_trace) printf("vma_memory_write23218:\n");
  arg1 = *(u64 *)&(processor->stackcachedata);   
  arg1 = (arg2 * 8) + arg1;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)arg1 = t6;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = t8;
  goto vma_memory_write23217;   

vma_memory_read23209:
  if (_trace) printf("vma_memory_read23209:\n");
  arg2 = *(u64 *)&(processor->stackcachedata);   
  arg1 = (arg1 * 8) + arg2;  		// reconstruct SCA 
  t9 = *(s32 *)arg1;   
  t8 = *(s32 *)(arg1 + 4);   		// Read from stack cache 
  goto vma_memory_read23208;   

vma_memory_read23211:
  if (_trace) printf("vma_memory_read23211:\n");
  if ((t11 & 1) == 0)   
    goto vma_memory_read23210;
  t2 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read23207;   

vma_memory_read23210:
  if (_trace) printf("vma_memory_read23210:\n");
  arg1 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t11 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + arg1;   		// Adjust for a longword load 
  arg1 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read23213:
  /* Perform memory action */
  arg1 = arg1;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read23199:
  if (_trace) printf("vma_memory_read23199:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read23198;   

vma_memory_read23201:
  if (_trace) printf("vma_memory_read23201:\n");
  if ((arg1 & 1) == 0)   
    goto vma_memory_read23200;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23197;   

vma_memory_read23200:
  if (_trace) printf("vma_memory_read23200:\n");
  arg2 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg1 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  arg1 = (arg1 * 4) + arg2;   		// Adjust for a longword load 
  arg2 = *(s32 *)arg1;   		// Get the memory action 

vma_memory_read23203:
  /* Perform memory action */
  arg1 = arg2;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read23189:
  if (_trace) printf("vma_memory_read23189:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read23188;   

vma_memory_read23191:
  if (_trace) printf("vma_memory_read23191:\n");
  if ((arg1 & 1) == 0)   
    goto vma_memory_read23190;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read23187;   

vma_memory_read23190:
  if (_trace) printf("vma_memory_read23190:\n");
  arg2 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg1 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  arg1 = (arg1 * 4) + arg2;   		// Adjust for a longword load 
  arg2 = *(s32 *)arg1;   		// Get the memory action 

vma_memory_read23193:
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
    goto iloop_decrement_tos23221;
  t3 = (s32)t2 - (s32)1;   
  t4 = ((s64)t3 < (s64)t2) ? 1 : 0;   
  if (t4 == 0) 
    goto iloop_decrement_tos23223;
  t6 = Type_Fixnum;
  *(u32 *)iSP = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t6;
  if ((s64)t3 <= 0)  
    goto NEXTINSTRUCTION;
  /* Here if branch taken. */
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

iloop_decrement_tos23221:
  if (_trace) printf("iloop_decrement_tos23221:\n");
  t3 = t1 - Type_Fixnum;   
  t3 = t3 & 56;		// Strip CDR code, low bits 
  if (t3 != 0)   
    goto iloop_decrement_tos23222;

iloop_decrement_tos23223:
  if (_trace) printf("iloop_decrement_tos23223:\n");
  arg5 = iPC + arg1;		// Compute next-pc 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto loopexception;

iloop_decrement_tos23222:
  if (_trace) printf("iloop_decrement_tos23222:\n");
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
    goto iloop_increment_tos_less_than23224;
  t4 = *(s32 *)(iSP + -8);   		// Get arg1. 
  t3 = *(s32 *)(iSP + -4);   
  t4 = (u32)t4;   
  t5 = t3 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than23225;
  t5 = (s32)t2 + (s32)1;
  t6 = ((s64)t2 <= (s64)t5) ? 1 : 0;   
  if (t6 == 0) 
    goto iloop_increment_tos_less_than23226;
  t6 = Type_Fixnum;
  *(u32 *)iSP = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t6;
  t6 = ((s64)t5 <= (s64)t4) ? 1 : 0;   
  if (t6 == 0) 
    goto NEXTINSTRUCTION;
  /* Here if branch taken. */

force_alignment23228:
  if (_trace) printf("force_alignment23228:\n");
  iPC = iPC + arg1;		// Update the PC in halfwords 
  if (arg2 != 0)   
    goto interpretinstructionpredicted;
  goto interpretinstructionforbranch;   

iloop_increment_tos_less_than23224:
  if (_trace) printf("iloop_increment_tos_less_than23224:\n");
  t5 = t1 - Type_Fixnum;   
  t5 = t5 & 56;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than23227;

iloop_increment_tos_less_than23225:
  if (_trace) printf("iloop_increment_tos_less_than23225:\n");
  t5 = t3 - Type_Fixnum;   
  t5 = t5 & 56;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto iloop_increment_tos_less_than23227;

iloop_increment_tos_less_than23226:
  if (_trace) printf("iloop_increment_tos_less_than23226:\n");
  arg5 = iPC + arg1;		// Compute next-pc 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto loopexception;

iloop_increment_tos_less_than23227:
  if (_trace) printf("iloop_increment_tos_less_than23227:\n");
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
  r0 = (u64)&&return0137;
  goto carinternal;
return0137:
  /* TagType. */
  arg5 = arg5 & 63;
  arg5 = arg5 | t2;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = arg5;
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
  r0 = (u64)&&return0138;
  goto cdrinternal;
return0138:
  /* TagType. */
  arg5 = arg5 & 63;
  arg5 = arg5 | t2;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = arg5;
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

vma_memory_read23229:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23231;

vma_memory_read23230:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23233;

vma_memory_read23240:
  /* TagType. */
  t1 = t1 & 63;
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = arg5;
  iSP = iSP + 8;
  t1 = t1 | t3;		// Put back the original CDR codes 
  *(u32 *)arg1 = arg6;
		/* write the stack cache */
  *(u32 *)(arg1 + 4) = arg5;
  goto NEXTINSTRUCTION;   

vma_memory_read23233:
  if (_trace) printf("vma_memory_read23233:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23232;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23229;   

vma_memory_read23232:
  if (_trace) printf("vma_memory_read23232:\n");

vma_memory_read23231:
  if (_trace) printf("vma_memory_read23231:\n");
  r0 = (u64)&&return0139;
  goto memoryreaddatadecode;
return0139:
  goto vma_memory_read23240;   

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
  goto carcdrloop23242;   

assoccdr:
  if (_trace) printf("assoccdr:\n");
  t6 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop or trap? 
  /* Move cdr to car for next carcdr-internal */
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6;

carcdrloop23242:
  if (_trace) printf("carcdrloop23242:\n");
  t5 = t1 - Type_NIL;   
  if (t6 != 0)   		// Asked to stop, check for sequence break 
    goto carcdrloop23241;
  if (t5 == 0) 
    goto carcdrloop23243;
  r0 = (u64)&&return0140;
  goto carcdrinternal;
return0140:
  t7 = t1 & 63;		// Strip off any CDR code bits. 
  t8 = (t7 == Type_List) ? 1 : 0;   

force_alignment23261:
  if (_trace) printf("force_alignment23261:\n");
  if (t8 == 0) 
    goto basic_dispatch23245;
  /* Here if argument TypeList */
  arg2 = t2;
  t3 = arg5;
  arg1 = arg6;
  /* Memory Read Internal */

vma_memory_read23246:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23248;

vma_memory_read23247:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23250;

vma_memory_read23257:
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto NEXTINSTRUCTION;   

basic_dispatch23245:
  if (_trace) printf("basic_dispatch23245:\n");
  t8 = (t7 == Type_NIL) ? 1 : 0;   

force_alignment23262:
  if (_trace) printf("force_alignment23262:\n");
  if (t8 == 0) 
    goto basic_dispatch23258;
  /* Here if argument TypeNIL */
  goto assoccdr;   

basic_dispatch23258:
  if (_trace) printf("basic_dispatch23258:\n");
  /* Here for all other cases */
  /* SetTag. */
  t1 = arg4 << 32;   
  t1 = arg5 | t1;
  arg5 = t1;
  arg2 = 14;
  goto illegaloperand;

basic_dispatch23244:
  if (_trace) printf("basic_dispatch23244:\n");

carcdrloop23243:
  if (_trace) printf("carcdrloop23243:\n");
  t1 = *(u64 *)&(processor->niladdress);   		// Return NIL 
  *(u64 *)iSP = t1;   		// push the data 
  goto NEXTINSTRUCTION;   

assocexc:
  if (_trace) printf("assocexc:\n");
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

vma_memory_read23250:
  if (_trace) printf("vma_memory_read23250:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23249;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23246;   

vma_memory_read23249:
  if (_trace) printf("vma_memory_read23249:\n");

vma_memory_read23248:
  if (_trace) printf("vma_memory_read23248:\n");
  r0 = (u64)&&return0141;
  goto memoryreaddatadecode;
return0141:
  goto vma_memory_read23257;   

carcdrloop23241:
  if (_trace) printf("carcdrloop23241:\n");
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
  goto carcdrloop23264;   

membercdr:
  if (_trace) printf("membercdr:\n");
  t6 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop or trap? 
  /* Move cdr to car for next carcdr-internal */
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6;

carcdrloop23264:
  if (_trace) printf("carcdrloop23264:\n");
  /* TagType. */
  t3 = t1 & 63;
  arg1 = t2;
  t5 = t1 - Type_NIL;   
  if (t6 != 0)   		// Asked to stop, check for sequence break 
    goto carcdrloop23263;
  if (t5 == 0) 
    goto carcdrloop23265;
  r0 = (u64)&&return0142;
  goto carcdrinternal;
return0142:
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t3;
  goto NEXTINSTRUCTION;   

carcdrloop23265:
  if (_trace) printf("carcdrloop23265:\n");
  t1 = *(u64 *)&(processor->niladdress);   		// Return NIL 
  *(u64 *)iSP = t1;   		// push the data 
  goto NEXTINSTRUCTION;   

memberexc:
  if (_trace) printf("memberexc:\n");
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

carcdrloop23263:
  if (_trace) printf("carcdrloop23263:\n");
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
  goto carcdrloop23267;   

rgetfcdr:
  if (_trace) printf("rgetfcdr:\n");
  r0 = (u64)&&return0143;
  goto cdrinternal;
return0143:
  t6 = *(u64 *)&(processor->stop_interpreter);   		// Have we been asked to stop or trap? 
  /* Move cdr to car for next carcdr-internal */
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6;

carcdrloop23267:
  if (_trace) printf("carcdrloop23267:\n");
  t5 = t1 - Type_NIL;   
  if (t6 != 0)   		// Asked to stop, check for sequence break 
    goto carcdrloop23266;
  if (t5 == 0) 
    goto carcdrloop23268;
  r0 = (u64)&&return0144;
  goto carcdrinternal;
return0144:
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
  r0 = (u64)&&return0145;
  goto carinternal;
return0145:
  /* TagType. */
  arg5 = arg5 & 63;		// Strip the CDR code 
  *(u32 *)iSP = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = arg5;
  arg2 = t1 & 63;		// set CDR-NEXT 
		/* Push the second result */
  *(u32 *)(iSP + 8) = t2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = arg2;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

carcdrloop23268:
  if (_trace) printf("carcdrloop23268:\n");
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

carcdrloop23266:
  if (_trace) printf("carcdrloop23266:\n");
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

vma_memory_read23272:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23274;

vma_memory_read23273:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  if (t10 & 1)   
    goto vma_memory_read23276;

vma_memory_read23281:
  t2 = t2 & Array_LengthMask;
  t5 = t2 - arg2;   
  if ((s64)t5 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg1 = arg1 + arg2;
  arg1 = arg1 + 1;
  /* Memory Read Internal */

vma_memory_read23282:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23284;

vma_memory_read23283:
  t9 = zero + 240;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read23286;

vma_memory_read23293:
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
    goto locate_instance0variable_mapped23271;

locate_instance0variable_mapped23270:
  if (_trace) printf("locate_instance0variable_mapped23270:\n");
  arg1 = arg1 + t3;

locate_instance0variable_mapped23269:
  if (_trace) printf("locate_instance0variable_mapped23269:\n");
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  iSP = iSP - 8;   		// Pop Stack. 
  t1 = (u32)t1;   
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read23294:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read23296;

vma_memory_read23295:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read23298;

vma_memory_read23304:
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

force_alignment23306:
  if (_trace) printf("force_alignment23306:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write23305;
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

vma_memory_write23305:
  if (_trace) printf("vma_memory_write23305:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read23296:
  if (_trace) printf("vma_memory_read23296:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read23295;   

vma_memory_read23298:
  if (_trace) printf("vma_memory_read23298:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read23297;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read23294;   

vma_memory_read23297:
  if (_trace) printf("vma_memory_read23297:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read23301:

vma_memory_read23300:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read23284:
  if (_trace) printf("vma_memory_read23284:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23283;   

vma_memory_read23286:
  if (_trace) printf("vma_memory_read23286:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23285;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23282;   

vma_memory_read23285:
  if (_trace) printf("vma_memory_read23285:\n");
  t10 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23290:
  if (_trace) printf("vma_memory_read23290:\n");
  t9 = t10 & MemoryActionTransform;
  if (t9 == 0) 
    goto vma_memory_read23289;
  t1 = t1 & ~63L;
  t1 = t1 | Type_ExternalValueCellPointer;
  goto vma_memory_read23293;   

vma_memory_read23289:

vma_memory_read23288:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23274:
  if (_trace) printf("vma_memory_read23274:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23273;   

vma_memory_read23276:
  if (_trace) printf("vma_memory_read23276:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23275;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23272;   

vma_memory_read23275:
  if (_trace) printf("vma_memory_read23275:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23278:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

locate_instance0variable_mapped23271:
  if (_trace) printf("locate_instance0variable_mapped23271:\n");
  t5 = arg1;
  /* Memory Read Internal */

vma_memory_read23307:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23309;

vma_memory_read23308:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read23311;

vma_memory_read23316:
  t5 = t5 - arg1;   
  if (t5 != 0)   
    goto locate_instance0variable_mapped23270;
  /* TagType. */
  t6 = t6 & 63;
  t6 = t6 | 64;		// Set CDR code to 1 
		/* Update self */
  *(u32 *)(iFP + 24) = arg1;
		/* write the stack cache */
  *(u32 *)(iFP + 28) = t6;
  goto locate_instance0variable_mapped23270;   

vma_memory_read23309:
  if (_trace) printf("vma_memory_read23309:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23308;   

vma_memory_read23311:
  if (_trace) printf("vma_memory_read23311:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23310;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23307;   

vma_memory_read23310:
  if (_trace) printf("vma_memory_read23310:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23313:
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

vma_memory_read23320:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23322;

vma_memory_read23321:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  if (t10 & 1)   
    goto vma_memory_read23324;

vma_memory_read23329:
  t2 = t2 & Array_LengthMask;
  t5 = t2 - arg2;   
  if ((s64)t5 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg1 = arg1 + arg2;
  arg1 = arg1 + 1;
  /* Memory Read Internal */

vma_memory_read23330:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23332;

vma_memory_read23331:
  t9 = zero + 240;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read23334;

vma_memory_read23341:
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
    goto locate_instance0variable_mapped23319;

locate_instance0variable_mapped23318:
  if (_trace) printf("locate_instance0variable_mapped23318:\n");
  arg1 = arg1 + t3;

locate_instance0variable_mapped23317:
  if (_trace) printf("locate_instance0variable_mapped23317:\n");
  t1 = *(s32 *)iSP;   
  t2 = *(s32 *)(iSP + 4);   
  t1 = (u32)t1;   
  arg5 = *(u64 *)&(processor->stackcachebasevma);   
  arg6 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  /* Memory Read Internal */

vma_memory_read23342:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read23344;

vma_memory_read23343:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read23346;

vma_memory_read23352:
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

force_alignment23354:
  if (_trace) printf("force_alignment23354:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write23353;
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

vma_memory_write23353:
  if (_trace) printf("vma_memory_write23353:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read23344:
  if (_trace) printf("vma_memory_read23344:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read23343;   

vma_memory_read23346:
  if (_trace) printf("vma_memory_read23346:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read23345;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read23342;   

vma_memory_read23345:
  if (_trace) printf("vma_memory_read23345:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read23349:

vma_memory_read23348:
  /* Perform memory action */
  arg1 = t9;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read23332:
  if (_trace) printf("vma_memory_read23332:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23331;   

vma_memory_read23334:
  if (_trace) printf("vma_memory_read23334:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23333;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23330;   

vma_memory_read23333:
  if (_trace) printf("vma_memory_read23333:\n");
  t10 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23338:
  if (_trace) printf("vma_memory_read23338:\n");
  t9 = t10 & MemoryActionTransform;
  if (t9 == 0) 
    goto vma_memory_read23337;
  t1 = t1 & ~63L;
  t1 = t1 | Type_ExternalValueCellPointer;
  goto vma_memory_read23341;   

vma_memory_read23337:

vma_memory_read23336:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23322:
  if (_trace) printf("vma_memory_read23322:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23321;   

vma_memory_read23324:
  if (_trace) printf("vma_memory_read23324:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23323;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23320;   

vma_memory_read23323:
  if (_trace) printf("vma_memory_read23323:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23326:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

locate_instance0variable_mapped23319:
  if (_trace) printf("locate_instance0variable_mapped23319:\n");
  t5 = arg1;
  /* Memory Read Internal */

vma_memory_read23355:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23357;

vma_memory_read23356:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read23359;

vma_memory_read23364:
  t5 = t5 - arg1;   
  if (t5 != 0)   
    goto locate_instance0variable_mapped23318;
  /* TagType. */
  t6 = t6 & 63;
  t6 = t6 | 64;		// Set CDR code to 1 
		/* Update self */
  *(u32 *)(iFP + 24) = arg1;
		/* write the stack cache */
  *(u32 *)(iFP + 28) = t6;
  goto locate_instance0variable_mapped23318;   

vma_memory_read23357:
  if (_trace) printf("vma_memory_read23357:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23356;   

vma_memory_read23359:
  if (_trace) printf("vma_memory_read23359:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23358;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23355;   

vma_memory_read23358:
  if (_trace) printf("vma_memory_read23358:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23361:
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

vma_memory_read23368:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23370;

vma_memory_read23369:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  if (t10 & 1)   
    goto vma_memory_read23372;

vma_memory_read23377:
  t2 = t2 & Array_LengthMask;
  t5 = t2 - arg2;   
  if ((s64)t5 <= 0)  		// J. if mapping-table-index-out-of-bounds 
    goto ivbadindex;
  arg1 = arg1 + arg2;
  arg1 = arg1 + 1;
  /* Memory Read Internal */

vma_memory_read23378:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->dataread_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23380;

vma_memory_read23379:
  t9 = zero + 240;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read23382;

vma_memory_read23389:
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
    goto locate_instance0variable_mapped23367;

locate_instance0variable_mapped23366:
  if (_trace) printf("locate_instance0variable_mapped23366:\n");
  arg1 = arg1 + t3;

locate_instance0variable_mapped23365:
  if (_trace) printf("locate_instance0variable_mapped23365:\n");
  t7 = Type_Locative;
  *(u32 *)(iSP + 8) = arg1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t7;
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

vma_memory_read23380:
  if (_trace) printf("vma_memory_read23380:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23379;   

vma_memory_read23382:
  if (_trace) printf("vma_memory_read23382:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23381;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23378;   

vma_memory_read23381:
  if (_trace) printf("vma_memory_read23381:\n");
  t10 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23386:
  if (_trace) printf("vma_memory_read23386:\n");
  t9 = t10 & MemoryActionTransform;
  if (t9 == 0) 
    goto vma_memory_read23385;
  t1 = t1 & ~63L;
  t1 = t1 | Type_ExternalValueCellPointer;
  goto vma_memory_read23389;   

vma_memory_read23385:

vma_memory_read23384:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23370:
  if (_trace) printf("vma_memory_read23370:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23369;   

vma_memory_read23372:
  if (_trace) printf("vma_memory_read23372:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23371;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23368;   

vma_memory_read23371:
  if (_trace) printf("vma_memory_read23371:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23374:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 6;
  goto performmemoryaction;

locate_instance0variable_mapped23367:
  if (_trace) printf("locate_instance0variable_mapped23367:\n");
  t5 = arg1;
  /* Memory Read Internal */

vma_memory_read23390:
  t9 = arg1 + ivory;
  t2 = (t9 * 4);   
  t1 = LDQ_U(t9);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t10 = *(u64 *)&(processor->header_mask);   
  t8 = ((u64)t7 < (u64)arg6) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t9&7)*8));   
  if (t8 != 0)   
    goto vma_memory_read23392;

vma_memory_read23391:
  t9 = zero + 64;   
  t10 = t10 >> (t1 & 63);   
  t9 = t9 >> (t1 & 63);   
  t2 = (u32)t2;   
  if (t10 & 1)   
    goto vma_memory_read23394;

vma_memory_read23399:
  t5 = t5 - arg1;   
  if (t5 != 0)   
    goto locate_instance0variable_mapped23366;
  /* TagType. */
  t6 = t6 & 63;
  t6 = t6 | 64;		// Set CDR code to 1 
		/* Update self */
  *(u32 *)(iFP + 24) = arg1;
		/* write the stack cache */
  *(u32 *)(iFP + 28) = t6;
  goto locate_instance0variable_mapped23366;   

vma_memory_read23392:
  if (_trace) printf("vma_memory_read23392:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t2 = *(s32 *)t7;   
  t1 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23391;   

vma_memory_read23394:
  if (_trace) printf("vma_memory_read23394:\n");
  if ((t9 & 1) == 0)   
    goto vma_memory_read23393;
  arg1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23390;   

vma_memory_read23393:
  if (_trace) printf("vma_memory_read23393:\n");
  t10 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t9 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t9 = (t9 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t9;   		// Get the memory action 

vma_memory_read23396:
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

vma_memory_read23400:
  t6 = arg1 + ivory;
  t1 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = arg1 - arg5;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->dataread_mask);   
  t5 = ((u64)t4 < (u64)arg6) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read23402;

vma_memory_read23401:
  t6 = zero + 240;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read23404;

vma_memory_read23411:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = t2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t7;
  iSP = iSP + 8;
  goto cachevalid;   

vma_memory_read23402:
  if (_trace) printf("vma_memory_read23402:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read23401;   

vma_memory_read23404:
  if (_trace) printf("vma_memory_read23404:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read23403;
  arg1 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read23400;   

vma_memory_read23403:
  if (_trace) printf("vma_memory_read23403:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read23408:
  if (_trace) printf("vma_memory_read23408:\n");
  t6 = t7 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read23407;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read23411;   

vma_memory_read23407:

vma_memory_read23406:
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

vma_memory_read23412:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read23414;

vma_memory_read23413:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read23416;

vma_memory_read23422:
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

force_alignment23424:
  if (_trace) printf("force_alignment23424:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write23423;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write23423:
  if (_trace) printf("vma_memory_write23423:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read23414:
  if (_trace) printf("vma_memory_read23414:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read23413;   

vma_memory_read23416:
  if (_trace) printf("vma_memory_read23416:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read23415;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read23412;   

vma_memory_read23415:
  if (_trace) printf("vma_memory_read23415:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read23419:

vma_memory_read23418:
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

vma_memory_read23425:
  t8 = arg1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = arg1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read23427;

vma_memory_read23426:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read23429;

vma_memory_read23435:
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

force_alignment23437:
  if (_trace) printf("force_alignment23437:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write23436;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write23436:
  if (_trace) printf("vma_memory_write23436:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read23427:
  if (_trace) printf("vma_memory_read23427:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read23426;   

vma_memory_read23429:
  if (_trace) printf("vma_memory_read23429:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read23428;
  arg1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read23425;   

vma_memory_read23428:
  if (_trace) printf("vma_memory_read23428:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read23432:

vma_memory_read23431:
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t7;
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

vma_memory_read23438:
  t7 = arg4 + ivory;
  t1 = (t7 * 4);   
  t2 = LDQ_U(t7);   
  t5 = arg4 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23440;

vma_memory_read23439:
  t7 = zero + 64;   
  t8 = t8 >> (t2 & 63);   
  t7 = t7 >> (t2 & 63);   
  t1 = (u32)t1;   
  if (t8 & 1)   
    goto vma_memory_read23442;

vma_memory_read23447:
  t1 = t1 - 1;   
  /* Memory Read Internal */

vma_memory_read23448:
  t7 = t1 + ivory;
  t2 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = t1 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23450;

vma_memory_read23449:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23452;

vma_memory_read23459:
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

vma_memory_read23460:
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
    goto vma_memory_read23462;

vma_memory_read23461:
  t6 = zero + 240;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read23464;

vma_memory_read23471:
  t2 = t2 & 63;		// set CDR-NEXT 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)iSP = t1;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t2;
  goto cachevalid;   

vma_memory_read23462:
  if (_trace) printf("vma_memory_read23462:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read23461;   

vma_memory_read23464:
  if (_trace) printf("vma_memory_read23464:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read23463;
  arg5 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read23460;   

vma_memory_read23463:
  if (_trace) printf("vma_memory_read23463:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg5;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read23468:
  if (_trace) printf("vma_memory_read23468:\n");
  t6 = t7 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read23467;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read23471;   

vma_memory_read23467:

vma_memory_read23466:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23450:
  if (_trace) printf("vma_memory_read23450:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t2 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23449;   

vma_memory_read23452:
  if (_trace) printf("vma_memory_read23452:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23451;
  t1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23448;   

vma_memory_read23451:
  if (_trace) printf("vma_memory_read23451:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23456:
  if (_trace) printf("vma_memory_read23456:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read23455;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read23459;   

vma_memory_read23455:

vma_memory_read23454:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23440:
  if (_trace) printf("vma_memory_read23440:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t1 = *(s32 *)t5;   
  t2 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23439;   

vma_memory_read23442:
  if (_trace) printf("vma_memory_read23442:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23441;
  arg4 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read23438;   

vma_memory_read23441:
  if (_trace) printf("vma_memory_read23441:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23444:
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

vma_memory_read23472:
  t7 = arg4 + ivory;
  t1 = (t7 * 4);   
  t2 = LDQ_U(t7);   
  t5 = arg4 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23474;

vma_memory_read23473:
  t7 = zero + 64;   
  t8 = t8 >> (t2 & 63);   
  t7 = t7 >> (t2 & 63);   
  t1 = (u32)t1;   
  if (t8 & 1)   
    goto vma_memory_read23476;

vma_memory_read23481:
  t1 = t1 - 1;   
  /* Memory Read Internal */

vma_memory_read23482:
  t7 = t1 + ivory;
  t2 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = t1 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23484;

vma_memory_read23483:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23486;

vma_memory_read23493:
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

vma_memory_read23494:
  t7 = arg5 + ivory;
  t4 = (t7 * 4);   
  t3 = LDQ_U(t7);   
  t5 = arg5 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t4 = *(s32 *)t4;   
  t3 = (u8)(t3 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23496;

vma_memory_read23495:
  t7 = zero + 240;   
  t8 = t8 >> (t3 & 63);   
  t7 = t7 >> (t3 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23498;

vma_memory_read23504:
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

force_alignment23506:
  if (_trace) printf("force_alignment23506:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)t4 = t1;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write23505;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

ivrefbadinst3:
  if (_trace) printf("ivrefbadinst3:\n");
  arg5 = 0;
  arg2 = 4;
  goto illegaloperand;

vma_memory_write23505:
  if (_trace) printf("vma_memory_write23505:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t6 = arg5 - t11;   		// Stack cache offset 
  t5 = (t6 * 8) + t5;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t5 = t1;
		/* write the stack cache */
  *(u32 *)(t5 + 4) = t3;
  goto NEXTINSTRUCTION;   

vma_memory_read23496:
  if (_trace) printf("vma_memory_read23496:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t4 = *(s32 *)t5;   
  t3 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23495;   

vma_memory_read23498:
  if (_trace) printf("vma_memory_read23498:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23497;
  arg5 = (u32)t4;   		// Do the indirect thing 
  goto vma_memory_read23494;   

vma_memory_read23497:
  if (_trace) printf("vma_memory_read23497:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg5;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23501:

vma_memory_read23500:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read23484:
  if (_trace) printf("vma_memory_read23484:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t2 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23483;   

vma_memory_read23486:
  if (_trace) printf("vma_memory_read23486:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23485;
  t1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23482;   

vma_memory_read23485:
  if (_trace) printf("vma_memory_read23485:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23490:
  if (_trace) printf("vma_memory_read23490:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read23489;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read23493;   

vma_memory_read23489:

vma_memory_read23488:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23474:
  if (_trace) printf("vma_memory_read23474:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t1 = *(s32 *)t5;   
  t2 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23473;   

vma_memory_read23476:
  if (_trace) printf("vma_memory_read23476:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23475;
  arg4 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read23472;   

vma_memory_read23475:
  if (_trace) printf("vma_memory_read23475:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23478:
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

vma_memory_read23507:
  t7 = arg4 + ivory;
  t1 = (t7 * 4);   
  t2 = LDQ_U(t7);   
  t5 = arg4 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->header_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t1 = *(s32 *)t1;   
  t2 = (u8)(t2 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23509;

vma_memory_read23508:
  t7 = zero + 64;   
  t8 = t8 >> (t2 & 63);   
  t7 = t7 >> (t2 & 63);   
  t1 = (u32)t1;   
  if (t8 & 1)   
    goto vma_memory_read23511;

vma_memory_read23516:
  t1 = t1 - 1;   
  /* Memory Read Internal */

vma_memory_read23517:
  t7 = t1 + ivory;
  t2 = (t7 * 4);   
  t4 = LDQ_U(t7);   
  t5 = t1 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t4 = (u8)(t4 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read23519;

vma_memory_read23518:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read23521;

vma_memory_read23528:
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t7;
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

vma_memory_read23519:
  if (_trace) printf("vma_memory_read23519:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t2 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23518;   

vma_memory_read23521:
  if (_trace) printf("vma_memory_read23521:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23520;
  t1 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23517;   

vma_memory_read23520:
  if (_trace) printf("vma_memory_read23520:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23525:
  if (_trace) printf("vma_memory_read23525:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read23524;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read23528;   

vma_memory_read23524:

vma_memory_read23523:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23509:
  if (_trace) printf("vma_memory_read23509:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t1 = *(s32 *)t5;   
  t2 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23508;   

vma_memory_read23511:
  if (_trace) printf("vma_memory_read23511:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23510;
  arg4 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read23507;   

vma_memory_read23510:
  if (_trace) printf("vma_memory_read23510:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23513:
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

force_alignment23534:
  if (_trace) printf("force_alignment23534:\n");
  if (t4 == 0) 
    goto basic_dispatch23530;
  /* Here if argument TypeFixnum */
  t2 = (s32)arg6 - (s32)t2;   
  arg2 = (s32)zero - (s32)arg6;   
  if (t2 == 0) 
    goto unaryminusexc;
  iPC = t6;
		/* Semi-cheat, we know t5 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 12) = t5;
  iCP = t7;
		/* Push the data */
  *(u32 *)(iSP + 8) = arg2;
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch23530:
  if (_trace) printf("basic_dispatch23530:\n");
  t4 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment23535:
  if (_trace) printf("force_alignment23535:\n");
  if (t4 == 0) 
    goto basic_dispatch23531;
  /* Here if argument TypeSingleFloat */
  /* NIL */
  SUBS(0, f0, 3, f31, 1, f1); /* subs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = t6;
		/* Semi-cheat, we know t5 has CDRNext/TypeSingleFloat */
  *(u32 *)(iSP + 12) = t5;
  iCP = t7;
  STS( (u32 *)(iSP + 8), 0, f0 );   		// Push the data 
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch23531:
  if (_trace) printf("basic_dispatch23531:\n");
  /* Here for all other cases */

unaryminusexc:
  if (_trace) printf("unaryminusexc:\n");
  arg6 = arg5;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto unarynumericexception;

basic_dispatch23529:
  if (_trace) printf("basic_dispatch23529:\n");

DoUnaryMinusIM:
  if (_trace) printf("DoUnaryMinusIM:\n");
  arg2 = (s32)zero - (s32)arg2;   		// Negate the 8 bit immediate operand 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t7 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t7;
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

force_alignment23575:
  if (_trace) printf("force_alignment23575:\n");
  if (t10 == 0) 
    goto basic_dispatch23546;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23552:
  if (_trace) printf("force_alignment23552:\n");
  if (t12 == 0) 
    goto basic_dispatch23548;
  /* Here if argument TypeFixnum */
  t6 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t5 = (s64)((s32)t2 * (s64)(s32)t4); /* mull/v */   		// compute 64-bit result 
  if (t5 >> 32)
    exception();  // WARNING !!! THIS IS ADJUSTED BY THE DIFF FILE
  t7 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
		/* Semi-cheat, we know temp2 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 4) = t9;
  iPC = t6;
  *(u32 *)iSP = t5;
  iCP = t7;
  goto cachevalid;   

basic_dispatch23548:
  if (_trace) printf("basic_dispatch23548:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23553:
  if (_trace) printf("force_alignment23553:\n");
  if (t12 == 0) 
    goto basic_dispatch23549;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation23536;   

basic_dispatch23549:
  if (_trace) printf("basic_dispatch23549:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23554:
  if (_trace) printf("force_alignment23554:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23543;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto simple_binary_arithmetic_operation23539;   

basic_dispatch23547:
  if (_trace) printf("basic_dispatch23547:\n");

basic_dispatch23546:
  if (_trace) printf("basic_dispatch23546:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment23576:
  if (_trace) printf("force_alignment23576:\n");
  if (t10 == 0) 
    goto basic_dispatch23555;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23561:
  if (_trace) printf("force_alignment23561:\n");
  if (t12 == 0) 
    goto basic_dispatch23557;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation23536:
  if (_trace) printf("simple_binary_arithmetic_operation23536:\n");
  MULS(0, f0, 1, f1, 2, f2); /* muls */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  goto cachevalid;   

basic_dispatch23557:
  if (_trace) printf("basic_dispatch23557:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23562:
  if (_trace) printf("force_alignment23562:\n");
  if (t12 == 0) 
    goto basic_dispatch23558;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation23536;   

basic_dispatch23558:
  if (_trace) printf("basic_dispatch23558:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23563:
  if (_trace) printf("force_alignment23563:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23543;
  /* Here if argument TypeDoubleFloat */

simple_binary_arithmetic_operation23539:
  if (_trace) printf("simple_binary_arithmetic_operation23539:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto simple_binary_arithmetic_operation23540;   

basic_dispatch23556:
  if (_trace) printf("basic_dispatch23556:\n");

basic_dispatch23555:
  if (_trace) printf("basic_dispatch23555:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23577:
  if (_trace) printf("force_alignment23577:\n");
  if (t10 == 0) 
    goto basic_dispatch23564;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23570:
  if (_trace) printf("force_alignment23570:\n");
  if (t12 == 0) 
    goto basic_dispatch23566;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0146;
  goto fetchdoublefloat;
return0146:
  LDT(1, f1, processor->fp0);   

simple_binary_arithmetic_operation23540:
  if (_trace) printf("simple_binary_arithmetic_operation23540:\n");
  arg2 = (u32)t4;   
  r0 = (u64)&&return0147;
  goto fetchdoublefloat;
return0147:
  LDT(2, f2, processor->fp0);   

simple_binary_arithmetic_operation23537:
  if (_trace) printf("simple_binary_arithmetic_operation23537:\n");
  MULT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0148;
  goto consdoublefloat;
return0148:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  goto cachevalid;   

basic_dispatch23566:
  if (_trace) printf("basic_dispatch23566:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23571:
  if (_trace) printf("force_alignment23571:\n");
  if (t12 == 0) 
    goto basic_dispatch23567;
  /* Here if argument TypeSingleFloat */

simple_binary_arithmetic_operation23538:
  if (_trace) printf("simple_binary_arithmetic_operation23538:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  r0 = (u64)&&return0149;
  goto fetchdoublefloat;
return0149:
  LDT(1, f1, processor->fp0);   
  goto simple_binary_arithmetic_operation23537;   

basic_dispatch23567:
  if (_trace) printf("basic_dispatch23567:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23572:
  if (_trace) printf("force_alignment23572:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23543;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto simple_binary_arithmetic_operation23538;   

basic_dispatch23565:
  if (_trace) printf("basic_dispatch23565:\n");

basic_dispatch23564:
  if (_trace) printf("basic_dispatch23564:\n");
  /* Here for all other cases */

binary_type_dispatch23542:
  if (_trace) printf("binary_type_dispatch23542:\n");

domulovfl:
  if (_trace) printf("domulovfl:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch23544;   

binary_type_dispatch23543:
  if (_trace) printf("binary_type_dispatch23543:\n");
  t1 = t3;
  goto domulovfl;   

binary_type_dispatch23544:
  if (_trace) printf("binary_type_dispatch23544:\n");

basic_dispatch23545:
  if (_trace) printf("basic_dispatch23545:\n");

DoMultiplyIM:
  if (_trace) printf("DoMultiplyIM:\n");
  arg2 = arg2 << 56;   
  t1 = (u32)(arg6 >> ((4&7)*8));   
  t2 = (s32)arg6;		// get ARG1 tag/data 
  arg2 = (s64)arg2 >> 56;   
  t11 = t1 & 63;		// Strip off any CDR code bits. 
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23582:
  if (_trace) printf("force_alignment23582:\n");
  if (t12 == 0) 
    goto basic_dispatch23579;
  /* Here if argument TypeFixnum */
  t3 = t2 * arg2;   		// compute 64-bit result 
  t4 = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  t10 = (s32)t3;		// compute 32-bit sign-extended result 
  t5 = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t10 = (t3 == t10) ? 1 : 0;   		// is it the same as the 64-bit result? 
  if (t10 == 0) 		// if not, we overflowed 
    goto domulovfl;
		/* Semi-cheat, we know temp2 has CDRNext/TypeFixnum */
  *(u32 *)(iSP + 4) = t11;
  iPC = t4;
  *(u32 *)iSP = t3;
  iCP = t5;
  goto cachevalid;   

basic_dispatch23579:
  if (_trace) printf("basic_dispatch23579:\n");
  /* Here for all other cases */
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  arg2 = zero;
  goto begindomultiply;   

basic_dispatch23578:
  if (_trace) printf("basic_dispatch23578:\n");

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

force_alignment23620:
  if (_trace) printf("force_alignment23620:\n");
  if (t10 == 0) 
    goto basic_dispatch23593;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23599:
  if (_trace) printf("force_alignment23599:\n");
  if (t12 == 0) 
    goto basic_dispatch23595;
  /* Here if argument TypeFixnum */
  CVTLQ(1, f1, f31, 1, f1);
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(1, f1, f31, 1, f1);
  CVTQT(2, f2, f31, 2, f2);

basic_dispatch23594:
  if (_trace) printf("basic_dispatch23594:\n");

basic_dispatch23592:
  if (_trace) printf("basic_dispatch23592:\n");

binary_arithmetic_division_prelude23583:
  if (_trace) printf("binary_arithmetic_division_prelude23583:\n");
  sp = sp + 8;   
  goto *r0; /* ret */

basic_dispatch23593:
  if (_trace) printf("basic_dispatch23593:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment23621:
  if (_trace) printf("force_alignment23621:\n");
  if (t10 == 0) 
    goto basic_dispatch23600;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23606:
  if (_trace) printf("force_alignment23606:\n");
  if (t12 != 0)   
    goto binary_arithmetic_division_prelude23583;

basic_dispatch23602:
  if (_trace) printf("basic_dispatch23602:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23607:
  if (_trace) printf("force_alignment23607:\n");
  if (t12 == 0) 
    goto basic_dispatch23603;
  /* Here if argument TypeFixnum */
  t3 = t1;		// contagion 
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto binary_arithmetic_division_prelude23583;   

basic_dispatch23603:
  if (_trace) printf("basic_dispatch23603:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23608:
  if (_trace) printf("force_alignment23608:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23590;
  /* Here if argument TypeDoubleFloat */

binary_arithmetic_division_prelude23585:
  if (_trace) printf("binary_arithmetic_division_prelude23585:\n");
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  goto binary_arithmetic_division_prelude23586;   

basic_dispatch23601:
  if (_trace) printf("basic_dispatch23601:\n");

basic_dispatch23600:
  if (_trace) printf("basic_dispatch23600:\n");
  t10 = (t9 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23622:
  if (_trace) printf("force_alignment23622:\n");
  if (t10 == 0) 
    goto basic_dispatch23609;
  /* Here if argument TypeDoubleFloat */
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23615:
  if (_trace) printf("force_alignment23615:\n");
  if (t12 == 0) 
    goto basic_dispatch23611;
  /* Here if argument TypeDoubleFloat */
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0150;
  goto fetchdoublefloat;
return0150:
  r0 = *(u64 *)sp;   
  LDT(1, f1, processor->fp0);   

binary_arithmetic_division_prelude23586:
  if (_trace) printf("binary_arithmetic_division_prelude23586:\n");
  arg2 = (u32)t4;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0151;
  goto fetchdoublefloat;
return0151:
  r0 = *(u64 *)sp;   
  LDT(2, f2, processor->fp0);   
  goto binary_arithmetic_division_prelude23583;   

basic_dispatch23611:
  if (_trace) printf("basic_dispatch23611:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23616:
  if (_trace) printf("force_alignment23616:\n");
  if (t12 == 0) 
    goto basic_dispatch23612;
  /* Here if argument TypeSingleFloat */

binary_arithmetic_division_prelude23584:
  if (_trace) printf("binary_arithmetic_division_prelude23584:\n");
  t3 = t1;		// contagion 
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  t12 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  arg2 = (u32)t2;   
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0152;
  goto fetchdoublefloat;
return0152:
  r0 = *(u64 *)sp;   
  LDT(1, f1, processor->fp0);   
  goto binary_arithmetic_division_prelude23583;   

basic_dispatch23612:
  if (_trace) printf("basic_dispatch23612:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23617:
  if (_trace) printf("force_alignment23617:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23590;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQT(2, f2, f31, 2, f2);
  goto binary_arithmetic_division_prelude23584;   

basic_dispatch23610:
  if (_trace) printf("basic_dispatch23610:\n");

basic_dispatch23609:
  if (_trace) printf("basic_dispatch23609:\n");
  /* Here for all other cases */

binary_type_dispatch23589:
  if (_trace) printf("binary_type_dispatch23589:\n");

binary_arithmetic_division_prelude23587:
  if (_trace) printf("binary_arithmetic_division_prelude23587:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch23591;   

binary_type_dispatch23590:
  if (_trace) printf("binary_type_dispatch23590:\n");
  t1 = t3;
  goto binary_arithmetic_division_prelude23587;   

binary_type_dispatch23591:
  if (_trace) printf("binary_type_dispatch23591:\n");

basic_dispatch23595:
  if (_trace) printf("basic_dispatch23595:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23623:
  if (_trace) printf("force_alignment23623:\n");
  if (t12 == 0) 
    goto basic_dispatch23596;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto binary_arithmetic_division_prelude23583;   

basic_dispatch23596:
  if (_trace) printf("basic_dispatch23596:\n");
  t12 = (t11 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23624:
  if (_trace) printf("force_alignment23624:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23590;
  /* Here if argument TypeDoubleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQT(1, f1, f31, 1, f1);
  goto binary_arithmetic_division_prelude23585;   

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
  r0 = (u64)&&return0153;
  goto binaryarithmeticdivisionprelude;
return0153:
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment23630:
  if (_trace) printf("force_alignment23630:\n");
  if (t9 == 0) 
    goto basic_dispatch23626;
  /* Here if argument TypeFixnum */
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVC(0, f0, f31, 0, f0);
  CVTQLV(0, f0, f31, 0, f0);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   

basic_dispatch23625:
  if (_trace) printf("basic_dispatch23625:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch23626:
  if (_trace) printf("basic_dispatch23626:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment23631:
  if (_trace) printf("force_alignment23631:\n");
  if (t9 == 0) 
    goto basic_dispatch23627;
  /* Here if argument TypeSingleFloat */
  DIVS(0, f0, 1, f1, 2, f2); /* divs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  goto basic_dispatch23625;   

basic_dispatch23627:
  if (_trace) printf("basic_dispatch23627:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23632:
  if (_trace) printf("force_alignment23632:\n");
  if (t9 == 0) 
    goto basic_dispatch23625;
  /* Here if argument TypeDoubleFloat */
  DIVT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0154;
  goto consdoublefloat;
return0154:
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  goto basic_dispatch23625;   

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
  r0 = (u64)&&return0155;
  goto binaryarithmeticdivisionprelude;
return0155:
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment23638:
  if (_trace) printf("force_alignment23638:\n");
  if (t9 == 0) 
    goto basic_dispatch23634;
  /* Here if argument TypeFixnum */
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQSVI(0, f0, f31, 0, f0);
  CVTQLV(0, f0, f31, 0, f0);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   

basic_dispatch23633:
  if (_trace) printf("basic_dispatch23633:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch23634:
  if (_trace) printf("basic_dispatch23634:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment23639:
  if (_trace) printf("force_alignment23639:\n");
  if (t9 == 0) 
    goto basic_dispatch23635;
  /* Here if argument TypeSingleFloat */
  DIVS(0, f0, 1, f1, 2, f2); /* divs */   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  goto basic_dispatch23633;   

basic_dispatch23635:
  if (_trace) printf("basic_dispatch23635:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23640:
  if (_trace) printf("force_alignment23640:\n");
  if (t9 == 0) 
    goto basic_dispatch23633;
  /* Here if argument TypeDoubleFloat */
  DIVT(0, f0, 1, f1, 2, f2);   
  STT( (u64 *)&processor->fp0, 0, f0 );   
  r0 = (u64)&&return0156;
  goto consdoublefloat;
return0156:
  t8 = Type_DoubleFloat;
  *(u32 *)iSP = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  goto basic_dispatch23633;   

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
  r0 = (u64)&&return0157;
  goto binaryarithmeticdivisionprelude;
return0157:
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVM(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment23646:
  if (_trace) printf("force_alignment23646:\n");
  if (t9 == 0) 
    goto basic_dispatch23642;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch23641:
  if (_trace) printf("basic_dispatch23641:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch23642:
  if (_trace) printf("basic_dispatch23642:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment23647:
  if (_trace) printf("force_alignment23647:\n");
  if (t9 == 0) 
    goto basic_dispatch23643;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch23641;   

basic_dispatch23643:
  if (_trace) printf("basic_dispatch23643:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23648:
  if (_trace) printf("force_alignment23648:\n");
  if (t9 == 0) 
    goto basic_dispatch23641;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0158;
  goto consdoublefloat;
return0158:
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto basic_dispatch23641;   

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
  r0 = (u64)&&return0159;
  goto binaryarithmeticdivisionprelude;
return0159:
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

force_alignment23654:
  if (_trace) printf("force_alignment23654:\n");
  if (t9 == 0) 
    goto basic_dispatch23650;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch23649:
  if (_trace) printf("basic_dispatch23649:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch23650:
  if (_trace) printf("basic_dispatch23650:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment23655:
  if (_trace) printf("force_alignment23655:\n");
  if (t9 == 0) 
    goto basic_dispatch23651;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch23649;   

basic_dispatch23651:
  if (_trace) printf("basic_dispatch23651:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23656:
  if (_trace) printf("force_alignment23656:\n");
  if (t9 == 0) 
    goto basic_dispatch23649;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0160;
  goto consdoublefloat;
return0160:
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto basic_dispatch23649;   

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
  r0 = (u64)&&return0161;
  goto binaryarithmeticdivisionprelude;
return0161:
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQVC(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment23662:
  if (_trace) printf("force_alignment23662:\n");
  if (t9 == 0) 
    goto basic_dispatch23658;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch23657:
  if (_trace) printf("basic_dispatch23657:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch23658:
  if (_trace) printf("basic_dispatch23658:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment23663:
  if (_trace) printf("force_alignment23663:\n");
  if (t9 == 0) 
    goto basic_dispatch23659;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch23657;   

basic_dispatch23659:
  if (_trace) printf("basic_dispatch23659:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23664:
  if (_trace) printf("force_alignment23664:\n");
  if (t9 == 0) 
    goto basic_dispatch23657;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0162;
  goto consdoublefloat;
return0162:
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto basic_dispatch23657;   

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
  r0 = (u64)&&return0163;
  goto binaryarithmeticdivisionprelude;
return0163:
  DIVT(0, f0, 1, f1, 2, f2);   
  CVTTQV(0, f0, f31, 0, f0);
  CVTQT(3, f3, f31, 0, f0);
  MULT(3, f3, 3, f3, 2, f2);   
  SUBT(3, f3, 1, f1, 3, f3);   
  CVTQLV(0, f0, f31, 0, f0);
  t8 = t3 & 63;		// Strip off any CDR code bits. 
  t9 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment23670:
  if (_trace) printf("force_alignment23670:\n");
  if (t9 == 0) 
    goto basic_dispatch23666;
  /* Here if argument TypeFixnum */
  CVTTQ(3, f3, f31, 3, f3);
  CVTQL(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;

basic_dispatch23665:
  if (_trace) printf("basic_dispatch23665:\n");
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  goto cachevalid;   

basic_dispatch23666:
  if (_trace) printf("basic_dispatch23666:\n");
  t9 = (t8 == Type_SingleFloat) ? 1 : 0;   

force_alignment23671:
  if (_trace) printf("force_alignment23671:\n");
  if (t9 == 0) 
    goto basic_dispatch23667;
  /* Here if argument TypeSingleFloat */
  CVTTS(3, f3, f31, 3, f3);
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  STS( (u32 *)(iSP + 8), 3, f3 );   
  iSP = iSP + 8;
  goto basic_dispatch23665;   

basic_dispatch23667:
  if (_trace) printf("basic_dispatch23667:\n");
  t9 = (t8 == Type_DoubleFloat) ? 1 : 0;   

force_alignment23672:
  if (_trace) printf("force_alignment23672:\n");
  if (t9 == 0) 
    goto basic_dispatch23665;
  /* Here if argument TypeDoubleFloat */
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  STT( (u64 *)&processor->fp0, 3, f3 );   
  r0 = (u64)&&return0164;
  goto consdoublefloat;
return0164:
  t8 = Type_Fixnum;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 0, f0 );   
  t8 = Type_DoubleFloat;
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto basic_dispatch23665;   

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

force_alignment23696:
  if (_trace) printf("force_alignment23696:\n");
  if (t10 == 0) 
    goto basic_dispatch23680;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23685:
  if (_trace) printf("force_alignment23685:\n");
  if (t12 == 0) 
    goto basic_dispatch23682;
  /* Here if argument TypeFixnum */
  t5 = t2 - t4;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if ((s64)t5 > 0)   
    t4 = t2;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
		/* We know temp2 has CDRNext/TypeFixnum */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t9;
  goto cachevalid;   

basic_dispatch23682:
  if (_trace) printf("basic_dispatch23682:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23686:
  if (_trace) printf("force_alignment23686:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23677;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto simple_binary_minmax23674;   

basic_dispatch23681:
  if (_trace) printf("basic_dispatch23681:\n");

basic_dispatch23680:
  if (_trace) printf("basic_dispatch23680:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment23697:
  if (_trace) printf("force_alignment23697:\n");
  if (t10 == 0) 
    goto basic_dispatch23687;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23692:
  if (_trace) printf("force_alignment23692:\n");
  if (t12 == 0) 
    goto basic_dispatch23689;
  /* Here if argument TypeSingleFloat */

simple_binary_minmax23674:
  if (_trace) printf("simple_binary_minmax23674:\n");
  /* NIL */
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (FLTU64(0, f0) > 0.0)   
    f2 = f1;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 2, f2 );   
  goto cachevalid;   

basic_dispatch23689:
  if (_trace) printf("basic_dispatch23689:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23693:
  if (_trace) printf("force_alignment23693:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23677;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto simple_binary_minmax23674;   

basic_dispatch23688:
  if (_trace) printf("basic_dispatch23688:\n");

basic_dispatch23687:
  if (_trace) printf("basic_dispatch23687:\n");
  /* Here for all other cases */

binary_type_dispatch23676:
  if (_trace) printf("binary_type_dispatch23676:\n");

simple_binary_minmax23673:
  if (_trace) printf("simple_binary_minmax23673:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch23678;   

binary_type_dispatch23677:
  if (_trace) printf("binary_type_dispatch23677:\n");
  t1 = t3;
  goto simple_binary_minmax23673;   

binary_type_dispatch23678:
  if (_trace) printf("binary_type_dispatch23678:\n");

basic_dispatch23679:
  if (_trace) printf("basic_dispatch23679:\n");

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

force_alignment23721:
  if (_trace) printf("force_alignment23721:\n");
  if (t10 == 0) 
    goto basic_dispatch23705;
  /* Here if argument TypeFixnum */
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23710:
  if (_trace) printf("force_alignment23710:\n");
  if (t12 == 0) 
    goto basic_dispatch23707;
  /* Here if argument TypeFixnum */
  t5 = t2 - t4;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if ((s64)t5 < 0)   
    t4 = t2;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
		/* We know temp2 has CDRNext/TypeFixnum */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t9;
  goto cachevalid;   

basic_dispatch23707:
  if (_trace) printf("basic_dispatch23707:\n");
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23711:
  if (_trace) printf("force_alignment23711:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23702;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto simple_binary_minmax23699;   

basic_dispatch23706:
  if (_trace) printf("basic_dispatch23706:\n");

basic_dispatch23705:
  if (_trace) printf("basic_dispatch23705:\n");
  t10 = (t9 == Type_SingleFloat) ? 1 : 0;   

force_alignment23722:
  if (_trace) printf("force_alignment23722:\n");
  if (t10 == 0) 
    goto basic_dispatch23712;
  /* Here if argument TypeSingleFloat */
  t12 = (t11 == Type_SingleFloat) ? 1 : 0;   

force_alignment23717:
  if (_trace) printf("force_alignment23717:\n");
  if (t12 == 0) 
    goto basic_dispatch23714;
  /* Here if argument TypeSingleFloat */

simple_binary_minmax23699:
  if (_trace) printf("simple_binary_minmax23699:\n");
  /* NIL */
  SUBS(0, f0, 1, f1, 2, f2); /* subs */   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  if (FLTU64(0, f0) < 0.0)   
    f2 = f1;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t8 = Type_SingleFloat;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t8;
  STS( (u32 *)iSP, 2, f2 );   
  goto cachevalid;   

basic_dispatch23714:
  if (_trace) printf("basic_dispatch23714:\n");
  t12 = (t11 == Type_Fixnum) ? 1 : 0;   

force_alignment23718:
  if (_trace) printf("force_alignment23718:\n");
  if (t12 == 0) 
    goto binary_type_dispatch23702;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto simple_binary_minmax23699;   

basic_dispatch23713:
  if (_trace) printf("basic_dispatch23713:\n");

basic_dispatch23712:
  if (_trace) printf("basic_dispatch23712:\n");
  /* Here for all other cases */

binary_type_dispatch23701:
  if (_trace) printf("binary_type_dispatch23701:\n");

simple_binary_minmax23698:
  if (_trace) printf("simple_binary_minmax23698:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch23703;   

binary_type_dispatch23702:
  if (_trace) printf("binary_type_dispatch23702:\n");
  t1 = t3;
  goto simple_binary_minmax23698;   

binary_type_dispatch23703:
  if (_trace) printf("binary_type_dispatch23703:\n");

basic_dispatch23704:
  if (_trace) printf("basic_dispatch23704:\n");

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

force_alignment23723:
  if (_trace) printf("force_alignment23723:\n");
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
		/* Put the result back on the stack */
  *(u32 *)iSP = t6;
  t1 = Type_Fixnum;
		/* Push high order half */
  *(u32 *)(iSP + 8) = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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

vma_memory_read23724:
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
    goto vma_memory_read23726;

vma_memory_read23725:
  t3 = zero + 64;   
  t4 = t4 >> (arg5 & 63);   
  t3 = t3 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t4 & 1)   
    goto vma_memory_read23728;

vma_memory_read23733:
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6 >> (Array_LongPrefixBitPos & 63);   
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto aref1illegal;
  if (t2 & 1)   
    goto aref1exception;
		/* store the array */
  *(u32 *)&((ARRAYCACHEP)t7)->array = t12;
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
		/* store the array register word [implicit fixnum] */
  *(u32 *)&((ARRAYCACHEP)t7)->arword = t10;
  *(u64 *)&((ARRAYCACHEP)t7)->locat = t9;   		// store the storage [implicit locative] 
  arg5 = arg6 >> (Array_BytePackingPos & 63);   		// get BP into arg5 
  arg6 = arg6 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg5 = arg5 & Array_BytePackingMask;
  arg4 = zero;
  arg6 = arg6 & Array_ElementTypeMask;
  goto aref1restart;   

vma_memory_read23726:
  if (_trace) printf("vma_memory_read23726:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg6 = *(s32 *)t1;   
  arg5 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read23725;   

vma_memory_read23728:
  if (_trace) printf("vma_memory_read23728:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23727;
  arg4 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23724;   

vma_memory_read23727:
  if (_trace) printf("vma_memory_read23727:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23730:
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
    goto recompute_array_register23735;
  /* Memory Read Internal */

vma_memory_read23737:
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
    goto vma_memory_read23739;

vma_memory_read23738:
  t3 = zero + 64;   
  t1 = t1 >> (t7 & 63);   
  t3 = t3 >> (t7 & 63);   
  t6 = (u32)t6;   
  if (t1 & 1)   
    goto vma_memory_read23741;

vma_memory_read23746:
  /* TagType. */
  t8 = t7 & 63;
  t2 = t6 >> (Array_LongPrefixBitPos & 63);   
  t8 = t8 - Type_HeaderI;   
  if (t8 != 0)   
    goto recompute_array_register23734;
  if (t2 & 1)   
    goto recompute_array_register23736;
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

recompute_array_register23736:
  if (_trace) printf("recompute_array_register23736:\n");
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
  r0 = (u64)&&return0165;
  goto setup1dlongarray;
return0165:
  t4 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t4 != 0)   
    goto recompute_array_register23735;
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

recompute_array_register23735:
  if (_trace) printf("recompute_array_register23735:\n");
  arg6 = t4;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 12;
  goto arrayexception;

recompute_array_register23734:
  if (_trace) printf("recompute_array_register23734:\n");
  arg5 = 0;
  arg2 = 12;
  goto illegaloperand;

vma_memory_read23739:
  if (_trace) printf("vma_memory_read23739:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t2;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read23738;   

vma_memory_read23741:
  if (_trace) printf("vma_memory_read23741:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23740;
  t5 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read23737;   

vma_memory_read23740:
  if (_trace) printf("vma_memory_read23740:\n");
  t1 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t1;   		// Adjust for a longword load 
  t1 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23743:
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
  r0 = (u64)&&return0166;
  goto setup1dlongarray;
return0166:
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
		/* store the array */
  *(u32 *)&((ARRAYCACHEP)t7)->array = t9;
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

vma_memory_read23747:
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
    goto vma_memory_read23749;

vma_memory_read23748:
  t3 = zero + 64;   
  t4 = t4 >> (arg5 & 63);   
  t3 = t3 >> (arg5 & 63);   
  arg6 = (u32)arg6;   
  if (t4 & 1)   
    goto vma_memory_read23751;

vma_memory_read23756:
  /* TagType. */
  t1 = arg5 & 63;
  t2 = arg6 >> (Array_LongPrefixBitPos & 63);   
  t1 = t1 - Type_HeaderI;   
  if (t1 != 0)   
    goto aset1illegal;
  if (t2 & 1)   
    goto aset1exception;
		/* store the array */
  *(u32 *)&((ARRAYCACHEP)t7)->array = t12;
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
		/* store the array register word [implicit fixnum] */
  *(u32 *)&((ARRAYCACHEP)t7)->arword = t10;
  *(u64 *)&((ARRAYCACHEP)t7)->locat = t9;   		// store the storage [implicit locative] 
  arg5 = arg6 >> (Array_BytePackingPos & 63);   		// get BP into arg5 
  arg6 = arg6 >> (Array_ElementTypePos & 63);   		// get element type into arg6 
  arg5 = arg5 & Array_BytePackingMask;
  arg4 = zero;
  arg6 = arg6 & Array_ElementTypeMask;
  goto aset1restart;   

vma_memory_read23749:
  if (_trace) printf("vma_memory_read23749:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg6 = *(s32 *)t1;   
  arg5 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read23748;   

vma_memory_read23751:
  if (_trace) printf("vma_memory_read23751:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23750;
  arg4 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23747;   

vma_memory_read23750:
  if (_trace) printf("vma_memory_read23750:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23753:
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
    goto recompute_array_register23758;
  /* Memory Read Internal */

vma_memory_read23760:
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
    goto vma_memory_read23762;

vma_memory_read23761:
  t3 = zero + 64;   
  t1 = t1 >> (t7 & 63);   
  t3 = t3 >> (t7 & 63);   
  t6 = (u32)t6;   
  if (t1 & 1)   
    goto vma_memory_read23764;

vma_memory_read23769:
  /* TagType. */
  t8 = t7 & 63;
  t2 = t6 >> (Array_LongPrefixBitPos & 63);   
  t8 = t8 - Type_HeaderI;   
  if (t8 != 0)   
    goto recompute_array_register23757;
  if (t2 & 1)   
    goto recompute_array_register23759;
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

recompute_array_register23759:
  if (_trace) printf("recompute_array_register23759:\n");
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
  r0 = (u64)&&return0167;
  goto setup1dlongarray;
return0167:
  t4 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t4 != 0)   
    goto recompute_array_register23758;
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

recompute_array_register23758:
  if (_trace) printf("recompute_array_register23758:\n");
  arg6 = t4;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 3;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 12;
  goto arrayexception;

recompute_array_register23757:
  if (_trace) printf("recompute_array_register23757:\n");
  arg5 = 0;
  arg2 = 12;
  goto illegaloperand;

vma_memory_read23762:
  if (_trace) printf("vma_memory_read23762:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t2;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read23761;   

vma_memory_read23764:
  if (_trace) printf("vma_memory_read23764:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23763;
  t5 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read23760;   

vma_memory_read23763:
  if (_trace) printf("vma_memory_read23763:\n");
  t1 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t1;   		// Adjust for a longword load 
  t1 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23766:
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
  r0 = (u64)&&return0168;
  goto setup1dlongarray;
return0168:
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
		/* store the array */
  *(u32 *)&((ARRAYCACHEP)t7)->array = t9;
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

vma_memory_read23770:
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
    goto vma_memory_read23772;

vma_memory_read23771:
  t3 = zero + 64;   
  t4 = t4 >> (arg5 & 63);   
  t3 = t3 >> (arg5 & 63);   
  if (t4 & 1)   
    goto vma_memory_read23774;

vma_memory_read23779:
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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

vma_memory_read23772:
  if (_trace) printf("vma_memory_read23772:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg6 = *(s32 *)t1;   
  arg5 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read23771;   

vma_memory_read23774:
  if (_trace) printf("vma_memory_read23774:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23773;
  arg4 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23770;   

vma_memory_read23773:
  if (_trace) printf("vma_memory_read23773:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23776:
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

force_alignment23793:
  if (_trace) printf("force_alignment23793:\n");
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
    goto setup_array_register23781;
  /* Memory Read Internal */

vma_memory_read23783:
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
    goto vma_memory_read23785;

vma_memory_read23784:
  t7 = zero + 64;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  t3 = (u32)t3;   
  if (t8 & 1)   
    goto vma_memory_read23787;

vma_memory_read23792:
  /* TagType. */
  t5 = t4 & 63;
  t6 = t3 >> (Array_LongPrefixBitPos & 63);   
  t5 = t5 - Type_HeaderI;   
  if (t5 != 0)   
    goto setup_array_register23780;
  if (t6 & 1)   
    goto setup_array_register23782;
  t5 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  t8 = t3 >> (Array_RegisterBytePackingPos & 63);   
  t7 = Type_Fixnum;
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t5 = arg1 + 1;
  t8 = t8 + t1;		// Construct the array register word 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t6;
  iSP = iSP + 8;
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  t6 = zero + Array_LengthMask;   
  t6 = t3 & t6;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

setup_array_register23781:
  if (_trace) printf("setup_array_register23781:\n");
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

setup_array_register23780:
  if (_trace) printf("setup_array_register23780:\n");
  arg5 = 0;
  arg2 = 71;
  goto illegaloperand;

setup_array_register23782:
  if (_trace) printf("setup_array_register23782:\n");
  r0 = (u64)&&return0169;
  goto setup1dlongarray;
return0169:
  t1 = (t2 == ReturnValue_Normal) ? 1 : 0;   
  if (t1 != 0)   
    goto NEXTINSTRUCTION;
  t1 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register23781;
  t1 = (t2 == ReturnValue_IllegalOperand) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register23780;
  goto NEXTINSTRUCTION;   

vma_memory_read23785:
  if (_trace) printf("vma_memory_read23785:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t3 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23784;   

vma_memory_read23787:
  if (_trace) printf("vma_memory_read23787:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23786;
  arg1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read23783;   

vma_memory_read23786:
  if (_trace) printf("vma_memory_read23786:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23789:
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

force_alignment23807:
  if (_trace) printf("force_alignment23807:\n");
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
    goto setup_array_register23795;
  /* Memory Read Internal */

vma_memory_read23797:
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
    goto vma_memory_read23799;

vma_memory_read23798:
  t7 = zero + 64;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  t3 = (u32)t3;   
  if (t8 & 1)   
    goto vma_memory_read23801;

vma_memory_read23806:
  /* TagType. */
  t5 = t4 & 63;
  t6 = t3 >> (Array_LongPrefixBitPos & 63);   
  t5 = t5 - Type_HeaderI;   
  if (t5 != 0)   
    goto setup_array_register23794;
  if (t6 & 1)   
    goto setup_array_register23796;
  t5 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  t8 = t3 >> (Array_RegisterBytePackingPos & 63);   
  t7 = Type_Fixnum;
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t5 = arg1 + 1;
  t8 = t8 + t1;		// Construct the array register word 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t6;
  iSP = iSP + 8;
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  t6 = zero + Array_LengthMask;   
  t6 = t3 & t6;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

setup_array_register23795:
  if (_trace) printf("setup_array_register23795:\n");
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

setup_array_register23794:
  if (_trace) printf("setup_array_register23794:\n");
  arg5 = 0;
  arg2 = 71;
  goto illegaloperand;

setup_array_register23796:
  if (_trace) printf("setup_array_register23796:\n");
  r0 = (u64)&&return0170;
  goto setup1dlongarray;
return0170:
  t1 = (t2 == ReturnValue_Normal) ? 1 : 0;   
  if (t1 != 0)   
    goto NEXTINSTRUCTION;
  t1 = (t2 == ReturnValue_Exception) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register23795;
  t1 = (t2 == ReturnValue_IllegalOperand) ? 1 : 0;   
  if (t1 != 0)   
    goto setup_array_register23794;
  goto NEXTINSTRUCTION;   

vma_memory_read23799:
  if (_trace) printf("vma_memory_read23799:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t3 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read23798;   

vma_memory_read23801:
  if (_trace) printf("vma_memory_read23801:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read23800;
  arg1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read23797;   

vma_memory_read23800:
  if (_trace) printf("vma_memory_read23800:\n");
  t8 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read23803:
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

vma_memory_read23817:
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
    goto vma_memory_read23819;

vma_memory_read23818:
  t10 = zero + 240;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  arg4 = (u32)arg4;   
  if (t11 & 1)   
    goto vma_memory_read23821;

vma_memory_read23828:
  t8 = t6 - Type_Fixnum;   
  t8 = t8 & 63;		// Strip CDR code 
  if (t8 != 0)   
    goto setup_long_array_register23808;
  t1 = t1 + 1;   		// Offset is adata+2 
  /* Memory Read Internal */

vma_memory_read23829:
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
    goto vma_memory_read23831;

vma_memory_read23830:
  t10 = zero + 240;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  arg3 = (u32)arg3;   
  if (t11 & 1)   
    goto vma_memory_read23833;

vma_memory_read23840:
  t8 = t6 - Type_Fixnum;   
  t8 = t8 & 63;		// Strip CDR code 
  if (t8 != 0)   
    goto setup_long_array_register23808;
  t1 = t1 + 1;   		// Indirect is adata+3 
  /* Memory Read Internal */

vma_memory_read23841:
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
    goto vma_memory_read23843;

vma_memory_read23842:
  t10 = zero + 240;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  t5 = (u32)t5;   
  if (t11 & 1)   
    goto vma_memory_read23845;

vma_memory_read23852:
  t10 = t6 & 63;		// Strip off any CDR code bits. 
  t11 = (t10 == Type_Locative) ? 1 : 0;   

force_alignment23917:
  if (_trace) printf("force_alignment23917:\n");
  if (t11 == 0) 
    goto basic_dispatch23854;
  /* Here if argument TypeLocative */

setup_long_array_register23811:
  if (_trace) printf("setup_long_array_register23811:\n");
  t10 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t10;
  iSP = iSP + 8;
  t8 = t3 >> (Array_BytePackingPos & 63);   
  t7 = Type_Fixnum;
  t1 = *(u64 *)&(processor->areventcount);   
  t8 = t8 << (Array_RegisterBytePackingPos & 63);   
  t8 = t8 + t1;		// Construct the array register word 
  t6 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t8;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t6;
  iSP = iSP + 8;
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg4;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto setup_long_array_register23816;   

basic_dispatch23854:
  if (_trace) printf("basic_dispatch23854:\n");
  t11 = (t10 == Type_Fixnum) ? 1 : 0;   

force_alignment23918:
  if (_trace) printf("force_alignment23918:\n");
  if (t11 == 0) 
    goto basic_dispatch23855;
  /* Here if argument TypeFixnum */
  goto setup_long_array_register23811;   

basic_dispatch23855:
  if (_trace) printf("basic_dispatch23855:\n");
  t11 = (t10 == Type_Array) ? 1 : 0;   

force_alignment23919:
  if (_trace) printf("force_alignment23919:\n");
  if (t11 == 0) 
    goto basic_dispatch23856;
  /* Here if argument TypeArray */

setup_long_array_register23815:
  if (_trace) printf("setup_long_array_register23815:\n");
  t1 = t3 & 7;
  t1 = (t1 == 1) ? 1 : 0;   
  t1 = t1 | t2;		// Force true if FORCE 
  if (t1 == 0) 
    goto setup_long_array_register23808;
  t12 = t3 >> (Array_BytePackingPos & 63);   
  t12 = t12 & Array_BytePackingMask;
  t2 = arg3;

setup_long_array_register23810:
  if (_trace) printf("setup_long_array_register23810:\n");
  /* Memory Read Internal */

vma_memory_read23857:
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
    goto vma_memory_read23859;

vma_memory_read23858:
  t10 = zero + 64;   
  t11 = t11 >> (t6 & 63);   
  t10 = t10 >> (t6 & 63);   
  t4 = (u32)t4;   
  if (t11 & 1)   
    goto vma_memory_read23861;

vma_memory_read23866:
  t10 = t4 >> (Array_BytePackingPos & 63);   
  t10 = t10 & Array_BytePackingMask;
  arg1 = t12 - t10;   
  t7 = t4 >> (Array_LongPrefixBitPos & 63);   
  if (t7 & 1)   
    goto setup_long_array_register23812;
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

setup_long_array_register23809:
  if (_trace) printf("setup_long_array_register23809:\n");
  arg4 = arg4 - t2;   
  t10 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t9;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t10;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t6;
  iSP = iSP + 8;
  if ((s64)arg4 <= 0)   
    arg4 = zero;
  if (arg4 == 0) 
    goto setup_long_array_register23813;
  t1 = zero - t12;   
  t1 = t2 << (t1 & 63);   
  t2 = t2 >> (t12 & 63);   
  if ((s64)t12 <= 0)   
    t2 = t1;
  t5 = t2 + t5;

setup_long_array_register23813:
  if (_trace) printf("setup_long_array_register23813:\n");
  t8 = Type_Locative;
  *(u32 *)(iSP + 8) = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  t8 = t7 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg4;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto setup_long_array_register23816;   

setup_long_array_register23812:
  if (_trace) printf("setup_long_array_register23812:\n");
  t1 = t5 + 1;		// length=array+1 
  /* Memory Read Internal */

vma_memory_read23867:
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
    goto vma_memory_read23869;

vma_memory_read23868:
  t10 = zero + 240;   
  t11 = t11 >> (t4 & 63);   
  t10 = t10 >> (t4 & 63);   
  arg6 = (u32)arg6;   
  if (t11 & 1)   
    goto vma_memory_read23871;

vma_memory_read23878:
  t1 = t4 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto setup_long_array_register23808;
  t1 = t5 + 2;		// offset=array+2 
  /* Memory Read Internal */

vma_memory_read23879:
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
    goto vma_memory_read23881;

vma_memory_read23880:
  t10 = zero + 240;   
  t11 = t11 >> (t4 & 63);   
  t10 = t10 >> (t4 & 63);   
  arg5 = (u32)arg5;   
  if (t11 & 1)   
    goto vma_memory_read23883;

vma_memory_read23890:
  t1 = t4 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto setup_long_array_register23808;
  t1 = t5 + 3;		// next=array+3 
  /* Memory Read Internal */

vma_memory_read23891:
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
    goto vma_memory_read23893;

vma_memory_read23892:
  t10 = zero + 240;   
  t11 = t11 >> (t4 & 63);   
  t10 = t10 >> (t4 & 63);   
  t5 = (u32)t5;   
  if (t11 & 1)   
    goto vma_memory_read23895;

vma_memory_read23902:
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

force_alignment23910:
  if (_trace) printf("force_alignment23910:\n");
  if (t10 == 0) 
    goto basic_dispatch23904;
  /* Here if argument TypeLocative */
  goto setup_long_array_register23809;   

basic_dispatch23904:
  if (_trace) printf("basic_dispatch23904:\n");
  t10 = (t8 == Type_Fixnum) ? 1 : 0;   

force_alignment23911:
  if (_trace) printf("force_alignment23911:\n");
  if (t10 == 0) 
    goto basic_dispatch23905;
  /* Here if argument TypeFixnum */
  goto setup_long_array_register23809;   

basic_dispatch23905:
  if (_trace) printf("basic_dispatch23905:\n");
  t10 = (t8 == Type_Array) ? 1 : 0;   

force_alignment23912:
  if (_trace) printf("force_alignment23912:\n");
  if (t10 == 0) 
    goto basic_dispatch23906;
  /* Here if argument TypeArray */

setup_long_array_register23814:
  if (_trace) printf("setup_long_array_register23814:\n");
  t7 = zero - arg1;   
  t7 = arg5 >> (t7 & 63);   
  arg3 = arg5 << (arg1 & 63);   
  if ((s64)arg1 <= 0)   
    arg3 = t7;
  t2 = t2 + arg3;
  goto setup_long_array_register23810;   

basic_dispatch23906:
  if (_trace) printf("basic_dispatch23906:\n");
  t10 = (t8 == Type_String) ? 1 : 0;   

force_alignment23913:
  if (_trace) printf("force_alignment23913:\n");
  if (t10 == 0) 
    goto basic_dispatch23907;
  /* Here if argument TypeString */
  goto setup_long_array_register23814;   

basic_dispatch23907:
  if (_trace) printf("basic_dispatch23907:\n");
  /* Here for all other cases */
  goto setup_long_array_register23808;   

basic_dispatch23903:
  if (_trace) printf("basic_dispatch23903:\n");

basic_dispatch23856:
  if (_trace) printf("basic_dispatch23856:\n");
  t11 = (t10 == Type_String) ? 1 : 0;   

force_alignment23920:
  if (_trace) printf("force_alignment23920:\n");
  if (t11 == 0) 
    goto basic_dispatch23914;
  /* Here if argument TypeString */
  goto setup_long_array_register23815;   

basic_dispatch23914:
  if (_trace) printf("basic_dispatch23914:\n");
  /* Here for all other cases */
  goto setup_long_array_register23808;   

basic_dispatch23853:
  if (_trace) printf("basic_dispatch23853:\n");

setup_long_array_register23808:
  if (_trace) printf("setup_long_array_register23808:\n");
  t2 = ReturnValue_Exception;
  goto *r0; /* ret */

setup_long_array_register23816:
  if (_trace) printf("setup_long_array_register23816:\n");
  t2 = ReturnValue_Normal;
  goto *r0; /* ret */

vma_memory_read23893:
  if (_trace) printf("vma_memory_read23893:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t5 = *(s32 *)t7;   
  t4 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23892;   

vma_memory_read23895:
  if (_trace) printf("vma_memory_read23895:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23894;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read23891;   

vma_memory_read23894:
  if (_trace) printf("vma_memory_read23894:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23899:
  if (_trace) printf("vma_memory_read23899:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read23898;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read23902;   

vma_memory_read23898:

vma_memory_read23897:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23881:
  if (_trace) printf("vma_memory_read23881:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg5 = *(s32 *)t7;   
  t4 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23880;   

vma_memory_read23883:
  if (_trace) printf("vma_memory_read23883:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23882;
  t1 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read23879;   

vma_memory_read23882:
  if (_trace) printf("vma_memory_read23882:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23887:
  if (_trace) printf("vma_memory_read23887:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read23886;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read23890;   

vma_memory_read23886:

vma_memory_read23885:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23869:
  if (_trace) printf("vma_memory_read23869:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg6 = *(s32 *)t7;   
  t4 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23868;   

vma_memory_read23871:
  if (_trace) printf("vma_memory_read23871:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23870;
  t1 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read23867;   

vma_memory_read23870:
  if (_trace) printf("vma_memory_read23870:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23875:
  if (_trace) printf("vma_memory_read23875:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read23874;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read23878;   

vma_memory_read23874:

vma_memory_read23873:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23859:
  if (_trace) printf("vma_memory_read23859:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t4 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23858;   

vma_memory_read23861:
  if (_trace) printf("vma_memory_read23861:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23860;
  t5 = (u32)t4;   		// Do the indirect thing 
  goto vma_memory_read23857;   

vma_memory_read23860:
  if (_trace) printf("vma_memory_read23860:\n");
  t11 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23863:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 6;
  goto performmemoryaction;

vma_memory_read23843:
  if (_trace) printf("vma_memory_read23843:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  t5 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23842;   

vma_memory_read23845:
  if (_trace) printf("vma_memory_read23845:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23844;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read23841;   

vma_memory_read23844:
  if (_trace) printf("vma_memory_read23844:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23849:
  if (_trace) printf("vma_memory_read23849:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read23848;
  t6 = t6 & ~63L;
  t6 = t6 | Type_ExternalValueCellPointer;
  goto vma_memory_read23852;   

vma_memory_read23848:

vma_memory_read23847:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23831:
  if (_trace) printf("vma_memory_read23831:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg3 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23830;   

vma_memory_read23833:
  if (_trace) printf("vma_memory_read23833:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23832;
  t1 = (u32)arg3;   		// Do the indirect thing 
  goto vma_memory_read23829;   

vma_memory_read23832:
  if (_trace) printf("vma_memory_read23832:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23837:
  if (_trace) printf("vma_memory_read23837:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read23836;
  t6 = t6 & ~63L;
  t6 = t6 | Type_ExternalValueCellPointer;
  goto vma_memory_read23840;   

vma_memory_read23836:

vma_memory_read23835:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23819:
  if (_trace) printf("vma_memory_read23819:\n");
  t8 = *(u64 *)&(processor->stackcachedata);   
  t7 = (t7 * 8) + t8;  		// reconstruct SCA 
  arg4 = *(s32 *)t7;   
  t6 = *(s32 *)(t7 + 4);   		// Read from stack cache 
  goto vma_memory_read23818;   

vma_memory_read23821:
  if (_trace) printf("vma_memory_read23821:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read23820;
  t1 = (u32)arg4;   		// Do the indirect thing 
  goto vma_memory_read23817;   

vma_memory_read23820:
  if (_trace) printf("vma_memory_read23820:\n");
  t11 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read23825:
  if (_trace) printf("vma_memory_read23825:\n");
  t10 = t11 & MemoryActionTransform;
  if (t10 == 0) 
    goto vma_memory_read23824;
  t6 = t6 & ~63L;
  t6 = t6 | Type_ExternalValueCellPointer;
  goto vma_memory_read23828;   

vma_memory_read23824:

vma_memory_read23823:
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

force_alignment23931:
  if (_trace) printf("force_alignment23931:\n");
  if (t12 == 0) 
    goto basic_dispatch23927;
  /* Here if argument ArrayElementTypeCharacter */
  t2 = t1 - Type_Character;   
  if (t2 == 0) 
    goto aset_1_internal23922;
  arg5 = 0;
  arg2 = 29;
  goto illegaloperand;

aset_1_internal23922:
  if (_trace) printf("aset_1_internal23922:\n");
  if (t6 == 0) 		// Certainly will fit if not packed! 
    goto aset_1_internal23921;
  t2 = 32;
  t2 = t2 >> (t6 & 63);   		// Compute size of byte 
  t1 = ~zero;   
  t1 = t1 << (t2 & 63);   
  t1 = ~t1;   		// Compute mask for byte 
  t1 = t11 & t1;
  t1 = t11 - t1;   
  if (t1 == 0) 		// J. if character fits. 
    goto aset_1_internal23921;
  arg5 = 0;
  arg2 = 62;
  goto illegaloperand;

basic_dispatch23927:
  if (_trace) printf("basic_dispatch23927:\n");
  t12 = (t8 == Array_ElementTypeFixnum) ? 1 : 0;   

force_alignment23932:
  if (_trace) printf("force_alignment23932:\n");
  if (t12 == 0) 
    goto basic_dispatch23928;
  /* Here if argument ArrayElementTypeFixnum */
  t2 = t1 - Type_Fixnum;   
  if (t2 == 0) 
    goto aset_1_internal23921;
  arg5 = 0;
  arg2 = 33;
  goto illegaloperand;

basic_dispatch23928:
  if (_trace) printf("basic_dispatch23928:\n");
  t12 = (t8 == Array_ElementTypeBoolean) ? 1 : 0;   

force_alignment23933:
  if (_trace) printf("force_alignment23933:\n");
  if (t12 == 0) 
    goto basic_dispatch23926;
  /* Here if argument ArrayElementTypeBoolean */
  t11 = 1;
  t1 = t1 - Type_NIL;   
  if (t1 != 0)   		// J. if True 
    goto aset_1_internal23921;
  t11 = zero;
  goto aset_1_internal23921;   		// J. if False 

basic_dispatch23926:
  if (_trace) printf("basic_dispatch23926:\n");
  /* Shove it in. */

aset_1_internal23921:
  if (_trace) printf("aset_1_internal23921:\n");
  if (t6 != 0)   		// J. if packed 
    goto aset_1_internal23923;
  t1 = t8 - Array_ElementTypeObject;   
  if (t1 != 0)   
    goto aset_1_internal23923;
  /* Here for the simple non packed case */
  t1 = t9 + arg4;
  /* Memory Read Internal */

vma_memory_read23934:
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
    goto vma_memory_read23936;

vma_memory_read23935:
  t12 = zero + 240;   
  arg3 = arg3 >> (t2 & 63);   
  t12 = t12 >> (t2 & 63);   
  if (arg3 & 1)   
    goto vma_memory_read23938;

vma_memory_read23944:
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

force_alignment23946:
  if (_trace) printf("force_alignment23946:\n");
  t12 = t12 | t5;
  STQ_U(t4, t12);   
  *(u32 *)t3 = t11;
  if (arg3 != 0)   		// J. if in cache 
    goto vma_memory_write23945;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   
  /* Here for the slow packed version */

aset_1_internal23923:
  if (_trace) printf("aset_1_internal23923:\n");
  arg4 = t7 + arg4;
  t1 = arg4 >> (t6 & 63);   		// Convert byte index to word index 
  t1 = t1 + t9;		// Address of word containing byte 
  /* Memory Read Internal */

vma_memory_read23947:
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
    goto vma_memory_read23949;

vma_memory_read23948:
  t4 = zero + 240;   
  t5 = t5 >> (arg5 & 63);   
  t4 = t4 >> (arg5 & 63);   
  t9 = (u32)t9;   
  if (t5 & 1)   
    goto vma_memory_read23951;

vma_memory_read23958:
  /* Check fixnum element type */
  /* TagType. */
  t2 = arg5 & 63;
  t2 = t2 - Type_Fixnum;   
  if (t2 != 0)   		// J. if element type not fixnum. 
    goto aset_1_internal23924;
  if (t6 == 0) 		// J. if unpacked fixnum element type. 
    goto aset_1_internal23925;
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
    goto array_element_dpb23959;
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
  goto array_element_dpb23960;   

array_element_dpb23959:
  if (_trace) printf("array_element_dpb23959:\n");
  /* Inserting the byte into the low byte */
  t9 = t9 & t3;		// Remove the old low byte 
  t12 = t11 & t4;		// Remove unwanted bits from the new byte 
  t9 = t9 | t12;		// Insert the new byte in place of the old byte 

array_element_dpb23960:
  if (_trace) printf("array_element_dpb23960:\n");
  t11 = t9;

aset_1_internal23925:
  if (_trace) printf("aset_1_internal23925:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t2 = t1 + ivory;
  t12 = *(s32 *)&processor->scovlimit;   
  t5 = (t2 * 4);   
  t4 = LDQ_U(t2);   
  t3 = t1 - t3;   		// Stack cache offset 
  t12 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  t3 = (arg5 & 0xff) << ((t2&7)*8);   
  t4 = t4 & ~(0xffL << (t2&7)*8);   

force_alignment23962:
  if (_trace) printf("force_alignment23962:\n");
  t4 = t4 | t3;
  STQ_U(t2, t4);   
  *(u32 *)t5 = t11;
  if (t12 != 0)   		// J. if in cache 
    goto vma_memory_write23961;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

aset_1_internal23924:
  if (_trace) printf("aset_1_internal23924:\n");
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

vma_memory_write23961:
  if (_trace) printf("vma_memory_write23961:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment23963:
  if (_trace) printf("force_alignment23963:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t3 = t1 - t3;   		// Stack cache offset 
  t2 = (t3 * 8) + t2;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t2 = t11;
		/* write the stack cache */
  *(u32 *)(t2 + 4) = arg5;
  goto NEXTINSTRUCTION;   

vma_memory_read23949:
  if (_trace) printf("vma_memory_read23949:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = (t2 * 8) + t3;  		// reconstruct SCA 
  t9 = *(s32 *)t2;   
  arg5 = *(s32 *)(t2 + 4);   		// Read from stack cache 
  goto vma_memory_read23948;   

vma_memory_read23951:
  if (_trace) printf("vma_memory_read23951:\n");
  if ((t4 & 1) == 0)   
    goto vma_memory_read23950;
  t1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read23947;   

vma_memory_read23950:
  if (_trace) printf("vma_memory_read23950:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t4 = arg5 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t4 = (t4 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t4;   		// Get the memory action 

vma_memory_read23955:
  if (_trace) printf("vma_memory_read23955:\n");
  t4 = t5 & MemoryActionTransform;
  if (t4 == 0) 
    goto vma_memory_read23954;
  arg5 = arg5 & ~63L;
  arg5 = arg5 | Type_ExternalValueCellPointer;
  goto vma_memory_read23958;   

vma_memory_read23954:

vma_memory_read23953:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_write23945:
  if (_trace) printf("vma_memory_write23945:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment23964:
  if (_trace) printf("force_alignment23964:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t1 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t4 = t11;
		/* write the stack cache */
  *(u32 *)(t4 + 4) = t2;
  goto NEXTINSTRUCTION;   

vma_memory_read23936:
  if (_trace) printf("vma_memory_read23936:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t3 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read23935;   

vma_memory_read23938:
  if (_trace) printf("vma_memory_read23938:\n");
  if ((t12 & 1) == 0)   
    goto vma_memory_read23937;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read23934;   

vma_memory_read23937:
  if (_trace) printf("vma_memory_read23937:\n");
  arg3 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t12 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t12 = (t12 * 4) + arg3;   		// Adjust for a longword load 
  arg3 = *(s32 *)t12;   		// Get the memory action 

vma_memory_read23941:

vma_memory_read23940:
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

vma_memory_read23965:
  t3 = arg4 + ivory;
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg4 - t11;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t12) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read23967;

vma_memory_read23966:
  t3 = zero + 64;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read23969;

vma_memory_read23974:
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

vma_memory_read23975:
  t3 = arg2 + ivory;
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg2 - t11;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->dataread_mask);   
  t2 = ((u64)t1 < (u64)t12) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read23977;

vma_memory_read23976:
  t3 = zero + 240;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read23979;

vma_memory_read23986:
  t1 = arg6 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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

vma_memory_read23977:
  if (_trace) printf("vma_memory_read23977:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read23976;   

vma_memory_read23979:
  if (_trace) printf("vma_memory_read23979:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23978;
  arg2 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read23975;   

vma_memory_read23978:
  if (_trace) printf("vma_memory_read23978:\n");
  t4 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23983:
  if (_trace) printf("vma_memory_read23983:\n");
  t3 = t4 & MemoryActionTransform;
  if (t3 == 0) 
    goto vma_memory_read23982;
  arg6 = arg6 & ~63L;
  arg6 = arg6 | Type_ExternalValueCellPointer;
  goto vma_memory_read23986;   

vma_memory_read23982:

vma_memory_read23981:
  /* Perform memory action */
  arg1 = t4;
  arg2 = 0;
  goto performmemoryaction;

vma_memory_read23967:
  if (_trace) printf("vma_memory_read23967:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read23966;   

vma_memory_read23969:
  if (_trace) printf("vma_memory_read23969:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23968;
  arg4 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read23965;   

vma_memory_read23968:
  if (_trace) printf("vma_memory_read23968:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23971:
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

vma_memory_read23987:
  t3 = arg4 + ivory;
  arg5 = (t3 * 4);   
  arg6 = LDQ_U(t3);   
  t1 = arg4 - t11;   		// Stack cache offset 
  t4 = *(u64 *)&(processor->header_mask);   
  t2 = ((u64)t1 < (u64)t12) ? 1 : 0;   		// In range? 
  arg5 = *(s32 *)arg5;   
  arg6 = (u8)(arg6 >> ((t3&7)*8));   
  if (t2 != 0)   
    goto vma_memory_read23989;

vma_memory_read23988:
  t3 = zero + 64;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read23991;

vma_memory_read23996:
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

vma_memory_read23997:
  t5 = arg2 + ivory;
  t2 = (t5 * 4);   
  t1 = LDQ_U(t5);   
  t3 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->datawrite_mask);   
  t4 = ((u64)t3 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t1 = (u8)(t1 >> ((t5&7)*8));   
  if (t4 != 0)   
    goto vma_memory_read23999;

vma_memory_read23998:
  t5 = zero + 240;   
  t8 = t8 >> (t1 & 63);   
  t5 = t5 >> (t1 & 63);   
  if (t8 & 1)   
    goto vma_memory_read24001;

vma_memory_read24007:
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

force_alignment24009:
  if (_trace) printf("force_alignment24009:\n");
  t5 = t5 | t4;
  STQ_U(t3, t5);   
  *(u32 *)t2 = t7;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write24008;
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

vma_memory_write24008:
  if (_trace) printf("vma_memory_write24008:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = arg2 - t11;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t3 = t7;
		/* write the stack cache */
  *(u32 *)(t3 + 4) = t1;
  goto NEXTINSTRUCTION;   

vma_memory_read23999:
  if (_trace) printf("vma_memory_read23999:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t2 = *(s32 *)t3;   
  t1 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read23998;   

vma_memory_read24001:
  if (_trace) printf("vma_memory_read24001:\n");
  if ((t5 & 1) == 0)   
    goto vma_memory_read24000;
  arg2 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read23997;   

vma_memory_read24000:
  if (_trace) printf("vma_memory_read24000:\n");
  t8 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t5 = t1 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg2;   		// stash the VMA for the (likely) trap 
  t5 = (t5 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t5;   		// Get the memory action 

vma_memory_read24004:

vma_memory_read24003:
  /* Perform memory action */
  arg1 = t8;
  arg2 = 1;
  goto performmemoryaction;

vma_memory_read23989:
  if (_trace) printf("vma_memory_read23989:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read23988;   

vma_memory_read23991:
  if (_trace) printf("vma_memory_read23991:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read23990;
  arg4 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read23987;   

vma_memory_read23990:
  if (_trace) printf("vma_memory_read23990:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read23993:
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

vma_memory_read24010:
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
    goto vma_memory_read24012;

vma_memory_read24011:
  t3 = zero + 64;   
  t4 = t4 >> (arg6 & 63);   
  t3 = t3 >> (arg6 & 63);   
  if (t4 & 1)   
    goto vma_memory_read24014;

vma_memory_read24019:
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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

vma_memory_read24012:
  if (_trace) printf("vma_memory_read24012:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  arg5 = *(s32 *)t1;   
  arg6 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read24011;   

vma_memory_read24014:
  if (_trace) printf("vma_memory_read24014:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read24013;
  arg4 = (u32)arg5;   		// Do the indirect thing 
  goto vma_memory_read24010;   

vma_memory_read24013:
  if (_trace) printf("vma_memory_read24013:\n");
  t4 = *(u64 *)&(processor->header);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = arg6 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg4;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t4;   		// Adjust for a longword load 
  t4 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read24016:
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

force_alignment24025:
  if (_trace) printf("force_alignment24025:\n");
  if (t3 == 0) 
    goto basic_dispatch24022;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t6;
  iSP = iSP + 8;
  goto cachevalid;   

basic_dispatch24022:
  if (_trace) printf("basic_dispatch24022:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 63;
  goto illegaloperand;

basic_dispatch24021:
  if (_trace) printf("basic_dispatch24021:\n");

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

force_alignment24043:
  if (_trace) printf("force_alignment24043:\n");
  if (t6 == 0) 
    goto basic_dispatch24031;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment24035:
  if (_trace) printf("force_alignment24035:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24026;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 > 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch24032:
  if (_trace) printf("basic_dispatch24032:\n");

basic_dispatch24031:
  if (_trace) printf("basic_dispatch24031:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment24044:
  if (_trace) printf("force_alignment24044:\n");
  if (t6 == 0) 
    goto basic_dispatch24036;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment24040:
  if (_trace) printf("force_alignment24040:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24026;
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

basic_dispatch24037:
  if (_trace) printf("basic_dispatch24037:\n");

basic_dispatch24036:
  if (_trace) printf("basic_dispatch24036:\n");
  /* Here for all other cases */

binary_type_dispatch24026:
  if (_trace) printf("binary_type_dispatch24026:\n");
  goto greaterpmmexc;   

basic_dispatch24030:
  if (_trace) printf("basic_dispatch24030:\n");

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

force_alignment24049:
  if (_trace) printf("force_alignment24049:\n");
  if (t4 == 0) 
    goto basic_dispatch24046;
  /* Here if argument TypeFixnum */
  t2 = arg4 - arg2;   
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if ((s64)t2 > 0)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch24046:
  if (_trace) printf("basic_dispatch24046:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch24045:
  if (_trace) printf("basic_dispatch24045:\n");

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

force_alignment24062:
  if (_trace) printf("force_alignment24062:\n");
  if (t6 == 0) 
    goto basic_dispatch24055;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment24059:
  if (_trace) printf("force_alignment24059:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24052;
  /* Here if argument TypeFixnum */
  t2 = arg4 & arg2;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  		// Pop/No-pop 
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch24056:
  if (_trace) printf("basic_dispatch24056:\n");

basic_dispatch24055:
  if (_trace) printf("basic_dispatch24055:\n");
  /* Here for all other cases */

binary_type_dispatch24051:
  if (_trace) printf("binary_type_dispatch24051:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch24053;   

binary_type_dispatch24052:
  if (_trace) printf("binary_type_dispatch24052:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch24053:
  if (_trace) printf("binary_type_dispatch24053:\n");

basic_dispatch24054:
  if (_trace) printf("basic_dispatch24054:\n");

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

force_alignment24067:
  if (_trace) printf("force_alignment24067:\n");
  if (t4 == 0) 
    goto basic_dispatch24064;
  /* Here if argument TypeFixnum */
  t2 = arg4 & arg2;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iSP = (t7 * 8) + iSP;  
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  if (t2)   		// T if the test succeeds 
    t11 = t12;
  *(u64 *)iSP = t11;   
  goto cachevalid;   

basic_dispatch24064:
  if (_trace) printf("basic_dispatch24064:\n");
  /* Here for all other cases */
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

basic_dispatch24063:
  if (_trace) printf("basic_dispatch24063:\n");

/* end DoLogtest */
  /* End of Halfword operand from stack instruction - DoLogtest */
/* start EqualNumberMMExc */


equalnumbermmexc:
  if (_trace) printf("equalnumbermmexc:\n");
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment24085:
  if (_trace) printf("force_alignment24085:\n");
  if (t6 == 0) 
    goto basic_dispatch24073;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment24077:
  if (_trace) printf("force_alignment24077:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24070;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto equalnumbermmexcfltflt;   

basic_dispatch24074:
  if (_trace) printf("basic_dispatch24074:\n");

basic_dispatch24073:
  if (_trace) printf("basic_dispatch24073:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment24086:
  if (_trace) printf("force_alignment24086:\n");
  if (t6 == 0) 
    goto basic_dispatch24078;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment24082:
  if (_trace) printf("force_alignment24082:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24070;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto equalnumbermmexcfltflt;   

basic_dispatch24079:
  if (_trace) printf("basic_dispatch24079:\n");

basic_dispatch24078:
  if (_trace) printf("basic_dispatch24078:\n");
  /* Here for all other cases */

binary_type_dispatch24069:
  if (_trace) printf("binary_type_dispatch24069:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch24071;   

binary_type_dispatch24070:
  if (_trace) printf("binary_type_dispatch24070:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch24071:
  if (_trace) printf("binary_type_dispatch24071:\n");

basic_dispatch24072:
  if (_trace) printf("basic_dispatch24072:\n");

/* end EqualNumberMMExc */
/* start LesspMMExc */


lesspmmexc:
  if (_trace) printf("lesspmmexc:\n");
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment24104:
  if (_trace) printf("force_alignment24104:\n");
  if (t6 == 0) 
    goto basic_dispatch24092;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment24096:
  if (_trace) printf("force_alignment24096:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24089;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto lesspmmexcfltflt;   

basic_dispatch24093:
  if (_trace) printf("basic_dispatch24093:\n");

basic_dispatch24092:
  if (_trace) printf("basic_dispatch24092:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment24105:
  if (_trace) printf("force_alignment24105:\n");
  if (t6 == 0) 
    goto basic_dispatch24097;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment24101:
  if (_trace) printf("force_alignment24101:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24089;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto lesspmmexcfltflt;   

basic_dispatch24098:
  if (_trace) printf("basic_dispatch24098:\n");

basic_dispatch24097:
  if (_trace) printf("basic_dispatch24097:\n");
  /* Here for all other cases */

binary_type_dispatch24088:
  if (_trace) printf("binary_type_dispatch24088:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch24090;   

binary_type_dispatch24089:
  if (_trace) printf("binary_type_dispatch24089:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch24090:
  if (_trace) printf("binary_type_dispatch24090:\n");

basic_dispatch24091:
  if (_trace) printf("basic_dispatch24091:\n");

/* end LesspMMExc */
/* start GreaterpMMExc */


greaterpmmexc:
  if (_trace) printf("greaterpmmexc:\n");
  t5 = arg3 & 63;		// Strip off any CDR code bits. 
  t4 = t1 & 63;		// Strip off any CDR code bits. 
  t6 = (t5 == Type_Fixnum) ? 1 : 0;   

force_alignment24123:
  if (_trace) printf("force_alignment24123:\n");
  if (t6 == 0) 
    goto basic_dispatch24111;
  /* Here if argument TypeFixnum */
  t3 = (t4 == Type_SingleFloat) ? 1 : 0;   

force_alignment24115:
  if (_trace) printf("force_alignment24115:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24108;
  /* Here if argument TypeSingleFloat */
  CVTLQ(1, f1, f31, 1, f1);
  CVTQS(1, f1, f31, 1, f1);
  goto greaterpmmexcfltflt;   

basic_dispatch24112:
  if (_trace) printf("basic_dispatch24112:\n");

basic_dispatch24111:
  if (_trace) printf("basic_dispatch24111:\n");
  t6 = (t5 == Type_SingleFloat) ? 1 : 0;   

force_alignment24124:
  if (_trace) printf("force_alignment24124:\n");
  if (t6 == 0) 
    goto basic_dispatch24116;
  /* Here if argument TypeSingleFloat */
  t3 = (t4 == Type_Fixnum) ? 1 : 0;   

force_alignment24120:
  if (_trace) printf("force_alignment24120:\n");
  if (t3 == 0) 
    goto binary_type_dispatch24108;
  /* Here if argument TypeFixnum */
  CVTLQ(2, f2, f31, 2, f2);
  CVTQS(2, f2, f31, 2, f2);
  goto greaterpmmexcfltflt;   

basic_dispatch24117:
  if (_trace) printf("basic_dispatch24117:\n");

basic_dispatch24116:
  if (_trace) printf("basic_dispatch24116:\n");
  /* Here for all other cases */

binary_type_dispatch24107:
  if (_trace) printf("binary_type_dispatch24107:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch24109;   

binary_type_dispatch24108:
  if (_trace) printf("binary_type_dispatch24108:\n");
  arg6 = t1;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch24109:
  if (_trace) printf("binary_type_dispatch24109:\n");

basic_dispatch24110:
  if (_trace) printf("basic_dispatch24110:\n");

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

force_alignment24125:
  if (_trace) printf("force_alignment24125:\n");
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
    goto i_allocate_block24126;
  t4 = *(s32 *)&processor->lclength;   
  t2 = (arg3 == t1) ? 1 : 0;   
  if (t2 == 0) 		// Wrong area 
    goto i_allocate_block24127;
  t2 = t4 - arg1;   		// Effectively an unsigned 32-bit compare 
  if ((s64)t2 < 0)   		// Insufficient cache 
    goto i_allocate_block24127;
  t1 = *(u64 *)&(processor->lcaddress);   		// Fetch address 
  t3 = (-16384) << 16;   
  t3 = (u32)t3;   
		/* Store remaining length */
  *(u32 *)&processor->lclength = t2;
  *(u64 *)iSP = t1;   		// Cache address/tag -> TOS 
		/* Cache address -> BAR1 */
  *(u32 *)&processor->bar1 = t1;
  t1 = (u32)t1;   
  t4 = *(s32 *)&processor->control;   		// Verify trap mode 
  t1 = t1 + arg1;		// Increment address 
		/* Store updated address */
  *(u32 *)&processor->lcaddress = t1;
  t3 = t3 & t4;
  if (t3 != 0)   		// Already above emulator mode 
    goto NEXTINSTRUCTION;
  t3 = (16384) << 16;   
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;
  goto NEXTINSTRUCTION;   

i_allocate_block24126:
  if (_trace) printf("i_allocate_block24126:\n");
  arg5 = 0;
  arg2 = 1;
  goto illegaloperand;

i_allocate_block24127:
  if (_trace) printf("i_allocate_block24127:\n");
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
    goto i_allocate_block24128;
  t4 = *(s32 *)&processor->sclength;   
  t2 = (arg3 == t1) ? 1 : 0;   
  if (t2 == 0) 		// Wrong area 
    goto i_allocate_block24129;
  t2 = t4 - arg1;   		// Effectively an unsigned 32-bit compare 
  if ((s64)t2 < 0)   		// Insufficient cache 
    goto i_allocate_block24129;
  t1 = *(u64 *)&(processor->scaddress);   		// Fetch address 
  t3 = (-16384) << 16;   
  t3 = (u32)t3;   
		/* Store remaining length */
  *(u32 *)&processor->sclength = t2;
  *(u64 *)iSP = t1;   		// Cache address/tag -> TOS 
		/* Cache address -> BAR1 */
  *(u32 *)&processor->bar1 = t1;
  t1 = (u32)t1;   
  t4 = *(s32 *)&processor->control;   		// Verify trap mode 
  t1 = t1 + arg1;		// Increment address 
		/* Store updated address */
  *(u32 *)&processor->scaddress = t1;
  t3 = t3 & t4;
  if (t3 != 0)   		// Already above emulator mode 
    goto NEXTINSTRUCTION;
  t3 = (16384) << 16;   
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;
  goto NEXTINSTRUCTION;   

i_allocate_block24128:
  if (_trace) printf("i_allocate_block24128:\n");
  arg5 = 0;
  arg2 = 1;
  goto illegaloperand;

i_allocate_block24129:
  if (_trace) printf("i_allocate_block24129:\n");
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
		/* Save result and coerce to a FIXNUM */
  *(u32 *)iSP = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
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
		/* Save result and coerce to a FIXNUM */
  *(u32 *)iSP = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
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
		/* Put result back */
  *(u32 *)arg1 = t3;
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

force_alignment24146:
  if (_trace) printf("force_alignment24146:\n");
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

vma_memory_read24130:
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
    goto vma_memory_read24132;

vma_memory_read24131:
  t1 = *(u64 *)&(processor->dataread_mask);   
  t3 = zero + 240;   
  t1 = t1 >> (t4 & 63);   
  t3 = t3 >> (t4 & 63);   
  if (t1 & 1)   
    goto vma_memory_read24134;

vma_memory_read24141:
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

force_alignment24144:
  if (_trace) printf("force_alignment24144:\n");
  t3 = t3 | t2;
  STQ_U(t1, t3);   
  *(u32 *)t5 = arg1;
  if (t6 != 0)   		// J. if in cache 
    goto vma_memory_write24143;

vma_memory_write24142:
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

vma_memory_write24143:
  if (_trace) printf("vma_memory_write24143:\n");
  t2 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24145:
  if (_trace) printf("force_alignment24145:\n");
  t1 = *(u64 *)&(processor->stackcachedata);   
  t2 = arg6 - t2;   		// Stack cache offset 
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t1 = arg1;
		/* write the stack cache */
  *(u32 *)(t1 + 4) = t4;
  goto vma_memory_write24142;   

vma_memory_read24132:
  if (_trace) printf("vma_memory_read24132:\n");
  t2 = *(u64 *)&(processor->stackcachedata);   
  t1 = (t1 * 8) + t2;  		// reconstruct SCA 
  t5 = *(s32 *)t1;   
  t4 = *(s32 *)(t1 + 4);   		// Read from stack cache 
  goto vma_memory_read24131;   

vma_memory_read24134:
  if (_trace) printf("vma_memory_read24134:\n");
  if ((t3 & 1) == 0)   
    goto vma_memory_read24133;
  arg6 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read24130;   

vma_memory_read24133:
  if (_trace) printf("vma_memory_read24133:\n");
  t1 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t3 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg6;   		// stash the VMA for the (likely) trap 
  t3 = (t3 * 4) + t1;   		// Adjust for a longword load 
  t1 = *(s32 *)t3;   		// Get the memory action 

vma_memory_read24138:
  if (_trace) printf("vma_memory_read24138:\n");
  t3 = t1 & MemoryActionTransform;
  if (t3 == 0) 
    goto vma_memory_read24137;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read24141;   

vma_memory_read24137:

vma_memory_read24136:
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

force_alignment24150:
  if (_trace) printf("force_alignment24150:\n");
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

force_alignment24148:
  if (_trace) printf("force_alignment24148:\n");
  t3 = t3 | t2;
  STQ_U(t1, t3);   
  *(u32 *)t4 = arg1;
  if (t5 != 0)   		// J. if in cache 
    goto vma_memory_write24147;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write24147:
  if (_trace) printf("vma_memory_write24147:\n");
  t2 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24149:
  if (_trace) printf("force_alignment24149:\n");
  t1 = *(u64 *)&(processor->stackcachedata);   
  t2 = arg4 - t2;   		// Stack cache offset 
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t1 = arg1;
		/* write the stack cache */
  *(u32 *)(t1 + 4) = arg2;
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

force_alignment24163:
  if (_trace) printf("force_alignment24163:\n");
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

vma_memory_read24151:
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
    goto vma_memory_read24153;

vma_memory_read24152:

vma_memory_read24159:
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

force_alignment24161:
  if (_trace) printf("force_alignment24161:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = arg1;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write24160;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

vma_memory_write24160:
  if (_trace) printf("vma_memory_write24160:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24162:
  if (_trace) printf("force_alignment24162:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg4 - t7;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = arg1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read24153:
  if (_trace) printf("vma_memory_read24153:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read24152;   

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
		/* Replace the CDE CODE/TAG */
  *(u32 *)(arg1 + 4) = t1;
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
		/* Replace the CDE CODE/TAG */
  *(u32 *)(arg1 + 4) = t1;
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

force_alignment24234:
  if (_trace) printf("force_alignment24234:\n");
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

force_alignment24224:
  if (_trace) printf("force_alignment24224:\n");
  if (t1 == 0) 
    goto basic_dispatch24165;
  /* Here if argument ALUFunctionBoolean */
  t10 = arg6 >> 10;   
  t10 = t10 & 15;		// Extract the ALU boolean function 
  t1 = (t10 == Boole_Clear) ? 1 : 0;   

force_alignment24184:
  if (_trace) printf("force_alignment24184:\n");
  if (t1 != 0)   
    goto basic_dispatch24166;

basic_dispatch24167:
  if (_trace) printf("basic_dispatch24167:\n");
  t1 = (t10 == Boole_And) ? 1 : 0;   

force_alignment24185:
  if (_trace) printf("force_alignment24185:\n");
  if (t1 == 0) 
    goto basic_dispatch24168;
  /* Here if argument BooleAnd */
  t10 = arg4 & arg1;
  goto basic_dispatch24166;   

basic_dispatch24168:
  if (_trace) printf("basic_dispatch24168:\n");
  t1 = (t10 == Boole_AndC1) ? 1 : 0;   

force_alignment24186:
  if (_trace) printf("force_alignment24186:\n");
  if (t1 == 0) 
    goto basic_dispatch24169;
  /* Here if argument BooleAndC1 */
  t10 = arg1 & ~arg4;
  goto basic_dispatch24166;   

basic_dispatch24169:
  if (_trace) printf("basic_dispatch24169:\n");
  t1 = (t10 == Boole_2) ? 1 : 0;   

force_alignment24187:
  if (_trace) printf("force_alignment24187:\n");
  if (t1 == 0) 
    goto basic_dispatch24170;
  /* Here if argument Boole2 */
  t10 = arg1;
  goto basic_dispatch24166;   

basic_dispatch24170:
  if (_trace) printf("basic_dispatch24170:\n");
  t1 = (t10 == Boole_AndC2) ? 1 : 0;   

force_alignment24188:
  if (_trace) printf("force_alignment24188:\n");
  if (t1 == 0) 
    goto basic_dispatch24171;
  /* Here if argument BooleAndC2 */
  t10 = arg4 & ~arg1;
  goto basic_dispatch24166;   

basic_dispatch24171:
  if (_trace) printf("basic_dispatch24171:\n");
  t1 = (t10 == Boole_1) ? 1 : 0;   

force_alignment24189:
  if (_trace) printf("force_alignment24189:\n");
  if (t1 == 0) 
    goto basic_dispatch24172;
  /* Here if argument Boole1 */
  t10 = arg4;
  goto basic_dispatch24166;   

basic_dispatch24172:
  if (_trace) printf("basic_dispatch24172:\n");
  t1 = (t10 == Boole_Xor) ? 1 : 0;   

force_alignment24190:
  if (_trace) printf("force_alignment24190:\n");
  if (t1 == 0) 
    goto basic_dispatch24173;
  /* Here if argument BooleXor */
  t10 = arg4 ^ arg1;   
  goto basic_dispatch24166;   

basic_dispatch24173:
  if (_trace) printf("basic_dispatch24173:\n");
  t1 = (t10 == Boole_Ior) ? 1 : 0;   

force_alignment24191:
  if (_trace) printf("force_alignment24191:\n");
  if (t1 == 0) 
    goto basic_dispatch24174;
  /* Here if argument BooleIor */
  t10 = arg4 | arg1;
  goto basic_dispatch24166;   

basic_dispatch24174:
  if (_trace) printf("basic_dispatch24174:\n");
  t1 = (t10 == Boole_Nor) ? 1 : 0;   

force_alignment24192:
  if (_trace) printf("force_alignment24192:\n");
  if (t1 == 0) 
    goto basic_dispatch24175;
  /* Here if argument BooleNor */
  t10 = arg4 | arg1;
  t10 = ~t10;   
  goto basic_dispatch24166;   

basic_dispatch24175:
  if (_trace) printf("basic_dispatch24175:\n");
  t1 = (t10 == Boole_Equiv) ? 1 : 0;   

force_alignment24193:
  if (_trace) printf("force_alignment24193:\n");
  if (t1 == 0) 
    goto basic_dispatch24176;
  /* Here if argument BooleEquiv */
  t10 = arg4 ^ arg1;   
  t10 = ~t10;   
  goto basic_dispatch24166;   

basic_dispatch24176:
  if (_trace) printf("basic_dispatch24176:\n");
  t1 = (t10 == Boole_C1) ? 1 : 0;   

force_alignment24194:
  if (_trace) printf("force_alignment24194:\n");
  if (t1 == 0) 
    goto basic_dispatch24177;
  /* Here if argument BooleC1 */
  t10 = ~arg4;   
  goto basic_dispatch24166;   

basic_dispatch24177:
  if (_trace) printf("basic_dispatch24177:\n");
  t1 = (t10 == Boole_OrC1) ? 1 : 0;   

force_alignment24195:
  if (_trace) printf("force_alignment24195:\n");
  if (t1 == 0) 
    goto basic_dispatch24178;
  /* Here if argument BooleOrC1 */
  t10 = arg1 | ~(arg4);   
  goto basic_dispatch24166;   

basic_dispatch24178:
  if (_trace) printf("basic_dispatch24178:\n");
  t1 = (t10 == Boole_C2) ? 1 : 0;   

force_alignment24196:
  if (_trace) printf("force_alignment24196:\n");
  if (t1 == 0) 
    goto basic_dispatch24179;
  /* Here if argument BooleC2 */
  t10 = ~arg1;   
  goto basic_dispatch24166;   

basic_dispatch24179:
  if (_trace) printf("basic_dispatch24179:\n");
  t1 = (t10 == Boole_OrC2) ? 1 : 0;   

force_alignment24197:
  if (_trace) printf("force_alignment24197:\n");
  if (t1 == 0) 
    goto basic_dispatch24180;
  /* Here if argument BooleOrC2 */
  t10 = arg4 & ~arg1;
  goto basic_dispatch24166;   

basic_dispatch24180:
  if (_trace) printf("basic_dispatch24180:\n");
  t1 = (t10 == Boole_Nand) ? 1 : 0;   

force_alignment24198:
  if (_trace) printf("force_alignment24198:\n");
  if (t1 == 0) 
    goto basic_dispatch24181;
  /* Here if argument BooleNand */
  t10 = arg4 & arg1;
  goto basic_dispatch24166;   

basic_dispatch24181:
  if (_trace) printf("basic_dispatch24181:\n");
  t1 = (t10 == Boole_Set) ? 1 : 0;   

force_alignment24199:
  if (_trace) printf("force_alignment24199:\n");
  if (t1 == 0) 
    goto basic_dispatch24166;
  /* Here if argument BooleSet */
  t10 = ~zero;   

basic_dispatch24166:
  if (_trace) printf("basic_dispatch24166:\n");
  *(u32 *)iSP = t10;
  goto NEXTINSTRUCTION;   

basic_dispatch24165:
  if (_trace) printf("basic_dispatch24165:\n");
  t1 = (arg5 == ALUFunction_Byte) ? 1 : 0;   

force_alignment24225:
  if (_trace) printf("force_alignment24225:\n");
  if (t1 == 0) 
    goto basic_dispatch24200;
  /* Here if argument ALUFunctionByte */
  t2 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t3 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t1 = arg6 >> 10;   
  t1 = t1 & 3;		// Extract the byte background 
  t4 = (t1 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment24207:
  if (_trace) printf("force_alignment24207:\n");
  if (t4 == 0) 
    goto basic_dispatch24203;
  /* Here if argument ALUByteBackgroundOp1 */
  t1 = arg4;

basic_dispatch24202:
  if (_trace) printf("basic_dispatch24202:\n");
  t5 = arg6 >> 12;   
  t5 = t5 & 1;		// Extractthe byte rotate latch 
  t10 = arg1 << (t2 & 63);   
  t4 = (u32)(t10 >> ((4&7)*8));   
  t10 = (u32)t10;   
  t10 = t10 | t4;		// OP2 rotated 
  if (t5 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte24201;
  *(u64 *)&processor->rotatelatch = t10;   

alu_function_byte24201:
  if (_trace) printf("alu_function_byte24201:\n");
  t5 = zero + -2;   
  t5 = t5 << (t3 & 63);   
  t5 = ~t5;   		// Compute mask 
  /* Get byte function */
  t4 = arg6 >> 13;   
  t4 = t4 & 1;
  t3 = (t4 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment24212:
  if (_trace) printf("force_alignment24212:\n");
  if (t3 == 0) 
    goto basic_dispatch24209;
  /* Here if argument ALUByteFunctionDpb */
  t5 = t5 << (t2 & 63);   		// Position mask 

basic_dispatch24208:
  if (_trace) printf("basic_dispatch24208:\n");
  t10 = t10 & t5;		// rotated&mask 
  t1 = t1 & ~t5;		// background&~mask 
  t10 = t10 | t1;
  *(u32 *)iSP = t10;
  goto NEXTINSTRUCTION;   

basic_dispatch24200:
  if (_trace) printf("basic_dispatch24200:\n");
  t1 = (arg5 == ALUFunction_Adder) ? 1 : 0;   

force_alignment24226:
  if (_trace) printf("force_alignment24226:\n");
  if (t1 == 0) 
    goto basic_dispatch24213;
  /* Here if argument ALUFunctionAdder */
  t3 = arg6 >> 11;   
  t3 = t3 & 3;		// Extract the op2 
  t2 = arg6 >> 10;   
  t2 = t2 & 1;		// Extract the adder carry in 
  t4 = (t3 == ALUAdderOp2_Op2) ? 1 : 0;   

force_alignment24221:
  if (_trace) printf("force_alignment24221:\n");
  if (t4 == 0) 
    goto basic_dispatch24216;
  /* Here if argument ALUAdderOp2Op2 */
  t1 = arg1;

basic_dispatch24215:
  if (_trace) printf("basic_dispatch24215:\n");
  t10 = arg4 + t1;
  t10 = t10 + t2;
  t3 = t10 >> 31;   		// Sign bit 
  t4 = t10 >> 32;   		// Next bit 
  t3 = t3 ^ t4;   		// Low bit is now overflow indicator 
  t4 = arg6 >> 24;   		// Get the load-carry-in bit 
  *(u64 *)&processor->aluoverflow = t3;   
  if ((t4 & 1) == 0)   
    goto alu_function_adder24214;
  t3 = (u32)(t10 >> ((4&7)*8));   		// Get the carry 
  t4 = zero + 1024;   
  arg6 = arg6 & ~t4;
  t4 = t3 & 1;
  t4 = t4 << 10;   
  arg6 = arg6 | t4;		// Set the adder carry in 
  *(u64 *)&processor->aluandrotatecontrol = arg6;   

alu_function_adder24214:
  if (_trace) printf("alu_function_adder24214:\n");
  t3 = ((s64)arg4 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->aluborrow = t3;   
  arg4 = (s32)arg4;
  arg1 = (s32)arg1;
  t3 = ((s64)arg4 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->alulessthan = t3;   
  *(u32 *)iSP = t10;
  goto NEXTINSTRUCTION;   

basic_dispatch24213:
  if (_trace) printf("basic_dispatch24213:\n");
  t1 = (arg5 == ALUFunction_MultiplyDivide) ? 1 : 0;   

force_alignment24227:
  if (_trace) printf("force_alignment24227:\n");
  if (t1 == 0) 
    goto basic_dispatch24164;
  /* Here if argument ALUFunctionMultiplyDivide */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;
  *(u32 *)iSP = t10;
  goto NEXTINSTRUCTION;   

basic_dispatch24164:
  if (_trace) printf("basic_dispatch24164:\n");

aluexc:
  if (_trace) printf("aluexc:\n");
  arg5 = 0;
  arg2 = 80;
  goto illegaloperand;

basic_dispatch24216:
  if (_trace) printf("basic_dispatch24216:\n");
  t4 = (t3 == ALUAdderOp2_Zero) ? 1 : 0;   

force_alignment24228:
  if (_trace) printf("force_alignment24228:\n");
  if (t4 == 0) 
    goto basic_dispatch24217;
  /* Here if argument ALUAdderOp2Zero */
  t1 = zero;
  goto basic_dispatch24215;   

basic_dispatch24217:
  if (_trace) printf("basic_dispatch24217:\n");
  t4 = (t3 == ALUAdderOp2_Invert) ? 1 : 0;   

force_alignment24229:
  if (_trace) printf("force_alignment24229:\n");
  if (t4 == 0) 
    goto basic_dispatch24218;
  /* Here if argument ALUAdderOp2Invert */
  t1 = (s32)arg1;
  t1 = zero - t1;   
  t1 = (u32)t1;   
  goto basic_dispatch24215;   

basic_dispatch24218:
  if (_trace) printf("basic_dispatch24218:\n");
  t4 = (t3 == ALUAdderOp2_MinusOne) ? 1 : 0;   

force_alignment24230:
  if (_trace) printf("force_alignment24230:\n");
  if (t4 == 0) 
    goto basic_dispatch24215;
  /* Here if argument ALUAdderOp2MinusOne */
  t1 = ~zero;   
  t1 = (u32)t1;   
  goto basic_dispatch24215;   

basic_dispatch24209:
  if (_trace) printf("basic_dispatch24209:\n");
  t3 = (t4 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment24231:
  if (_trace) printf("force_alignment24231:\n");
  if (t3 != 0)   
    goto basic_dispatch24208;
  goto basic_dispatch24208;   

basic_dispatch24203:
  if (_trace) printf("basic_dispatch24203:\n");
  t4 = (t1 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment24232:
  if (_trace) printf("force_alignment24232:\n");
  if (t4 == 0) 
    goto basic_dispatch24204;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t1 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch24202;   

basic_dispatch24204:
  if (_trace) printf("basic_dispatch24204:\n");
  t4 = (t1 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment24233:
  if (_trace) printf("force_alignment24233:\n");
  if (t4 == 0) 
    goto basic_dispatch24202;
  /* Here if argument ALUByteBackgroundZero */
  t1 = zero;
  goto basic_dispatch24202;   

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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterSP */
/* start ReadRegisterStackCacheLowerBound */


ReadRegisterStackCacheLowerBound:
  if (_trace) printf("ReadRegisterStackCacheLowerBound:\n");
  t3 = *(u64 *)&(processor->stackcachebasevma);   
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterAluAndRotateControl */
/* start ReadRegisterControlRegister */


ReadRegisterControlRegister:
  if (_trace) printf("ReadRegisterControlRegister:\n");
  t3 = *(s32 *)&processor->control;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterCRArgumentSize */
/* start ReadRegisterEphemeralOldspaceRegister */


ReadRegisterEphemeralOldspaceRegister:
  if (_trace) printf("ReadRegisterEphemeralOldspaceRegister:\n");
  t3 = *(s32 *)&processor->ephemeraloldspace;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterEphemeralOldspaceRegister */
/* start ReadRegisterZoneOldspaceRegister */


ReadRegisterZoneOldspaceRegister:
  if (_trace) printf("ReadRegisterZoneOldspaceRegister:\n");
  t3 = *(s32 *)&processor->zoneoldspace;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterZoneOldspaceRegister */
/* start ReadRegisterChipRevision */


ReadRegisterChipRevision:
  if (_trace) printf("ReadRegisterChipRevision:\n");
  t3 = 5;
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterChipRevision */
/* start ReadRegisterFPCoprocessorPresent */


ReadRegisterFPCoprocessorPresent:
  if (_trace) printf("ReadRegisterFPCoprocessorPresent:\n");
  t4 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPreemptRegister */
/* start ReadRegisterIcacheControl */


ReadRegisterIcacheControl:
  if (_trace) printf("ReadRegisterIcacheControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterIcacheControl */
/* start ReadRegisterPrefetcherControl */


ReadRegisterPrefetcherControl:
  if (_trace) printf("ReadRegisterPrefetcherControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPrefetcherControl */
/* start ReadRegisterMapCacheControl */


ReadRegisterMapCacheControl:
  if (_trace) printf("ReadRegisterMapCacheControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterMapCacheControl */
/* start ReadRegisterMemoryControl */


ReadRegisterMemoryControl:
  if (_trace) printf("ReadRegisterMemoryControl:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStackCacheOverflowLimit */
/* start ReadRegisterMicrosecondClock */


ReadRegisterMicrosecondClock:
  if (_trace) printf("ReadRegisterMicrosecondClock:\n");
  t1 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterControlStackLimit */
/* start ReadRegisterControlStackExtraLimit */


ReadRegisterControlStackExtraLimit:
  if (_trace) printf("ReadRegisterControlStackExtraLimit:\n");
  t3 = *(s32 *)&processor->csextralimit;   
  t5 = Type_Locative;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPHTBase */
/* start ReadRegisterPHTMask */


ReadRegisterPHTMask:
  if (_trace) printf("ReadRegisterPHTMask:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterPHTMask */
/* start ReadRegisterCountMapReloads */


ReadRegisterCountMapReloads:
  if (_trace) printf("ReadRegisterCountMapReloads:\n");
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = zero;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterChoicePointer */
/* start ReadRegisterStructureStackChoicePointer */


ReadRegisterStructureStackChoicePointer:
  if (_trace) printf("ReadRegisterStructureStackChoicePointer:\n");
  t3 = *(s32 *)&processor->sstkchoiceptr;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

/* end ReadRegisterStackFrameMaximumSize */
/* start ReadRegisterStackCacheDumpQuantum */


ReadRegisterStackCacheDumpQuantum:
  if (_trace) printf("ReadRegisterStackCacheDumpQuantum:\n");
  t3 = zero + 896;   
  t5 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
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
#ifdef IVERIFY
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of the stack cache 
  t1 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t2 = arg3 - t2;   		// Stack cache offset 
  t3 = ((u64)t2 < (u64)t1) ? 1 : 0;   		// In range? 
  t1 = *(u64 *)&(processor->stackcachedata);   
  if (t3 == 0) 		// J. if not in cache 
    goto badregister;
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
  iFP = t1;
  goto NEXTINSTRUCTION;   
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;
#endif

/* end WriteRegisterFP */
/* start WriteRegisterLP */


WriteRegisterLP:
  if (_trace) printf("WriteRegisterLP:\n");
#ifdef IVERIFY
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of the stack cache 
  t1 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t2 = arg3 - t2;   		// Stack cache offset 
  t3 = ((u64)t2 < (u64)t1) ? 1 : 0;   		// In range? 
  t1 = *(u64 *)&(processor->stackcachedata);   
  if (t3 == 0) 		// J. if not in cache 
    goto badregister;
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
  iLP = t1;
  goto NEXTINSTRUCTION;   
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;
#endif

/* end WriteRegisterLP */
/* start WriteRegisterSP */


WriteRegisterSP:
  if (_trace) printf("WriteRegisterSP:\n");
#ifdef IVERIFY
  t2 = *(u64 *)&(processor->stackcachebasevma);   		// Base of the stack cache 
  t1 = *(s32 *)&processor->scovlimit;   		// Size of the stack cache (words) 
  t2 = arg3 - t2;   		// Stack cache offset 
  t3 = ((u64)t2 < (u64)t1) ? 1 : 0;   		// In range? 
  t1 = *(u64 *)&(processor->stackcachedata);   
  if (t3 == 0) 		// J. if not in cache 
    goto badregister;
  t1 = (t2 * 8) + t1;  		// reconstruct SCA 
  iSP = t1;
  goto NEXTINSTRUCTION;   
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;
#endif

/* end WriteRegisterSP */
/* start WriteRegisterStackCacheLowerBound */


WriteRegisterStackCacheLowerBound:
  if (_trace) printf("WriteRegisterStackCacheLowerBound:\n");
#ifdef IVERIFY
  *(u64 *)&processor->stackcachebasevma = arg3;   
  t1 = *(u64 *)&(processor->stackcachesize);   
  t1 = arg3 + t1;
  *(u64 *)&processor->stackcachetopvma = t1;   
  goto NEXTINSTRUCTION;   
  arg5 = 0;
  arg2 = 84;
  goto illegaloperand;
#endif

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
    goto mondo_dispatch24236;
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
		/* Put it back */
  *(u32 *)(iFP + 4) = t1;
  t2 = t2 | 192;		// Set CDR-CODE to 3 
		/* Put it back */
  *(u32 *)(iFP + 12) = t2;
  /* Copy the current trap-on-exit bit into the saved control register */
  t1 = *(s32 *)&processor->control;   		// Get control register 
  t2 = *(s32 *)(iFP + 8);   		// Get saved control register 
  t2 = (u32)t2;   
  t3 = (256) << 16;   
  t2 = t2 & ~t3;		// Remove saved control register's trap-on-exit bit 
  t1 = t1 & t3;		// Extract control register's trap-on-exit bit 
  t2 = t2 | t1;		// Copy it into saved control register 
		/* Update saved control register */
  *(u32 *)(iFP + 8) = t2;
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
  goto mondo_dispatch24235;   

mondo_dispatch24236:
  if (_trace) printf("mondo_dispatch24236:\n");
  t2 = zero + CoprocessorRegister_FlushIDCaches;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch24237;
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
  goto mondo_dispatch24235;   

mondo_dispatch24237:
  if (_trace) printf("mondo_dispatch24237:\n");
  t2 = zero + CoprocessorRegister_FlushCachesForVMA;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch24238;
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
  goto mondo_dispatch24235;   

mondo_dispatch24238:
  if (_trace) printf("mondo_dispatch24238:\n");
  t2 = zero + CoprocessorRegister_FlushHiddenArrayRegisters;   
  t2 = arg1 - t2;   
  if (t2 != 0)   
    goto mondo_dispatch24239;
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
  goto mondo_dispatch24235;   

mondo_dispatch24239:
  if (_trace) printf("mondo_dispatch24239:\n");
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
  goto mondo_dispatch24235;   

mondo_dispatch24240:
  if (_trace) printf("mondo_dispatch24240:\n");

mondo_dispatch24235:
  if (_trace) printf("mondo_dispatch24235:\n");
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
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

vma_memory_read24242:
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
    goto vma_memory_read24244;

vma_memory_read24243:
  arg4 = (u32)arg4;   

vma_memory_read24250:
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
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

vma_memory_read24244:
  if (_trace) printf("vma_memory_read24244:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg4 = *(s32 *)t3;   
  arg3 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read24243;   

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

vma_memory_read24251:
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
    goto vma_memory_read24253;

vma_memory_read24252:

vma_memory_read24259:
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
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

vma_memory_read24253:
  if (_trace) printf("vma_memory_read24253:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg4 = *(s32 *)t3;   
  arg3 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read24252;   

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

force_alignment24272:
  if (_trace) printf("force_alignment24272:\n");
  if (t2 == 0) 
    goto basic_dispatch24265;
  /* Here if argument TypeFixnum */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;   

force_alignment24269:
  if (_trace) printf("force_alignment24269:\n");
  if (arg5 == 0) 
    goto binary_type_dispatch24262;
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
  goto cachevalid;   

basic_dispatch24266:
  if (_trace) printf("basic_dispatch24266:\n");

basic_dispatch24265:
  if (_trace) printf("basic_dispatch24265:\n");
  /* Here for all other cases */

binary_type_dispatch24261:
  if (_trace) printf("binary_type_dispatch24261:\n");
  arg6 = t5;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch24263;   

binary_type_dispatch24262:
  if (_trace) printf("binary_type_dispatch24262:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch24263:
  if (_trace) printf("binary_type_dispatch24263:\n");

basic_dispatch24264:
  if (_trace) printf("basic_dispatch24264:\n");

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

force_alignment24285:
  if (_trace) printf("force_alignment24285:\n");
  if (t2 == 0) 
    goto basic_dispatch24278;
  /* Here if argument TypeCharacter */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;   

force_alignment24282:
  if (_trace) printf("force_alignment24282:\n");
  if (arg5 == 0) 
    goto binary_type_dispatch24275;
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
  goto cachevalid;   

basic_dispatch24279:
  if (_trace) printf("basic_dispatch24279:\n");

basic_dispatch24278:
  if (_trace) printf("basic_dispatch24278:\n");
  /* Here for all other cases */

binary_type_dispatch24274:
  if (_trace) printf("binary_type_dispatch24274:\n");
  arg6 = t5;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 27;
  goto spareexception;
  goto binary_type_dispatch24276;   

binary_type_dispatch24275:
  if (_trace) printf("binary_type_dispatch24275:\n");
  arg5 = 0;
  arg2 = 27;
  goto illegaloperand;

binary_type_dispatch24276:
  if (_trace) printf("binary_type_dispatch24276:\n");

basic_dispatch24277:
  if (_trace) printf("basic_dispatch24277:\n");

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

vma_memory_read24286:
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
    goto vma_memory_read24288;

vma_memory_read24287:
  t6 = (u32)t6;   

vma_memory_read24294:
  t6 = (u32)t6;   
  t1 = arg3 & 63;		// Strip off any CDR code bits. 
  t10 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment24301:
  if (_trace) printf("force_alignment24301:\n");
  if (t10 == 0) 
    goto basic_dispatch24296;
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

force_alignment24298:
  if (_trace) printf("force_alignment24298:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);   
  *(u32 *)t5 = t6;
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write24297;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

basic_dispatch24296:
  if (_trace) printf("basic_dispatch24296:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

basic_dispatch24295:
  if (_trace) printf("basic_dispatch24295:\n");

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

vma_memory_write24297:
  if (_trace) printf("vma_memory_write24297:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24302:
  if (_trace) printf("force_alignment24302:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = t2 - t4;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t3 = t6;
		/* write the stack cache */
  *(u32 *)(t3 + 4) = t8;
  goto NEXTINSTRUCTION;   

vma_memory_read24288:
  if (_trace) printf("vma_memory_read24288:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t6 = *(s32 *)t3;   
  t8 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read24287;   

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

vma_memory_read24303:
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
    goto vma_memory_read24305;

vma_memory_read24304:

vma_memory_read24311:
  t1 = arg3 & 63;		// Strip off any CDR code bits. 
  t10 = (t1 == Type_Fixnum) ? 1 : 0;   

force_alignment24318:
  if (_trace) printf("force_alignment24318:\n");
  if (t10 == 0) 
    goto basic_dispatch24313;
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

force_alignment24315:
  if (_trace) printf("force_alignment24315:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);   
  *(u32 *)t5 = t8;
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write24314;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

basic_dispatch24313:
  if (_trace) printf("basic_dispatch24313:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

basic_dispatch24312:
  if (_trace) printf("basic_dispatch24312:\n");

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

vma_memory_write24314:
  if (_trace) printf("vma_memory_write24314:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24319:
  if (_trace) printf("force_alignment24319:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = t2 - t4;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t3 = t8;
		/* write the stack cache */
  *(u32 *)(t3 + 4) = t6;
  goto NEXTINSTRUCTION;   

vma_memory_read24305:
  if (_trace) printf("vma_memory_read24305:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t8 = *(s32 *)t3;   
  t6 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma_memory_read24304;   

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

vma_memory_read24320:
  t6 = t1 + ivory;
  t3 = (t6 * 4);   
  t2 = LDQ_U(t6);   
  t4 = t1 - arg5;   		// Stack cache offset 
  t7 = *(u64 *)&(processor->dataread_mask);   
  t5 = ((u64)t4 < (u64)arg6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t6&7)*8));   
  if (t5 != 0)   
    goto vma_memory_read24322;

vma_memory_read24321:
  t6 = zero + 240;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read24324;

vma_memory_read24331:
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  t4 = t2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
  iSP = iSP + 8;
  goto cachevalid;   

pushlexvariop:
  if (_trace) printf("pushlexvariop:\n");
  arg5 = 0;
  arg2 = 82;
  goto illegaloperand;

vma_memory_read24322:
  if (_trace) printf("vma_memory_read24322:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t3 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read24321;   

vma_memory_read24324:
  if (_trace) printf("vma_memory_read24324:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read24323;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24320;   

vma_memory_read24323:
  if (_trace) printf("vma_memory_read24323:\n");
  t7 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read24328:
  if (_trace) printf("vma_memory_read24328:\n");
  t6 = t7 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read24327;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read24331;   

vma_memory_read24327:

vma_memory_read24326:
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

vma_memory_read24332:
  t8 = t1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = t1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read24334;

vma_memory_read24333:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read24336;

vma_memory_read24342:
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

force_alignment24344:
  if (_trace) printf("force_alignment24344:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t3;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write24343;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

poplexvariop:
  if (_trace) printf("poplexvariop:\n");
  arg5 = 0;
  arg2 = 17;
  goto illegaloperand;

vma_memory_write24343:
  if (_trace) printf("vma_memory_write24343:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t3;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read24334:
  if (_trace) printf("vma_memory_read24334:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read24333;   

vma_memory_read24336:
  if (_trace) printf("vma_memory_read24336:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read24335;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read24332;   

vma_memory_read24335:
  if (_trace) printf("vma_memory_read24335:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read24339:

vma_memory_read24338:
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

vma_memory_read24345:
  t8 = t1 + ivory;
  t5 = (t8 * 4);   
  t4 = LDQ_U(t8);   
  t6 = t1 - arg5;   		// Stack cache offset 
  t9 = *(u64 *)&(processor->datawrite_mask);   
  t7 = ((u64)t6 < (u64)arg6) ? 1 : 0;   		// In range? 
  t5 = *(s32 *)t5;   
  t4 = (u8)(t4 >> ((t8&7)*8));   
  if (t7 != 0)   
    goto vma_memory_read24347;

vma_memory_read24346:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read24349;

vma_memory_read24355:
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

force_alignment24357:
  if (_trace) printf("force_alignment24357:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t3;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write24356;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

movemlexvariop:
  if (_trace) printf("movemlexvariop:\n");
  arg5 = 0;
  arg2 = 17;
  goto illegaloperand;

vma_memory_write24356:
  if (_trace) printf("vma_memory_write24356:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - arg5;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t3;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto NEXTINSTRUCTION;   

vma_memory_read24347:
  if (_trace) printf("vma_memory_read24347:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read24346;   

vma_memory_read24349:
  if (_trace) printf("vma_memory_read24349:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read24348;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read24345;   

vma_memory_read24348:
  if (_trace) printf("vma_memory_read24348:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read24352:

vma_memory_read24351:
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
    goto ilogical24358;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical24359;
  /* Here we know that both args are fixnums! */
  t4 = t4 & arg1;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
		/* Push result */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto cachevalid;   

ilogical24358:
  if (_trace) printf("ilogical24358:\n");
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

ilogical24359:
  if (_trace) printf("ilogical24359:\n");
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
    goto ilogical_immediate24360;
  /* Here we know that both args are fixnums! */
  t4 = t4 & arg2;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
		/* Push result */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto cachevalid;   

ilogical_immediate24360:
  if (_trace) printf("ilogical_immediate24360:\n");
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
    goto ilogical24361;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical24362;
  /* Here we know that both args are fixnums! */
  t4 = t4 | arg1;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
		/* Push result */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto cachevalid;   

ilogical24361:
  if (_trace) printf("ilogical24361:\n");
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

ilogical24362:
  if (_trace) printf("ilogical24362:\n");
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
    goto ilogical_immediate24363;
  /* Here we know that both args are fixnums! */
  t4 = t4 | arg2;		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
		/* Push result */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto cachevalid;   

ilogical_immediate24363:
  if (_trace) printf("ilogical_immediate24363:\n");
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
    goto ilogical24364;
  t6 = t1 - Type_Fixnum;   
  t6 = t6 & 63;		// Strip CDR code 
  if (t6 != 0)   
    goto ilogical24365;
  /* Here we know that both args are fixnums! */
  t4 = t4 ^ arg1;   		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
		/* Push result */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto cachevalid;   

ilogical24364:
  if (_trace) printf("ilogical24364:\n");
  arg6 = t3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

ilogical24365:
  if (_trace) printf("ilogical24365:\n");
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
    goto ilogical_immediate24366;
  /* Here we know that both args are fixnums! */
  t4 = t4 ^ arg2;   		// Do the operation 
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  r31 = r31 | r31;
  t4 = (u32)t4;   		// Strip high bits 
  t1 = Type_Fixnum;
		/* Push result */
  *(u32 *)iSP = t4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
  goto cachevalid;   

ilogical_immediate24366:
  if (_trace) printf("ilogical_immediate24366:\n");
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

force_alignment24380:
  if (_trace) printf("force_alignment24380:\n");
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

force_alignment24379:
  if (_trace) printf("force_alignment24379:\n");
  if (t2 == 0) 
    goto basic_dispatch24372;
  /* Here if argument TypeFixnum */
  t4 = (t3 == Type_Fixnum) ? 1 : 0;   

force_alignment24376:
  if (_trace) printf("force_alignment24376:\n");
  if (t4 == 0) 
    goto binary_type_dispatch24369;
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
		/* write the stack cache */
  *(u32 *)(iSP + 4) = arg2;
  goto NEXTINSTRUCTION;   

negash:
  if (_trace) printf("negash:\n");
  arg1 = zero - arg1;   
  arg4 = (s32)arg4;		// Sign extend ARG1 before shifting. 
  arg5 = (s64)arg4 >> $27(arg1 & 63);   		// Shift Right 
  /* TagType. */
  arg2 = arg2 & 63;
  *(u32 *)iSP = arg5;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = arg2;
  goto NEXTINSTRUCTION;   

zerash:
  if (_trace) printf("zerash:\n");
  arg5 = Type_Fixnum;
  *(u32 *)iSP = arg4;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = arg5;
  goto NEXTINSTRUCTION;   

basic_dispatch24373:
  if (_trace) printf("basic_dispatch24373:\n");

basic_dispatch24372:
  if (_trace) printf("basic_dispatch24372:\n");
  /* Here for all other cases */

binary_type_dispatch24368:
  if (_trace) printf("binary_type_dispatch24368:\n");
  arg1 = (u32)arg1;   
  /* SetTag. */
  t2 = arg2 << 32;   
  t2 = arg1 | t2;
  arg6 = arg2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary_type_dispatch24370;   

binary_type_dispatch24369:
  if (_trace) printf("binary_type_dispatch24369:\n");
  arg1 = (u32)arg1;   
  /* SetTag. */
  t2 = arg2 << 32;   
  t2 = arg1 | t2;
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 1;		// arg4 = arithmeticp 
  goto numericexception;

binary_type_dispatch24370:
  if (_trace) printf("binary_type_dispatch24370:\n");

basic_dispatch24371:
  if (_trace) printf("basic_dispatch24371:\n");

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

with_simple_binary_fixnum_operation24382:
  if (_trace) printf("with_simple_binary_fixnum_operation24382:\n");
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
    goto with_simple_binary_fixnum_operation24381;
  t2 = (u32)t2;   
  t5 = t5 - Type_Fixnum;   

force_alignment24383:
  if (_trace) printf("force_alignment24383:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation24381;
  t2 = t2 & 31;		// Get low 5 bits of the rotation 
  t3 = t1 << (t2 & 63);   		// Shift left to get new high bits 
  t6 = (u32)(t3 >> ((4&7)*8));   		// Get new low bits 
  t3 = t3 | t6;		// Glue two parts of shifted operand together 

force_alignment24384:
  if (_trace) printf("force_alignment24384:\n");
  iPC = t7;
		/* Put the result back on the stack */
  *(u32 *)iSP = t3;
  iCP = t8;
  goto cachevalid;   

DoRotIM:
  if (_trace) printf("DoRotIM:\n");
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation24382;   

with_simple_binary_fixnum_operation24381:
  if (_trace) printf("with_simple_binary_fixnum_operation24381:\n");
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

with_simple_binary_fixnum_operation24386:
  if (_trace) printf("with_simple_binary_fixnum_operation24386:\n");
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
    goto with_simple_binary_fixnum_operation24385;
  t5 = t5 - Type_Fixnum;   

force_alignment24387:
  if (_trace) printf("force_alignment24387:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation24385;
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

force_alignment24388:
  if (_trace) printf("force_alignment24388:\n");
  iPC = t7;
		/* Put the result back on the stack */
  *(u32 *)iSP = t3;
  iCP = t8;
  goto cachevalid;   

DoLshIM:
  if (_trace) printf("DoLshIM:\n");
  arg2 = arg2 << 56;   		// sign extend the byte argument. 

force_alignment24389:
  if (_trace) printf("force_alignment24389:\n");
  arg2 = (s64)arg2 >> 56;   		// Rest of sign extension 
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation24386;   

with_simple_binary_fixnum_operation24385:
  if (_trace) printf("with_simple_binary_fixnum_operation24385:\n");
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

with_simple_binary_fixnum_operation24391:
  if (_trace) printf("with_simple_binary_fixnum_operation24391:\n");
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
    goto with_simple_binary_fixnum_operation24390;
  t2 = (u32)t2;   
  t5 = t5 - Type_Fixnum;   

force_alignment24392:
  if (_trace) printf("force_alignment24392:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation24390;
  t3 = t1 + t2;		// Perform the 32 bit Add. 

force_alignment24393:
  if (_trace) printf("force_alignment24393:\n");
  iPC = t7;
		/* Put the result back on the stack */
  *(u32 *)iSP = t3;
  iCP = t8;
  goto cachevalid;   

Do32BitPlusIM:
  if (_trace) printf("Do32BitPlusIM:\n");
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation24391;   

with_simple_binary_fixnum_operation24390:
  if (_trace) printf("with_simple_binary_fixnum_operation24390:\n");
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

with_simple_binary_fixnum_operation24395:
  if (_trace) printf("with_simple_binary_fixnum_operation24395:\n");
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
    goto with_simple_binary_fixnum_operation24394;
  t2 = (u32)t2;   
  t5 = t5 - Type_Fixnum;   

force_alignment24396:
  if (_trace) printf("force_alignment24396:\n");
  if (t5 != 0)   
    goto with_simple_binary_fixnum_operation24394;
  t3 = t1 - t2;   		// Perform the 32 bit Difference. 

force_alignment24397:
  if (_trace) printf("force_alignment24397:\n");
  iPC = t7;
		/* Put the result back on the stack */
  *(u32 *)iSP = t3;
  iCP = t8;
  goto cachevalid;   

Do32BitDifferenceIM:
  if (_trace) printf("Do32BitDifferenceIM:\n");
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = (u64)&processor->immediate_arg;   
  goto with_simple_binary_fixnum_operation24395;   

with_simple_binary_fixnum_operation24394:
  if (_trace) printf("with_simple_binary_fixnum_operation24394:\n");
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

force_alignment24398:
  if (_trace) printf("force_alignment24398:\n");
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

vma_memory_read24403:
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
    goto vma_memory_read24405;

vma_memory_read24404:
  t12 = t12 >> (t8 & 63);   
  t7 = (u32)t7;   
  if (t12 & 1)   
    goto vma_memory_read24407;

vma_memory_read24414:
  if (t6 == 0) 		// J. if we don't have to test for fixnump. 
    goto i_block_n_read_shift24399;
  t9 = t8 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_shift24402;

i_block_n_read_shift24399:
  if (_trace) printf("i_block_n_read_shift24399:\n");
  if (t4 != 0)   		// J. if we don't have to increment the address. 
    goto i_block_n_read_shift24400;
  t2 = t2 + 1;		// Increment the address 

i_block_n_read_shift24400:
  if (_trace) printf("i_block_n_read_shift24400:\n");
		/* Store updated vma in BAR */
  *(u32 *)arg2 = t2;
  if (t5 == 0) 		// J. if we don't have to clear CDR codes. 
    goto i_block_n_read_shift24401;
  t8 = t8 & 63;

i_block_n_read_shift24401:
  if (_trace) printf("i_block_n_read_shift24401:\n");
  t1 = zero + 21504;   
  t3 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t4 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t2 = t1 >> 10;   
  t2 = t2 & 3;		// Extract the byte background 
  t5 = (t2 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment24421:
  if (_trace) printf("force_alignment24421:\n");
  if (t5 == 0) 
    goto basic_dispatch24417;
  /* Here if argument ALUByteBackgroundOp1 */
  t2 = t1;

basic_dispatch24416:
  if (_trace) printf("basic_dispatch24416:\n");
  t6 = t1 >> 12;   
  t6 = t6 & 1;		// Extractthe byte rotate latch 
  t7 = t7 << (t3 & 63);   
  t5 = (u32)(t7 >> ((4&7)*8));   
  t7 = (u32)t7;   
  t7 = t7 | t5;		// OP2 rotated 
  if (t6 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte24415;
  *(u64 *)&processor->rotatelatch = t7;   

alu_function_byte24415:
  if (_trace) printf("alu_function_byte24415:\n");
  t6 = zero + -2;   
  t6 = t6 << (t4 & 63);   
  t6 = ~t6;   		// Compute mask 
  /* Get byte function */
  t5 = t1 >> 13;   
  t5 = t5 & 1;
  t4 = (t5 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment24426:
  if (_trace) printf("force_alignment24426:\n");
  if (t4 == 0) 
    goto basic_dispatch24423;
  /* Here if argument ALUByteFunctionDpb */
  t6 = t6 << (t3 & 63);   		// Position mask 

basic_dispatch24422:
  if (_trace) printf("basic_dispatch24422:\n");
  t7 = t7 & t6;		// rotated&mask 
  t2 = t2 & ~t6;		// background&~mask 
  t7 = t7 | t2;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);   
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);   
  *(u32 *)(iSP + 8) = t7;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
  iSP = iSP + 8;
  goto cachevalid;   

i_block_n_read_shift24402:
  if (_trace) printf("i_block_n_read_shift24402:\n");
  arg5 = t2;
  arg2 = 23;
  goto illegaloperand;

basic_dispatch24423:
  if (_trace) printf("basic_dispatch24423:\n");
  t4 = (t5 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment24427:
  if (_trace) printf("force_alignment24427:\n");
  if (t4 != 0)   
    goto basic_dispatch24422;
  goto basic_dispatch24422;   

basic_dispatch24417:
  if (_trace) printf("basic_dispatch24417:\n");
  t5 = (t2 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment24428:
  if (_trace) printf("force_alignment24428:\n");
  if (t5 == 0) 
    goto basic_dispatch24418;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t2 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch24416;   

basic_dispatch24418:
  if (_trace) printf("basic_dispatch24418:\n");
  t5 = (t2 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment24429:
  if (_trace) printf("force_alignment24429:\n");
  if (t5 == 0) 
    goto basic_dispatch24416;
  /* Here if argument ALUByteBackgroundZero */
  t2 = zero;
  goto basic_dispatch24416;   

vma_memory_read24405:
  if (_trace) printf("vma_memory_read24405:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t10;  		// reconstruct SCA 
  t7 = *(s32 *)t9;   
  t8 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read24404;   

vma_memory_read24407:
  if (_trace) printf("vma_memory_read24407:\n");

vma_memory_read24406:
  if (_trace) printf("vma_memory_read24406:\n");
  t12 = (t1 * 4);   		// Cycle-number -> table offset 
  t12 = (t12 * 4) + ivory;   
  t12 = *(u64 *)(t12 + PROCESSORSTATE_DATAREAD);   
  /* TagType. */
  t11 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + t12;   		// Adjust for a longword load 
  t12 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read24412:
  if (_trace) printf("vma_memory_read24412:\n");
  t10 = t12 & MemoryActionIndirect;
  if (t10 == 0) 
    goto vma_memory_read24411;
  t2 = (u32)t7;   		// Do the indirect thing 
  goto vma_memory_read24403;   

vma_memory_read24411:
  if (_trace) printf("vma_memory_read24411:\n");
  t11 = t12 & MemoryActionTransform;
  if (t11 == 0) 
    goto vma_memory_read24410;
  t8 = t8 & ~63L;
  t8 = t8 | Type_ExternalValueCellPointer;
  goto vma_memory_read24414;   

vma_memory_read24410:

vma_memory_read24409:
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
    goto i_block_n_read_alu24430;
  t1 = (u32)t1;   
  /* Memory Read Internal */

vma_memory_read24432:
  t11 = t1 + ivory;
  t3 = (t11 * 4);   
  t2 = LDQ_U(t11);   
  t9 = t1 - arg5;   		// Stack cache offset 
  t12 = *(u64 *)&(processor->dataread_mask);   
  t10 = ((u64)t9 < (u64)arg6) ? 1 : 0;   		// In range? 
  t3 = *(s32 *)t3;   
  t2 = (u8)(t2 >> ((t11&7)*8));   
  if (t10 != 0)   
    goto vma_memory_read24434;

vma_memory_read24433:
  t11 = zero + 240;   
  t12 = t12 >> (t2 & 63);   
  t11 = t11 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t12 & 1)   
    goto vma_memory_read24436;

vma_memory_read24443:
  t9 = t2 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_alu24431;
  t1 = t1 + 1;		// Increment the address 
		/* Store updated vma in BAR */
  *(u32 *)arg2 = t1;
  t6 = *(u64 *)&(processor->aluop);   
  *(u64 *)&processor->aluoverflow = zero;   
  t7 = *(u64 *)&(processor->aluandrotatecontrol);   
  t1 = (t6 == ALUFunction_Boolean) ? 1 : 0;   

force_alignment24504:
  if (_trace) printf("force_alignment24504:\n");
  if (t1 == 0) 
    goto basic_dispatch24445;
  /* Here if argument ALUFunctionBoolean */
  t8 = t7 >> 10;   
  t8 = t8 & 15;		// Extract the ALU boolean function 
  t1 = (t8 == Boole_Clear) ? 1 : 0;   

force_alignment24464:
  if (_trace) printf("force_alignment24464:\n");
  if (t1 != 0)   
    goto basic_dispatch24446;

basic_dispatch24447:
  if (_trace) printf("basic_dispatch24447:\n");
  t1 = (t8 == Boole_And) ? 1 : 0;   

force_alignment24465:
  if (_trace) printf("force_alignment24465:\n");
  if (t1 == 0) 
    goto basic_dispatch24448;
  /* Here if argument BooleAnd */
  t8 = t3 & t5;
  goto basic_dispatch24446;   

basic_dispatch24448:
  if (_trace) printf("basic_dispatch24448:\n");
  t1 = (t8 == Boole_AndC1) ? 1 : 0;   

force_alignment24466:
  if (_trace) printf("force_alignment24466:\n");
  if (t1 == 0) 
    goto basic_dispatch24449;
  /* Here if argument BooleAndC1 */
  t8 = t5 & ~t3;
  goto basic_dispatch24446;   

basic_dispatch24449:
  if (_trace) printf("basic_dispatch24449:\n");
  t1 = (t8 == Boole_2) ? 1 : 0;   

force_alignment24467:
  if (_trace) printf("force_alignment24467:\n");
  if (t1 == 0) 
    goto basic_dispatch24450;
  /* Here if argument Boole2 */
  t8 = t5;
  goto basic_dispatch24446;   

basic_dispatch24450:
  if (_trace) printf("basic_dispatch24450:\n");
  t1 = (t8 == Boole_AndC2) ? 1 : 0;   

force_alignment24468:
  if (_trace) printf("force_alignment24468:\n");
  if (t1 == 0) 
    goto basic_dispatch24451;
  /* Here if argument BooleAndC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch24446;   

basic_dispatch24451:
  if (_trace) printf("basic_dispatch24451:\n");
  t1 = (t8 == Boole_1) ? 1 : 0;   

force_alignment24469:
  if (_trace) printf("force_alignment24469:\n");
  if (t1 == 0) 
    goto basic_dispatch24452;
  /* Here if argument Boole1 */
  t8 = t3;
  goto basic_dispatch24446;   

basic_dispatch24452:
  if (_trace) printf("basic_dispatch24452:\n");
  t1 = (t8 == Boole_Xor) ? 1 : 0;   

force_alignment24470:
  if (_trace) printf("force_alignment24470:\n");
  if (t1 == 0) 
    goto basic_dispatch24453;
  /* Here if argument BooleXor */
  t8 = t3 ^ t5;   
  goto basic_dispatch24446;   

basic_dispatch24453:
  if (_trace) printf("basic_dispatch24453:\n");
  t1 = (t8 == Boole_Ior) ? 1 : 0;   

force_alignment24471:
  if (_trace) printf("force_alignment24471:\n");
  if (t1 == 0) 
    goto basic_dispatch24454;
  /* Here if argument BooleIor */
  t8 = t3 | t5;
  goto basic_dispatch24446;   

basic_dispatch24454:
  if (_trace) printf("basic_dispatch24454:\n");
  t1 = (t8 == Boole_Nor) ? 1 : 0;   

force_alignment24472:
  if (_trace) printf("force_alignment24472:\n");
  if (t1 == 0) 
    goto basic_dispatch24455;
  /* Here if argument BooleNor */
  t8 = t3 | t5;
  t8 = ~t8;   
  goto basic_dispatch24446;   

basic_dispatch24455:
  if (_trace) printf("basic_dispatch24455:\n");
  t1 = (t8 == Boole_Equiv) ? 1 : 0;   

force_alignment24473:
  if (_trace) printf("force_alignment24473:\n");
  if (t1 == 0) 
    goto basic_dispatch24456;
  /* Here if argument BooleEquiv */
  t8 = t3 ^ t5;   
  t8 = ~t8;   
  goto basic_dispatch24446;   

basic_dispatch24456:
  if (_trace) printf("basic_dispatch24456:\n");
  t1 = (t8 == Boole_C1) ? 1 : 0;   

force_alignment24474:
  if (_trace) printf("force_alignment24474:\n");
  if (t1 == 0) 
    goto basic_dispatch24457;
  /* Here if argument BooleC1 */
  t8 = ~t3;   
  goto basic_dispatch24446;   

basic_dispatch24457:
  if (_trace) printf("basic_dispatch24457:\n");
  t1 = (t8 == Boole_OrC1) ? 1 : 0;   

force_alignment24475:
  if (_trace) printf("force_alignment24475:\n");
  if (t1 == 0) 
    goto basic_dispatch24458;
  /* Here if argument BooleOrC1 */
  t8 = t5 | ~(t3);   
  goto basic_dispatch24446;   

basic_dispatch24458:
  if (_trace) printf("basic_dispatch24458:\n");
  t1 = (t8 == Boole_C2) ? 1 : 0;   

force_alignment24476:
  if (_trace) printf("force_alignment24476:\n");
  if (t1 == 0) 
    goto basic_dispatch24459;
  /* Here if argument BooleC2 */
  t8 = ~t5;   
  goto basic_dispatch24446;   

basic_dispatch24459:
  if (_trace) printf("basic_dispatch24459:\n");
  t1 = (t8 == Boole_OrC2) ? 1 : 0;   

force_alignment24477:
  if (_trace) printf("force_alignment24477:\n");
  if (t1 == 0) 
    goto basic_dispatch24460;
  /* Here if argument BooleOrC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch24446;   

basic_dispatch24460:
  if (_trace) printf("basic_dispatch24460:\n");
  t1 = (t8 == Boole_Nand) ? 1 : 0;   

force_alignment24478:
  if (_trace) printf("force_alignment24478:\n");
  if (t1 == 0) 
    goto basic_dispatch24461;
  /* Here if argument BooleNand */
  t8 = t3 & t5;
  goto basic_dispatch24446;   

basic_dispatch24461:
  if (_trace) printf("basic_dispatch24461:\n");
  t1 = (t8 == Boole_Set) ? 1 : 0;   

force_alignment24479:
  if (_trace) printf("force_alignment24479:\n");
  if (t1 == 0) 
    goto basic_dispatch24446;
  /* Here if argument BooleSet */
  t8 = ~zero;   

basic_dispatch24446:
  if (_trace) printf("basic_dispatch24446:\n");
  *(u32 *)arg1 = t8;
  goto NEXTINSTRUCTION;   

basic_dispatch24445:
  if (_trace) printf("basic_dispatch24445:\n");
  t1 = (t6 == ALUFunction_Byte) ? 1 : 0;   

force_alignment24505:
  if (_trace) printf("force_alignment24505:\n");
  if (t1 == 0) 
    goto basic_dispatch24480;
  /* Here if argument ALUFunctionByte */
  t9 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t10 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t1 = t7 >> 10;   
  t1 = t1 & 3;		// Extract the byte background 
  t11 = (t1 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment24487:
  if (_trace) printf("force_alignment24487:\n");
  if (t11 == 0) 
    goto basic_dispatch24483;
  /* Here if argument ALUByteBackgroundOp1 */
  t1 = t3;

basic_dispatch24482:
  if (_trace) printf("basic_dispatch24482:\n");
  t12 = t7 >> 12;   
  t12 = t12 & 1;		// Extractthe byte rotate latch 
  t8 = t5 << (t9 & 63);   
  t11 = (u32)(t8 >> ((4&7)*8));   
  t8 = (u32)t8;   
  t8 = t8 | t11;		// OP2 rotated 
  if (t12 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte24481;
  *(u64 *)&processor->rotatelatch = t8;   

alu_function_byte24481:
  if (_trace) printf("alu_function_byte24481:\n");
  t12 = zero + -2;   
  t12 = t12 << (t10 & 63);   
  t12 = ~t12;   		// Compute mask 
  /* Get byte function */
  t11 = t7 >> 13;   
  t11 = t11 & 1;
  t10 = (t11 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment24492:
  if (_trace) printf("force_alignment24492:\n");
  if (t10 == 0) 
    goto basic_dispatch24489;
  /* Here if argument ALUByteFunctionDpb */
  t12 = t12 << (t9 & 63);   		// Position mask 

basic_dispatch24488:
  if (_trace) printf("basic_dispatch24488:\n");
  t8 = t8 & t12;		// rotated&mask 
  t1 = t1 & ~t12;		// background&~mask 
  t8 = t8 | t1;
  *(u32 *)arg1 = t8;
  goto NEXTINSTRUCTION;   

basic_dispatch24480:
  if (_trace) printf("basic_dispatch24480:\n");
  t1 = (t6 == ALUFunction_Adder) ? 1 : 0;   

force_alignment24506:
  if (_trace) printf("force_alignment24506:\n");
  if (t1 == 0) 
    goto basic_dispatch24493;
  /* Here if argument ALUFunctionAdder */
  t10 = t7 >> 11;   
  t10 = t10 & 3;		// Extract the op2 
  t9 = t7 >> 10;   
  t9 = t9 & 1;		// Extract the adder carry in 
  t11 = (t10 == ALUAdderOp2_Op2) ? 1 : 0;   

force_alignment24501:
  if (_trace) printf("force_alignment24501:\n");
  if (t11 == 0) 
    goto basic_dispatch24496;
  /* Here if argument ALUAdderOp2Op2 */
  t1 = t5;

basic_dispatch24495:
  if (_trace) printf("basic_dispatch24495:\n");
  t8 = t3 + t1;
  t8 = t8 + t9;
  t10 = t8 >> 31;   		// Sign bit 
  t11 = t8 >> 32;   		// Next bit 
  t10 = t10 ^ t11;   		// Low bit is now overflow indicator 
  t11 = t7 >> 24;   		// Get the load-carry-in bit 
  *(u64 *)&processor->aluoverflow = t10;   
  if ((t11 & 1) == 0)   
    goto alu_function_adder24494;
  t10 = (u32)(t8 >> ((4&7)*8));   		// Get the carry 
  t11 = zero + 1024;   
  t7 = t7 & ~t11;
  t11 = t10 & 1;
  t11 = t11 << 10;   
  t7 = t7 | t11;		// Set the adder carry in 
  *(u64 *)&processor->aluandrotatecontrol = t7;   

alu_function_adder24494:
  if (_trace) printf("alu_function_adder24494:\n");
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->aluborrow = t10;   
  t3 = (s32)t3;
  t5 = (s32)t5;
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->alulessthan = t10;   
  *(u32 *)arg1 = t8;
  goto NEXTINSTRUCTION;   

basic_dispatch24493:
  if (_trace) printf("basic_dispatch24493:\n");
  t1 = (t6 == ALUFunction_MultiplyDivide) ? 1 : 0;   

force_alignment24507:
  if (_trace) printf("force_alignment24507:\n");
  if (t1 == 0) 
    goto basic_dispatch24444;
  /* Here if argument ALUFunctionMultiplyDivide */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;
  *(u32 *)arg1 = t8;
  goto NEXTINSTRUCTION;   

basic_dispatch24444:
  if (_trace) printf("basic_dispatch24444:\n");

i_block_n_read_alu24430:
  if (_trace) printf("i_block_n_read_alu24430:\n");
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  t9 = arg1 - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  t1 = t9 + arg5;		// reconstruct VMA 
  arg5 = t1;
  arg2 = 23;
  goto illegaloperand;

i_block_n_read_alu24431:
  if (_trace) printf("i_block_n_read_alu24431:\n");
  arg5 = t1;
  arg2 = 23;
  goto illegaloperand;

basic_dispatch24496:
  if (_trace) printf("basic_dispatch24496:\n");
  t11 = (t10 == ALUAdderOp2_Zero) ? 1 : 0;   

force_alignment24508:
  if (_trace) printf("force_alignment24508:\n");
  if (t11 == 0) 
    goto basic_dispatch24497;
  /* Here if argument ALUAdderOp2Zero */
  t1 = zero;
  goto basic_dispatch24495;   

basic_dispatch24497:
  if (_trace) printf("basic_dispatch24497:\n");
  t11 = (t10 == ALUAdderOp2_Invert) ? 1 : 0;   

force_alignment24509:
  if (_trace) printf("force_alignment24509:\n");
  if (t11 == 0) 
    goto basic_dispatch24498;
  /* Here if argument ALUAdderOp2Invert */
  t1 = (s32)t5;
  t1 = zero - t1;   
  t1 = (u32)t1;   
  goto basic_dispatch24495;   

basic_dispatch24498:
  if (_trace) printf("basic_dispatch24498:\n");
  t11 = (t10 == ALUAdderOp2_MinusOne) ? 1 : 0;   

force_alignment24510:
  if (_trace) printf("force_alignment24510:\n");
  if (t11 == 0) 
    goto basic_dispatch24495;
  /* Here if argument ALUAdderOp2MinusOne */
  t1 = ~zero;   
  t1 = (u32)t1;   
  goto basic_dispatch24495;   

basic_dispatch24489:
  if (_trace) printf("basic_dispatch24489:\n");
  t10 = (t11 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment24511:
  if (_trace) printf("force_alignment24511:\n");
  if (t10 != 0)   
    goto basic_dispatch24488;
  goto basic_dispatch24488;   

basic_dispatch24483:
  if (_trace) printf("basic_dispatch24483:\n");
  t11 = (t1 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment24512:
  if (_trace) printf("force_alignment24512:\n");
  if (t11 == 0) 
    goto basic_dispatch24484;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t1 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch24482;   

basic_dispatch24484:
  if (_trace) printf("basic_dispatch24484:\n");
  t11 = (t1 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment24513:
  if (_trace) printf("force_alignment24513:\n");
  if (t11 == 0) 
    goto basic_dispatch24482;
  /* Here if argument ALUByteBackgroundZero */
  t1 = zero;
  goto basic_dispatch24482;   

vma_memory_read24434:
  if (_trace) printf("vma_memory_read24434:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t10;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read24433;   

vma_memory_read24436:
  if (_trace) printf("vma_memory_read24436:\n");
  if ((t11 & 1) == 0)   
    goto vma_memory_read24435;
  t1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24432;   

vma_memory_read24435:
  if (_trace) printf("vma_memory_read24435:\n");
  t12 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t11 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + t12;   		// Adjust for a longword load 
  t12 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read24440:
  if (_trace) printf("vma_memory_read24440:\n");
  t11 = t12 & MemoryActionTransform;
  if (t11 == 0) 
    goto vma_memory_read24439;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read24443;   

vma_memory_read24439:

vma_memory_read24438:
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

vma_memory_read24520:
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
    goto vma_memory_read24522;

vma_memory_read24521:
  t12 = t12 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t12 & 1)   
    goto vma_memory_read24524;

vma_memory_read24531:
  t1 = arg1 & 32;		// =fixnum onlyp 
  if (t1 == 0) 		// J. if we don't have to test for fixnump. 
    goto i_block_n_read_test24514;
  t9 = t2 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_test24517;
  t9 = t4 - Type_Fixnum;   
  t9 = t9 & 63;		// Strip CDR code 
  if (t9 != 0)   
    goto i_block_n_read_test24518;

i_block_n_read_test24514:
  if (_trace) printf("i_block_n_read_test24514:\n");
  t1 = arg1 & 16;		// =cdr-code-nextp 
  if (t1 == 0) 		// J. if we don't have to clear CDR codes. 
    goto i_block_n_read_test24516;
  /* TagType. */
  t2 = t2 & 63;

i_block_n_read_test24516:
  if (_trace) printf("i_block_n_read_test24516:\n");
  t6 = *(u64 *)&(processor->aluop);   
  *(u64 *)&processor->aluoverflow = zero;   
  t7 = *(u64 *)&(processor->aluandrotatecontrol);   
  t1 = (t6 == ALUFunction_Boolean) ? 1 : 0;   

force_alignment24592:
  if (_trace) printf("force_alignment24592:\n");
  if (t1 == 0) 
    goto basic_dispatch24533;
  /* Here if argument ALUFunctionBoolean */
  t8 = t7 >> 10;   
  t8 = t8 & 15;		// Extract the ALU boolean function 
  t1 = (t8 == Boole_Clear) ? 1 : 0;   

force_alignment24552:
  if (_trace) printf("force_alignment24552:\n");
  if (t1 != 0)   
    goto basic_dispatch24534;

basic_dispatch24535:
  if (_trace) printf("basic_dispatch24535:\n");
  t1 = (t8 == Boole_And) ? 1 : 0;   

force_alignment24553:
  if (_trace) printf("force_alignment24553:\n");
  if (t1 == 0) 
    goto basic_dispatch24536;
  /* Here if argument BooleAnd */
  t8 = t3 & t5;
  goto basic_dispatch24534;   

basic_dispatch24536:
  if (_trace) printf("basic_dispatch24536:\n");
  t1 = (t8 == Boole_AndC1) ? 1 : 0;   

force_alignment24554:
  if (_trace) printf("force_alignment24554:\n");
  if (t1 == 0) 
    goto basic_dispatch24537;
  /* Here if argument BooleAndC1 */
  t8 = t5 & ~t3;
  goto basic_dispatch24534;   

basic_dispatch24537:
  if (_trace) printf("basic_dispatch24537:\n");
  t1 = (t8 == Boole_2) ? 1 : 0;   

force_alignment24555:
  if (_trace) printf("force_alignment24555:\n");
  if (t1 == 0) 
    goto basic_dispatch24538;
  /* Here if argument Boole2 */
  t8 = t5;
  goto basic_dispatch24534;   

basic_dispatch24538:
  if (_trace) printf("basic_dispatch24538:\n");
  t1 = (t8 == Boole_AndC2) ? 1 : 0;   

force_alignment24556:
  if (_trace) printf("force_alignment24556:\n");
  if (t1 == 0) 
    goto basic_dispatch24539;
  /* Here if argument BooleAndC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch24534;   

basic_dispatch24539:
  if (_trace) printf("basic_dispatch24539:\n");
  t1 = (t8 == Boole_1) ? 1 : 0;   

force_alignment24557:
  if (_trace) printf("force_alignment24557:\n");
  if (t1 == 0) 
    goto basic_dispatch24540;
  /* Here if argument Boole1 */
  t8 = t3;
  goto basic_dispatch24534;   

basic_dispatch24540:
  if (_trace) printf("basic_dispatch24540:\n");
  t1 = (t8 == Boole_Xor) ? 1 : 0;   

force_alignment24558:
  if (_trace) printf("force_alignment24558:\n");
  if (t1 == 0) 
    goto basic_dispatch24541;
  /* Here if argument BooleXor */
  t8 = t3 ^ t5;   
  goto basic_dispatch24534;   

basic_dispatch24541:
  if (_trace) printf("basic_dispatch24541:\n");
  t1 = (t8 == Boole_Ior) ? 1 : 0;   

force_alignment24559:
  if (_trace) printf("force_alignment24559:\n");
  if (t1 == 0) 
    goto basic_dispatch24542;
  /* Here if argument BooleIor */
  t8 = t3 | t5;
  goto basic_dispatch24534;   

basic_dispatch24542:
  if (_trace) printf("basic_dispatch24542:\n");
  t1 = (t8 == Boole_Nor) ? 1 : 0;   

force_alignment24560:
  if (_trace) printf("force_alignment24560:\n");
  if (t1 == 0) 
    goto basic_dispatch24543;
  /* Here if argument BooleNor */
  t8 = t3 | t5;
  t8 = ~t8;   
  goto basic_dispatch24534;   

basic_dispatch24543:
  if (_trace) printf("basic_dispatch24543:\n");
  t1 = (t8 == Boole_Equiv) ? 1 : 0;   

force_alignment24561:
  if (_trace) printf("force_alignment24561:\n");
  if (t1 == 0) 
    goto basic_dispatch24544;
  /* Here if argument BooleEquiv */
  t8 = t3 ^ t5;   
  t8 = ~t8;   
  goto basic_dispatch24534;   

basic_dispatch24544:
  if (_trace) printf("basic_dispatch24544:\n");
  t1 = (t8 == Boole_C1) ? 1 : 0;   

force_alignment24562:
  if (_trace) printf("force_alignment24562:\n");
  if (t1 == 0) 
    goto basic_dispatch24545;
  /* Here if argument BooleC1 */
  t8 = ~t3;   
  goto basic_dispatch24534;   

basic_dispatch24545:
  if (_trace) printf("basic_dispatch24545:\n");
  t1 = (t8 == Boole_OrC1) ? 1 : 0;   

force_alignment24563:
  if (_trace) printf("force_alignment24563:\n");
  if (t1 == 0) 
    goto basic_dispatch24546;
  /* Here if argument BooleOrC1 */
  t8 = t5 | ~(t3);   
  goto basic_dispatch24534;   

basic_dispatch24546:
  if (_trace) printf("basic_dispatch24546:\n");
  t1 = (t8 == Boole_C2) ? 1 : 0;   

force_alignment24564:
  if (_trace) printf("force_alignment24564:\n");
  if (t1 == 0) 
    goto basic_dispatch24547;
  /* Here if argument BooleC2 */
  t8 = ~t5;   
  goto basic_dispatch24534;   

basic_dispatch24547:
  if (_trace) printf("basic_dispatch24547:\n");
  t1 = (t8 == Boole_OrC2) ? 1 : 0;   

force_alignment24565:
  if (_trace) printf("force_alignment24565:\n");
  if (t1 == 0) 
    goto basic_dispatch24548;
  /* Here if argument BooleOrC2 */
  t8 = t3 & ~t5;
  goto basic_dispatch24534;   

basic_dispatch24548:
  if (_trace) printf("basic_dispatch24548:\n");
  t1 = (t8 == Boole_Nand) ? 1 : 0;   

force_alignment24566:
  if (_trace) printf("force_alignment24566:\n");
  if (t1 == 0) 
    goto basic_dispatch24549;
  /* Here if argument BooleNand */
  t8 = t3 & t5;
  goto basic_dispatch24534;   

basic_dispatch24549:
  if (_trace) printf("basic_dispatch24549:\n");
  t1 = (t8 == Boole_Set) ? 1 : 0;   

force_alignment24567:
  if (_trace) printf("force_alignment24567:\n");
  if (t1 == 0) 
    goto basic_dispatch24534;
  /* Here if argument BooleSet */
  t8 = ~zero;   

basic_dispatch24534:
  if (_trace) printf("basic_dispatch24534:\n");

basic_dispatch24532:
  if (_trace) printf("basic_dispatch24532:\n");
  t1 = t7 >> 16;   
  t1 = t1 & 31;		// Extract ALU condition 
  t10 = *(u64 *)&(processor->aluoverflow);   
  t11 = *(u64 *)&(processor->aluborrow);   
  t12 = *(u64 *)&(processor->alulessthan);   
  t9 = (t1 == ALUCondition_SignedLessThanOrEqual) ? 1 : 0;   

force_alignment24624:
  if (_trace) printf("force_alignment24624:\n");
  if (t9 == 0) 
    goto basic_dispatch24597;
  /* Here if argument ALUConditionSignedLessThanOrEqual */
  if (t12 != 0)   
    goto alu_compute_condition24593;
  if (t8 == 0) 
    goto alu_compute_condition24593;

basic_dispatch24596:
  if (_trace) printf("basic_dispatch24596:\n");

alu_compute_condition24594:
  if (_trace) printf("alu_compute_condition24594:\n");
  t1 = zero;
  goto alu_compute_condition24595;   

alu_compute_condition24593:
  if (_trace) printf("alu_compute_condition24593:\n");
  t1 = 1;

alu_compute_condition24595:
  if (_trace) printf("alu_compute_condition24595:\n");
  t9 = t7 >> 21;   
  t9 = t9 & 1;		// Extract the condition sense 
  t1 = t1 ^ t9;   
  if (t1 != 0)   
    goto i_block_n_read_test24519;
  t1 = arg1 & 4;		// =no-incrementp 
  if (t1 != 0)   		// J. if we don't have to increment the address. 
    goto i_block_n_read_test24515;
  arg3 = arg3 + 1;		// Increment the address 

i_block_n_read_test24515:
  if (_trace) printf("i_block_n_read_test24515:\n");
		/* Store updated vma in BAR */
  *(u32 *)arg2 = arg3;
  goto NEXTINSTRUCTION;   

i_block_n_read_test24519:
  if (_trace) printf("i_block_n_read_test24519:\n");
  t10 = *(s32 *)(iSP + -8);   
  t9 = *(s32 *)(iSP + -4);   
  t10 = (u32)t10;   
  t10 = t10 << 1;   
  iPC = t9 & 1;
  iPC = iPC + t10;
  goto interpretinstructionforjump;   

i_block_n_read_test24518:
  if (_trace) printf("i_block_n_read_test24518:\n");
  /* Convert stack cache address to VMA */
  t9 = *(u64 *)&(processor->stackcachedata);   
  arg3 = *(u64 *)&(processor->stackcachebasevma);   
  t9 = iSP - t9;   		// stack cache base relative offset 
  t9 = t9 >> 3;   		// convert byte address to word address 
  arg3 = t9 + arg3;		// reconstruct VMA 
  arg5 = arg3;
  arg2 = 23;
  goto illegaloperand;

i_block_n_read_test24517:
  if (_trace) printf("i_block_n_read_test24517:\n");
  arg5 = arg3;
  arg2 = 23;
  goto illegaloperand;

basic_dispatch24597:
  if (_trace) printf("basic_dispatch24597:\n");
  t9 = (t1 == ALUCondition_SignedLessThan) ? 1 : 0;   

force_alignment24625:
  if (_trace) printf("force_alignment24625:\n");
  if (t9 == 0) 
    goto basic_dispatch24598;
  /* Here if argument ALUConditionSignedLessThan */
  if (t12 != 0)   
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24598:
  if (_trace) printf("basic_dispatch24598:\n");
  t9 = (t1 == ALUCondition_Negative) ? 1 : 0;   

force_alignment24626:
  if (_trace) printf("force_alignment24626:\n");
  if (t9 == 0) 
    goto basic_dispatch24599;
  /* Here if argument ALUConditionNegative */
  if ((s64)t8 < 0)   
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24599:
  if (_trace) printf("basic_dispatch24599:\n");
  t9 = (t1 == ALUCondition_SignedOverflow) ? 1 : 0;   

force_alignment24627:
  if (_trace) printf("force_alignment24627:\n");
  if (t9 == 0) 
    goto basic_dispatch24600;
  /* Here if argument ALUConditionSignedOverflow */
  if (t10 != 0)   
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24600:
  if (_trace) printf("basic_dispatch24600:\n");
  t9 = (t1 == ALUCondition_UnsignedLessThanOrEqual) ? 1 : 0;   

force_alignment24628:
  if (_trace) printf("force_alignment24628:\n");
  if (t9 == 0) 
    goto basic_dispatch24601;
  /* Here if argument ALUConditionUnsignedLessThanOrEqual */
  if (t11 != 0)   
    goto alu_compute_condition24593;
  if (t8 == 0) 
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24601:
  if (_trace) printf("basic_dispatch24601:\n");
  t9 = (t1 == ALUCondition_UnsignedLessThan) ? 1 : 0;   

force_alignment24629:
  if (_trace) printf("force_alignment24629:\n");
  if (t9 == 0) 
    goto basic_dispatch24602;
  /* Here if argument ALUConditionUnsignedLessThan */
  if (t11 != 0)   
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24602:
  if (_trace) printf("basic_dispatch24602:\n");
  t9 = (t1 == ALUCondition_Zero) ? 1 : 0;   

force_alignment24630:
  if (_trace) printf("force_alignment24630:\n");
  if (t9 == 0) 
    goto basic_dispatch24603;
  /* Here if argument ALUConditionZero */
  if (t8 == 0) 
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24603:
  if (_trace) printf("basic_dispatch24603:\n");
  t9 = (t1 == ALUCondition_High25Zero) ? 1 : 0;   

force_alignment24631:
  if (_trace) printf("force_alignment24631:\n");
  if (t9 == 0) 
    goto basic_dispatch24604;
  /* Here if argument ALUConditionHigh25Zero */
  t1 = t8 >> 7;   
  if (t1 == 0) 
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24604:
  if (_trace) printf("basic_dispatch24604:\n");
  t9 = (t1 == ALUCondition_Eq) ? 1 : 0;   

force_alignment24632:
  if (_trace) printf("force_alignment24632:\n");
  if (t9 == 0) 
    goto basic_dispatch24605;
  /* Here if argument ALUConditionEq */
  if (t8 != 0)   
    goto alu_compute_condition24594;
  t9 = t2 ^ t4;   
  /* TagType. */
  t9 = t9 & 63;
  if (t9 == 0) 
    goto alu_compute_condition24593;
  goto basic_dispatch24596;   

basic_dispatch24605:
  if (_trace) printf("basic_dispatch24605:\n");
  t9 = (t1 == ALUCondition_Op1Ephemeralp) ? 1 : 0;   

force_alignment24633:
  if (_trace) printf("force_alignment24633:\n");
  if (t9 == 0) 
    goto basic_dispatch24606;
  /* Here if argument ALUConditionOp1Ephemeralp */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24606:
  if (_trace) printf("basic_dispatch24606:\n");
  t9 = (t1 == ALUCondition_ResultTypeNil) ? 1 : 0;   

force_alignment24634:
  if (_trace) printf("force_alignment24634:\n");
  if (t9 == 0) 
    goto basic_dispatch24607;
  /* Here if argument ALUConditionResultTypeNil */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24607:
  if (_trace) printf("basic_dispatch24607:\n");
  t9 = (t1 == ALUCondition_Op2Fixnum) ? 1 : 0;   

force_alignment24635:
  if (_trace) printf("force_alignment24635:\n");
  if (t9 == 0) 
    goto basic_dispatch24608;
  /* Here if argument ALUConditionOp2Fixnum */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24608:
  if (_trace) printf("basic_dispatch24608:\n");
  t9 = (t1 == ALUCondition_False) ? 1 : 0;   

force_alignment24636:
  if (_trace) printf("force_alignment24636:\n");
  if (t9 == 0) 
    goto basic_dispatch24609;
  /* Here if argument ALUConditionFalse */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24609:
  if (_trace) printf("basic_dispatch24609:\n");
  t9 = (t1 == ALUCondition_ResultCdrLow) ? 1 : 0;   

force_alignment24637:
  if (_trace) printf("force_alignment24637:\n");
  if (t9 == 0) 
    goto basic_dispatch24610;
  /* Here if argument ALUConditionResultCdrLow */
  /* TagCdr. */
  t9 = t2 >> 6;   
  t1 = t9 & 1;
  goto alu_compute_condition24595;   

basic_dispatch24610:
  if (_trace) printf("basic_dispatch24610:\n");
  t9 = (t1 == ALUCondition_CleanupBitsSet) ? 1 : 0;   

force_alignment24638:
  if (_trace) printf("force_alignment24638:\n");
  if (t9 == 0) 
    goto basic_dispatch24611;
  /* Here if argument ALUConditionCleanupBitsSet */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24611:
  if (_trace) printf("basic_dispatch24611:\n");
  t9 = (t1 == ALUCondition_AddressInStackCache) ? 1 : 0;   

force_alignment24639:
  if (_trace) printf("force_alignment24639:\n");
  if (t9 == 0) 
    goto basic_dispatch24612;
  /* Here if argument ALUConditionAddressInStackCache */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24612:
  if (_trace) printf("basic_dispatch24612:\n");
  t9 = (t1 == ALUCondition_ExtraStackMode) ? 1 : 0;   

force_alignment24640:
  if (_trace) printf("force_alignment24640:\n");
  if (t9 == 0) 
    goto basic_dispatch24613;
  /* Here if argument ALUConditionExtraStackMode */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24613:
  if (_trace) printf("basic_dispatch24613:\n");
  t9 = (t1 == ALUCondition_FepMode) ? 1 : 0;   

force_alignment24641:
  if (_trace) printf("force_alignment24641:\n");
  if (t9 == 0) 
    goto basic_dispatch24614;
  /* Here if argument ALUConditionFepMode */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24614:
  if (_trace) printf("basic_dispatch24614:\n");
  t9 = (t1 == ALUCondition_FpCoprocessorPresent) ? 1 : 0;   

force_alignment24642:
  if (_trace) printf("force_alignment24642:\n");
  if (t9 == 0) 
    goto basic_dispatch24615;
  /* Here if argument ALUConditionFpCoprocessorPresent */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24615:
  if (_trace) printf("basic_dispatch24615:\n");
  t9 = (t1 == ALUCondition_Op1Oldspacep) ? 1 : 0;   

force_alignment24643:
  if (_trace) printf("force_alignment24643:\n");
  if (t9 == 0) 
    goto basic_dispatch24616;
  /* Here if argument ALUConditionOp1Oldspacep */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24616:
  if (_trace) printf("basic_dispatch24616:\n");
  t9 = (t1 == ALUCondition_PendingSequenceBreakEnabled) ? 1 : 0;   

force_alignment24644:
  if (_trace) printf("force_alignment24644:\n");
  if (t9 == 0) 
    goto basic_dispatch24617;
  /* Here if argument ALUConditionPendingSequenceBreakEnabled */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24617:
  if (_trace) printf("basic_dispatch24617:\n");
  t9 = (t1 == ALUCondition_Op1TypeAcceptable) ? 1 : 0;   

force_alignment24645:
  if (_trace) printf("force_alignment24645:\n");
  if (t9 == 0) 
    goto basic_dispatch24618;
  /* Here if argument ALUConditionOp1TypeAcceptable */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24618:
  if (_trace) printf("basic_dispatch24618:\n");
  t9 = (t1 == ALUCondition_Op1TypeCondition) ? 1 : 0;   

force_alignment24646:
  if (_trace) printf("force_alignment24646:\n");
  if (t9 == 0) 
    goto basic_dispatch24619;
  /* Here if argument ALUConditionOp1TypeCondition */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24619:
  if (_trace) printf("basic_dispatch24619:\n");
  t9 = (t1 == ALUCondition_StackCacheOverflow) ? 1 : 0;   

force_alignment24647:
  if (_trace) printf("force_alignment24647:\n");
  if (t9 == 0) 
    goto basic_dispatch24620;
  /* Here if argument ALUConditionStackCacheOverflow */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24620:
  if (_trace) printf("basic_dispatch24620:\n");
  t9 = (t1 == ALUCondition_OrLogicVariable) ? 1 : 0;   

force_alignment24648:
  if (_trace) printf("force_alignment24648:\n");
  if (t9 == 0) 
    goto basic_dispatch24621;
  /* Here if argument ALUConditionOrLogicVariable */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24621:
  if (_trace) printf("basic_dispatch24621:\n");
  /* Here for all other cases */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24533:
  if (_trace) printf("basic_dispatch24533:\n");
  t1 = (t6 == ALUFunction_Byte) ? 1 : 0;   

force_alignment24649:
  if (_trace) printf("force_alignment24649:\n");
  if (t1 == 0) 
    goto basic_dispatch24568;
  /* Here if argument ALUFunctionByte */
  t9 = *(u64 *)&(processor->byterotate);   		// Get rotate 
  t10 = *(u64 *)&(processor->bytesize);   		// Get bytesize 
  /* Get background */
  t1 = t7 >> 10;   
  t1 = t1 & 3;		// Extract the byte background 
  t11 = (t1 == ALUByteBackground_Op1) ? 1 : 0;   

force_alignment24575:
  if (_trace) printf("force_alignment24575:\n");
  if (t11 == 0) 
    goto basic_dispatch24571;
  /* Here if argument ALUByteBackgroundOp1 */
  t1 = t3;

basic_dispatch24570:
  if (_trace) printf("basic_dispatch24570:\n");
  t12 = t7 >> 12;   
  t12 = t12 & 1;		// Extractthe byte rotate latch 
  t8 = t5 << (t9 & 63);   
  t11 = (u32)(t8 >> ((4&7)*8));   
  t8 = (u32)t8;   
  t8 = t8 | t11;		// OP2 rotated 
  if (t12 == 0) 		// Don't update rotate latch if not requested 
    goto alu_function_byte24569;
  *(u64 *)&processor->rotatelatch = t8;   

alu_function_byte24569:
  if (_trace) printf("alu_function_byte24569:\n");
  t12 = zero + -2;   
  t12 = t12 << (t10 & 63);   
  t12 = ~t12;   		// Compute mask 
  /* Get byte function */
  t11 = t7 >> 13;   
  t11 = t11 & 1;
  t10 = (t11 == ALUByteFunction_Dpb) ? 1 : 0;   

force_alignment24580:
  if (_trace) printf("force_alignment24580:\n");
  if (t10 == 0) 
    goto basic_dispatch24577;
  /* Here if argument ALUByteFunctionDpb */
  t12 = t12 << (t9 & 63);   		// Position mask 

basic_dispatch24576:
  if (_trace) printf("basic_dispatch24576:\n");
  t8 = t8 & t12;		// rotated&mask 
  t1 = t1 & ~t12;		// background&~mask 
  t8 = t8 | t1;
  goto basic_dispatch24532;   

basic_dispatch24568:
  if (_trace) printf("basic_dispatch24568:\n");
  t1 = (t6 == ALUFunction_Adder) ? 1 : 0;   

force_alignment24650:
  if (_trace) printf("force_alignment24650:\n");
  if (t1 == 0) 
    goto basic_dispatch24581;
  /* Here if argument ALUFunctionAdder */
  t10 = t7 >> 11;   
  t10 = t10 & 3;		// Extract the op2 
  t9 = t7 >> 10;   
  t9 = t9 & 1;		// Extract the adder carry in 
  t11 = (t10 == ALUAdderOp2_Op2) ? 1 : 0;   

force_alignment24589:
  if (_trace) printf("force_alignment24589:\n");
  if (t11 == 0) 
    goto basic_dispatch24584;
  /* Here if argument ALUAdderOp2Op2 */
  t1 = t5;

basic_dispatch24583:
  if (_trace) printf("basic_dispatch24583:\n");
  t8 = t3 + t1;
  t8 = t8 + t9;
  t10 = t8 >> 31;   		// Sign bit 
  t11 = t8 >> 32;   		// Next bit 
  t10 = t10 ^ t11;   		// Low bit is now overflow indicator 
  t11 = t7 >> 24;   		// Get the load-carry-in bit 
  *(u64 *)&processor->aluoverflow = t10;   
  if ((t11 & 1) == 0)   
    goto alu_function_adder24582;
  t10 = (u32)(t8 >> ((4&7)*8));   		// Get the carry 
  t11 = zero + 1024;   
  t7 = t7 & ~t11;
  t11 = t10 & 1;
  t11 = t11 << 10;   
  t7 = t7 | t11;		// Set the adder carry in 
  *(u64 *)&processor->aluandrotatecontrol = t7;   

alu_function_adder24582:
  if (_trace) printf("alu_function_adder24582:\n");
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->aluborrow = t10;   
  t3 = (s32)t3;
  t5 = (s32)t5;
  t10 = ((s64)t3 < (s64)t1) ? 1 : 0;   
  *(u64 *)&processor->alulessthan = t10;   
  goto basic_dispatch24532;   

basic_dispatch24581:
  if (_trace) printf("basic_dispatch24581:\n");
  t1 = (t6 == ALUFunction_MultiplyDivide) ? 1 : 0;   

force_alignment24651:
  if (_trace) printf("force_alignment24651:\n");
  if (t1 == 0) 
    goto basic_dispatch24532;
  /* Here if argument ALUFunctionMultiplyDivide */
  /* This instruction has not been written yet. */
  arg5 = 0;
  arg2 = 38;
  goto illegaloperand;

basic_dispatch24584:
  if (_trace) printf("basic_dispatch24584:\n");
  t11 = (t10 == ALUAdderOp2_Zero) ? 1 : 0;   

force_alignment24652:
  if (_trace) printf("force_alignment24652:\n");
  if (t11 == 0) 
    goto basic_dispatch24585;
  /* Here if argument ALUAdderOp2Zero */
  t1 = zero;
  goto basic_dispatch24583;   

basic_dispatch24585:
  if (_trace) printf("basic_dispatch24585:\n");
  t11 = (t10 == ALUAdderOp2_Invert) ? 1 : 0;   

force_alignment24653:
  if (_trace) printf("force_alignment24653:\n");
  if (t11 == 0) 
    goto basic_dispatch24586;
  /* Here if argument ALUAdderOp2Invert */
  t1 = (s32)t5;
  t1 = zero - t1;   
  t1 = (u32)t1;   
  goto basic_dispatch24583;   

basic_dispatch24586:
  if (_trace) printf("basic_dispatch24586:\n");
  t11 = (t10 == ALUAdderOp2_MinusOne) ? 1 : 0;   

force_alignment24654:
  if (_trace) printf("force_alignment24654:\n");
  if (t11 == 0) 
    goto basic_dispatch24583;
  /* Here if argument ALUAdderOp2MinusOne */
  t1 = ~zero;   
  t1 = (u32)t1;   
  goto basic_dispatch24583;   

basic_dispatch24577:
  if (_trace) printf("basic_dispatch24577:\n");
  t10 = (t11 == ALUByteFunction_Ldb) ? 1 : 0;   

force_alignment24655:
  if (_trace) printf("force_alignment24655:\n");
  if (t10 != 0)   
    goto basic_dispatch24576;
  goto basic_dispatch24576;   

basic_dispatch24571:
  if (_trace) printf("basic_dispatch24571:\n");
  t11 = (t1 == ALUByteBackground_RotateLatch) ? 1 : 0;   

force_alignment24656:
  if (_trace) printf("force_alignment24656:\n");
  if (t11 == 0) 
    goto basic_dispatch24572;
  /* Here if argument ALUByteBackgroundRotateLatch */
  t1 = *(u64 *)&(processor->rotatelatch);   
  goto basic_dispatch24570;   

basic_dispatch24572:
  if (_trace) printf("basic_dispatch24572:\n");
  t11 = (t1 == ALUByteBackground_Zero) ? 1 : 0;   

force_alignment24657:
  if (_trace) printf("force_alignment24657:\n");
  if (t11 == 0) 
    goto basic_dispatch24570;
  /* Here if argument ALUByteBackgroundZero */
  t1 = zero;
  goto basic_dispatch24570;   

vma_memory_read24522:
  if (_trace) printf("vma_memory_read24522:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t10;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read24521;   

vma_memory_read24524:
  if (_trace) printf("vma_memory_read24524:\n");

vma_memory_read24523:
  if (_trace) printf("vma_memory_read24523:\n");
  t12 = (t1 * 4);   		// Cycle-number -> table offset 
  t12 = (t12 * 4) + ivory;   
  t12 = *(u64 *)(t12 + PROCESSORSTATE_DATAREAD);   
  /* TagType. */
  t11 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg3;   		// stash the VMA for the (likely) trap 
  t11 = (t11 * 4) + t12;   		// Adjust for a longword load 
  t12 = *(s32 *)t11;   		// Get the memory action 

vma_memory_read24529:
  if (_trace) printf("vma_memory_read24529:\n");
  t10 = t12 & MemoryActionIndirect;
  if (t10 == 0) 
    goto vma_memory_read24528;
  arg3 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24520;   

vma_memory_read24528:
  if (_trace) printf("vma_memory_read24528:\n");
  t11 = t12 & MemoryActionTransform;
  if (t11 == 0) 
    goto vma_memory_read24527;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read24531;   

vma_memory_read24527:

vma_memory_read24526:
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

force_alignment24690:
  if (_trace) printf("force_alignment24690:\n");
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
#ifdef MINIMA
  t2 = arg3 >> 32;   
#endif
  arg3 = (u32)arg3;   
  arg4 = (u32)arg4;   
  t1 = arg3 - arg4;   
  if ((s64)t1 >= 0)   		// J. if binding stack overflow 
    goto bindloctovalov;
  t3 = arg3 + 1;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t1 = t2 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto bindloctovaldeep;
#endif
  t9 = *(s32 *)&processor->control;   
  t8 = arg6;
  /* Memory Read Internal */

vma_memory_read24658:
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
    goto vma_memory_read24660;

vma_memory_read24659:
  t6 = zero + 224;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read24662;

vma_memory_read24667:
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

force_alignment24670:
  if (_trace) printf("force_alignment24670:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = arg6;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write24669;

vma_memory_write24668:
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

force_alignment24673:
  if (_trace) printf("force_alignment24673:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = t1;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write24672;

vma_memory_write24671:
  t1 = (512) << 16;   
  /* Memory Read Internal */

vma_memory_read24674:
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
    goto vma_memory_read24676;

vma_memory_read24675:
  t8 = zero + 224;   
  t10 = t10 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t10 & 1)   
    goto vma_memory_read24678;

vma_memory_read24683:
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

force_alignment24686:
  if (_trace) printf("force_alignment24686:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = arg1;
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write24685;

vma_memory_write24684:
  t9 = t1 | t9;		// Set cr.cleanup-bindings bit 
  *(u32 *)&processor->control = t9;
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t3;
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

vma_memory_write24685:
  if (_trace) printf("vma_memory_write24685:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24687:
  if (_trace) printf("force_alignment24687:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = arg6 - t7;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = arg1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto vma_memory_write24684;   

vma_memory_read24676:
  if (_trace) printf("vma_memory_read24676:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read24675;   

vma_memory_read24678:
  if (_trace) printf("vma_memory_read24678:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read24677;
  arg6 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read24674;   

vma_memory_read24677:
  if (_trace) printf("vma_memory_read24677:\n");
  t10 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg6;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read24680:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_write24672:
  if (_trace) printf("vma_memory_write24672:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24688:
  if (_trace) printf("force_alignment24688:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t4 = t1;
		/* write the stack cache */
  *(u32 *)(t4 + 4) = t2;
  goto vma_memory_write24671;   

vma_memory_write24669:
  if (_trace) printf("vma_memory_write24669:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24689:
  if (_trace) printf("force_alignment24689:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t4 = arg6;
		/* write the stack cache */
  *(u32 *)(t4 + 4) = t11;
  goto vma_memory_write24668;   

vma_memory_read24660:
  if (_trace) printf("vma_memory_read24660:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read24659;   

vma_memory_read24662:
  if (_trace) printf("vma_memory_read24662:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read24661;
  t8 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read24658;   

vma_memory_read24661:
  if (_trace) printf("vma_memory_read24661:\n");
  t7 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read24664:
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
#ifdef MINIMA
  t2 = arg3 >> 32;   
#endif
  arg3 = (u32)arg3;   
  arg4 = (u32)arg4;   
  t1 = arg3 - arg4;   
  if ((s64)t1 >= 0)   		// J. if binding stack overflow 
    goto bindlocov;
  t3 = arg3 + 1;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t1 = t2 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto bindlocdeep;
#endif
  t9 = *(s32 *)&processor->control;   
  t8 = arg6;
  /* Memory Read Internal */

vma_memory_read24691:
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
    goto vma_memory_read24693;

vma_memory_read24692:
  t6 = zero + 224;   
  t7 = t7 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  if (t7 & 1)   
    goto vma_memory_read24695;

vma_memory_read24700:
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

force_alignment24703:
  if (_trace) printf("force_alignment24703:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = arg6;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write24702;

vma_memory_write24701:
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

force_alignment24706:
  if (_trace) printf("force_alignment24706:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);   
  *(u32 *)t7 = t1;
  if (t8 != 0)   		// J. if in cache 
    goto vma_memory_write24705;

vma_memory_write24704:
  t1 = (512) << 16;   
  t9 = t1 | t9;		// Set cr.cleanup-bindings bit 
  *(u32 *)&processor->control = t9;
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t3;
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

vma_memory_write24705:
  if (_trace) printf("vma_memory_write24705:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24707:
  if (_trace) printf("force_alignment24707:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t4 = t1;
		/* write the stack cache */
  *(u32 *)(t4 + 4) = t2;
  goto vma_memory_write24704;   

vma_memory_write24702:
  if (_trace) printf("vma_memory_write24702:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24708:
  if (_trace) printf("force_alignment24708:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t5 = t3 - t5;   		// Stack cache offset 
  t4 = (t5 * 8) + t4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t4 = arg6;
		/* write the stack cache */
  *(u32 *)(t4 + 4) = t11;
  goto vma_memory_write24701;   

vma_memory_read24693:
  if (_trace) printf("vma_memory_read24693:\n");
  t5 = *(u64 *)&(processor->stackcachedata);   
  t4 = (t4 * 8) + t5;  		// reconstruct SCA 
  t1 = *(s32 *)t4;   
  t2 = *(s32 *)(t4 + 4);   		// Read from stack cache 
  goto vma_memory_read24692;   

vma_memory_read24695:
  if (_trace) printf("vma_memory_read24695:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read24694;
  t8 = (u32)t1;   		// Do the indirect thing 
  goto vma_memory_read24691;   

vma_memory_read24694:
  if (_trace) printf("vma_memory_read24694:\n");
  t7 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t7;   		// Adjust for a longword load 
  t7 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read24697:
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
#ifdef MINIMA
  arg3 = *(u64 *)&(processor->bindingstackpointer);   
#endif
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t1 = arg2 - Type_Fixnum;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto unbindniop;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t2 = arg3 >> 32;   
  t1 = t2 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto unbindndeep;
#endif
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
    goto g24709;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g24709:
  if (_trace) printf("g24709:\n");
  /* Memory Read Internal */

vma_memory_read24710:
  arg4 = t1 + ivory;
  t6 = (arg4 * 4);   
  t7 = LDQ_U(arg4);   
  t8 = t1 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read24712;

vma_memory_read24711:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t7 & 63);   
  arg4 = arg4 >> (t7 & 63);   
  if (arg5 & 1)   
    goto vma_memory_read24714;

vma_memory_read24719:
  /* Memory Read Internal */

vma_memory_read24720:
  arg4 = t5 + ivory;
  t2 = (arg4 * 4);   
  t3 = LDQ_U(arg4);   
  t8 = t5 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t3 = (u8)(t3 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read24722;

vma_memory_read24721:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t3 & 63);   
  arg4 = arg4 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (arg5 & 1)   
    goto vma_memory_read24724;

vma_memory_read24729:
  /* Memory Read Internal */

vma_memory_read24730:
  arg6 = t2 + ivory;
  arg3 = (arg6 * 4);   
  t8 = LDQ_U(arg6);   
  arg4 = t2 - t11;   		// Stack cache offset 
  arg5 = ((u64)arg4 < (u64)t12) ? 1 : 0;   		// In range? 
  arg3 = *(s32 *)arg3;   
  t8 = (u8)(t8 >> ((arg6&7)*8));   
  if (arg5 != 0)   
    goto vma_memory_read24732;

vma_memory_read24731:
  arg4 = *(u64 *)&(processor->bindwrite_mask);   
  arg6 = zero + 224;   
  arg4 = arg4 >> (t8 & 63);   
  arg6 = arg6 >> (t8 & 63);   
  if (arg4 & 1)   
    goto vma_memory_read24734;

vma_memory_read24739:
  /* Merge cdr-code */
  arg3 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | arg3;
  arg4 = t2 + ivory;
  arg3 = (arg4 * 4);   
  arg6 = LDQ_U(arg4);   
  arg5 = (t8 & 0xff) << ((arg4&7)*8);   
  arg6 = arg6 & ~(0xffL << (arg4&7)*8);   

force_alignment24742:
  if (_trace) printf("force_alignment24742:\n");
  arg6 = arg6 | arg5;
  STQ_U(arg4, arg6);   
  arg4 = *(s32 *)&processor->scovlimit;   
  arg5 = t2 - t11;   		// Stack cache offset 
  arg4 = ((u64)arg5 < (u64)arg4) ? 1 : 0;   		// In range? 
  *(u32 *)arg3 = t6;
  if (arg4 != 0)   		// J. if in cache 
    goto vma_memory_write24741;

vma_memory_write24740:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t1;
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
#ifdef MINIMA

unbindndeep:
  if (_trace) printf("unbindndeep:\n");
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
#endif

vma_memory_write24741:
  if (_trace) printf("vma_memory_write24741:\n");
  arg4 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg5 * 8) + arg4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)arg4 = t6;
		/* write the stack cache */
  *(u32 *)(arg4 + 4) = t8;
  goto vma_memory_write24740;   

vma_memory_read24732:
  if (_trace) printf("vma_memory_read24732:\n");
  arg5 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg4 * 8) + arg5;  		// reconstruct SCA 
  arg3 = *(s32 *)arg4;   
  t8 = *(s32 *)(arg4 + 4);   		// Read from stack cache 
  goto vma_memory_read24731;   

vma_memory_read24734:
  if (_trace) printf("vma_memory_read24734:\n");
  if ((arg6 & 1) == 0)   
    goto vma_memory_read24733;
  t2 = (u32)arg3;   		// Do the indirect thing 
  goto vma_memory_read24730;   

vma_memory_read24733:
  if (_trace) printf("vma_memory_read24733:\n");
  arg4 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  arg6 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  arg6 = (arg6 * 4) + arg4;   		// Adjust for a longword load 
  arg4 = *(s32 *)arg6;   		// Get the memory action 

vma_memory_read24736:
  /* Perform memory action */
  arg1 = arg4;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read24722:
  if (_trace) printf("vma_memory_read24722:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read24721;   

vma_memory_read24724:
  if (_trace) printf("vma_memory_read24724:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read24723;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read24720;   

vma_memory_read24723:
  if (_trace) printf("vma_memory_read24723:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read24726:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read24712:
  if (_trace) printf("vma_memory_read24712:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read24711;   

vma_memory_read24714:
  if (_trace) printf("vma_memory_read24714:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read24713;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read24710;   

vma_memory_read24713:
  if (_trace) printf("vma_memory_read24713:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read24716:
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
#ifdef MINIMA
  arg3 = *(u64 *)&(processor->bindingstackpointer);   
#endif
  arg2 = arg1 >> 32;   
  arg1 = (u32)arg1;   
  t1 = arg2 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto restorebsiop;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t2 = arg3 >> 32;   
  t1 = t2 - Type_Locative;   
  t1 = t1 & 63;		// Strip CDR code 
  if (t1 != 0)   
    goto restorebsdeep;
#endif
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
    goto g24743;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g24743:
  if (_trace) printf("g24743:\n");
  /* Memory Read Internal */

vma_memory_read24744:
  arg4 = t1 + ivory;
  t6 = (arg4 * 4);   
  t7 = LDQ_U(arg4);   
  t8 = t1 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t6 = *(s32 *)t6;   
  t7 = (u8)(t7 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read24746;

vma_memory_read24745:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t7 & 63);   
  arg4 = arg4 >> (t7 & 63);   
  if (arg5 & 1)   
    goto vma_memory_read24748;

vma_memory_read24753:
  /* Memory Read Internal */

vma_memory_read24754:
  arg4 = t5 + ivory;
  t2 = (arg4 * 4);   
  t3 = LDQ_U(arg4);   
  t8 = t5 - t11;   		// Stack cache offset 
  arg5 = *(u64 *)&(processor->bindread_mask);   
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;   		// In range? 
  t2 = *(s32 *)t2;   
  t3 = (u8)(t3 >> ((arg4&7)*8));   
  if (arg3 != 0)   
    goto vma_memory_read24756;

vma_memory_read24755:
  arg4 = zero + 224;   
  arg5 = arg5 >> (t3 & 63);   
  arg4 = arg4 >> (t3 & 63);   
  t2 = (u32)t2;   
  if (arg5 & 1)   
    goto vma_memory_read24758;

vma_memory_read24763:
  /* Memory Read Internal */

vma_memory_read24764:
  arg6 = t2 + ivory;
  arg3 = (arg6 * 4);   
  t8 = LDQ_U(arg6);   
  arg4 = t2 - t11;   		// Stack cache offset 
  arg5 = ((u64)arg4 < (u64)t12) ? 1 : 0;   		// In range? 
  arg3 = *(s32 *)arg3;   
  t8 = (u8)(t8 >> ((arg6&7)*8));   
  if (arg5 != 0)   
    goto vma_memory_read24766;

vma_memory_read24765:
  arg4 = *(u64 *)&(processor->bindwrite_mask);   
  arg6 = zero + 224;   
  arg4 = arg4 >> (t8 & 63);   
  arg6 = arg6 >> (t8 & 63);   
  if (arg4 & 1)   
    goto vma_memory_read24768;

vma_memory_read24773:
  /* Merge cdr-code */
  arg3 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | arg3;
  arg4 = t2 + ivory;
  arg3 = (arg4 * 4);   
  arg6 = LDQ_U(arg4);   
  arg5 = (t8 & 0xff) << ((arg4&7)*8);   
  arg6 = arg6 & ~(0xffL << (arg4&7)*8);   

force_alignment24776:
  if (_trace) printf("force_alignment24776:\n");
  arg6 = arg6 | arg5;
  STQ_U(arg4, arg6);   
  arg4 = *(s32 *)&processor->scovlimit;   
  arg5 = t2 - t11;   		// Stack cache offset 
  arg4 = ((u64)arg5 < (u64)arg4) ? 1 : 0;   		// In range? 
  *(u32 *)arg3 = t6;
  if (arg4 != 0)   		// J. if in cache 
    goto vma_memory_write24775;

vma_memory_write24774:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t1;
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
#ifdef MINIMA

restorebsdeep:
  if (_trace) printf("restorebsdeep:\n");
  t1 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);   
  t2 = *(u64 *)&(processor->stackcachebasevma);   
  t3 = t1 - t3;   		// stack cache base relative offset 
  t3 = t3 >> 3;   		// convert byte address to word address 
  t2 = t3 + t2;		// reconstruct VMA 
  arg5 = t2;
  arg2 = 66;
  goto illegaloperand;
#endif

vma_memory_write24775:
  if (_trace) printf("vma_memory_write24775:\n");
  arg4 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg5 * 8) + arg4;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)arg4 = t6;
		/* write the stack cache */
  *(u32 *)(arg4 + 4) = t8;
  goto vma_memory_write24774;   

vma_memory_read24766:
  if (_trace) printf("vma_memory_read24766:\n");
  arg5 = *(u64 *)&(processor->stackcachedata);   
  arg4 = (arg4 * 8) + arg5;  		// reconstruct SCA 
  arg3 = *(s32 *)arg4;   
  t8 = *(s32 *)(arg4 + 4);   		// Read from stack cache 
  goto vma_memory_read24765;   

vma_memory_read24768:
  if (_trace) printf("vma_memory_read24768:\n");
  if ((arg6 & 1) == 0)   
    goto vma_memory_read24767;
  t2 = (u32)arg3;   		// Do the indirect thing 
  goto vma_memory_read24764;   

vma_memory_read24767:
  if (_trace) printf("vma_memory_read24767:\n");
  arg4 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  arg6 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t2;   		// stash the VMA for the (likely) trap 
  arg6 = (arg6 * 4) + arg4;   		// Adjust for a longword load 
  arg4 = *(s32 *)arg6;   		// Get the memory action 

vma_memory_read24770:
  /* Perform memory action */
  arg1 = arg4;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read24756:
  if (_trace) printf("vma_memory_read24756:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t2 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read24755;   

vma_memory_read24758:
  if (_trace) printf("vma_memory_read24758:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read24757;
  t5 = (u32)t2;   		// Do the indirect thing 
  goto vma_memory_read24754;   

vma_memory_read24757:
  if (_trace) printf("vma_memory_read24757:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read24760:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read24746:
  if (_trace) printf("vma_memory_read24746:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + arg3;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read24745;   

vma_memory_read24748:
  if (_trace) printf("vma_memory_read24748:\n");
  if ((arg4 & 1) == 0)   
    goto vma_memory_read24747;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read24744;   

vma_memory_read24747:
  if (_trace) printf("vma_memory_read24747:\n");
  arg5 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  arg4 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  arg4 = (arg4 * 4) + arg5;   		// Adjust for a longword load 
  arg5 = *(s32 *)arg4;   		// Get the memory action 

vma_memory_read24750:
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
		/* Store fixnum result */
  *(u32 *)(iSP + -8) = arg5;
		/* write the stack cache */
  *(u32 *)(iSP + -4) = t1;
		/* Store the carry if any */
  *(u32 *)iSP = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
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
		/* Store fixnum result */
  *(u32 *)(iSP + -8) = arg5;
		/* write the stack cache */
  *(u32 *)(iSP + -4) = t1;
  arg6 = arg6 + t6;		// Compute borrow 
		/* Store the borrow if any */
  *(u32 *)iSP = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
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
		/* Store fixnum result ls word */
  *(u32 *)iSP = arg3;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
		/* Store ms word */
  *(u32 *)(iSP + 8) = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t1;
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
		/* store quotient (already fixnum) */
  *(u32 *)(iSP + -8) = t1;
		/* store remainder (already fixnum) */
  *(u32 *)iSP = t2;
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

force_alignment24777:
  if (_trace) printf("force_alignment24777:\n");
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
		/* Store the result as a fixnum */
  *(u32 *)iSP = arg6;
		/* write the stack cache */
  *(u32 *)(iSP + 4) = t1;
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
    goto do_unwind_protect24778;

do_unwind_protect24779:
  if (_trace) printf("do_unwind_protect24779:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t4 = *(s32 *)&processor->control;   
  t1 = (u32)t1;   		// vma only 
  arg1 = (512) << 16;   
  t5 = t1 - 1;   
  t3 = t4 & arg1;
  t4 = t4 & ~arg1;		// Turn off the bit 
  if (t3 != 0)   
    goto g24780;
  t4 = *(u64 *)&(processor->restartsp);   		// Get the SP, ->op2 
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g24780:
  if (_trace) printf("g24780:\n");
  /* Memory Read Internal */

vma_memory_read24781:
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
    goto vma_memory_read24783;

vma_memory_read24782:
  t10 = zero + 224;   
  t11 = t11 >> (t7 & 63);   
  t10 = t10 >> (t7 & 63);   
  if (t11 & 1)   
    goto vma_memory_read24785;

vma_memory_read24790:
  /* Memory Read Internal */

vma_memory_read24791:
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
    goto vma_memory_read24793;

vma_memory_read24792:
  t10 = zero + 224;   
  t11 = t11 >> (t3 & 63);   
  t10 = t10 >> (t3 & 63);   
  arg1 = (u32)arg1;   
  if (t11 & 1)   
    goto vma_memory_read24795;

vma_memory_read24800:
  /* Memory Read Internal */

vma_memory_read24801:
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
    goto vma_memory_read24803;

vma_memory_read24802:
  t10 = *(u64 *)&(processor->bindwrite_mask);   
  t12 = zero + 224;   
  t10 = t10 >> (t8 & 63);   
  t12 = t12 >> (t8 & 63);   
  if (t10 & 1)   
    goto vma_memory_read24805;

vma_memory_read24810:
  /* Merge cdr-code */
  t9 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | t9;
  t10 = arg1 + ivory;
  t9 = (t10 * 4);   
  t12 = LDQ_U(t10);   
  t11 = (t8 & 0xff) << ((t10&7)*8);   
  t12 = t12 & ~(0xffL << (t10&7)*8);   

force_alignment24813:
  if (_trace) printf("force_alignment24813:\n");
  t12 = t12 | t11;
  t11 = *(u64 *)&(processor->stackcachebasevma);   
  STQ_U(t10, t12);   
  t10 = *(s32 *)&processor->scovlimit;   
  t11 = arg1 - t11;   		// Stack cache offset 
  t10 = ((u64)t11 < (u64)t10) ? 1 : 0;   		// In range? 
  *(u32 *)t9 = t6;
  if (t10 != 0)   		// J. if in cache 
    goto vma_memory_write24812;

vma_memory_write24811:
  t3 = t3 & 64;		// Get the old cleanup-bindings bit 
  t3 = t3 << 19;   
  t1 = t1 - 2;   
		/* vma only */
  *(u32 *)&processor->bindingstackpointer = t1;
  t4 = t4 | t3;
  *(u32 *)&processor->control = t4;
  t1 = *(u64 *)&(processor->bindingstackpointer);   
  t3 = (s32)t1 - (s32)t2;   
  if (t3 != 0)   		// J. if binding level/= binding stack 
    goto do_unwind_protect24779;
  t2 = *(s32 *)&processor->interruptreg;   
  t3 = t2 & 2;
  t3 = (t3 == 2) ? 1 : 0;   
  t2 = t2 | t3;
  *(u32 *)&processor->interruptreg = t2;
  if (t2 == 0) 
    goto do_unwind_protect24778;
  *(u64 *)&processor->stop_interpreter = t2;   

do_unwind_protect24778:
  if (_trace) printf("do_unwind_protect24778:\n");
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
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

vma_memory_write24812:
  if (_trace) printf("vma_memory_write24812:\n");
  t10 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t11 * 8) + t10;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t10 = t6;
		/* write the stack cache */
  *(u32 *)(t10 + 4) = t8;
  goto vma_memory_write24811;   

vma_memory_read24803:
  if (_trace) printf("vma_memory_read24803:\n");
  t11 = *(u64 *)&(processor->stackcachedata);   
  t10 = (t10 * 8) + t11;  		// reconstruct SCA 
  t9 = *(s32 *)t10;   
  t8 = *(s32 *)(t10 + 4);   		// Read from stack cache 
  goto vma_memory_read24802;   

vma_memory_read24805:
  if (_trace) printf("vma_memory_read24805:\n");
  if ((t12 & 1) == 0)   
    goto vma_memory_read24804;
  arg1 = (u32)t9;   		// Do the indirect thing 
  goto vma_memory_read24801;   

vma_memory_read24804:
  if (_trace) printf("vma_memory_read24804:\n");
  t10 = *(u64 *)&(processor->bindwrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t12 = t8 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t12 = (t12 * 4) + t10;   		// Adjust for a longword load 
  t10 = *(s32 *)t12;   		// Get the memory action 

vma_memory_read24807:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

vma_memory_read24793:
  if (_trace) printf("vma_memory_read24793:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  arg1 = *(s32 *)t8;   
  t3 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read24792;   

vma_memory_read24795:
  if (_trace) printf("vma_memory_read24795:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read24794;
  t5 = (u32)arg1;   		// Do the indirect thing 
  goto vma_memory_read24791;   

vma_memory_read24794:
  if (_trace) printf("vma_memory_read24794:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t3 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t5;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read24797:
  /* Perform memory action */
  arg1 = t11;
  arg2 = 2;
  goto performmemoryaction;

vma_memory_read24783:
  if (_trace) printf("vma_memory_read24783:\n");
  t9 = *(u64 *)&(processor->stackcachedata);   
  t8 = (t8 * 8) + t9;  		// reconstruct SCA 
  t6 = *(s32 *)t8;   
  t7 = *(s32 *)(t8 + 4);   		// Read from stack cache 
  goto vma_memory_read24782;   

vma_memory_read24785:
  if (_trace) printf("vma_memory_read24785:\n");
  if ((t10 & 1) == 0)   
    goto vma_memory_read24784;
  t1 = (u32)t6;   		// Do the indirect thing 
  goto vma_memory_read24781;   

vma_memory_read24784:
  if (_trace) printf("vma_memory_read24784:\n");
  t11 = *(u64 *)&(processor->bindread);   		// Load the memory action table for cycle 
  /* TagType. */
  t10 = t7 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t10 = (t10 * 4) + t11;   		// Adjust for a longword load 
  t11 = *(s32 *)t10;   		// Get the memory action 

vma_memory_read24787:
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

force_alignment24842:
  if (_trace) printf("force_alignment24842:\n");
  if (t1 == 0) 
    goto basic_dispatch24815;
  /* Here if argument MemoryActionTrap */
  t1 = *(u64 *)&(processor->vma);   		// Get the failing VMA 
  t2 = (arg2 == Cycle_DataRead) ? 1 : 0;   

force_alignment24829:
  if (_trace) printf("force_alignment24829:\n");
  if (t2 == 0) 
    goto basic_dispatch24817;
  /* Here if argument CycleDataRead */
  arg5 = t1;
  arg2 = 57;
  goto illegaloperand;

basic_dispatch24817:
  if (_trace) printf("basic_dispatch24817:\n");
  t2 = (arg2 == Cycle_DataWrite) ? 1 : 0;   

force_alignment24830:
  if (_trace) printf("force_alignment24830:\n");
  if (t2 == 0) 
    goto basic_dispatch24818;
  /* Here if argument CycleDataWrite */
  arg5 = t1;
  arg2 = 58;
  goto illegaloperand;

basic_dispatch24818:
  if (_trace) printf("basic_dispatch24818:\n");
  t2 = (arg2 == Cycle_BindRead) ? 1 : 0;   

force_alignment24831:
  if (_trace) printf("force_alignment24831:\n");
  if (t2 != 0)   
    goto basic_dispatch24820;
  t2 = (arg2 == Cycle_BindReadNoMonitor) ? 1 : 0;   

force_alignment24832:
  if (_trace) printf("force_alignment24832:\n");
  if (t2 == 0) 
    goto basic_dispatch24819;

basic_dispatch24820:
  if (_trace) printf("basic_dispatch24820:\n");
  /* Here if argument (CycleBindRead CycleBindReadNoMonitor) */
  arg5 = t1;
  arg2 = 54;
  goto illegaloperand;

basic_dispatch24819:
  if (_trace) printf("basic_dispatch24819:\n");
  t2 = (arg2 == Cycle_BindWrite) ? 1 : 0;   

force_alignment24833:
  if (_trace) printf("force_alignment24833:\n");
  if (t2 != 0)   
    goto basic_dispatch24822;
  t2 = (arg2 == Cycle_BindWriteNoMonitor) ? 1 : 0;   

force_alignment24834:
  if (_trace) printf("force_alignment24834:\n");
  if (t2 == 0) 
    goto basic_dispatch24821;

basic_dispatch24822:
  if (_trace) printf("basic_dispatch24822:\n");
  /* Here if argument (CycleBindWrite CycleBindWriteNoMonitor) */
  arg5 = t1;
  arg2 = 55;
  goto illegaloperand;

basic_dispatch24821:
  if (_trace) printf("basic_dispatch24821:\n");
  t2 = (arg2 == Cycle_Header) ? 1 : 0;   

force_alignment24835:
  if (_trace) printf("force_alignment24835:\n");
  if (t2 != 0)   
    goto basic_dispatch24824;
  t2 = (arg2 == Cycle_StructureOffset) ? 1 : 0;   

force_alignment24836:
  if (_trace) printf("force_alignment24836:\n");
  if (t2 == 0) 
    goto basic_dispatch24823;

basic_dispatch24824:
  if (_trace) printf("basic_dispatch24824:\n");
  /* Here if argument (CycleHeader CycleStructureOffset) */
  arg5 = t1;
  arg2 = 59;
  goto illegaloperand;

basic_dispatch24823:
  if (_trace) printf("basic_dispatch24823:\n");
  t2 = (arg2 == Cycle_Scavenge) ? 1 : 0;   

force_alignment24837:
  if (_trace) printf("force_alignment24837:\n");
  if (t2 != 0)   
    goto basic_dispatch24826;
  t2 = (arg2 == Cycle_GCCopy) ? 1 : 0;   

force_alignment24838:
  if (_trace) printf("force_alignment24838:\n");
  if (t2 == 0) 
    goto basic_dispatch24825;

basic_dispatch24826:
  if (_trace) printf("basic_dispatch24826:\n");
  /* Here if argument (CycleScavenge CycleGCCopy) */
  arg5 = t1;
  arg2 = 60;
  goto illegaloperand;

basic_dispatch24825:
  if (_trace) printf("basic_dispatch24825:\n");
  t2 = (arg2 == Cycle_Cdr) ? 1 : 0;   

force_alignment24839:
  if (_trace) printf("force_alignment24839:\n");
  if (t2 == 0) 
    goto basic_dispatch24816;
  /* Here if argument CycleCdr */
  arg5 = t1;
  arg2 = 56;
  goto illegaloperand;

basic_dispatch24816:
  if (_trace) printf("basic_dispatch24816:\n");

basic_dispatch24815:
  if (_trace) printf("basic_dispatch24815:\n");
  t1 = (arg1 == MemoryActionMonitor) ? 1 : 0;   

force_alignment24843:
  if (_trace) printf("force_alignment24843:\n");
  if (t1 == 0) 
    goto basic_dispatch24814;
  /* Here if argument MemoryActionMonitor */
  goto monitortrap;

basic_dispatch24814:
  if (_trace) printf("basic_dispatch24814:\n");

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
    goto exception_handler24845;
  t1 = (u16)(arg2 >> ((4&7)*8));   		// Get original operand 
  t3 = (t1 == 512) ? 1 : 0;   		// t3 is non-zero iff SP|POP operand 
  if (t3 != 0)   		// SP|POP operand recovered by restoring SP 
    goto exception_handler24845;
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
    goto exception_handler24846;
  t1 = t2 << 56;   
  t3 = arg2 >> 16;   
  t1 = (s64)t1 >> 56;   
  arg5 = (u64)&processor->immediate_arg;   		// Immediate mode constant 
  if ((t3 & 1) == 0)   		// Signed immediate 
   t2 = t1;
  *(u32 *)&processor->immediate_arg = t2;

exception_handler24846:
  if (_trace) printf("exception_handler24846:\n");
  t1 = zero + -32768;   
  t1 = t1 + ((2) << 16);   
  t2 = arg2 & t1;
  t3 = (t1 == t2) ? 1 : 0;   
  if (t3 == 0) 		// J. if not address-format operand 
    goto exception_handler24847;
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
  goto exception_handler24848;   

exception_handler24847:
  if (_trace) printf("exception_handler24847:\n");
  arg5 = *(u64 *)arg5;   		// Fetch the arg 

exception_handler24848:
  if (_trace) printf("exception_handler24848:\n");
  *(u64 *)(iSP + 8) = arg5;   
  iSP = iSP + 8;

exception_handler24845:
  if (_trace) printf("exception_handler24845:\n");
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
    goto exception_handler24850;
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
    goto exception_handler24851;
  t1 = t2 << 56;   
  t3 = arg2 >> 16;   
  t1 = (s64)t1 >> 56;   
  arg5 = (u64)&processor->immediate_arg;   		// Immediate mode constant 
  if ((t3 & 1) == 0)   		// Signed immediate 
   t2 = t1;
  *(u32 *)&processor->immediate_arg = t2;

exception_handler24851:
  if (_trace) printf("exception_handler24851:\n");
  t1 = zero + -32768;   
  t1 = t1 + ((2) << 16);   
  t2 = arg2 & t1;
  t3 = (t1 == t2) ? 1 : 0;   
  if (t3 == 0) 		// J. if not address-format operand 
    goto exception_handler24852;
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
  goto exception_handler24853;   

exception_handler24852:
  if (_trace) printf("exception_handler24852:\n");
  arg5 = *(u64 *)arg5;   		// Fetch the arg 

exception_handler24853:
  if (_trace) printf("exception_handler24853:\n");
  *(u64 *)(iSP + 8) = arg5;   
  iSP = iSP + 8;

exception_handler24850:
  if (_trace) printf("exception_handler24850:\n");
  t4 = arg2 >> 17;   		// Get unary/nary bit of opcode 
  arg1 = 1;		// Assume unary 
  t11 = zero;
  t2 = iSP;
  if ((t4 & 1) == 0)   		// J. if not binary arithmetic dispatch 
    goto exception_handler24849;
  arg1 = 2;		// Nary -> Binary 
  t11 = *(s32 *)(iSP + 4);   
  t2 = t2 - 8;   
  t11 = t11 & 7;		// low three bits has opcode tag for op2 

exception_handler24849:
  if (_trace) printf("exception_handler24849:\n");
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
    goto exception_handler24855;
  t1 = (u16)(arg2 >> ((4&7)*8));   		// Get original operand 
  t3 = (t1 == 512) ? 1 : 0;   		// t3 is non-zero iff SP|POP operand 
  if (t3 != 0)   		// SP|POP operand recovered by restoring SP 
    goto exception_handler24855;
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
    goto exception_handler24856;
  t1 = t2 << 56;   
  t3 = arg2 >> 16;   
  t1 = (s64)t1 >> 56;   
  arg5 = (u64)&processor->immediate_arg;   		// Immediate mode constant 
  if ((t3 & 1) == 0)   		// Signed immediate 
   t2 = t1;
  *(u32 *)&processor->immediate_arg = t2;

exception_handler24856:
  if (_trace) printf("exception_handler24856:\n");
  t1 = zero + -32768;   
  t1 = t1 + ((2) << 16);   
  t2 = arg2 & t1;
  t3 = (t1 == t2) ? 1 : 0;   
  if (t3 == 0) 		// J. if not address-format operand 
    goto exception_handler24857;
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
  goto exception_handler24858;   

exception_handler24857:
  if (_trace) printf("exception_handler24857:\n");
  arg5 = *(u64 *)arg5;   		// Fetch the arg 

exception_handler24858:
  if (_trace) printf("exception_handler24858:\n");
  *(u64 *)(iSP + 8) = arg5;   
  iSP = iSP + 8;

exception_handler24855:
  if (_trace) printf("exception_handler24855:\n");
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

vma_memory_read24862:
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
    goto vma_memory_read24864;

vma_memory_read24863:
  t6 = zero + 240;   
  t5 = t5 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t5 & 1)   
    goto vma_memory_read24866;

vma_memory_read24873:
  t5 = t2 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto get_trap_vector_entry24861;
		/* Restore the cr */
  *(u32 *)&processor->control = t4;
  t8 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t5 = zero + 128;   
  t6 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t5 = t5 + 8;		// Account for what we're about to push 
  t5 = (t5 * 8) + iSP;  		// SCA of desired end of cache 
  t6 = (t8 * 8) + t6;  		// SCA of current end of cache 
  t8 = ((s64)t5 <= (s64)t6) ? 1 : 0;   
  if (t8 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check24874;
  iFP = (arg1 * 8) + zero;  
  iFP = iSP - iFP;   
  iFP = iFP + 8;
  if (arg1 == 0) 
    goto take_post_trap24859;
  t5 = *(u64 *)iSP;   
  *(u64 *)(iSP + 32) = t5;   
  arg1 = arg1 - 1;   
  if (arg1 == 0) 
    goto take_post_trap24859;
  t5 = *(u64 *)(iSP + -8);   
  *(u64 *)(iSP + 24) = t5;   
  arg1 = arg1 - 1;   
  if (arg1 == 0) 
    goto take_post_trap24859;
  t5 = *(u64 *)(iSP + -16);   
  *(u64 *)(iSP + 16) = t5;   
  arg1 = arg1 - 1;   
  if (arg1 == 0) 
    goto take_post_trap24859;
  t5 = *(u64 *)(iSP + -24);   
  *(u64 *)(iSP + 8) = t5;   
  arg1 = arg1 - 1;   

take_post_trap24859:
  if (_trace) printf("take_post_trap24859:\n");
  iSP = iSP + 32;
  t5 = *(s32 *)&processor->continuation;   
  t7 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t8 = (8192) << 16;   
  t4 = (u32)t4;   
  t7 = t7 | 192;
  *(u32 *)iFP = t5;
		/* write the stack cache */
  *(u32 *)(iFP + 4) = t7;
  t8 = t4 & t8;
  t8 = t8 >> 2;   
  t6 = Type_Fixnum+0xC0;
  t8 = t4 | t8;
  *(u32 *)(iFP + 8) = t8;
		/* write the stack cache */
  *(u32 *)(iFP + 12) = t6;
  iLP = iSP + 8;
  t6 = Type_Fixnum;
  t8 = t11;
  *(u32 *)(iFP + 16) = t8;
		/* write the stack cache */
  *(u32 *)(iFP + 20) = t6;
  /* Convert PC to a real continuation. */
  t6 = iPC & 1;
  t8 = iPC >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *(u32 *)(iFP + 24) = t8;
		/* write the stack cache */
  *(u32 *)(iFP + 28) = t6;
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
    goto take_post_trap24860;
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

take_post_trap24860:
  if (_trace) printf("take_post_trap24860:\n");
  if (t6 == 0) 		// Take the overflow if in emulator mode 
    goto stackoverflow;
  goto fatalstackoverflow;

stack_cache_overflow_check24874:
  if (_trace) printf("stack_cache_overflow_check24874:\n");
  arg2 = 8;
  goto stackcacheoverflowhandler;   

vma_memory_read24864:
  if (_trace) printf("vma_memory_read24864:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read24863;   

vma_memory_read24866:
  if (_trace) printf("vma_memory_read24866:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read24865;
  t8 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24862;   

vma_memory_read24865:
  if (_trace) printf("vma_memory_read24865:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read24870:
  if (_trace) printf("vma_memory_read24870:\n");
  t6 = t5 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read24869;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read24873;   

vma_memory_read24869:

vma_memory_read24868:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

get_trap_vector_entry24861:
  if (_trace) printf("get_trap_vector_entry24861:\n");
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

vma_memory_read24878:
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
    goto vma_memory_read24880;

vma_memory_read24879:
  t6 = zero + 240;   
  t5 = t5 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t5 & 1)   
    goto vma_memory_read24882;

vma_memory_read24889:
  t5 = t2 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto get_trap_vector_entry24877;
		/* Restore the cr */
  *(u32 *)&processor->control = t4;
  t8 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t5 = zero + 128;   
  t6 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t5 = t5 + 8;		// Account for what we're about to push 
  t5 = (t5 * 8) + iSP;  		// SCA of desired end of cache 
  t6 = (t8 * 8) + t6;  		// SCA of current end of cache 
  t8 = ((s64)t5 <= (s64)t6) ? 1 : 0;   
  if (t8 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check24890;
  iFP = (zero * 8) + zero;  
  iFP = iSP - iFP;   
  iFP = iFP + 8;
  if (zero == 0) 
    goto take_post_trap24875;
  t5 = *(u64 *)iSP;   
  *(u64 *)(iSP + 32) = t5;   
  if (zero == 0) 
    goto take_post_trap24875;
  t5 = *(u64 *)(iSP + -8);   
  *(u64 *)(iSP + 24) = t5;   
  if (zero == 0) 
    goto take_post_trap24875;
  t5 = *(u64 *)(iSP + -16);   
  *(u64 *)(iSP + 16) = t5;   
  if (zero == 0) 
    goto take_post_trap24875;
  t5 = *(u64 *)(iSP + -24);   
  *(u64 *)(iSP + 8) = t5;   

take_post_trap24875:
  if (_trace) printf("take_post_trap24875:\n");
  iSP = iSP + 32;
  t5 = *(s32 *)&processor->continuation;   
  t7 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t8 = (8192) << 16;   
  t4 = (u32)t4;   
  t7 = t7 | 192;
  *(u32 *)iFP = t5;
		/* write the stack cache */
  *(u32 *)(iFP + 4) = t7;
  t8 = t4 & t8;
  t8 = t8 >> 2;   
  t6 = Type_Fixnum+0xC0;
  t8 = t4 | t8;
  *(u32 *)(iFP + 8) = t8;
		/* write the stack cache */
  *(u32 *)(iFP + 12) = t6;
  iLP = iSP + 8;
  t6 = Type_Fixnum;
  t8 = TrapVector_StackOverflow;
  *(u32 *)(iFP + 16) = t8;
		/* write the stack cache */
  *(u32 *)(iFP + 20) = t6;
  /* Convert PC to a real continuation. */
  t6 = iPC & 1;
  t8 = iPC >> 1;   		// convert PC to a real word address. 
  t6 = t6 + Type_EvenPC;   
  *(u32 *)(iFP + 24) = t8;
		/* write the stack cache */
  *(u32 *)(iFP + 28) = t6;
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
    goto take_post_trap24876;
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

take_post_trap24876:
  if (_trace) printf("take_post_trap24876:\n");
  if (t6 == 0) 		// Take the overflow if in emulator mode 
    goto stackoverflow;
  goto fatalstackoverflow;

stack_cache_overflow_check24890:
  if (_trace) printf("stack_cache_overflow_check24890:\n");
  arg2 = 8;
  goto stackcacheoverflowhandler;   

vma_memory_read24880:
  if (_trace) printf("vma_memory_read24880:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read24879;   

vma_memory_read24882:
  if (_trace) printf("vma_memory_read24882:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read24881;
  t8 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24878;   

vma_memory_read24881:
  if (_trace) printf("vma_memory_read24881:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read24886:
  if (_trace) printf("vma_memory_read24886:\n");
  t6 = t5 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read24885;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read24889;   

vma_memory_read24885:

vma_memory_read24884:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

get_trap_vector_entry24877:
  if (_trace) printf("get_trap_vector_entry24877:\n");
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

vma_memory_read24892:
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
    goto vma_memory_read24894;

vma_memory_read24893:
  t6 = zero + 240;   
  t5 = t5 >> (t2 & 63);   
  t6 = t6 >> (t2 & 63);   
  t3 = (u32)t3;   
  if (t5 & 1)   
    goto vma_memory_read24896;

vma_memory_read24903:
  t5 = t2 - Type_EvenPC;   
  t5 = t5 & 62;		// Strip CDR code, low bits 
  if (t5 != 0)   
    goto get_trap_vector_entry24891;
		/* Restore the cr */
  *(u32 *)&processor->control = t4;
  iSP = *(u64 *)&(processor->restartsp);   
  t7 = *(s32 *)&processor->scovlimit;   		// Current stack cache limit (words) 
  t4 = zero + 128;   
  t5 = *(u64 *)&(processor->stackcachedata);   		// Alpha base of stack cache 
  t4 = t4 + 8;		// Account for what we're about to push 
  t4 = (t4 * 8) + iSP;  		// SCA of desired end of cache 
  t5 = (t7 * 8) + t5;  		// SCA of current end of cache 
  t7 = ((s64)t4 <= (s64)t5) ? 1 : 0;   
  if (t7 == 0) 		// We're done if new SCA is within bounds 
    goto stack_cache_overflow_check24904;
  t5 = *(s32 *)&processor->continuation;   
  t4 = *((s32 *)(&processor->continuation)+1);   
  t5 = (u32)t5;   
  t7 = *(s32 *)&processor->control;   
  t7 = (u32)t7;   
  t4 = t4 | 192;
  *(u32 *)(iSP + 8) = t5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
  iSP = iSP + 8;
  t6 = Type_Fixnum+0xC0;
  *(u32 *)(iSP + 8) = t7;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t6;
  iSP = iSP + 8;
  t6 = t10;
  t8 = Type_Fixnum;
  *(u32 *)(iSP + 8) = t6;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t8;
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t9;
  iSP = iSP + 8;
  goto *r0; /* ret */

stack_cache_overflow_check24904:
  if (_trace) printf("stack_cache_overflow_check24904:\n");
  arg2 = 8;
  goto stackcacheoverflowhandler;   

vma_memory_read24894:
  if (_trace) printf("vma_memory_read24894:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t9 = (t9 * 8) + t7;  		// reconstruct SCA 
  t3 = *(s32 *)t9;   
  t2 = *(s32 *)(t9 + 4);   		// Read from stack cache 
  goto vma_memory_read24893;   

vma_memory_read24896:
  if (_trace) printf("vma_memory_read24896:\n");
  if ((t6 & 1) == 0)   
    goto vma_memory_read24895;
  t8 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24892;   

vma_memory_read24895:
  if (_trace) printf("vma_memory_read24895:\n");
  t5 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t6 = t2 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t8;   		// stash the VMA for the (likely) trap 
  t6 = (t6 * 4) + t5;   		// Adjust for a longword load 
  t5 = *(s32 *)t6;   		// Get the memory action 

vma_memory_read24900:
  if (_trace) printf("vma_memory_read24900:\n");
  t6 = t5 & MemoryActionTransform;
  if (t6 == 0) 
    goto vma_memory_read24899;
  t2 = t2 & ~63L;
  t2 = t2 | Type_ExternalValueCellPointer;
  goto vma_memory_read24903;   

vma_memory_read24899:

vma_memory_read24898:
  /* Perform memory action */
  arg1 = t5;
  arg2 = 0;
  goto performmemoryaction;

get_trap_vector_entry24891:
  if (_trace) printf("get_trap_vector_entry24891:\n");
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
  r0 = (u64)&&return0171;
  goto startpretrap;
return0171:
  t11 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t11;
  iSP = iSP + 8;
  t11 = Type_Locative;
  *(u32 *)(iSP + 8) = arg5;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t11;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end ILLEGALOPERAND */
/* start RESETTRAP */


resettrap:
  if (_trace) printf("resettrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Reset;		// save the trap vector index 
  r0 = (u64)&&return0172;
  goto startpretrap;
return0172:
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
  r0 = (u64)&&return0173;
  goto startpretrap;
return0173:
  arg2 = Type_Fixnum;
  *(u32 *)(iSP + 8) = arg1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = arg2;
  iSP = iSP + 8;
  arg2 = t11 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t12;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = arg2;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PULLAPPLYARGSTRAP */
/* start TRACETRAP */


tracetrap:
  if (_trace) printf("tracetrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Trace;		// save the trap vector index 
  r0 = (u64)&&return0174;
  goto startpretrap;
return0174:
  goto finishpretrap;   

/* end TRACETRAP */
/* start PREEMPTREQUESTTRAP */


preemptrequesttrap:
  if (_trace) printf("preemptrequesttrap:\n");
#endif
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PreemptRequest;		// save the trap vector index 
  r0 = (u64)&&return0175;
  goto startpretrap;
return0175:
  goto finishpretrap;   

/* end PREEMPTREQUESTTRAP */
/* start HIGHPRIORITYSEQUENCEBREAK */


highprioritysequencebreak:
  if (_trace) printf("highprioritysequencebreak:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_HighPrioritySequenceBreak;		// save the trap vector index 
  r0 = (u64)&&return0176;
  goto startpretrap;
return0176:
  goto finishpretrap;   

/* end HIGHPRIORITYSEQUENCEBREAK */
/* start LOWPRIORITYSEQUENCEBREAK */


lowprioritysequencebreak:
  if (_trace) printf("lowprioritysequencebreak:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_LowPrioritySequenceBreak;		// save the trap vector index 
  r0 = (u64)&&return0177;
  goto startpretrap;
return0177:
  goto finishpretrap;   

/* end LOWPRIORITYSEQUENCEBREAK */
/* start DBUNWINDFRAMETRAP */


dbunwindframetrap:
  if (_trace) printf("dbunwindframetrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_DBUnwindFrame;		// save the trap vector index 
  r0 = (u64)&&return0178;
  goto startpretrap;
return0178:
  t11 = *(u64 *)&(processor->bindingstackpointer);   
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end DBUNWINDFRAMETRAP */
/* start DBUNWINDCATCHTRAP */


dbunwindcatchtrap:
  if (_trace) printf("dbunwindcatchtrap:\n");
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_DBUnwindCatch;		// save the trap vector index 
  r0 = (u64)&&return0179;
  goto startpretrap;
return0179:
  t11 = *(u64 *)&(processor->bindingstackpointer);   
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end DBUNWINDCATCHTRAP */
/* start TRANSPORTTRAP */


transporttrap:
  if (_trace) printf("transporttrap:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Transport;		// save the trap vector index 
  r0 = (u64)&&return0180;
  goto startpretrap;
return0180:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end TRANSPORTTRAP */
/* start MONITORTRAP */


monitortrap:
  if (_trace) printf("monitortrap:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_Monitor;		// save the trap vector index 
  r0 = (u64)&&return0181;
  goto startpretrap;
return0181:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end MONITORTRAP */
/* start PAGENOTRESIDENT */


pagenotresident:
  if (_trace) printf("pagenotresident:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PageNotResident;		// save the trap vector index 
  r0 = (u64)&&return0182;
  goto startpretrap;
return0182:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PAGENOTRESIDENT */
/* start PAGEFAULTREQUESTHANDLER */


pagefaultrequesthandler:
  if (_trace) printf("pagefaultrequesthandler:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PageFaultRequest;		// save the trap vector index 
  r0 = (u64)&&return0183;
  goto startpretrap;
return0183:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PAGEFAULTREQUESTHANDLER */
/* start PAGEWRITEFAULT */


pagewritefault:
  if (_trace) printf("pagewritefault:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_PageWriteFault;		// save the trap vector index 
  r0 = (u64)&&return0184;
  goto startpretrap;
return0184:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end PAGEWRITEFAULT */
#ifdef MINIMA
/* start DBCACHEMISSTRAP */


dbcachemisstrap:
  if (_trace) printf("dbcachemisstrap:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = trapvectordbcachemiss;		// save the trap vector index 
  r0 = (u64)&&return0185;
  goto startpretrap;
return0185:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end DBCACHEMISSTRAP */
#endif
  /* The following handlers should never be invoked. */
/* start UNCORRECTABLEMEMORYERROR */


uncorrectablememoryerror:
  if (_trace) printf("uncorrectablememoryerror:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_UncorrectableMemoryError;		// save the trap vector index 
  r0 = (u64)&&return0186;
  goto startpretrap;
return0186:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
  iSP = iSP + 8;
  goto finishpretrap;   

/* end UNCORRECTABLEMEMORYERROR */
/* start BUSERROR */


buserror:
  if (_trace) printf("buserror:\n");
  t11 = *(u64 *)&(processor->vma);   		// Preserve VMA against reading trap vector 
  t1 = iFP;		// save old frame pointer 
  t10 = TrapVector_MemoryBusError;		// save the trap vector index 
  r0 = (u64)&&return0187;
  goto startpretrap;
return0187:
  t12 = Type_Locative;
  *(u32 *)(iSP + 8) = t11;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t12;
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

force_alignment24910:
  if (_trace) printf("force_alignment24910:\n");
  if (t3 == 0) 
    goto basic_dispatch24906;
  /* Here if argument TrapReasonHighPrioritySequenceBreak */
  t4 = ((u64)t4 <= (u64)TrapMode_ExtraStack) ? 1 : 0;   		// Only interrupts EXTRA-STACK and EMULATOR 
  if (t4 == 0) 
    goto continuecurrentinstruction;
  goto highprioritysequencebreak;

basic_dispatch24906:
  if (_trace) printf("basic_dispatch24906:\n");
  t3 = (r0 == TrapReason_LowPrioritySequenceBreak) ? 1 : 0;   

force_alignment24911:
  if (_trace) printf("force_alignment24911:\n");
  if (t3 == 0) 
    goto basic_dispatch24907;
  /* Here if argument TrapReasonLowPrioritySequenceBreak */
  if (t4 != 0)   		// Only interrupts EMULATOR 
    goto continuecurrentinstruction;
  goto lowprioritysequencebreak;

basic_dispatch24907:
  if (_trace) printf("basic_dispatch24907:\n");
  /* Here for all other cases */
  /* Check for preempt-request trap */
  t5 = *(s32 *)&processor->interruptreg;   		// Get the preempt-pending bit 
  if (t4 != 0)   		// Don't take preempt trap unless in emulator mode 
    goto continuecurrentinstruction;
  if ((t5 & 1) == 0)   		// Jump if preempt request not pending 
    goto continuecurrentinstruction;
  goto preemptrequesttrap;

basic_dispatch24905:
  if (_trace) printf("basic_dispatch24905:\n");

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
		/* Clear the request flag */
  *(u32 *)&processor->please_stop = zero;
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

vma_memory_read24913:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read24915;

vma_memory_read24914:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read24917;

vma_memory_read24924:
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto fetch_double_float_internal24912;
  *((u32 *)(&processor->fp0)+1) = arg6;
  arg2 = arg2 + 1;
  /* Memory Read Internal */

vma_memory_read24925:
  t7 = arg2 + ivory;
  arg6 = (t7 * 4);   
  arg5 = LDQ_U(t7);   
  t5 = arg2 - t11;   		// Stack cache offset 
  t8 = *(u64 *)&(processor->dataread_mask);   
  t6 = ((u64)t5 < (u64)t12) ? 1 : 0;   		// In range? 
  arg6 = *(s32 *)arg6;   
  arg5 = (u8)(arg5 >> ((t7&7)*8));   
  if (t6 != 0)   
    goto vma_memory_read24927;

vma_memory_read24926:
  t7 = zero + 240;   
  t8 = t8 >> (arg5 & 63);   
  t7 = t7 >> (arg5 & 63);   
  if (t8 & 1)   
    goto vma_memory_read24929;

vma_memory_read24936:
  t5 = arg5 - Type_Fixnum;   
  t5 = t5 & 63;		// Strip CDR code 
  if (t5 != 0)   
    goto fetch_double_float_internal24912;
  *(u32 *)&processor->fp0 = arg6;
  sp = sp + 8;   
  goto *r0; /* ret */

vma_memory_read24929:
  if (_trace) printf("vma_memory_read24929:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read24928;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read24925;   

vma_memory_read24928:
  if (_trace) printf("vma_memory_read24928:\n");

vma_memory_read24927:
  if (_trace) printf("vma_memory_read24927:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0188;
  goto memoryreaddatadecode;
return0188:
  r0 = *(u64 *)sp;   
  goto vma_memory_read24936;   

vma_memory_read24917:
  if (_trace) printf("vma_memory_read24917:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read24916;
  arg2 = (u32)arg6;   		// Do the indirect thing 
  goto vma_memory_read24913;   

vma_memory_read24916:
  if (_trace) printf("vma_memory_read24916:\n");

vma_memory_read24915:
  if (_trace) printf("vma_memory_read24915:\n");
  *(u64 *)sp = r0;   
  r0 = (u64)&&return0189;
  goto memoryreaddatadecode;
return0189:
  r0 = *(u64 *)sp;   
  goto vma_memory_read24924;   

fetch_double_float_internal24912:
  if (_trace) printf("fetch_double_float_internal24912:\n");
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
    goto cons_double_float_internal24937;
  t7 = t6 - 2;   		// Effectively an unsigned 32-bit compare 
  if ((s64)t7 < 0)   		// Insufficient cache 
    goto cons_double_float_internal24937;
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
		/* Store remaining length */
  *(u32 *)&processor->lclength = t7;
  t8 = (u32)arg2;   
  t8 = t8 + 2;		// Increment address 
		/* Store updated address */
  *(u32 *)&processor->lcaddress = t8;
  arg2 = (u32)arg2;   
  t9 = Type_Fixnum;
  t9 = t9 | 128;
  t5 = arg2 + ivory;
  t8 = (t5 * 4);   
  t7 = LDQ_U(t5);   
  t6 = (t9 & 0xff) << ((t5&7)*8);   
  t7 = t7 & ~(0xffL << (t5&7)*8);   

force_alignment24938:
  if (_trace) printf("force_alignment24938:\n");
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

force_alignment24939:
  if (_trace) printf("force_alignment24939:\n");
  t7 = t7 | t6;
  STQ_U(t5, t7);   
  *(u32 *)t8 = arg6;
  sp = sp + 8;   
  goto *r0; /* ret */

cons_double_float_internal24937:
  if (_trace) printf("cons_double_float_internal24937:\n");
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
    goto mondo_dispatch24941;
  /* Here if argument DoubleFloatOpAdd */
  ADDT(1, f1, 1, f1, 2, f2); /* addt */   
  goto mondo_dispatch24940;   

mondo_dispatch24941:
  if (_trace) printf("mondo_dispatch24941:\n");
  t3 = zero + DoubleFloatOp_Sub;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch24942;
  /* Here if argument DoubleFloatOpSub */
  SUBT(1, f1, 1, f1, 2, f2);   
  goto mondo_dispatch24940;   

mondo_dispatch24942:
  if (_trace) printf("mondo_dispatch24942:\n");
  t3 = zero + DoubleFloatOp_Multiply;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch24943;
  /* Here if argument DoubleFloatOpMultiply */
  MULT(1, f1, 1, f1, 2, f2);   
  goto mondo_dispatch24940;   

mondo_dispatch24943:
  if (_trace) printf("mondo_dispatch24943:\n");
  t3 = zero + DoubleFloatOp_Divide;   
  t3 = t1 - t3;   
  if (t3 != 0)   
    goto mondo_dispatch24944;
  /* Here if argument DoubleFloatOpDivide */
  DIVT(1, f1, 1, f1, 2, f2);   
  goto mondo_dispatch24940;   

mondo_dispatch24944:
  if (_trace) printf("mondo_dispatch24944:\n");

mondo_dispatch24940:
  if (_trace) printf("mondo_dispatch24940:\n");
  /* trapb force the trap to occur here */   		// Force the trap to occur here 
  t3 = *(u64 *)&(processor->niladdress);   		// There was no FP exception 

doublefloatmerge:
  STT( (u64 *)&processor->fp0, 1, f1 );   
  t1 = *(s32 *)&processor->fp0;   
  t2 = *((s32 *)(&processor->fp0)+1);   
  iSP = iSP - 32;   		// Pop all the operands 
  t4 = Type_Fixnum;
		/* Push high result */
  *(u32 *)(iSP + 8) = t2;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
  iSP = iSP + 8;
  t4 = Type_Fixnum;
		/* Push low result */
  *(u32 *)(iSP + 8) = t1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t4;
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

force_alignment24969:
  if (_trace) printf("force_alignment24969:\n");
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

force_alignment24964:
  if (_trace) printf("force_alignment24964:\n");
  if (t2 != 0)   
    goto basic_dispatch24960;
  t2 = (t1 == Type_ElementForward) ? 1 : 0;   

force_alignment24965:
  if (_trace) printf("force_alignment24965:\n");
  if (t2 != 0)   
    goto basic_dispatch24960;
  t2 = (t1 == Type_HeaderForward) ? 1 : 0;   

force_alignment24966:
  if (_trace) printf("force_alignment24966:\n");
  if (t2 != 0)   
    goto basic_dispatch24960;
  t2 = (t1 == Type_ExternalValueCellPointer) ? 1 : 0;   

force_alignment24967:
  if (_trace) printf("force_alignment24967:\n");
  if (t2 == 0) 
    goto basic_dispatch24947;

basic_dispatch24960:
  if (_trace) printf("basic_dispatch24960:\n");
  /* Here if argument (TypeOneQForward TypeElementForward TypeHeaderForward
                  TypeExternalValueCellPointer) */
  /* Memory Read Internal */

vma_memory_read24948:
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
    goto vma_memory_read24950;

vma_memory_read24949:
  t7 = zero + 240;   
  t8 = t8 >> (t4 & 63);   
  t7 = t7 >> (t4 & 63);   
  if (t8 & 1)   
    goto vma_memory_read24952;

vma_memory_read24959:
  t5 = t4 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = t3;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch24947:
  if (_trace) printf("basic_dispatch24947:\n");
  t2 = (t1 == Type_LogicVariable) ? 1 : 0;   

force_alignment24968:
  if (_trace) printf("force_alignment24968:\n");
  if (t2 == 0) 
    goto basic_dispatch24961;
  /* Here if argument TypeLogicVariable */
  t5 = Type_ExternalValueCellPointer;
  *(u32 *)(iSP + 8) = arg1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch24961:
  if (_trace) printf("basic_dispatch24961:\n");
  /* Here for all other cases */
  t5 = arg2 & 63;		// set CDR-NEXT 
  *(u32 *)(iSP + 8) = arg1;
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t5;
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch24946:
  if (_trace) printf("basic_dispatch24946:\n");

vma_memory_read24950:
  if (_trace) printf("vma_memory_read24950:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t5 = (t5 * 8) + t6;  		// reconstruct SCA 
  t3 = *(s32 *)t5;   
  t4 = *(s32 *)(t5 + 4);   		// Read from stack cache 
  goto vma_memory_read24949;   

vma_memory_read24952:
  if (_trace) printf("vma_memory_read24952:\n");
  if ((t7 & 1) == 0)   
    goto vma_memory_read24951;
  arg1 = (u32)t3;   		// Do the indirect thing 
  goto vma_memory_read24948;   

vma_memory_read24951:
  if (_trace) printf("vma_memory_read24951:\n");
  t8 = *(u64 *)&(processor->dataread);   		// Load the memory action table for cycle 
  /* TagType. */
  t7 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = arg1;   		// stash the VMA for the (likely) trap 
  t7 = (t7 * 4) + t8;   		// Adjust for a longword load 
  t8 = *(s32 *)t7;   		// Get the memory action 

vma_memory_read24956:
  if (_trace) printf("vma_memory_read24956:\n");
  t7 = t8 & MemoryActionTransform;
  if (t7 == 0) 
    goto vma_memory_read24955;
  t4 = t4 & ~63L;
  t4 = t4 | Type_ExternalValueCellPointer;
  goto vma_memory_read24959;   

vma_memory_read24955:

vma_memory_read24954:
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

force_alignment24970:
  if (_trace) printf("force_alignment24970:\n");
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = arg6;
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

force_alignment24987:
  if (_trace) printf("force_alignment24987:\n");
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
		/* write the stack cache */
  *(u32 *)(iSP + 12) = t3;
  iSP = iSP + 8;
  /* Memory Read Internal */

vma_memory_read24972:
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
    goto vma_memory_read24974;

vma_memory_read24973:
  t8 = zero + 240;   
  t9 = t9 >> (t4 & 63);   
  t8 = t8 >> (t4 & 63);   
  if (t9 & 1)   
    goto vma_memory_read24976;

vma_memory_read24982:
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

force_alignment24985:
  if (_trace) printf("force_alignment24985:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);   
  *(u32 *)t5 = t1;
  if (t9 != 0)   		// J. if in cache 
    goto vma_memory_write24984;

vma_memory_write24983:
  t2 = t1 + 1;		// Increment the structure-stack-pointer 
		/* Set the structure stack pointer */
  *(u32 *)&processor->bar2 = t2;
  goto NEXTINSTRUCTION;   

vma_memory_write24984:
  if (_trace) printf("vma_memory_write24984:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);   

force_alignment24986:
  if (_trace) printf("force_alignment24986:\n");
  t6 = *(u64 *)&(processor->stackcachedata);   
  t7 = t1 - t7;   		// Stack cache offset 
  t6 = (t7 * 8) + t6;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t6 = t1;
		/* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto vma_memory_write24983;   

vma_memory_read24974:
  if (_trace) printf("vma_memory_read24974:\n");
  t7 = *(u64 *)&(processor->stackcachedata);   
  t6 = (t6 * 8) + t7;  		// reconstruct SCA 
  t5 = *(s32 *)t6;   
  t4 = *(s32 *)(t6 + 4);   		// Read from stack cache 
  goto vma_memory_read24973;   

vma_memory_read24976:
  if (_trace) printf("vma_memory_read24976:\n");
  if ((t8 & 1) == 0)   
    goto vma_memory_read24975;
  t1 = (u32)t5;   		// Do the indirect thing 
  goto vma_memory_read24972;   

vma_memory_read24975:
  if (_trace) printf("vma_memory_read24975:\n");
  t9 = *(u64 *)&(processor->datawrite);   		// Load the memory action table for cycle 
  /* TagType. */
  t8 = t4 & 63;		// Discard the CDR code 
  *(u64 *)&processor->vma = t1;   		// stash the VMA for the (likely) trap 
  t8 = (t8 * 4) + t9;   		// Adjust for a longword load 
  t9 = *(s32 *)t8;   		// Get the memory action 

vma_memory_read24979:

vma_memory_read24978:
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

force_alignment24997:
  if (_trace) printf("force_alignment24997:\n");
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

force_alignment24994:
  if (_trace) printf("force_alignment24994:\n");
  if (t2 == 0) 
    goto basic_dispatch24989;
  /* Here if argument TypeList */
  t3 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t3;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch24989:
  if (_trace) printf("basic_dispatch24989:\n");
  t2 = (t1 == Type_ExternalValueCellPointer) ? 1 : 0;   

force_alignment24995:
  if (_trace) printf("force_alignment24995:\n");
  if (t2 == 0) 
    goto basic_dispatch24990;
  /* Here if argument TypeExternalValueCellPointer */
  t3 = *(u64 *)&(processor->taddress);   
  *(u64 *)(iSP + 8) = t3;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch24990:
  if (_trace) printf("basic_dispatch24990:\n");
  t2 = (t1 == Type_ListInstance) ? 1 : 0;   

force_alignment24996:
  if (_trace) printf("force_alignment24996:\n");
  if (t2 == 0) 
    goto basic_dispatch24991;
  /* Here if argument TypeListInstance */
  t3 = *(u64 *)&(processor->niladdress);   
  *(u64 *)(iSP + 8) = t3;   		// push the data 
  iSP = iSP + 8;
  goto NEXTINSTRUCTION;   

basic_dispatch24991:
  if (_trace) printf("basic_dispatch24991:\n");
  /* Here for all other cases */
  arg6 = t2;		// arg6 = tag to dispatch on 
  arg3 = 0;		// arg3 = stackp 
  arg1 = 1;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto exception;

basic_dispatch24988:
  if (_trace) printf("basic_dispatch24988:\n");

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
  r0 = (u64)&&return0190;
  goto carinternal;
return0190:
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
  r0 = (u64)&&return0191;
  goto cdrinternal;
return0191:
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
  r0 = (u64)&&return0192;
  goto carcdrinternal;
return0192:
  r0 = *(u64 *)sp;   
  *(u64 *)&processor->linkage = zero;   
  sp = sp + 8;   
  goto *r0; /* ret */

/* end CarCdrSubroutine */



/* End of file automatically generated from ../alpha-emulator/ifuntran.as */


void SpinWheels()
{
    int i;
    for (i = 0; i < 0x2000000; i++)
        ;
}

#include "blanks.c"
