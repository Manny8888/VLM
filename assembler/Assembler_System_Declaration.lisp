(defsystem :grimoire-assembler
  (:pretty-name "Assembler")
  (:serial
   "Assembler_Assembly_Emission"
   "Assembler_Constants_File_Generator"
   ;; "Assembler_System_Compiler"
   ))

(defpackage :grimoire-assembler
  (:use :common-lisp)
  (:export collecting-function-epilogue))
