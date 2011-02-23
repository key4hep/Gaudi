import os, sys, string, getopt
import difflib

code   = 0
result = ''
ignoredlines = ["//GP: ",
                "Time User",
                "Welcome to",
                "TIME:",
                "running on",
                "ToolSvc.Sequenc...   INFO",
                "DataListenerSvc      INFO XML written to file:",
                "[INFO]","[WARNING]"]

def status() : return code
def output() : return result
def cdiff(file,ref) :
  global code, result
  result = ''
  cmd = 'diff -a '
  for il in ignoredlines:
    cmd += '--ignore-matching-lines="' + il + '" '
  cmd += '%s %s'%(file,ref)
  diff = os.popen(cmd)
  lines = diff.readlines()
  for l in lines :
    if   l[0] == '>' : result += 'REF>'+l[1:]
    elif l[0] == '<' : result += 'OUT>'+l[1:]
    else             : result += l      
  code   = diff.close()
  
def linejunkfilter(line):
  line = line[:-1]
  for il in ignoredlines:
    if line.find(il) != -1 :
      return False
  return True

def pdiff(out,ref):
  global code,result
  result = ''
  outlines = open(out).readlines()
  filtoutlines = filter(linejunkfilter,outlines)
  reflines = open(ref).readlines()
  filtreflines = filter(linejunkfilter,reflines)
  diff = difflib.ndiff(filtoutlines,filtreflines)
  for i in diff:
    if i[0] == '-': result +=  "OUT>" + i[1:]
    if i[0] == '+': result +=  "REF>" + i[1:]
    if i[0] == '?': result +=  "???>" + i[1:]
  if result == '':
    code = False
  else:
    code = True
