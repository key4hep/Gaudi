# Changelog
All notable changes to this project will be documented in this file.

Project Coordinators: Marco Clemencic @clemenci, Charles Leggett @leggett, Stefan Roiser @roiser

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased][]


## [v35r3][] - 2021-04-20
Another minor bugfix release with more fixes for ROOT 6.22 and improvements to the CMake configuration.

Here is a summary of the changes (see the [full list of changes](https://gitlab.cern.ch/gaudi/Gaudi/compare/v35r2...v35r3)).

### Changed
- Change how optional dependencies are handled in downstream projects (gaudi/Gaudi!1194)
- Remove serial task queue (gaudi/Gaudi!1195)
- Switch to latest ATLAS task precedence scenario in a test (gaudi/Gaudi!1190)
- Change `DevBuildType` to also enable `-Wsuggest-override` on clang if version >= 11 (gaudi/Gaudi!1191)

### Added
- Test algorithm with internal multithreading (gaudi/Gaudi!1196)
- Added a cardinality setting for `AtlasMCRecoScenario` (gaudi/Gaudi!1201)
- Extend the task precedence tracer for dumping dynamic graphs (gaudi/Gaudi!1189)
- Allow use of local targets instead of imported ones (gaudi/Gaudi!1186)

### Fixed
- Allow returning void in a `MergingTransformer` (gaudi/Gaudi!1199)
- Add small test and fix for GaudiPython.Bindings.iDataSvc traversal (gaudi/Gaudi!1178)
- Fix handling of properties for tuples and arrays of size 1 (gaudi/Gaudi#21, gaudi/Gaudi!1197)
- Add dynamic dependencies scan for `genreflex` dictionaries (gaudi/Gaudi#152  gaudi/Gaudi!1183)
- Improvements on Monitoring::Hub::Entity and accumulators (gaudi/Gaudi!1185)
- Suppress warnings about `nodiscard` in dictionary generation with ROOT 6.22 (gaudi/Gaudi!1182)
- Add custom pythonization callback to SmartRefVector (gaudi/Gaudi!1180)
- Fixed bug in Histogram filling (gaudi/Gaudi!1176)
- Improve support for new style CMake projects (gaudi/Gaudi!1177)


## [v35r2][] - 2021-03-10
Just a minor bugfix release with changes required for
- ROOT 6.22 (updated PyROOT/cppyy)
- modernization of CMake configuration of LHCb projects
- use of new monitoring interfaces in LHCb Online system

Here is a summary of the changes (see the [full list of changes](https://gitlab.cern.ch/gaudi/Gaudi/compare/v35r1...v35r2)).

### Changed
- Allow use of local targets instead of imported ones (gaudi/Gaudi!1186)
- Improvements on `Monitoring::Hub::Entity` and accumulators (gaudi/Gaudi!1185)
- Improve support for new style `CMake` downstream projects (gaudi/Gaudi!1177)

### Fixed
- Add dynamic dependencies scan for `genreflex` dictionaries (gaudi/Gaudi#152, gaudi/Gaudi!1183)
- Supress warnings about `__nodiscard__` attribute in dictionary generation with ROOT 6.22 (gaudi/Gaudi!1182)
- Add custom pythonization callback to `SmartRefVector` (gaudi/Gaudi!1180)
- Fixed bug in Histogram filling (gaudi/Gaudi!1176)


## [v35r1][] - 2021-01-28
This is mostly a bugfix release, with a couple of backward incompatible changes in
Python `DataHandle` class (gaudi/Gaudi!1144, gaudi/Gaudi!1159) and deprecation warnings
from `IJobOptionsSvc`, which will be dropped in v36r0 (gaudi/Gaudi#140).

Here is a summary of the [full list of changes](https://gitlab.cern.ch/gaudi/Gaudi/compare/v35r0...v35r1).

### Changed
- DataHandle: use all members for `==` and improve unit test (gaudi/Gaudi!1159)
- Deprecate use of `IJobOptionsSvc` (gaudi/Gaudi#139, gaudi/Gaudi!1156)
- DataHandle: Remove `__add__` operators (gaudi/Gaudi#146, gaudi/Gaudi!1144)

### Added
- Add support to `DataWriteHandle` for writing (partially type erased) 'views' into the TES (gaudi/Gaudi!1151)

### Fixed
- Adapt tests to a change in TFile "file not found" error message (gaudi/Gaudi!1174)
- ARM Fixes (gaudi/Gaudi!1153)
- IncidentSvc: fix ever-growing incident map (gaudi/Gaudi!1164)
- Fix CF bug arising from empty sequence (gaudi/Gaudi#135, gaudi/Gaudi!1106)
- Re-enabled the installation of the `PartPropSvc` auxiliary files (gaudi/Gaudi!1172)
- Make iteration ordering predictable (gaudi/Gaudi!1169)
- StatusCode tweaks (gaudi/Gaudi!1171)
- GaudiToolbox: Fix generated `__init__` files to handle symlinks (gaudi/Gaudi!1170)
- Check result of `dynamic_cast<>` for null before dereferencing (gaudi/Gaudi!1168)
- Fix build instructions (gaudi/Gaudi#159, gaudi/Gaudi#161, gaudi/Gaudi!1162)
- Resolve "GAUDI_GENCONF_NO_FAIL option not working" (gaudi/Gaudi#160, gaudi/Gaudi!1161)
- Fix clang 10 warnings (gaudi/Gaudi!1166)
- Fix MacOS compilation (gaudi/Gaudi!1158)
- Do not set `Python_FIND_STRATEGY` (gaudi/Gaudi#157, gaudi/Gaudi!1157)
- Property: Catch all parsing errors (gaudi/Gaudi!1165)
- simplify `iid_cast` implementation (gaudi/Gaudi!1154)
- Monitor Algorithm instance misses (gaudi/Gaudi!1148)
- Fix GaudiPython classes (follows up gaudi/Gaudi!1116) (gaudi/Gaudi!1155)
- Fix py-formatting of exec (gaudi/Gaudi!1160)
- Fix Algorithm destructor invocation in multithreading applications (gaudi/Gaudi#150, gaudi/Gaudi!1149)

## [v35r0][] - 2020-11-10
This version features a complete rewrite of the CMake configuration. See gaudi/Gaudi!986 and gaudi/Gaudi!922 for details.

Here is a summary of the [full list of changes](https://gitlab.cern.ch/gaudi/Gaudi/compare/v34r1...v35r0).

### Changed
- Complete rewrite of CMake configuration in *modern* CMake (gaudi/Gaudi!986, gaudi/Gaudi!922)

### Fixed
- Restore use of `GENREFLEX_JOB_POOL` (gaudi/Gaudi#151, gaudi/Gaudi!1150)
- Improve resilience of CI build jobs (gaudi/Gaudi#149, gaudi/Gaudi!1146)
- Fix test checking the wrong thing (gaudi/Gaudi!1142)
- Resolve "test wrapper report logic is the wrong way around" (gaudi/Gaudi#3, gaudi/Gaudi!1132)
- Ignore relative paths when generating the run script (gaudi/Gaudi#147, gaudi/Gaudi!1145)
- Fix QMTest reference filename lookup (gaudi/Gaudi!1143)

## [v34r1][] - 2020-10-16
This is mostly a bugfix release meant as a checkpoint before the tag of [v35r0][]
(which will introduce the refactoring of the CMake configuration, gaudi/Gaudi!986, gaudi/Gaudi!922).

We anyway have some important changes:
- update of physics constants to match the current release of CLHEP (gaudi/Gaudi!1101)
- a new way of dealing with monitorable quantities like counters and histograms (gaudi/Gaudi!1112, gaudi/Gaudi!1113, gaudi/Gaudi!1129)
- `DataObjectHandleProperty` renamed to `DataHandleProperty` (gaudi/Gaudi!1091)
- allow services to *autoretrieve* tool handles (gaudi/Gaudi!1124)
- new helper class to recursively renounce inputs from all tools of an algorithm or tool (gaudi/Gaudi!1118, gaudi/Gaudi!1130, gaudi/Gaudi!1135)

Here is a summary of the [full list of changes](https://gitlab.cern.ch/gaudi/Gaudi/compare/v34r0...v34r1).

### Changed
- Autoretrieve `AlgTools` in `Services`, check for data deps (gaudi/Gaudi!1124)
- Change `DataObjectHandleProperty` to `DataHandleProperty` (gaudi/Gaudi!1091)
- Update units and constants (gaudi/Gaudi!1101)
- Deprecate `put` with a plain pointer as argument (gaudi/Gaudi!1086)

### Added
- Add helper class to renounce recursively inputs from all tools of an algorithm or tool (gaudi/Gaudi!1118, gaudi/Gaudi!1130, gaudi/Gaudi!1135)
- New monitoring facility for counters and histograms (gaudi/Gaudi!1112, gaudi/Gaudi!1113, gaudi/Gaudi!1129)
- Messages: Refine source field truncation (gaudi/Gaudi!1125)
- DataObjID: Add accessor for class name (gaudi/Gaudi!1126)

### Fixed
- Fixes for new ROOT/cppyy (gaudi/Gaudi!1116)
- Resolve "Use of (private) `ToolHandles` may trigger the configuration of spurious public tools" (#141, !1131)
- Fixes for clang builds (gaudi/Gaudi!1121)
- Add test reference files for AVX2/AVX512 builds (gaudi/Gaudi!1122)
- Avoid output file conflict in `MetaDataSvc` test (gaudi/Gaudi!1123)
- Use Boost filesystem instead of std::fs with clang 10 (gaudi/Gaudi!1120)
- Tweak leak sanitizer suppressions for Cling, XrootD and TStreamerInfo with LCG97a (gaudi/Gaudi!1104)
- IUpdateManagerSvc: do not specify unnecessary template arguments (gaudi/Gaudi!1090)
- Update some python examples to python 3 syntax (gaudi/Gaudi!1088)
- Test for the scheduler hang on alg exceptions (gaudi/Gaudi!979, gaudi/Gaudi!1078, gaudi/Gaudi!1107)
- Check build warnings in GitLab CI  (gaudi/Gaudi!1109)
- Remove deprecation warnings on old `IJobOptionsSvc` (gaudi/Gaudi!1114)
- Explicitly flush standard output in `MessageSvc::finalize()` (gaudi/Gaudi!1117)
- Use `Gaudi/Property.h` instead of `GaudiKernel/Property` (gaudi/Gaudi!1105)
- Algorithm state profiling (gaudi/Gaudi!1072)

## [v34r0][] - 2020-08-04
This is a release meant to introduce a major rewrite of the *Job Options Service*, as described in the merge request gaudi/Gaudi!577, plus, of course, some fixes.

### Changed
- Bump version to v34r0 (gaudi/Gaudi!1096)
- Redesign of job options management (gaudi/Gaudi!577, gaudi/Gaudi#105)
- Re-apply gaudi/Gaudi!1064: Isolate sub-slot data from the parent slot (gaudi/Gaudi!1095)

### Added
- Add CopyInputStream (gaudi/Gaudi!1077, gaudi/Gaudi#102)

### Fixed
- FMT Library Usage Improvement (gaudi/Gaudi!1103)
- Minor fixes for Clang 10 build, adapt BinaryTagUtils to allow compiler version >10 (gaudi/Gaudi!1097)
- Do not tinker with output file names in GaudiMP (gaudi/Gaudi!1098)
- Add support for special LCG versions (gaudi/Gaudi!1099)
- Fix Python 3 compatibility issues (gaudi/Gaudi!1102, b87519600879cde3be7694d9fd7567c6efd8db1c)
- Protect messages in event loop (gaudi/Gaudi!1100)

## [v33r2][] - 2020-06-19
This is a bugfix release with just a couple of minor backward compatible improvements.

We also updated the LCG baseline version to [LCG 97a](http://lcginfo.cern.ch/release/97a/) (ROOT 6.20/06).

This is the last release with the legacy CMake configuration. From the next release (v34r0)
we will use a modern CMake configuration (see gaudi/Gaudi!922 and gaudi/Gaudi!986 for details).

### Added
- Improve event context extension management (gaudi/Gaudi!1080)
- Introducing Set-semantics (gaudi/Gaudi!1066)
- Make gitlab use cpp syntax highlighting for icpp files (gaudi/Gaudi!1057)

### Fixed
- Fixes for ROOT master (gaudi/Gaudi!1071)
- Use fmtlib instead of `boost::format` (where possible) (gaudi/Gaudi!1046, gaudi/Gaudi#112)
- Conditions alg test, with stall debug info (gaudi/Gaudi!1092)
- Make sure the TES is cleaned by `EventLoopMgr` before finalize (gaudi/Gaudi!1082, LHCBPS-1850)
- Fix race condition between tests (gaudi/Gaudi!1074)
- Fix propagation of `StopIteration` exception (gaudi/Gaudi!1093, gaudi/Gaudi#129)
- Update HiveDataBroker to use C++20 ranges if available. (gaudi/Gaudi!1089)
- Fixes for C++20 (gaudi/Gaudi!1085)
- Fix thread-safety problem in GaudiHandle (gaudi/Gaudi!1084)
- Misc. sanitiser improvements (gaudi/Gaudi!1081)
- Make `GaudiHandleArray.__str__` consistent with `GaudiHandle.__str__` (gaudi/Gaudi!1070)
- Fix trivial typo in message (gaudi/Gaudi!1087)
- `GaudiConfig2`: fix unpickling of derived Configurables (gaudi/Gaudi!1076, gaudi/Gaudi#124)
- Remove data flow dumps or make them configurable (gaudi/Gaudi!1075)
- Change `StatusCode::orThrow` so that the return value can be ignored (gaudi/Gaudi!1073, gaudi/Gaudi#118)
- Fix bugs in `ThreadPoolSvc` (gaudi/Gaudi!1068)
- Sequence semantics update (gaudi/Gaudi!1069)
- Remove now unused `ATLAS_GAUDI_V21` and `HAVE_GAUDI_PLUGINSVC` definitions (gaudi/Gaudi!1062)
- Few fixes to `GaudiConfig2` (gaudi/Gaudi!1061)
- Fix compatibility with C++ GSL 3 (gaudi/Gaudi!1063, gaudi/Gaudi#121)
- Consolidate and optimize scheduling of blocking tasks (gaudi/Gaudi!1051)
- Do not run public headers build test on generated headers (gaudi/Gaudi!974)
- Add array `operator<<` into `SerializeSTL.h` (gaudi/Gaudi!1049)
- Dereference instead of `operator*` in `get_from_handle` to enable `Gaudi::Ranges` in `MergingTransformer`s (gaudi/Gaudi!1047)
- Fixes for `GetHostBinaryTag` (gaudi/Gaudi!1058)
- Remove unused capture of `this` in `MultiTransformerFilter` (gaudi/Gaudi!1059)
- Fix deprecated copy constructor warning in `StreamBuffer::ContainedLink` (gaudi/Gaudi!1060)
- `AlgExecStateSvc`: require minimum state size to be 1 (gaudi/Gaudi!1055)
- `IncidentSvc`: improve error reporting (gaudi/Gaudi!1054)
- Remove mention of deprecated `tbb::recursive_mutex` (gaudi/Gaudi!1053)

## [v33r1][] - 2020-03-23
This release contains multiple fixes and some ABI changes, in particular:
- `StatusCode` values not checked now produce a compile time warning.
- some interfaces have been changed to accept `std::string_view` instead of `const std::string&`

By default, Gaudi now uses LCG 97 (ROOT 6.20/02).

### Changed
- Removed `FindXercesC.cmake` now that new versions of Xerces are integrated with CMake (!1048)
- Add `[[nodiscard]]` attribute to `StatusCode` (!763)
- Use local memory pool in EvtStoreSvc (!1026)
- Deprecate use of `AnyDataHandle` (!1029)
- Cleanup some string handling & remove long deprecated functions (!1016)
- confDB2: Use 'PublicToolHandle', 'PrivateToolHandle' and 'ServiceHandle' instead of GaudiHandleBase as cpp_type (!1034)

### Added
- Add options to EvtStoreSvc to restrict registering addresses (#84, !1032)

### Fixed
- Make formatting check more stable and fast (!1050)
- Fix propagation of `GaudiSequencer` `RootInTES` (#94, !990)
- Fix handling of const vector in `DataObjectHandle` for `MergingTransformer` (!1045)
- Minor fixes to build on MacOS (!1044)
- Avoid explicit call to `EnableAutoLoading` for ROOT > 6.18 (!1036)
- Use PyROOT nullptr instead of None (1035)
- Support `DataObjectHandle<Gaudi::NamedRange_<T>>` (!1043)
- AlgExecStateSvc: Remove a fatal message and minor cleanups (!1037)
- Improve memory management in `ParticlePropertySvc` (!1040)
- JobOptionSvc: Fix DUMPFILE option (!1041)
- Resolve "GaudiConfigDB2: Fragile handling of templated components" (#115, !1042)
- Prefer `std::scoped_lock` over `lock_guard` (!1030, !1038)
- Reduce dynamic allocation in `GaudiKernel/Time.h` (!1033)
- Allow legacy algorithms to work with LHCb condition handles (!1031)
- Fix Cling warnings when using `GaudiPython::Helper` struct (!1028)
- Define a module `__spec__` for Configurables (!1027)
- clang requires -ffp-contract=fast with -mfma to actually enable FMA instructions (!1022)
- genconf: Fixing the handling of NAN (!1025)

## [v33r0][] - 2019-12-16
This major release of Gaudi contains additions, improvements and fixes, but most of all it adds
the agreed on copyright statement and license:

> © 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations,
> Apache version 2 license

Among the new features I'd like to highlight:
- new Python Configurables implementation (!721)
- experimental `Gaudi::Accumulators::Histogram<...>` (!1020)
- `StatusCode::{andThen|orElse|orThrow}` chaining functions (!988)

### Changed
- Enable sse 4.2 by default for clang >= 6.0 (!1019)
- Use gcc9 in Gilab-CI builds (!1013)
- Remove begin/endRun methods (!1008)
- Make `HistogramPersistencySvc` a little quieter (!992)
- Reorganization of Counters headers (!1006)
- Add new `StatusCode` category to return *filter passed* states from functional algorithms (!981)
- Use `xenv` 1.0.0 if not available from the system (!984)
- Remove const-incorrect `ToolHandle`s exception for ATLAS (!978)

### Added
- Set Gaudi copyright and license (!1018)
- Add experimental support for histograms as counters (!1020)
- Add detection of strongly connected components in the data flow precedence rules (!998)
- Debug output for a stall expecting conditions data (!994)
- Add PRG validators for detection of unconditional "editing" of data objects (!980)
- Prototype for a renewed Python Configurables implementation (!721)
- Add helpers to chain `StatusCode` dependent executions (!988)
- Update `THistSvc` to support `TEfficiency` (!1011)
- Support bulk increment of `BinomialAccumulator` (!999)

### Fixed
- `genconf`: fix quoting of strings containing quotes (!1023)
- Ignore unicode decode errors in test stdout (!1021)
- Do not use `xenv` from LCG (!1005)
- Properly escape strings with embedded quotes in `Gaudi::Utils::toStream` (!1012)
- Detect ROOT C++ standard from `ROOTConfig.cmake` (!985)
- Adjust clang warning suppression pragmas in `Transformer.h` (!1014)
- Improve scheduler performance with fewer calls to updateState (!870)
- `Gaudi::Functional`: Add possibility to query input/output location by type (!1009, !1015)
- Enable `Gaudi::Functional::Transformer<Result(const EventContext&)>` (!1007)
- More precise message for multiple algorithms having the same output in `HiveDataBroker` (!1003)
- Add flag to turn on/off use of `std::filesystem` (!987)
- `IoComponentMgr`: Only release components during finalize (!1004)
- Allow calling `f(std::string_view)` with a `Gaudi::Property<std::string>` (!1002)
- Specify file encoding when reading (!1000)
- Suppress printout of unused `MsgCounter` instances (!996)
- Do not print `SequencerTimerTool` header in finalize if there are no timing results (!997)
- Support for latest TBB and fixes for C++GSL and Range-v3 (!982)
- Fixes for "defects" detected by Coverity (!939)

## [v32r2][] - 2019-09-27
This is mostly a bugfix release, but there are nonetheless a couple of
important changes:
- now Gaudi can be used with Python 3 (#56)
- the `IAsyncEventProcessor`experimental interface introduced in v32r0 (!878)
  has been replaced with `IQueueingEventProcessor` (!966)
- it's now possible to get type information from Python version of `DataHandle`s

### Changed
- Replace `IAsyncEventProcessor` with `IQueueingEventProcessor` (!966)

### Added
- GaudiKernel `instructionsetLevel`: Add additional AVX512 levels (!954)
- Add Python 3 support (#56, !875, !975, #88, !976)
- Add vecwid256 microarchitecture option to enable `-mprefer-vector-width=256` (!965)
- Add type information to Python representation of `DataHandle`s (!951)

### Fixed
- Backward compatible interface of `IAlgorithm` for `GaudiPython` (#75, !977)
- Misc fixes for `GaudiMP` (#87, #75, !973)
- Remove unneeded change in refactoring of TES handling (#70, !971)
- Various ToolHandleArray bug fixes (!960)
- Functional ranges v3 0.9.x warning suppression (!968)
- Fix clang `-Wpotentially-evaluated-expression` warning (!970)
- Switch from Boost test minimal (deprecated) to single header variant (!948)
- Fixes for MacOS (!956)
- Make `ContainedObject` assignment consistent with copy construction (!967)
- `THistSvc::io_reinit`: Handle case of empty new filename (!959)
- `EventSelector` - Always set context ptr to null on release (!961)
- Minor improvements to sanitizers (!955)
- `ChronoStatSvc` - Clear maps in finalize (!958)
- Minor updates to `Gaudi::Functional` implementation (!952)
- `THistSvc`: Fix crash in MP if root file already exists (!949)
- Fix gcc9 warnings (!962)

## [v32r1][] - 2019-07-18
Bugfix release, with some backward compatible changes, mostly meant to pick up
[LCG 96](http://lcginfo.cern.ch/release/96/) and [ROOT 6.18/00](https://root.cern.ch/content/release-61800).

**Note**: Because of the updated version of Boost in LCG 96, you should use a recent version of CMake (>= 3.14).

### Changed
- Add algorithm name to warning in `HiveDataBroker` (!923)
- Make `Configurable.getGaudiType` a `classmethod` (!929)
- Prefer `xyz_v<T>` over `xyz<T>::value`, and `xyz_t<T>` over `typename xyz<T>::type` (!925)
- Prefer `std::{variant,optional}` over `boost::{variant,optional}` (!926)

### Added
- Specify `JOB_POOL` for `genreflex` custom commands (!933)
- Add an example `MergingTransformer` algorithm (!934)
- Add loop unroll hint macros to `GaudiKernel/Kernel.h` (!931)
- Add `MsgCounter` (!921)
- Introduce a `MergingMultiTransformer` (!901)

### Fixed
- Removed a few unused `.cpp` files (!945)
- Print counters in Gaudi::Algorithm::finalize (!943, !946, !947)
- Make asynceventprocessor tests more stable (!940)
- Small fixes to toolchain (!938)
- Use the `AlgContext` guard constructor without `EventContext` for single thread (!973, #72, #73)
- Hide spurious warning from `clang-8` (!941)
- Check that the `clang-format` command found can be executed (!935)
- Improve scheduler stall debug output (!930)
- Fix `CMake` modules test on special architectures (!927)
- Fix handling of strings with quotes in properties (!919)
- Remove reference to removed `gaudimain.runSerialOld` (!918)
- Restore backward compatibility after !878 (!917)
- Revert !869: *Create a queue for all schedule-able algorithms* (!924)
- Add `GaudiKernel` linkage to Counters unit test (!920)
- Misc. sanitizer fixes (!916)
- Fix `distcc`/`icecc` support (!911)
- Made the `LibLZMA` dependency in `GaudiKernel` optional (!909)
- `GaudiKernel`: check *likely* macros before defining them (!912)
- Fix `IOpaqueAddress` lifetime in `EvtStoreSvc` (!914)

## [v32r0][] - 2019-05-18
This release features many fixes and improvements, but also a few backward incompatible changes:
- removed  `GaudiGSL` package (in preparation for the licensing under Apache 2) (!879)
- change of `IEventProcessor` interface (!877)
- change (const correctness) of custom containers interfaces (!896)
- removed a few unused components: `HLTEventLoopMgr` (!876), `AlgErrorAuditor` (!874)
- drop support for Python *custom event loop* in `gaudirun.py` (!878)

Among the various additions you can find:
- `Counter`-based timers (!787)
- example of an interface for accessing conditions, which require experiment specific implementation (!838)
- a new interface (`Gaudi::Interfaces::IAsyncEventProcessor`) to allow decoupling of
  the main thread from the event processing thread(s) (!878)
- a simplified application steering class (`Gaudi::Application`) (!878)

### Changed
- Update `clang-format` to 8 (!872)
- Factor TES path related code out of `GaudiCommon` (!904)
- More explicit management of `EventContext` in `IEventProcessor` (!877)
- Small clean up in `EventLoopMgr` implementations (!876)
- Remove `GaudiGSL` (!879)
- Do not return non-const pointers to elements of const containers (!896)
- Drop backwards compatibility code (!883, !900)
- `HiveDataBroker`: throw exception in case of same output locations in two different algorithms (!858)
- Delete `AlgErrorAuditor` (!874)
- `PrecedenceSvc`: demote 'Verifying rules' message to debug (!873)
- Replace `boost::string_ref` with `std::string_view` (!859)

### Added
- Add optional extension of `EventProcessor` for asynchronous processing (!878)
- Add example of `ConditionAccessorHolder` implementation for integration in Functional (!838)
- Bits for the cross-experiment study on heterogeneous workflows (!828)
- Generalize `Gaudi::Functional` to support LHCb-style conditions handles (!899)
- Add a synthetic performance stress test for the scheduler (!865)
- New Minimal Event Store: `EvtStoreSvc` (!882)
- Add Counter-based timers (!787)
- Add support for `Gaudi::Algorithm` as base class of `Functional` algorithms (!897)
- Create a queue for all schedule-able algorithms (!869)
- Add support for specific architectures in BINARY_TAG (!853)
- Add `getIfExits` for `DataHandle`s with `AnyDataWrapper` (!854)
- Add support for `std::chrono::duration` in counters (!849)
- Add the possibility to ask for Mode in python data handle property (!845)

### Fixed
- Tweaks to `Gaudi::Functional` (!903)
- `HiveDataBroker`: Improve error message when unknown requested input (!906)
- Avoid copy of `EventContext` in `Gaudi::Utils::AlgContext` (!902)
- Reverse order of tbb `task_scheduler_init` and `global_control` (!895)
- Make `StatusCodeSvc` thread-safe (!885)
- `ThreadPoolSvc`: prefer `std::unique_ptr` (!889)
- Ensure thread termination tools only run in initialized threads (!887)
- Updates for MacOS and clang (!888, !894, !893)
- Enable `std::iterator_traits<vector_of_const_<T>::iterator>` (!855)
- Fix `MessageSvc::reinitialize` (!871)
- Fixes to `EventIDRange` (!860)
- `ApplicationMgr`: Call all state transition for `MessageSvc` and `JobOptionsSvc` (!866)
- Fix tunneling of CF decisions to inactive graph regions in multi-parent configurations (!863)
- Use `[[fallthrough]]` attribute instead of the _fallthrough_ comment (!862)
- Restructure PRG sources and add visitor for Concurrent/Prompt CF inconsistency detection (!861)
- Sort data dependencies in `HiveDataBroker` debug printout (!868)
- Avoid displaying unused Binomial counters (!830)
- Python 2 & 3 compatibility (!832, !852)

## [v29r5][] - 2019-03-06
This is a bugfix release only to fix a warning with the most recent version of
CMake (3.13), see the changes on
[Gitlab](https://gitlab.cern.ch/gaudi/Gaudi/compare/v29r4...v29r5).

## [v31r0][] - 2019-02-11
The main reason for this major release is to the change of the `Algorithm` base
class with a re-entrant `execute` method (#47, !703). For backward compatibility the
base class for algorithms is `Gaudi::Algorithm`, while the name `Algorithm` in
the top level namespace is used for a backward compatible implementation. This
change also required a backward incompatible change in the `IAlgorithm`
interface, so some code may require adaptation.

We also took the occasion to update the version of `clang-format` we use from
3.9 to 7, and to switch Python formatting from `autopep8` to `yapf` (!788).
Because of the differences (fixes and new features) in the versions of
`clang-format`, we also updated the style rules.

As of this version, Gaudi requires C++17 compatible compiler.
C++14 compatibility is not required in merge requests, nor guaranteed on the
_master_ branch.

On top of that, we have the usual collection of minor fixes and improvements.

### Changed
- Use gcc 8 instead of gcc 7, and drop gcc 6.2 in CI jobs (!836)
- Make new counters work with Gaudi::Algorithm (#57, !834, !839)
- Update clang-format version and style rules (!788)
- Make `DataObjectHandle::{put,getOrCreate}` const (!833)
- Simplify IAlgExecStateSvc interface (!775)
- Simplify Algorithm constructor (!766)
- Improve the scheduler's execution flow simulator and fix the intra-event occupancy logger (!819)
- Reentrant Algorithm base class (#47, !703)

### Added
- Introduced `CPUCrunchSvc` (!840)
- Added a boolean parameter to turn on verbose view algorithm status (!835)
- Added `MsgStream` support for counters (!826)
- Message: Add option to print full event ID (!824)
- IoComponentMgr: Add interface to retrieve all filenames of component (!823)
- Add example for `Consumer<void()>` (!797)

### Fixed
- Fix lookup/use of `nosetests` for LCG 95 (!842)
- Check coding conventions in _build_ stage of Gitlab-CI (!848)
- Make `gaudi_add_compile_test` also work for succeeding compilations (!844)
- Fix build of Gaudi with LCG 95rc1 in ATLAS context (!847)
- Explicitly define `StatusCode` constants (!841)
- Fix handling of `EventContext` in `Gaudi::Functional::Transformer` arguments (!837)
- `Extensions` to allow `EventIDRanges` limited by both time-stamps and run/LB pairs (!831)
- Improve ROOT dictionary dependency computation (!825)
- Fix issues in preparation for LCG 95 (!820)
- Improve QMT test wrapper (!812)
- Improvements for tests (!812, !817)
- Fixes/workrounds for thread-safety (!815, !829)
- Code clean up (!808, !811, !814, !809, !827, !843)
- Fixes to thread local initialization (!816, !807)


## [v30r5][] - 2018-11-15
This release is mainly to revert !462 (Reimplemenation of `DataHandle`), which
caused problems in ATLAS code.

**Supersedes [v30r4](https://gitlab.cern.ch/gaudi/Gaudi/tags/v30r4)**

### Changed
- refer error in databroker when multiple algorithms declare the same output
  and remove some error messages (!799)
- Remove redundant code from `GaudiKernel/StdArrayAsProperty.h` (!790)
- Use generic `boost::callable_traits` over hand-rolled solution (!785)
- Revert "Reimplementation of `DataHandle` (!462)" (!794)

### Added
- Added printing of Inputs/Outputs for `HiveDataBroker` in debug mode (!801)
- Support Boost >= 1.67 Python library name convention (!743)

### Fixed
- Reduce dependency on LHCb env tools in CI jobs (!805)
- Make sure algorithms managed by `AlgorithmManager` are correctly initialized and started (!804)
- `test_LBCORE_716`: Use the default linker and not the system linker (!783)
- `CountersUnitTest`: Fix C++14 compiler warning (!795)


## [v30r4][] - 2018-11-02 [YANKED]
Improvements, modernization and fixes.

### Changed
- Reorganization of Gaudi::Parsers code (!759)
- Update LCG to 94 and Gaudi version to v30r4 (!776)
- Update minimum required CMake version to 3.6 (!789)
- Remove m_event_context from `Algorithm` (!784)
- Reimplementation of `DataHandle` (!462)
- Improve extension API in `EventContext` (!770)
- Change return type of interger counters operations, like _mean_, to float (!786, #50)
- Clean up `IDataProviderSvc` and `IDataManagerSvc` interfaces (!674)
- Avoid printing empty counters (!777)
- Use `unique_ptr` for `EventContexts` (!732, #37)
- `ConcurrencyFlags`: move setting from `AvalancheScheduler` to `HiveWhiteBoard` (!726)
- Introduce a generic `IDataStoreAgent` implementation (!661)
- `MetaDataSvc`: save data for all clients of the `JobOptionsSvc` (!720)
- Adding actual type of object in data store to error messages (!715)

### Added
- Add `Gaudi::Property` constructors which accept an update handler (!769)
- Optionally capture and print exception backtrace (!653)
- Add build and runtime support for gcc/clang sanitizers (!744)
- Add debug builds to Gitlab-CI and other improvements to CI configuration (!756)
- Introduce `IDataBroker` interface, and `HiveDataBroker` implementation (!685, !739)
- Added an accumulate helper function to new counters (!717)

### Fixed
- `HiveWhiteBoard::freeStore`: check whether slot was already free (!779, #49)
- Fix segfault when scheduling `EventViews` (!781)
- Fixed bug in `BinomialCounter` when using buffer (!764)
- Clarify scalar `operator()` signature in `MultiScalarTransform` (!755)
- Fix potential race condition in `TimelineSvc` (!750)
- Implicitly convert `DataObjectHandleBase` to `str` properties (!773)
- Filter out install and build files from `make apply-formatting` (!747, )
- Fixed issues reported by gcc/clang sanitizers (!749, !757, !758, !752, !751, !746, !772, !753)
- Make sure a test fails if its reference file is not found (!734)
- Fix Floating Point Exception in `counters.h` (!760)
- Make the CMake tests more resilient to symlinks (!742)
- Add support for versioned ROOT libraries (!737)
- Make event views bookkeeping thread-safe (!729, #36)
- Avoid using a (possibly invalid) `EventContext` pointer (!727)
- Mark dependencies as required when the build requires them (!731)
- Use std::filesystem only with C++17 (!738)
- Fixed indentation of new counters to be backward compatible (!717)
- Fixed error handling of `qmt` parsing errors (!714)
- Stop profiling on finalize if needed (!713)
- `THistSvc`: fix memory corruption in `deReg(string)` (!728)
- Avoid possible memory corruption in `PluginService` (!733)
- Added correct `vector_of_const_::operator[]` and ::at (!710)

### Clean up
- Make `VectorMap` `nothrow_move_constructible` (!791)
- Fix some gcc 8 warnings (!782)
- Remove unused mutex member in `Algorithm` (!767)
- Replaced invalid `std::forward` with `std::move` in `PluginService` (!745)
- Counters: improve doxygen (!754)
- Minor code clean up (!408, !740, !774)


## [v30r3][] - 2018-08-01
Improvements, modernization and fixes.

### Deprecated
- Print warning if begin/endRun are used in serial jobs (!666)
- Fully deprecate `DeclareFactoryEntries.h` (!648)

### Changed
- Implement error detection for Hive scheduler/eventloopmgr (!706)
- Modernize memory management in `PrecedenceRulesGraph` (!684)
- Simplify `HLTEventLoopMgr` (!686)
- Rewrite of `PluginService` in modern C++ (!709)
- Sort uids map before printing the content at finalize (in DEBUG) (!702)
- Remove `NTUPLEINFO` (!688)
- Revert `IRegsitry` modification from !604 (!704)
- Move to event-wise stall detection (!690)
- Avoid superfluous visitor-to-scheduler handovers of in-view DR algorithms (!689)
- Add set-to-set lookups for faster `AlgsExecutionStates` matching (!687)
- `FileMgr`: replace integer with bitset (!667)
- Enable top-level 3T dumps for event views (!675)
- Add support for multiple CF parents to 3T (!672)
- Applied new formatting (see !651, !448)
- Remove `DataHandle::commit()` (!625)
- Modernize Registry usage in (Ts)DataSvc (!604, #5)
- Improve Gitlab-CI configuration (!651, !448)
- Implemented a new property for IoComponentMgr to hold search patterns for
  direct I/O input file names (!646)
- Allow retrieval of const services, lost with !526 (!647)
- Add renamePFN and deletePFN methods to FileCatalog (!645)
- Do string conversion in stream operator for `AlgsExecutionStates::State`
  (!643)
- Use `apply` in `Gaudi::Functional` (!622)

### Fixed
- Fixes for clang build (!724)
- Fix dependency of generated PCM files (!705)
- Fix tests in special configurations (!701)
- Fix asynchronous task scheduling (!697)
- Make sure we do not use `ninja` from LCG (!695)
- Increase timeout for event view tests (!696)
- Fix message service format in `GaudiMP` (!694)
- `CommonAuditor`: only print `CustomEventType` deprecation warning if the property is used (!691)
- Fix for crash when events are deleted while view algorithms are still running (!670)
- Fix environment for Gitlab-CI
- Unset `GIT_DIR`/`GIT_WORK_TREE` before cloning `xenv` (!669)
- Add test and fix of version propagation in KeyedContainer move (!663, #27)
- Fixed the list of files used for formatting check in Gitlab-CI (!664)
- Avoid (false positive) clang warning (!656)
- Handle incompatible values for DataObjectHandleBase props (!650)
- Fixed bug in THistSvc introduced with !594 (!652)

### Added
- Add support for `EventContext` as input to `Gaudi::Functional` algorithms (!671)
- Add `operator()` to `Property` (!679)
- Add producer for `KeyedContainer` and vector to `GaudiExamples` (!673)
- Implementation of new counters classes (!629, !676, !677)
- [Change log](CHANGELOG.md) (!649)

### Clean up
- Simplify `Property` forwarding functions (!683)
- Simplify counter implementation (!682)
- Update C++17 / lib fundamentals v2 implementations (!681)
- Modernize `GaudiPython` (!680)
- Modernize `RootCnv` (!660)
- Modernize `RootHistCnv` (!659)
- Protect logging and clean up headers in HiveSlimEventLoopMgr (!692)
- Remove unused variables in Algorithm (!668)
- Remove unreferenced code (!658)


## [v28r2p1][] - 2018-06-15
Bugfix release

### Fixed
- Add test and fix of version propagation in KeyedContainer move (!665, #27)

### Added
- Gitlab-CI configuration (!665, backport from master)


## [v29r4][] - 2018-04-17
Bugfix release.

### Fixed
- Add test and fix of version propagation in KeyedContainer move (!662, #27)
- Handle incompatible values for DataObjectHandleBase props (!654)

### Changed
- Improve Gitlab-CI configuration (!655, backport of !651)


## Old versions
Details about old versions of the project can be found in the
[ReleaseNotes](ReleaseNotes) folder and in the `release.notes` files in
[GaudiRelease/doc](GaudiRelease/doc).


[Unreleased]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v35r3...master
[v35r3]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v35r3
[v35r2]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v35r2
[v35r1]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v35r1
[v35r0]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v35r0
[v34r1]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v34r1
[v34r0]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v34r0
[v33r2]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v33r2
[v33r1]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v33r1
[v33r0]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v33r0
[v32r2]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v32r2
[v32r1]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v32r1
[v32r0]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v32r0
[v31r0]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v31r0
[v30r5]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v30r5
[v30r4]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v30r4
[v30r3]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v30r3
[v29r5]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v29r5
[v29r4]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v29r4
[v28r2p1]: https://gitlab.cern.ch/gaudi/Gaudi/-/releases/v28r2p1
