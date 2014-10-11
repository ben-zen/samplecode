;; Copyright (C) 2014 Ben Lewis <benjf5@gmail.com>

;; define SAL faces
(defface sal-function-param-face
  '((t (:inherit font-lock-builtin-face))) ;; might not want builtin-face
  "Face for SAL function parameter annotations.")
(defvar sal-function-param-face 'sal-function-param-face)
;; Additional faces for other parameter types should be added later.

(defconst function-annotation-keywords
  (list
   '("\\<_In_\\>" 0 sal-function-param-face))
  "Keywords for annotating function parameters.")

(defun sal-mode-add-font-locks ()
  (font-lock-add-keywords
   nil
   function-annotation-keywords t))

(define-minor-mode sal-mode
  "Support Microsoft SAL2 in C++-mode."
  :lighter " SAL"
  (sal-mode-add-font-locks))

(provide 'sal-mode)

