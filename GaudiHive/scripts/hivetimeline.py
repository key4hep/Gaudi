#!/usr/bin/env python
"""Plot timeline from TimelineSvc"""

__author__ = "Frank Winklmeier"

import sys
import os
import re
import argparse
import operator
from collections import defaultdict

algcolors = []
evtcolors = []

# Attributes will be added in read()
class Data:
   pass

def read(f,regex='.*'):
   data = []
   regex = re.compile(regex)
   for l in open(f,'r'):
      if l.startswith('#'):  # e.g. #start end algorithm thread slot event
         names = l.lstrip('#').split()   
         continue
      d = Data()
      for i,f in enumerate(l.split()):
         setattr(d, names[i], int(f) if f.isdigit() else f)
      data.append(d)

   return data

def findEvents(data):
   """Find event start/stop times"""

   t = defaultdict(lambda : [sys.maxint, 0])  # start,stop
   for d in data:
      if d.start<t[d.event][0]: t[d.event][0] = d.start
      if d.end>t[d.event][1]: t[d.event][1] = d.end
      
   return t


def setPalette(nevts):
   global algcolors, evtcolors
   
   from ROOT import TColor
   algcolors = range(2,10)+[20,28,29,30,33,38,40]+range(41,50)
   evtcolors = [TColor.GetColor(g,g,g) for g in range(20,255,(255-20)/nevts)]


def plot(data, showThreads=True, batch=False):
   import ROOT

   tmin = min(f.start for f in data)
   tmax = max(f.end for f in data)
   slots = 1+max(f.slot for f in data)
   threads = sorted(list(set(f.thread for f in data)))
   threadid = dict((k,v) for v,k in enumerate(threads))  # map thread : id   
   ymax = len(threads) if showThreads else slots
   
   c = ROOT.TCanvas('timeline','Timeline',1200,500)
   c.SetLeftMargin(0.05)
   c.SetRightMargin(0.2)
   c.SetTopMargin(0.1)
   c.coord = ROOT.TH2I('coord',';Time',100,0,tmax-tmin,ymax,0,ymax)
   c.coord.GetYaxis().SetTitle(('Thread' if showThreads else 'Slot'))
   c.coord.GetYaxis().SetTitleOffset(0.6)
   c.coord.SetStats(False)
   c.coord.GetYaxis().SetNdivisions(ymax)
   
   c.Draw()
   c.coord.Draw()

   c.lines = []
   colors = {}
   setPalette(ymax)
   mycolors = algcolors 
   for d in data:
      y = (threadid[d.thread] if showThreads else d.slot) + 0.4
      alg = d.algorithm
      if not alg in colors and len(mycolors)>0:         
         colors[alg] = mycolors.pop(0)
         if len(mycolors)==0:
            print "Too many algorithm to show"

      if alg in colors:
         # Alg
         l = ROOT.TLine(d.start-tmin,y,d.end-tmin,y)         
         l.SetLineColor(colors[alg])
         l.SetLineWidth(30)

         # Event
         l2 = ROOT.TLine(d.start-tmin,y+0.3,d.end-tmin,y+0.3)
         l2.SetLineColor(evtcolors[d.event % ymax])
         l2.SetLineWidth(10)
         c.lines += [l,l2]

         l2.Draw()
         l.Draw()

   # Gloabl event timeline
   tevt = findEvents(data)
   for k,v in tevt.iteritems():
      m = k % ymax # event modulo
      y = ymax+0.06*m+0.05
      l = ROOT.TLine(v[0]-tmin,y,v[1]-tmin,y)
      l.SetLineColor(evtcolors[m])
      l.SetLineWidth(5)
      c.lines += [l]
      l.Draw()
      
   # Alg legend
   c.leg = ROOT.TLegend(0.8,0.4,0.98,0.9)
   for alg,cl in sorted(colors.iteritems(),key=operator.itemgetter(1)):
      e = c.leg.AddEntry('',alg,'F')
      e.SetLineColor(cl)
      e.SetFillColor(cl)
      e.SetFillStyle(1001)

   # Event legend
   for cl in range(ymax):
      l = ROOT.TLine()
      c.lines.append(l)
      l.SetLineWidth(10)
      l.SetLineColor(evtcolors[cl])
      l.DrawLineNDC(0.807+0.02*cl,0.37,0.807+0.02*(cl+1),0.37)

   c.t1 = ROOT.TText(0.807,0.314,'Events')
   c.t1.SetNDC()
   c.t1.SetTextFont(42)
   c.t1.SetTextSize(0.04)
   c.t1.Draw()

   ROOT.TLine()
   c.leg.Draw()
   c.Update()
   if not batch: raw_input()
   return c


def main():
   parser = argparse.ArgumentParser(description=__doc__)

   parser.add_argument('timeline', nargs=1,
                       help='timeline file')

   parser.add_argument('-s', '--select', default='.*',
                       help='Regular expression to filter algorithms')

   parser.add_argument('-b', '--batch', action='store_true', default=False,
                       help='Do not wait for user input')

   parser.add_argument('--slots', action='store_true', default=False,
                       help='Show slots instead of threads (leads to overlaps!)')

   parser.add_argument('-p', '--print', dest='outfile' , nargs='?', 
                       const='timeline.png',
                       help='Save to FILE [%(const)s]')

   args = parser.parse_args()

   data = read(args.timeline[0], args.select)
   c = plot(data, not args.slots, args.batch)
   if args.outfile:
      c.SaveAs(args.outfile)
   
   return 0

if __name__ == '__main__':
   sys.exit(main())
