## -*- python -*-

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "GaudiPython",
    "authors": ["gaudi collaboration"],

    "version":  "v13r0",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("GaudiKernel", version="*", public=True)
    ctx.use_pkg("GaudiAlg", version="*", public=True)
    ctx.use_pkg("GaudiUtils", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/AIDA", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/CLHEP", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Python", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Reflex", version="*", public=True)

    ## private dependencies
    ctx.use_pkg("GaudiCoreSvc", version="*", private=True)
    ctx.use_pkg("GaudiCommonSvc", version="*", private=True)

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
        name     = "GaudiPythonLib",
        source   = ["src/Lib/*.cpp"],
        use      = ["GaudiAlgLib",
                    "python",
                    "CLHEP",
                    "AIDA",
                    ],
    )

    ctx(
        features = "gaudi_module",
        name     = "GaudiPython",
        source   = ["src/Services/*.cpp"],
        use      = ["GaudiPythonLib", "GaudiUtilsLib"],
    )

    ctx(
        features = "gaudi_dictionary",
        name     = "GaudiPythonDict",
        source   = "dict/kernel.h",
        selection_file = "dict/selection_kernel.xml",
        use      = ["GaudiPythonLib", "GaudiUtilsLib"],
    )

    ctx.gaudi_install_python_modules()

    ## -- tests --
    ctx(
        features = "gaudi_library",
        name     = "GPyTest",
        target   = "GPyTest",
        source   = ["src/Test/*.cpp"],
        use      = ["GaudiKernel", "python", "CLHEP", "AIDA"],
        includes = ["src/Test"],
    )

    ctx(
        features = "gaudi_dictionary",
        name     = "GPyTestDict",
        selection_file = "src/Test/test_selection.xml",
        source   = "src/Test/test.h",
        use      = ["GaudiKernel", "python", "CLHEP"],
    )

    return # build

## EOF ##
