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

domessagedispatch : if (_trace) printf("domessagedispatch:\n");

DoMessageDispatchSP : if (_trace) printf("DoMessageDispatchSP:\n");
arg1 = arg5; // Assume SP mode
if (arg2 == 0) // SP-pop mode
    arg1 = iSP;
if (arg2 == 0) // Adjust SP if SP-pop mode
    iSP = arg4;

DoMessageDispatchLP : if (_trace) printf("DoMessageDispatchLP:\n");

DoMessageDispatchFP : if (_trace) printf("DoMessageDispatchFP:\n");

begindomessagedispatch : if (_trace) printf("begindomessagedispatch:\n");
/* arg1 has the operand address. */
arg1 = (arg2 * 8) + arg1; // Compute operand address
arg2 = *(s32 *)&processor->control;
arg1 = *(s32 *)(iFP + 28); // get message tag and data
t1 = *(s32 *)(iFP + 24);
arg5 = arg2 & 255; // get number of arguments
arg3 = *(s32 *)(iFP + 20); // get instance tag and data
arg4 = *(s32 *)(iFP + 16);
arg5 = arg5 - 4; // done if 2 or more arguments (plus 2 extra words)
if ((s64)arg5 < 0)
    goto verifygenericarity;
t1 = (u32)t1;
arg4 = (u32)arg4;
r0 = (u64) && return0035;
goto lookuphandler;
return0035 : arg4 = *(u64 *)(iFP + 16); // clobbered by |LookupHandler|
t3 = t4 - Type_EvenPC;
t3 = t3 & 62; // Strip CDR code, low bits
if (t3 != 0)
    goto message_dispatch44546;
t3 = t6 & 63; // Strip CDR code
t3 = t3 - Type_NIL;
if (t3 == 0)
    goto message_dispatch44544;
*(u32 *)(iFP + 16) = t7;
*(u32 *)(iFP + 20) = t6; // write the stack cache
goto message_dispatch44545;

message_dispatch44544 : if (_trace) printf("message_dispatch44544:\n");
*(u32 *)(iFP + 16) = t1; // swap message/instance in the frame
*(u32 *)(iFP + 20) = arg1; // write the stack cache

message_dispatch44545 : if (_trace) printf("message_dispatch44545:\n");
*(u64 *)(iFP + 24) = arg4;
/* Convert real continuation to PC. */
iPC = t4 & 1;
iPC = t9 + iPC;
iPC = t9 + iPC;
goto interpretinstructionforjump;

message_dispatch44546 : if (_trace) printf("message_dispatch44546:\n");
/* Convert stack cache address to VMA */
t2 = *(u64 *)&(processor->stackcachedata);
t3 = *(u64 *)&(processor->stackcachebasevma);
t2 = iSP - t2; // stack cache base relative offset
t2 = t2 >> 3; // convert byte address to word address
t3 = t2 + t3; // reconstruct VMA
arg5 = t3;
arg2 = 37;
goto illegaloperand;

DoMessageDispatchIM : goto doistageerror;

/* end DoMessageDispatch */
/* End of Halfword operand from stack instruction - DoMessageDispatch */
/* Fin. */

/* End of file automatically generated from ../alpha-emulator/ifungene.as */
