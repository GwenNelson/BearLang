# BearLang
BearLang is a lispy-like language (ok, it's a lisp essentially) written by me to match my own mental model of coding better.
At the same time, it's a heavy work in progress so expect bugs.

## Why bears?
BOW!!!!

## Building + running it

Build and install the dependencies under vendor/ first (running build.sh as root will do this for you automatically).

Next do this:

```
 mkdir build
 cd build
 cmake ..
 make
 ./bli ../examples/euler/problem01.bl
```

The REPL is named bli (for BearLang Interpreter), a compiler is currently under development.

Documentation on the language can be accessed by running the documentation browser webapp in docs/docs.bl


## Basic features

* Lispy primitives (list operations, lambdas etc)
* Nested functions
* Tail call optimisation
* Fast flex-based parsing
* Boehms GC

