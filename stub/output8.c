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

dosettocar : if (_trace) printf("dosettocar:\n");

DoSetToCarSP : if (_trace) printf("DoSetToCarSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 == 0) // SP-pop mode
    arg1 = iSP;
if (arg2 == 0) // Adjust SP if SP-pop mode
    iSP = arg4;

DoSetToCarLP : if (_trace) printf("DoSetToCarLP:\n");

DoSetToCarFP : if (_trace) printf("DoSetToCarFP:\n");

begindosettocar : if (_trace) printf("begindosettocar:\n");
/* arg1 has the operand address. */
arg1 = (arg2 * 8) + arg1; // Compute operand address
t11 = *(u64 *)&(processor->stackcachebasevma);
t12 = *(s32 *)&processor->scovlimit; // Size of the stack cache (words)
arg5 = *(s32 *)(arg1 + 4); // Get the operand from the stack.
arg6 = *(s32 *)arg1;
t2 = arg5 & 192; // Save the old CDR code
r0 = (u64) && return0040;
goto carinternal;
return0040 :
    /* TagType. */
    arg5
    = arg5 & 63;
arg5 = arg5 | t2; // Put back the original CDR codes
*(u32 *)arg1 = arg6;
*(u32 *)(arg1 + 4) = arg5; // write the stack cache
goto NEXTINSTRUCTION;

DoSetToCarIM : goto doistageerror;

/* end DoSetToCar */
/* End of Halfword operand from stack instruction - DoSetToCar */
/* start DoSetToCdr */

/* Halfword operand from stack instruction - DoSetToCdr */
/* arg2 has the preloaded 8 bit operand. */

dosettocdr : if (_trace) printf("dosettocdr:\n");

DoSetToCdrSP : if (_trace) printf("DoSetToCdrSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 == 0) // SP-pop mode
    arg1 = iSP;
if (arg2 == 0) // Adjust SP if SP-pop mode
    iSP = arg4;

DoSetToCdrLP : if (_trace) printf("DoSetToCdrLP:\n");

DoSetToCdrFP : if (_trace) printf("DoSetToCdrFP:\n");

begindosettocdr : if (_trace) printf("begindosettocdr:\n");
/* arg1 has the operand address. */
arg1 = (arg2 * 8) + arg1; // Compute operand address
t11 = *(u64 *)&(processor->stackcachebasevma);
t12 = *(s32 *)&processor->scovlimit; // Size of the stack cache (words)
arg5 = *(s32 *)(arg1 + 4); // Get the operand from the stack.
arg6 = *(s32 *)arg1;
t2 = arg5 & 192; // Save the old CDR code
r0 = (u64) && return0041;
goto cdrinternal;
return0041 :
    /* TagType. */
    arg5
    = arg5 & 63;
arg5 = arg5 | t2; // Put back the original CDR codes
*(u32 *)arg1 = arg6;
*(u32 *)(arg1 + 4) = arg5; // write the stack cache
goto NEXTINSTRUCTION;

DoSetToCdrIM : goto doistageerror;

/* end DoSetToCdr */
/* End of Halfword operand from stack instruction - DoSetToCdr */
/* start SetToCdrPushCarLocative */

SetToCdrPushCarLocative : if (_trace) printf("SetToCdrPushCarLocative:\n");

settocdrpushcarlocative : if (_trace) printf("settocdrpushcarlocative:\n");
arg2 = t2;
/* Memory Read Internal */

vma_memory_read44756 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(u64 *)&(processor->dataread_mask);
t6 = ((u64)t5 < (u64)t12) ? 1 : 0; // In range?
arg6 = *(s32 *)arg6;
arg5 = (u8)(arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read44758;

vma_memory_read44757 : t7 = zero + 240;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
if (t8 & 1)
    goto vma_memory_read44760;

vma_memory_read44767 :
    /* TagType. */
    t1
    = t1 & 63;
*(u32 *)(iSP + 8) = arg6;
*(u32 *)(iSP + 12) = arg5; // write the stack cache
iSP = iSP + 8;
t1 = t1 | t3; // Put back the original CDR codes
*(u32 *)arg1 = arg6;
*(u32 *)(arg1 + 4) = arg5; // write the stack cache
goto NEXTINSTRUCTION;

vma_memory_read44760 : if (_trace) printf("vma_memory_read44760:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read44759;
arg2 = (u32)arg6; // Do the indirect thing
goto vma_memory_read44756;

vma_memory_read44759 : if (_trace) printf("vma_memory_read44759:\n");

vma_memory_read44758 : if (_trace) printf("vma_memory_read44758:\n");
r0 = (u64) && return0042;
goto memoryreaddatadecode;
return0042 : goto vma_memory_read44767;

/* end SetToCdrPushCarLocative */
/* start DoAssoc */

/* Halfword operand from stack instruction - DoAssoc */
/* arg2 has the preloaded 8 bit operand. */

doassoc : if (_trace) printf("doassoc:\n");

DoAssocSP : if (_trace) printf("DoAssocSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 != 0)
    goto begindoassoc;
arg6 = *(u64 *)arg4; // SP-pop, Reload TOS
arg1 = iSP; // SP-pop mode
iSP = arg4; // Adjust SP

DoAssocLP : if (_trace) printf("DoAssocLP:\n");

DoAssocFP : if (_trace) printf("DoAssocFP:\n");

begindoassoc : if (_trace) printf("begindoassoc:\n");
/* arg1 has the operand address. */
arg1 = (arg2 * 8) + arg1; // Compute operand address
t11 = *(u64 *)&(processor->stackcachebasevma);
t12 = *(s32 *)&processor->scovlimit; // Size of the stack cache (words)
t5 = zero + -2048;
t5 = t5 + ((1) << 16);
arg3 = (u32)(arg6 >> ((4 & 7) * 8));
arg4 = (u32)arg6;
t1 = *(s32 *)(arg1 + 4);
t2 = *(s32 *)arg1;
/* TagType. */
arg3 = arg3 & 63; // Get the object type bits
t5 = t5 >> (arg3 & 63); // Low bit will set iff EQ-NOT-EQL
/* TagType. */
t1 = t1 & 63; // Strip cdr code
t2 = (u32)t2; // Remove sign-extension
if (t5 & 1)
    goto assocexc;
t6 = zero;
goto carcdrloop44769;

assoccdr : if (_trace) printf("assoccdr:\n");
t6 = *(u64 *)&(processor->stop_interpreter); // Have we been asked to stop or trap?
/* Move cdr to car for next carcdr-internal */
/* TagType. */
t1 = arg5 & 63;
t2 = arg6;

carcdrloop44769 : if (_trace) printf("carcdrloop44769:\n");
t5 = t1 - Type_NIL;
if (t6 != 0) // Asked to stop, check for sequence break
    goto carcdrloop44768;
if (t5 == 0)
    goto carcdrloop44770;
r0 = (u64) && return0043;
goto carcdrinternal;
return0043 : t7 = t1 & 63; // Strip off any CDR code bits.
t8 = (t7 == Type_List) ? 1 : 0;

force_alignment44788 : if (_trace) printf("force_alignment44788:\n");
if (t8 == 0)
    goto basic_dispatch44772;
/* Here if argument TypeList */
arg2 = t2;
t3 = arg5;
arg1 = arg6;
/* Memory Read Internal */

vma_memory_read44773 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(u64 *)&(processor->dataread_mask);
t6 = ((u64)t5 < (u64)t12) ? 1 : 0; // In range?
arg6 = *(s32 *)arg6;
arg5 = (u8)(arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read44775;

vma_memory_read44774 : t7 = zero + 240;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
if (t8 & 1)
    goto vma_memory_read44777;

vma_memory_read44784 :
    /* TagType. */
    t5
    = arg5 & 63;
arg5 = t3;
t6 = (s32)arg4 - (s32)arg6; // t6=0 if data same
arg6 = arg1;
if (t6 != 0) // J. if different
    goto assoccdr;
t5 = arg3 - t5; // t5 zero if same tag
if (t5 != 0) // J. if tags different
    goto assoccdr;
/* we found a match! */
/* TagType. */
t1 = t1 & 63;
*(u32 *)iSP = t2;
*(u32 *)(iSP + 4) = t1; // write the stack cache
goto NEXTINSTRUCTION;

basic_dispatch44772 : if (_trace) printf("basic_dispatch44772:\n");
t8 = (t7 == Type_NIL) ? 1 : 0;

force_alignment44789 : if (_trace) printf("force_alignment44789:\n");
if (t8 == 0)
    goto basic_dispatch44785;
/* Here if argument TypeNIL */
goto assoccdr;

basic_dispatch44785 : if (_trace) printf("basic_dispatch44785:\n");
/* Here for all other cases */
/* SetTag. */
t1 = arg4 << 32;
t1 = arg5 | t1;
arg5 = t1;
arg2 = 14;
goto illegaloperand;

basic_dispatch44771 : if (_trace) printf("basic_dispatch44771:\n");

carcdrloop44770 : if (_trace) printf("carcdrloop44770:\n");
t1 = *(u64 *)&(processor->niladdress); // Return NIL
*(u64 *)iSP = t1; // push the data
goto NEXTINSTRUCTION;

assocexc : if (_trace) printf("assocexc:\n");
arg3 = 0; // arg3 = stackp
arg1 = 2; // arg1 = instruction arity
arg4 = 0; // arg4 = arithmeticp
goto exception;

vma_memory_read44777 : if (_trace) printf("vma_memory_read44777:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read44776;
arg2 = (u32)arg6; // Do the indirect thing
goto vma_memory_read44773;

vma_memory_read44776 : if (_trace) printf("vma_memory_read44776:\n");

vma_memory_read44775 : if (_trace) printf("vma_memory_read44775:\n");
r0 = (u64) && return0044;
goto memoryreaddatadecode;
return0044 : goto vma_memory_read44784;

carcdrloop44768 : if (_trace) printf("carcdrloop44768:\n");
iSP = *(u64 *)&(processor->restartsp);
goto INTERPRETINSTRUCTION;

DoAssocIM : goto doistageerror;

/* end DoAssoc */
/* End of Halfword operand from stack instruction - DoAssoc */
/* start DoMember */

/* Halfword operand from stack instruction - DoMember */
/* arg2 has the preloaded 8 bit operand. */

domember : if (_trace) printf("domember:\n");

DoMemberSP : if (_trace) printf("DoMemberSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 != 0)
    goto begindomember;
arg6 = *(u64 *)arg4; // SP-pop, Reload TOS
arg1 = iSP; // SP-pop mode
iSP = arg4; // Adjust SP

DoMemberLP : if (_trace) printf("DoMemberLP:\n");

DoMemberFP : if (_trace) printf("DoMemberFP:\n");

begindomember : if (_trace) printf("begindomember:\n");
/* arg1 has the operand address. */
arg1 = (arg2 * 8) + arg1; // Compute operand address
t11 = *(u64 *)&(processor->stackcachebasevma);
t12 = *(s32 *)&processor->scovlimit; // Size of the stack cache (words)
t5 = zero + -2048;
t5 = t5 + ((1) << 16);
arg3 = (u32)(arg6 >> ((4 & 7) * 8));
arg4 = (u32)arg6;
t1 = *(s32 *)(arg1 + 4);
t2 = *(s32 *)arg1;
/* TagType. */
arg3 = arg3 & 63; // Get the object type bits
t5 = t5 >> (arg3 & 63); // Low bit will set iff EQ-NOT-EQL
/* TagType. */
t1 = t1 & 63; // Strip cdr code
t2 = (u32)t2; // Remove sign-extension
if (t5 & 1)
    goto memberexc;
t6 = zero;
goto carcdrloop44791;

membercdr : if (_trace) printf("membercdr:\n");
t6 = *(u64 *)&(processor->stop_interpreter); // Have we been asked to stop or trap?
/* Move cdr to car for next carcdr-internal */
/* TagType. */
t1 = arg5 & 63;
t2 = arg6;

carcdrloop44791 : if (_trace) printf("carcdrloop44791:\n");
/* TagType. */
t3 = t1 & 63;
arg1 = t2;
t5 = t1 - Type_NIL;
if (t6 != 0) // Asked to stop, check for sequence break
    goto carcdrloop44790;
if (t5 == 0)
    goto carcdrloop44792;
r0 = (u64) && return0045;
goto carcdrinternal;
return0045 :
    /* TagType. */
    t5
    = t1 & 63;
t7 = arg4 - t2; // t7=0 if data same
if (t7 != 0) // J. if different
    goto membercdr;
t6 = arg3 - t5; // t6 zero if same tag
if (t6 != 0) // J. if tags different
    goto membercdr;
/* we found a match! */
*(u32 *)iSP = arg1;
*(u32 *)(iSP + 4) = t3; // write the stack cache
goto NEXTINSTRUCTION;

carcdrloop44792 : if (_trace) printf("carcdrloop44792:\n");
t1 = *(u64 *)&(processor->niladdress); // Return NIL
*(u64 *)iSP = t1; // push the data
goto NEXTINSTRUCTION;

memberexc : if (_trace) printf("memberexc:\n");
arg3 = 0; // arg3 = stackp
arg1 = 2; // arg1 = instruction arity
arg4 = 0; // arg4 = arithmeticp
goto exception;

carcdrloop44790 : if (_trace) printf("carcdrloop44790:\n");
iSP = *(u64 *)&(processor->restartsp);
goto INTERPRETINSTRUCTION;

DoMemberIM : goto doistageerror;

/* end DoMember */
/* End of Halfword operand from stack instruction - DoMember */
/* start DoRgetf */

/* Halfword operand from stack instruction - DoRgetf */
/* arg2 has the preloaded 8 bit operand. */

dorgetf : if (_trace) printf("dorgetf:\n");

DoRgetfSP : if (_trace) printf("DoRgetfSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 != 0)
    goto begindorgetf;
arg6 = *(u64 *)arg4; // SP-pop, Reload TOS
arg1 = iSP; // SP-pop mode
iSP = arg4; // Adjust SP

DoRgetfLP : if (_trace) printf("DoRgetfLP:\n");

DoRgetfFP : if (_trace) printf("DoRgetfFP:\n");

begindorgetf : if (_trace) printf("begindorgetf:\n");
/* arg1 has the operand address. */
arg1 = (arg2 * 8) + arg1; // Compute operand address
t11 = *(u64 *)&(processor->stackcachebasevma);
t12 = *(s32 *)&processor->scovlimit; // Size of the stack cache (words)
t5 = zero + -2048;
t5 = t5 + ((1) << 16);
arg3 = (u32)(arg6 >> ((4 & 7) * 8));
arg4 = (u32)arg6;
t1 = *(s32 *)(arg1 + 4);
t2 = *(s32 *)arg1;
/* TagType. */
arg3 = arg3 & 63; // Get the object type bits
t5 = t5 >> (arg3 & 63); // Low bit will set iff EQ-NOT-EQL
/* TagType. */
t1 = t1 & 63; // Strip cdr code
t2 = (u32)t2; // Remove sign-extension
if (t5 & 1)
    goto rgetfexc;
t6 = zero;
goto carcdrloop44794;

rgetfcdr : if (_trace) printf("rgetfcdr:\n");
r0 = (u64) && return0046;
goto cdrinternal;
return0046 : t6 = *(u64 *)&(processor->stop_interpreter); // Have we been asked to stop or trap?
/* Move cdr to car for next carcdr-internal */
/* TagType. */
t1 = arg5 & 63;
t2 = arg6;

carcdrloop44794 : if (_trace) printf("carcdrloop44794:\n");
t5 = t1 - Type_NIL;
if (t6 != 0) // Asked to stop, check for sequence break
    goto carcdrloop44793;
if (t5 == 0)
    goto carcdrloop44795;
r0 = (u64) && return0047;
goto carcdrinternal;
return0047 :
    /* TagType. */
    t5
    = t1 & 63;
t7 = arg4 - t2; // t7=0 if data same
if (t7 != 0) // J. if different
    goto rgetfcdr;
t6 = arg3 - t5; // t6 zero if same tag
if (t6 != 0) // J. if tags different
    goto rgetfcdr;
/* we found a match! */
/* TagType. */
t1 = arg5 & 63; // Strip CDR code
t5 = t1 - Type_NIL; // t5=0 if end of list
if (t5 == 0) // after all this effort we lose!
    goto rgetfexc;
t2 = arg6;
r0 = (u64) && return0048;
goto carinternal;
return0048 :
    /* TagType. */
    arg5
    = arg5 & 63; // Strip the CDR code
*(u32 *)iSP = arg6;
*(u32 *)(iSP + 4) = arg5; // write the stack cache
arg2 = t1 & 63; // set CDR-NEXT
*(u32 *)(iSP + 8) = t2; // Push the second result
*(u32 *)(iSP + 12) = arg2; // write the stack cache
iSP = iSP + 8;
goto NEXTINSTRUCTION;

carcdrloop44795 : if (_trace) printf("carcdrloop44795:\n");
arg2 = *(u64 *)&(processor->niladdress); // Return NIL
*(u64 *)iSP = arg2;
*(u64 *)(iSP + 8) = arg2; // push the data
iSP = iSP + 8;
goto NEXTINSTRUCTION;

rgetfexc : if (_trace) printf("rgetfexc:\n");
arg3 = 0; // arg3 = stackp
arg1 = 2; // arg1 = instruction arity
arg4 = 0; // arg4 = arithmeticp
goto exception;

carcdrloop44793 : if (_trace) printf("carcdrloop44793:\n");
iSP = *(u64 *)&(processor->restartsp);
goto INTERPRETINSTRUCTION;

DoRgetfIM : goto doistageerror;

/* end DoRgetf */
/* End of Halfword operand from stack instruction - DoRgetf */
/* Fin. */

/* End of file automatically generated from ../alpha-emulator/ifunlist.as */
