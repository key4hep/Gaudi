## -*- python -*-

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "GaudiMP",
    "authors": ["gaudi collaboration"],

    "version":  "v3r0",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("GaudiKernel", version="*", public=True)
    ctx.use_pkg("GaudiAlg", version="*", public=True)
    ctx.use_pkg("GaudiPython", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Python", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Reflex", version="*", public=True)
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
        features = "gaudi_module",
        name     = "GaudiMP",
        target   = "GaudiMP",
        source   = ["src/component/*.cpp"],
        use      = ["GaudiMPLib",
                    "GaudiAlgLib",
                    "ROOT",
                    "python",
                    ],
    )

    ctx(
        features = "gaudi_library",
        name     = "GaudiMPLib",
        target   = "GaudiMPLib",
        source   = ["src/Lib/*.cpp"],
        use      = [
            "GaudiKernel",
            "ROOT", "python"
            ],
    )

    ctx(
        features = "gaudi_dictionary",
        name     = "GaudiMPLibDict",
        source   = "dict/gaudimp_dict.h",
        selection_file = "dict/selection.xml",
        use      = ["GaudiMPLib",
                    ],
    )

    ctx.gaudi_install_python_modules()
    ctx.gaudi_install_scripts()

    return # build

## EOF ##
