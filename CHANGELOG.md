# Changelog
All notable changes to this project will be documented in this file.

Project Coordinators: Marco Clemencic @clemenci, Charles Leggett @leggett

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased][]

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


[Unreleased]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v31r0...master
[v31r0]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r5...v31r0
[v30r5]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r4...v30r5
[v30r4]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r3...v30r4
[v30r3]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r2...v30r3
[v29r4]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v29r3...v29r4
[v28r2p1]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v28r2...v28r2p1
