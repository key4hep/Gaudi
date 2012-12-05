#!/usr/bin/env python

import os
import sys
import re
from optparse import OptionParser
def main():
    parser = OptionParser(usage="ERROR: Usage %prog <project> <version> <outputfile>")
    parser.add_option("-q", "--quiet", action="store_true",
                      help="Do not print messages.")
    opts, args = parser.parse_args()
    if len(args) != 3:
        parser.error("wrong number of arguments")

    project, version, outputfile = args
    if not opts.quiet:
        print "Creating %s for %s %s" % (outputfile, project, version)

    if version.startswith('HEAD'):
        majver, minver, patver = 999, 999, 0 # special handling
    else:
        m = re.match("(v|([A-Za-z]+\-))(?P<maj_ver>[0-9]+)(r|\-)(?P<min_ver>[0-9]+)(?:(p|\-)(?P<pat_ver>[0-9]+))?", version)
        majver = int(m.groupdict()['maj_ver'])
        minver = int(m.groupdict()['min_ver'])
        patver = int(m.groupdict()['pat_ver'] or 0)

    outdir = os.path.dirname(outputfile)
    if not os.path.exists(outdir):
        if not opts.quiet:
            print "Creating directory", outdir
        os.makedirs(outdir)

    # Prepare data to be written
    outputdata = """#ifndef %(proj)s_VERSION
/* Automatically generated file: do not modify! */
#ifndef CALC_GAUDI_VERSION
#define CALC_GAUDI_VERSION(maj,min) (((maj) << 16) + (min))
#endif
#define %(proj)s_MAJOR_VERSION %(maj)d
#define %(proj)s_MINOR_VERSION %(min)d
#define %(proj)s_PATCH_VERSION %(pat)d
#define %(proj)s_VERSION CALC_GAUDI_VERSION(%(proj)s_MAJOR_VERSION,%(proj)s_MINOR_VERSION)
#endif
""" % { 'proj': project.upper(), 'min': minver, 'maj': majver, 'pat': patver }

    # Get the current content of the destination file (if any)
    try:
        f = open(outputfile, "r")
        olddata = f.read()
        f.close()
    except IOError:
        olddata = None

    # Overwrite the file only if there are changes
    if outputdata != olddata:
        open(outputfile, "w").write(outputdata)

if __name__ == "__main__":
    main()
