# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["$GAUDIEXAMPLESROOT/options/TimingHistograms.py"]
        self.reference="refs/TimingHistograms.ref"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        # Default validation.
        self.validateWithReference()

        # Check the content of the ROOT file.
        import os
        from subprocess import Popen, PIPE

        testscript = os.path.join(os.environ['GAUDIEXAMPLESROOT'], 'tests', 'scripts', 'test_timing_histo_file.py')

        test = Popen(['python', testscript], stdout=PIPE, stderr=PIPE)
        out, err = test.communicate()

        result['root_file_check.returncode'] = str(test.returncode)
        if test.returncode:
            causes.append('root file content')

        if out:
            result['root_file_check.stdout'] = result.Quote(out)

        if err:
            causes.append('standard error')
            result['root_file_check.stderr'] = result.Quote(err)
