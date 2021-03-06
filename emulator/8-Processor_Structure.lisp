
(in-package :grimoire)

;;; The processor state object is the big daddy.  It acts like a data segment
;;; for the interpreter.  It constantly resides in a register, and provides
;;; direct addressing to its components.  Everything that the interpreter
;;; needs can be found here.  Note: this object should not be grown larger
;;; than 64 K bytes or 4K 64 bit words.

;;; Note well!  All of these slots are grouped in units of four "related"
;;; longwords to minimize Alpha dcache thrashing.  Do *not* add or remove
;;; anything from these 4-longword groups.  Instead, use a pad slot, or
;;; create a new 4-longword group.

;;; This structure is indexed "backwards" from the Ivory register in
;;; assembly code (c.f., the :base-pointer slot which clues in the dsdl
;;; processor to emit proper offsets) and Lisp tagspace is indexed
;;; forwards from Ivory

(define-structure (processorstate)
    ;; *** DO NOT REORDER THE FOLLOWING SLOTS ***
    (:unsigned-long transpare3)
  (:unsigned-long transpare2)
  (:unsigned-long transpare1)
  (:unsigned-long carcdrsubroutine)

  (:unsigned-long cdrsubroutine)
  (:unsigned-long carsubroutine)
  (:unsigned-long linkage)
  (:unsigned-long resumeema)			;resume procedure address.
  ;; *** END OF DO NOT REORDER ***

  (:pointer statistics)				; Instruction usage statistics
  (:pointer trace-hook)				; function called to do instruction tracing
  (:signed-long instruction-count)		; number of instructions executed so far.
  (:unsigned-long long-pad0)

  (:unsigned-long asrr9)
  (:unsigned-long asrr10)
  (:unsigned-long asrr11)
  (:unsigned-long asrr12)

  (:unsigned-long asrr13)
  (:unsigned-long asrr14)
  (:unsigned-long asrr15)
  (:unsigned-long long-pad1)

  (:unsigned-long asrr26)
  (:unsigned-long asrr27)
  (:unsigned-long asrr29)
  (:unsigned-long asrr30)

  ;; The floating point registers
  (:unsigned-long asrf2)
  (:unsigned-long asrf3)
  (:unsigned-long asrf4)
  (:unsigned-long asrf5)

  (:unsigned-long asrf6)
  (:unsigned-long asrf7)
  (:unsigned-long asrf8)
  (:unsigned-long asrf9)

  ;; The following is a control block for cache miss metering.
  ;; They can be removed when this facility is no longer required.
  (:pointer meterdatabuff)			; the buffer that contains meter data.
  (:unsigned-int meterpos)			; the place where the next data item goes.
  (:unsigned-int metermax)			; the highest value ever recorded.
  (:unsigned-int meterfreq)			; sample size.
  (:unsigned-int metermask)			; mask for wrap
  (:unsigned-int metervalue)			; current number of misses.
  (:unsigned-int metercount)			; number remaining

  (:unsigned-long choiceptr)			; the choice pointer
  (:unsigned-long sstkchoiceptr)		; the structure stack choice pointer
  (:unsigned-long dbcbase)			; dynamic binding cache base
  (:unsigned-long dbcmask)			; dynamic binding cache mask

  (:pointer coprocessorreadhook)		; function called to do coprocessor read
  (:pointer coprocessorwritehook)		; function called to do coprocessor write
  (:pointer flushcaches-hook)			; function called to flush I/D caches
  (:pointer i-stage-error-hook)			; function called to generate an I-STAGE-ERROR

  (:unsigned-long sfp1)
  (:unsigned-long fp0)
  (:unsigned-long fp1)
  (:unsigned-long floating-exception)

  ;; ALU support
  (:unsigned-long aluandrotatecontrol)
  (:unsigned-long rotatelatch)
  (:unsigned-long aluborrow)
  (:unsigned-long aluoverflow)

  (:unsigned-long alulessthan)
  (:unsigned-long aluop)
  (:unsigned-long byterotate)
  (:unsigned-long bytesize)

  (:signed-long bindingstacklimit)		; binding stack limit
  (:signed-long bindingstackpointer)		; binding stack pointer
  (:unsigned-long catchblock)			; the catch block
  (:unsigned-long extraandcatch)		; 1_8 + 1_26

  (:unsigned-long msclockcache)			; Microsecond Clock cache
  (:unsigned-long mscmultiplier)		; microsecond clock multiplier
  (:unsigned-long previousrcpp)
  (:pointer rlink)				; return address.

  (:unsigned-int interruptreg)			; the interrupt register, set only by the interpreter
  (:unsigned-int zoneoldspace)			; the zone oldspace register
  (:unsigned-int ephemeraloldspace)		; the ephemeral oldspace register
  (:unsigned-int int-pad0)
  (:unsigned-long eqnoteql)			; bit mask for types for which EQ is not EQL
  (:unsigned-int lclength)			; list cache length
  (:unsigned-int sclength)			; structure cache length

  (:unsigned-long lcarea)			; the list cache area
  (:unsigned-long lcaddress)			; the list cache
  (:unsigned-long scarea)			; the structure cache area
  (:unsigned-long scaddress)			; the structure cache

  (:unsigned-long restartsp)
  (:unsigned-long stop-interpreter)
  (:unsigned-long immediate-arg)		; temp storage for immediates
  (:unsigned-long continuationcp)		; cp of continuation (or zero)

  (:signed-long continuation)
  (:signed-long control)
  (:signed-long niladdress)
  (:signed-long taddress)

  ;; The four BARs must be adjacent!
  (:signed-long bar0)
  (:signed-long bar1)
  (:signed-long bar2)
  (:signed-long bar3)

  (:signed-long epc)
  (:signed-long fp)
  (:signed-long lp)
  (:signed-long sp)

  (:pointer cp)
  (:unsigned-long fccrmask)			; finish call CR mask
  (:unsigned-int cslimit)			; control stack limit
  (:unsigned-int csextralimit)			; control stack extra limit
  (:pointer trapmeterdata)			; the buffer containing trap meter data

  (:unsigned-long fepmodetrapvecaddress)
  (:unsigned-long trapvecbase)
  (:unsigned-long tvi)				; non-zero if the previous instruction trapped
  (:unsigned-long fccrtrapmask)			; like fccrmask, but with trace bits, too

  (:pointer ptrtype)				; PTRTYPE[datatype] non-zero if it's a pointer
  (:pointer vmattributetable)			; pointer to the VMAttributeTable from memory.c
  (:unsigned-long vma)
  (:signed-long mostnegativefixnum)		; - 1_31

  (:pointer icachebase)				; the icache object.
  (:pointer endicache)				; past the end of the icache object.
  (:unsigned-long fullworddispatch)		; Fullword instruction dispatch table.
  (:unsigned-long halfworddispatch)		; Halfword instruction dispatch table.

  (:signed-long areventcount)			; array register event count
  (:unsigned-long stackcachesize)		; stack cache size
  (:unsigned-long stackcachetopvma)		; highest address in stack cache + 1
  (:unsigned-long cdrcodemask)			; #xC00000000

  (:pointer stackcachedata)			; storage used as the stack cache
  (:unsigned-long stackcachebasevma)		; lowest address in stack cache
  (:unsigned-int scovlimit)			; stack cache overflow limit
  (:unsigned-int scovdumpcount)			; temporary while dumping stack cache
  (:signed-long mostpositivefixnum)		; 1_31 - 1

  ;; Dispatch tables for reading and writing internal registers
  (:unsigned-long internalregisterread1)
  (:unsigned-long internalregisterread2)
  (:unsigned-long internalregisterwrite1)
  (:unsigned-long internalregisterwrite2)

  ;; Memory Action Tables
  (:unsigned-long dataread-mask)
  (:pointer dataread)
  (:unsigned-long datawrite-mask)
  (:pointer datawrite)

  (:unsigned-long bindread-mask)
  (:pointer bindread)
  (:unsigned-long bindwrite-mask)
  (:pointer bindwrite)

  (:unsigned-long bindreadnomonitor-mask)
  (:pointer bindreadnomonitor)
  (:unsigned-long bindwritenomonitor-mask)
  (:pointer bindwritenomonitor)

  (:unsigned-long header-mask)
  (:pointer header)
  (:unsigned-long structureoffset-mask)
  (:pointer structureoffset)

  (:unsigned-long scavenge-mask)
  (:pointer scavenge)
  (:unsigned-long cdr-mask)
  (:pointer cdr)

  (:unsigned-long gccopy-mask)
  (:pointer gccopy)
  (:unsigned-long raw-mask)
  (:pointer raw)

  (:unsigned-long rawtranslate-mask)
  (:pointer rawtranslate)
  ;; Magic bits:
  ;;    The following two longwords must be contiguous and aligned on a quadword boundary.
  ;;	The first is set only by the Spy and the second is set only by Life Support.
  ;;	Both are cleared only by the interpreter.
  (:signed-int please-stop)			; request interpreter to halt if nonzero.
  (:signed-int please-trap)			; request interpreter to trap if nonzero.
  (:signed-long runningp)			; non-zero if running, zero if stopped.

  (:unsigned-long ac0array)			; the automatic array register 0
  (:unsigned-long ac0arword)
  (:unsigned-long ac0locat)
  (:unsigned-long ac0length)

  (:unsigned-long ac1array)			; the automatic array register 1
  (:unsigned-long ac1arword)
  (:unsigned-long ac1locat)
  (:unsigned-long ac1length)

  (:unsigned-long ac2array)			; the automatic array register 2
  (:unsigned-long ac2arword)
  (:unsigned-long ac2locat)
  (:unsigned-long ac2length)

  (:unsigned-long ac3array)			; the automatic array register 3
  (:unsigned-long ac3arword)
  (:unsigned-long ac3locat)
  (:unsigned-long ac3length)

  (:unsigned-long ac4array)			; the automatic array register 4
  (:unsigned-long ac4arword)
  (:unsigned-long ac4locat)
  (:unsigned-long ac4length)

  (:unsigned-long ac5array)			; the automatic array register 5
  (:unsigned-long ac5arword)
  (:unsigned-long ac5locat)
  (:unsigned-long ac5length)

  (:unsigned-long ac6array)			; the automatic array register 6
  (:unsigned-long ac6arword)
  (:unsigned-long ac6locat)
  (:unsigned-long ac6length)

  (:unsigned-long ac7array)			; the automatic array register 7
  (:unsigned-long ac7arword)
  (:unsigned-long ac7locat)
  (:unsigned-long ac7length)

  ;;transactional memory state
  (:unsigned-int tmcurrenttransaction)		; current transaction id (0 means none)
  (:unsigned-int tmwritestart)			; write buffer start
  (:unsigned-int tmwritecurrent)		; write buffer next
  (:unsigned-int tmwritelimit)			; write buffer can't write limit
  (:unsigned-int tmrecordingreads)		; whether current transaction records reads (0 means not)
  (:unsigned-int tmreadstart)			; read buffer start
  (:unsigned-int tmreadcurrent)			; read buffer next
  (:unsigned-int tmreadlimit)			; read buffer can't write limit

  :base-pointer					; Ivory register points here
  (:size size))					; the fixed size


;; The fields in a cacheline are carefully organized so that they are
;; fetched in ascending order in the NextInstruction loop
(define-structure (cacheline)
    ;; The annotation field is used for branch-taken prediction and
    ;; metering.  In the branch-taken case, it will be fetched instead of
    ;; NEXTPC/NEXTCP, so we put it here to start a fill (even though we
    ;; then have to skip 2 quadwords).
    (:unsigned-long annotation)			; serves multiple purposes

  ;; NEXTPCDATA/NEXTPCTAG and NEXTCP get used together, in that order.
  ;; Even though these are not octaword-aligned, we expect cachelines
  ;; for NextInstruction to typically already be loaded.
  (:unsigned-int nextpcdata)			; the Ivory data for the next PC
  (:unsigned-int nextpctag)			; the Ivory tag for the next PC
  (:pointer nextcp)				; the cache entry for the next PC

  ;; PCDATA/PCTAG, INSTRUCTION/OPERAND, and CODE get used together, in
  ;; that order (and after NEXTPC and NEXTCP)

  ;; Nota Bene:  For full-word instructions, the operand and instruction
  ;; fields are concatenated, so that the "pointer" field of the
  ;; instruction can be stored as an unsigned long, that is the
  ;; full-word operand.  For packed instructions, the instruction field
  ;; contains the "pointer" (needed by entry and spare ops) and the
  ;; operand field contains the extracted operand.
  (:unsigned-int instruction)			; the actual instruction for this PC
  (:unsigned-int operand)			; the decoded operand
  (:unsigned-int pcdata)			; the Ivory data for this PC
  (:unsigned-int pctag)				; the Ivory tag for this PC
  (:pointer code)				; pointer to emulator routine

  (:size size))

(define-values (|CacheLine| :parameter)
    (|Bits| 18)					; Number of bits in cache mask
  (|Mask| #.(1- (ash 1 18)))			; Mask for computing cache address.
  (|RShift| 16)					; Shift to the right
  (|LShift| 6)					; Shift to the left
  ;; Must be <= (ash 1 LShift) and <= 1 vm page
  ;; 10 == (floor Prefetch-size cacheline-size)
  (|FillAmount| 20)) ; was 10 for 8k cache


(define-structure (arraycache)
    (:unsigned-long array)
  (:unsigned-long arword)
  (:unsigned-long locat)
  (:unsigned-long length))

(define-values (|AutoArrayReg| :parameter)
    (|Mask| #xE0)
  (|Size| 32)
  (|Shift| 0))


(define-values (|MSclock| :parameter)
    (|UnitsToMSShift| 24)
  (|UnitsPerMicrosecond| 16777216))


;; Stack cache sized to not conflict with processor state in data cache.
;; State is aligned to top of cache and is < 2048 bytes, so stack cache
;; is (8192 - 2048)/8 slots
(define-values (|Stack| :parameter)
    (|CacheSize| 1792) ;768 if 8k
  (|MaxFrameSize| 128) ;128
  ;; Must be >= frame size
  (|CacheMargin| 128) ;128
  ;; Must be >= 2 * cache margin, so that scrolling will clear overflow
  ;; condition; and <= cache-size - (maxframe + 2*margin), so that
  ;; scrolling does not scroll current frame out of stack.
  (|CacheDumpQuantum| 896) ; 384 if 8K -- pr I found a horrible bug in how this is used in the code (stackcacheoverflowhandler) should be fixed+++
  )


;;; These values represent the shift required to get the base address of ivory
;;; emulated memory. The data being at 1<<IvoryMemoryData, tags at 1<<IvoryMemoryTag.
;;; Note that the Data must be 4*Tags (for the tricky code in memoryem to work).
(define-values |IvoryMemory|
    (|Data| 35)
  (|Tag| 33))


;;; This structure defines the registers that are guaranteed to be preserved across a
;;; call in the ALPHA calling standards document.  Macros in alphamac.lisp provide a
;;; convenient means of complying with these obligations when entering and leaving the
;;; interpreter loop.
(define-structure (savedregisters)
    ;; The integer registers
    (:unsigned-long r9)
  (:unsigned-long r10)
  (:unsigned-long r11)
  (:unsigned-long r12)
  (:unsigned-long r13)
  (:unsigned-long r14)
  (:unsigned-long r15)
  (:unsigned-long r29)
  ;; The floating point registers
  (:unsigned-long f2)
  (:unsigned-long f3)
  (:unsigned-long f4)
  (:unsigned-long f5)
  (:unsigned-long f6)
  (:unsigned-long f7)
  (:unsigned-long f8)
  (:unsigned-long f9)

  (:size size))


;;; Instruction tracing ...

(define-structure (tracedata)
    (:unsigned-long n_entries)
  (:unsigned-int recording_p)
  (:unsigned-int wrap_p)
  (:unsigned-long start_pc)
  (:unsigned-long stop_pc)
  (:pointer records_start)
  (:pointer records_end)
  (:pointer current_entry)
  (:pointer printer)

  (:size size))

(define-structure (tracerecord)
    (:unsigned-long counter)
  (:unsigned-long epc)
  (:unsigned-long tos)
  (:unsigned-long sp)
  (:pointer instruction)
  (:unsigned-long instruction_data)
  (:unsigned-int operand)
  (:unsigned-int trap_p)
  (:unsigned-long trap_data_0)
  (:unsigned-long trap_data_1)
  (:unsigned-long trap_data_2)
  (:unsigned-long trap_data_3)
  (:unsigned-int catch_block_p)
  (:unsigned-int int-pad0)
  (:unsigned-long catch_block_0)
  (:unsigned-long catch_block_1)
  (:unsigned-long catch_block_2)
  (:unsigned-long catch_block_3)

  (:size size))


(define-values (|CacheMeter| :parameter)
    (|Pwr| 14)
  (|DefaultFreq| 1000))
