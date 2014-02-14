## -*- python -*-

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "GaudiKernel",
    "authors": ["LHCb Gaudi Team"],

    "version":  "v30r0",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("GaudiPolicy", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Reflex", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Boost", version="*", public=True)

    ## private dependencies
    ctx.use_pkg("LCG_Interfaces/CppUnit", version="*", private=True)

    ## runtime dependencies
    ctx.use_pkg("LCG_Interfaces/CppUnit", version="*", runtime=True)
    return # pkg_deps


### ---------------------------------------------------------------------------
def options(ctx):

    return # options


### ---------------------------------------------------------------------------
def configure(ctx):

    return # configure


### ---------------------------------------------------------------------------
def build(ctx):

    ctx(
        features = "gaudi_library",
        name     = "GaudiKernel",
        source   = ["src/Lib/*.cpp"],
        use      = ["ROOT",
                    "Reflex",
                    "boost-thread",
                    "boost-filesystem",
                    "boost-system",
                    ],
    )

    ctx(
        features = "gaudi_application",
        name     = "genconf",
        source   = ["src/Util/genconf.cpp"],
        use = ["GaudiKernel",
               "boost-filesystem",
               "boost-regex",
               "boost-program_options",
               "Reflex",
               ],
    )
    ctx.env['GENCONF'] = 'genconf.exe'

    ctx(
        features = "gaudi_dictionary",
        name = "GaudiKernelDict",
        source = "dict/dictionary.h",
        selection_file = "dict/dictionary.xml",
        use = ["GaudiKernel", "Reflex"],
    )

    ctx.gaudi_install_python_modules()
    ctx.gaudi_install_scripts()

    ## tests --
    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "DirSearchPath_test",
        source   = ["tests/src/DirSearchPath_test.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "Memory_test",
        source   = ["tests/src/Memory_test.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "Parsers_test",
        target   = "Parsers_test",
        source   = ["tests/src/parsers.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "PathResolver_test",
        target   = "PathResolver_test",
        source   = ["tests/src/PathResolver_test.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "test_GaudiTime",
        source   = ["tests/src/test_GaudiTime.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "test_GaudiTiming",
        source   = ["tests/src/test_GaudiTiming.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "test_SerializeSTL",
        source   = ["tests/src/test_SerializeSTL.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "test_headers_build",
        source   = ["tests/src/test_headers_build.cpp"],
        use      = ["GaudiKernel", "CppUnit"],
    )
    return # build

## EOF ##
