/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/ifunloop.as
 ************************************************************************/

/* Branch and loop instructions. */
/* start DoBranchTrueElseNoPop */

/* Halfword 10 bit immediate instruction - DoBranchTrueElseNoPop */

dobranchtrueelsenopop : if (_trace) printf("dobranchtrueelsenopop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueElseNoPopIM : if (_trace) printf("DoBranchTrueElseNoPopIM:\n");

DoBranchTrueElseNoPopSP : if (_trace) printf("DoBranchTrueElseNoPopSP:\n");

DoBranchTrueElseNoPopLP : if (_trace) printf("DoBranchTrueElseNoPopLP:\n");

DoBranchTrueElseNoPopFP : if (_trace) printf("DoBranchTrueElseNoPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 == 0)
    goto NEXTINSTRUCTION;
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 8;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchTrueElseNoPop */
/* End of Halfword operand from stack instruction - DoBranchTrueElseNoPop */
/* start DoBranchTrueElseExtraPop */

/* Halfword 10 bit immediate instruction - DoBranchTrueElseExtraPop */

dobranchtrueelseextrapop : if (_trace) printf("dobranchtrueelseextrapop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueElseExtraPopIM : if (_trace) printf("DoBranchTrueElseExtraPopIM:\n");

DoBranchTrueElseExtraPopSP : if (_trace) printf("DoBranchTrueElseExtraPopSP:\n");

DoBranchTrueElseExtraPopLP : if (_trace) printf("DoBranchTrueElseExtraPopLP:\n");

DoBranchTrueElseExtraPopFP : if (_trace) printf("DoBranchTrueElseExtraPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 != 0)
    goto dobrelsepopextrapop;
/* Here if branch not taken.  Pop the argument. */
iPC = *(uint64_t *)&(((CACHELINEP)iCP)->nextpcdata);
iCP = *(uint64_t *)&(((CACHELINEP)iCP)->nextcp);
iSP = iSP - 16;
goto cachevalid;

dobrelsepopextrapop : if (_trace) printf("dobrelsepopextrapop:\n");
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 8;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchTrueElseExtraPop */
/* End of Halfword operand from stack instruction - DoBranchTrueElseExtraPop */
/* start DoBranchFalseElseExtraPop */

/* Halfword 10 bit immediate instruction - DoBranchFalseElseExtraPop */

dobranchfalseelseextrapop : if (_trace) printf("dobranchfalseelseextrapop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseElseExtraPopIM : if (_trace) printf("DoBranchFalseElseExtraPopIM:\n");

DoBranchFalseElseExtraPopSP : if (_trace) printf("DoBranchFalseElseExtraPopSP:\n");

DoBranchFalseElseExtraPopLP : if (_trace) printf("DoBranchFalseElseExtraPopLP:\n");

DoBranchFalseElseExtraPopFP : if (_trace) printf("DoBranchFalseElseExtraPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 == 0)
    goto dobrnelsepopextrapop;
/* Here if branch not taken.  Pop the argument. */
iPC = *(uint64_t *)&(((CACHELINEP)iCP)->nextpcdata);
iCP = *(uint64_t *)&(((CACHELINEP)iCP)->nextcp);
iSP = iSP - 16;
goto cachevalid;

dobrnelsepopextrapop : if (_trace) printf("dobrnelsepopextrapop:\n");
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 8;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchFalseElseExtraPop */
/* End of Halfword operand from stack instruction - DoBranchFalseElseExtraPop */
/* start DoBranchFalseExtraPop */

/* Halfword 10 bit immediate instruction - DoBranchFalseExtraPop */

dobranchfalseextrapop : if (_trace) printf("dobranchfalseextrapop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseExtraPopIM : if (_trace) printf("DoBranchFalseExtraPopIM:\n");

DoBranchFalseExtraPopSP : if (_trace) printf("DoBranchFalseExtraPopSP:\n");

DoBranchFalseExtraPopLP : if (_trace) printf("DoBranchFalseExtraPopLP:\n");

DoBranchFalseExtraPopFP : if (_trace) printf("DoBranchFalseExtraPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 == 0)
    goto dobrnpopelsepopextrapop;
/* Here if branch not taken.  Pop the argument. */
iPC = *(uint64_t *)&(((CACHELINEP)iCP)->nextpcdata);
iCP = *(uint64_t *)&(((CACHELINEP)iCP)->nextcp);
iSP = iSP - 16;
goto cachevalid;

dobrnpopelsepopextrapop : if (_trace) printf("dobrnpopelsepopextrapop:\n");
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 16;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchFalseExtraPop */
/* End of Halfword operand from stack instruction - DoBranchFalseExtraPop */
/* start DoLoopDecrementTos */

/* Halfword 10 bit immediate instruction - DoLoopDecrementTos */

doloopdecrementtos : if (_trace) printf("doloopdecrementtos:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoLoopDecrementTosIM : if (_trace) printf("DoLoopDecrementTosIM:\n");

DoLoopDecrementTosSP : if (_trace) printf("DoLoopDecrementTosSP:\n");

DoLoopDecrementTosLP : if (_trace) printf("DoLoopDecrementTosLP:\n");

DoLoopDecrementTosFP : if (_trace) printf("DoLoopDecrementTosFP:\n");
arg1 = (int64_t) arg3 >> 48;
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8));
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
t2 = (uint32_t) arg6;
t3 = t1 - Type_Fixnum;
t3 = t3 & 63; // Strip CDR code
if (t3 != 0)
    goto iloop_decrement_tos44748;
t3 = (int32_t) t2 - (int32_t) 1;
t4 = ((int64_t) t3 < (int64_t) t2) ? 1 : 0;
if (t4 == 0)
    goto iloop_decrement_tos44750;
t6 = Type_Fixnum;
*(uint32_t *)iSP = t3;
*(uint32_t *)(iSP + 4) = t6; // write the stack cache
if ((int64_t) t3 <= 0)
    goto NEXTINSTRUCTION;
/* Here if branch taken. */
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

iloop_decrement_tos44748 : if (_trace) printf("iloop_decrement_tos44748:\n");
t3 = t1 - Type_Fixnum;
t3 = t3 & 56; // Strip CDR code, low bits
if (t3 != 0)
    goto iloop_decrement_tos44749;

iloop_decrement_tos44750 : if (_trace) printf("iloop_decrement_tos44750:\n");
arg5 = iPC + arg1; // Compute next-pc
arg3 = 1; // arg3 = stackp
arg1 = 1; // arg1 = instruction arity
arg4 = 0; // arg4 = arithmeticp
goto loopexception;

iloop_decrement_tos44749 : if (_trace) printf("iloop_decrement_tos44749:\n");
arg5 = 0;
arg2 = 81;
goto illegaloperand;

/* end DoLoopDecrementTos */
/* End of Halfword operand from stack instruction - DoLoopDecrementTos */
/* start DoLoopIncrementTosLessThan */

/* Halfword 10 bit immediate instruction - DoLoopIncrementTosLessThan */

doloopincrementtoslessthan : if (_trace) printf("doloopincrementtoslessthan:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoLoopIncrementTosLessThanIM : if (_trace) printf("DoLoopIncrementTosLessThanIM:\n");

DoLoopIncrementTosLessThanSP : if (_trace) printf("DoLoopIncrementTosLessThanSP:\n");

DoLoopIncrementTosLessThanLP : if (_trace) printf("DoLoopIncrementTosLessThanLP:\n");

DoLoopIncrementTosLessThanFP : if (_trace) printf("DoLoopIncrementTosLessThanFP:\n");
arg1 = (int64_t) arg3 >> 48;
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8));
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
t2 = (uint32_t) arg6;
t5 = t1 - Type_Fixnum;
t5 = t5 & 63; // Strip CDR code
if (t5 != 0)
    goto iloop_increment_tos_less_than44751;
t4 = *(int32_t *)(iSP + -8); // Get arg1.
t3 = *(int32_t *)(iSP + -4);
t4 = (uint32_t) t4;
t5 = t3 - Type_Fixnum;
t5 = t5 & 63; // Strip CDR code
if (t5 != 0)
    goto iloop_increment_tos_less_than44752;
t5 = (int32_t) t2 + (int32_t) 1;
t6 = ((int64_t) t2 <= (int64_t) t5) ? 1 : 0;
if (t6 == 0)
    goto iloop_increment_tos_less_than44753;
t6 = Type_Fixnum;
*(uint32_t *)iSP = t5;
*(uint32_t *)(iSP + 4) = t6; // write the stack cache
t6 = ((int64_t) t5 <= (int64_t) t4) ? 1 : 0;
if (t6 == 0)
    goto NEXTINSTRUCTION;
/* Here if branch taken. */

force_alignment44755 : if (_trace) printf("force_alignment44755:\n");
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

iloop_increment_tos_less_than44751 : if (_trace) printf("iloop_increment_tos_less_than44751:\n");
t5 = t1 - Type_Fixnum;
t5 = t5 & 56; // Strip CDR code, low bits
if (t5 != 0)
    goto iloop_increment_tos_less_than44754;

iloop_increment_tos_less_than44752 : if (_trace) printf("iloop_increment_tos_less_than44752:\n");
t5 = t3 - Type_Fixnum;
t5 = t5 & 56; // Strip CDR code, low bits
if (t5 != 0)
    goto iloop_increment_tos_less_than44754;

iloop_increment_tos_less_than44753 : if (_trace) printf("iloop_increment_tos_less_than44753:\n");
arg5 = iPC + arg1; // Compute next-pc
arg3 = 1; // arg3 = stackp
arg1 = 2; // arg1 = instruction arity
arg4 = 0; // arg4 = arithmeticp
goto loopexception;

iloop_increment_tos_less_than44754 : if (_trace) printf("iloop_increment_tos_less_than44754:\n");
arg5 = 0;
arg2 = 16;
goto illegaloperand;

/* end DoLoopIncrementTosLessThan */
/* End of Halfword operand from stack instruction - DoLoopIncrementTosLessThan */
/* start DoBranchTrueExtraPop */

/* Halfword 10 bit immediate instruction - DoBranchTrueExtraPop */

dobranchtrueextrapop : if (_trace) printf("dobranchtrueextrapop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueExtraPopIM : if (_trace) printf("DoBranchTrueExtraPopIM:\n");

DoBranchTrueExtraPopSP : if (_trace) printf("DoBranchTrueExtraPopSP:\n");

DoBranchTrueExtraPopLP : if (_trace) printf("DoBranchTrueExtraPopLP:\n");

DoBranchTrueExtraPopFP : if (_trace) printf("DoBranchTrueExtraPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 != 0)
    goto dobrpopelsepopextrapop;
/* Here if branch not taken.  Pop the argument. */
iPC = *(uint64_t *)&(((CACHELINEP)iCP)->nextpcdata);
iCP = *(uint64_t *)&(((CACHELINEP)iCP)->nextcp);
iSP = iSP - 16;
goto cachevalid;

dobrpopelsepopextrapop : if (_trace) printf("dobrpopelsepopextrapop:\n");
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 16;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchTrueExtraPop */
/* End of Halfword operand from stack instruction - DoBranchTrueExtraPop */
/* start DoBranchTrueAndNoPopElseNoPopExtraPop */

/* Halfword 10 bit immediate instruction - DoBranchTrueAndNoPopElseNoPopExtraPop */

dobranchtrueandnopopelsenopopextrapop : if (_trace) printf("dobranchtrueandnopopelsenopopextrapop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchTrueAndNoPopElseNoPopExtraPopIM : if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopIM:\n");

DoBranchTrueAndNoPopElseNoPopExtraPopSP : if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopSP:\n");

DoBranchTrueAndNoPopElseNoPopExtraPopLP : if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopLP:\n");

DoBranchTrueAndNoPopElseNoPopExtraPopFP : if (_trace) printf("DoBranchTrueAndNoPopElseNoPopExtraPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 != 0)
    goto dobrextrapop;
/* Here if branch not taken.  Pop the argument. */
iPC = *(uint64_t *)&(((CACHELINEP)iCP)->nextpcdata);
iCP = *(uint64_t *)&(((CACHELINEP)iCP)->nextcp);
iSP = iSP - 8;
goto cachevalid;

dobrextrapop : if (_trace) printf("dobrextrapop:\n");
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 8;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchTrueAndNoPopElseNoPopExtraPop */
/* End of Halfword operand from stack instruction - DoBranchTrueAndNoPopElseNoPopExtraPop */
/* start DoBranchFalseAndNoPopElseNoPopExtraPop */

/* Halfword 10 bit immediate instruction - DoBranchFalseAndNoPopElseNoPopExtraPop */

dobranchfalseandnopopelsenopopextrapop : if (_trace) printf("dobranchfalseandnopopelsenopopextrapop:\n");
/* Actually only one entry point, but simulate others for dispatch */

DoBranchFalseAndNoPopElseNoPopExtraPopIM : if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopIM:\n");

DoBranchFalseAndNoPopElseNoPopExtraPopSP : if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopSP:\n");

DoBranchFalseAndNoPopElseNoPopExtraPopLP : if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopLP:\n");

DoBranchFalseAndNoPopElseNoPopExtraPopFP : if (_trace) printf("DoBranchFalseAndNoPopElseNoPopExtraPopFP:\n");
/* arg1 has signed operand preloaded. */
t1 = (uint32_t) (arg6 >> ((4 & 7) * 8)); // Check tag of word in TOS.
arg2 = *(uint64_t *)&(((CACHELINEP)iCP)->annotation);
arg1 = (int64_t) arg3 >> 48; // Get signed 10-bit immediate arg
/* TagType. */
t1 = t1 & 63; // strip the cdr code off.
t1 = t1 - Type_NIL; // Compare to NIL
if (t1 == 0)
    goto dobrnextrapop;
/* Here if branch not taken.  Pop the argument. */
iPC = *(uint64_t *)&(((CACHELINEP)iCP)->nextpcdata);
iCP = *(uint64_t *)&(((CACHELINEP)iCP)->nextcp);
iSP = iSP - 8;
goto cachevalid;

dobrnextrapop : if (_trace) printf("dobrnextrapop:\n");
if (arg1 == 0) // Can't branch to ourself
    goto branchexception;
iSP = iSP - 8;
iPC = iPC + arg1; // Update the PC in halfwords
if (arg2 != 0)
    goto interpretinstructionpredicted;
goto interpretinstructionforbranch;

/* end DoBranchFalseAndNoPopElseNoPopExtraPop */
/* End of Halfword operand from stack instruction - DoBranchFalseAndNoPopElseNoPopExtraPop */
/* start BranchException */

branchexception : if (_trace) printf("branchexception:\n");
arg5 = 0;
arg2 = 24;
goto illegaloperand;

/* end BranchException */
/* Fin. */

/* End of file automatically generated from ../alpha-emulator/ifunloop.as */
