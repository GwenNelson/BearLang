import sys
import random

PRNG = random.Random()
builtins = ['+','-','*','/']

def rand_list(recurse=True):
    list_len = PRNG.randint(10,100)
    retval = []
    if recurse:
       other_list = rand_list(False)
    else:
       other_list = [1,2,3,4,5,6,7,8,9,10]
    for x in xrange(list_len):
        retval.append(PRNG.choice([PRNG.randint(-1000,1000),PRNG.sample(other_list,PRNG.randint(1,10))]+builtins))
    return retval

def print_list(L):
    if type(L) is list:
       sys.stdout.write('(')
       for x in L:
           print_list(x)
       sys.stdout.write(')')
    else:
       sys.stdout.write(str(L))
       sys.stdout.write(' ')

print_list(rand_list()) 
        
