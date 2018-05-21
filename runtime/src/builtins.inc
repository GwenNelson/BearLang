
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
BUILTIN_X(fn,"fn", "")
BUILTIN_X(fun,"fun", "")
BUILTIN_X(oper,"oper", "")
BUILTIN_X(map,"map", "")
BUILTIN_X(eq,"eq", "")
BUILTIN_X(lt,"lt", "")
BUILTIN_X(gt,"gt", "")
BUILTIN_X(print,"print", "")
BUILTIN_X(and,"and", "")
BUILTIN_X(not,"not", "")
BUILTIN_X(or,"or", "")
BUILTIN_X(xor,"xor", "")
BUILTIN_X(first,"first", "")
BUILTIN_X(second,"second", "")
BUILTIN_X(third,"third", "")
BUILTIN_X(rest,"rest", "")
BUILTIN_X(append,"append", "")
BUILTIN_X(prepend,"prepend", "")
BUILTIN_X(reverse,"reverse", "")

BUILTIN_X(include,"include", "")
BUILTIN_X(import,"import", "")
BUILTIN_X(isset,"isset", "")
BUILTIN_X(serexp,"serexp", "")
BUILTIN_X(inc,"inc", "")
BUILTIN_X(dec,"dec", "")
BUILTIN_X(parse,"parse", "")
BUILTIN_X(eval,"eval", "")
BUILTIN_X(try,"try", "")
BUILTIN_X(quote,"quote", "")
BUILTIN_X(using,"using", "")
BUILTIN_X(doc,"doc", "")
BUILTIN_X(dir,"dir", "")
BUILTIN_X(mksym,"mksym", "")
BUILTIN_X(ctxget,"ctxget", "")
BUILTIN_X(type,"type", "")
