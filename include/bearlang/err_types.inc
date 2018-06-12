ERR_X(ANY,              0)    // matches any error
ERR_X(UNKNOWN,          1)    // generic / unknown error
ERR_X(PARSE,            2)    // failed to parse an s-expression
ERR_X(INSUFFICIENT_ARGS,3)    // not enough arguments provided to a function/oper
ERR_X(TOOMANY_ARGS,     4)    // too many arguments provided to a function/oper
ERR_X(INVALID_ARGTYPE,  5)    // an invalid data type was given to a function/oper
ERR_X(SYMBOL_NOTFOUND,  6)    // a symbol could not be found
ERR_X(DIVIDE_BY_ZERO,   7)    // attempted to divide by zero
ERR_X(CUSTOM,           8)    // custom error, used by C modules
ERR_X(IO,               9)    // generic I/O error
ERR_X(MODULE_NOTFOUND,  10)   // a module could not be found
ERR_X(BL_CUSTOM,        11)   // custom error, used by BearLang code
