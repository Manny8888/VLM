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

vma-memory-read13436:
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
    goto vma-memory-read13438;

vma-memory-read13437:
  arg4 = (u32)arg4;   

vma-memory-read13444:
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

vma-memory-read13438:
  if (_trace) printf("vma-memory-read13438:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg4 = *(s32 *)t3;   
  arg3 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma-memory-read13437;   

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

vma-memory-read13445:
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
    goto vma-memory-read13447;

vma-memory-read13446:

vma-memory-read13453:
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

vma-memory-read13447:
  if (_trace) printf("vma-memory-read13447:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  arg4 = *(s32 *)t3;   
  arg3 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma-memory-read13446;   

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

force-alignment13466:
  if (_trace) printf("force-alignment13466:\n");
  if (t2 == 0) 
    goto basic-dispatch13459;
  /* Here if argument TypeFixnum */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;   

force-alignment13463:
  if (_trace) printf("force-alignment13463:\n");
  if (arg5 == 0) 
    goto binary-type-dispatch13456;
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

basic-dispatch13460:
  if (_trace) printf("basic-dispatch13460:\n");

basic-dispatch13459:
  if (_trace) printf("basic-dispatch13459:\n");
  /* Here for all other cases */

binary-type-dispatch13455:
  if (_trace) printf("binary-type-dispatch13455:\n");
  arg6 = t5;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;
  goto binary-type-dispatch13457;   

binary-type-dispatch13456:
  if (_trace) printf("binary-type-dispatch13456:\n");
  arg6 = arg3;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  goto numericexception;

binary-type-dispatch13457:
  if (_trace) printf("binary-type-dispatch13457:\n");

basic-dispatch13458:
  if (_trace) printf("basic-dispatch13458:\n");

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

force-alignment13479:
  if (_trace) printf("force-alignment13479:\n");
  if (t2 == 0) 
    goto basic-dispatch13472;
  /* Here if argument TypeCharacter */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;   

force-alignment13476:
  if (_trace) printf("force-alignment13476:\n");
  if (arg5 == 0) 
    goto binary-type-dispatch13469;
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

basic-dispatch13473:
  if (_trace) printf("basic-dispatch13473:\n");

basic-dispatch13472:
  if (_trace) printf("basic-dispatch13472:\n");
  /* Here for all other cases */

binary-type-dispatch13468:
  if (_trace) printf("binary-type-dispatch13468:\n");
  arg6 = t5;		// arg6 = tag to dispatch on 
  arg3 = 1;		// arg3 = stackp 
  arg1 = 2;		// arg1 = instruction arity 
  arg4 = 0;		// arg4 = arithmeticp 
  arg5 = 0;
  arg2 = 27;
  goto spareexception;
  goto binary-type-dispatch13470;   

binary-type-dispatch13469:
  if (_trace) printf("binary-type-dispatch13469:\n");
  arg5 = 0;
  arg2 = 27;
  goto illegaloperand;

binary-type-dispatch13470:
  if (_trace) printf("binary-type-dispatch13470:\n");

basic-dispatch13471:
  if (_trace) printf("basic-dispatch13471:\n");

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

vma-memory-read13480:
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
    goto vma-memory-read13482;

vma-memory-read13481:
  t6 = (u32)t6;   

vma-memory-read13488:
  t6 = (u32)t6;   
  t1 = arg3 & 63;		// Strip off any CDR code bits. 
  t10 = (t1 == Type_Fixnum) ? 1 : 0;   

force-alignment13495:
  if (_trace) printf("force-alignment13495:\n");
  if (t10 == 0) 
    goto basic-dispatch13490;
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

force-alignment13492:
  if (_trace) printf("force-alignment13492:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);   
  *(u32 *)t5 = t6;
  if (t10 != 0)   		// J. if in cache 
    goto vma-memory-write13491;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

basic-dispatch13490:
  if (_trace) printf("basic-dispatch13490:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

basic-dispatch13489:
  if (_trace) printf("basic-dispatch13489:\n");

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

vma-memory-write13491:
  if (_trace) printf("vma-memory-write13491:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);   

force-alignment13496:
  if (_trace) printf("force-alignment13496:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = t2 - t4;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t3 = t6;
		/* write the stack cache */
  *(u32 *)(t3 + 4) = t8;
  goto NEXTINSTRUCTION;   

vma-memory-read13482:
  if (_trace) printf("vma-memory-read13482:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t6 = *(s32 *)t3;   
  t8 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma-memory-read13481;   

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

vma-memory-read13497:
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
    goto vma-memory-read13499;

vma-memory-read13498:

vma-memory-read13505:
  t1 = arg3 & 63;		// Strip off any CDR code bits. 
  t10 = (t1 == Type_Fixnum) ? 1 : 0;   

force-alignment13512:
  if (_trace) printf("force-alignment13512:\n");
  if (t10 == 0) 
    goto basic-dispatch13507;
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

force-alignment13509:
  if (_trace) printf("force-alignment13509:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);   
  *(u32 *)t5 = t8;
  if (t10 != 0)   		// J. if in cache 
    goto vma-memory-write13508;
  goto NEXTINSTRUCTION;   
  goto NEXTINSTRUCTION;   

basic-dispatch13507:
  if (_trace) printf("basic-dispatch13507:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

basic-dispatch13506:
  if (_trace) printf("basic-dispatch13506:\n");

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

vma-memory-write13508:
  if (_trace) printf("vma-memory-write13508:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);   

force-alignment13513:
  if (_trace) printf("force-alignment13513:\n");
  t3 = *(u64 *)&(processor->stackcachedata);   
  t4 = t2 - t4;   		// Stack cache offset 
  t3 = (t4 * 8) + t3;  		// reconstruct SCA 
		/* Store in stack */
  *(u32 *)t3 = t8;
		/* write the stack cache */
  *(u32 *)(t3 + 4) = t6;
  goto NEXTINSTRUCTION;   

vma-memory-read13499:
  if (_trace) printf("vma-memory-read13499:\n");
  t4 = *(u64 *)&(processor->stackcachedata);   
  t3 = (t3 * 8) + t4;  		// reconstruct SCA 
  t8 = *(s32 *)t3;   
  t6 = *(s32 *)(t3 + 4);   		// Read from stack cache 
  goto vma-memory-read13498;   

/* end DoPTagDpb */
  /* End of Halfword operand from stack instruction - DoPTagDpb */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunfext.as */
