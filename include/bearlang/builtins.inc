BUILTIN_X(listbuiltins,"listbuiltins","(listbuiltins)\n"
	                              "\t Returns a list of all builtins in the current runtime\n")

BUILTIN_X(aget,"aget","(aget <SYM> <LIST>)\n"
                      "\t Gets the value of a symbol from an assoc list")

BUILTIN_X(split,"split","(split <STR> <STR>)\n"
	                "\t Splits the first string by the delimiter specified in the second string\n")

BUILTIN_X(startswith,"startswith","(startswith <STR> <STR>)\n"
	                          "\t Checks if the first string starts with the second string\n")

BUILTIN_X(pmatch,"pmatch","(pmatch <STR> <ALIST>)\n"
	                  "\t Matches the string prefix against an alist, that is, returns the cdr of the first pair in the alist where the string starts with the car\n"
		  	  "\t Returns None if no match found")

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

BUILTIN_X(fn,"fn", "(fn (args ...) expr ...)"
		   "\t Lambda expression, defines and returns function")

BUILTIN_X(fun,"fun", "(fun name (args ...) expr ...)"
		     "\t Defines and returns new function and binds it to the specified name")

BUILTIN_X(foreach,"foreach","(foreach sym list expr)"
	                    "\t A classic for-each loop, binds sym and then evaluates expr for each item in list")

BUILTIN_X(filtered,"filtered","(filtered L items)"
	                      "\t Returns the list L without any of the items in the list \"items\"")

BUILTIN_X(throw,"throw","(throw errtype errval)"
	                "\t Throws a custom error, errtype should be a symbol specifying the type, errval is an optional param with an arbitrary value")

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
