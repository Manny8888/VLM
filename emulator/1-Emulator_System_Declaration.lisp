#+T (defsystem grimoire-emulator
      :name "Grimoire"
      :author "Someone"
      :license "TBD"
      :description "Genera Lisp VM"
      ;; :depends-on ()
      :components ((:module assembler
                    :serial t
                    :components
                    ((:file "Assembler_Assembly_Emission")
                     (:file "Assembler_Constants_File_Generator")
                     ;; (:file "Assembler_System_Compiler")
                     (:file "Assembler_System_Declaration")))
                   (:module emulator
                    :serial t
                    :components
                    ((:file "1-Emulator_System_Declaration")
                     (:file "2-SBCL_Support")
                     (:file "3-Commonly_Used_1-functions")
                     (:file "4-Commonly_Used_2-functions")
                     (:file "5-Field_Extraction_Instructions-macros")
                     (:file "6-Interpretation_Loop_Out_of_Line_Parts-Trapping")
                     (:file "7-Native_Exception_CAR_CDR")
                     (:file "8-Processor_Structure")
                     (:file "Error-1-Error_Table")
                     (:file "Error_Table")
                     (:file "Macros-1-Assembler_Instructions-macros")
                     (:file "Macros-2-Interpreter_Macro")
                     (:file "Includes-1-aihead")
                     (:file "Includes-2-aistat")
                     (:file "Includes-3-traps")
                     ;; (:file "aihead_origin.sid")
                     ;; (:file "aihead_result")
                     (:file "Definitions-1-Memory_Operations")
                     (:file "Definitions-2-Stack_Interface")
                     (:file "Definitions-3-Generic_Dispatch-macros")
                     (:file "Definitions-4-Branch_and_Loop-macros")
                     (:file "Definitions-5-Function_Calling-macros")
                     (:file "Definitions-6-List_Operations-macros")
                     (:file "Definitions-7-Instance_Variable_Accessors-macros")
                     (:file "Definitions-8-ALU_Instructions-macros")
                     (:file "Definitions-9-Arithmetic-macros")
                     (:file "Definitions-9-Predicates-macros")
                     (:file "Definitions-10-Array_Instructions-macros")
                     (:file "Definitions-11-Subprimitives-macros")
                     (:file "Definitions-12-Lexical_Variable_Accessors-macros")
                     (:file "Definitions-13-Logical_Instructions_macros")
                     (:file "Definitions-14-Block_Instructions-macros")
                     (:file "Definitions-15-Binding_Instructions-macros")
                     (:file "Definitions-16-Flow_Trapping-macros")
                     (:file "Definitions-17-AI_Instructions-macros")
                     (:file "Emulator-1-Interpretation_Loop_Entry_Point")
                     (:file "Emulator-2-Instructions_Dispatch_Loop")
                     (:file "Emulator-3-Branch_and_Loop-functions")
                     (:file "Emulator-3-Generic_Dispatch-functions")
                     (:file "Emulator-4-Function_Calling-functions")
                     (:file "Emulator-5-List_Operations-functions")
                     (:file "Emulator-6-Instance_Variable_Accessors-functions")
                     (:file "Emulator-8-Arithmetic-functions")
                     (:file "Emulator-9-Array_Instructions-functions")
                     (:file "Emulator-10-Data_Movement-functions")
                     (:file "Emulator-11-Predicates-functions")
                     (:file "Emulator-12-Subprimitives-functions")
                     (:file "Emulator-13-Field_Extraction_Instructions-functions")
                     (:file "Emulator-13-Lexical_Variable_Accessors-functions")
                     (:file "Emulator-14-Logical_Instructions_functions")
                     (:file "Emulator-15-Block_Instructions-functions")
                     (:file "Emulator-16-Binding_Instructions-functions")
                     (:file "Emulator-17-Full_Word_Instructions-functions")
                     (:file "Emulator-18-Bignums-functions")
                     (:file "Emulator-19-Flow_Trapping-functions")
                     (:file "Emulator-20-Floating_Point-functions")
                     (:file "Emulator-21-AI_Instructions-functions")
                     (:file "externals.c")
                     (:file "ivoryrep.h")
                     (:file "aistat.h")
                     (:file "aistat.s")
                     (:file "asmfuns.h")
                     (:file "memory.h")
                     (:file "aistat.c")
                     (:file "interfac.c")
                     (:file "interpds.c")
                     (:file "memory.c")
                     (:file "kludges.s")
                     (:file "tranrule")
                     (:file "translat")
                     ))))




(defpackage :grimoire-emulator
  (:use :cl :assembler))
