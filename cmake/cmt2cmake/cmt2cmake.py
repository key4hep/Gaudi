#!/usr/bin/env python
"""
Script to convert CMT projects/packages to CMake Gaudi-based configuration.
"""
import os
import sys
import re
import logging
import shelve

def makeParser():
    from pyparsing import ( Word, QuotedString, Keyword, Literal, SkipTo, StringEnd,
                            ZeroOrMore, Optional, Combine,
                            alphas, alphanums, printables )
    dblQuotedString = QuotedString(quoteChar='"', escChar='\\', unquoteResults=False)
    sglQuotedString = QuotedString(quoteChar="'", escChar='\\', unquoteResults=False)
    value = dblQuotedString | sglQuotedString | Word(printables)

    tag_name = Word(alphas + "_", alphanums + "_-")
    tag_expression = Combine(tag_name + ZeroOrMore('&' + tag_name))
    values = value + ZeroOrMore(tag_expression + value)

    identifier = Word(alphas + "_", alphanums + "_")
    variable = Combine(identifier + '=' + value)

    constituent_option = (Keyword('-no_share')
                          | Keyword('-no_static')
                          | Keyword('-prototypes')
                          | Keyword('-no_prototypes')
                          | Keyword('-check')
                          | Keyword('-target_tag')
                          | Combine('-group=' + value)
                          | Combine('-suffix=' + value)
                          | Combine('-import=' + value)
                          | variable
                          | Keyword('-OS9')
                          | Keyword('-windows'))
    source = (Word(alphanums + "_*./$()")
              | Combine('-s=' + value)
              | Combine('-k=' + value)
              | Combine('-x=' + value))

    # statements
    comment = (Literal("#") + SkipTo(StringEnd())).suppress()

    package = Keyword('package') + Word(printables)
    version = Keyword("version") + Word(printables)
    use = Keyword("use") + identifier + Word(printables) + Optional(identifier) + Optional(Keyword("-no_auto_imports"))

    constituent = ((Keyword('library') | Keyword('application') | Keyword('document'))
                   + identifier + ZeroOrMore(constituent_option | source))
    macro = (Keyword('macro') | Keyword('macro_append')) + identifier + values

    apply_pattern = Keyword("apply_pattern") + identifier + ZeroOrMore(variable)


    statement = (package | version | use | constituent | macro | apply_pattern)

    return Optional(statement) + Optional(comment) + StringEnd()

CMTParser = makeParser()

# mappings
ignored_packages = set(["GaudiSys", "GaudiRelease", "GaudiPolicy"])
ignore_dep_on_subdirs = set(ignored_packages)
ignore_dep_on_subdirs.update(['Det/SQLDDDB', 'FieldMap'])

needing_python = ('LoKiCore', 'XMLSummaryKernel', 'CondDBUI')

# packages that must have the pedantic option disabled
no_pedantic = set(['LHCbMath'])

# record of known subdirs with their libraries
# {'subdir': {'libraries': [...]}}
_shelve_file = os.environ.get('CMT2CMAKECACHE',
                              os.path.join(os.path.dirname(__file__), 'known_subdirs.cache'))
known_subdirs = shelve.open(_shelve_file)

def extName(n):
    mapping = {'Reflex': 'ROOT',
               'Python': 'PythonLibs'}
    return mapping.get(n, n)

def isPackage(path):
    return os.path.isfile(os.path.join(path, "cmt", "requirements"))

def isProject(path):
    return os.path.isfile(os.path.join(path, "cmt", "project.cmt"))

def projectCase(name):
    if name.upper() == "DAVINCI":
        return "DaVinci"
    return name.capitalize()

def callStringWithIndent(cmd, arglines):
    '''
    Produce a string for a call of a command with indented arguments.

    >>> print callStringWithIndent('example_command', ['arg1', 'arg2', 'arg3'])
    example_command(arg1
                    arg2
                    arg3)
    >>> print callStringWithIndent('example_command', ['', 'arg2', 'arg3'])
    example_command(arg2
                    arg3)
    '''
    indent = '\n' + ' ' * (len(cmd) + 1)
    return cmd + '(' + indent.join(filter(None, arglines)) + ')'

class Package(object):
    def __init__(self, path, project=None):
        self.path = os.path.realpath(path)
        if not isPackage(self.path):
            raise ValueError("%s is not a package" % self.path)

        self.name = os.path.basename(self.path)
        self.requirements = os.path.join(self.path, "cmt", "requirements")
        self.project = project

        # prepare attributes filled during parsing of requirements
        self.uses = {}
        self.version = None
        self.libraries = []
        self.applications = []
        self.macros = {}

        self.singleton_patterns = set(["QMTest", "install_python_modules", "install_scripts",
                                       "install_more_includes", "god_headers", "god_dictionary"])
        self.install_more_includes = {}
        self.install_python_modules = self.install_scripts = self.QMTest = False
        self.god_headers = {}
        self.god_dictionary = {}

        self.multi_patterns = set(["reflex_dictionary", 'component_library', 'linker_library'])
        self.reflex_dictionary = []
        self.component_library = []
        self.linker_library = []

        self.reflex_dictionaries = {}
        self.component_libraries = set()
        self.linker_libraries = set()

        self.log = logging.getLogger('Package(%s)' % self.name)
        try:
            self._parseRequirements()
        except:
            print "Processing %s" % self.requirements
            raise
        # update the known subdirs
        known_subdirs[self.name] = {# list of linker libraries provided by the package
                                    'libraries': list(self.linker_libraries),
                                    # true if it's a headers-only package
                                    'includes': bool(self.install_more_includes and
                                                     not self.linker_libraries)}

    def generate(self):
        # header
        data = ["#" * 80,
                "# Package: %s" % self.name,
                "#" * 80,
                "gaudi_subdir(%s %s)" % (self.name, self.version),
                ""]
        # dependencies
        #  subdirectories (excluding specials)
        subdirs = [n for n in sorted(self.uses)
                   if not n.startswith("LCG_Interfaces/")
                      and n not in ignore_dep_on_subdirs]

        inc_dirs = []
        if subdirs:
            # check if we are missing info for a subdir
            missing_subdirs = set([s.rsplit('/')[-1] for s in subdirs]) - set(known_subdirs)
            if missing_subdirs:
                self.log.warning('Missing info cache for subdirs %s', ' '.join(sorted(missing_subdirs)))
            # declare inclusion order
            data.append(callStringWithIndent('gaudi_depends_on_subdirs', subdirs))
            data.append('')
            # consider header-only subdirs
            #  for each required subdir that comes with only headers, add its
            #  location to the call to 'include_directories'
            inc_only = lambda s: known_subdirs.get(s.rsplit('/')[-1], {}).get('includes')
            inc_dirs = filter(inc_only, subdirs)


        #  externals (excluding specials)
        #  - Python needs to be treated in a special way
        find_packages = {}
        for n in sorted(self.uses):
            if n.startswith("LCG_Interfaces/"):
                n = extName(n[15:])
                # FIXME: find a general way to treat these special cases
                if n == "PythonLibs":
                    if self.name not in needing_python: # only these packages actually link against Python
                        continue
                # get custom link options
                linkopts = self.macros.get(n + '_linkopts', '')
                components = [m.group(1) or m.group(2)
                              for m in re.finditer(r'(?:\$\(%s_linkopts_([^)]*)\))|(?:-l(\w*))' % n,
                                                   linkopts)]
                # FIXME: find a general way to treat the special cases
                if n == 'COOL':
                    components = ['CoolKernel', 'CoolApplication']
                elif n == 'CORAL':
                    components = ['CoralBase', 'CoralKernel', 'RelationalAccess']

                find_packages[n] = find_packages.get(n, []) + components

        # this second loops avoid double entries do to converging results of extName()
        for n in sorted(find_packages):
            args = [n]
            components = find_packages[n]
            if components:
                args.append('COMPONENTS')
                args.extend(components)
            data.append('find_package(%s)' % ' '.join(args))
        data.append("")

        if self.name in no_pedantic:
            data.append('string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")\n')

        # the headers can be installed via "PUBLIC_HEADERS" or by hand
        if self.install_more_includes:
            headers = [d for d in self.install_more_includes.values()
                       if os.path.isdir(os.path.join(self.path, d))]
        else:
            headers = []

        if self.god_headers or self.god_dictionary:
            data.append("include(GaudiObjDesc)")
            data.append("")

        god_headers_dest = None
        if self.god_headers:
            godargs = [self.god_headers["files"].replace("../", "")]

            godflags = self.macros.get('%sObj2Doth_GODflags' % self.name, "")
            godflags = re.search(r'-s\s*(\S+)', godflags)
            if godflags:
                god_headers_dest = os.path.normpath('Event/' + godflags.group(1))
                godargs.append('DESTINATION ' + god_headers_dest)

            data.append(callStringWithIndent('god_build_headers', godargs))
            data.append("")

        god_dict = []
        if self.god_dictionary:
            god_dict = [('--GOD--',
                        [self.god_dictionary["files"].replace("../", "")],
                        None, [])]

        rflx_dict = []
        for d in self.reflex_dictionary:
            for k in d:
                v = d[k]
                v = v.replace("$(%sROOT)/" % self.name.upper(), "")
                v = v.replace("../", "")
                d[k] = v

            rflx_dict.append((d['dictionary'] + 'Dict',
                              [d['headerfiles'], d['selectionfile']],
                              None,
                              []))

        # libraries
        global_imports = [extName(name[15:])
                          for name in self.uses
                          if name.startswith('LCG_Interfaces/') and self.uses[name][1]] # list of imported ext
        if 'PythonLibs' in global_imports and self.name not in needing_python:
            global_imports.remove('PythonLibs')

        subdir_imports = [s.rsplit('/')[-1] for s in subdirs if self.uses[s][1]]
        local_links = [] # keep track of linker libraries found so far
        applications_names = set([a[0] for a in self.applications])
        # Note: a god_dictionary, a reflex_dictionary or an application is like a module
        for name, sources, group, imports in self.libraries + god_dict + rflx_dict + self.applications:
            isGODDict = isRflxDict = isComp = isApp = isLinker = False
            if name == '--GOD--':
                isGODDict = True
                name = '' # no library name for GOD dictionaries
            elif name.endswith('Dict') and name[:-4] in self.reflex_dictionaries:
                isRflxDict = True
                name = name[:-4]
            elif name in self.component_libraries:
                isComp = True
            elif name in applications_names:
                isApp = True
            else:
                if name not in self.linker_libraries:
                    self.log.warning('library %s not declared as component or linker, assume linker', name)
                isLinker = True

            # prepare the bits of the command: cmd, name, sources, args
            if isComp:
                cmd = 'gaudi_add_module'
            elif isGODDict:
                cmd = 'god_build_dictionary'
            elif isRflxDict:
                cmd = 'gaudi_add_dictionary'
            elif isApp:
                cmd = 'gaudi_add_executable'
            else: # i.e. isLinker (a fallback)
                cmd = 'gaudi_add_library'

            if not sources:
                self.log.warning("Missing sources for target %s", name)

            args = []
            if isLinker:
                if headers:
                    args.append('PUBLIC_HEADERS ' + ' '.join(headers))
                else:
                    args.append('NO_PUBLIC_HEADERS')
            elif isGODDict and god_headers_dest:
                args.append('HEADERS_DESTINATION ' + god_headers_dest)

            # # collection of link libraries. #
            # Externals and subdirs are treated differently:
            # - externals: just use the package name
            # - subdirs: find the exported libraries in the global var known_subdirs
            # We also have to add the local linker libraries.

            # separate external and subdir explicit imports
            subdirsnames = [s.rsplit('/')[-1] for s in subdirs]
            subdir_local_imports = [i for i in imports if i in subdirsnames]
            ext_local_imports = [extName(i) for i in imports if i not in subdir_local_imports]

            # prepare the link list with the externals
            links = global_imports + ext_local_imports
            if links or inc_dirs:
                # external links need the include dirs
                args.append('INCLUDE_DIRS ' + ' '.join(links + inc_dirs))
            # add subdirs...
            for s in subdir_imports + subdir_local_imports:
                if s in known_subdirs:
                    links.extend(known_subdirs[s]['libraries'])
            # ... and local libraries
            links.extend(local_links)
            if 'AIDA' in links:
                links.remove('AIDA') # FIXME: AIDA does not have a library

            if links:
                # note: in some cases we get quoted library names
                args.append('LINK_LIBRARIES ' + ' '.join([l.strip('"') for l in links]))

            if isRflxDict and self.reflex_dictionaries[name]:
                args.append('OPTIONS ' + self.reflex_dictionaries[name])

            if isLinker:
                local_links.append(name)

            # FIXME: very very special case :(
            if name == 'garbage' and self.name == 'FileStager':
                data.append('# only for the applications\nfind_package(Boost COMPONENTS program_options)\n')

            # write command
            sources = [s.replace('../src/', '') for s in sources]
            # FIXME: special case
            sources = [s.replace('$(GAUDICONFROOT)', '${CMAKE_SOURCE_DIR}/GaudiConf') for s in sources]
            libdata = callStringWithIndent(cmd, [name] + sources + args)

            # FIXME: wrap the test libraries in one if block (instead of several)
            if group in ('tests', 'test'):
                # increase indentation
                libdata = ['  ' + l for l in libdata.splitlines()]
                # and wrap
                libdata.insert(0, 'if(BUILD_TESTS)')
                libdata.append('endif()')
                libdata = '\n'.join(libdata)
            data.append(libdata)
            data.append('') # empty line

        # installation
        installs = []
        if headers and not self.linker_libraries: # not installed yet
            installs.append("gaudi_install_headers(%s)" % (" ".join(headers)))
        if self.install_python_modules:
            installs.append("gaudi_install_python_modules()")
        if self.install_scripts:
            installs.append("gaudi_install_scripts()")
        if installs:
            data.extend(installs)
            data.append('') # empty line
        # tests
        if self.QMTest:
            data.append("\ngaudi_add_test(QMTest QMTEST)")

        return "\n".join(data) + "\n"

    def process(self, force=False):
        # @FIXME: do something for the package
        cml = os.path.join(self.path, "CMakeLists.txt")
        if not force and os.path.exists(cml):
            self.log.warning("file %s already exists", cml)
            return
        data = self.generate()
        f = open(cml, "w")
        f.write(data)
        f.close()

    def _parseRequirements(self):
        def requirements():
            statement = ""
            for l in open(self.requirements):
                if '#' in l:
                    l = l[:l.find('#')]
                l = l.strip()
                if l:
                    statement += l
                    if statement.endswith('\\'):
                        statement = statement[:-1] + ' '
                        continue
                    else:
                        try:
                            yield list(CMTParser.parseString(statement))
                        except:
                            # ignore not know statements
                            self.log.debug("Failed to parse statement: %r", statement)
                        statement = ""

        for args in requirements():
            cmd = args.pop(0)
            if cmd == 'version':
                self.version = args[0]
            elif cmd == "use":
                if "-no_auto_imports" in args:
                    imp = False
                    args.remove("-no_auto_imports")
                else:
                    imp = True
                if len(args) > 1: # only one argument means usually a conditional use
                    if len(args) > 2:
                        name = "%s/%s" % (args[2], args[0])
                    else:
                        name = args[0]
                    self.uses[name] = (args[1], imp)

            elif cmd == "apply_pattern":
                pattern = args.pop(0)
                args = dict([x.split('=', 1) for x in args])
                if pattern in self.singleton_patterns:
                    setattr(self, pattern, args or True)
                elif pattern in self.multi_patterns:
                    getattr(self, pattern).append(args)

            elif cmd == 'library':
                name = args.pop(0)
                # digest arguments (options, variables, sources)
                imports = []
                group = None
                sources = []
                for a in args:
                    if a.startswith('-'): # options
                        if a.startswith('-import='):
                            imports.append(a[8:])
                        elif a.startswith('-group='):
                            group = a[7:]
                    elif '=' in a: # variable
                        pass
                    else: # source
                        sources.append(a)
                self.libraries.append((name, sources, group, imports))

            elif cmd == 'application':
                name = args.pop(0)
                # digest arguments (options, variables, sources)
                imports = []
                group = None
                sources = []
                for a in args:
                    if a.startswith('-'): # options
                        if a.startswith('-import='):
                            imports.append(a[8:])
                        elif a.startswith('-group='):
                            group = a[7:]
                        elif a == '-check': # used for test applications
                            group = 'tests'
                    elif '=' in a: # variable
                        pass
                    else: # source
                        sources.append(a)
                if 'test' in name.lower() or [s for s in sources if 'test' in s.lower()]:
                    # usually, developers do not put tests in the right group
                    group = 'tests'
                self.applications.append((name, sources, group, imports))

            elif cmd == 'macro':
                # FIXME: should handle macro tags
                name = args.pop(0)
                value = args[0].strip('"').strip("'")
                self.macros[name] = value

            elif cmd == 'macro_append':
                # FIXME: should handle macro tags
                name = args.pop(0)
                value = args[0].strip('"').strip("'")
                self.macros[name] = self.macros.get(name, "") + value


        # classification of libraries in the package
        unquote = lambda x: x.strip('"').strip("'")
        self.component_libraries = set([unquote(l['library']) for l in self.component_library])
        self.linker_libraries = set([unquote(l['library']) for l in self.linker_library])
        self.reflex_dictionaries = dict([(unquote(l['dictionary']), l.get('options', ''))
                                         for l in self.reflex_dictionary])

class Project(object):
    def __init__(self, path):
        """
        Crete a project instance from the root directory of the project.
        """
        self.path = os.path.realpath(path)
        if not isProject(self.path):
            raise ValueError("%s is not a project" % self.path)
        self.requirements = os.path.join(self.path, "cmt", "project.cmt")
        # Private variables for cached properties
        self._packages = None
        self._container = None

    @property
    def packages(self):
        """
        Dictionary of packages contained in the project.
        """
        if self._packages is None:
            self._packages = {}
            for root, dirs, _files in os.walk(self.path):
                if isPackage(root):
                    p = Package(root, self)
                    name = os.path.relpath(p.path, self.path)
                    self._packages[name] = p
                    dirs[:] = []
        return self._packages

    @property
    def container(self):
        """
        Name of the container package of the project.

        The name of the container is deduced using the usual LHCb convention
        (instead of the content of project.cmt).
        """
        if self._container is None:
            for suffix in ["Release", "Sys"]:
                try:
                    # gets the first package that ends with the suffix, and does
                    # not have a hat.. or raise StopIteration
                    c = (p for p in self.packages
                         if p.endswith(suffix) and "/" not in p).next()
                    self._container = self.packages[c]
                    break
                except StopIteration:
                    pass
        return self._container

    @property
    def name(self):
        # The name of the project is the same of the container without
        # the 'Release' or 'Sys' suffixes.
        return self.container.name.replace("Release", "").replace("Sys", "")

    @property
    def version(self):
        return self.container.version

    def uses(self):
        for l in open(self.requirements):
            l = l.split()
            if l and l[0] == "use" and l[1] != "LCGCMT" and len(l) == 3:
                yield (projectCase(l[1]), l[2].rsplit('_', 1)[-1])

    def generate(self):
        # list containing the lines to write to the file
        data = ["CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)",
                "",
                "set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/modules  ${CMAKE_MODULE_PATH})",
                "",
                "#---------------------------------------------------------------",
                "# Load macros and functions for Gaudi-based projects",
                "find_package(GaudiProject)",
                "#---------------------------------------------------------------",
                "",
                "# Declare project name and version"]
        l = "gaudi_project(%s %s" % (self.name, self.version)
        use = "\n                  ".join(["%s %s" % u for u in self.uses()])
        if use:
            l += "\n              USE " + use
        l += ")"
        data.append(l)
        return "\n".join(data) + "\n"

    def process(self, force=False):
        # Prepare the project configuration
        cml = os.path.join(self.path, "CMakeLists.txt")
        if force or not os.path.exists(cml):
            # write the file
            data = self.generate()
            f = open(cml, "w")
            f.write(data)
            f.close()
        else:
            logging.warning("file %s already exists", cml)
        # Recurse in the packages
        for p in sorted(self.packages):
            self.packages[p].process(force)


def main(args=None):
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] [path to project or package]",
                          description="Convert CMT-based projects/packages to CMake (Gaudi project)")
    parser.add_option("-f", "--force", action="store_true",
                      help="overwrite existing files")
    parser.add_option('--cache-only', action='store_true',
                      help='just update the cache without creating the CMakeLists.txt files.')
    #parser.add_option('--cache-file', action='store',
    #                  help='file to be used for the cache')

    opts, args = parser.parse_args(args=args)

    logging.basicConfig(level=logging.INFO)

    top_dir = os.getcwd()
    if args:
        top_dir = args[0]
        if not os.path.isdir(top_dir):
            parser.error("%s is not a directory" % top_dir)

    if isProject(top_dir):
        root = Project(top_dir)
    elif isPackage(top_dir):
        root = Package(top_dir)
        if opts.cache_only:
            return # the cache is updated instantiating the package
    else:
        raise ValueError("%s is neither a project nor a package" % top_dir)

    if opts.cache_only:
        root.packages # the cache is updated by instantiating the packages
        # note that we can get here only if root is a project
    else:
        root.process(opts.force)


if __name__ == '__main__':
    main()
    sys.exit(0)

all_packs = ["Kernel/XMLSummaryBase",
"DAQ/Tell1Kernel",
"Si/SiDAQ",
"Calo/CaloKernel",
"GaudiObjDesc",
"GaudiConfUtils",
"Kernel/Relations",
"Event/DAQEvent",
"Tools/FileStager",
"Tools/XmlTools",
"Kernel/HistoStrings",
"L0/L0Base",
"GaudiMTTools",
"DAQ/MDF",
"DAQ/MDF_ROOT",
"Kernel/LHCbMath",
"Kernel/HltInterfaces",
"Det/DetDesc",
"Det/DetDescSvc",
"Det/DetDescCnv",
"Det/BcmDet",
"Kernel/PartProp",
"Kernel/LHCbKernel",
"L0/ProcessorKernel",
"L0/L0MuonKernel",
"Event/VeloEvent",
"Det/VeloPixDet",
"Det/VeloDet",
"Det/STDet",
"Rich/RichKernel",
"Det/RichDet",
"GaudiConf",
"Kernel/XMLSummaryKernel",
"Det/DetCond",
"Tools/CondDBEntityResolver",
"Ex/DetCondExample",
"Det/DDDB",
"Tools/CondDBUI",
"Det/Magnet",
"Det/DetDescChecks",
"Det/OTDet",
"Muon/MuonKernel",
"Det/MuonDet",
"Event/L0Event",
"L0/L0Interfaces",
"Det/CaloDet",
"Det/CaloDetXmlCnv",
"Ex/DetDescExample",
"Det/DetSys",
"Event/LinkerEvent",
"Event/TrackEvent",
"Tr/TrackKernel",
"Event/DigiEvent",
"OT/OTDAQ",
"Event/EventBase",
"Event/LumiEvent",
"Event/GenEvent",
"Event/MCEvent",
"Kernel/MCInterfaces",
"Sim/SimComponents",
"Event/RecEvent",
"Rich/RichRecBase",
"Phys/LoKiCore",
"Phys/LoKiNumbers",
"Phys/LoKiMC",
"Phys/LoKiGen",
"Muon/MuonInterfaces",
"Tf/TfKernel",
"Tf/TsaKernel",
"Tf/PatKernel",
"ST/STKernel",
"ST/STTELL1Event",
"Kernel/KernelSys",
"Kernel/LHCbAlgs",
"Event/RecreatePIDTools",
"DAQ/DAQUtils",
"Muon/MuonDAQ",
"Tr/TrackInterfaces",
"Calo/CaloInterfaces",
"Event/PhysEvent",
"Event/SwimmingEvent",
"Event/LinkerInstances",
"Event/MicroDst",
"Event/PackedEvent",
"Event/HltEvent",
"Phys/LoKiHlt",
"Event/EventPacker",
"Ex/IOExample",
"Event/EventAssoc",
"Event/EventSys",
"Calo/CaloUtils",
"Calo/CaloDAQ",
"DAQ/DAQSys",
"Associators/MCAssociators",
"LHCbSys",
]

#idx = 6
idx = all_packs.index('Event/GenEvent')

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    pr = Project("/home/marco/Devel/LHCb/workspace/LHCb_trunk")
    print pr.packages.keys()
    print "Woring on", all_packs[idx]

    print "=== Project CMakeLists.txt", "=" * 80
    print pr.generate()

    pack = pr.packages[all_packs[idx]]
    print "=== Package requirements", "=" * 80
    print open(pack.requirements).read()
    print "=== Pacakge CMakeLists.txt", "=" * 80
    print pack.generate()

    #print "=" * 80
    #print pr.packages["Det/DetCond"].generate()

    #print "=" * 80
    #print pr.packages["Kernel/LHCbKernel"].generate()
