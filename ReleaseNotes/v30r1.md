---
version: v30r1
date: 2017-12-21
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

This minor release features mainly bugfixes ans clean up of the code, but also
some new features and improvements in the task scheduling.

*   [Build System](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&milestone_title=v30r1&scope=all&state=merged)
    -   Proper use of C++ GSL and Range-v3 in CMake ([mr !497](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/497))
    -   Add support for +cov optimization subtype ([mr !519](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/519))
    -   Cache os calls to make xenv faster ([mr !533](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/533))
*   [Configuration](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=configuration&milestone_title=v30r1&scope=all&state=merged)
    -   Fixed use of zlib ([mr !553](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/553))
    -   Update versions for Gaudi v30r1 ([mr !549](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/549))
*   [C++ Framework](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&milestone_title=v30r1&scope=all&state=merged)
    -   Replace `DataObjID{,Coll}Property` with `Gaudi::Property<DataObjID{,Coll}>`` ([mr !426](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/426))
    -   Add AnyDataWrapperBase::size() ([mr !454](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/454))
    -   Avoid heap-allocation of ParticleDataTable ([mr !490](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/490))
    -   Optimize AlgsExecutionStates ([mr !498](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/498))
    -   Set algorithm exec state/status in Algorithm::sysExecute ([mr !523](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/523))
    -   GaudiAlg FunctionalDetails apply range size check in debug builds only ([mr !529](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/529))
    -   Fixed missing check for outputEnabled in RootHistCnv/src/PersSvc.cpp ([mr !536](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/536))
    -   Optimized allocation of TES slot in HiveWhiteBoard ([mr !538](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/538))
    -   Fix counter printout issue in Hive by using TBB map instead of std map ([mr !544](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/544))
    -   Sort components by name in HistorySvc dump ([mr !552](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/552))
*   [Code Cleanup](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=code+cleanup&milestone_title=v30r1&scope=all&state=merged)
    -   Modernize ContextSpecificPtr ([mr !456](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/456))
    -   Replace FuncPtrCast with reinterpret_cast ([mr !485](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/485))
    -   Modernize MetaDataSvc ([mr !488](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/488))
    -   Modernize AlgExecStateSvc ([mr !489](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/489))
    -   Modernize HiveSlimEventLoopMgr ([mr !512](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/512))
    -   Modernize ContextEventCounter ([mr !513](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/513))
    -   Modernize constructors and destructors ([mr !515](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/515))
    -   Add missing header guard ([mr !516](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/516))
    -   Remove implicit StatusCode bool/long conversions ([mr !517](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/517))
    -   Remove `HiveEventRegistryEntry.h` ([mr !521](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/521))
    -   Remove `virtual` qualifiers on member functions of a `final` class ([mr !522](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/522))
    -   Fix clang 5.0 warnings ([mr !525](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/525))
    -   Simplify CallgrindProfile ([mr !527](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/527))
    -   Modernize InertMessageSvc ([mr !528](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/528))
    -   Prefer std::make_unique over explicit call to new ([mr !530](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/530))
    -   Reduce indirection in Service lock map ([mr !531](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/531))
    -   Fix TBB/clang feature detection problem symptoms and some clang warnings ([mr !532](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/532))
*   [Task Scheduling](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=task+scheduling&milestone_title=v30r1&scope=all&state=merged)
    -   Add EventView handling to AvalancheScheduler ([mr !496](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/496))
    -   Added a new event loop manager for HLT like usage, that is very simple and very fast scheduling ([mr !537](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/537))
    -   Fix a bug in data/condition object state detection in temporal and topological tracer's (3T) graph assembling ([mr !543](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/543))


The full list of changes can be found on [Gitlab](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?milestone_title=v30r1&scope=all&state=merged).

**Externals version**: [LCG 91](http://lcginfo.cern.ch/release/92/)
