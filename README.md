# BearLang
BearLang is a lispy-like language (ok, it's a lisp essentially) written by me to match my own mental model of coding better.
At the same time, it's a heavy work in progress so expect bugs.

## Why bears?
BOW!!!!

## Building + running it

Ensure you have cmake and the LLVM developer packages installed, then do this:

```
 mkdir build
 cd build
 cmake ..
 make
 ./bli ../examples/euler/problem01.bl
 ````

Running bli on its own will give a REPL, and at some point blc will spit out a compiled binary.

Read the code for more details, proper documentation is coming.

## Basic features

* Lispy primitives (list operations, lambdas etc)
* Nested functions
* Tail call optimisation
* Fast flex-based parsing
* Boehms GC

Read DESIGN for a better overview, and TODO to see what is outstanding.
