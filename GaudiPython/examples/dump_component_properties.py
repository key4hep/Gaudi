import os, gaudimodule
#---Get the list of DLLs available--------------------------------
modules = [ e[:-3] for e in os.environ if e[-3:].upper() == 'SHR']
for m in modules :   #---For each one dump the cotents of Component and Properties
  try:  properties = gaudimodule.getComponentProperties(m)
  except: continue
  for c in properties :
    print 80*'-'
    print 'Properties of %s %s in library %s' % (properties[c][0], c, m)
    print 80*'-'
    for p in properties[c][1] :
      value = properties[c][1][p]
      if type(value) is str :
        print "%-30s = '%s'" % ( c+'.'+p , value)
      else :
        print "%-30s = %s" % ( c+'.'+p , str(value))
      

