#!/usr/bin/env python

import os
from subprocess import call

tmpfile = os.path.join(os.getcwd(), 'tmp.opts')

with open(tmpfile, 'w') as f:
    f.write('MyAlg.MyOpt = 1;\n')

os.environ['GAUDI_TEMP_OPTS_FILE'] = tmpfile

assert call(['gaudirun.py', '--dry-run', '--output', 'out.py']) == 0

assert not os.path.exists(tmpfile)

assert os.path.exists('out.py')

expected = {'MyAlg': {'MyOpt': 1}}
observed = eval(open('out.py').read())

assert expected == observed
