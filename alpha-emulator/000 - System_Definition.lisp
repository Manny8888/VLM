(defpackage :alpha-axp-internals
  (:nicknames AXPI)
  (:use :cl :asdf)
  (:shadow AND))

(in-package :alpha-axp-internals)

(defsystem alpha-axp-internals
  :name alpha-axp-internals
  :version "0.0.1"
  :components ((:file "001 - Assembler_Instructions-macros")
               (:file "002 - Memory_Operations")
               (:file "003 - Interpreter_macrosStack_Interface")
  ;;             (:file "99-Alpha_DSDL"))
  ))


