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

BUILTIN_X(fn,"fn", "(fn (args ...) expr ...)\n"
		   "\t Lambda expression, defines and returns function")

BUILTIN_X(fun,"fun", "(fun name (args ...) expr ...)\n"
		     "\t Defines and returns new function and binds it to the specified name")

BUILTIN_X(foreach,"foreach","(foreach sym list expr)\n"
	                    "\t A classic for-each loop, binds sym and then evaluates expr for each item in list")

BUILTIN_X(filtered,"filtered","(filtered L items)\n"
	                      "\t Returns the list L without any of the items in the list \"items\"")

BUILTIN_X(throw,"throw","(throw errtype errval)\n"
	                "\t Throws a custom error, errtype should be a symbol specifying the type, errval is an optional param with an arbitrary value")

BUILTIN_X(oper,"oper", "(oper name (args ...) expr ...)\n"
                       "\t Defines and returns a new oper and binds it to the specified name")

BUILTIN_X(map,"map", "(map func L)\n"
                     "\t Runs func on every item of list L and returns a new list with the results")

BUILTIN_X(eq,"eq", "(eq a b)\n"
                   "\t Returns True if a and b are equal, False otherwise")

BUILTIN_X(lt,"lt", "(lt a b)\n"
                   "\t Check is a is lower than b")

BUILTIN_X(gt,"gt", "(gt a b)\n"
                   "\t Check is a is greater than b")

BUILTIN_X(print,"print", "(print ...)\n"
                         "\t Serialises all arguments and then prints them to stdout")

BUILTIN_X(and,"and", "(and a b)\n"
                     "\t Boolean and operation")

BUILTIN_X(not,"not", "(not x)\n"
                     "\t Boolean not operation")

BUILTIN_X(or,"or", "(or a b)\n"
                   "\t Boolean or operation")

BUILTIN_X(xor,"xor", "(xor a b)\n"
                     "\t Boolean xor operation")

BUILTIN_X(first,"first", "(first L)\n"
                         "\t Returns first item in list L")

BUILTIN_X(second,"second", "(second L)\n"
                           "\t Returns second item in list L")

BUILTIN_X(third,"third", "(third L)\n"
                         "\t Returns the third item in the list L")

BUILTIN_X(rest,"rest", "(rest L)\n"
                       "\t Returns the rest of the list L without the first item")

BUILTIN_X(append,"append", "(append L x)\n"
                           "\t Returns the list L with x appended to the end of it")

BUILTIN_X(prepend,"prepend", "(prepend L x)\n"
                             "\t Returns the list L with x appended to the start of it")

BUILTIN_X(reverse,"reverse", "(reverse L)\n"
                             "\t Returns the list L backwards")

BUILTIN_X(include,"include", "(include filename)\n"
                             "\t Loads the specified filename, parses it and includes it literally")

BUILTIN_X(import,"import", "(import modulename)\n"
                           "\t Imports the specified module")

BUILTIN_X(isset,"isset", "(isset x)\n"
                         "\t Checks if the item x is set in the current environment, returns a boolean")

BUILTIN_X(serexp,"serexp", "(serexp E)\n"
                           "\t Serialises the expression E and returns a string")

BUILTIN_X(inc,"inc", "(inc x)\n"
                     "\t Increments specified variable in-place")

BUILTIN_X(dec,"dec", "(dec x)\n"
                     "\t Decrements specified variable in-place")

BUILTIN_X(parse,"parse", "(parse s)\n"
                         "\t Tries to parse the string s into an s-expression")

BUILTIN_X(eval,"eval", "(eval E)\n"
                       "\t Evaluates the expression E and returns the result")

BUILTIN_X(try,"try", "(try e (catch <ERR> handler))\n"
                     "\t Evaluates e, if an error occurs, evaluates handler")

BUILTIN_X(quote,"quote", "(quote E)\n"
                         "\t Returns E un-evaluated")

BUILTIN_X(using,"using", "(using module::symbol)\n"
                         "\t Brings a symbol from a module into the current context, use * to bring in all symbols")

BUILTIN_X(doc,"doc", "(doc item)\n"
                     "\t Returns the docstring for the specified item")

BUILTIN_X(dir,"dir", "(dir ctx)\n"
                     "\t Returns the contents of a context as a list of symbols")

BUILTIN_X(mksym,"mksym", "(mksym s)\n"
                         "\t Create a new symbol from string s")

BUILTIN_X(ctxget,"ctxget", "(ctxget ctx sym)\n"
                           "\t Gets the value of a symbol from a context")

BUILTIN_X(type,"type", "(type x)\n"
                       "\t Returns the type of x")
