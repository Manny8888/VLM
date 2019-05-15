
#ifndef _TRAPS_H
#define _TRAPS_H

extern int TakePreTrap(Integer index, LispObj *extra1, LispObj *extra2);
extern int TakeInstructionException(int instruction, LispObj *op2, LispObj *nextpc);

#endif

