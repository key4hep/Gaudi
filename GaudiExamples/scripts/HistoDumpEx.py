#!/usr/bin/env python

import ROOT

from GaudiPython.HistoUtils import histoDump

import random

r = random.Random()

h1 = ROOT.TH1D    ("h1", 'title', 200 , -4  ,  4  )
p1 = ROOT.TProfile("p1", 'title', 200 , -4  ,  4  , -10 , 10 , 's' )
p2 = ROOT.TProfile("p2", 'title', 200 , -4  ,  4  , -10 , 10       )

for i in xrange(0,1000000) :
    v  = r.gauss  ( 0 , 2 )
    v2 = r.gauss  ( 0 , 2 )
    if v < 0 : h1.Fill ( v , -1 )
    else     : h1.Fill ( v ,  1 )
    p1.Fill ( v , -0.1*v2 , 1 )
    p2.Fill ( v , -0.1*v2 , 1 )

print h1 . dump ( 50 , 50 , True )
print h1 . dump ( 60 , 30 )

print histoDump ( h1 , 10 , 10 )
print histoDump ( h1 , 10 , 10 , True )

print histoDump ( p1 , 10 , 10 , True )
print histoDump ( p2 , 10 , 10 , True )
