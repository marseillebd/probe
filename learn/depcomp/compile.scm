#lang typed-scheme
(require scheme/string)
(require scheme/file)

(: parse-file (String -> Number))
(define (parse-file inp)
  (define r (string->number inp))
  (cond
    (r r)
    (else
      (display "parse error\n")
      (exit 2))))


(: compile-program (Number -> String))
(define (compile-program ast)
  (compile-function "main" ast))

(: compile-function (-> String Number String))
(define (compile-function name ast)
  (string-join
    (list
      "export function w $main() {"
      "  @start"
      "  %v0 =w copy 42"
      "  ret %v0"
      "}"
      "")
    "\n"))

(display (compile-program
           (parse-file
             "0"
            ; (file->string "test.toy")
             )))
