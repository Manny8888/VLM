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
#ifdef TRACING
#endif

DoCharLdbIM:
  if (_trace) printf("DoCharLdbIM:\n");

DoCharLdbSP:
  if (_trace) printf("DoCharLdbSP:\n");

DoCharLdbLP:
  if (_trace) printf("DoCharLdbLP:\n");

DoCharLdbFP:
  if (_trace) printf("DoCharLdbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* t7= -1 */
  t7 = zero - 1;
  /* get ARG1 tag/data */
  arg3 = *(s32 *)(iSP + 4);
  arg4 = *(s32 *)iSP;
  /* Size of field */
  arg1 = arg1 + 1;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* TagType. */
  t8 = arg3 & 63;
  t9 = t8 - Type_Character;
  /* Clear sign extension now */
  arg4 = (u32)arg4;
  /* Not a character */
  if (t9 != 0)
    goto charldbexc;
  /* T4= shifted value if PP==0 */
  t4 = arg4 << (arg2 & 63);
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);
  /* T5= shifted value if PP<>0 */
  t5 = t4 >> 32;
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);
  /* T5= shifted value */
  if (arg2 == 0)
    t5 = t4;
  /* T3= masked value. */
  t3 = t5 & ~t7;
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
#ifdef TRACING
#endif

DoPLdbIM:
  if (_trace) printf("DoPLdbIM:\n");

DoPLdbSP:
  if (_trace) printf("DoPLdbSP:\n");

DoPLdbLP:
  if (_trace) printf("DoPLdbLP:\n");

DoPLdbFP:
  if (_trace) printf("DoPLdbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* get arg1 tag/data */
  t2 = *(s32 *)iSP;
  t1 = *(s32 *)(iSP + 4);
  t2 = (u32)t2;
  t3 = t1 - Type_PhysicalAddress;
  t3 = t3 & 63;
  if (t3 == 0)
    goto pldbillop;
  /* Memory Read Internal */

g2936:
  /* Base of stack cache */
  t3 = *(u64 *)&(processor->stackcachebasevma);
  t5 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;
  arg4 = (t5 * 4);
  arg3 = LDQ_U(t5);
  /* Stack cache offset */
  t3 = t2 - t3;
  /* In range? */
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;
  arg4 = *(s32 *)arg4;
  arg3 = (u8)(arg3 >> ((t5&7)*8));
  if (t4 != 0)
    goto g2938;

g2937:
  arg4 = (u32)arg4;

g2944:
  /* t7= -1 */
  t7 = zero - 1;
  /* Size of field */
  arg1 = arg1 + 1;
  /* T4= shifted value if PP==0 */
  t4 = arg4 << (arg2 & 63);
  /* T5= shifted value if PP<>0 */
  t5 = t4 >> 32;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* T5= shifted value */
  if (arg2 == 0)
    t5 = t4;
  /* T3= masked value. */
  t3 = t5 & ~t7;
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
  /* stack cache base relative offset */
  t2 = iSP - t2;
  /* convert byte address to word address */
  t2 = t2 >> 3;
  /* reconstruct VMA */
  t1 = t2 + t1;
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

g2938:
  if (_trace) printf("g2938:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t3 = (t3 * 8) + t4;
  arg4 = *(s32 *)t3;
  /* Read from stack cache */
  arg3 = *(s32 *)(t3 + 4);
  goto g2937;

/* end DoPLdb */
  /* End of Halfword operand from stack instruction - DoPLdb */
/* start DoPTagLdb */

  /* Field Extraction instruction - DoPTagLdb */

doptagldb:
  if (_trace) printf("doptagldb:\n");
  /* Actually only one entry point, but simulate others for dispatch */
#ifdef TRACING
#endif

DoPTagLdbIM:
  if (_trace) printf("DoPTagLdbIM:\n");

DoPTagLdbSP:
  if (_trace) printf("DoPTagLdbSP:\n");

DoPTagLdbLP:
  if (_trace) printf("DoPTagLdbLP:\n");

DoPTagLdbFP:
  if (_trace) printf("DoPTagLdbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* get arg1 tag/data */
  t2 = *(s32 *)iSP;
  t1 = *(s32 *)(iSP + 4);
  t2 = (u32)t2;
  t3 = t1 - Type_PhysicalAddress;
  t3 = t3 & 63;
  if (t3 == 0)
    goto ptagldbillop;
  /* Memory Read Internal */

g2945:
  /* Base of stack cache */
  t3 = *(u64 *)&(processor->stackcachebasevma);
  t5 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;
  arg4 = (t5 * 4);
  arg3 = LDQ_U(t5);
  /* Stack cache offset */
  t3 = t2 - t3;
  /* In range? */
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;
  arg4 = *(s32 *)arg4;
  arg3 = (u8)(arg3 >> ((t5&7)*8));
  if (t4 != 0)
    goto g2947;

g2946:

g2953:
  /* t7= -1 */
  t7 = zero - 1;
  /* Size of field */
  arg1 = arg1 + 1;
  /* T4= shifted value if PP==0 */
  t4 = arg3 << (arg2 & 63);
  /* T5= shifted value if PP<>0 */
  t5 = t4 >> 32;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* T5= shifted value */
  if (arg2 == 0)
    t5 = t4;
  /* T3= masked value. */
  t3 = t5 & ~t7;
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
  /* stack cache base relative offset */
  t2 = iSP - t2;
  /* convert byte address to word address */
  t2 = t2 >> 3;
  /* reconstruct VMA */
  t1 = t2 + t1;
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

g2947:
  if (_trace) printf("g2947:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t3 = (t3 * 8) + t4;
  arg4 = *(s32 *)t3;
  /* Read from stack cache */
  arg3 = *(s32 *)(t3 + 4);
  goto g2946;

/* end DoPTagLdb */
  /* End of Halfword operand from stack instruction - DoPTagLdb */
/* start DoDpb */

  /* Field Extraction instruction - DoDpb */

dodpb:
  if (_trace) printf("dodpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */
#ifdef TRACING
#endif

DoDpbIM:
  if (_trace) printf("DoDpbIM:\n");

DoDpbSP:
  if (_trace) printf("DoDpbSP:\n");

DoDpbLP:
  if (_trace) printf("DoDpbLP:\n");

DoDpbFP:
  if (_trace) printf("DoDpbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* Get arg2 tag/data */
  t6 = *(s32 *)iSP;
  /* Get arg2 tag/data */
  t5 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  t6 = (u32)t6;
  /* get arg1 tag/data */
  arg4 = *(s32 *)iSP;
  arg3 = *(s32 *)(iSP + 4);
  arg4 = (u32)arg4;
  /* Strip off any CDR code bits. */
  t1 = t5 & 63;
  /* Strip off any CDR code bits. */
  arg6 = arg3 & 63;
  t2 = (t1 == Type_Fixnum) ? 1 : 0;

g2966:
  if (_trace) printf("g2966:\n");
  if (t2 == 0)
    goto g2959;
  /* Here if argument TypeFixnum */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;

g2963:
  if (_trace) printf("g2963:\n");
  if (arg5 == 0)
    goto g2956;
  /* Here if argument TypeFixnum */
  /* t7= -2 */
  t7 = zero - 2;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* reuse t5 as mask */
  t5 = ~t7;
  /* T3= masked new value. */
  t3 = arg4 & ~t7;
  /* t5 is the inplace mask */
  t5 = t5 << (arg2 & 63);
  /* t4 is the shifted field */
  t4 = t3 << (arg2 & 63);
  /* Clear out existing bits in arg2 field */
  t6 = t6 & ~t5;
  /* Put the new bits in */
  t6 = t4 | t6;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);
  t4 = Type_Fixnum;
  *(u32 *)iSP = t6;
  /* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
  goto cachevalid;

g2960:
  if (_trace) printf("g2960:\n");

g2959:
  if (_trace) printf("g2959:\n");
  /* Here for all other cases */

g2955:
  if (_trace) printf("g2955:\n");
  /* arg6 = tag to dispatch on */
  arg6 = t5;
  /* arg3 = stackp */
  arg3 = 1;
  /* arg1 = instruction arity */
  arg1 = 2;
  /* arg4 = arithmeticp */
  arg4 = 0;
  goto numericexception;
  goto g2957;

g2956:
  if (_trace) printf("g2956:\n");
  /* arg6 = tag to dispatch on */
  arg6 = arg3;
  /* arg3 = stackp */
  arg3 = 1;
  /* arg1 = instruction arity */
  arg1 = 2;
  /* arg4 = arithmeticp */
  arg4 = 0;
  goto numericexception;

g2957:
  if (_trace) printf("g2957:\n");

g2958:
  if (_trace) printf("g2958:\n");

/* end DoDpb */
  /* End of Halfword operand from stack instruction - DoDpb */
/* start DoCharDpb */

  /* Field Extraction instruction - DoCharDpb */

dochardpb:
  if (_trace) printf("dochardpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */
#ifdef TRACING
#endif

DoCharDpbIM:
  if (_trace) printf("DoCharDpbIM:\n");

DoCharDpbSP:
  if (_trace) printf("DoCharDpbSP:\n");

DoCharDpbLP:
  if (_trace) printf("DoCharDpbLP:\n");

DoCharDpbFP:
  if (_trace) printf("DoCharDpbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* Get arg2 tag/data */
  t6 = *(s32 *)iSP;
  /* Get arg2 tag/data */
  t5 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  t6 = (u32)t6;
  /* get arg1 tag/data */
  arg4 = *(s32 *)iSP;
  arg3 = *(s32 *)(iSP + 4);
  arg4 = (u32)arg4;
  /* Strip off any CDR code bits. */
  t1 = t5 & 63;
  /* Strip off any CDR code bits. */
  arg6 = arg3 & 63;
  t2 = (t1 == Type_Character) ? 1 : 0;

g2979:
  if (_trace) printf("g2979:\n");
  if (t2 == 0)
    goto g2972;
  /* Here if argument TypeCharacter */
  arg5 = (arg6 == Type_Fixnum) ? 1 : 0;

g2976:
  if (_trace) printf("g2976:\n");
  if (arg5 == 0)
    goto g2969;
  /* Here if argument TypeFixnum */
  /* t7= -2 */
  t7 = zero - 2;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* reuse t5 as mask */
  t5 = ~t7;
  /* T3= masked new value. */
  t3 = arg4 & ~t7;
  /* t5 is the inplace mask */
  t5 = t5 << (arg2 & 63);
  /* t4 is the shifted field */
  t4 = t3 << (arg2 & 63);
  /* Clear out existing bits in arg2 field */
  t6 = t6 & ~t5;
  /* Put the new bits in */
  t6 = t4 | t6;
  iPC = *(u64 *)&(((CACHELINEP)iCP)->nextpcdata);
  iCP = *(u64 *)&(((CACHELINEP)iCP)->nextcp);
  t4 = Type_Character;
  *(u32 *)iSP = t6;
  /* write the stack cache */
  *(u32 *)(iSP + 4) = t4;
  goto cachevalid;

g2973:
  if (_trace) printf("g2973:\n");

g2972:
  if (_trace) printf("g2972:\n");
  /* Here for all other cases */

g2968:
  if (_trace) printf("g2968:\n");
  /* arg6 = tag to dispatch on */
  arg6 = t5;
  /* arg3 = stackp */
  arg3 = 1;
  /* arg1 = instruction arity */
  arg1 = 2;
  /* arg4 = arithmeticp */
  arg4 = 0;
  arg5 = 0;
  arg2 = 27;
  goto spareexception;
  goto g2970;

g2969:
  if (_trace) printf("g2969:\n");
  arg5 = 0;
  arg2 = 27;
  goto illegaloperand;

g2970:
  if (_trace) printf("g2970:\n");

g2971:
  if (_trace) printf("g2971:\n");

/* end DoCharDpb */
  /* End of Halfword operand from stack instruction - DoCharDpb */
/* start DoPDpb */

  /* Field Extraction instruction - DoPDpb */

dopdpb:
  if (_trace) printf("dopdpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */
#ifdef TRACING
#endif

DoPDpbIM:
  if (_trace) printf("DoPDpbIM:\n");

DoPDpbSP:
  if (_trace) printf("DoPDpbSP:\n");

DoPDpbLP:
  if (_trace) printf("DoPDpbLP:\n");

DoPDpbFP:
  if (_trace) printf("DoPDpbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* Get arg2 tag/data */
  t2 = *(s32 *)iSP;
  /* Get arg2 tag/data */
  t1 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  t2 = (u32)t2;
  t3 = t1 - Type_PhysicalAddress;
  t3 = t3 & 63;
  if (t3 == 0)
    goto pdpbillop;
  /* get arg1 tag/data */
  arg4 = *(s32 *)iSP;
  /* get arg1 tag/data */
  arg3 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  arg4 = (u32)arg4;
  /* Memory Read Internal */

g2980:
  /* Base of stack cache */
  t3 = *(u64 *)&(processor->stackcachebasevma);
  t1 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;
  t6 = (t1 * 4);
  t8 = LDQ_U(t1);
  /* Stack cache offset */
  t3 = t2 - t3;
  /* In range? */
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;
  t6 = *(s32 *)t6;
  t8 = (u8)(t8 >> ((t1&7)*8));
  if (t4 != 0)
    goto g2982;

g2981:
  t6 = (u32)t6;

g2988:
  t6 = (u32)t6;
  /* Strip off any CDR code bits. */
  t1 = arg3 & 63;
  t10 = (t1 == Type_Fixnum) ? 1 : 0;

g2995:
  if (_trace) printf("g2995:\n");
  if (t10 == 0)
    goto g2990;
  /* Here if argument TypeFixnum */
  /* t7= -2 */
  t7 = zero - 2;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* reuse t5 as mask */
  t5 = ~t7;
  /* T3= masked new value. */
  t3 = arg4 & ~t7;
  /* t5 is the inplace mask */
  t5 = t5 << (arg2 & 63);
  /* t4 is the shifted field */
  t4 = t3 << (arg2 & 63);
  /* Clear out existing bits in arg2 field */
  t6 = t6 & ~t5;
  /* Put the new bits in */
  t6 = t4 | t6;
  t4 = *(u64 *)&(processor->stackcachebasevma);
  t3 = t2 + ivory;
  t10 = *(s32 *)&processor->scovlimit;
  t5 = (t3 * 4);
  t1 = LDQ_U(t3);
  /* Stack cache offset */
  t4 = t2 - t4;
  /* In range? */
  t10 = ((u64)t4 < (u64)t10) ? 1 : 0;
  t4 = (t8 & 0xff) << ((t3&7)*8);
  t1 = t1 & ~(0xffL << (t3&7)*8);

g2992:
  if (_trace) printf("g2992:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);
  *(u32 *)t5 = t6;
  /* J. if in cache */
  if (t10 != 0)
    goto g2991;
  goto NEXTINSTRUCTION;
  goto NEXTINSTRUCTION;

g2990:
  if (_trace) printf("g2990:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

g2989:
  if (_trace) printf("g2989:\n");

pdpbillop:
  if (_trace) printf("pdpbillop:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);
  t1 = *(u64 *)&(processor->stackcachebasevma);
  /* stack cache base relative offset */
  t2 = iSP - t2;
  /* convert byte address to word address */
  t2 = t2 >> 3;
  /* reconstruct VMA */
  t1 = t2 + t1;
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

g2991:
  if (_trace) printf("g2991:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);

g2996:
  if (_trace) printf("g2996:\n");
  t3 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t4 = t2 - t4;
  /* reconstruct SCA */
  t3 = (t4 * 8) + t3;
  /* Store in stack */
  *(u32 *)t3 = t6;
  /* write the stack cache */
  *(u32 *)(t3 + 4) = t8;
  goto NEXTINSTRUCTION;

g2982:
  if (_trace) printf("g2982:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t3 = (t3 * 8) + t4;
  t6 = *(s32 *)t3;
  /* Read from stack cache */
  t8 = *(s32 *)(t3 + 4);
  goto g2981;

/* end DoPDpb */
  /* End of Halfword operand from stack instruction - DoPDpb */
/* start DoPTagDpb */

  /* Field Extraction instruction - DoPTagDpb */

doptagdpb:
  if (_trace) printf("doptagdpb:\n");
  /* Actually only one entry point, but simulate others for dispatch */
#ifdef TRACING
#endif

DoPTagDpbIM:
  if (_trace) printf("DoPTagDpbIM:\n");

DoPTagDpbSP:
  if (_trace) printf("DoPTagDpbSP:\n");

DoPTagDpbLP:
  if (_trace) printf("DoPTagDpbLP:\n");

DoPTagDpbFP:
  if (_trace) printf("DoPTagDpbFP:\n");
  /* Shift the 'size-1' bits into place */
  arg1 = arg3 >> 37;
  /* mask out the unwanted bits in arg2 */
  arg2 = arg2 & 31;
  /* mask out the unwanted bits in arg1 */
  arg1 = arg1 & 31;
  /* arg1 has size-1, arg2 has position. */
  /* Get arg2 tag/data */
  t2 = *(s32 *)iSP;
  /* Get arg2 tag/data */
  t1 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  t2 = (u32)t2;
  t3 = t1 - Type_PhysicalAddress;
  t3 = t3 & 63;
  if (t3 == 0)
    goto ptagdpbillop;
  /* get arg1 tag/data */
  arg4 = *(s32 *)iSP;
  /* get arg1 tag/data */
  arg3 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  arg4 = (u32)arg4;
  /* Memory Read Internal */

g2997:
  /* Base of stack cache */
  t3 = *(u64 *)&(processor->stackcachebasevma);
  t1 = t2 + ivory;
  t4 = *(s32 *)&processor->scovlimit;
  t8 = (t1 * 4);
  t6 = LDQ_U(t1);
  /* Stack cache offset */
  t3 = t2 - t3;
  /* In range? */
  t4 = ((u64)t3 < (u64)t4) ? 1 : 0;
  t8 = *(s32 *)t8;
  t6 = (u8)(t6 >> ((t1&7)*8));
  if (t4 != 0)
    goto g2999;

g2998:

g3005:
  /* Strip off any CDR code bits. */
  t1 = arg3 & 63;
  t10 = (t1 == Type_Fixnum) ? 1 : 0;

g3012:
  if (_trace) printf("g3012:\n");
  if (t10 == 0)
    goto g3007;
  /* Here if argument TypeFixnum */
  /* t7= -2 */
  t7 = zero - 2;
  /* Unmask */
  t7 = t7 << (arg1 & 63);
  /* reuse t5 as mask */
  t5 = ~t7;
  /* T3= masked new value. */
  t3 = arg4 & ~t7;
  /* t5 is the inplace mask */
  t5 = t5 << (arg2 & 63);
  /* t4 is the shifted field */
  t4 = t3 << (arg2 & 63);
  /* Clear out existing bits in arg2 field */
  t6 = t6 & ~t5;
  /* Put the new bits in */
  t6 = t4 | t6;
  t4 = *(u64 *)&(processor->stackcachebasevma);
  t3 = t2 + ivory;
  t10 = *(s32 *)&processor->scovlimit;
  t5 = (t3 * 4);
  t1 = LDQ_U(t3);
  /* Stack cache offset */
  t4 = t2 - t4;
  /* In range? */
  t10 = ((u64)t4 < (u64)t10) ? 1 : 0;
  t4 = (t6 & 0xff) << ((t3&7)*8);
  t1 = t1 & ~(0xffL << (t3&7)*8);

g3009:
  if (_trace) printf("g3009:\n");
  t1 = t1 | t4;
  STQ_U(t3, t1);
  *(u32 *)t5 = t8;
  /* J. if in cache */
  if (t10 != 0)
    goto g3008;
  goto NEXTINSTRUCTION;
  goto NEXTINSTRUCTION;

g3007:
  if (_trace) printf("g3007:\n");
  /* Here for all other cases */
  arg5 = 0;
  arg2 = 6;
  goto illegaloperand;

g3006:
  if (_trace) printf("g3006:\n");

ptagdpbillop:
  if (_trace) printf("ptagdpbillop:\n");
  /* Convert stack cache address to VMA */
  t2 = *(u64 *)&(processor->stackcachedata);
  t1 = *(u64 *)&(processor->stackcachebasevma);
  /* stack cache base relative offset */
  t2 = iSP - t2;
  /* convert byte address to word address */
  t2 = t2 >> 3;
  /* reconstruct VMA */
  t1 = t2 + t1;
  arg5 = t2;
  arg2 = 57;
  goto illegaloperand;

g3008:
  if (_trace) printf("g3008:\n");
  t4 = *(u64 *)&(processor->stackcachebasevma);

g3013:
  if (_trace) printf("g3013:\n");
  t3 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t4 = t2 - t4;
  /* reconstruct SCA */
  t3 = (t4 * 8) + t3;
  /* Store in stack */
  *(u32 *)t3 = t8;
  /* write the stack cache */
  *(u32 *)(t3 + 4) = t6;
  goto NEXTINSTRUCTION;

g2999:
  if (_trace) printf("g2999:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t3 = (t3 * 8) + t4;
  t8 = *(s32 *)t3;
  /* Read from stack cache */
  t6 = *(s32 *)(t3 + 4);
  goto g2998;

/* end DoPTagDpb */
  /* End of Halfword operand from stack instruction - DoPTagDpb */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunfext.as */
