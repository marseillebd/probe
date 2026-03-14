#include "name.h"

#include <caml/alloc.h>
#include <caml/memory.h>

CAMLprim
value name_value_4ocaml(value unit) {
  CAMLparam1(unit);
  CAMLlocal1(ostr);
  ostr = caml_copy_string(name);
  CAMLreturn(ostr);
}
