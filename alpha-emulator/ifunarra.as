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
(comment; "Array operations.";)





|
DoAref1 | and | DoAset1 | are in IFUNCOM2.AS
;The;
following is all;
of;
their;
out - of - line;
code
(define-procedure |Aref1Regset| ()
    (BIS; arg4; zero; t12;)
    (memory-read; arg4; arg5; arg6; PROCESSORSTATE_HEADER; t1; t2; t3; t4; nil; nil;)
    (check-array-header-and-prefix; arg5; arg6; Aref1Illegal |Aref1Exception| t1; t2;)
What;
we;
are;
about;
to;
do is strictly;
wrong--;
but;
it;
works.If;
the
;memory;
read;
moved;
the;
array, we;
put;
the;
array;
into;
the;
WRONG;
register,
;and;
then;

use

it.next;
time;
through, it;
will;
miss(because;
we;
put;
it
;
in
the;
wrong;
place;
),
and;
the;
miss;
code;
will;
fix;
it;
up.It;
's better
;than;
slowing;
down;
the;
common;
case
with a check.
(STL; t12; ARRAYCACHE_ARRAY (t7); "store the array";)
    (LDA; t2 |ArrayLengthMask| (zero);)
(AND;
arg6;
t2;
t1;
)
get;
array;
length;
into;
t1
;(check - array - bounds;
arg2;
t1;
Aref1Bounds;
t2;
)
(CMPULT; arg2; t1; t2;)
    (branch-false; t2; Aref1Bounds;)
    (STQ; t1; ARRAYCACHE_LENGTH (t7); "store the array length [implicit fixnum]";)
    (SRL; arg6 |ArrayRegisterBytePackingPos| t10;)
    (LDQ; t8; PROCESSORSTATE_AREVENTCOUNT (ivory);)
    (SLL; t10 |ArrayRegisterBytePackingPos| t10;)
    (ADDQ; arg4; 1; t9;)
    (ADDQ; t10; t8; t10; "Construct the array register word";)
    (STL; t10; ARRAYCACHE_ARWORD (t7); "store the array register word [implicit fixnum]";)
    (STQ; t9;  ARRAYCACHE_LOCAT (t7); "store the storage [implicit locative]";)
    (SRL; arg6 |ArrayBytePackingPos| arg5; "get BP into arg5";)
(SRL;
arg6 | ArrayRegisterByteOffsetPos | arg4;
)
(SRL; arg6 |ArrayElementTypePos| arg6; "get element type into arg6";)
    (AND; arg5 |ArrayBytePackingMask| arg5;)
(AND;
arg4 | ArrayRegisterByteOffsetMask | arg4;
)
(BIS; zero; zero; arg4;)
    (AND; arg6 |ArrayElementTypeMask| arg6;)
    (BR; zero; aref1restart;))

(define-procedure |Aref1RecomputeArrayRegister| ()
    (recompute-array-register; arg1; fast-aref-1; t1; t2; t3; FastAref1Retry;  t4; t5; t6; t7; t8;))

(define-procedure |Aref1Exception| ()
    ;(BR; zero; ReallyAref1Exc;)
    (STQ; arg2; PROCESSORSTATE_ASRF4 (ivory); "Just a place to save these values";)
    (STQ; t7; PROCESSORSTATE_ASRF5 (ivory); "Just a place to save these values";)
(BIS;
t12;
zero;
t9;
)
unforwarded;
arrayr
(BIS;
arg3;
zero;
arg2;
)
atag
(BIS;
arg4;
zero;
arg1;
)
adata
(BIS;
arg5;
zero;
t4;
)
t4 / t3;
contains;
the;
header
(BIS;
arg6;
zero;
t3;
)
(BIS;
zero;
zero;
t2;
)
don;
't force it!
(ADDQ; iSP; 24; iSP;)
(BSR;
r0 | Setup1DLongArray |;
)
long;
array;
reg;
w / o;
trap;
!
    (LDL; arg2; PROCESSORSTATE_ASRF4 (ivory); "Just a place to save these values";)
    (LDQ; t7; PROCESSORSTATE_ASRF5 (ivory); "Just a place to save these values";)
    (stack-pop2; t5; t1; "Length";)
    (stack-pop; t5; "base";)
    (stack-pop; t3; "control";)
    (stack-pop2; arg3; t9; "The original array";)
    (SUBQ; iSP; 24; iSP;)
    (STQ; t1; ARRAYCACHE_LENGTH (t7);)
    (STL; t3; ARRAYCACHE_ARWORD (t7);)
    (STL; t5; ARRAYCACHE_LOCAT (t7);)
    (STL; t9; ARRAYCACHE_ARRAY (t7); "store the array";)
    (EXTLL; t5; 0; t9;)
    (CMPEQ; t2 |ReturnValueException| t2;)
(branch - true;
t2;
ReallyAref1Exc;
)
we;
really;
need;
that;
exception;
after;
all;
!
    (CMPULT; arg2; t1; t5;)
    (branch-false; t5; Aref1Bounds;)
    (SRL; t3 |ArrayBytePackingPos| arg5; "get BP into arg5";)
    (SRL; t3 |ArrayElementTypePos| arg6; "get element type into arg6";)
    (SRL; t3 |ArrayRegisterByteOffsetPos| arg4;)
    (AND; arg5 |ArrayBytePackingMask| arg5;)
    (AND; arg4 |ArrayRegisterByteOffsetMask| arg4;)
    (AND; arg6 |ArrayElementTypeMask| arg6;)
Goes;
back;
to;
do (new - aref - 1 - internal; arg3;
t9;
arg5;
arg4;
arg6;
arg2;
t1;
t2;
t3;
t5;
t6;
)
(BR; zero; aref1restart;)

  (label; ReallyAref1Exc;)
At;
this;
point, we;
know;
that;
the;
type;
of;
ARG2 is fixnum
;(STQ; zero; ARRAYCACHE_ARRAY (t7);)
    (BIS; zero |TypeFixnum| arg1;)
    (SetTag; arg1; arg2; t1;)
    (ArrayTypeException; arg3; aref-1; t1 (array-access-type-check; :binary;))
  (label; Aref1Illegal;)
(STQ;
zero;
ARRAYCACHE_ARRAY(t7);
)
(illegal-operand (array-access-type-check; :binary;))
  (label; Aref1Bounds;)
    (STQ; zero; ARRAYCACHE_ARRAY (t7);)
    (illegal-operand; subscript-bounds-error;))


(define-procedure |Aset1Regset| ()
    (BIS; arg4; zero; t12;)
    (memory-read; arg4; arg5; arg6; PROCESSORSTATE_HEADER; t1; t2; t3; t4; nil; nil;)
    (check-array-header-and-prefix; arg5; arg6; aset1illegal |Aset1Exception| t1; t2;)
What;
we;
are;
about;
to;
do is strictly;
wrong--;
but;
it;
works.If;
the
;memory;
read;
moved;
the;
array, we;
put;
the;
array;
into;
the;
WRONG;
register,
;and;
then;

use

it.next;
time;
through, it;
will;
miss(because;
we;
put;
it
;
in
the;
wrong;
place;
),
and;
the;
miss;
code;
will;
fix;
it;
up.It;
's better
;than;
slowing;
down;
the;
common;
case
with a check.
(STL; t12; ARRAYCACHE_ARRAY (t7); "store the array";)
    (LDA; t2 |ArrayLengthMask| (zero);)
(AND;
arg6;
t2;
t1;
)
get;
array;
length;
into;
t1
;(check - array - bounds;
arg2;
t1;
Aref1Bounds;
t2;
)
(CMPULT; arg2; t1; t2;)
    (branch-false; t2; Aset1Bounds;)
    (STQ; t1; ARRAYCACHE_LENGTH (t7); "store the array length [implicit fixnum]";)
    (SRL; arg6 |ArrayRegisterBytePackingPos| t10;)
    (LDQ; t8; PROCESSORSTATE_AREVENTCOUNT (ivory);)
    (SLL; t10 |ArrayRegisterBytePackingPos| t10;)
    (ADDQ; arg4; 1; t9;)
    (ADDQ; t10; t8; t10; "Construct the array register word";)
    (STL; t10; ARRAYCACHE_ARWORD (t7); "store the array register word [implicit fixnum]";)
    (STQ; t9;  ARRAYCACHE_LOCAT (t7); "store the storage [implicit locative]";)
    (SRL; arg6 |ArrayBytePackingPos| arg5; "get BP into arg5";)
(SRL;
arg6 | ArrayRegisterByteOffsetPos | arg4;
)
(SRL; arg6 |ArrayElementTypePos| arg6; "get element type into arg6";)
    (AND; arg5 |ArrayBytePackingMask| arg5;)
(AND;
arg4 | ArrayRegisterByteOffsetMask | arg4;
)
(BIS; zero; zero; arg4;)
    (AND; arg6 |ArrayElementTypeMask| arg6;)
    (BR; zero; aset1restart;))

(align16k)
(define-procedure |Aset1RecomputeArrayRegister| ()
    (recompute-array-register; arg1; fast-aset-1; t1; t2; t3; FastAset1Retry;  t4; t5; t6; t7; t8;))

(define-procedure |Aset1Exception| ()
    ;(BR; zero; ReallyAset1Exc;)
    (STQ; arg2; PROCESSORSTATE_ASRF4 (ivory); "Just a place to save these values";)
    (STQ; t5; PROCESSORSTATE_ASRF3 (ivory); "Just a place to save these values";)
    (STQ; t6; PROCESSORSTATE_ASRF6 (ivory); "Just a place to save these values";)
    (STQ; t7; PROCESSORSTATE_ASRF5 (ivory); "Just a place to save these values";)
(BIS;
t12;
zero;
t9;
)
unforwarded;
array
(BIS;
arg3;
zero;
arg2;
)
atag
(BIS;
arg4;
zero;
arg1;
)
adata
(BIS;
arg5;
zero;
t4;
)
t4 / t3;
contains;
the;
header
(BIS;
arg6;
zero;
t3;
)
(BIS;
zero;
zero;
t2;
)
don;
't force it!
(ADDQ; iSP; 24; iSP;)
(BSR;
r0 | Setup1DLongArray |;
)
long;
array;
reg;
w / o;
trap;
!
    (CMPEQ; t2 |ReturnValueException| t1;)
(branch - true;
t1;
reallyaset1exc;
)
we;
really;
need;
that;
exception;
after;
all;
!
    (LDL; arg2; PROCESSORSTATE_ASRF4 (ivory); "Just a place to save these values";)
    (LDQ; t5; PROCESSORSTATE_ASRF3 (ivory); "Just a place to save these values";)
    (LDQ; t6; PROCESSORSTATE_ASRF6 (ivory); "Just a place to save these values";)
    (LDQ; t7; PROCESSORSTATE_ASRF5 (ivory); "Just a place to save these values";)
    (stack-pop2; t2; t1; "Length";)
    (stack-pop; t2; "base";)
    (stack-pop; t3; "control";)
    (stack-pop2; arg3; t9; "The original array";)
    (SUBQ; iSP; 24; iSP;)
    (STQ; t1; ARRAYCACHE_LENGTH (t7);)
    (STL; t3; ARRAYCACHE_ARWORD (t7);)
    (STL; t2; ARRAYCACHE_LOCAT (t7);)
    (STL; t9; ARRAYCACHE_ARRAY (t7); "store the array";)
    (EXTLL; t2; 0; t9;)
    (CMPULT; arg2; t1; t2;)
    (branch-false; t2; aset1bounds;)
    (SRL; t3 |ArrayBytePackingPos| arg5; "get BP into arg5";)
    (SRL; t3 |ArrayElementTypePos| arg6; "get element type into arg6";)
    (SRL; t3 |ArrayRegisterByteOffsetPos| arg4;)
    (AND; arg5 |ArrayBytePackingMask| arg5;)
    (AND; arg4 |ArrayRegisterByteOffsetMask| arg4;)
    (AND; arg6 |ArrayElementTypeMask| arg6;)
    (BR; zero; aset1restart;)

  (label; ReallyAset1Exc;)
At;
this;
point, we;
know;
that;
the;
type;
of;
ARG2 is fixnum
;(STQ; zero; ARRAYCACHE_ARRAY (t7);)
    (BIS; zero |TypeFixnum| arg1;)
    (SetTag; arg1; arg2; t1;)
    (ArrayTypeException; arg3; aset-1; t1 (array-access-type-check; :three-argument;))
  (label; Aset1Illegal;)
(STQ;
zero;
ARRAYCACHE_ARRAY(t7);
)
(illegal-operand (array-access-type-check; :three-argument;))
  (label; Aset1Bounds;)
    (STQ; zero; ARRAYCACHE_ARRAY (t7);)
    (illegal-operand; subscript-bounds-error;))


(define-instruction |DoAloc1|; :operand-from-stack-immediate (;:own-immediate; t;)
    (stack-pop2; arg3; arg4; "Get the array tag/data";)
    (EXTLL; arg1; 0; arg2; "Index Data";)
    (SRL; arg1; 32; arg1; "Index Tag";)
    (CheckDataType; arg1 |TypeFixnum| aloc1illegal; t1;)
  (label; aloc1merge;)
    (CheckAdjacentDataTypes; arg3 |TypeArray| 2; aloc1exception; t1;)
    (memory-read; arg4; arg5; arg6; PROCESSORSTATE_HEADER; t1; t2; t3; t4; nil; t;)
    (check-array-header-and-prefix; arg5; arg6; aloc1illegal; aloc1exception; t1; t2;)
    (LDA; t2 |ArrayLengthMask| (zero);)
(AND;
arg6;
t2;
t1;
)
get;
array;
length;
into;
t1
(check-array-bounds; arg2; t1; aloc1illegal; t3;)
    (SRL; arg6 |ArrayElementTypePos| arg6; "get element type into arg6";)
    (ADDQ; arg4; 1; arg4;)
    (ADDQ; arg4; arg2; arg4;)
    (AND; arg6 |ArrayElementTypeMask| arg6;)
    (SUBQ; arg6 |ArrayElementTypeObject| arg6;)
    (BNE; arg6; aloc1notobject;)
    (stack-push-ir |TypeLocative| arg4; t1;)
    (ContinueToNextInstruction)
  (label; aloc1exception;)
    (BIS; zero |TypeFixnum| arg1;)
    (SetTag; arg1; arg2; t1;)
    (ArrayTypeException; arg3; aloc-1; t1 (array-access-type-check; :binary;))
  (label; aloc1illegal;)
    (illegal-operand (array-access-type-check; :binary;))
  (label; aloc1bounds;)
    (illegal-operand; subscript-bounds-error;)
  (label; aloc1notobject;)
    (illegal-operand; aloc-non-object-array;)
  (immediate-handler |DoAloc1|)
    (stack-pop2; arg3; arg4; "Get the array tag/data";)
    (BR; zero; aloc1merge;))


(comment; "Array register operations.";)

(define-instruction |DoSetup1DArray|; :operand-from-stack-signed-immediate ()
    (SRL; arg1; 32; arg2; "Get the tag";)
    (EXTLL; arg1; 0; arg1; "and the data";)
    (BIS; zero; 0; t2; "Indicate not forcing 1d";)
    (setup-array-register; setup-1;d-array; arg2; arg1; NextInstruction;
			  t1; t2; t3; t4; t5; t6; t7; t8; t9; t10; t11; t12; arg6; arg5; arg4; arg3;)
    (ContinueToNextInstruction);)

(define-instruction |DoSetupForce1DArray|; :operand-from-stack-signed-immediate ()
    (SRL; arg1; 32; arg2; "Get the tag";)
    (EXTLL; arg1; 0; arg1; "and the data";)
    (BIS; zero; 1; t2; "Indicate forcing 1d";)
    (setup-array-register; setup-force-1;d-array; arg2; arg1; NextInstruction;
			  t1; t2; t3; t4; t5; t6; t7; t8; t9; t10; t11; t12; arg6; arg5; arg4; arg3;)
    (ContinueToNextInstruction);)

(define-procedure |Setup1DLongArray| (t3; t9;)
  (setup-long-array-register; arg2; arg1; t1; t2; t3; t4; t5; t6; t7; t8; t9; t10; t11; t12;
			     arg6; arg5; arg4; arg3;))





|
DoFastAref1 | is in IFUNCOM2.AS
(define-instruction |DoFastAset1|; :operand-from-stack ()
    (stack-pop2; arg3; arg4; "Index";)
    (stack-pop2; t10; t11; "value";)
    (checkDataType; arg3 |TypeFixnum| fastaset1iop; t1;)
  (label; FastAset1Retry;)
Get;
control;
register, base, and;
length,;
as
we;
do above.
(LDL; arg6; 0 (arg1);)
    (LDL; t9;   8 (arg1);)
    (LDL; t3;  16 (arg1);)
    (EXTLL; arg6; 0; arg6;)
    (EXTLL; t9; 0; t9;)
    (SLL; arg6; #.(- 64 |array$K-registereventcountsize|); t5;)
    (EXTLL; t3; 0; t3;)
    (LDQ; t4; PROCESSORSTATE_AREVENTCOUNT (ivory);)
    (SRL; t5; #.(- 64 |array$K-registereventcountsize|); t5;)
    (check-array-bounds; arg4; t3; fastaset1bounds; t2;)
    (SUBQ; t4; t5; t6;)
    (BNE; t6 |Aset1RecomputeArrayRegister|;)
    (SRL; arg6 |ArrayRegisterBytePackingPos| t6;)
    (SRL; arg6 |ArrayRegisterByteOffsetPos| t7;)
    (SRL; arg6 |ArrayRegisterElementTypePos| t8;)
    (AND; t6 |ArrayRegisterBytePackingMask| t6;)
    (AND; t7 |ArrayRegisterByteOffsetMask| t7;)
    (AND; t8 |ArrayRegisterElementTypeMask| t8;)
    (aset-1-internal; arg5; t9; t6; t7; t8; arg4; t10; t11;  t1; t2; t3; t4; t5; t12; arg3;)
  (label; fastaset1iop;)
    (illegal-operand; fast-array-access-type-check;)
  (label; fastaset1bounds;)
    (illegal-operand; array-register-format-error-or-subscript-bounds-error;))


(comment; "Array leaders.";)

(define-instruction |DoArrayLeader|; :operand-from-stack-immediate (;:own-immediate; t;)
    (stack-pop2; arg3; arg4; "arg3=arraytag, arg4=arraydata";)
    (EXTLL; arg1; 0; arg2; "index data";)
    (SRL; arg1; 32; arg1; "index tag";)
    (CheckDataType; arg1 |TypeFixnum| arrayleaderiop; t1;)
  (label; arrayleadermerge;)
Array;
or;
String
(CheckAdjacentDataTypes; arg3 |TypeArray| 2; arrayleaderexception; t1;)
    (;with-multiple-memory-reads (t9 t10; t11; t12;)
      (memory-read; arg4; arg6; arg5; PROCESSORSTATE_HEADER; t1; t2; t3; t4; nil; t;)
      (check-array-header; arg6; arrayleaderiop; t1;)
      (SRL; arg5 |ArrayLeaderLengthFieldPos| t8;)
      (AND; t8 |ArrayLeaderLengthFieldMask| t8;)
      (check-array-bounds; arg2; t8; arrayleaderbounds; t1;)
      (SUBQ; arg4; arg2; arg2;)
      (SUBQ; arg2; 1; arg2;)
      (memory-read; arg2; arg6; arg5; PROCESSORSTATE_DATAREAD; t1; t2; t3; t4; nil; t;)
      (stack-push2; arg6; arg5; t1;)
      (ContinueToNextInstruction);)
  (label; arrayleaderexception;)
At;
this;
point, we;
know;
that;
the;
type;
of;
ARG2 is fixnum
(BIS; zero |TypeFixnum| arg1;)
    (SetTag; arg1; arg2; t1;)
    (ArrayTypeException; arg3; array-leader; t1 (array-leader-access-type-check; :binary;))
  (label; arrayleaderiop;)
    (illegal-operand (array-leader-access-type-check; :binary;))
  (label; arrayleaderbounds;)
    (illegal-operand; subscript-bounds-error;)
  (immediate-handler |DoArrayLeader|)
    (stack-pop2; arg3; arg4; "arg3=arraytag, arg4=arraydata";)
    (BR; zero; arrayleadermerge;))

(define-instruction |DoStoreArrayLeader|; :operand-from-stack-immediate (;:own-immediate; t;)
    (stack-pop2; arg3; arg4; "arg3=arraytag, arg4=arraydata";)
    (stack-pop2; t6; t7; "t6=valuetag, t7=valuedata";)
    (EXTLL; arg1; 0; arg2; "index data";)
    (SRL; arg1; 32; arg1; "index tag";)
    (checkDataType; arg1 |TypeFixnum| storearrayleaderiop; t1;)
  (label; storearrayleadermerge;)
    (CheckAdjacentDataTypes; arg3 |TypeArray| 2; storearrayleaderexception; t1;)
    (;with-multiple-memory-reads (t9 t10; t11; t12;)
      (memory-read; arg4; arg6; arg5; PROCESSORSTATE_HEADER; t1; t2; t3; t4; nil; t;)
      (check-array-header; arg6; storearrayleaderiop; t1;)
      (SRL; arg5 |ArrayLeaderLengthFieldPos| t2;)
      (AND; t2 |ArrayLeaderLengthFieldMask| t2;)
      (check-array-bounds; arg2; t2; storearrayleaderbounds; t1;)
      (SUBQ; arg4; arg2; arg2;)
      (SUBQ; arg2; 1; arg2;)
      (store-contents; arg2; t6; t7; PROCESSORSTATE_DATAWRITE; t1; t2; t3; t4; t5; t8;
		      NextInstruction;)
      (ContinueToNextInstruction);)
  (label; storearrayleaderexception;)
    (BIS; zero |TypeFixnum| arg1;)
    (SetTag; arg1; arg2; t1;)
    (ArrayTypeException; arg3; store-array-leader; t1 (array-leader-access-type-check; :three-argument;))
  (label; storearrayleaderiop;)
    (illegal-operand (array-leader-access-type-check; :three-argument;))
  (label; storearrayleaderbounds;)
    (illegal-operand; subscript-bounds-error;)
  (immediate-handler |DoStoreArrayLeader|)
    (stack-pop2; arg3; arg4; "arg3=arraytag, arg4=arraydata";)
    (stack-pop2; t6; t7; "t6=valuetag, t7=valuedata";)
    (BR; zero; storearrayleadermerge;))

(define-instruction |DoAlocLeader|; :operand-from-stack-immediate (;:own-immediate; t;)
    (stack-pop2; arg3; arg4; "arg3=arraytag, arg4=arraydata";)
    (EXTLL; arg1; 0; arg2; "index data";)
    (SRL; arg1; 32; arg1; "index tag";)
    (checkDataType; arg1 |TypeFixnum| alocleaderiop; t1;)
  (label; alocleadermerge;)
    (CheckAdjacentDataTypes; arg3 |TypeArray| 2; alocleaderexception; t1;)
    (memory-read; arg4; arg6; arg5; PROCESSORSTATE_HEADER; t1; t2; t3; t4; nil; t;)
    (check-array-header; arg6; alocleaderiop; t1;)
    (SRL; arg5 |ArrayLeaderLengthFieldPos| t9;)
    (AND; t9 |ArrayLeaderLengthFieldMask| t9;)
    (check-array-bounds; arg2; t9; alocleaderbounds; t1;)
    (SUBQ; arg4; arg2; arg2;)
    (SUBQ; arg2; 1; arg2;)
    (stack-push-ir |TypeLocative| arg2; t1;)
    (ContinueToNextInstruction)
  (label; alocleaderexception;)
    (BIS; zero |TypeFixnum| arg1;)
    (SetTag; arg1; arg2; t1;)
    (ArrayTypeException; arg3; aloc-leader; t1 (array-leader-access-type-check; :binary;))
  (label; alocleaderiop;)
    (illegal-operand (array-leader-access-type-check; :binary;))
  (label; alocleaderbounds;)
    (illegal-operand; subscript-bounds-error;)
  (immediate-handler |DoAlocLeader|)
    (stack-pop2; arg3; arg4; "arg3=arraytag, arg4=arraydata";)
    (BR; zero; alocleadermerge;))

(comment; "Fin.";)


