# A Toy Compiler

I know I want to bring dependent types to C(-like languages),
  but I've never properly written a compiler before.
So, I'm following the methodology of
  Abdulaziz Ghuloum's "[An Incremental Approach to Compiler Construction][inc-comp-constr]".
They use Scheme as both implementation and source language, but that is too different from the source language I want to implement.
For my source language, I will use Loh, McBride, and Swierstra's [Simply Easy! An Implementation of a Dependently Typed Lambda Calculus][simply-easy].

[inc-comp-constr]: http://scheme2006.cs.uchicago.edu/11-ghuloum.pdf
[simply-easy]: http://strictlypositive.org/Easy.pdf

## Preliminary Issues

My target audience is, well, just me.
I've not written a real compiler before, but have implemented plenty of interpreters, some type checkers,
  and have experience with systems programming and assembly.

### The Source Language WIP

Following [simply-easy][], there will be two languages, a simply-typed lambda calculus and a dependently-typed lambda calculus.
However, I do want to throw in a number of wrinkles even to the dependently typed language.

First, and _maybe_ most easily: staged computation, ie splicing, compiletime metaprogramming, &c.
Second, and this has been explored elsewhere (Idris2, Austral), resource/linear types.
Finally, I want to distinguish terms for their availability: compiletime vs runtime aka host vs target, and ofc it could be both.

Host/target checking I think would do a lot for being able to use a functional dependent language to proove and metaprogram,
  while still targeting a system with no gc or closures.

### The Implementation Language WIP

I've been stuck on streaming algorithms, but is the complexity really worth it?
It proves algorithmic properties about lookahead, but today's memory is easily capable of loading several files into memory at once.
The easiest way is probably to just do each pass in-full.

I'd like to fully develop under nix, but that means using nix for dependency management.
That's easy enough in C, and maybe even in Scheme(s), but I haven't figured it out in Haskell or OCaml.

- C:
  +obvious "path to ground",
  -manual memory management can obscure core algorithms,
  -batteries not included,
  -poor type system
- Rust:
  +batteries included,
  -ownership/lifetime can obscure core algorithms,
  -ridiculous "path to ground" (ie no standard, and only one implementation)
- Scheme:
  +fairly easy "path to ground",
  +good support for recursion,
  -untyped makes it difficult to evolve,
  +allows us to follow the paper more closely
- Haskell:
  +familiar to me,
  +excellent support for recursion,
  -inspires overspending my complexity budget,
  -extremely difficult "path to ground"
- OCaml:
  +excellent support for recursion,
  -somewhat unfamiliar
-[Typed Scheme](http://download.plt-scheme.org/doc/4.2.2/ts-guide/index.html)
  +fairly easy "path to ground",
  +good support for recursion,
  -obscure (because who knows how well it's supported?),
  +allows us to follow the paper more closely
- Go:
  -I don't know it at all,
  +alledgedly super easy to learn,
  +not _too_ much power (for tying myself up in abstractions or pathing to ground)
  -[no TCO with only 2KiB of stack depth](https://mattermost.com/blog/a-deep-dive-into-deeply-recursive-go/) make recursive algorithms not scale very well



To make C work I would need to develop liba as I go.
Ofc a string table and symbol table, but likely some form of garbage collection or at least reference counting for manipulating all these trees.
That said, if the target language is C (which would make an FFI fairly easy), then I might need liba as a support for the rts anyway.

#### Update 2026-07-18

Typed Scheme takes a second to compile/type-check, which is extremely concerning for how small the program is.
The primary reason I chose it despite its obscurity was it's simple "path to ground"
  (and ofc that it otherwise had good properites for wirking with structural recursion).
After seeing its limitations, I'm beginning to think the obscurity (and thus lack of implementation maturity vis-a-vis at least performance)
  is too much of a cost to pay for the path to ground, which is honestly kinda a pet thing for me.
Perhaps it's better to use a more mature language like Haskell despite the difficult path to ground,
  simply because path to ground could be obtained by porting from a less easy-to-implement language.

Finding documentation on typed scheme is not the easiest:
- [An Intro](http://download.plt-scheme.org/doc/4.2/ts-guide/index.html)
- [The Reference](http://download.plt-scheme.org/doc/4.2/ts-reference/index.html)
- It uses the [Scheme Libraries](http://download.plt-scheme.org/doc/4.2/reference/index.html)


### Choosing the Target Architecture WIP

First thoughts:
  x86 is weird and ofc not portable;
  llvm has hatable docs and is quite heavyweight;
  QBE is much smaller than llvm but is again very low level;
  C oddly is a decent target since it already has good compilers, is very portable, and enables low-level stuff, despite its undefined behavior;
  C-- is too obscure, but it does have GC, so can serve as an inspiration;
  Scheme already has compilers and GC and is not so difficult to compile itself, it would focus on the dependent typing rather than the functional compilation;

I am giving genuine thought to compiling to (untyped) Scheme (a simpler version).
The Ghuloum paper already tells me how to compile scheme, so a second step can take that scheme all the way to a RAM-machine-like
As for what instruction set to target, I'm leaning towards QBE, at least at first.
It would be awesome to then re-implement QBE in my language.

I'm only worried about a couple of things with QBE: tail calls and inline assembly.
I dont' recall reading anywhere that those are supported.
Ofc, the Abdulaziz paper uses trampolines, which I'd like to learn for myself anyway.
And if I don't care about performance as much, I can use QBE's ability to call C to have my non-inline assembly.
