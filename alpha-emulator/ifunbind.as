
;(include-header; "aihead.s";)
(include - header;
"aistat.s";
)
(include - header;
"ifunhead.s";
)
(comment; "Binding Instructions.";)

+++Figure;
out;
if we can;

use

WITH - MULTIPLE - MEMORY - READS
(define-instruction |DoBindLocativeToValue|; :operand-from-stack-signed-immediate ()
    (stack-pop2; arg5; arg6; "ltag/ldata";)
    (LDQ; arg3; PROCESSORSTATE_BINDINGSTACKPOINTER (ivory);)
    (SRL; arg1; 32; arg2; "new tag";)
    (LDQ; arg4; PROCESSORSTATE_BINDINGSTACKLIMIT (ivory);)
    (EXTLL; arg1; 0; arg1; "new data";)
    (CheckDataType; arg5 |TypeLocative| bindloctovaliop; t1;)
    (EXTLL; arg3; 0; arg3;)
    (EXTLL; arg4; 0; arg4;)
    (SUBQ; arg3; arg4; t1;)
    (BGE; t1; bindloctovalov; "J. if binding stack overflow";)
    (ADDQ; arg3; 1; t3;)
    (get-control-register; t9;)
    (BIS; arg6; zero; t8;)
    (memory-read; t8; t2; t1; PROCESSORSTATE_BINDREAD; t4; t5; t6; t7; nil; t;)
set;
the;
ls;
cdcode;
bit;
for ltag ifcleanupbindings
(SRL; t9; #.(- 25; 6;) t10;)
    (TagType; arg5; t8;)
    (AND; t10; #x40; t10; "Extract the CR.cleanup-bindings bit";)
    (BIS; t10; t8; t11;)
    (memory-write; t3; t11; arg6; PROCESSORSTATE_RAW; t4; t5; t6; t7; t8;)
    (ADDQ; arg3; 2; t3;)
    (memory-write; t3; t2; t1; PROCESSORSTATE_RAW; t4; t5; t6; t7; t8;)
    (load-constant; t1; #.1_25; "cr.cleanup-bindings";)
    (store-contents; arg6; arg2; arg1; PROCESSORSTATE_BINDWRITE; t4; t5; t6; t7; t8; t10;)
    (BIS; t1; t9; t9; "Set cr.cleanup-bindings bit";)
    (set-control-register; t9;)
    (STL; t3; PROCESSORSTATE_BINDINGSTACKPOINTER (ivory); "vma only";)
    (ContinueToNextInstruction)
  (label; bindloctovalov;)
    (illegal-operand; binding-stack-overflow;)
(label;
bindloctovaliop;
)
+++exception;
if spare pointer;
type
(illegal-operand; bind-locative-type-error;)
  (label; bindloctovaldeep;)
    (LDQ; t1; PROCESSORSTATE_RESTARTSP (ivory); "Get the SP, ->op2";)
    (SCAtoVMA; t1; t2; t3;)
    (illegal-operand; shallow-binding-operation-in-deep-binding-mode; t2;))

+++Figure;
out;
if we can;

use

WITH - MULTIPLE - MEMORY - READS
(define-instruction |DoBindLocative|; :operand-from-stack ()
    (LDQ; arg1; 0 (arg1); "Get the operand";)
    (LDQ; arg3; PROCESSORSTATE_BINDINGSTACKPOINTER (ivory);)
    (SRL; arg1; 32; arg5; "tag";)
    (LDQ; arg4; PROCESSORSTATE_BINDINGSTACKLIMIT (ivory);)
    (EXTLL; arg1; 0; arg6; "data";)
    (CheckDataType; arg5 |TypeLocative| bindlociop; t1;)
    (EXTLL; arg3; 0; arg3;)
    (EXTLL; arg4; 0; arg4;)
    (SUBQ; arg3; arg4; t1;)
    (BGE; t1; bindlocov; "J. if binding stack overflow";)
    (ADDQ; arg3; 1; t3;)
    (get-control-register; t9;)
    (BIS; arg6; zero; t8;)
    (memory-read; t8; t2; t1; PROCESSORSTATE_BINDREAD; t4; t5; t6; t7; nil; t;)
set;
the;
ls;
cdcode;
bit;
for ltag ifcleanupbindings
(SRL; t9; #.(- 25; 6;) t10;)
    (TagType; arg5; t8;)
    (AND; t10; #x40; t10; "Extract the CR.cleanup-bindings bit";)
    (BIS; t10; t8; t11;)
    (memory-write; t3; t11; arg6; PROCESSORSTATE_RAW; t4; t5; t6; t7; t8;)
    (ADDQ; arg3; 2; t3;)
    (memory-write; t3; t2; t1; PROCESSORSTATE_RAW; t4; t5; t6; t7; t8;)
    (load-constant; t1; #.1_25; "cr.cleanup-bindings";)
    (BIS; t1; t9; t9; "Set cr.cleanup-bindings bit";)
    (set-control-register; t9;)
    (STL; t3; PROCESSORSTATE_BINDINGSTACKPOINTER (ivory); "vma only";)
    (ContinueToNextInstruction)
  (label; bindlocov;)
    (illegal-operand; binding-stack-overflow;)
  (label; bindlociop;)
    (illegal-operand; bind-locative-type-error;)
  (label; bindlocdeep;)
    (LDQ; t1; PROCESSORSTATE_RESTARTSP (ivory); "Get the SP, ->op2";)
    (SCAtoVMA; t1; t2; t3;)
    (illegal-operand; shallow-binding-operation-in-deep-binding-mode; t2;))


(align16k)
(define-instruction |DoUnbindN|; :operand-from-stack-immediate ()
    (SRL; arg1; 32; arg2;)
    (EXTLL; arg1; 0; arg1;)
    (CheckDataType; arg2 |TypeFixnum| unbindniop; t1;)
    (;with-multiple-memory-reads (t9 t10; t11; t12;)
	(BR; zero; unbindnendloop;)
      (label; unbindntoploop;)
	(SUBQ; arg1; 1; arg1;)
	(unbind; t1; t2; t3; t4; t5; t6; t7; t8; arg3; arg4; arg5; arg6;)
      (label; unbindnendloop;)
	(BGT; arg1; unbindntoploop;)
After;
we;
've unbound everything, check for a preempt request
(check-preempt-request; NextInstruction; t3; t4;)
	(ContinueToNextInstruction);)
  (label; unbindniop;)
    (illegal-operand; one-operand-fixnum-type-error;))


(define-instruction |DoRestoreBindingStack|; :operand-from-stack-immediate ()
    (SRL; arg1; 32; arg2;)
    (EXTLL; arg1; 0; arg1;)
    (CheckDataType; arg2 |TypeLocative| restorebsiop; t1;)
    (LDQ; t1; PROCESSORSTATE_BINDINGSTACKPOINTER (ivory);)
    (;with-multiple-memory-reads (t9 t10; t11; t12;)
	(BR; zero; restorebsendloop;)
      (label; restorebstoploop;)
Leaves;
T1 as the;
new binding;
stack;
pointer
(unbind; t1; t2; t3; t4; t5; t6; t7; t8; arg3; arg4; arg5; arg6;)
      (label; restorebsendloop;)
	(SUBL; t1; arg1; arg4;)
	(BGT; arg4; restorebstoploop;)
After;
we;
've unbound everything, check for a preempt request
(check-preempt-request; NextInstruction; t3; t4;)
	(ContinueToNextInstruction);)
  (label; restorebsiop;)
    (illegal-operand; operand-locative-type-error;)
    )


(comment; "Fin.";)
