;;; Functions to implement American casino rules craps.

(defun throw-die () "Generate a roll from a six-sided die." (1+ (random 6)))

(defun throw-dice () "Generate a pair of rolls of six-sided dice."
       (list (throw-die) (throw-die)))

(defun snake-eyes-p (l) "Determine if a roll was snake-eyes (two ones)."
       (and
        (equal (car l) 1)
        (equal (car l) (cadr l))))

(defun boxcars-p (l) "Determine if a roll was boxcars (two sixes)."
       (and
        (equal (car l) 6)
        (equal (car l) (cadr l))))

(defun sum-throw (l) "Produce the sum of a throw of dice." (+ (car l) (cadr l)))

(defun instant-win-p (toss)
  "Check if a throw was a first-throw instant win in American casino rules."
  (or
   (equal (sum-throw toss) 7)
   (equal (sum-throw toss) 11)))

(defun instant-loss-p (toss)
  "Check if a throw was a first-throw loss in American casino rules."
  (or
   (snake-eyes-p toss)
   (boxcars-p toss)
   (equal (sum-throw toss) 3)))

(defun say-throw (toss) "Print either the sum of the dice or a special term."
       (cond
         ((snake-eyes-p toss) 'SNAKE-EYES)
         ((boxcars-p toss) 'BOXCARS)
         (t (sum-throw toss))))

(defvar point) ; Create a value to store the current point value
(setf point nil)

(defun craps () "Start a game of craps."
       (let (toss (throw-dice))
         (cond
           ((instant-win-p toss)
            (list 'throw (car toss) 'and (cadr toss)
                  '-- (say-throw toss) '-- 'you 'win))
           ((instant-loss-p toss)
            (list 'throw (car toss) 'and (cadr toss)
                  '-- (say-throw toss) '-- 'you 'lose))
           (t
            (and
             (setf point (sum-throw toss))
             (list 'throw (car toss) 'and (cadr toss)
                      '-- 'your 'point 'is (say-throw toss))))
           )
         )
       )

(defun try-for-point () "With a point already defined, try to win."
       )
