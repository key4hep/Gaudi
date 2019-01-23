---
version: v30r4
date: 2018-11-02
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett

# Release Notes
Improvements, modernization and fixes (see [all changes][]).

## Changed
- Reorganization of Gaudi::Parsers code ([mr !759][])
- Update LCG to 94 and Gaudi version to v30r4 ([mr !776][])
- Update minimum required CMake version to 3.6 ([mr !789][])
- Remove m_event_context from `Algorithm` ([mr !784][])
- Reimplementation of DataHandle ([mr !462][])
- Improve extension API in `EventContext` ([mr !770][])
- Change return type of interger counters operations, like _mean_, to float ([mr !786][], [issue #50][])
- Clean up `IDataProviderSvc` and `IDataManagerSvc` interfaces ([mr !674][])
- Avoid printing empty counters ([mr !777][])
- Use `unique_ptr` for `EventContexts` ([mr !732][], [issue #37][])
- `ConcurrencyFlags`: move setting from `AvalancheScheduler` to `HiveWhiteBoard` ([mr !726][])
- Introduce a generic `IDataStoreAgent` implementation ([mr !661][])
- `MetaDataSvc`: save data for all clients of the `JobOptionsSvc` ([mr !720][])
- Adding actual type of object in data store to error messages ([mr !715][])

## Added
- Add `Gaudi::Property` constructors which accept an update handler ([mr !769][])
- Optionally capture and print exception backtrace ([mr !653][])
- Add build and runtime support for gcc/clang sanitizers ([mr !744][])
- Add debug builds to Gitlab-CI and other improvements to CI configuration ([mr !756][])
- Introduce `IDataBroker` interface, and `HiveDataBroker` implementation ([mr !685][], [mr !739][])
- Added an accumulate helper function to new counters ([mr !717][])

## Fixed
- `HiveWhiteBoard::freeStore`: check whether slot was already free ([mr !779][], [issue #49][])
- Fix segfault when scheduling `EventViews` ([mr !781][])
- Fixed bug in `BinomialCounter` when using buffer ([mr !764][])
- Clarify scalar `operator()` signature in `MultiScalarTransform` ([mr !755][])
- Fix potential race condition in `TimelineSvc` ([mr !750][])
- Implicitly convert `DataObjectHandleBase` to `str` properties ([mr !773][])
- Filter out install and build files from `make apply-formatting` ([mr !747][], )
- Fixed issues reported by gcc/clang sanitizers ([mr !749][], [mr !757][], [mr !758][], [mr !752][], [mr !751][], [mr !746][], [mr !772][], [mr !753][])
- Make sure a test fails if its reference file is not found ([mr !734][])
- Fix Floating Point Exception in `counters.h` ([mr !760][])
- Make the CMake tests more resilient to symlinks ([mr !742][])
- Add support for versioned ROOT libraries ([mr !737][])
- Make event views bookkeeping thread-safe ([mr !729][], [issue #36][])
- Avoid using a (possibly invalid) `EventContext` pointer ([mr !727][])
- Mark dependencies as required when the build requires them ([mr !731][])
- Use std::filesystem only with C++17 ([mr !738][])
- Fixed indentation of new counters to be backward compatible ([mr !717][])
- Fixed error handling of `qmt` parsing errors ([mr !714][])
- Stop profiling on finalize if needed ([mr !713][])
- `THistSvc`: fix memory corruption in `deReg(string)` ([mr !728][])
- Avoid possible memory corruption in `PluginService` ([mr !733][])
- Added correct `vector_of_const_::operator[]` and ::at ([mr !710][])

## Clean up
- Make `VectorMap` `nothrow_move_constructible` ([mr !791][])
- Fix some gcc 8 warnings ([mr !782][])
- Remove unused mutex member in `Algorithm` ([mr !767][])
- Replaced invalid `std::forward` with `std::move` in `PluginService` ([mr !745][])
- Counters: improve doxygen ([mr !754][])
- Minor code clean up ([mr !408][], [mr !740][], [mr !774][])


[mr !791]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/791
[mr !789]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/789
[mr !786]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/786
[mr !784]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/784
[mr !782]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/782
[mr !781]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/781
[mr !779]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/779
[mr !777]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/777
[mr !776]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/776
[mr !774]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/774
[mr !773]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/773
[mr !772]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/772
[mr !770]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/770
[mr !769]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/769
[mr !767]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/767
[mr !764]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/764
[mr !760]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/760
[mr !759]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/759
[mr !758]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/758
[mr !757]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/757
[mr !756]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/756
[mr !755]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/755
[mr !754]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/754
[mr !753]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/753
[mr !752]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/752
[mr !751]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/751
[mr !750]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/750
[mr !749]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/749
[mr !747]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/747
[mr !746]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/746
[mr !745]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/745
[mr !744]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/744
[mr !742]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/742
[mr !740]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/740
[mr !739]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/739
[mr !738]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/738
[mr !737]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/737
[mr !734]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/734
[mr !733]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/733
[mr !732]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/732
[mr !731]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/731
[mr !729]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/729
[mr !728]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/728
[mr !727]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/727
[mr !726]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/726
[mr !720]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/720
[mr !717]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/717
[mr !715]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/715
[mr !714]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/714
[mr !713]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/713
[mr !710]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/710
[mr !685]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/685
[mr !674]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/674
[mr !661]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/661
[mr !653]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/653
[mr !462]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/462
[mr !408]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/408

[issue #50]: https://gitlab.cern.ch/gaudi/Gaudi/issues/50
[issue #49]: https://gitlab.cern.ch/gaudi/Gaudi/issues/49
[issue #37]: https://gitlab.cern.ch/gaudi/Gaudi/issues/37
[issue #36]: https://gitlab.cern.ch/gaudi/Gaudi/issues/36

[all changes]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r3...v30r4
