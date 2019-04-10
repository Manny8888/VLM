These are a number of notes gathering information.

# Current status

## Ideally

The current target is to simplify the emulator with the ultimate goal to rewrite
it either in common lisp, Nim, Go or Julia. Nim is currently the most tempting
candidate after seeing the impressively small size of Nimes NES emulator binary:
sub 300k when the Go version on which it is based is around 11M. In the
meantime, the pure C emulator might be looked into.

Currently, the clang compiler is used.

In the meantime, simplification/changes are ongoing with an ongoing integration
test of merely compiling the genera runtime (i.e.not even running it - just
successful compile).

Only one compile target: genera. No iVerify or Minima. Only one platform: Linux.
No FreeBSD, Darwin or MacOSF since I have no access to them. In due time, if
Nim/Go/Julia work, FreeBSD and Darwin should be easier to port into.

## Current environment

What do I use:
- Spacemacs + slime + SBCL for the lisp development
- VSCode for the C development

## How to run:

### Step 1: recreate the C files from the ALPHA assembly

From an SBCL prompt:
- load/compile stub/convert-asm-to-c.lisp
- go into: (in-package :alpha-axp-internals)
- run: (build)

### Step 2: compile the Genera runtime

Ensure the following is available: usual build packages, clang, libpcap-dev

In the top directory:
- make clean
- make genera

Cross fingers...

# Structure of the emulator 

The emulator is split into 2 parts:
- the "life support" that emulates the hardware environment (console, FEP,
  disks, network)
- the emulator proper which emulates the CPU

The life support is common to all emulators and is coded in C.

The emulator comes in different flavours: Alpha assembly, C version started but
unfinished by Symbolics, C version by Brian PARKER. The Alpha version is written
in lisp (the assembly) instructions are lisp functions/macros) that is then
assembled by a lisp assembler. The Brian P. C version basically takes the Alpha
version and assembles it into C instructions (instead of generating Alpha
binary).


# Build system

## .sid files

No idea where .sid files come from.

They are manipulated by assembler/alphadsdl.lisp (DSDL = Document Schema
Definition Language?). DSDL is a facility that describes a file type and methods
to convert them into other file types. Here, the .sid files are converted into
.asm (assembly), .c, .h and .lisp files.

## BP Emulator

stub/process.lisp: converts Alpha ISA into C statements, then converts assembly
(written in lisp) files to c files.

## Various

The assembly file describe the instructions as _list of symbols_. Each item in a
list must be a symbol; therefore no item must be evaluated past read time. This
explains why many values are prefixed with the '#.' reader macro which forces a
read-time evaluation (see
<http://www.lispworks.com/documentation/HyperSpec/Body/02_dh.htm>). (Not
entirely sure that this is a correct explanation, but the point remains.)

Far example ```fcallmac.lisp``` line 42: ```#.1_22``` forces the read-time
evaluation of ```1_22``` which is a constant defined as ```2^22```.


# CPU

An ancestor of the LM process instruction set comes from the SECD abstract
machine.

## SECD Abstract machine 
[fn::[[https://www.cs.utah.edu/~mflatt/past-courses/cs6510/public_html/lispm.pdf][See
Architecture of Lisp Machines]]]

### Heap

Cells in use start from memory bottom containing values Terminal cell with
tag+integer value Non-terminal cell + CDR cell address + CAR cell address

Top of memory tracked by Free Pointer f-pointer

### Stack 

* Stack is what instruction operate on.
* Objects to be processed are pushed on by cons’ing a new cell on top of the
  current stack and car of this points to object’s value.
* S-register after such a push points to the new cell.
* Unlike conventional stack, this does not overwrite original inputs.
* Cells garbage collected later.

### Environment ###

* tracked by E–Register
* Points to current value list of function arguments
  - The list is referenced by m/c when a value for the argument is needed.
  - List is augmented when a new environment for a function is created.
  - It’s modified when a previously created closure is unpackedand the pointer
    from the closure’s cdr.
  - replaces the contents of E-register.

* Prior value list designated by E is not overwritten.

### C–Register (Control register/pointer) ###

* Acts as the program counter and points to the memory cell that designates
  through it’s car the next instruction to be executed.
* The instructions are simple integers specifying desired operation.
* Instructions do not have any sub-fields for registers etc. If additional
  information is required, it’s accessed through from the cells chained through
  the instruction cell’s cdr.
* “Increment of PC” takes place by replacement of C registers contents by the
  contents of the last cell used by the instruction.
* For return from completed applications, new function calls and branches, the C
  register is replaced by a pointer provided by some other part of the m/c.


### D–register (Dump register) ###

* Points to a list in memory called “dump”.
* This data structure remembers the state of a function application when a new
  application in that function body is started.
* That is done by appending onto dump the 3 new cells which record in their cars
  the value of registers S, E, and C.
* When the application completes, popping the top of the dump restores those
  registers. This is very similar to call-return sequence in conventional m/c
  for procedure return and activation.

### The SECD Abstract Machine Basic Instruction Set ###

Instruction can be classified into following 6 groups:

1. Push object values onto the S stack.
2. Perform built-in function applications on the S stack and return the result
   to that stack.
3. Handle the if-then-else special form.
4. Build, apply and return from closures representing non-recursive function
   applications.
5. Extend the above to handle recursive functions.
6. Handle I/O and machine control.




Source:
<https://groups.google.com/forum/?nomobile=true#!topic/comp.lang.lisp/jvZIz-uxAIw>

The MIT CADR, the LMI CADR and Symbolics CADR had 2 bits of CDR codes, 6 bits of
tags and 24 bit word addresses.

The LMI Lambda and the TI Explorer I had 2 bits of CDR codes 5 bits of tags and
25 bit word addresses.

The LMI K had no CDR codes, 6 bits of tags and 26 bit word addresses.

