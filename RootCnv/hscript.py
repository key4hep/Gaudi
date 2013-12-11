## -*- python -*-

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "RootCnv",
    "authors": ["gaudi collaboration"],

    "version":  "v1r21",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("GaudiKernel", version="*", public=True)
    ctx.use_pkg("GaudiUtils", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/ROOT", version="*", public=True)
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
        name     = "RootCnvLib",
        source   = ["src/*.cpp"],
        defines  = ["__POOL_COMPATIBILITY"],
        use      = ["GaudiKernel",
                    "GaudiUtilsLib",
                    "ROOT",
                    "ROOT-TreePlayer",
                    ],
    )

    ctx(
        features = "gaudi_module",
        name     = "RootCnv",
        source   = ["components/*.cpp"],
        defines  = ["__POOL_COMPATIBILITY"],
        use      = ["RootCnvLib"],
    )

    ctx(
        features = "gaudi_dictionary",
        name     = "RootCnvDict",
        source   = "dict/RootCnv_dict.h",
        selection_file = "dict/RootCnv_dict.xml",
        defines  = ["__POOL_COMPATIBILITY"],
        use      = ["RootCnvLib", "GaudiKernel", "Reflex"],
    )

    ## -- tests --
    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "gaudi_merge",
        source   = ["merge/merge.cpp"],
        use      = ["RootCnvLib"],
    )
    return # build

## EOF ##
