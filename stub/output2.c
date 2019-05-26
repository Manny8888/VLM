/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * ANY CHANGES MADE TO THIS FILE WILL BE LOST
 *
 * File translated:      ../alpha-emulator/idispat.as
 ************************************************************************/

/* This file implements the main instruction dispatch loop. */
/* start DummyDoNothingSubroutine */

dummydonothingsubroutine : if (_trace) printf("dummydonothingsubroutine:\n");
goto continuecurrentinstruction;

/* end DummyDoNothingSubroutine */
/* start MemoryReadData */

memoryreaddata : if (_trace) printf("memoryreaddata:\n");
/* Memory Read Internal */

vma_memory_read43838 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(uint64_t *)&(processor->dataread_mask);
t6 = ((uint64_t) t5 < (uint64_t) t12) ? 1 : 0; // In range?
arg6 = *(int32_t *)arg6;
arg5 = (uint8_t) (arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read43840;

vma_memory_read43839 : t7 = zero + 240;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
arg6 = (uint32_t) arg6;
if (t8 & 1)
    goto vma_memory_read43842;

vma_memory_read43848 : goto *r0; /* ret */

memoryreaddatadecode : if (_trace) printf("memoryreaddatadecode:\n");
if (t6 == 0)
    goto vma_memory_read43841;

vma_memory_read43840 : if (_trace) printf("vma_memory_read43840:\n");
t6 = *(uint64_t *)&(processor->stackcachedata);
t5 = (t5 * 8) + t6; // reconstruct SCA
arg6 = *(int32_t *)t5;
arg5 = *(int32_t *)(t5 + 4); // Read from stack cache
goto vma_memory_read43839;

vma_memory_read43842 : if (_trace) printf("vma_memory_read43842:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read43841;
arg2 = (uint32_t) arg6; // Do the indirect thing
goto vma_memory_read43838;

vma_memory_read43841 : if (_trace) printf("vma_memory_read43841:\n");
t8 = *(uint64_t *)&(processor->dataread); // Load the memory action table for cycle
/* TagType. */
t7 = arg5 & 63; // Discard the CDR code
*(uint64_t *)&processor->vma = arg2; // stash the VMA for the (likely) trap
t7 = (t7 * 4) + t8; // Adjust for a longword load
t8 = *(int32_t *)t7; // Get the memory action

vma_memory_read43845 : if (_trace) printf("vma_memory_read43845:\n");
t7 = t8 & MemoryActionTransform;
if (t7 == 0)
    goto vma_memory_read43844;
arg5 = arg5 & ~63L;
arg5 = arg5 | Type_ExternalValueCellPointer;
goto vma_memory_read43848;

vma_memory_read43844 :

    vma_memory_read43843 :
    /* Perform memory action */
    arg1
    = t8;
arg2 = 0;
goto performmemoryaction;

/* end MemoryReadData */
/* start MemoryReadGeneral */

memoryreadgeneral : if (_trace) printf("memoryreadgeneral:\n");
/* Memory Read Internal */

vma_memory_read43849 : t7 = arg2 + ivory;
t8 = (arg3 * 4); // Cycle-number -> table offset
arg5 = LDQ_U(t7);
t8 = (t8 * 4) + ivory;
arg6 = (t7 * 4);
t5 = arg2 - t11; // Stack cache offset
t8 = *(uint64_t *)(t8 + PROCESSORSTATE_DATAREAD_MASK);
t6 = ((uint64_t) t5 < (uint64_t) t12) ? 1 : 0; // In range?
arg6 = *(int32_t *)arg6;
arg5 = (uint8_t) (arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read43851;

vma_memory_read43850 : t8 = t8 >> (arg5 & 63);
arg6 = (uint32_t) arg6;
if (t8 & 1)
    goto vma_memory_read43853;

vma_memory_read43859 : goto *r0; /* ret */

memoryreadgeneraldecode : if (_trace) printf("memoryreadgeneraldecode:\n");
if (t6 == 0)
    goto vma_memory_read43852;

vma_memory_read43851 : if (_trace) printf("vma_memory_read43851:\n");
t6 = *(uint64_t *)&(processor->stackcachedata);
t5 = (t5 * 8) + t6; // reconstruct SCA
arg6 = *(int32_t *)t5;
arg5 = *(int32_t *)(t5 + 4); // Read from stack cache
goto vma_memory_read43850;

vma_memory_read43853 : if (_trace) printf("vma_memory_read43853:\n");

vma_memory_read43852 : if (_trace) printf("vma_memory_read43852:\n");
t8 = (arg3 * 4); // Cycle-number -> table offset
t8 = (t8 * 4) + ivory;
t8 = *(uint64_t *)(t8 + PROCESSORSTATE_DATAREAD);
/* TagType. */
t7 = arg5 & 63; // Discard the CDR code
*(uint64_t *)&processor->vma = arg2; // stash the VMA for the (likely) trap
t7 = (t7 * 4) + t8; // Adjust for a longword load
t8 = *(int32_t *)t7; // Get the memory action

vma_memory_read43857 : if (_trace) printf("vma_memory_read43857:\n");
t6 = t8 & MemoryActionIndirect;
if (t6 == 0)
    goto vma_memory_read43856;
arg2 = (uint32_t) arg6; // Do the indirect thing
goto vma_memory_read43849;

vma_memory_read43856 : if (_trace) printf("vma_memory_read43856:\n");
t7 = t8 & MemoryActionTransform;
if (t7 == 0)
    goto vma_memory_read43855;
arg5 = arg5 & ~63L;
arg5 = arg5 | Type_ExternalValueCellPointer;
goto vma_memory_read43859;

vma_memory_read43855 :

    vma_memory_read43854 :
    /* Perform memory action */
    arg1
    = t8;
arg2 = arg3;
goto performmemoryaction;

/* end MemoryReadGeneral */
/* start MemoryReadHeader */

memoryreadheader : if (_trace) printf("memoryreadheader:\n");
/* Memory Read Internal */

vma_memory_read43860 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(uint64_t *)&(processor->header_mask);
t6 = ((uint64_t) t5 < (uint64_t) t12) ? 1 : 0; // In range?
arg6 = *(int32_t *)arg6;
arg5 = (uint8_t) (arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read43862;

vma_memory_read43861 : t7 = zero + 64;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
arg6 = (uint32_t) arg6;
if (t8 & 1)
    goto vma_memory_read43864;

vma_memory_read43868 : goto *r0; /* ret */

memoryreadheaderdecode : if (_trace) printf("memoryreadheaderdecode:\n");
if (t6 == 0)
    goto vma_memory_read43863;

vma_memory_read43862 : if (_trace) printf("vma_memory_read43862:\n");
t6 = *(uint64_t *)&(processor->stackcachedata);
t5 = (t5 * 8) + t6; // reconstruct SCA
arg6 = *(int32_t *)t5;
arg5 = *(int32_t *)(t5 + 4); // Read from stack cache
goto vma_memory_read43861;

vma_memory_read43864 : if (_trace) printf("vma_memory_read43864:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read43863;
arg2 = (uint32_t) arg6; // Do the indirect thing
goto vma_memory_read43860;

vma_memory_read43863 : if (_trace) printf("vma_memory_read43863:\n");
t8 = *(uint64_t *)&(processor->header); // Load the memory action table for cycle
/* TagType. */
t7 = arg5 & 63; // Discard the CDR code
*(uint64_t *)&processor->vma = arg2; // stash the VMA for the (likely) trap
t7 = (t7 * 4) + t8; // Adjust for a longword load
t8 = *(int32_t *)t7; // Get the memory action

vma_memory_read43865 :
    /* Perform memory action */
    arg1
    = t8;
arg2 = 6;
goto performmemoryaction;

/* end MemoryReadHeader */
/* start MemoryReadCdr */

memoryreadcdr : if (_trace) printf("memoryreadcdr:\n");
/* Memory Read Internal */

vma_memory_read43869 : t7 = arg2 + ivory;
arg6 = (t7 * 4);
arg5 = LDQ_U(t7);
t5 = arg2 - t11; // Stack cache offset
t8 = *(uint64_t *)&(processor->cdr_mask);
t6 = ((uint64_t) t5 < (uint64_t) t12) ? 1 : 0; // In range?
arg6 = *(int32_t *)arg6;
arg5 = (uint8_t) (arg5 >> ((t7 & 7) * 8));
if (t6 != 0)
    goto vma_memory_read43871;

vma_memory_read43870 : t7 = zero + 192;
t8 = t8 >> (arg5 & 63);
t7 = t7 >> (arg5 & 63);
arg6 = (uint32_t) arg6;
if (t8 & 1)
    goto vma_memory_read43873;

vma_memory_read43877 : goto *r0; /* ret */

memoryreadcdrdecode : if (_trace) printf("memoryreadcdrdecode:\n");
if (t6 == 0)
    goto vma_memory_read43872;

vma_memory_read43871 : if (_trace) printf("vma_memory_read43871:\n");
t6 = *(uint64_t *)&(processor->stackcachedata);
t5 = (t5 * 8) + t6; // reconstruct SCA
arg6 = *(int32_t *)t5;
arg5 = *(int32_t *)(t5 + 4); // Read from stack cache
goto vma_memory_read43870;

vma_memory_read43873 : if (_trace) printf("vma_memory_read43873:\n");
if ((t7 & 1) == 0)
    goto vma_memory_read43872;
arg2 = (uint32_t) arg6; // Do the indirect thing
goto vma_memory_read43869;

vma_memory_read43872 : if (_trace) printf("vma_memory_read43872:\n");
t8 = *(uint64_t *)&(processor->cdr); // Load the memory action table for cycle
/* TagType. */
t7 = arg5 & 63; // Discard the CDR code
*(uint64_t *)&processor->vma = arg2; // stash the VMA for the (likely) trap
t7 = (t7 * 4) + t8; // Adjust for a longword load
t8 = *(int32_t *)t7; // Get the memory action

vma_memory_read43874 :
    /* Perform memory action */
    arg1
    = t8;
arg2 = 9;
goto performmemoryaction;

/* end MemoryReadCdr */
/* start DoICacheFill */

doicachefill : if (_trace) printf("doicachefill:\n");

ICACHEMISS : if (_trace) printf("ICACHEMISS:\n");
/* Here when instruction cache miss detected.  Fill the cache from */
/* PC and then resume interpreter loop */
/* First round the PC down to an even halfword address */
arg2 = *(uint64_t *)&(processor->icachebase); // get the base of the icache
epc = iPC & ~1L; // the even PC
ecp = epc >> (CacheLine_RShift & 63);
arg1 = zero + -1;
arg1 = arg1 + ((4) << 16);
ecp = ecp << (CacheLine_LShift & 63);
instn = iPC >> 1; // instn is instruction address here
ecp = epc + ecp;
ecp = ecp & arg1;
arg3 = ecp << 5; // temp=cpos*32
ecp = ecp << 4; // cpos=cpos*16
arg4 = arg2 + arg3; // temp2=base+cpos*32
ecp = arg4 + ecp; // cpos=base+cpos*48
opc = epc | 1; // the odd PC
iCP = ecp; // Assume iPC is the even PC
arg1 = (iPC == opc) ? 1 : 0; // See if iPC is the odd PC
ocp = ecp + CACHELINE_SIZE;
if (arg1) // Stash the odd cache pointer if iPC is the odd PC
    iCP = ocp;
hwdispatch = *(uint64_t *)&(processor->halfworddispatch);
hwopmask = zero + 1023;
fwdispatch = *(uint64_t *)&(processor->fullworddispatch);
count = zero + 20;
t11 = instn + ivory;
iword = (t11 * 4);
arg4 = LDQ_U(t11);
iword = *(int32_t *)iword;
arg4 = (uint8_t) (arg4 >> ((t11 & 7) * 8));
goto fillicacheprefetched;

pcbackone : if (_trace) printf("pcbackone:\n");
/* Wire in continuation for even half */
*(uint64_t *)&((CACHELINEP)ocp)->nextpcdata = epc;
t10 = ecp - CACHELINE_SIZE; // Backup in cache too
*(uint64_t *)&((CACHELINEP)ocp)->nextcp = ecp;
arg1 = epc - 1; // Backup PC one halfword
*(uint64_t *)&((CACHELINEP)ecp)->nextcp = t10;
/* TagType. */
arg4 = arg4 & 63; // arg4=tag-cdr code
*(uint64_t *)&((CACHELINEP)ecp)->nextpcdata = arg1;
/* Wire in continuation for odd half */
goto maybeunpack;

pcadvone : if (_trace) printf("pcadvone:\n");
*(uint64_t *)&((CACHELINEP)ecp)->nextpcdata = opc; // Simple advance of PC one halfword.
arg1 = opc + 1;
*(uint64_t *)&((CACHELINEP)ecp)->nextcp = ocp;
t10 = ocp + CACHELINE_SIZE;
*(uint64_t *)&((CACHELINEP)ocp)->nextpcdata = arg1;
/* TagType. */
arg4 = arg4 & 63; // arg4=tag-cdr code
*(uint64_t *)&((CACHELINEP)ocp)->nextcp = t10;
goto maybeunpack;
/* This is the cache fill loop. */

fillicache : if (_trace) printf("fillicache:\n");
t11 = instn + ivory;
iword = (t11 * 4);
arg4 = LDQ_U(t11);
iword = *(int32_t *)iword;
arg4 = (uint8_t) (arg4 >> ((t11 & 7) * 8));

fillicacheprefetched : if (_trace) printf("fillicacheprefetched:\n");
*(uint64_t *)&((CACHELINEP)ecp)->pcdata = epc; // Set address of even cache posn.
arg1 = arg4 & 192; // CDR code << 6
/* TagType. */
arg4 = arg4 & 63; // Strip cdr
*(uint64_t *)&((CACHELINEP)ocp)->pcdata = opc; // Set address of odd cache posn.
iword = (uint32_t) iword; // Strip nasty bits out.

force_alignment43878 : if (_trace) printf("force_alignment43878:\n");
arg2 = arg4 << 32; // ready to remerge
if (arg1 == 0) // Zerotag means advance one HW
    goto pcadvone;
arg1 = arg1 - 128; // 2<<6
if (arg1 == 0) // Tag=2 means backup one HW
    goto pcbackone;
if ((int64_t) arg1 < 0) // Tag=1 means end of compiled function
    goto pcendcf;

pcadvtwo : if (_trace) printf("pcadvtwo:\n");
/* Tag=3 means advance over one full word */
/* Wire in continuation for even half */
arg1 = epc + 2; // Next word
r31 = r31 | r31;
t10 = ecp + TWOCACHELINESIZE; // corresponding CP entry
*(uint64_t *)&((CACHELINEP)ecp)->nextpcdata = arg1; // Next PC even of next word
arg1 = epc + 4; // Skip one fullword
*(uint64_t *)&((CACHELINEP)ecp)->nextcp = t10; // Next CP
/* Wire in continuation for odd half */
t10 = ecp + FOURCACHELINESIZE; // corresponding CP entry
*(uint64_t *)&((CACHELINEP)ocp)->nextpcdata = arg1;
/* TagType. */
arg4 = arg4 & 63; // arg4=tag-cdr code
*(uint64_t *)&((CACHELINEP)ocp)->nextcp = t10;
goto maybeunpack;

decodepackedword : if (_trace) printf("decodepackedword:\n");
/* Here to decode a packed word */
arg4 = iword >> 18; // arg4 contains the odd packedword
t10 = iword >> 8; // even opcode+2bits
*(uint64_t *)&((CACHELINEP)ocp)->instruction = arg4; // Save the odd instruction
t11 = iword << 54; // First phase of even operand sign extension.
t12 = iword & hwopmask; // even operand+2bits
t10 = t10 & hwopmask; // even opcode
t11 = (int64_t) t11 >> 38; // Second phase of even operand sign extension.
arg2 = t10 - 92;
t10 = (t10 * 8) + hwdispatch;
t12 = t11 | t12; // Merge signed/unsigned even operand
arg2 = arg2 & ~3L;
*(uint32_t *)&((CACHELINEP)ecp)->operand = t12;
if (arg2 == 0) // clear count if finish-call seen
    count = arg2;
arg2 = arg4 >> 8; // odd opcode+2bits
t11 = arg4 << 54; // First phase of odd operand sign extension.
arg1 = arg4 & hwopmask; // odd operand+2bits
t10 = *(uint64_t *)t10;
arg2 = arg2 & hwopmask; // odd opcode
t11 = (int64_t) t11 >> 38; // Second phase of odd operand sign extension.
*(uint64_t *)&((CACHELINEP)ecp)->code = t10;
t12 = arg2 - 92;
arg2 = (arg2 * 8) + hwdispatch;
arg1 = t11 | arg1; // Merge signed/unsigned odd operand
*(uint32_t *)&((CACHELINEP)ocp)->operand = arg1;
t12 = t12 & ~3L;
arg2 = *(uint64_t *)arg2;
if (t12 == 0) // clear count if finish-call seen
    count = t12;
*(uint64_t *)&((CACHELINEP)ocp)->code = arg2;
goto enddecode;

maybeunpack : if (_trace) printf("maybeunpack:\n");
iword = arg2 | iword; // reassemble tag and word.
*(uint64_t *)&((CACHELINEP)ecp)->instruction = iword; // save the even instruction
t10 = arg4 - 48; // t10>=0 if packed
if ((int64_t) t10 >= 0) // B. if a packed instruction
    goto decodepackedword;
t11 = (arg4 * 8) + fwdispatch; // t11 is the fwdispatch index
t12 = *(uint64_t *)&(processor->i_stage_error_hook);
arg1 = arg4 - 33;
t11 = *(uint64_t *)t11; // Extract the opcode handler
*(uint64_t *)&((CACHELINEP)ocp)->code = t12; // Store I-STATE-ERROR at odd pc
if (arg1 == 0) // clear count if native instn seen
    count = arg1;
*(uint64_t *)&((CACHELINEP)ecp)->code = t11;

enddecode : if (_trace) printf("enddecode:\n");
/* Here we decide if to stop filling the cache and return to the */
/* instruction interpretation stream, or whether to fill further */
instn = instn + 1;
if ((int64_t) count <= 0) // If count is zero, resume
    goto cachevalid;
epc = instn << 1;
count = count - 1; // decrement count
opc = epc | 1;
t10 = *(uint64_t *)&(processor->endicache); // pointer to the end of icache
ocp = ocp + TWOCACHELINESIZE;
ecp = ecp + TWOCACHELINESIZE;
t10 = ocp - t10;
if ((int64_t) t10 <= 0) // Still room for more
    goto fillicache;
goto cachevalid;

pcendcf : if (_trace) printf("pcendcf:\n");
t11 = *(uint64_t *)&(processor->i_stage_error_hook);
count = r31 | r31; // We reached the end of the fcn.
*(uint64_t *)&((CACHELINEP)ecp)->code = t11; // Store I-STATE-ERROR dispatch at even and odd pc
*(uint64_t *)&((CACHELINEP)ocp)->code = t11;
goto enddecode;

/* end DoICacheFill */
/* These are the instruction reentry points.  Instructions end by returning */
/* control to one of these tags.  Most normal instructions reenter by jumping */
/* to NEXTINSTRUCTION, which advances the PC and continues normally.   */
/* Instructions that change the PC usually go directly to INTERPRETINSTRUCTION. */
/* Instructions that fail/trap/exception etc, go to one of the other places. */
/* start iInterpret */

iinterpret : if (_trace) printf("iinterpret:\n");
*(uint64_t *)&processor->asrr9 = r9;
*(uint64_t *)&processor->asrr10 = r10;
*(uint64_t *)&processor->asrr11 = r11;
*(uint64_t *)&processor->asrr12 = r12;
*(uint64_t *)&processor->asrr13 = r13;
*(uint64_t *)&processor->asrr15 = r15;
*(uint64_t *)&processor->asrr26 = r26;
*(uint64_t *)&processor->asrr27 = r27;
*(uint64_t *)&processor->asrr29 = r29;
*(uint64_t *)&processor->asrr30 = r30;
*(uint64_t *)&processor->asrr14 = r14;
ivory = arg1; // Setup our processor object handle
/* Upon entry, load cached state. */
iCP = *(uint64_t *)&(processor->cp);
iPC = *(uint64_t *)&(processor->epc);
iSP = *(uint64_t *)&(processor->sp);
iFP = *(uint64_t *)&(processor->fp);
iLP = *(uint64_t *)&(processor->lp);
if (iCP != 0) // First time in iCP will be zero.
    goto INTERPRETINSTRUCTION;
goto ICACHEMISS; // If this is the first time in cache is empty!

interpretinstructionpredicted : if (_trace) printf("interpretinstructionpredicted:\n");
t2 = *(uint64_t *)&(((CACHELINEP)arg2)->pcdata); // Get the PC to check cache hit.
arg1 = iFP; // Assume FP mode
r0 = *(uint64_t *)&(processor->stop_interpreter); // Have we been asked to stop?
arg4 = iSP + -8; // SP-pop mode constant
arg3 = *(uint64_t *)&(((CACHELINEP)arg2)->instruction); // Grab the instruction/operand while stalled
t1 = iPC - t2;
if (t1 != 0)
    goto interpretinstructionforbranch;
iCP = arg2;
if (r0 != 0) // Stop the world! someone wants out.
    goto traporsuspendmachine;
goto continuecurrentinstruction;

interpretinstructionforjump : if (_trace) printf("interpretinstructionforjump:\n");

interpretinstructionforbranch : if (_trace) printf("interpretinstructionforbranch:\n");
t5 = *(uint64_t *)&(processor->icachebase); // get the base of the icache
t4 = zero + -1;
t4 = t4 + ((4) << 16);
arg2 = iPC >> 10;
t3 = zero + -64;
arg2 = arg2 & t3;
arg2 = iPC + arg2;
arg2 = arg2 & t4;
t4 = arg2 << 5; // temp=cpos*32
arg2 = arg2 << 4; // cpos=cpos*16
t5 = t5 + t4; // temp2=base+cpos*32

force_alignment43879 : if (_trace) printf("force_alignment43879:\n");
arg2 = t5 + arg2; // cpos=base+cpos*48
iCP = arg2;

INTERPRETINSTRUCTION : if (_trace) printf("INTERPRETINSTRUCTION:\n");
r30 = *(uint64_t *)&(processor->asrr30);
r0 = *(uint64_t *)&(processor->stop_interpreter); // Have we been asked to stop?
arg1 = iFP; // Assume FP mode
arg3 = *(uint64_t *)&(((CACHELINEP)iCP)->instruction); // Grab the instruction/operand while stalled
arg4 = iSP + -8; // SP-pop mode constant
t2 = *(uint64_t *)&(((CACHELINEP)iCP)->pcdata); // Get the PC to check cache hit.
if (r0 != 0) // Stop the world! someone wants out.
    goto traporsuspendmachine;
goto continuecurrentinstruction;

/* end iInterpret */

/* End of file automatically generated from ../alpha-emulator/idispat.as */
