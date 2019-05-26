#include "../std.h"

#include "aihead.h"
#include "ivoryrep.h"
#include "../life-support/embed.h"

static int first_call = 1;

int iInterpret(PROCESSORSTATEP ivory)
{

    FILE *log_fd = fopen(log_file_genera, "a");

    fprintf(log_fd, "iInterpret\n");
    fflush(log_fd);
    printf("[iInterpret]\n");

    if (first_call) {
        first_call = 0;
        return (HaltReason_Halted);
    } else {
        sleep(1);
        return (HaltReason_SpyCalled);
    }

fclose(log_fd);
}

void SpinWheels()
{
    int i;
    for (i = 0; i < 0x2000000; i++) {
        ;
    }
}

void ARITHMETICEXCEPTION() {}
void DECODEFAULT() {}

void CarCdrSubroutine() {}
void CarSubroutine() {}
void CdrSubroutine() {}
void Do32BitDifferenceFP() {}
void Do32BitDifferenceIM() {}
void Do32BitDifferenceLP() {}
void Do32BitDifferenceSP() {}
void Do32BitPlusFP() {}
void Do32BitPlusIM() {}
void Do32BitPlusLP() {}
void Do32BitPlusSP() {}
void DoAddBignumStepFP() {}
void DoAddBignumStepIM() {}
void DoAddBignumStepLP() {}
void DoAddBignumStepSP() {}
void DoAddFP() {}
void DoAddIM() {}
void DoAddLP() {}
void DoAddSP() {}
void DoAllocateListBlockFP() {}
void DoAllocateListBlockIM() {}
void DoAllocateListBlockLP() {}
void DoAllocateListBlockSP() {}
void DoAllocateStructureBlockFP() {}
void DoAllocateStructureBlockIM() {}
void DoAllocateStructureBlockLP() {}
void DoAllocateStructureBlockSP() {}
void DoAloc1FP() {}
void DoAloc1IM() {}
void DoAloc1LP() {}
void DoAloc1SP() {}
void DoAlocLeaderFP() {}
void DoAlocLeaderIM() {}
void DoAlocLeaderLP() {}
void DoAlocLeaderSP() {}
void DoAluFP() {}
void DoAluIM() {}
void DoAluLP() {}
void DoAluSP() {}
void DoAref1FP() {}
void DoAref1IM() {}
void DoAref1LP() {}
void DoAref1SP() {}
void DoArrayLeaderFP() {}
void DoArrayLeaderIM() {}
void DoArrayLeaderLP() {}
void DoArrayLeaderSP() {}
void DoAset1FP() {}
void DoAset1IM() {}
void DoAset1LP() {}
void DoAset1SP() {}
void DoAshFP() {}
void DoAshIM() {}
void DoAshLP() {}
void DoAshSP() {}
void DoAssocFP() {}
void DoAssocIM() {}
void DoAssocLP() {}
void DoAssocSP() {}
void DoBindLocativeFP() {}
void DoBindLocativeIM() {}
void DoBindLocativeLP() {}
void DoBindLocativeSP() {}
void DoBindLocativeToValueFP() {}
void DoBindLocativeToValueIM() {}
void DoBindLocativeToValueLP() {}
void DoBindLocativeToValueSP() {}
void DoBlock0ReadAluFP() {}
void DoBlock0ReadAluIM() {}
void DoBlock0ReadAluLP() {}
void DoBlock0ReadAluSP() {}
void DoBlock0ReadFP() {}
void DoBlock0ReadIM() {}
void DoBlock0ReadLP() {}
void DoBlock0ReadSP() {}
void DoBlock0ReadShiftFP() {}
void DoBlock0ReadShiftIM() {}
void DoBlock0ReadShiftLP() {}
void DoBlock0ReadShiftSP() {}
void DoBlock0ReadTestFP() {}
void DoBlock0ReadTestIM() {}
void DoBlock0ReadTestLP() {}
void DoBlock0ReadTestSP() {}
void DoBlock0WriteFP() {}
void DoBlock0WriteIM() {}
void DoBlock0WriteLP() {}
void DoBlock0WriteSP() {}
void DoBlock1ReadAluFP() {}
void DoBlock1ReadAluIM() {}
void DoBlock1ReadAluLP() {}
void DoBlock1ReadAluSP() {}
void DoBlock1ReadFP() {}
void DoBlock1ReadIM() {}
void DoBlock1ReadLP() {}
void DoBlock1ReadSP() {}
void DoBlock1ReadShiftFP() {}
void DoBlock1ReadShiftIM() {}
void DoBlock1ReadShiftLP() {}
void DoBlock1ReadShiftSP() {}
void DoBlock1ReadTestFP() {}
void DoBlock1ReadTestIM() {}
void DoBlock1ReadTestLP() {}
void DoBlock1ReadTestSP() {}
void DoBlock1WriteFP() {}
void DoBlock1WriteIM() {}
void DoBlock1WriteLP() {}
void DoBlock1WriteSP() {}
void DoBlock2ReadAluFP() {}
void DoBlock2ReadAluIM() {}
void DoBlock2ReadAluLP() {}
void DoBlock2ReadAluSP() {}
void DoBlock2ReadFP() {}
void DoBlock2ReadIM() {}
void DoBlock2ReadLP() {}
void DoBlock2ReadSP() {}
void DoBlock2ReadShiftFP() {}
void DoBlock2ReadShiftIM() {}
void DoBlock2ReadShiftLP() {}
void DoBlock2ReadShiftSP() {}
void DoBlock2ReadTestFP() {}
void DoBlock2ReadTestIM() {}
void DoBlock2ReadTestLP() {}
void DoBlock2ReadTestSP() {}
void DoBlock2WriteFP() {}
void DoBlock2WriteIM() {}
void DoBlock2WriteLP() {}
void DoBlock2WriteSP() {}
void DoBlock3ReadAluFP() {}
void DoBlock3ReadAluIM() {}
void DoBlock3ReadAluLP() {}
void DoBlock3ReadAluSP() {}
void DoBlock3ReadFP() {}
void DoBlock3ReadIM() {}
void DoBlock3ReadLP() {}
void DoBlock3ReadSP() {}
void DoBlock3ReadShiftFP() {}
void DoBlock3ReadShiftIM() {}
void DoBlock3ReadShiftLP() {}
void DoBlock3ReadShiftSP() {}
void DoBlock3ReadTestFP() {}
void DoBlock3ReadTestIM() {}
void DoBlock3ReadTestLP() {}
void DoBlock3ReadTestSP() {}
void DoBlock3WriteFP() {}
void DoBlock3WriteIM() {}
void DoBlock3WriteLP() {}
void DoBlock3WriteSP() {}
void DoBranchFP() {}
void DoBranchFalseAndExtraPopFP() {}
void DoBranchFalseAndExtraPopIM() {}
void DoBranchFalseAndExtraPopLP() {}
void DoBranchFalseAndExtraPopSP() {}
void DoBranchFalseAndNoPopElseNoPopExtraPopFP() {}
void DoBranchFalseAndNoPopElseNoPopExtraPopIM() {}
void DoBranchFalseAndNoPopElseNoPopExtraPopLP() {}
void DoBranchFalseAndNoPopElseNoPopExtraPopSP() {}
void DoBranchFalseAndNoPopFP() {}
void DoBranchFalseAndNoPopIM() {}
void DoBranchFalseAndNoPopLP() {}
void DoBranchFalseAndNoPopSP() {}
void DoBranchFalseElseExtraPopFP() {}
void DoBranchFalseElseExtraPopIM() {}
void DoBranchFalseElseExtraPopLP() {}
void DoBranchFalseElseExtraPopSP() {}
void DoBranchFalseElseNoPopFP() {}
void DoBranchFalseElseNoPopIM() {}
void DoBranchFalseElseNoPopLP() {}
void DoBranchFalseElseNoPopSP() {}
void DoBranchFalseExtraPopFP() {}
void DoBranchFalseExtraPopIM() {}
void DoBranchFalseExtraPopLP() {}
void DoBranchFalseExtraPopSP() {}
void DoBranchFalseFP() {}
void DoBranchFalseIM() {}
void DoBranchFalseLP() {}
void DoBranchFalseNoPopFP() {}
void DoBranchFalseNoPopIM() {}
void DoBranchFalseNoPopLP() {}
void DoBranchFalseNoPopSP() {}
void DoBranchFalseSP() {}
void DoBranchIM() {}
void DoBranchLP() {}
void DoBranchSP() {}
void DoBranchTrueAndExtraPopFP() {}
void DoBranchTrueAndExtraPopIM() {}
void DoBranchTrueAndExtraPopLP() {}
void DoBranchTrueAndExtraPopSP() {}
void DoBranchTrueAndNoPopElseNoPopExtraPopFP() {}
void DoBranchTrueAndNoPopElseNoPopExtraPopIM() {}
void DoBranchTrueAndNoPopElseNoPopExtraPopLP() {}
void DoBranchTrueAndNoPopElseNoPopExtraPopSP() {}
void DoBranchTrueAndNoPopFP() {}
void DoBranchTrueAndNoPopIM() {}
void DoBranchTrueAndNoPopLP() {}
void DoBranchTrueAndNoPopSP() {}
void DoBranchTrueElseExtraPopFP() {}
void DoBranchTrueElseExtraPopIM() {}
void DoBranchTrueElseExtraPopLP() {}
void DoBranchTrueElseExtraPopSP() {}
void DoBranchTrueElseNoPopFP() {}
void DoBranchTrueElseNoPopIM() {}
void DoBranchTrueElseNoPopLP() {}
void DoBranchTrueElseNoPopSP() {}
void DoBranchTrueExtraPopFP() {}
void DoBranchTrueExtraPopIM() {}
void DoBranchTrueExtraPopLP() {}
void DoBranchTrueExtraPopSP() {}
void DoBranchTrueFP() {}
void DoBranchTrueIM() {}
void DoBranchTrueLP() {}
void DoBranchTrueNoPopFP() {}
void DoBranchTrueNoPopIM() {}
void DoBranchTrueNoPopLP() {}
void DoBranchTrueNoPopSP() {}
void DoBranchTrueSP() {}
void DoCarFP() {}
void DoCarIM() {}
void DoCarLP() {}
void DoCarSP() {}
void DoCatchCloseFP() {}
void DoCatchCloseIM() {}
void DoCatchCloseLP() {}
void DoCatchCloseSP() {}
void DoCatchOpenFP() {}
void DoCatchOpenIM() {}
void DoCatchOpenLP() {}
void DoCatchOpenSP() {}
void DoCdrFP() {}
void DoCdrIM() {}
void DoCdrLP() {}
void DoCdrSP() {}
void DoCeilingFP() {}
void DoCeilingIM() {}
void DoCeilingLP() {}
void DoCeilingSP() {}
void DoCharDpbFP() {}
void DoCharDpbIM() {}
void DoCharDpbLP() {}
void DoCharDpbSP() {}
void DoCharLdbFP() {}
void DoCharLdbIM() {}
void DoCharLdbLP() {}
void DoCharLdbSP() {}
void DoCheckPreemptRequestFP() {}
void DoCheckPreemptRequestIM() {}
void DoCheckPreemptRequestLP() {}
void DoCheckPreemptRequestSP() {}
void DoCoprocessorReadFP() {}
void DoCoprocessorReadIM() {}
void DoCoprocessorReadLP() {}
void DoCoprocessorReadSP() {}
void DoCoprocessorWriteFP() {}
void DoCoprocessorWriteIM() {}
void DoCoprocessorWriteLP() {}
void DoCoprocessorWriteSP() {}
void DoDecrementFP() {}
void DoDecrementIM() {}
void DoDecrementLP() {}
void DoDecrementSP() {}
void DoDereferenceFP() {}
void DoDereferenceIM() {}
void DoDereferenceLP() {}
void DoDereferenceSP() {}
void DoDivideBignumStepFP() {}
void DoDivideBignumStepIM() {}
void DoDivideBignumStepLP() {}
void DoDivideBignumStepSP() {}
void DoDoubleFloatOpFP() {}
void DoDoubleFloatOpIM() {}
void DoDoubleFloatOpLP() {}
void DoDoubleFloatOpSP() {}
void DoDpbFP() {}
void DoDpbIM() {}
void DoDpbLP() {}
void DoDpbSP() {}
void DoEndpFP() {}
void DoEndpIM() {}
void DoEndpLP() {}
void DoEndpSP() {}
void DoEntryRestAcceptedFP() {}
void DoEntryRestAcceptedIM() {}
void DoEntryRestAcceptedLP() {}
void DoEntryRestAcceptedSP() {}
void DoEntryRestNotAcceptedFP() {}
void DoEntryRestNotAcceptedIM() {}
void DoEntryRestNotAcceptedLP() {}
void DoEntryRestNotAcceptedSP() {}
void DoEphemeralpFP() {}
void DoEphemeralpIM() {}
void DoEphemeralpLP() {}
void DoEphemeralpSP() {}
void DoEqFP() {}
void DoEqIM() {}
void DoEqLP() {}
void DoEqSP() {}
void DoEqlFP() {}
void DoEqlIM() {}
void DoEqlLP() {}
void DoEqlSP() {}
void DoEqualNumberFP() {}
void DoEqualNumberIM() {}
void DoEqualNumberLP() {}
void DoEqualNumberSP() {}
void DoFastAref1FP() {}
void DoFastAref1IM() {}
void DoFastAref1LP() {}
void DoFastAref1SP() {}
void DoFastAset1FP() {}
void DoFastAset1IM() {}
void DoFastAset1LP() {}
void DoFastAset1SP() {}
void DoFinishCallNFP() {}
void DoFinishCallNIM() {}
void DoFinishCallNLP() {}
void DoFinishCallNSP() {}
void DoFinishCallTosFP() {}
void DoFinishCallTosIM() {}
void DoFinishCallTosLP() {}
void DoFinishCallTosSP() {}
void DoFloorFP() {}
void DoFloorIM() {}
void DoFloorLP() {}
void DoFloorSP() {}
void DoGenericDispatchFP() {}
void DoGenericDispatchIM() {}
void DoGenericDispatchLP() {}
void DoGenericDispatchSP() {}
void DoGreaterpFP() {}
void DoGreaterpIM() {}
void DoGreaterpLP() {}
void DoGreaterpSP() {}
void DoHaltFP() {}
void DoHaltIM() {}
void DoHaltLP() {}
void DoHaltSP() {}
void DoIStageError() {}
void DoIncrementFP() {}
void DoIncrementIM() {}
void DoIncrementLP() {}
void DoIncrementSP() {}
void DoInstanceLocFP() {}
void DoInstanceLocIM() {}
void DoInstanceLocLP() {}
void DoInstanceLocSP() {}
void DoInstanceRefFP() {}
void DoInstanceRefIM() {}
void DoInstanceRefLP() {}
void DoInstanceRefSP() {}
void DoInstanceSetFP() {}
void DoInstanceSetIM() {}
void DoInstanceSetLP() {}
void DoInstanceSetSP() {}
void DoJumpFP() {}
void DoJumpIM() {}
void DoJumpLP() {}
void DoJumpSP() {}
void DoLdbFP() {}
void DoLdbIM() {}
void DoLdbLP() {}
void DoLdbSP() {}
void DoLesspFP() {}
void DoLesspIM() {}
void DoLesspLP() {}
void DoLesspSP() {}
void DoLocateLocalsFP() {}
void DoLocateLocalsIM() {}
void DoLocateLocalsLP() {}
void DoLocateLocalsSP() {}
void DoLogandFP() {}
void DoLogandIM() {}
void DoLogandLP() {}
void DoLogandSP() {}
void DoLogicTailTestFP() {}
void DoLogicTailTestIM() {}
void DoLogicTailTestLP() {}
void DoLogicTailTestSP() {}
void DoLogiorFP() {}
void DoLogiorIM() {}
void DoLogiorLP() {}
void DoLogiorSP() {}
void DoLogtestFP() {}
void DoLogtestIM() {}
void DoLogtestLP() {}
void DoLogtestSP() {}
void DoLogxorFP() {}
void DoLogxorIM() {}
void DoLogxorLP() {}
void DoLogxorSP() {}
void DoLoopDecrementTosFP() {}
void DoLoopDecrementTosIM() {}
void DoLoopDecrementTosLP() {}
void DoLoopDecrementTosSP() {}
void DoLoopIncrementTosLessThanFP() {}
void DoLoopIncrementTosLessThanIM() {}
void DoLoopIncrementTosLessThanLP() {}
void DoLoopIncrementTosLessThanSP() {}
void DoLshFP() {}
void DoLshIM() {}
void DoLshLP() {}
void DoLshSP() {}
void DoLshcBignumStepFP() {}
void DoLshcBignumStepIM() {}
void DoLshcBignumStepLP() {}
void DoLshcBignumStepSP() {}
void DoMaxFP() {}
void DoMaxIM() {}
void DoMaxLP() {}
void DoMaxSP() {}
void DoMemberFP() {}
void DoMemberIM() {}
void DoMemberLP() {}
void DoMemberSP() {}
void DoMemoryReadFP() {}
void DoMemoryReadIM() {}
void DoMemoryReadLP() {}
void DoMemoryReadSP() {}
void DoMemoryWriteFP() {}
void DoMemoryWriteIM() {}
void DoMemoryWriteLP() {}
void DoMemoryWriteSP() {}
void DoMergeCdrNoPopFP() {}
void DoMergeCdrNoPopIM() {}
void DoMergeCdrNoPopLP() {}
void DoMergeCdrNoPopSP() {}
void DoMessageDispatchFP() {}
void DoMessageDispatchIM() {}
void DoMessageDispatchLP() {}
void DoMessageDispatchSP() {}
void DoMinFP() {}
void DoMinIM() {}
void DoMinLP() {}
void DoMinSP() {}
void DoMinuspFP() {}
void DoMinuspIM() {}
void DoMinuspLP() {}
void DoMinuspSP() {}
void DoMovemFP() {}
void DoMovemIM() {}
void DoMovemInstanceVariableFP() {}
void DoMovemInstanceVariableIM() {}
void DoMovemInstanceVariableLP() {}
void DoMovemInstanceVariableOrderedFP() {}
void DoMovemInstanceVariableOrderedIM() {}
void DoMovemInstanceVariableOrderedLP() {}
void DoMovemInstanceVariableOrderedSP() {}
void DoMovemInstanceVariableSP() {}
void DoMovemLP() {}
void DoMovemLexicalVarNFP() {}
void DoMovemLexicalVarNIM() {}
void DoMovemLexicalVarNLP() {}
void DoMovemLexicalVarNSP() {}
void DoMovemSP() {}
void DoMultiplyBignumStepFP() {}
void DoMultiplyBignumStepIM() {}
void DoMultiplyBignumStepLP() {}
void DoMultiplyBignumStepSP() {}
void DoMultiplyDoubleFP() {}
void DoMultiplyDoubleIM() {}
void DoMultiplyDoubleLP() {}
void DoMultiplyDoubleSP() {}
void DoMultiplyFP() {}
void DoMultiplyIM() {}
void DoMultiplyLP() {}
void DoMultiplySP() {}
void DoNoOpFP() {}
void DoNoOpIM() {}
void DoNoOpLP() {}
void DoNoOpSP() {}
void DoPDpbFP() {}
void DoPDpbIM() {}
void DoPDpbLP() {}
void DoPDpbSP() {}
void DoPLdbFP() {}
void DoPLdbIM() {}
void DoPLdbLP() {}
void DoPLdbSP() {}
void DoPStoreContentsFP() {}
void DoPStoreContentsIM() {}
void DoPStoreContentsLP() {}
void DoPStoreContentsSP() {}
void DoPTagDpbFP() {}
void DoPTagDpbIM() {}
void DoPTagDpbLP() {}
void DoPTagDpbSP() {}
void DoPTagLdbFP() {}
void DoPTagLdbIM() {}
void DoPTagLdbLP() {}
void DoPTagLdbSP() {}
void DoPluspFP() {}
void DoPluspIM() {}
void DoPluspLP() {}
void DoPluspSP() {}
void DoPointerDifferenceFP() {}
void DoPointerDifferenceIM() {}
void DoPointerDifferenceLP() {}
void DoPointerDifferenceSP() {}
void DoPointerIncrementFP() {}
void DoPointerIncrementIM() {}
void DoPointerIncrementLP() {}
void DoPointerIncrementSP() {}
void DoPointerPlusFP() {}
void DoPointerPlusIM() {}
void DoPointerPlusLP() {}
void DoPointerPlusSP() {}
void DoPopFP() {}
void DoPopIM() {}
void DoPopInstanceVariableFP() {}
void DoPopInstanceVariableIM() {}
void DoPopInstanceVariableLP() {}
void DoPopInstanceVariableOrderedFP() {}
void DoPopInstanceVariableOrderedIM() {}
void DoPopInstanceVariableOrderedLP() {}
void DoPopInstanceVariableOrderedSP() {}
void DoPopInstanceVariableSP() {}
void DoPopLP() {}
void DoPopLexicalVarNFP() {}
void DoPopLexicalVarNIM() {}
void DoPopLexicalVarNLP() {}
void DoPopLexicalVarNSP() {}
void DoPopSP() {}
void DoPushAddressFP() {}
void DoPushAddressIM() {}
void DoPushAddressInstanceVariableFP() {}
void DoPushAddressInstanceVariableIM() {}
void DoPushAddressInstanceVariableLP() {}
void DoPushAddressInstanceVariableOrderedFP() {}
void DoPushAddressInstanceVariableOrderedIM() {}
void DoPushAddressInstanceVariableOrderedLP() {}
void DoPushAddressInstanceVariableOrderedSP() {}
void DoPushAddressInstanceVariableSP() {}
void DoPushAddressLP() {}
void DoPushAddressSP() {}
void DoPushAddressSpRelativeFP() {}
void DoPushAddressSpRelativeIM() {}
void DoPushAddressSpRelativeLP() {}
void DoPushAddressSpRelativeSP() {}
void DoPushFP() {}
void DoPushGlobalLogicVariableFP() {}
void DoPushGlobalLogicVariableIM() {}
void DoPushGlobalLogicVariableLP() {}
void DoPushGlobalLogicVariableSP() {}
void DoPushIM() {}
void DoPushInstanceVariableFP() {}
void DoPushInstanceVariableIM() {}
void DoPushInstanceVariableLP() {}
void DoPushInstanceVariableOrderedFP() {}
void DoPushInstanceVariableOrderedIM() {}
void DoPushInstanceVariableOrderedLP() {}
void DoPushInstanceVariableOrderedSP() {}
void DoPushInstanceVariableSP() {}
void DoPushLP() {}
void DoPushLexicalVarNFP() {}
void DoPushLexicalVarNIM() {}
void DoPushLexicalVarNLP() {}
void DoPushLexicalVarNSP() {}
void DoPushLocalLogicVariablesFP() {}
void DoPushLocalLogicVariablesIM() {}
void DoPushLocalLogicVariablesLP() {}
void DoPushLocalLogicVariablesSP() {}
void DoPushNNilsFP() {}
void DoPushNNilsIM() {}
void DoPushNNilsLP() {}
void DoPushNNilsSP() {}
void DoPushSP() {}
void DoQuotientFP() {}
void DoQuotientIM() {}
void DoQuotientLP() {}
void DoQuotientSP() {}
void DoRationalQuotientFP() {}
void DoRationalQuotientIM() {}
void DoRationalQuotientLP() {}
void DoRationalQuotientSP() {}
void DoReadInternalRegisterFP() {}
void DoReadInternalRegisterIM() {}
void DoReadInternalRegisterLP() {}
void DoReadInternalRegisterSP() {}
void DoRestoreBindingStackFP() {}
void DoRestoreBindingStackIM() {}
void DoRestoreBindingStackLP() {}
void DoRestoreBindingStackSP() {}
void DoReturnKludgeFP() {}
void DoReturnKludgeIM() {}
void DoReturnKludgeLP() {}
void DoReturnKludgeSP() {}
void DoReturnMultipleFP() {}
void DoReturnMultipleIM() {}
void DoReturnMultipleLP() {}
void DoReturnMultipleSP() {}
void DoReturnSingleFP() {}
void DoReturnSingleIM() {}
void DoReturnSingleLP() {}
void DoReturnSingleSP() {}
void DoRgetfFP() {}
void DoRgetfIM() {}
void DoRgetfLP() {}
void DoRgetfSP() {}
void DoRotFP() {}
void DoRotIM() {}
void DoRotLP() {}
void DoRotSP() {}
void DoRoundFP() {}
void DoRoundIM() {}
void DoRoundLP() {}
void DoRoundSP() {}
void DoRplacaFP() {}
void DoRplacaIM() {}
void DoRplacaLP() {}
void DoRplacaSP() {}
void DoRplacdFP() {}
void DoRplacdIM() {}
void DoRplacdLP() {}
void DoRplacdSP() {}
void DoSetCdrCode1FP() {}
void DoSetCdrCode1IM() {}
void DoSetCdrCode1LP() {}
void DoSetCdrCode1SP() {}
void DoSetCdrCode2FP() {}
void DoSetCdrCode2IM() {}
void DoSetCdrCode2LP() {}
void DoSetCdrCode2SP() {}
void DoSetSpToAddressFP() {}
void DoSetSpToAddressIM() {}
void DoSetSpToAddressLP() {}
void DoSetSpToAddressSP() {}
void DoSetSpToAddressSaveTosFP() {}
void DoSetSpToAddressSaveTosIM() {}
void DoSetSpToAddressSaveTosLP() {}
void DoSetSpToAddressSaveTosSP() {}
void DoSetTagFP() {}
void DoSetTagIM() {}
void DoSetTagLP() {}
void DoSetTagSP() {}
void DoSetToCarFP() {}
void DoSetToCarIM() {}
void DoSetToCarLP() {}
void DoSetToCarSP() {}
void DoSetToCdrFP() {}
void DoSetToCdrIM() {}
void DoSetToCdrLP() {}
void DoSetToCdrPushCarFP() {}
void DoSetToCdrPushCarIM() {}
void DoSetToCdrPushCarLP() {}
void DoSetToCdrPushCarSP() {}
void DoSetToCdrSP() {}
void DoSetup1DArrayFP() {}
void DoSetup1DArrayIM() {}
void DoSetup1DArrayLP() {}
void DoSetup1DArraySP() {}
void DoSetupForce1DArrayFP() {}
void DoSetupForce1DArrayIM() {}
void DoSetupForce1DArrayLP() {}
void DoSetupForce1DArraySP() {}
void DoSpareOpFP() {}
void DoSpareOpIM() {}
void DoSpareOpLP() {}
void DoSpareOpSP() {}
void DoStackBltAddressFP() {}
void DoStackBltAddressIM() {}
void DoStackBltAddressLP() {}
void DoStackBltAddressSP() {}
void DoStackBltFP() {}
void DoStackBltIM() {}
void DoStackBltLP() {}
void DoStackBltSP() {}
void DoStartCallFP() {}
void DoStartCallIM() {}
void DoStartCallLP() {}
void DoStartCallSP() {}
void DoStoreArrayLeaderFP() {}
void DoStoreArrayLeaderIM() {}
void DoStoreArrayLeaderLP() {}
void DoStoreArrayLeaderSP() {}
void DoStoreConditionalFP() {}
void DoStoreConditionalIM() {}
void DoStoreConditionalLP() {}
void DoStoreConditionalSP() {}
void DoSubBignumStepFP() {}
void DoSubBignumStepIM() {}
void DoSubBignumStepLP() {}
void DoSubBignumStepSP() {}
void DoSubFP() {}
void DoSubIM() {}
void DoSubLP() {}
void DoSubSP() {}
void DoTagFP() {}
void DoTagIM() {}
void DoTagLP() {}
void DoTagSP() {}
void DoTakeValuesFP() {}
void DoTakeValuesIM() {}
void DoTakeValuesLP() {}
void DoTakeValuesSP() {}
void DoTruncateFP() {}
void DoTruncateIM() {}
void DoTruncateLP() {}
void DoTruncateSP() {}
void DoTypeMemberFP() {}
void DoTypeMemberIM() {}
void DoTypeMemberLP() {}
void DoTypeMemberSP() {}
void DoUnaryMinusFP() {}
void DoUnaryMinusIM() {}
void DoUnaryMinusLP() {}
void DoUnaryMinusSP() {}
void DoUnbindNFP() {}
void DoUnbindNIM() {}
void DoUnbindNLP() {}
void DoUnbindNSP() {}
void DoUnifyFP() {}
void DoUnifyIM() {}
void DoUnifyLP() {}
void DoUnifySP() {}
void DoUnsignedLesspFP() {}
void DoUnsignedLesspIM() {}
void DoUnsignedLesspLP() {}
void DoUnsignedLesspSP() {}
void DoWriteInternalRegisterFP() {}
void DoWriteInternalRegisterIM() {}
void DoWriteInternalRegisterLP() {}
void DoWriteInternalRegisterSP() {}
void DoZeropFP() {}
void DoZeropIM() {}
void DoZeropLP() {}
void DoZeropSP() {}
void ICACHEMISS() {}
void ReadRegisterAluAndRotateControl() {}
void ReadRegisterBARx() {}
void ReadRegisterBindingStackLimit() {}
void ReadRegisterBindingStackPointer() {}
void ReadRegisterCRArgumentSize() {}
void ReadRegisterCatchBlockList() {}
void ReadRegisterChipRevision() {}
void ReadRegisterChoicePointer() {}
void ReadRegisterConstantNIL() {}
void ReadRegisterConstantT() {}
void ReadRegisterContinuation() {}
void ReadRegisterControlRegister() {}
void ReadRegisterControlStackExtraLimit() {}
void ReadRegisterControlStackLimit() {}
void ReadRegisterCountMapReloads() {}
void ReadRegisterDynamicBindingCacheBase() {}
void ReadRegisterDynamicBindingCacheMask() {}
void ReadRegisterEphemeralOldspaceRegister() {}
void ReadRegisterError() {}
void ReadRegisterEventCount() {}
void ReadRegisterFEPModeTrapVectorAddress() {}
void ReadRegisterFP() {}
void ReadRegisterFPCoprocessorPresent() {}
void ReadRegisterIcacheControl() {}
void ReadRegisterLP() {}
void ReadRegisterListCacheAddress() {}
void ReadRegisterListCacheArea() {}
void ReadRegisterListCacheLength() {}
void ReadRegisterMapCacheControl() {}
void ReadRegisterMemoryControl() {}
void ReadRegisterMicrosecondClock() {}
void ReadRegisterPHTBase() {}
void ReadRegisterPHTMask() {}
void ReadRegisterPreemptRegister() {}
void ReadRegisterPrefetcherControl() {}
void ReadRegisterSP() {}
void ReadRegisterStackCacheDumpQuantum() {}
void ReadRegisterStackCacheLowerBound() {}
void ReadRegisterStackCacheOverflowLimit() {}
void ReadRegisterStackFrameMaximumSize() {}
void ReadRegisterStructureCacheAddress() {}
void ReadRegisterStructureCacheArea() {}
void ReadRegisterStructureCacheLength() {}
void ReadRegisterStructureStackChoicePointer() {}
void ReadRegisterTOS() {}
void ReadRegisterZoneOldspaceRegister() {}
void WriteRegisterAluAndRotateControl() {}
void WriteRegisterBARx() {}
void WriteRegisterBindingStackLimit() {}
void WriteRegisterBindingStackPointer() {}
void WriteRegisterCatchBlockList() {}
void WriteRegisterChoicePointer() {}
void WriteRegisterContinuation() {}
void WriteRegisterControlRegister() {}
void WriteRegisterControlStackExtraLimit() {}
void WriteRegisterControlStackLimit() {}
void WriteRegisterDynamicBindingCacheBase() {}
void WriteRegisterDynamicBindingCacheMask() {}
void WriteRegisterEphemeralOldspaceRegister() {}
void WriteRegisterError() {}
void WriteRegisterEventCount() {}
void WriteRegisterFEPModeTrapVectorAddress() {}
void WriteRegisterFP() {}
void WriteRegisterFPCoprocessorPresent() {}
void WriteRegisterLP() {}
void WriteRegisterListCacheAddress() {}
void WriteRegisterListCacheArea() {}
void WriteRegisterListCacheLength() {}
void WriteRegisterMappingTableCache() {}
void WriteRegisterPreemptRegister() {}
void WriteRegisterSP() {}
void WriteRegisterStackCacheLowerBound() {}
void WriteRegisterStackCacheOverflowLimit() {}
void WriteRegisterStructureCacheAddress() {}
void WriteRegisterStructureCacheArea() {}
void WriteRegisterStructureCacheLength() {}
void WriteRegisterStructureStackChoicePointer() {}
void WriteRegisterTOS() {}
void WriteRegisterZoneOldspaceRegister() {}
void boundlocationfw() {}
void callcompiledeven() {}
void callcompiledevenprefetch() {}
void callcompiledodd() {}
void callcompiledoddprefetch() {}
void callgeneric() {}
void callgenericprefetch() {}
void callindirect() {}
void callindirectprefetch() {}
void elementforwardfw() {}
void gcforwardfw() {}
void headerforwardfw() {}
void headerifw() {}
void headerpfw() {}
void logicvariablefw() {}
void monitorforwardfw() {}
void nativeinstruction() {}
void nullfw() {}
void oneqforwardfw() {}
void pushconstantvalue() {}
void resumeemulated() {}
void valuecell() {}
