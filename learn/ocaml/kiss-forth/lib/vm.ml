(* DELME *)
module Name = struct
  let v : string = "me"
  external v_c : unit -> string = "name_value_4ocaml"
end

(* Exists to define platform-specific limits.
   Of course, this will likely be an 8-bit byte and 32-bit words,
   but perhaps a 31-bit word or 6-bit byte is possible.
   My separating this out as a signature, further structs can serve
   to implement the VM on many sorts of machines.
 *)
module type MACHINE = sig

  (* Platform Architecture *)
  module Arch : sig
    type word
    type byte
  end


  module Stack : sig
    type t
    val init : t

    val push : Arch.word * t -> t
    val pop : t -> Arch.word * t
    (* TODO peek, dupAt *)
  end

  module Instruction : sig
    type t =
      | Halt
      | Imm of Arch.word
  end

end

(* I'm not sure if this really should exist, but 8-bit byte, 32-bit words. *)
module Prototype8b32w_Inner = struct
  module Arch = struct
    type word = Int32.t
    type byte = char
  end
  open Arch

  module Stack = struct
    type t = word list
    let init = []

    exception StackUnderflow

    let push (v, mem) = v :: mem
    let pop mem = match mem with
      v :: rest -> (v, rest)
      | [] -> raise StackUnderflow
  end

  (* Ocaml has an unsigned byte array, and a signed int32 array, but not really a way to alias them.
    So, it looks like I may need some C code quite quickly after all!
    Gotta love pointer casting and arithmetic lmao.
    TODO Actually, I'm just going to explicitly do the bitshifts for some endianness.
  *)

  module Instruction = struct
    type t =
      | Halt
      | Imm of word

    open Sexplib
    let to_sexp i = match i with
      | Halt -> Sexp.(Atom "Halt")
      | Imm v -> Sexp.(Atom (Int32.to_string v))
    let of_sexp e = match e with
      | Sexp.Atom "Halt" -> Halt
      | Sexp.Atom other -> Imm (Int32.of_string other)
      | Sexp.List _ -> raise (Failure "not a kiss-forth instruction")
  end

end

module Prototype8b32w : MACHINE = Prototype8b32w_Inner

(* I'm thinking about targeting 15-bit address space for the heap and a 4kiB stack.
   Perhaps a 14-bit addr space (16kiB) is enough for the simple compilers/interpreters I have in mind.
 *)

module type FOO = sig
  type t
  val hello : t
end

module FOO_CODEC (Base : FOO) = struct
  module type CODEC = sig
    val to_bytes : Base.t -> string
  end
end

