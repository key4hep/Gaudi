## -*- python -*-

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "GaudiProfiling",
    "authors": [],

    "version":  "v1r8",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("GaudiKernel", version="v*", public=True)
    ctx.use_pkg("GaudiAlg", version="v*", public=True)
    ctx.use_pkg("LCG_Interfaces/ROOT", version="v*", public=True)
    ctx.use_pkg("LCG_Interfaces/Boost", version="v*", public=True)

    ## private dependencies
    ctx.use_pkg("LCG_Interfaces/libunwind", version="v*", private=True)
    ctx.use_pkg("LCG_Interfaces/tcmalloc", version="v*", private=True)
    ctx.use_pkg("LCG_Interfaces/Python", version="v*", private=True)

    ## runtime dependencies
    ctx.use_pkg("LCG_Interfaces/tcmalloc", version="v*", runtime=True)
    ctx.use_pkg("LCG_Interfaces/Python", version="v*", runtime=True)
    return # pkg_deps


### ---------------------------------------------------------------------------
def options(ctx):

    return # options


### ---------------------------------------------------------------------------
def configure(ctx):


    ## FIXME:
    if 1:
        return

    #### macro &{{VTune_config_version [{default [2011]}]}}
    ctx.hwaf_declare_macro("VTune_config_version", (
      {"default": "2011"},
    ))
    #### macro &{{VTune_native_version [{default [${VTune_config_version}]}]}}
    ctx.hwaf_declare_macro("VTune_native_version", (
      {"default": "${VTune_config_version}"},
    ))
    #### macro &{{Intel_base [{default [/afs/cern.ch/sw/IntelSoftware/linux/x86_64]} {target-i686 [/afs/cern.ch/sw/IntelSoftware/linux/ia32]}]}}
    ctx.hwaf_declare_macro("Intel_base", (
      {"default": "/afs/cern.ch/sw/IntelSoftware/linux/x86_64"},
      {"target-i686": "/afs/cern.ch/sw/IntelSoftware/linux/ia32"},
    ))
    #### macro &{{VTune_home [{default [${Intel_base}/xe${VTune_native_version}/vtune_amplifier_xe_${VTune_config_version}]}]}}
    ctx.hwaf_declare_macro("VTune_home", (
      {"default": "${Intel_base}/xe${VTune_native_version}/vtune_amplifier_xe_${VTune_config_version}"},
    ))
    ##### **** statement *hlib.SetStmt (&{{VTUNE_HOME [{default [${VTune_home}]}]}})
    #### macro &{{VTune_linkopts [{default [${VTune_home}/${unixdirname}/libittnotify.a]}]}}
    ctx.hwaf_declare_macro("VTune_linkopts", (
      {"default": "${VTune_home}/${unixdirname}/libittnotify.a"},
    ))
    #### macro &{{VTune_cppflags [{default [-I${VTune_home}/include]}]}}
    ctx.hwaf_declare_macro("VTune_cppflags", (
      {"default": "-I${VTune_home}/include"},
    ))
    #### macro &{{have_vtune [{default [not-have-vtune]} {target-x86_64&target-linux [`${GaudiProfiling_root}/cmt/check_vtune`]}]}}
    ctx.hwaf_declare_macro("have_vtune", (
      {"default": "not-have-vtune"},
      {("target-x86_64", "target-linux"): "${GaudiProfiling_root}/cmt/check_vtune"},
    ))
    #### apply_tag &{{$(have_vtune) []}}
    ctx.hwaf_apply_tag("${have_vtune}")
    #### macro_remove &{{constituents [{default [IntelProfilerConfDbMerge]} {have-vtune []}]}}
    ctx.hwaf_macro_remove("constituents", (
      {"default": "IntelProfilerConfDbMerge"},
      {"have-vtune": ""},
    ))
    #### macro_remove &{{constituents [{default [IntelProfilerConf]} {have-vtune []}]}}
    ctx.hwaf_macro_remove("constituents", (
      {"default": "IntelProfilerConf"},
      {"have-vtune": ""},
    ))
    #### macro_remove &{{constituents [{default [IntelProfilerMergeMap]} {have-vtune []}]}}
    ctx.hwaf_macro_remove("constituents", (
      {"default": "IntelProfilerMergeMap"},
      {"have-vtune": ""},
    ))
    #### macro_remove &{{constituents [{default [IntelProfilerRootMap]} {have-vtune []}]}}
    ctx.hwaf_macro_remove("constituents", (
      {"default": "IntelProfilerRootMap"},
      {"have-vtune": ""},
    ))
    #### macro_remove &{{constituents [{default [IntelProfiler]} {have-vtune []}]}}
    ctx.hwaf_macro_remove("constituents", (
      {"default": "IntelProfiler"},
      {"have-vtune": ""},
    ))
    #### macro_append &{{use_includes [{default []} {have-vtune [${VTune_cppflags}]}]}}
    ctx.hwaf_macro_append("use_includes", (
      {"default": ""},
      {"have-vtune": "${VTune_cppflags}"},
    ))
    #### macro &{{libunwind_linkopts [{default []}]}}
    ctx.hwaf_declare_macro("libunwind_linkopts", (
      {"default": ""},
    ))
    #### macro_remove &{{componentshr_linkopts [{default [-Wl,--no-undefined]}]}}
    ctx.hwaf_macro_remove("componentshr_linkopts", (
      {"default": ["-Wl,--no-undefined"]},
    ))
    #### macro_remove &{{PyCPUFamily_use_linkopts [{default [${GaudiProfiling_linkopts}]}]}}
    ctx.hwaf_macro_remove("PyCPUFamily_use_linkopts", (
      {"default": "${GaudiProfiling_linkopts}"},
    ))

    return # configure


### ---------------------------------------------------------------------------
def build(ctx):


    ##### **** statement *hlib.ApplyPatternStmt (&{pyd_boost_module [module=PyCPUFamily files=python/CPUFamily.cpp]})

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "GaudiGenProfilingHtml",
        target   = "GaudiGenProfilingHtml",
        source   = ["app/pfm_gen_analysis.cpp"],
        use      = ["GaudiKernel", "z"],
    )

    ctx(
        features = "gaudi_library",
        name     = "GaudiGoogleProfiling",
        target   = "GaudiGoogleProfiling",
        source   = ["component/google/*.cpp"],
        use      = ["GaudiKernel", "GaudiAlgLib", "z", "tcmalloc"],
    )

    ctx(
        features = "gaudi_module",
        name     = "GaudiProfiling",
        target   = "GaudiProfiling",
        source   = ["component/*.cpp"],
        use      = ["GaudiKernel", "z", "libunwind"],
    )

    ctx.gaudi_install_python_modules()
    ctx.gaudi_install_scripts()

    ## FIXME
    if 0:
        ctx(
        features = "gaudi_module",
        name     = "IntelProfiler",
        target   = "IntelProfiler",
        source   = ["component/intel/*.cpp"],
        use      = ["GaudiKernel"],
    )
    return # build

## EOF ##
