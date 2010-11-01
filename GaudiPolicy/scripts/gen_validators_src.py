#!/usr/bin/env python

import os, sys, anydbm

if os.path.exists(sys.argv[1]):
    db = anydbm.open(sys.argv[1])
else:
    print "%s:WARNING:" % sys.argv[0], "cannot find database file", sys.argv[1]
    db = {}
keys = db.keys()

cpp = ['// needs to be first to avoid warnings',
       '#include <boost/python.hpp>',
       '#include <GaudiKernel/Property.h>']

# collect extra headers
hdrs = set()
for k in [ k for k in keys if k.endswith("->headers") ]:
    hdrs.update(db[k].split(";"))
for h in sorted(hdrs):
    cpp.append("#include <%s>" % h)

module = os.path.splitext(os.path.basename(sys.argv[2]))[0]
cpp += ['#define PYCONF_VALIDATOR_MODULE',
        '#include <GaudiKernel/PyConfValidators.h>',
        'BOOST_PYTHON_MODULE(%s)' % module,
        '{',
        'using namespace boost::python;']

# collect all the validators
all = set()
for k in [ k for k in db.keys() if "->" not in k ]:
    all.update([ x[3] for x in eval(db[k]) ])
for p in sorted(all):
    cpp.append('def("%s",' % p)
    pos = p.find("<")
    if pos >= 0:
        t = p[pos:]
    elif p == "GaudiHandleProperty": # equivalent to a string
        t = "<std::string,NullVerifier<std::string> >"
    elif p == "GaudiHandleArrayProperty": # equivalent to vector<string>
        t = "<std::vector<std::string,std::allocator<std::string> >,NullVerifier<std::vector<std::string,std::allocator<std::string> > > >"
    else: # If unknown, use the catch-all string type
        t = "<std::string,NullVerifier<std::string> >"
    cpp.append('    check%s);' % t)

cpp.append("}")

open(sys.argv[2], "w").write("\n".join(cpp) + "\n")
