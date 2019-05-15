(in-package :grimoire)

;;; Macros in support of field extraction.

(defmacro ldb-shift (value position result)
  (let ((noshift (gensym)))
    `((BEQ ,position ,noshift "No shifting needed when byte position is zero")
      (SLL ,value ,position ,result)
      (SRL ,result 32 ,result "t4 is the shifted field")
      (label ,noshift))))
