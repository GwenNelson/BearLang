
BUILTIN_X(add,"+","(+ <NUM> <NUM> ...)\n"
                  "\t Adds up the numbers provided and returns the sum\n"
                  "(+ <ANY> <ANY> ...)\n"
                  "\t Serialises the provided values into strings and concatenates them together\n")

BUILTIN_X(sub,"-","(- a b)\n"
                  "\t Subtracts b from a")

BUILTIN_X(mult,"*","(* a b)\n"
                   "\t Multiplies a by b")

BUILTIN_X(div,"/","(/ a b)\n"
                  "\t Divides a by b")

BUILTIN_X(mod,"%","(% a b)\n"
                  "\t Returns a modulo b")

BUILTIN_X(set,"=","(= <SYM> <ANY>)\n"
                  "\t Assigns a value to a symbol")
