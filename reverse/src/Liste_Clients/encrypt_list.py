#!/usr/bin/env python
# -*- coding: utf-8 -*-
def l(v_,v__):
    k_=""
    for x in range(0,(len(v__)/len(v_))+1):
        k_+=u4(v_,v__[x*len(v_):(x+1)*len(v_)])
    k__ = k_[0]
    for x in range(1,len(k_)):
        k__+=u4(k_[x],k_[x-1])
    o=k__
    return o[int(False):len(k__)]
def u4(___,____):
    return "".join(chr(ord(M) ^ ord(m)) for M,m in zip(___,____))
def a():
    z = open("liste.txt","r")
    d = z.read()
    z.close()
    k = open("key.png","r")
    r = k.read()
    k.close()
    r = r[:4]
    s = l(r,d)
    z = open("o.enc","w")
    z.write(s)
    z.close()
a()
