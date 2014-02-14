## -*- python -*-
## automatically generated from a hscript
## do NOT edit.

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "Gaudi",
    "authors": [],

    "version":  "v25r0",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("GaudiSys", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/pytools", version="*", public=True)

    ## no private dependencies
    ## runtime dependencies
    ctx.use_pkg("LCG_Interfaces/pytools", version="*", runtime=True)
    return # pkg_deps


### ---------------------------------------------------------------------------
def options(ctx):

    return # options


### ---------------------------------------------------------------------------
def configure(ctx):



    ##### **** statement *hlib.SetStmt (&{{GAUDIEXE [{default [${GAUDIROOT}/${tag}/Gaudi.exe]} {GAUDI_with_installarea [${Gaudi_cmtpath}/${GAUDI_installarea_prefix}/${tag}/bin/Gaudi.exe]}]}})
    #### path_remove &{{JOBOPTSEARCHPATH [{default [/Gaudi/]} {target-winxp [\Gaudi]}]}}
    ctx.hwaf_path_remove("JOBOPTSEARCHPATH", (
      {"default": "/Gaudi/"},
      {"target-winxp": "\\Gaudi"},
    ))
    #### path_prepend &{{JOBOPTSEARCHPATH [{default [${GAUDIROOT}/options]} {target-winxp [${GAUDIROOT}\options]}]}}
    ctx.hwaf_path_prepend("JOBOPTSEARCHPATH", (
      {"default": "${GAUDIROOT}/options"},
      {"target-winxp": "${GAUDIROOT}\\options"},
    ))
    #### path_prepend &{{JOBOPTSEARCHPATH [{default []} {QMTest&target-winxp [${GAUDIROOT}\tests\pyjobopts]} {QMTest [${GAUDIROOT}/tests/pyjobopts]}]}}
    ctx.hwaf_path_prepend("JOBOPTSEARCHPATH", (
      {"default": ""},
      {("QMTest", "target-winxp"): "${GAUDIROOT}\\tests\\pyjobopts"},
      {"QMTest": "${GAUDIROOT}/tests/pyjobopts"},
    ))
    #### path_prepend &{{PYTHONPATH [{default []} {QMTest&target-winxp [${GAUDIROOT}\tests\python]} {QMTest [${GAUDIROOT}/tests/python]}]}}
    ctx.hwaf_path_prepend("PYTHONPATH", (
      {"default": ""},
      {("QMTest", "target-winxp"): "${GAUDIROOT}\\tests\\python"},
      {"QMTest": "${GAUDIROOT}/tests/python"},
    ))

    return # configure


### ---------------------------------------------------------------------------
def build(ctx):


    ctx(
        features = "gaudi_application",
        name     = "Gaudi",
        target   = "Gaudi",
        source   = ["main.cpp"],
        use      = ["GaudiKernel"],
    )

    ctx.gaudi_install_python_modules()
    ctx.gaudi_install_scripts()

    return # build

## EOF ##
