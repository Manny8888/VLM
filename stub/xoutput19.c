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
#ifdef TRACING
#endif

DoBindLocativeToValueIM:
  if (_trace) printf("DoBindLocativeToValueIM:\n");
  /* This sequence only sucks a moderate amount */
  /* sign extend the byte argument. */
  arg2 = arg2 << 56;

g6091:
  if (_trace) printf("g6091:\n");
  /* Rest of sign extension */
  arg2 = (s64)arg2 >> 56;
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = *(u64 *)&(processor->immediate_arg);
  goto begindobindlocativetovalue;
#ifdef TRACING
#endif

DoBindLocativeToValueSP:
  if (_trace) printf("DoBindLocativeToValueSP:\n");
  /* Assume SP mode */
  arg1 = arg5;
  /* SP-pop mode */
  if (arg2 == 0)
    arg1 = iSP;
  /* Adjust SP if SP-pop mode */
  if (arg2 == 0)
    iSP = arg4;
#ifdef TRACING
  goto headdobindlocativetovalue;
#endif

DoBindLocativeToValueLP:
  if (_trace) printf("DoBindLocativeToValueLP:\n");
#ifdef TRACING
  goto headdobindlocativetovalue;
#endif

DoBindLocativeToValueFP:
  if (_trace) printf("DoBindLocativeToValueFP:\n");

headdobindlocativetovalue:
  if (_trace) printf("headdobindlocativetovalue:\n");
  /* Compute operand address */
  arg1 = (arg2 * 8) + arg1;
  /* Get the operand */
  arg1 = *(u64 *)arg1;

begindobindlocativetovalue:
  if (_trace) printf("begindobindlocativetovalue:\n");
  /* arg1 has the operand, sign extended if immediate. */
  /* ltag/ldata */
  arg6 = *(s32 *)iSP;
  /* ltag/ldata */
  arg5 = *(s32 *)(iSP + 4);
  /* Pop Stack. */
  iSP = iSP - 8;
  arg6 = (u32)arg6;
  arg3 = *(u64 *)&(processor->bindingstackpointer);
  /* new tag */
  arg2 = arg1 >> 32;
  arg4 = *(u64 *)&(processor->bindingstacklimit);
  /* new data */
  arg1 = (u32)arg1;
  t1 = arg5 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto bindloctovaliop;
#ifdef MINIMA
  t2 = arg3 >> 32;
#endif
  arg3 = (u32)arg3;
  arg4 = (u32)arg4;
  t1 = arg3 - arg4;
  /* J. if binding stack overflow */
  if ((s64)t1 >= 0)
    goto bindloctovalov;
  t3 = arg3 + 1;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t1 = t2 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto bindloctovaldeep;
#endif
  t9 = *(s32 *)&processor->control;
  t8 = arg6;
  /* Memory Read Internal */

g6059:
  /* Base of stack cache */
  t4 = *(u64 *)&(processor->stackcachebasevma);
  t6 = t8 + ivory;
  t5 = *(s32 *)&processor->scovlimit;
  t1 = (t6 * 4);
  t2 = LDQ_U(t6);
  /* Stack cache offset */
  t4 = t8 - t4;
  t7 = *(u64 *)&(processor->bindread_mask);
  /* In range? */
  t5 = ((u64)t4 < (u64)t5) ? 1 : 0;
  t1 = *(s32 *)t1;
  t2 = (u8)(t2 >> ((t6&7)*8));
  if (t5 != 0)
    goto g6061;

g6060:
  t6 = zero + 224;
  t7 = t7 >> (t2 & 63);
  t6 = t6 >> (t2 & 63);
  if (t7 & 1)
    goto g6063;

g6068:
  t10 = t9 >> 19;
  /* TagType. */
  t8 = arg5 & 63;
  /* Extract the CR.cleanup-bindings bit */
  t10 = t10 & 64;
  t11 = t10 | t8;
  t5 = *(u64 *)&(processor->stackcachebasevma);
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;
  t7 = (t4 * 4);
  t6 = LDQ_U(t4);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* In range? */
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;
  t5 = (t11 & 0xff) << ((t4&7)*8);
  t6 = t6 & ~(0xffL << (t4&7)*8);

g6071:
  if (_trace) printf("g6071:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);
  *(u32 *)t7 = arg6;
  /* J. if in cache */
  if (t8 != 0)
    goto g6070;

g6069:
  t3 = arg3 + 2;
  t5 = *(u64 *)&(processor->stackcachebasevma);
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;
  t7 = (t4 * 4);
  t6 = LDQ_U(t4);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* In range? */
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;
  t5 = (t2 & 0xff) << ((t4&7)*8);
  t6 = t6 & ~(0xffL << (t4&7)*8);

g6074:
  if (_trace) printf("g6074:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);
  *(u32 *)t7 = t1;
  /* J. if in cache */
  if (t8 != 0)
    goto g6073;

g6072:
  t1 = (512) << 16;
  /* Memory Read Internal */

g6075:
  /* Base of stack cache */
  t6 = *(u64 *)&(processor->stackcachebasevma);
  t8 = arg6 + ivory;
  t7 = *(s32 *)&processor->scovlimit;
  t5 = (t8 * 4);
  t4 = LDQ_U(t8);
  /* Stack cache offset */
  t6 = arg6 - t6;
  t10 = *(u64 *)&(processor->bindwrite_mask);
  /* In range? */
  t7 = ((u64)t6 < (u64)t7) ? 1 : 0;
  t5 = *(s32 *)t5;
  t4 = (u8)(t4 >> ((t8&7)*8));
  if (t7 != 0)
    goto g6077;

g6076:
  t8 = zero + 224;
  t10 = t10 >> (t4 & 63);
  t8 = t8 >> (t4 & 63);
  if (t10 & 1)
    goto g6079;

g6084:
  /* Merge cdr-code */
  t5 = arg2 & 63;
  t4 = t4 & 192;
  t4 = t4 | t5;
  t7 = *(u64 *)&(processor->stackcachebasevma);
  t6 = arg6 + ivory;
  t10 = *(s32 *)&processor->scovlimit;
  t5 = (t6 * 4);
  t8 = LDQ_U(t6);
  /* Stack cache offset */
  t7 = arg6 - t7;
  /* In range? */
  t10 = ((u64)t7 < (u64)t10) ? 1 : 0;
  t7 = (t4 & 0xff) << ((t6&7)*8);
  t8 = t8 & ~(0xffL << (t6&7)*8);

g6087:
  if (_trace) printf("g6087:\n");
  t8 = t8 | t7;
  STQ_U(t6, t8);
  *(u32 *)t5 = arg1;
  /* J. if in cache */
  if (t10 != 0)
    goto g6086;

g6085:
  /* Set cr.cleanup-bindings bit */
  t9 = t1 | t9;
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
  /* Get the SP, ->op2 */
  t1 = *(u64 *)&(processor->restartsp);
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);
  t2 = *(u64 *)&(processor->stackcachebasevma);
  /* stack cache base relative offset */
  t3 = t1 - t3;
  /* convert byte address to word address */
  t3 = t3 >> 3;
  /* reconstruct VMA */
  t2 = t3 + t2;
  arg5 = t2;
  arg2 = 72;
  goto illegaloperand;

g6086:
  if (_trace) printf("g6086:\n");
  t7 = *(u64 *)&(processor->stackcachebasevma);

g6088:
  if (_trace) printf("g6088:\n");
  t6 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t7 = arg6 - t7;
  /* reconstruct SCA */
  t6 = (t7 * 8) + t6;
  /* Store in stack */
  *(u32 *)t6 = arg1;
  /* write the stack cache */
  *(u32 *)(t6 + 4) = t4;
  goto g6085;

g6077:
  if (_trace) printf("g6077:\n");
  t7 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t6 = (t6 * 8) + t7;
  t5 = *(s32 *)t6;
  /* Read from stack cache */
  t4 = *(s32 *)(t6 + 4);
  goto g6076;

g6079:
  if (_trace) printf("g6079:\n");
  if ((t8 & 1) == 0)
    goto g6078;
  /* Do the indirect thing */
  arg6 = (u32)t5;
  goto g6075;

g6078:
  if (_trace) printf("g6078:\n");
  /* Load the memory action table for cycle */
  t10 = *(u64 *)&(processor->bindwrite);
  /* TagType. */
  /* Discard the CDR code */
  t8 = t4 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = arg6;
  /* Adjust for a longword load */
  t8 = (t8 * 4) + t10;
  /* Get the memory action */
  t10 = *(s32 *)t8;

g6081:
  /* Perform memory action */
  arg1 = t10;
  arg2 = 3;
  goto performmemoryaction;

g6073:
  if (_trace) printf("g6073:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);

g6089:
  if (_trace) printf("g6089:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* reconstruct SCA */
  t4 = (t5 * 8) + t4;
  /* Store in stack */
  *(u32 *)t4 = t1;
  /* write the stack cache */
  *(u32 *)(t4 + 4) = t2;
  goto g6072;

g6070:
  if (_trace) printf("g6070:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);

g6090:
  if (_trace) printf("g6090:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* reconstruct SCA */
  t4 = (t5 * 8) + t4;
  /* Store in stack */
  *(u32 *)t4 = arg6;
  /* write the stack cache */
  *(u32 *)(t4 + 4) = t11;
  goto g6069;

g6061:
  if (_trace) printf("g6061:\n");
  t5 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t4 = (t4 * 8) + t5;
  t1 = *(s32 *)t4;
  /* Read from stack cache */
  t2 = *(s32 *)(t4 + 4);
  goto g6060;

g6063:
  if (_trace) printf("g6063:\n");
  if ((t6 & 1) == 0)
    goto g6062;
  /* Do the indirect thing */
  t8 = (u32)t1;
  goto g6059;

g6062:
  if (_trace) printf("g6062:\n");
  /* Load the memory action table for cycle */
  t7 = *(u64 *)&(processor->bindread);
  /* TagType. */
  /* Discard the CDR code */
  t6 = t2 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t8;
  /* Adjust for a longword load */
  t6 = (t6 * 4) + t7;
  /* Get the memory action */
  t7 = *(s32 *)t6;

g6065:
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
#ifdef TRACING
#endif

DoBindLocativeSP:
  if (_trace) printf("DoBindLocativeSP:\n");
  /* Assume SP mode */
  arg1 = arg5;
  /* SP-pop mode */
  if (arg2 == 0)
    arg1 = iSP;
  /* Adjust SP if SP-pop mode */
  if (arg2 == 0)
    iSP = arg4;
#ifdef TRACING
  goto begindobindlocative;
#endif

DoBindLocativeLP:
  if (_trace) printf("DoBindLocativeLP:\n");
#ifdef TRACING
  goto begindobindlocative;
#endif

DoBindLocativeFP:
  if (_trace) printf("DoBindLocativeFP:\n");

begindobindlocative:
  if (_trace) printf("begindobindlocative:\n");
  /* arg1 has the operand address. */
  /* Compute operand address */
  arg1 = (arg2 * 8) + arg1;
  /* Get the operand */
  arg1 = *(u64 *)arg1;
  arg3 = *(u64 *)&(processor->bindingstackpointer);
  /* tag */
  arg5 = arg1 >> 32;
  arg4 = *(u64 *)&(processor->bindingstacklimit);
  /* data */
  arg6 = (u32)arg1;
  t1 = arg5 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto bindlociop;
#ifdef MINIMA
  t2 = arg3 >> 32;
#endif
  arg3 = (u32)arg3;
  arg4 = (u32)arg4;
  t1 = arg3 - arg4;
  /* J. if binding stack overflow */
  if ((s64)t1 >= 0)
    goto bindlocov;
  t3 = arg3 + 1;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t1 = t2 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto bindlocdeep;
#endif
  t9 = *(s32 *)&processor->control;
  t8 = arg6;
  /* Memory Read Internal */

g6092:
  /* Base of stack cache */
  t4 = *(u64 *)&(processor->stackcachebasevma);
  t6 = t8 + ivory;
  t5 = *(s32 *)&processor->scovlimit;
  t1 = (t6 * 4);
  t2 = LDQ_U(t6);
  /* Stack cache offset */
  t4 = t8 - t4;
  t7 = *(u64 *)&(processor->bindread_mask);
  /* In range? */
  t5 = ((u64)t4 < (u64)t5) ? 1 : 0;
  t1 = *(s32 *)t1;
  t2 = (u8)(t2 >> ((t6&7)*8));
  if (t5 != 0)
    goto g6094;

g6093:
  t6 = zero + 224;
  t7 = t7 >> (t2 & 63);
  t6 = t6 >> (t2 & 63);
  if (t7 & 1)
    goto g6096;

g6101:
  t10 = t9 >> 19;
  /* TagType. */
  t8 = arg5 & 63;
  /* Extract the CR.cleanup-bindings bit */
  t10 = t10 & 64;
  t11 = t10 | t8;
  t5 = *(u64 *)&(processor->stackcachebasevma);
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;
  t7 = (t4 * 4);
  t6 = LDQ_U(t4);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* In range? */
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;
  t5 = (t11 & 0xff) << ((t4&7)*8);
  t6 = t6 & ~(0xffL << (t4&7)*8);

g6104:
  if (_trace) printf("g6104:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);
  *(u32 *)t7 = arg6;
  /* J. if in cache */
  if (t8 != 0)
    goto g6103;

g6102:
  t3 = arg3 + 2;
  t5 = *(u64 *)&(processor->stackcachebasevma);
  t4 = t3 + ivory;
  t8 = *(s32 *)&processor->scovlimit;
  t7 = (t4 * 4);
  t6 = LDQ_U(t4);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* In range? */
  t8 = ((u64)t5 < (u64)t8) ? 1 : 0;
  t5 = (t2 & 0xff) << ((t4&7)*8);
  t6 = t6 & ~(0xffL << (t4&7)*8);

g6107:
  if (_trace) printf("g6107:\n");
  t6 = t6 | t5;
  STQ_U(t4, t6);
  *(u32 *)t7 = t1;
  /* J. if in cache */
  if (t8 != 0)
    goto g6106;

g6105:
  t1 = (512) << 16;
  /* Set cr.cleanup-bindings bit */
  t9 = t1 | t9;
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
  /* Get the SP, ->op2 */
  t1 = *(u64 *)&(processor->restartsp);
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);
  t2 = *(u64 *)&(processor->stackcachebasevma);
  /* stack cache base relative offset */
  t3 = t1 - t3;
  /* convert byte address to word address */
  t3 = t3 >> 3;
  /* reconstruct VMA */
  t2 = t3 + t2;
  arg5 = t2;
  arg2 = 72;
  goto illegaloperand;

g6106:
  if (_trace) printf("g6106:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);

g6108:
  if (_trace) printf("g6108:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* reconstruct SCA */
  t4 = (t5 * 8) + t4;
  /* Store in stack */
  *(u32 *)t4 = t1;
  /* write the stack cache */
  *(u32 *)(t4 + 4) = t2;
  goto g6105;

g6103:
  if (_trace) printf("g6103:\n");
  t5 = *(u64 *)&(processor->stackcachebasevma);

g6109:
  if (_trace) printf("g6109:\n");
  t4 = *(u64 *)&(processor->stackcachedata);
  /* Stack cache offset */
  t5 = t3 - t5;
  /* reconstruct SCA */
  t4 = (t5 * 8) + t4;
  /* Store in stack */
  *(u32 *)t4 = arg6;
  /* write the stack cache */
  *(u32 *)(t4 + 4) = t11;
  goto g6102;

g6094:
  if (_trace) printf("g6094:\n");
  t5 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t4 = (t4 * 8) + t5;
  t1 = *(s32 *)t4;
  /* Read from stack cache */
  t2 = *(s32 *)(t4 + 4);
  goto g6093;

g6096:
  if (_trace) printf("g6096:\n");
  if ((t6 & 1) == 0)
    goto g6095;
  /* Do the indirect thing */
  t8 = (u32)t1;
  goto g6092;

g6095:
  if (_trace) printf("g6095:\n");
  /* Load the memory action table for cycle */
  t7 = *(u64 *)&(processor->bindread);
  /* TagType. */
  /* Discard the CDR code */
  t6 = t2 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t8;
  /* Adjust for a longword load */
  t6 = (t6 * 4) + t7;
  /* Get the memory action */
  t7 = *(s32 *)t6;

g6098:
  /* Perform memory action */
  arg1 = t7;
  arg2 = 2;
  goto performmemoryaction;
#ifdef TRACING
#endif

DoBindLocativeIM:
  goto doistageerror;

/* end DoBindLocative */
  /* End of Halfword operand from stack instruction - DoBindLocative */
/* start DoUnbindN */

  /* Halfword operand from stack instruction - DoUnbindN */
  /* arg2 has the preloaded 8 bit operand. */

dounbindn:
  if (_trace) printf("dounbindn:\n");
#ifdef TRACING
#endif

DoUnbindNIM:
  if (_trace) printf("DoUnbindNIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = *(u64 *)&(processor->immediate_arg);
  goto begindounbindn;
#ifdef TRACING
#endif

DoUnbindNSP:
  if (_trace) printf("DoUnbindNSP:\n");
  /* Assume SP mode */
  arg1 = arg5;
  /* SP-pop mode */
  if (arg2 == 0)
    arg1 = iSP;
  /* Adjust SP if SP-pop mode */
  if (arg2 == 0)
    iSP = arg4;
#ifdef TRACING
  goto headdounbindn;
#endif

DoUnbindNLP:
  if (_trace) printf("DoUnbindNLP:\n");
#ifdef TRACING
  goto headdounbindn;
#endif

DoUnbindNFP:
  if (_trace) printf("DoUnbindNFP:\n");

headdounbindn:
  if (_trace) printf("headdounbindn:\n");
  /* Compute operand address */
  arg1 = (arg2 * 8) + arg1;
  /* Get the operand */
  arg1 = *(u64 *)arg1;

begindounbindn:
  if (_trace) printf("begindounbindn:\n");
  /* arg1 has the operand, not sign extended if immediate. */
#ifdef MINIMA
  arg3 = *(u64 *)&(processor->bindingstackpointer);
#endif
  arg2 = arg1 >> 32;
  arg1 = (u32)arg1;
  t1 = arg2 - Type_Fixnum;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto unbindniop;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t2 = arg3 >> 32;
  t1 = t2 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto unbindndeep;
#endif
  t11 = *(u64 *)&(processor->stackcachebasevma);
  /* Size of the stack cache (words) */
  t12 = *(s32 *)&processor->scovlimit;
  goto unbindnendloop;

unbindntoploop:
  if (_trace) printf("unbindntoploop:\n");
  arg1 = arg1 - 1;
  t1 = *(u64 *)&(processor->bindingstackpointer);
  t4 = *(s32 *)&processor->control;
  /* vma only */
  t1 = (u32)t1;
  t2 = (512) << 16;
  t5 = t1 - 1;
  t3 = t4 & t2;
  /* Turn off the bit */
  t4 = t4 & ~t2;
  if (t3 != 0)
    goto g6110;
  /* Get the SP, ->op2 */
  t4 = *(u64 *)&(processor->restartsp);
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g6110:
  if (_trace) printf("g6110:\n");
  /* Memory Read Internal */

g6111:
  arg4 = t1 + ivory;
  t6 = (arg4 * 4);
  t7 = LDQ_U(arg4);
  /* Stack cache offset */
  t8 = t1 - t11;
  arg5 = *(u64 *)&(processor->bindread_mask);
  /* In range? */
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;
  t6 = *(s32 *)t6;
  t7 = (u8)(t7 >> ((arg4&7)*8));
  if (arg3 != 0)
    goto g6113;

g6112:
  arg4 = zero + 224;
  arg5 = arg5 >> (t7 & 63);
  arg4 = arg4 >> (t7 & 63);
  if (arg5 & 1)
    goto g6115;

g6120:
  /* Memory Read Internal */

g6121:
  arg4 = t5 + ivory;
  t2 = (arg4 * 4);
  t3 = LDQ_U(arg4);
  /* Stack cache offset */
  t8 = t5 - t11;
  arg5 = *(u64 *)&(processor->bindread_mask);
  /* In range? */
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;
  t2 = *(s32 *)t2;
  t3 = (u8)(t3 >> ((arg4&7)*8));
  if (arg3 != 0)
    goto g6123;

g6122:
  arg4 = zero + 224;
  arg5 = arg5 >> (t3 & 63);
  arg4 = arg4 >> (t3 & 63);
  t2 = (u32)t2;
  if (arg5 & 1)
    goto g6125;

g6130:
  /* Memory Read Internal */

g6131:
  arg6 = t2 + ivory;
  arg3 = (arg6 * 4);
  t8 = LDQ_U(arg6);
  /* Stack cache offset */
  arg4 = t2 - t11;
  /* In range? */
  arg5 = ((u64)arg4 < (u64)t12) ? 1 : 0;
  arg3 = *(s32 *)arg3;
  t8 = (u8)(t8 >> ((arg6&7)*8));
  if (arg5 != 0)
    goto g6133;

g6132:
  arg4 = *(u64 *)&(processor->bindwrite_mask);
  arg6 = zero + 224;
  arg4 = arg4 >> (t8 & 63);
  arg6 = arg6 >> (t8 & 63);
  if (arg4 & 1)
    goto g6135;

g6140:
  /* Merge cdr-code */
  arg3 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | arg3;
  arg4 = t2 + ivory;
  arg3 = (arg4 * 4);
  arg6 = LDQ_U(arg4);
  arg5 = (t8 & 0xff) << ((arg4&7)*8);
  arg6 = arg6 & ~(0xffL << (arg4&7)*8);

g6143:
  if (_trace) printf("g6143:\n");
  arg6 = arg6 | arg5;
  STQ_U(arg4, arg6);
  arg4 = *(s32 *)&processor->scovlimit;
  /* Stack cache offset */
  arg5 = t2 - t11;
  /* In range? */
  arg4 = ((u64)arg5 < (u64)arg4) ? 1 : 0;
  *(u32 *)arg3 = t6;
  /* J. if in cache */
  if (arg4 != 0)
    goto g6142;

g6141:
  /* Get the old cleanup-bindings bit */
  t3 = t3 & 64;
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
  /* Get the SP, ->op2 */
  t1 = *(u64 *)&(processor->restartsp);
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);
  t2 = *(u64 *)&(processor->stackcachebasevma);
  /* stack cache base relative offset */
  t3 = t1 - t3;
  /* convert byte address to word address */
  t3 = t3 >> 3;
  /* reconstruct VMA */
  t2 = t3 + t2;
  arg5 = t2;
  arg2 = 72;
  goto illegaloperand;
#endif

g6142:
  if (_trace) printf("g6142:\n");
  arg4 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  arg4 = (arg5 * 8) + arg4;
  /* Store in stack */
  *(u32 *)arg4 = t6;
  /* write the stack cache */
  *(u32 *)(arg4 + 4) = t8;
  goto g6141;

g6133:
  if (_trace) printf("g6133:\n");
  arg5 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  arg4 = (arg4 * 8) + arg5;
  arg3 = *(s32 *)arg4;
  /* Read from stack cache */
  t8 = *(s32 *)(arg4 + 4);
  goto g6132;

g6135:
  if (_trace) printf("g6135:\n");
  if ((arg6 & 1) == 0)
    goto g6134;
  /* Do the indirect thing */
  t2 = (u32)arg3;
  goto g6131;

g6134:
  if (_trace) printf("g6134:\n");
  /* Load the memory action table for cycle */
  arg4 = *(u64 *)&(processor->bindwrite);
  /* TagType. */
  /* Discard the CDR code */
  arg6 = t8 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t2;
  /* Adjust for a longword load */
  arg6 = (arg6 * 4) + arg4;
  /* Get the memory action */
  arg4 = *(s32 *)arg6;

g6137:
  /* Perform memory action */
  arg1 = arg4;
  arg2 = 3;
  goto performmemoryaction;

g6123:
  if (_trace) printf("g6123:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t8 = (t8 * 8) + arg3;
  t2 = *(s32 *)t8;
  /* Read from stack cache */
  t3 = *(s32 *)(t8 + 4);
  goto g6122;

g6125:
  if (_trace) printf("g6125:\n");
  if ((arg4 & 1) == 0)
    goto g6124;
  /* Do the indirect thing */
  t5 = (u32)t2;
  goto g6121;

g6124:
  if (_trace) printf("g6124:\n");
  /* Load the memory action table for cycle */
  arg5 = *(u64 *)&(processor->bindread);
  /* TagType. */
  /* Discard the CDR code */
  arg4 = t3 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t5;
  /* Adjust for a longword load */
  arg4 = (arg4 * 4) + arg5;
  /* Get the memory action */
  arg5 = *(s32 *)arg4;

g6127:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

g6113:
  if (_trace) printf("g6113:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t8 = (t8 * 8) + arg3;
  t6 = *(s32 *)t8;
  /* Read from stack cache */
  t7 = *(s32 *)(t8 + 4);
  goto g6112;

g6115:
  if (_trace) printf("g6115:\n");
  if ((arg4 & 1) == 0)
    goto g6114;
  /* Do the indirect thing */
  t1 = (u32)t6;
  goto g6111;

g6114:
  if (_trace) printf("g6114:\n");
  /* Load the memory action table for cycle */
  arg5 = *(u64 *)&(processor->bindread);
  /* TagType. */
  /* Discard the CDR code */
  arg4 = t7 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t1;
  /* Adjust for a longword load */
  arg4 = (arg4 * 4) + arg5;
  /* Get the memory action */
  arg5 = *(s32 *)arg4;

g6117:
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
#ifdef TRACING
#endif

DoRestoreBindingStackIM:
  if (_trace) printf("DoRestoreBindingStackIM:\n");
  /* This sequence is lukewarm */
  *(u32 *)&processor->immediate_arg = arg2;
  arg1 = *(u64 *)&(processor->immediate_arg);
  goto begindorestorebindingstack;
#ifdef TRACING
#endif

DoRestoreBindingStackSP:
  if (_trace) printf("DoRestoreBindingStackSP:\n");
  /* Assume SP mode */
  arg1 = arg5;
  /* SP-pop mode */
  if (arg2 == 0)
    arg1 = iSP;
  /* Adjust SP if SP-pop mode */
  if (arg2 == 0)
    iSP = arg4;
#ifdef TRACING
  goto headdorestorebindingstack;
#endif

DoRestoreBindingStackLP:
  if (_trace) printf("DoRestoreBindingStackLP:\n");
#ifdef TRACING
  goto headdorestorebindingstack;
#endif

DoRestoreBindingStackFP:
  if (_trace) printf("DoRestoreBindingStackFP:\n");

headdorestorebindingstack:
  if (_trace) printf("headdorestorebindingstack:\n");
  /* Compute operand address */
  arg1 = (arg2 * 8) + arg1;
  /* Get the operand */
  arg1 = *(u64 *)arg1;

begindorestorebindingstack:
  if (_trace) printf("begindorestorebindingstack:\n");
  /* arg1 has the operand, not sign extended if immediate. */
#ifdef MINIMA
  arg3 = *(u64 *)&(processor->bindingstackpointer);
#endif
  arg2 = arg1 >> 32;
  arg1 = (u32)arg1;
  t1 = arg2 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto restorebsiop;
#ifdef MINIMA
  /* BSP not a locative -> Deep-bound */
  t2 = arg3 >> 32;
  t1 = t2 - Type_Locative;
  /* Strip CDR code */
  t1 = t1 & 63;
  if (t1 != 0)
    goto restorebsdeep;
#endif
  t1 = *(u64 *)&(processor->bindingstackpointer);
  t11 = *(u64 *)&(processor->stackcachebasevma);
  /* Size of the stack cache (words) */
  t12 = *(s32 *)&processor->scovlimit;
  goto restorebsendloop;

restorebstoploop:
  if (_trace) printf("restorebstoploop:\n");
  t1 = *(u64 *)&(processor->bindingstackpointer);
  t4 = *(s32 *)&processor->control;
  /* vma only */
  t1 = (u32)t1;
  t2 = (512) << 16;
  t5 = t1 - 1;
  t3 = t4 & t2;
  /* Turn off the bit */
  t4 = t4 & ~t2;
  if (t3 != 0)
    goto g6144;
  /* Get the SP, ->op2 */
  t4 = *(u64 *)&(processor->restartsp);
  arg5 = 0;
  arg2 = 20;
  goto illegaloperand;

g6144:
  if (_trace) printf("g6144:\n");
  /* Memory Read Internal */

g6145:
  arg4 = t1 + ivory;
  t6 = (arg4 * 4);
  t7 = LDQ_U(arg4);
  /* Stack cache offset */
  t8 = t1 - t11;
  arg5 = *(u64 *)&(processor->bindread_mask);
  /* In range? */
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;
  t6 = *(s32 *)t6;
  t7 = (u8)(t7 >> ((arg4&7)*8));
  if (arg3 != 0)
    goto g6147;

g6146:
  arg4 = zero + 224;
  arg5 = arg5 >> (t7 & 63);
  arg4 = arg4 >> (t7 & 63);
  if (arg5 & 1)
    goto g6149;

g6154:
  /* Memory Read Internal */

g6155:
  arg4 = t5 + ivory;
  t2 = (arg4 * 4);
  t3 = LDQ_U(arg4);
  /* Stack cache offset */
  t8 = t5 - t11;
  arg5 = *(u64 *)&(processor->bindread_mask);
  /* In range? */
  arg3 = ((u64)t8 < (u64)t12) ? 1 : 0;
  t2 = *(s32 *)t2;
  t3 = (u8)(t3 >> ((arg4&7)*8));
  if (arg3 != 0)
    goto g6157;

g6156:
  arg4 = zero + 224;
  arg5 = arg5 >> (t3 & 63);
  arg4 = arg4 >> (t3 & 63);
  t2 = (u32)t2;
  if (arg5 & 1)
    goto g6159;

g6164:
  /* Memory Read Internal */

g6165:
  arg6 = t2 + ivory;
  arg3 = (arg6 * 4);
  t8 = LDQ_U(arg6);
  /* Stack cache offset */
  arg4 = t2 - t11;
  /* In range? */
  arg5 = ((u64)arg4 < (u64)t12) ? 1 : 0;
  arg3 = *(s32 *)arg3;
  t8 = (u8)(t8 >> ((arg6&7)*8));
  if (arg5 != 0)
    goto g6167;

g6166:
  arg4 = *(u64 *)&(processor->bindwrite_mask);
  arg6 = zero + 224;
  arg4 = arg4 >> (t8 & 63);
  arg6 = arg6 >> (t8 & 63);
  if (arg4 & 1)
    goto g6169;

g6174:
  /* Merge cdr-code */
  arg3 = t7 & 63;
  t8 = t8 & 192;
  t8 = t8 | arg3;
  arg4 = t2 + ivory;
  arg3 = (arg4 * 4);
  arg6 = LDQ_U(arg4);
  arg5 = (t8 & 0xff) << ((arg4&7)*8);
  arg6 = arg6 & ~(0xffL << (arg4&7)*8);

g6177:
  if (_trace) printf("g6177:\n");
  arg6 = arg6 | arg5;
  STQ_U(arg4, arg6);
  arg4 = *(s32 *)&processor->scovlimit;
  /* Stack cache offset */
  arg5 = t2 - t11;
  /* In range? */
  arg4 = ((u64)arg5 < (u64)arg4) ? 1 : 0;
  *(u32 *)arg3 = t6;
  /* J. if in cache */
  if (arg4 != 0)
    goto g6176;

g6175:
  /* Get the old cleanup-bindings bit */
  t3 = t3 & 64;
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
  /* Get the SP, ->op2 */
  t1 = *(u64 *)&(processor->restartsp);
  /* Convert stack cache address to VMA */
  t3 = *(u64 *)&(processor->stackcachedata);
  t2 = *(u64 *)&(processor->stackcachebasevma);
  /* stack cache base relative offset */
  t3 = t1 - t3;
  /* convert byte address to word address */
  t3 = t3 >> 3;
  /* reconstruct VMA */
  t2 = t3 + t2;
  arg5 = t2;
  arg2 = 66;
  goto illegaloperand;
#endif

g6176:
  if (_trace) printf("g6176:\n");
  arg4 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  arg4 = (arg5 * 8) + arg4;
  /* Store in stack */
  *(u32 *)arg4 = t6;
  /* write the stack cache */
  *(u32 *)(arg4 + 4) = t8;
  goto g6175;

g6167:
  if (_trace) printf("g6167:\n");
  arg5 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  arg4 = (arg4 * 8) + arg5;
  arg3 = *(s32 *)arg4;
  /* Read from stack cache */
  t8 = *(s32 *)(arg4 + 4);
  goto g6166;

g6169:
  if (_trace) printf("g6169:\n");
  if ((arg6 & 1) == 0)
    goto g6168;
  /* Do the indirect thing */
  t2 = (u32)arg3;
  goto g6165;

g6168:
  if (_trace) printf("g6168:\n");
  /* Load the memory action table for cycle */
  arg4 = *(u64 *)&(processor->bindwrite);
  /* TagType. */
  /* Discard the CDR code */
  arg6 = t8 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t2;
  /* Adjust for a longword load */
  arg6 = (arg6 * 4) + arg4;
  /* Get the memory action */
  arg4 = *(s32 *)arg6;

g6171:
  /* Perform memory action */
  arg1 = arg4;
  arg2 = 3;
  goto performmemoryaction;

g6157:
  if (_trace) printf("g6157:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t8 = (t8 * 8) + arg3;
  t2 = *(s32 *)t8;
  /* Read from stack cache */
  t3 = *(s32 *)(t8 + 4);
  goto g6156;

g6159:
  if (_trace) printf("g6159:\n");
  if ((arg4 & 1) == 0)
    goto g6158;
  /* Do the indirect thing */
  t5 = (u32)t2;
  goto g6155;

g6158:
  if (_trace) printf("g6158:\n");
  /* Load the memory action table for cycle */
  arg5 = *(u64 *)&(processor->bindread);
  /* TagType. */
  /* Discard the CDR code */
  arg4 = t3 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t5;
  /* Adjust for a longword load */
  arg4 = (arg4 * 4) + arg5;
  /* Get the memory action */
  arg5 = *(s32 *)arg4;

g6161:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

g6147:
  if (_trace) printf("g6147:\n");
  arg3 = *(u64 *)&(processor->stackcachedata);
  /* reconstruct SCA */
  t8 = (t8 * 8) + arg3;
  t6 = *(s32 *)t8;
  /* Read from stack cache */
  t7 = *(s32 *)(t8 + 4);
  goto g6146;

g6149:
  if (_trace) printf("g6149:\n");
  if ((arg4 & 1) == 0)
    goto g6148;
  /* Do the indirect thing */
  t1 = (u32)t6;
  goto g6145;

g6148:
  if (_trace) printf("g6148:\n");
  /* Load the memory action table for cycle */
  arg5 = *(u64 *)&(processor->bindread);
  /* TagType. */
  /* Discard the CDR code */
  arg4 = t7 & 63;
  /* stash the VMA for the (likely) trap */
  *(u64 *)&processor->vma = t1;
  /* Adjust for a longword load */
  arg4 = (arg4 * 4) + arg5;
  /* Get the memory action */
  arg5 = *(s32 *)arg4;

g6151:
  /* Perform memory action */
  arg1 = arg5;
  arg2 = 2;
  goto performmemoryaction;

/* end DoRestoreBindingStack */
  /* End of Halfword operand from stack instruction - DoRestoreBindingStack */
  /* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifunbind.as */