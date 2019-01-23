# -*- coding: utf-8 -*-
import os
import sys
import xml.sax.saxutils as XSS
import BaseTest as GT
import logging

# FIXME: module alias for backward compatibility
sys.modules['GaudiTest'] = GT


def basic_report(results):
    '''
    Report function taking the dictionary from BasicTest.run() and display
    stdout and stderr from it.
    '''
    print '=== stdout ==='
    print results.get('stdout', '')
    print '=== stderr ==='
    print results.get('stderr', '')
    print '=== result ==='
    print results.get('Status')
    if results.get('Status') != 'passed' and 'Causes' in results:
        print '   ', 'unexpected ' + ', '.join(results['Causes'])


def quiet_report(results):
    '''
    Do not report anything from the result of the test.
    '''
    pass


def ctest_report(results):
    '''
    Report function taking the dictionary from BasicTest.run() and report data
    from it in a CTest-friendly way.
    '''
    # It's weird, I know, but it tells CTest not to cut the output.
    print 'CTEST_FULL_OUTPUT'
    print results.get('stdout', '')
    handler = {'Environment': lambda v: '\n'.join('{0}={1}'.format(*item)
                                                  for item in sorted(v.iteritems())),
               'Causes': lambda v: 'unexpected ' + ', '.join(v)}

    def id_handler(v): return str(v)
    ignore = set(['Status', 'Name', 'stdout', 'Exit Code'])
    template = (
        '<DartMeasurement type="text/string" name="{0}">{1}</DartMeasurement>')

    for key in results:
        if key in ignore:
            continue
        hndlr = handler.get(key, id_handler)
        data = XSS.escape(GT.sanitize_for_xml(hndlr(results[key])))
        sys.stdout.write(template.format(key, data))


def pprint_report(results):
    '''
    Report function taking the dictionary from BasicTest.run() and print it with
    the pprint module.
    '''
    from pprint import pprint
    pprint(results)


def main():
    '''
    Main function of the script.
    '''
    from optparse import OptionParser, OptionGroup
    parser = OptionParser()

    parser.add_option('--report', action='store',
                      choices=[n.replace('_report', '')
                               for n in globals() if n.endswith('_report')],
                      help='choose a report method [default %default]')
    parser.add_option('--common-tmpdir', action='store',
                      help='directory to be used as common temporary directory')
    parser.add_option('-C', '--workdir', action='store',
                      help='directory to change to before starting the test')

    parser.add_option('--skip-return-code', type='int',
                      help='return code to use to flag a test as skipped '
                           '[default %default]')

    verbosity_opts = OptionGroup(parser, 'Verbosity Level',
                                 'set the verbosity level of messages')
    verbosity_opts.add_option('--silent',
                              action='store_const', dest='log_level',
                              const=logging.CRITICAL,
                              help='only critical error messages')
    verbosity_opts.add_option('--quiet',
                              action='store_const', dest='log_level',
                              const=logging.ERROR,
                              help='error messages')
    verbosity_opts.add_option('--warning',
                              action='store_const', dest='log_level',
                              const=logging.WARNING,
                              help='warning and error messages')
    verbosity_opts.add_option('--verbose',
                              action='store_const', dest='log_level',
                              const=logging.INFO,
                              help='progress information messages')
    verbosity_opts.add_option('--debug',
                              action='store_const', dest='log_level',
                              const=logging.DEBUG,
                              help='debugging messages')
    parser.add_option_group(verbosity_opts)

    parser.set_defaults(log_level=logging.WARNING,
                        report='basic',
                        workdir=os.curdir,
                        skip_return_code=0)

    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error('only one test allowed')
    filename = args[0]

    logging.basicConfig(level=opts.log_level)

    if opts.common_tmpdir:
        if not os.path.isdir(opts.common_tmpdir):
            os.makedirs(opts.common_tmpdir)
        GT.BaseTest._common_tmpdir = opts.common_tmpdir

    os.chdir(opts.workdir)

    # FIXME: whithout this, we get some spurious '\x1b[?1034' in the std out on SLC6
    if "slc6" in (os.environ.get('BINARY_TAG', '') or
                  os.environ.get('CMTCONFIG', '')):
        os.environ['TERM'] = 'dumb'

    # If running sanitizer builds, set LD_PRELOAD in environment
    sanitizer = os.environ.get('PRELOAD_SANITIZER_LIB', '')
    ld_preload = os.environ.get('LD_PRELOAD', '')
    if sanitizer and sanitizer not in ld_preload:
        if ld_preload:
            os.environ['LD_PRELOAD'] = sanitizer+" "+ld_preload
        else:
            os.environ['LD_PRELOAD'] = sanitizer

    # Testing the file beginning with "Test" or if it is a qmt file and doing the test
    logging.debug('processing %s', filename)
    if filename.endswith('_test.py'):
        indexFilePart = filename.rfind("/")
        fileToImport = filename[indexFilePart + 1:]
        sys.path.append(GT.RationalizePath(filename)[:-len(fileToImport) - 1])
        imp = __import__(fileToImport[:-3])
        fileToExec = imp.Test()
        results = fileToExec.run()
    elif filename.endswith(".qmt"):
        # Check which class should be used to instantiate QMTests
        # by default it is QMTTest but this can be overwritten via the environment
        test_module = os.environ.get('GAUDI_QMTEST_MODULE',
                                     'GaudiTesting.QMTTest')
        test_class = os.environ.get('GAUDI_QMTEST_CLASS', 'QMTTest')
        exec 'from {} import {} as test_class'.format(test_module, test_class)
        fileToTest = test_class(filename)
        results = fileToTest.run()

    report = globals()[opts.report + '_report']
    report(results)

    if results.get('Status') == 'failed':
        logging.debug('test failed: unexpected %s',
                      ', '.join(results['Causes']))
        return int(results.get('Exit Code', '1'))
    elif results.get('Status') == 'skipped':
        return opts.skip_return_code
    return 0


if __name__ == '__main__':
    sys.exit(main())
