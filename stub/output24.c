/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/idouble.as
 ************************************************************************/

/* Support for double precision floating point. */
/* start FetchDoubleFloat */

fetchdoublefloat : if (_trace) printf("fetchdoublefloat:\n");
sp = sp + -8;
/* Memory Read Internal */

vma_memory_read46440 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(uint64_t *)&(processor->dataread_mask);
t6 = ((uint64_t) t5 < (uint64_t) t12) ? 1 : 0; // In range?
arg6 = *(int32_t *)arg6;
arg5 = (uint8_t) (arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read46442;

vma_memory_read46441 : t7 = zero + 240;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
if (t8 & 1)
    goto vma_memory_read46444;

vma_memory_read46451 : t5 = arg5 - Type_Fixnum;
t5 = t5 & 63; // Strip CDR code
if (t5 != 0)
    goto fetch_double_float_internal46439;
*((uint32_t *)(&processor->fp0) + 1) = arg6;
arg2 = arg2 + 1;
/* Memory Read Internal */

vma_memory_read46452 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(uint64_t *)&(processor->dataread_mask);
t6 = ((uint64_t) t5 < (uint64_t) t12) ? 1 : 0; // In range?
arg6 = *(int32_t *)arg6;
arg5 = (uint8_t) (arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read46454;

vma_memory_read46453 : t7 = zero + 240;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
if (t8 & 1)
    goto vma_memory_read46456;

vma_memory_read46463 : t5 = arg5 - Type_Fixnum;
t5 = t5 & 63; // Strip CDR code
if (t5 != 0)
    goto fetch_double_float_internal46439;
*(uint32_t *)&processor->fp0 = arg6;
sp = sp + 8;
goto *r0; /* ret */

vma_memory_read46456 : if (_trace) printf("vma_memory_read46456:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read46455;
arg2 = (uint32_t) arg6; // Do the indirect thing
goto vma_memory_read46452;

vma_memory_read46455 : if (_trace) printf("vma_memory_read46455:\n");

vma_memory_read46454 : if (_trace) printf("vma_memory_read46454:\n");
*(uint64_t *)sp = r0;
r0 = (uint64_t)  && return0090;
goto memoryreaddatadecode;
return0090 : r0 = *(uint64_t *)sp;
goto vma_memory_read46463;

vma_memory_read46444 : if (_trace) printf("vma_memory_read46444:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read46443;
arg2 = (uint32_t) arg6; // Do the indirect thing
goto vma_memory_read46440;

vma_memory_read46443 : if (_trace) printf("vma_memory_read46443:\n");

vma_memory_read46442 : if (_trace) printf("vma_memory_read46442:\n");
*(uint64_t *)sp = r0;
r0 = (uint64_t)  && return0091;
goto memoryreaddatadecode;
return0091 : r0 = *(uint64_t *)sp;
goto vma_memory_read46451;

fetch_double_float_internal46439 : if (_trace) printf("fetch_double_float_internal46439:\n");
arg6 = Type_DoubleFloat; // arg6 = tag to dispatch on
arg3 = 0; // arg3 = stackp
arg1 = 2; // arg1 = instruction arity
arg4 = 1; // arg4 = arithmeticp
goto numericexception;

/* end FetchDoubleFloat */
/* start ConsDoubleFloat */

consdoublefloat : if (_trace) printf("consdoublefloat:\n");
sp = sp + -8;
arg6 = *(int32_t *)&processor->fp0;
arg5 = *((int32_t *)(&processor->fp0) + 1);
t5 = *(uint64_t *)&(processor->lcarea);
t8 = *(uint64_t *)&(processor->niladdress);
t6 = *(int32_t *)&processor->lclength;
arg2 = *(uint64_t *)&(processor->lcaddress); // Fetch address
t7 = (t5 == t8) ? 1 : 0;
if (t7 != 0) // Decached area
    goto cons_double_float_internal46464;
t7 = t6 - 2; // Effectively an unsigned 32-bit compare
if ((int64_t) t7 < 0) // Insufficient cache
    goto cons_double_float_internal46464;
/* trapb force the trap to occur here */ // Force the trap to occur here
*(uint32_t *)&processor->lclength = t7; // Store remaining length
t8 = (uint32_t) arg2;
t8 = t8 + 2; // Increment address
*(uint32_t *)&processor->lcaddress = t8; // Store updated address
arg2 = (uint32_t) arg2;
t9 = Type_Fixnum;
t9 = t9 | 128;
t5 = arg2 + ivory;
t8 = (t5 * 4);
t7 = LDQ_U(t5);
t6 = (t9 & 0xff) << ((t5 & 7) * 8);
t7 = t7 & ~(0xffL << (t5 & 7) * 8);

force_alignment46465 : if (_trace) printf("force_alignment46465:\n");
t7 = t7 | t6;
STQ_U(t5, t7);
*(uint32_t *)t8 = arg5;
t10 = arg2 + 1;
t9 = Type_Fixnum;
t9 = t9 | 64;
t5 = t10 + ivory;
t8 = (t5 * 4);
t7 = LDQ_U(t5);
t6 = (t9 & 0xff) << ((t5 & 7) * 8);
t7 = t7 & ~(0xffL << (t5 & 7) * 8);

force_alignment46466 : if (_trace) printf("force_alignment46466:\n");
t7 = t7 | t6;
STQ_U(t5, t7);
*(uint32_t *)t8 = arg6;
sp = sp + 8;
goto *r0; /* ret */

cons_double_float_internal46464 : if (_trace) printf("cons_double_float_internal46464:\n");
arg6 = Type_DoubleFloat; // arg6 = tag to dispatch on
arg3 = 0; // arg3 = stackp
arg1 = 2; // arg1 = instruction arity
arg4 = 1; // arg4 = arithmeticp
goto numericexception;

/* end ConsDoubleFloat */
/* start DoDoubleFloatOp */

/* Halfword operand from stack instruction - DoDoubleFloatOp */
/* arg2 has the preloaded 8 bit operand. */

dodoublefloatop : if (_trace) printf("dodoublefloatop:\n");

DoDoubleFloatOpIM : if (_trace) printf("DoDoubleFloatOpIM:\n");
/* This sequence is lukewarm */
*(uint32_t *)&processor->immediate_arg = arg2;
arg1 = *(uint64_t *)&(processor->immediate_arg);
goto begindodoublefloatop;

DoDoubleFloatOpSP : if (_trace) printf("DoDoubleFloatOpSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 == 0) // SP-pop mode
    arg1 = iSP;
if (arg2 == 0) // Adjust SP if SP-pop mode
    iSP = arg4;

DoDoubleFloatOpLP : if (_trace) printf("DoDoubleFloatOpLP:\n");

DoDoubleFloatOpFP : if (_trace) printf("DoDoubleFloatOpFP:\n");

headdodoublefloatop : if (_trace) printf("headdodoublefloatop:\n");
arg1 = (arg2 * 8) + arg1; // Compute operand address
arg1 = *(uint64_t *)arg1; // Get the operand

begindodoublefloatop : if (_trace) printf("begindodoublefloatop:\n");
/* arg1 has the operand, not sign extended if immediate. */
arg3 = *(int32_t *)(iSP + -24); // X high
arg4 = *(int32_t *)(iSP + -16); // X low
arg5 = *(int32_t *)(iSP + -8); // Y high
arg6 = *(int32_t *)iSP; // Y low
arg3 = arg3 << 32; // Get high part up top
arg4 = (uint32_t) arg4;
arg5 = arg5 << 32; // Get high part up top
arg6 = (uint32_t) arg6;
arg3 = arg3 | arg4; // ARG3 is now X
arg5 = arg5 | arg6; // ARG5 is now Y
*(uint64_t *)&processor->fp0 = arg3;
*(uint64_t *)&processor->fp1 = arg5;
t2 = arg1 >> 32; // Immediate tag
t1 = (uint32_t) arg1; // Immediate data
t3 = t2 - Type_Fixnum;
t3 = t3 & 63; // Strip CDR code
if (t3 != 0)
    goto doublefloatiop;
LDT(1, f1, processor->fp0);
LDT(2, f2, processor->fp1);
/* NIL */
t3 = zero + DoubleFloatOp_Add;
t3 = t1 - t3;
if (t3 != 0)
    goto mondo_dispatch46468;
/* Here if argument DoubleFloatOpAdd */
ADDT(1, f1, 1, f1, 2, f2); /* addt */
goto mondo_dispatch46467;

mondo_dispatch46468 : if (_trace) printf("mondo_dispatch46468:\n");
t3 = zero + DoubleFloatOp_Sub;
t3 = t1 - t3;
if (t3 != 0)
    goto mondo_dispatch46469;
/* Here if argument DoubleFloatOpSub */
SUBT(1, f1, 1, f1, 2, f2);
goto mondo_dispatch46467;

mondo_dispatch46469 : if (_trace) printf("mondo_dispatch46469:\n");
t3 = zero + DoubleFloatOp_Multiply;
t3 = t1 - t3;
if (t3 != 0)
    goto mondo_dispatch46470;
/* Here if argument DoubleFloatOpMultiply */
MULT(1, f1, 1, f1, 2, f2);
goto mondo_dispatch46467;

mondo_dispatch46470 : if (_trace) printf("mondo_dispatch46470:\n");
t3 = zero + DoubleFloatOp_Divide;
t3 = t1 - t3;
if (t3 != 0)
    goto mondo_dispatch46471;
/* Here if argument DoubleFloatOpDivide */
DIVT(1, f1, 1, f1, 2, f2);
goto mondo_dispatch46467;

mondo_dispatch46471 : if (_trace) printf("mondo_dispatch46471:\n");

mondo_dispatch46467 : if (_trace) printf("mondo_dispatch46467:\n");
/* trapb force the trap to occur here */ // Force the trap to occur here
t3 = *(uint64_t *)&(processor->niladdress); // There was no FP exception

doublefloatmerge : STT((uint64_t *)&processor->fp0, 1, f1);
t1 = *(int32_t *)&processor->fp0;
t2 = *((int32_t *)(&processor->fp0) + 1);
iSP = iSP - 32; // Pop all the operands
t4 = Type_Fixnum;
*(uint32_t *)(iSP + 8) = t2; // Push high result
*(uint32_t *)(iSP + 12) = t4; // write the stack cache
iSP = iSP + 8;
t4 = Type_Fixnum;
*(uint32_t *)(iSP + 8) = t1; // Push low result
*(uint32_t *)(iSP + 12) = t4; // write the stack cache
iSP = iSP + 8;
iSP = iSP + 8;
t4 = t3 << 26;
t4 = t4 >> 26;
*(uint64_t *)iSP = t4; // Push the exception predicate
goto NEXTINSTRUCTION;

doublefloatexc : if (_trace) printf("doublefloatexc:\n");
t3 = *(uint64_t *)&(processor->taddress); // Indicate an FP exception occurred
goto doublefloatmerge;

doublefloatiop : if (_trace) printf("doublefloatiop:\n");
arg5 = 0;
arg2 = 85;
goto illegaloperand;

/* end DoDoubleFloatOp */
/* End of Halfword operand from stack instruction - DoDoubleFloatOp */
/* Fin. */

/* End of file automatically generated from ../alpha-emulator/idouble.as */
