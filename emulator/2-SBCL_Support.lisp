
(declaim (sb-ext:muffle-conditions sb-ext:compiler-note))


(in-package :grimoire-emulator)

;; defun declaration with declaimed inlining
(defmacro defsubst (name arglist &body body)
  `(progn
     (declaim (inline ,name))
     (defun ,name ,arglist ,@body)))

(defmacro stack-let (vars-and-vals &body body)
  (let ((vars (loop for var-and-val in vars-and-vals
                    if (atom var-and-val)
                      collect var-and-val
                    else
                      collect (first var-and-val))))
    `(let ,vars-and-vals
       (declare (dynamic-extent ,@vars))
       ,@body)))

;; (declaim (inline circular-list))
;; (defun circular-list (&rest list)
;;   (let ((list (copy-list list)))
;;     (setf (cdr (last list)) list)
;;     list))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defsubst %logldb (bytespec integer)
  (ldb bytespec integer))

(defsubst %logdpb (value bytespec integer)
  (let ((result (dpb value bytespec integer)))
    (if (zerop (ldb (byte 1 31) result))
        result
        (- (ldb (byte 31 0) (1+ (lognot result)))))))


(defsubst %32-bit-difference (x y)
  (- x y))

;; (defun %32-bit-difference (x y)
;;   (- x y))

(export '(%logldb %logdpb %32-bit-difference))


(defmacro defenumerated (list-name code-list
                         &optional (start 0) (increment 1) end)
  (when (and end (not (= (length code-list) (/ (- end start) increment))))
    (error "defenumerated: enum called ~s has ~s codes but ~s are required"
           list-name (length code-list) (/ (- end start) increment)))
  `(progn
     (defsysconstant ,list-name ',code-list)
     ,@(loop for code in code-list and prev = 0 then code
             as value from start by increment
             unless (eq code prev)		;kludge for data-types
               collect `(defsysconstant ,code ,value))))


(defmacro defsysconstant (name value)
  `(progn
     (defconstant ,name ,value)
     (export ',name)))

(defmacro defsysbyte (name size position)
  `(defsysconstant ,name (byte ,size ,position)))


;;;
;;; The following definitions are from SYS:I-SYS;SYSDEF.LISP ...
;;;

;; --- most of the below is L-specific
;; To add a new data type, update the following (at least):
;;	*DATA-TYPES* and *POINTER-DATA-TYPES* in this file
;;
;;	Patch
;;      *DATA-TYPE-NAME*, set up by from *DATA-TYPES* by the cold-load generator
;;
;;      type-map-for-transport, transporter-type-map-alist in sys: l-ucode; uu.lisp
;;
;;      *storing-type-map* in sys: l-ucode; uux.lisp and reload that whole file
;;
;;	It is important that the form near the end of that file that sets up the
;;	no-trap type-map be executed before any other type maps are assigned.
;;
;;      simulate-transporter in sys: l-ucode; simx.lisp
;;
;;	and recompile the whole microcode to get the type-maps updated
;;
;;      typep-alist and related stuff in sys: sys; lcons.lisp
;;
;;	dbg:*good-data-types* if it is indeed a good data type
;;
;;	Send a message to the maintainer of the FEP-resident debugger.

(defenumerated +data-types+
    ( ;; Headers, special markers, and forwarding pointers.
     +dtp-null+                           ; 00 Unbound variable/function, uninitialized storage
     +dtp-monitor-forward+                ; 01 This cell being monitored
     dtp-header-p         ;02 Structure header, with pointer field
     dtp-header-i         ;03 Structure header, with immediate bits
     dtp-external-value-cell-pointer    ;04 Invisible except for binding
     dtp-one-q-forward        ;05 Invisible pointer (forwards one cell)
     dtp-header-forward       ;06 Invisible pointer (forwards whole structure)
     dtp-element-forward        ;07 Invisible pointer in element of structure

     ;; Numeric data types.
     dtp-fixnum         ;10 Small integer
     dtp-small-ratio        ;11 Ratio with small numerator and denominator
     dtp-single-float       ;12 Single-precision floating point
     dtp-double-float       ;13 Double-precision floating point
     dtp-bignum         ;14 Big integer
     dtp-big-ratio        ;15 Ratio with big numerator or denominator
     dtp-complex          ;16 Complex number
     dtp-spare-number       ;17 A number to the hardware trap mechanism

     ;; Instance data types.
     dtp-instance         ;20 Ordinary instance
     dtp-list-instance        ;21 Instance that masquerades as a cons
     dtp-array-instance       ;22 Instance that masquerades as an array
     dtp-string-instance        ;23 Instance that masquerades as a string

     ;; Primitive data types.
     dtp-nil          ;24 The symbol NIL
     dtp-list         ;25 A cons
     dtp-array          ;26 An array that is not a string
     dtp-string         ;27 A string
     dtp-symbol         ;30 A symbol other than NIL
     dtp-locative         ;31 Locative pointer
     dtp-lexical-closure        ;32 Lexical closure of a function
     dtp-dynamic-closure        ;33 Dynamic closure of a function
     dtp-compiled-function      ;34 Compiled code
     dtp-generic-function       ;35 Generic function (see later section)
     dtp-spare-pointer-1        ;36 Spare
     dtp-spare-pointer-2        ;37 Spare
     dtp-physical-address       ;40 Physical address
     dtp-spare-immediate-1      ;41 Spare
     dtp-bound-location       ;42 Deep bound marker
     dtp-character        ;43 Common Lisp character object
     dtp-logic-variable       ;44 Unbound logic variable marker
     dtp-gc-forward       ;45 Object-moved flag for garbage collector
     dtp-even-pc          ;46 PC at first instruction in word
     dtp-odd-pc         ;47 PC at second instruction in word

     ;; Full-word instructions.
     dtp-call-compiled-even     ;50 Start call, address is compiled function
     dtp-call-compiled-odd      ;51 Start call, address is compiled function
     dtp-call-indirect        ;52 Start call, address is function cell
     dtp-call-generic       ;53 Start call, address is generic function
     dtp-call-compiled-even-prefetch    ;54 Like above, but prefetching is desireable
     dtp-call-compiled-odd-prefetch   ;55 Like above, but prefetching is desireable
     dtp-call-indirect-prefetch     ;56 Like above, but prefetching is desireable
     dtp-call-generic-prefetch      ;57 Like above, but prefetching is desireable

     ;; Half-word (packed) instructions consume 4 bits of data type field (opcodes 60..77).
     dtp-packed-instruction-60 dtp-packed-instruction-61 dtp-packed-instruction-62
     dtp-packed-instruction-63 dtp-packed-instruction-64 dtp-packed-instruction-65
     dtp-packed-instruction-66 dtp-packed-instruction-67 dtp-packed-instruction-70
     dtp-packed-instruction-71 dtp-packed-instruction-72 dtp-packed-instruction-73
     dtp-packed-instruction-74 dtp-packed-instruction-75 dtp-packed-instruction-76
     dtp-packed-instruction-77
     )
    0 1 #o100)

(defenumerated *array-element-data-types*
    (array-element-type-fixnum
     array-element-type-character
     array-element-type-boolean
     array-element-type-object
     ))

;;; Control register.

(defsysbyte %%cr.argument-size 8. 0)    ;Number of spread arguments supplied by caller
(defsysbyte %%cr.apply 1 17.)     ;1 If caller used APPLY, 0 otherwise
(defsysbyte %%cr.value-disposition 2 18.) ;The value of this function
(defsysbyte %%cr.cleanup-bits 3 24.)    ;All the cleanup bits
(defsysbyte %%cr.cleanup-catch 1 26.)   ;There are active catch blocks in the current frame
(defsysbyte %%cr.cleanup-bindings 1 25.)  ;There are active bindings in the current frame
(defsysbyte %%cr.trap-on-exit-bit 1 24.)  ;Software trap before exiting this frame
(defsysbyte %%cr.trap-mode 2 30.)   ;1 If we are executing on the "extra stack"
                                        ;Extra stack inhibits sequence breaks and preemption
                                        ;It also allows the "overflow" part of the stack to
                                        ;be used without traps.
(defsysbyte %%cr.extra-argument 1 8.)   ;The call instruction supplied an "extra" argument
(defsysbyte %%cr.caller-frame-size 8 9.)  ;The frame size of the Caller
(defsysbyte %%cr.call-started 1 22.)    ;Between start-call and finish-call.
(defsysbyte %%cr.cleanup-in-progress 1 23.)
(defsysbyte %%cr.instruction-trace 1 29.)
(defsysbyte %%cr.call-trace 1 28.)
(defsysbyte %%cr.trace-pending 1 27.)
(defsysbyte %%cr.trace-bits 3 27.)

(defsysbyte %%cr.cleanup-and-trace-bits 6 24.)

(defenumerated *value-dispositions*
    (value-disposition-effect     ;The callers wants no return values
     value-disposition-value      ;The caller wants a single return value
     value-disposition-return     ;The caller wants to return whatever values are
                                        ;returned by this function
     value-disposition-multiple     ;The callers wants multiple values
     ))

(defenumerated *trap-modes*
    (trap-mode-emulator
     trap-mode-extra-stack
     trap-mode-io
     trap-mode-fep))

(defenumerated *memory-cycle-types*
    (%memory-data-read
     %memory-data-write
     %memory-bind-read
     %memory-bind-write
     %memory-bind-read-no-monitor
     %memory-bind-write-no-monitor
     %memory-header
     %memory-structure-offset
     %memory-scavenge
     %memory-cdr
     %memory-gc-copy
     %memory-raw
     %memory-raw-translate))

;;; Internal register definitions

;;; %REGISTER-ALU-AND-ROTATE-CONTROL fields (DP-OP in hardware spec)
(defsysbyte %%alu-byte-r     5  0.)
(defsysbyte %%alu-byte-s     5  5.)
(defsysbyte %%alu-function         6 10.)
(defsysbyte %%alu-function-class   2 14.)
(defsysbyte %%alu-function-bits    4 10.)
(defsysbyte %%alu-condition    5 16.)
(defsysbyte %%alu-condition-sense  1 21.)

;; The following are implemented in Rev3 only.
;; Software forces them to the proper value for compatible operation in Rev1 and Rev2.
(defsysbyte %%alu-output-condition 1 22.)
(defsysbyte %%alu-enable-condition-exception 1 23.)
(defsysbyte %%alu-enable-load-cin 1 24.)

(defenumerated *alu-condition-senses*
    (%alu-condition-sense-true
     %alu-condition-sense-false))

(defenumerated *alu-conditions*
    (%alu-condition-signed-less-than-or-equal   ;00
     %alu-condition-signed-less-than      ;01
     %alu-condition-negative        ;02
     %alu-condition-signed-overflow     ;03
     %alu-condition-unsigned-less-than-or-equal   ;04
     %alu-condition-unsigned-less-than      ;05
     %alu-condition-zero          ;06
     %alu-condition-high-25-zero        ;07
     %alu-condition-eq          ;10
     %alu-condition-op1-ephemeralp      ;11
     %alu-condition-op1-type-acceptable     ;12
     %alu-condition-op1-type-condition      ;13
     %alu-condition-result-type-nil     ;14
     %alu-condition-op2-fixnum        ;15
     %alu-condition-false         ;16
     %alu-condition-result-cdr-low      ;17
     %alu-condition-cleanup-bits-set      ;20
     %alu-condition-address-in-stack-cache    ;21
     %alu-condition-pending-sequence-break-enabled  ;22
     %alu-condition-extra-stack-mode      ;23
     %alu-condition-fep-mode        ;24
     %alu-condition-fp-coprocessor-present    ;25
     %alu-condition-op1-oldspacep       ;26
     %alu-condition-stack-cache-overflow      ;27
     %alu-condition-or-logic-variable     ;30
     ))

(defenumerated *alu-function-classes*
    (%alu-function-class-boolean
     %alu-function-class-byte
     %alu-function-class-adder
     %alu-function-class-multiply-divide))

(defenumerated *alu-functions*
    (%alu-function-op-boolean-0
     %alu-function-op-boolean-1
     %alu-function-op-dpb
     %alu-function-op-ldb
     %alu-function-op-add
     %alu-function-op-reserved
     %alu-function-op-multiply-step
     %alu-function-op-multiply-invert-step
     %alu-function-op-divide-step
     %alu-function-op-divide-invert-step))

(defenumerated *alu-byte-backgrounds*
    (%alu-byte-background-op1
     %alu-byte-background-rotate-latch
     %alu-byte-background-zero))

(defenumerated *alu-byte-rotate-latch*
    (%alu-byte-hold-rotate-latch
     %alu-byte-set-rotate-latch))

(defenumerated *alu-add-op2-actions*
    (%alu-add-op2-pass
     %alu-add-op2-invert))

(defenumerated *alu-adder-ops*
    (%alu-add-op2
     %alu-add-zero))

(defmacro %alu-function-dpb (background rotate-latch)
  `(%logdpb  %alu-function-op-dpb (byte 3 3)
             (%logdpb ,rotate-latch (byte 1 2)
                      (%logdpb ,background (byte 2 0)
                               0))))
(export '%alu-function-dpb)


;;;
;;; The following definitions are from SYS:I-SYS;SYSDF1.LISP ...
;;;
(defsysconstant %arithmetic-instruction-exception-vector #o0)
(defsysconstant %instruction-exception-vector #o4000)
(defsysconstant %interpreter-function-vector #o4400)
(defsysconstant %generic-dispatch-vector #o5000)

(defsysconstant %error-trap-vector #o5100)
(defsysconstant %reset-trap-vector #o5101)
(defsysconstant %pull-apply-args-trap-vector #o5102)
(defsysconstant %stack-overflow-trap-vector #o5103)
(defsysconstant %trace-trap-vector #o5104)
(defsysconstant %preempt-request-trap-vector #o5105)
(defsysconstant %transport-trap-vector #o5106)
(defsysconstant %fep-mode-trap-vector #o5107)

(defsysconstant %low-priority-sequence-break-trap-vector #o5110)
(defsysconstant %high-priority-sequence-break-trap-vector #o5111)
(defsysconstant %monitor-trap-vector #o5112)
;;; 5113 reserved for future use
(defsysconstant %generic-dispatch-trap-vector #o5114)
;;; 5115 reserved for a fence word
(defsysconstant %message-dispatch-trap-vector #o5116)
;;; 5117 reserved for a fence word

(defsysconstant %page-not-resident-trap-vector #o5120)
(defsysconstant %page-fault-request-trap-vector #o5121)
(defsysconstant %page-write-fault-trap-vector #o5122)
(defsysconstant %uncorrectable-memory-error-trap-vector #o5123)
(defsysconstant %memory-bus-error-trap-vector #o5124)
(defsysconstant %db-cache-miss-trap-vector #o5125)
(defsysconstant %db-unwind-frame-trap-vector #o5126)
(defsysconstant %db-unwind-catch-trap-vector 5127)
;;; 5130 through 5177 reserved for future use


;;;
;;; The following definitions are from SYS:I-SYS;OPSDEF.LISP ...
;;;

;; (in-package :i-lisp-compiler)

(defconstant +finish-call-n-opcode+ #o134)
