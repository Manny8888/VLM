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

Only 2 compile targets: genera (the current one) via the old makefile and 
genera_c, the C emulator via cmake. No iVerify or Minima. Only one platform: Linux.
No FreeBSD, Darwin or MacOSF since I have no access to them. In due time, if
Nim/Go/Julia work, FreeBSD and Darwin should be easier to port into.

- The last commit at which Brad's version still compiles is
  https://github.com/Manny8888/vlm/commit/da42b63bdc4eb1b18f816f7fbb93bccef0f8b028

  After that, changes are targeted to making the c-emulator compile with no
  attention whatsoever paid to that version.

- Last version got the c-emulator compile, although _NO VERSION HAS BEEN CHECKED
  THAT THE EMULATOR ACTUALLY RUNS_!

- Having a much reduced code base which is more understandable than Alpha
  assembly leaves me free to move to the next step: rewrite in Nim in another
  repo.


## Current environment

What do I use:
- Spacemacs + slime + SBCL for the lisp development
- VSCode or KDevelop for the C development
- VSCode for Nim
- cmake is used in VSCode to build the c-emulator (add the relevant cmake extensions in VSCode if desired)

## How to run:

### Step 1: recreate the C files from the ALPHA assembly

From an SBCL prompt:
- load/compile stub/convert-asm-to-c.lisp
  *WARNING*: output4.c had to be patched following the supplied diff file. Checked this is OK!

- go into: (in-package :alpha-axp-internals)
- run: (build)


### Step 2: compile the Genera runtime

Ensure the following is available: usual build packages, clang, libpcap-dev

In the top directory:
- make clean
- make genera

Cross fingers...


### Alternative Step 2: compile the Genera_c runtime

Ensure the following is available: usual build packages, clang, libpcap-dev, cmake, ninja
Click the VSCode build button... 


# Structure of the emulator 

The emulator is split into 3 parts:
- the "life support" that emulates the hardware environment (console, FEP,
  disks, network)
- the emulator proper which emulates everything but the CPU
- the CPU

## Life support

The life support is common to all emulators and is coded in C.

It grew out of the Mac Ivory environment where the life support package was
software available on the Mac to interface with the Ivory add-on hardware card
providing the Lisp Machine.

In the Genera version, the emulator replaces the physical add-on card, the
purpose of life support remains the same.

## Emulator

The emulator comes in different flavours: Alpha assembly, C version started but
unfinished by Symbolics, C version by Brian PARKER. The Alpha version is written
in lisp (the assembly) instructions are lisp functions/macros) that is then
assembled by a lisp assembler. The Brian P. C version basically takes the Alpha
version and assembles it into C instructions (instead of generating Alpha
binary).

The CPU is provided by the Aplha assembly, the Alpha assembly compiled to C, or the C emulator (emulator.c).

# Build system

## .sid files

No idea where .sid files come from.

They are manipulated by assembler/alphadsdl.lisp (Does DSDL mean Document Schema
Definition Language?). As best as I can see, DSDL is a LM facility that
describes a file type and methods to convert it into other file types. Here, the
.sid files are converted into .as (Alpha assembly), .c, .h and .lisp files.

Note that I have completely ignored the Power G5 version.

## BP Emulator 

(BP = Brian PARKER)

```stub/process.lisp```: converts Alpha ISA into C statements, then converts assembly
(written in lisp) files to c files. It has been renamed ```convert-asm-to-c.lisp```.

## Various notes

The assembly file describe the instructions as _list of symbols_. Each item in a
list must be a symbol; therefore no item must be evaluated past read time. This
explains why many values are prefixed with the '#.' reader macro which forces a
read-time evaluation (see
<http://www.lispworks.com/documentation/HyperSpec/Body/02_dh.htm>). (Not
entirely sure that this is a correct explanation, but the point remains.)

Far example ```fcallmac.lisp``` line 42: ```#.1_22``` forces the read-time
evaluation of ```1_22``` which is a constant defined as ```2^22```.

