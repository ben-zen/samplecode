;; Copyright (C) 2014 Ben Lewis <benjf5@gmail.com>

(define-minor-mode sal-mode
  "Support Microsoft SAL2 in C++-mode."
  :lighter " SAL")

(defconst function-annotation-keywords
  (list
  '("\\<\\(_\\(?:\\(?:In\\(?:_\\(?:opt\\(?:_z\\)?\\|reads\\(?:_\\(?:bytes\\(?:_opt\\)?\\|o\\(?:pt\\(?:_z\\)?\\|r_z\\)\\|to_ptr\\(?:_\\(?:opt\\(?:_z\\)?\\|z\\)\\)?\\|z\\)\\)?\\|z\\)\\|out\\(?:_\\(?:opt\\(?:_z\\)?\\|updates\\(?:_\\(?:all\\(?:_opt\\)?\\|bytes\\(?:_\\(?:all\\(?:_opt\\)?\\|opt\\|to\\(?:_opt\\)?\\)\\)?\\|opt\\(?:_z\\)?\\|to\\(?:_opt\\)?\\|z\\)\\)?\\|z\\)\\)?\\)?\\|Out\\(?:_\\(?:opt\\|writes\\(?:_\\(?:all\\(?:_opt\\)?\\|bytes\\(?:_\\(?:all\\(?:_opt\\)?\\|to\\(?:_opt\\)?\\)\\)?\\|opt\\(?:_z\\)?\\|to\\(?:_\\(?:opt\\|ptr\\(?:_\\(?:opt\\(?:_z\\)?\\|z\\)\\)?\\)\\)?\\|z\\)\\)?\\)\\)?\\)_\\)\\)\\>"
  . font-lock-function-parameter-annotation-face))
  "Keywords for annotating function parameters.")

