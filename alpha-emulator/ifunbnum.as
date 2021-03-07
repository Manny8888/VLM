;
-* - Mode;
:
LISP;
Common - Lisp;
ALPHA - AXP - INTERNALS;
10;
T - * -
;(include-header; "aihead.s";)
(include - header;
"aistat.s";
)
(include - header;
"ifunhead.s";
)
(comment; "Bignums.";)

no;
stack;
level;
change
(define-instruction |DoAddBignumStep|; :operand-from-stack-immediate ()
    (LDL; arg2; 0 (isp); "Get arg2";)
    (LDL; t2; 4 (isp); "and its tag";)
    (SRL; arg1; 32; t3;)
    (EXTLL; arg1; 0; arg1; "Strip type from arg3";)
    (CheckDataType; t3 |TypeFixnum| addbignumsteplose; t4;)
    (LDL; arg3 -8 (isp); "Get arg1";)
    (LDL; t1 -4 (isp); "and its tag";)
    (EXTLL; arg2; 0; arg2; "Clear sign extension from arg2";)
    (CheckDataType; t2 |TypeFixnum| addbignumsteplose; t4;)
    (EXTLL; arg3; 0; arg3; "Clear sign extension";)
    (CheckDataType; t1 |TypeFixnum| addbignumsteplose; t4;)
    (ADDQ; arg1; arg2; arg4;)
    (ADDQ; arg3; arg4; arg5;)
    (SRL; arg5; 32; arg6; "Shift the carry into arg6";)
T1;
has | TypeFixnum | in it;
here
(GetNextPCandCP)
    (stack-write2-disp; iSP -8; t1; arg5; "Store fixnum result";)
    (stack-write2; iSP; t1; arg6; "Store the carry if any";)
    (ContinueToNextInstruction-NoStall)
  (label; addbignumsteplose;)
    (illegal-operand; three-operand-fixnum-type-error;))

no;
stack;
level;
change
(define-instruction |DoSubBignumStep|; :operand-from-stack-immediate ()
    (LDL; arg2; 0 (isp); "Get arg2";)
    (LDL; t2; 4 (isp); "and its tag";)
    (SRL; arg1; 32; t3;)
    (EXTLL; arg1; 0; arg1; "Strip type from arg3";)
    (CheckDataType; t3 |TypeFixnum| subbignumsteplose; t4;)
    (LDL; arg3 -8 (isp); "Get arg1";)
    (LDL; t1 -4 (isp); "and its tag";)
    (EXTLL; arg2; 0; arg2; "Clear sign extension from arg2";)
    (CheckDataType; t2 |TypeFixnum| subbignumsteplose; t4;)
    (EXTLL; arg3; 0; arg3; "Clear sign extension";)
    (CheckDataType; t1 |TypeFixnum| subbignumsteplose; t4;)
    (SUBQ; arg3; arg2; arg4; "arg1-arg2";)
    (CMPLT; arg4; zero; arg6; "arg6=1 if we borrowed in 1st step";)
    (EXTLL; arg4; 0; arg4; "Truncate 1st step to 32-bits";)
    (SUBQ; arg4; arg1; arg5; "(arg1-arg2)-arg3";)
    (CMPLT; arg5; zero; t6; "t6=1 if we borrowed in 2nd step";)
T1;
has | TypeFixnum | in it;
here
(GetNextPCandCP)
    (stack-write2-disp; iSP -8; t1; arg5; "Store fixnum result";)
    (ADDQ; arg6; t6; arg6; "Compute borrow";)
    (stack-write2; iSP; t1; arg6; "Store the borrow if any";)
    (ContinueToNextInstruction-NoStall)
  (label; subbignumsteplose;)
    (illegal-operand; three-operand-fixnum-type-error;))

(define-instruction |DoMultiplyBignumStep|; :operand-from-stack-immediate ()
    (LDL; arg2; 0 (isp); "Get arg1";)
    (LDL; t1; 4 (isp);)
    (SRL; arg1; 32; t2;)
    (EXTLL; arg1; 0; arg1; "Strip type from arg2";)
    (CheckDataType; t2 |TypeFixnum| multbignumsteplose; t4;)
    (EXTLL; arg2; 0; arg2;)
    (CheckDataType; t1 |TypeFixnum| multbignumsteplose; t4;)
    (MULQ; arg2; arg1; arg3; "arg1*arg2";)
    (EXTLL; arg3; 4; arg6; "arg6=high order word";)
T1;
has | TypeFixnum | in it;
here
(GetNextPCandCP)
    (stack-write2; iSP; t1; arg3; "Store fixnum result ls word";)
    (stack-push2-;with-cdr t1; arg6; "Store ms word";)
    (ContinueToNextInstruction-NoStall)
  (label; multbignumsteplose;)
    (illegal-operand; two-operand-fixnum-type-error;))

+++Needs;
to;
signal;
DIVIDE - OVERFLOW;
if final carry is non - zero
(define-instruction |DoDivideBignumStep|; :operand-from-stack-immediate ()
    (LDL; arg2; 0 (isp); "Get arg2";)
    (LDL; t1; 4 (isp);)
    (SRL; arg1; 32; t2;)
(EXTLL;
arg1;
0;
arg1;
)
this is an;
unsigned;
divide
(CheckDataType; t2 |TypeFixnum| divbignumsteplose1; t4;)
    (BEQ; arg1; divbignumsteplose2; "J. if division by zero";)
    (EXTLL; arg2; 0; arg2;)
    (LDL; arg3 -8 (isp); "Get arg1";)
    (LDL; t3 -4 (isp);)
    (CheckDataType; t1 |TypeFixnum| divbignumsteplose1; t4;)
    (SLL; arg2; 32; arg2; "arg2=(ash arg2 32)";)
    (EXTLL; arg3; 0; arg3;)
    (CheckDataType; t3 |TypeFixnum| divbignumsteplose1; t4;)
    (BIS; arg3; arg2; arg4; "arg1+(ash arg2 32)";)
    (DIVQU; arg4; arg1; t1; "t1 is now the quotient";)
    (MULQ; t1; arg1; t2;)
    (SUBQ; arg4; t2; t2; "t2 is now the remainder";)
    (STL; t1 -8 (iSP); "store quotient (already fixnum)";)
    (STL; t2; 0 (iSP); "store remainder (already fixnum)";)
    (ContinueToNextInstruction)
  (label; divbignumsteplose1;)
    (illegal-operand; three-operand-fixnum-type-error;)
  (label; divbignumsteplose2;)
    (illegal-operand %divide-bignum-step-not-fixnum-or-zero);)

(define-instruction |DoLshcBignumStep|; :operand-from-stack-signed-immediate ()
    (LDL; arg2; 0 (isp); "Get arg2";)
    (LDL; t2; 4 (isp);)
    (SUBQ; isp; 8; isp; "Pop Stack";)
    (SRL; arg1; 32; t3;)
    (EXTLL; arg1; 0; arg1; "Strip type from arg3";)
    (CheckDataType; t3 |TypeFixnum| lshcbignumsteplose; t4;)
    (EXTLL; arg2; 0; arg2;)
    (LDL; arg3; 0 (isp); "Get arg1";)
    (LDL; t1; 4 (isp);)
    (CheckDataType; t2 |TypeFixnum| lshcbignumsteplose; t4;)
    (SLL; arg2; 32; arg2; "arg2=(ash arg2 32)";)
    (EXTLL; arg3; 0; arg3;)
    (CheckDataType; t1 |TypeFixnum| lshcbignumsteplose; t4;)
    (BIS; arg3; arg2; arg4; "arg1+(ash arg2 32)";)
    (SLL; arg4; arg1; arg5;)
    (SRA; arg5; 32; arg6; "Extract the result";)
T1;
has | TypeFixnum | in it;
here
(GetNextPCandCP)
    (stack-write2; iSP; t1; arg6; "Store the result as a fixnum";)
    (ContinueToNextInstruction-NoStall)
  (label; lshcbignumsteplose;)
    (illegal-operand; three-operand-fixnum-type-error;))


(comment; "Fin.";)
