#!/usr/bin/python2
import string
import itertools

key = "VivaLaCraptography"
data = "***INSERT SECRET HERE***"


def make_magic(number):
    lc,uc = string.ascii_lowercase, string.ascii_uppercase
    trans = string.maketrans(lc + uc, lc[number:] + lc[:number] + uc[number:] + uc[:number])
    return lambda s: string.translate(s, trans)

def magicify(foo, bar):
    return [ chr(ord(a) ^ ord(b)) for (a,b) in zip(foo, itertools.cycle(bar)) ]

# Scramble the key for extra 1337 entropy protection
key = make_magic(6)(key)
# mega-entropy-scramble
key = key[::-1]
# madness entropy added here
key = make_magic(16)(key)

encrypted = magicify(data,key)
with open("out.bin", 'wb') as output:
    output.write(bytearray(i for i in [ord(x) for x in encrypted]))

print("Your data is now $100% securely encrypted in out.bin.")
