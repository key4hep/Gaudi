## -*- python -*-

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "GaudiExamples",
    "authors": ["gaudi collaboration"],

    "version":  "v25r0",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("LCG_Interfaces/ROOT", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Reflex", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/RELAX", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/AIDA", version="*", public=True)
    ctx.use_pkg("Gaudi", version="*", public=True)
    ctx.use_pkg("GaudiPartProp", version="*", public=True)
    ctx.use_pkg("PartPropSvc", version="*", public=True)
    ctx.use_pkg("GaudiPython", version="*", public=True)
    ctx.use_pkg("GaudiGSL", version="*", public=True)
    ctx.use_pkg("RootHistCnv", version="*", public=True)
    ctx.use_pkg("RootCnv", version="*", public=True)

    ctx.use_pkg("LCG_Interfaces/Boost", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/HepPDT", version="*", public=True)

    ## private dependencies
    ctx.use_pkg("LCG_Interfaces/Python", version="*", private=True)

    ## runtime dependencies
    ctx.use_pkg("LCG_Interfaces/Reflex", version="*", runtime=True)
    ctx.use_pkg("LCG_Interfaces/RELAX", version="*", runtime=True)
    ctx.use_pkg("LCG_Interfaces/Python", version="*", runtime=True)
    return # pkg_deps


### ---------------------------------------------------------------------------
def options(ctx):

    return # options


### ---------------------------------------------------------------------------
def configure(ctx):

    ctx.hwaf_path_prepend("PYTHONPATH", (
      {"default": ""},
      {"QMTest": "${GAUDIEXAMPLESROOT}/tests/qmtest"},
    ))

    return # configure


### ---------------------------------------------------------------------------
def build(ctx):


    ctx.gaudi_install_joboptions()

    ctx(
        features = "gaudi_application hwaf_utest",
        name     = "Allocator",
        target   = "Allocator",
        source   = ["src/Allocator/*.cpp"],
        use      = ["GaudiKernel"],
    )

    ctx(
        features = "gaudi_module",
        name     = "GaudiExamples",
        target   = "GaudiExamples",
        source   = ["src/AbortEvent/*.cpp",
                    "src/AlgSequencer/*.cpp",
                    "src/AlgTools/*.cpp",
                    "src/DataOnDemand/*.cpp",
                    "src/AlgErrAud/*.cpp",
                    "src/GSLTools/*.cpp",
                    "src/RandomNumber/*.cpp",
                    "src/Histograms/*.cpp",
                    "src/NTuples/*.cpp",
                    "src/TupleEx/*.cpp",
                    "src/CounterEx/*.cpp",
                    "src/PartProp/*.cpp",
                    "src/Properties/*.cpp",
                    "src/ExtendedProperties/*.cpp",
                    "src/POOLIO/*.cpp",
                    "src/ColorMsg/ColorMsgAlg.cpp",
                    "src/History/History.cpp",
                    "src/THist/*.cpp",
                    "src/ErrorLog/ErrorLogTest.cpp",
                    "src/EvtColsEx/EvtColAlg.cpp",
                    "src/Maps/*.cpp",
                    "src/MultipleLogStreams/*.cpp",
                    "src/GaudiCommonTests/*.cpp",
                    "src/IncidentSvc/*.cpp",
                    "src/bug_34121/*.cpp",
                    "src/Auditors/*.cpp",
                    "src/Timing/*.cpp",
                    "src/Selections/*.cpp",
                    "src/SvcInitLoop/*.cpp",
                    "src/StringKeys/*.cpp",
                    "src/StatusCodeSvc/*.cpp",
                    "src/testing/*.cpp",
                    "src/IntelProfiler/*.cpp",
                ],
        includes = ["src/Lib"],
        use = ["GaudiExamplesLib"],
    )

    ctx(
        features = "gaudi_library",
        name     = "GaudiExamplesLib",
        target   = "GaudiExamplesLib",
        source   = ["src/Lib/*.cpp"],
        use = ["GaudiGSLLib", "GaudiUtilsLib",
               "HepPDT",
               "ROOT",
               "GSL",
               ],

    )

    ctx(
        features = "gaudi_pyext",
        name   = "PyExample",
        source = "src/PythonModule/*.cpp",
        use    = ["boost-python"],
    )

    ctx(
        features = "gaudi_dictionary",
        name = "GaudiExamplesDict",
        selection_file = ["src/POOLIO/dict.xml"],
        source = "src/POOLIO/dict.h",
        use = ["GaudiExamplesLib"],
        includes = ["src/Lib"],
    )

    ## -- tests --
    ctx.gaudi_qmtest(
        name = "Properties",
        joboptions = "GaudiExamples/Properties.opts",
        qmtest_dir = "tests/qmtest",
    )
    ctx.gaudi_qmtest(
        name = "AlgTools",
        joboptions = "GaudiExamples/AlgTools.opts",
        qmtest_dir = "tests/qmtest",
    )

    return # build

## EOF ##
