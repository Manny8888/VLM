
(in-package :alpha-axp-internals)

(defun %32-bit-difference (x y)
  (- x y))

;; From: http://www.sbcl.org/manual/#Defining-Constants
(defmacro define-constant (name value &optional doc)
  `(defconstant ,name (if (boundp ',name) (symbol-value ',name) ,value)
     ,@(when doc (list doc))))

(defmacro defsysconstant (name value)
  `(progn
     (define-constant ,name ,value)
     (export ',name)))

(defmacro defenumerated (list-name code-list &optional (start 0) (increment 1) end)
  (when (and end (not (= (length code-list) (/ (- end start) increment))))
    (error "~s has ~s codes where ~s are required"
	   list-name (length code-list) (/ (- end start) increment)))
  `(progn
     (defsysconstant ,list-name ',code-list)
     ,@(loop for code in code-list and prev = 0 then code
	     as value from start by increment
	     unless (eq code prev)		;kludge for data-types
	       collect `(defsysconstant ,code ,value))))

(defmacro defsysbyte (name size position)
  `(defsysconstant ,name (byte ,size ,position)))

(defmacro %alu-function-dpb (background rotate-latch)
  `(%logdpb  %alu-function-op-dpb (byte 3 3)
	           (%logdpb ,rotate-latch (byte 1 2)
		                  (%logdpb ,background (byte 2 0)
			                         0))))
(export '%alu-function-dpb)


;;; Powers of 2 constants
(define-constant 1_0  #.(ash 1  0))
(define-constant 1_1  #.(ash 1  1))
(define-constant 1_2  #.(ash 1  2))
(define-constant 1_3  #.(ash 1  3))
(define-constant 1_4  #.(ash 1  4))
(define-constant 1_5  #.(ash 1  5))
(define-constant 1_6  #.(ash 1  6))
(define-constant 1_7  #.(ash 1  7))
(define-constant 1_8  #.(ash 1  8))
(define-constant 1_9  #.(ash 1  9))
(define-constant 1_10 #.(ash 1 10))
(define-constant 1_11 #.(ash 1 11))
(define-constant 1_12 #.(ash 1 12))
(define-constant 1_13 #.(ash 1 13))
(define-constant 1_14 #.(ash 1 14))
(define-constant 1_15 #.(ash 1 15))
(define-constant 1_16 #.(ash 1 16))
(define-constant 1_17 #.(ash 1 17))
(define-constant 1_18 #.(ash 1 18))
(define-constant 1_19 #.(ash 1 19))
(define-constant 1_20 #.(ash 1 20))
(define-constant 1_21 #.(ash 1 21))
(define-constant 1_22 #.(ash 1 22))
(define-constant 1_23 #.(ash 1 23))
(define-constant 1_24 #.(ash 1 24))
(define-constant 1_25 #.(ash 1 25))
(define-constant 1_26 #.(ash 1 26))
(define-constant 1_27 #.(ash 1 27))
(define-constant 1_28 #.(ash 1 28))
(define-constant 1_29 #.(ash 1 29))
(define-constant 1_30 #.(ash 1 30))
(define-constant 1_31 #.(ash 1 31))






;;;
;;; The following definitions are from SYS:I-SYS;SYSDEF.LISP ...
;;;

;; --- most of the below is L-specific
;; To add a new data type, update the following (at least):
;;	*DATA-TYPES* and *POINTER-DATA-TYPES* in this file
;;	Patch *DATA-TYPE-NAME*, set up by from *DATA-TYPES* by the cold-load generator
;;	type-map-for-transport, transporter-type-map-alist in sys: l-ucode; uu.lisp
;;	*storing-type-map* in sys: l-ucode; uux.lisp and reload that whole file
;;	It is important that the form near the end of that file that sets up the
;;	no-trap type-map be executed before any other type maps are assigned.
;;	simulate-transporter in sys: l-ucode; simx.lisp
;;	and recompile the whole microcode to get the type-maps updated
;;	typep-alist and related stuff in sys: sys; lcons.lisp
;;	dbg:*good-data-types* if it is indeed a good data type
;;	Send a message to the maintainer of the FEP-resident debugger.

(defenumerated *data-types* 
    ;; headers, special markers, and forwarding pointers.
    (dtp-null               ;00 unbound variable/function, uninitialized storage
     dtp-monitor-forward    ;01 this cell being monitored
     dtp-header-p           ;02 structure header, with pointer field
     dtp-header-i           ;03 structure header, with immediate bits
     dtp-external-value-cell-pointer   ;04 invisible except for binding
     dtp-one-q-forward                 ;05 invisible pointer (forwards one cell)
     dtp-header-forward         ;06 invisible pointer (forwards whole structure)
     dtp-element-forward			  ;07 invisible pointer in element of structure

     ;; numeric data types.
     dtp-fixnum                   ;10 small integer
     dtp-small-ratio              ;11 ratio with small numerator and denominator
     dtp-single-float             ;12 single-precision floating point
     dtp-double-float             ;13 double-precision floating point
     dtp-bignum                   ;14 big integer
     dtp-big-ratio                ;15 ratio with big numerator or denominator
     dtp-complex                  ;16 complex number
     dtp-spare-number             ;17 a number to the hardware trap mechanism

     ;; instance data types.
     dtp-instance                  ;20 ordinary instance
     dtp-list-instance             ;21 instance that masquerades as a cons
     dtp-array-instance            ;22 instance that masquerades as an array
     dtp-string-instance           ;23 instance that masquerades as a string

     ;; primitive data types.
     dtp-nil                       ;24 the symbol nil
     dtp-list                      ;25 a cons
     dtp-array                     ;26 an array that is not a string
     dtp-string                    ;27 a string
     dtp-symbol                    ;30 a symbol other than nil
     dtp-locative                  ;31 locative pointer
     dtp-lexical-closure           ;32 lexical closure of a function
     dtp-dynamic-closure           ;33 dynamic closure of a function
     dtp-compiled-function         ;34 compiled code
     dtp-generic-function          ;35 generic function (see later section)
     dtp-spare-pointer-1           ;36 spare
     dtp-spare-pointer-2           ;37 spare
     dtp-physical-address          ;40 physical address
     dtp-spare-immediate-1         ;41 spare
     dtp-bound-location            ;42 deep bound marker
     dtp-character                 ;43 common lisp character object
     dtp-logic-variable            ;44 unbound logic variable marker
     dtp-gc-forward                ;45 object-moved flag for garbage collector
     dtp-even-pc                   ;46 pc at first instruction in word
     dtp-odd-pc                    ;47 pc at second instruction in word

     ;; full-word instructions.
     dtp-call-compiled-even        ;50 start call, address is compiled function
     dtp-call-compiled-odd         ;51 start call, address is compiled function
     dtp-call-indirect             ;52 start call, address is function cell
     dtp-call-generic              ;53 start call, address is generic function
     dtp-call-compiled-even-prefetch ;54 like above, but prefetching is desireable
     dtp-call-compiled-odd-prefetch ;55 like above, but prefetching is desireable
     dtp-call-indirect-prefetch    ;56 like above, but prefetching is desireable
     dtp-call-generic-prefetch     ;57 like above, but prefetching is desireable

     ;; half-word (packed) instructions consume 4 bits of data type field (opcodes 60..77).
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

;;; control register.

(defsysbyte %%cr.argument-size 8. 0)	  ;number of spread arguments supplied by caller
(defsysbyte %%cr.apply 1 17.)		  ;1 if caller used apply, 0 otherwise
(defsysbyte %%cr.value-disposition 2 18.) ;the value of this function
(defsysbyte %%cr.cleanup-bits 3 24.)	  ;all the cleanup bits
(defsysbyte %%cr.cleanup-catch 1 26.)	  ;there are active catch blocks in the current frame
(defsysbyte %%cr.cleanup-bindings 1 25.)  ;there are active bindings in the current frame
(defsysbyte %%cr.trap-on-exit-bit 1 24.)  ;software trap before exiting this frame
(defsysbyte %%cr.trap-mode 2 30.)	  ;1 if we are executing on the "extra stack"
;; extra stack inhibits sequence breaks and preemption
;; it also allows the "overflow" part of the stack to
;; be used without traps.
(defsysbyte %%cr.extra-argument 1 8.)	  ;the call instruction supplied an "extra" argument
(defsysbyte %%cr.caller-frame-size 8 9.)  ;the frame size of the caller
(defsysbyte %%cr.call-started 1 22.)	  ;between start-call and finish-call.
(defsysbyte %%cr.cleanup-in-progress 1 23.)
(defsysbyte %%cr.instruction-trace 1 29.)
(defsysbyte %%cr.call-trace 1 28.)
(defsysbyte %%cr.trace-pending 1 27.)
(defsysbyte %%cr.trace-bits 3 27.)

(defsysbyte %%cr.cleanup-and-trace-bits 6 24.)

(defenumerated *value-dispositions*
    (value-disposition-effect           ;the callers wants no return values
     value-disposition-value            ;the caller wants a single return value
     value-disposition-return    ;the caller wants to return whatever values are
                                        ;returned by this function
     value-disposition-multiple         ;the callers wants multiple values
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
     %memory-raw-translate
     ))

;;; internal register definitions

;;; %register-alu-and-rotate-control fields (dp-op in hardware spec)

(defsysbyte %%alu-byte-r	   5  0.)
(defsysbyte %%alu-byte-s	   5  5.)
(defsysbyte %%alu-function         6 10.)
(defsysbyte %%alu-function-class   2 14.)
(defsysbyte %%alu-function-bits    4 10.)
(defsysbyte %%alu-condition	   5 16.)
(defsysbyte %%alu-condition-sense  1 21.)

;; the following are implemented in rev3 only.
;; software forces them to the proper value for compatible operation in rev1 and rev2.
(defsysbyte %%alu-output-condition 1 22.)
(defsysbyte %%alu-enable-condition-exception 1 23.)
(defsysbyte %%alu-enable-load-cin 1 24.)

(defenumerated *alu-condition-senses*
  (%alu-condition-sense-true
   %alu-condition-sense-false))

(defenumerated *alu-conditions*
  (%alu-condition-signed-less-than-or-equal	  ;; #o00
   %alu-condition-signed-less-than		  ;; #o01
   %alu-condition-negative			  ;; #o02
   %alu-condition-signed-overflow		  ;; #o03
   %alu-condition-unsigned-less-than-or-equal	  ;; #o04
   %alu-condition-unsigned-less-than		  ;; #o05
   %alu-condition-zero				  ;; #o06
   %alu-condition-high-25-zero			  ;; #o07
   %alu-condition-eq				  ;; #o10
   %alu-condition-op1-ephemeralp		  ;; #o11
   %alu-condition-op1-type-acceptable		  ;; #o12
   %alu-condition-op1-type-condition		  ;; #o13
   %alu-condition-result-type-nil		  ;; #o14
   %alu-condition-op2-fixnum			  ;; #o15
   %alu-condition-false				  ;; #o16
   %alu-condition-result-cdr-low		  ;; #o17
   %alu-condition-cleanup-bits-set		  ;; #o20
   %alu-condition-address-in-stack-cache	  ;; #o21
   %alu-condition-pending-sequence-break-enabled  ;; #o22
   %alu-condition-extra-stack-mode		  ;; #o23
   %alu-condition-fep-mode			  ;; #o24
   %alu-condition-fp-coprocessor-present	  ;; #o25
   %alu-condition-op1-oldspacep			  ;; #o26
   %alu-condition-stack-cache-overflow		  ;; #o27
   %alu-condition-or-logic-variable		  ;; #o30
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
(defsysconstant %db-unwind-catch-trap-vector #o5127)
;;; 5130 through 5177 reserved for future use


;;;
;;; the following definitions are from sys:i-sys;opsdef.lisp ...
;;;

(define-constant *finish-call-n-opcode* #o134)
