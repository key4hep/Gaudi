def main():
    import os, sys, re
    if len(sys.argv) != 4:
        print "ERROR: Usage %s <project> <version> <outputfile>"%sys.argv[0]
        exit(1)
    
    project, version, outputfile = sys.argv[1:]
    print "Creating %s for %s %s"%(outputfile, project, version) 
    
    m = re.match("(v|([A-Za-z]+\-))(?P<maj_ver>[0-9]+)(r|\-)(?P<min_ver>[0-9]+)(?:(p|\-)(?P<pat_ver>[0-9]+))?",version)
    majver = int(m.groupdict()['maj_ver'])
    minver = int(m.groupdict()['min_ver'])
    
    outdir = os.path.dirname(outputfile)
    if not os.path.exists(outdir):
        print "Creating directory", outdir
        os.makedirs(outdir)
    
    open(outputfile,"w").write(
    """#ifndef %(proj)s_VERSION
/* Automatically generated file: do not modify! */
#ifndef CALC_GAUDI_VERSION
#define CALC_GAUDI_VERSION(maj,min) (((maj) << 16) + (min))
#endif
#define %(proj)s_MAJOR_VERSION %(maj)d
#define %(proj)s_MINOR_VERSION %(min)d
#define %(proj)s_VERSION CALC_GAUDI_VERSION(%(proj)s_MAJOR_VERSION,%(proj)s_MINOR_VERSION)
#endif
"""%{ 'proj': project.upper(), 'min': minver, 'maj': majver })

if __name__ == "__main__":
    main()
