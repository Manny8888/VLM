(in-package :grimoire)

;; ;; (comment "Lexical variable accessors.")


(define-instruction |DoPushLexicalVarN| :operand-from-stack ()
  (SRL arg3 10 t4 "Position the opcode")
  ;; in-line (stack-read2 arg1 t2 t1)
  (LDL t1 0 (arg1))
  (LDL t2 4 (arg1))
  (AND t4 7 t4 "Get the lexical var number")
  (EXTLL t1 0 t1)
  (TagType t2 t3)
  (SUBQ t3 |TypeList| t3)
  (BIC t3 4 t3)
  (ADDQ t1 t4 t1 "Compute the address of the lexical variable.")
  (BNE t3 pushlexvariop)
  (with-multiple-memory-reads (arg3 arg4 arg5 arg6)
    (memory-read t1 t2 t3 PROCESSORSTATE_DATAREAD t4 t5 t6 t7 nil t))
  (GetNextPCandCP)
  (stack-push2 t2 t3 t4)
  (ContinueToNextInstruction-NoStall)
  (label pushlexvariop)
  (illegal-operand unary-lexical-environment-type-error nil "Not a list or locative"))

(define-instruction |DoPopLexicalVarN| :operand-from-stack ()
  (SRL arg3 10 t4 "Position the opcode")
  ;; in-line (stack-read2 arg1 t2 t1)
  (LDL t1 0 (arg1))
  (LDL t2 4 (arg1))
  (AND t4 7 t4 "Get the lexical var number")
  (EXTLL t1 0 t1)
  (TagType t2 t3)
  (SUBQ t3 |TypeList| t3)
  (BIC t3 4 t3)
  (ADDQ t1 t4 t1 "Compute the address of the lexical variable.")
  (BNE t3 poplexvariop)
  (stack-pop2 t2 t3)
  (with-multiple-memory-reads (arg3 arg4 arg5 arg6)
    (store-contents t1 t2 t3 PROCESSORSTATE_DATAWRITE t4 t5 t6 t7 t8 t9
                    NextInstruction))
  (ContinueToNextInstruction)
  (label poplexvariop)
  (illegal-operand binary-lexical-environment-type-error nil "Not a list or locative"))

(define-instruction |DoMovemLexicalVarN| :operand-from-stack ()
  (SRL arg3 10 t4 "Position the opcode")
  ;; in-line (stack-read2 arg1 t2 t1)
  (LDL t1 0 (arg1))
  (LDL t2 4 (arg1))
  (AND t4 7 t4 "Get the lexical var number")
  (EXTLL t1 0 t1)
  (TagType t2 t3)
  (SUBQ t3 |TypeList| t3)
  (BIC t3 4 t3)
  (ADDQ t1 t4 t1 "Compute the address of the lexical variable.")
  (BNE t3 movemlexvariop)
  (stack-read2 iSP t2 t3)
  (with-multiple-memory-reads (arg3 arg4 arg5 arg6)
    (store-contents t1 t2 t3 PROCESSORSTATE_DATAWRITE t4 t5 t6 t7 t8 t9
                    NextInstruction))
  (ContinueToNextInstruction)
  (label movemlexvariop)
  (illegal-operand binary-lexical-environment-type-error nil "Not a list or locative"))


;; ;; (comment "Fin.")
