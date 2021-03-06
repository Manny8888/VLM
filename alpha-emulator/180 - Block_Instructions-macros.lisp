(in-package "ALPHA-AXP-INTERNALS")

;;; Macros in support of block instructions.  These are mostly in ifunblok.as

(assert (lisp:and (< (integer-length processorstate$q-bar0) 15)
                  (< (integer-length processorstate$q-bar1) 15)
                  (< (integer-length processorstate$q-bar2) 15)
                  (< (integer-length processorstate$q-bar3) 15))
        ()
        "The BAR registers have an offset of more than 15 bits")

;;; Note well!  We always store the updated VMA back into the BAR, even
;;; in the no-increment case.  This is because the BAR must get the result
;;; of having followed forwarding pointers.

;;; Note well!  We always store the updated VMA back into the BAR, even
;;; in the no-increment case.  This is because the BAR must get the result
;;; of having followed forwarding pointers.

(defmacro i%block-n-read (bar op vma tag data cycle
                          temp3 temp4 temp5 temp6 temp9 temp10 temp11 temp12)
  (check-temporaries (bar op)
                     (cycle vma temp3 temp4 temp5 temp6 data tag temp9 temp10 temp11 temp12))
  (let ((fntest (gensym))
        (nofntest (gensym))
        (ielab (gensym)))
    (push
     `((label ,fntest)
       (CheckDataType ,tag |TypeFixnum| ,ielab ,temp9)
       (BR zero ,nofntest))
     *function-epilogue*)
    `((LDL ,vma 0 (,bar) "Get the vma")
      (SRL ,op 6 ,cycle "cycle type")
      (AND ,op 4 ,temp4 "=no-incrementp")
      ;;; we don't care about last-word
      (AND ,op 16 ,temp5 "=cdr-code-nextp")
      (AND ,op 32 ,temp6 "=fixnum onlyp")
      (EXTLL ,vma 0 ,vma)
      (comment "Do the read cycle")
      (memory-read ,vma ,tag ,data ,cycle ,temp9 ,temp10 ,temp11 ,temp12 nil t)
      (BNE ,temp6 ,fntest "J. if we have to test for fixnump.")
      (unlikely-label ,nofntest)
      (ADDQ ,vma 1 ,temp6 "Compute Incremented address")
      (force-alignment)
      (CMOVEQ ,temp4 ,temp6 ,vma "Conditionally update address")
      (STL ,vma 0 (,bar) "Store updated vma in BAR")
      (AND ,tag #x3F ,temp4 "Compute CDR-NEXT")
      (GetNextPC)
      (CMOVNE ,temp5 ,temp4 ,tag "Conditionally Set CDR-NEXT")
      (GetNextCP)
      (stack-push2-with-cdr ,tag ,data)
      (ContinueToNextInstruction-NoStall)
      (label ,ielab)
      (illegal-operand block-read-transport-and-fixnum-type-check ,vma "Not a fixnum"))))

(defmacro i%block-n-write (bar-register bar-vma data
                           temp2 temp3 temp4 temp5 temp6 temp7 temp8 temp9)
  (check-temporaries (bar-register bar-vma data)
                     (temp2 temp3 temp4 temp5 temp6 temp7 temp8))
  `((SRL ,data 32 ,temp3 "Get tag")
    (EXTLL ,data 0 ,temp4 "Get data")
    (memory-write ,bar-vma ,temp3 ,temp4 PROCESSORSTATE_RAW ,temp9 ,temp5 ,temp6 ,temp7 ,temp8)
    (GetNextPCandCP)
    (ADDQ ,bar-vma 1 ,bar-vma "Increment the address")
    ;; Can't side-effect the BAR until after the write in case it would trap.
    (STL ,bar-vma 0 (,bar-register) "Store updated vma in BAR")
    (ContinueToNextInstruction-NoStall)))

(defmacro i%block-n-read-shift (bar op temp temp2 temp3 temp4 temp5 temp6
                                temp7 temp8 temp9 temp10 temp11 temp12)
  (check-temporaries (bar op)
                     (temp temp2 temp3 temp4 temp5 temp6 temp7 temp8 temp9 temp10 temp11 temp12))
  (let ((nofntest (gensym))
        (noincp (gensym))
        (noclrcdr (gensym))
        (ielab (gensym)))
    `((LDL ,temp2 0 (,bar) "Get the vma")
      (SRL ,op 6 ,temp "cycle type")
      (AND ,op 4 ,temp4 "=no-incrementp")
      ;;; we don't care about last-word
      (AND ,op 16 ,temp5 "=cdr-code-nextp")
      (AND ,op 32 ,temp6 "=fixnum onlyp")
      (EXTLL ,temp2 0 ,temp2)
      (memory-read ,temp2 ,temp8 ,temp7 ,temp ,temp9 ,temp10 ,temp11 ,temp12)
      (BEQ ,temp6 ,nofntest "J. if we don't have to test for fixnump.")
      (CheckDataType ,temp8 |TypeFixnum| ,ielab ,temp9)
      (label ,nofntest)
      (BNE ,temp4 ,noincp "J. if we don't have to increment the address.")
      (ADDQ ,temp2 1 ,temp2 "Increment the address")
      (label ,noincp)
      (STL ,temp2 0 (,bar) "Store updated vma in BAR")
      (BEQ ,temp5 ,noclrcdr "J. if we don't have to clear CDR codes.")
      (AND ,temp8 #x3F ,temp8)
      (label ,noclrcdr)
      (load-constant ,temp #.(dpb (sys:%alu-function-dpb sys:%alu-byte-background-rotate-latch
                                                         sys:%alu-byte-set-rotate-latch)
                                  sys:%%alu-function 0)
                     "Create a fake ALU control register")
      (alu-function-byte ,temp ,temp ,temp7 ,temp7 ,temp2 ,temp3 ,temp4 ,temp5 ,temp6)
      (GetNextPCandCP)
      (stack-push2-with-cdr ,temp8 ,temp7)
      (ContinueToNextInstruction-NoStall)
      (label ,ielab)
      (illegal-operand block-read-transport-and-fixnum-type-check ,temp2 "Not a fixnum"))))

(defmacro i%block-n-read-alu (bar addr temp temp2 temp3 temp4 temp5 temp6
                              temp7 temp8 temp9 temp10 temp11 temp12)
  (check-temporaries (bar addr)
                     (temp temp2 temp3 temp4 temp5 temp6 temp7 temp8 temp9 temp10 temp11 temp12))
  (let ((ielab2 (gensym))
        (ielab1 (gensym))
        (op1tag temp2)
        (op1data temp3)
        (op2tag temp4)
        (op2data temp5)
        (aluop temp6)
        (control temp7)
        (result temp8))
    `((LDL ,temp 0 (,bar) "Get the vma")
      (stack-read2 ,addr ,op2tag ,op2data)
      (CheckDataType ,op2tag |TypeFixnum| ,ielab2 ,temp9)
      (EXTLL ,temp 0 ,temp)
      (memory-read ,temp ,op1tag ,op1data PROCESSORSTATE_DATAREAD ,temp9 ,temp10 ,temp11 ,temp12)
      (CheckDataType ,op1tag |TypeFixnum| ,ielab1 ,temp9)
      (ADDQ ,temp 1 ,temp "Increment the address")
      (STL ,temp 0 (,bar) "Store updated vma in BAR")
      (LDQ ,aluop PROCESSORSTATE_ALUOP (ivory))
      (STQ zero PROCESSORSTATE_ALUOVERFLOW (ivory))
      (LDQ ,control PROCESSORSTATE_ALUANDROTATECONTROL (ivory))
      (basic-dispatch ,aluop ,temp
                      (|ALUFunctionBoolean|
                       (alu-function-boolean ,control ,result ,op1data ,op2data ,temp)
                       (stack-write-data ,addr ,result)
                       (ContinueToNextInstruction))
                      (|ALUFunctionByte|
                       (alu-function-byte ,control ,op1data ,op2data ,result
                                          ,temp ,temp9 ,temp10 ,temp11 ,temp12)
                       (stack-write-data ,addr ,result)
                       (ContinueToNextInstruction))
                      (|ALUFunctionAdder|
                       (alu-function-adder ,control ,op1data ,op2data ,result
                                           ,temp ,temp9 ,temp10 ,temp11)
                       (stack-write-data ,addr ,result)
                       (ContinueToNextInstruction))
                      (|ALUFunctionMultiplyDivide|
                       (alu-function-multiply-divide ,control ,op1data ,op2data ,result ,temp ,temp9)
                       (stack-write-data ,addr ,result)
                       (ContinueToNextInstruction)))
      (label ,ielab2)
      (SCAtoVMA ,addr ,temp ,temp9)
      (illegal-operand block-read-transport-and-fixnum-type-check ,temp "Not a fixnum")
      (label ,ielab1)
      (illegal-operand block-read-transport-and-fixnum-type-check ,temp "Not a fixnum"))))

(defmacro i%block-n-read-test (bar op vma temp temp2 temp3 temp4 temp5 temp6
                               temp7 temp8 temp9 temp10 temp11 temp12)
  (check-temporaries (bar op)
                     (temp temp2 temp3 temp4 temp5 temp6 temp7 temp8 temp9 temp10 temp11 temp12))
  (let ((nofntest (gensym))
        (noincp (gensym))
        (noclrcdr (gensym))
        (ielab1 (gensym))
        (ielab2 (gensym))
        (taken (gensym))
        (op1tag temp2 )
        (op1data temp3)
        (op2tag temp4)
        (op2data temp5)
        (aluop temp6)
        (control temp7)
        (result temp8))
    `((LDL ,vma 0 (,bar) "Get the vma")
      (SRL ,op 6 ,temp "cycle type")
      (stack-read2 iSP ,op2tag ,op2data)
      (EXTLL ,vma 0 ,vma)
      (memory-read ,vma ,op1tag ,op1data ,temp ,temp9 ,temp10 ,temp11 ,temp12)
      (AND ,op 32 ,temp "=fixnum onlyp")
      (BEQ ,temp ,nofntest "J. if we don't have to test for fixnump.")
      (CheckDataType ,op1tag |TypeFixnum| ,ielab1 ,temp9)
      (CheckDataType ,op2tag |TypeFixnum| ,ielab2 ,temp9)
      (label ,nofntest)
      (AND ,op 16 ,temp "=cdr-code-nextp")
      (BEQ ,temp ,noclrcdr "J. if we don't have to clear CDR codes.")
      (TagType ,op1tag ,op1tag)
      (label ,noclrcdr)
      (LDQ ,aluop PROCESSORSTATE_ALUOP (ivory))
      (STQ zero PROCESSORSTATE_ALUOVERFLOW (ivory))
      (LDQ ,control PROCESSORSTATE_ALUANDROTATECONTROL (ivory))
      (basic-dispatch ,aluop ,temp
                      (|ALUFunctionBoolean|
                       (alu-function-boolean ,control ,result ,op1data ,op2data ,temp))
                      (|ALUFunctionByte|
                       (alu-function-byte ,control ,op1data ,op2data ,result
                                          ,temp ,temp9 ,temp10 ,temp11 ,temp12))
                      (|ALUFunctionAdder|
                       (alu-function-adder ,control ,op1data ,op2data ,result
                                           ,temp ,temp9 ,temp10 ,temp11))
                      (|ALUFunctionMultiplyDivide|
                       (alu-function-multiply-divide ,control ,op1data ,op2data ,result
                                                     ,temp ,temp9)))
      (alu-compute-condition ,control ,op1tag ,op2tag ,result
                             ,temp ,temp9 ,temp10 ,temp11 ,temp12)
      (branch-true ,temp ,taken)
      (AND ,op 4 ,temp "=no-incrementp")
      (BNE ,temp ,noincp "J. if we don't have to increment the address.")
      (ADDQ ,vma 1 ,vma "Increment the address")
      (label ,noincp)
      (STL ,vma 0 (,bar) "Store updated vma in BAR")
      (ContinueToNextInstruction)
      (label ,taken)
      (stack-read2-disp iSP -8 ,temp9 ,temp10)
      #+++ignore (CheckAdjacentDataTypes ,temp9 |TypeEvenPC| 2 ,except ,temp10)
      (SLL ,temp10 1 ,temp10)
      (AND ,temp9 1 iPC)
      (ADDQ iPC ,temp10 iPC)
      (BR zero InterpretInstructionForJump)
      (label ,ielab2)
      (SCAtoVMA iSP ,vma ,temp9)
      (illegal-operand block-read-transport-and-fixnum-type-check ,vma "Not a fixnum")
      (label ,ielab1)
      (illegal-operand block-read-transport-and-fixnum-type-check ,vma "Not a fixnum"))))

;;; Fin.
