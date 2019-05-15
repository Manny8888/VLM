/************************************************************************
 * WARNING: DO NOT EDIT THIS FILE.  THIS FILE WAS AUTOMATICALLY GENERATED
 * FROM ../alpha-emulator/ifungene.as. ANY CHANGES MADE TO THIS FILE WILL BE LOST
 ************************************************************************/

/* Generic dispatching an method lookup */
/* start DoMessageDispatch */

/* Halfword operand from stack instruction - DoMessageDispatch */
/* arg2 has the preloaded 8 bit operand. */

domessagedispatch:
if ( _trace ) printf ( "domessagedispatch:\n" );
#ifdef TRACING
#endif

DoMessageDispatchSP:
if ( _trace ) printf ( "DoMessageDispatchSP:\n" );
/* Assume SP mode */
arg1 = arg5;
/* SP-pop mode */
if ( arg2 == 0 )
    arg1 = iSP;
/* Adjust SP if SP-pop mode */
if ( arg2 == 0 )
    iSP = arg4;
#ifdef TRACING
goto begindomessagedispatch;
#endif

DoMessageDispatchLP:
if ( _trace ) printf ( "DoMessageDispatchLP:\n" );
#ifdef TRACING
goto begindomessagedispatch;
#endif

DoMessageDispatchFP:
if ( _trace ) printf ( "DoMessageDispatchFP:\n" );

begindomessagedispatch:
if ( _trace ) printf ( "begindomessagedispatch:\n" );
/* arg1 has the operand address. */
/* Compute operand address */
arg1 = ( arg2 * 8 ) + arg1;
arg2 = * ( s32 * )&processor->control;
/* get message tag and data */
arg1 = * ( s32 * ) ( iFP + 28 );
t1 = * ( s32 * ) ( iFP + 24 );
/* get number of arguments */
arg5 = arg2 & 255;
/* get instance tag and data */
arg3 = * ( s32 * ) ( iFP + 20 );
arg4 = * ( s32 * ) ( iFP + 16 );
/* done if 2 or more arguments (plus 2 extra words) */
arg5 = arg5 - 4;
if ( ( s64 ) arg5 < 0 )
    goto verifygenericarity;
t1 = ( u32 ) t1;
arg4 = ( u32 ) arg4;
r0 = ( u64 ) &&return0035;
goto lookuphandler;
return0035:
/* clobbered by |LookupHandler| */
arg4 = * ( u64 * ) ( iFP + 16 );
t3 = t4 - Type_EvenPC;
/* Strip CDR code, low bits */
t3 = t3 & 62;
if ( t3 != 0 )
    goto g6970;
/* Strip CDR code */
t3 = t6 & 63;
t3 = t3 - Type_NIL;
if ( t3 == 0 )
    goto g6968;
* ( u32 * ) ( iFP + 16 ) = t7;
/* write the stack cache */
* ( u32 * ) ( iFP + 20 ) = t6;
goto g6969;

g6968:
if ( _trace ) printf ( "g6968:\n" );
/* swap message/instance in the frame */
* ( u32 * ) ( iFP + 16 ) = t1;
/* write the stack cache */
* ( u32 * ) ( iFP + 20 ) = arg1;

g6969:
if ( _trace ) printf ( "g6969:\n" );
* ( u64 * ) ( iFP + 24 ) = arg4;
/* Convert real continuation to PC. */
iPC = t4 & 1;
iPC = t9 + iPC;
iPC = t9 + iPC;
goto interpretinstructionforjump;

g6970:
if ( _trace ) printf ( "g6970:\n" );
/* Convert stack cache address to VMA */
t2 = * ( u64 * ) & ( processor->stackcachedata );
t3 = * ( u64 * ) & ( processor->stackcachebasevma );
/* stack cache base relative offset */
t2 = iSP - t2;
/* convert byte address to word address */
t2 = t2 >> 3;
/* reconstruct VMA */
t3 = t2 + t3;
arg5 = t3;
arg2 = 37;
goto illegaloperand;
#ifdef TRACING
#endif

DoMessageDispatchIM:
goto doistageerror;

/* end DoMessageDispatch */
/* End of Halfword operand from stack instruction - DoMessageDispatch */
/* Fin. */



/* End of file automatically generated from ../alpha-emulator/ifungene.as */
