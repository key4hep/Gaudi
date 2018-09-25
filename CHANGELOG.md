# Changelog
All notable changes to this project will be documented in this file.

Project Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased][]

### Changed
- Use `unique_ptr` for `EventContexts` ([mr !732][], [issue #37][])
- `ConcurrencyFlags`: move setting from `AvalancheScheduler` to `HiveWhiteBoard` ([mr !726][])
- Introduce a generic `IDataStoreAgent` implementation ([mr !661][])
- `MetaDataSvc`: save data for all clients of the `JobOptionsSvc` ([mr !720][])
- Adding actual type of object in data store to error messages ([mr !715][])

### Added
- Add build and runtime support for gcc/clang sanitizers ([mr !744][])
- Add debug builds to Gitlab-CI ([mr !756][])
- Introduce `IDataBroker` interface, and `HiveDataBroker` implementation ([mr !685][], [mr !739][])
- Added an accumulate helper function to new counters ([mr !717][])

### Fixed
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

### Clean up
- Counters: improve doxygen ([mr !754][])
- Minor code clean up ([mr !408][])


## [v30r3][] - 2018-08-01
Improvements, modernization and fixes.

### Deprecated
- Print warning if begin/endRun are used in serial jobs ([mr !666][])
- Fully deprecate `DeclareFactoryEntries.h` ([mr !648][])

### Changed
- Implement error detection for Hive scheduler/eventloopmgr ([mr !706][])
- Modernize memory management in `PrecedenceRulesGraph` ([mr !684][])
- Simplify `HLTEventLoopMgr` ([mr !686][])
- Rewrite of `PluginService` in modern C++ ([mr !709][])
- Sort uids map before printing the content at finalize (in DEBUG) ([mr !702][])
- Remove `NTUPLEINFO` ([mr !688][])
- Revert `IRegsitry` modification from [mr !604][] ([mr !704][])
- Move to event-wise stall detection ([mr !690][])
- Avoid superfluous visitor-to-scheduler handovers of in-view DR algorithms ([mr !689][])
- Add set-to-set lookups for faster `AlgsExecutionStates` matching ([mr !687][])
- `FileMgr`: replace integer with bitset ([mr !667][])
- Enable top-level 3T dumps for event views ([mr !675][])
- Add support for multiple CF parents to 3T ([mr !672][])
- Applied new formatting (see [mr !651][], [mr !448][])
- Remove `DataHandle::commit()` ([mr !625][])
- Modernize Registry usage in (Ts)DataSvc ([mr !604][], [issue #5][])
- Improve Gitlab-CI configuration ([mr !651][], [mr !448][])
- Implemented a new property for IoComponentMgr to hold search patterns for
  direct I/O input file names ([mr !646][])
- Allow retrieval of const services, lost with [mr !526][] ([mr !647][])
- Add renamePFN and deletePFN methods to FileCatalog ([mr !645][])
- Do string conversion in stream operator for `AlgsExecutionStates::State`
  ([mr !643][])
- Use `apply` in `Gaudi::Functional` ([mr !622][])

### Fixed
- Fixes for clang build ([mr !724][])
- Fix dependency of generated PCM files ([mr !705][])
- Fix tests in special configurations ([mr !701][])
- Fix asynchronous task scheduling ([mr !697][])
- Make sure we do not use `ninja` from LCG ([mr !695][])
- Increase timeout for event view tests ([mr !696][])
- Fix message service format in `GaudiMP` ([mr !694][])
- `CommonAuditor`: only print `CustomEventType` deprecation warning if the property is used ([mr !691][])
- Fix for crash when events are deleted while view algorithms are still running ([mr !670][])
- Fix environment for Gitlab-CI
- Unset `GIT_DIR`/`GIT_WORK_TREE` before cloning `xenv` ([mr !669][])
- Add test and fix of version propagation in KeyedContainer move ([mr !663][], [issue #27][])
- Fixed the list of files used for formatting check in Gitlab-CI ([mr !664][])
- Avoid (false positive) clang warning ([mr !656][])
- Handle incompatible values for DataObjectHandleBase props ([mr !650][])
- Fixed bug in THistSvc introduced with [mr !594][] ([mr !652][])

### Added
- Add support for `EventContext` as input to `Gaudi::Functional` algorithms ([mr !671][])
- Add `operator()` to `Property` ([mr !679][])
- Add producer for `KeyedContainer` and vector to `GaudiExamples` ([mr !673][])
- Implementation of new counters classes ([mr !629][], [mr !676][], [mr !677][])
- [Change log](CHANGELOG.md) ([mr !649][])

### Clean up
- Simplify `Property` forwarding functions ([mr !683][])
- Simplify counter implementation ([mr !682][])
- Update C++17 / lib fundamentals v2 implementations ([mr !681][])
- Modernize `GaudiPython` ([mr !680][])
- Modernize `RootCnv` ([mr !660][])
- Modernize `RootHistCnv` ([mr !659][])
- Protect logging and clean up headers in HiveSlimEventLoopMgr ([mr !692][])
- Remove unused variables in Algorithm ([mr !668][])
- Remove unreferenced code ([mr !658][])


## [v28r2p1][] - 2018-06-15
Bugfix release

### Fixed
- Add test and fix of version propagation in KeyedContainer move ([mr !665][], [issue #27][])

### Added
- Gitlab-CI configuration ([mr !665][], backport from master)


## [v29r4][] - 2018-04-17
Bugfix release.

### Fixed
- Add test and fix of version propagation in KeyedContainer move ([mr !662][], [issue #27][])
- Handle incompatible values for DataObjectHandleBase props ([mr !654][])

### Changed
- Improve Gitlab-CI configuration ([mr !655][], backport of [mr !651][])


## Old versions
Details about old versions of the project can be found in the
[ReleaseNotes](ReleaseNotes) folder and in the `release.notes` files in
[GaudiRelease/doc](GaudiRelease/doc).

[mr !760]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/760
[mr !756]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/756
[mr !754]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/754
[mr !744]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/744
[mr !742]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/742
[mr !739]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/739
[mr !738]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/738
[mr !737]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/737
[mr !733]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/733
[mr !732]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/732
[mr !731]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/731
[mr !729]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/729
[mr !728]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/728
[mr !727]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/727
[mr !726]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/726
[mr !724]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/724
[mr !720]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/720
[mr !717]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/717
[mr !715]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/715
[mr !714]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/714
[mr !713]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/713
[mr !710]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/710
[mr !709]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/709
[mr !706]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/706
[mr !705]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/705
[mr !704]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/704
[mr !702]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/702
[mr !701]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/701
[mr !697]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/697
[mr !696]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/696
[mr !695]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/695
[mr !694]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/694
[mr !692]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/692
[mr !691]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/691
[mr !690]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/690
[mr !689]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/689
[mr !688]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/688
[mr !687]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/687
[mr !686]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/686
[mr !685]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/685
[mr !684]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/684
[mr !683]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/683
[mr !682]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/682
[mr !681]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/681
[mr !680]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/680
[mr !679]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/679
[mr !677]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/677
[mr !676]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/676
[mr !675]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/675
[mr !673]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/673
[mr !672]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/672
[mr !671]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/671
[mr !670]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/670
[mr !669]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/669
[mr !668]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/668
[mr !667]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/667
[mr !666]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/666
[mr !665]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/665
[mr !664]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/664
[mr !663]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/663
[mr !662]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/662
[mr !661]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/661
[mr !660]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/660
[mr !659]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/659
[mr !658]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/658
[mr !656]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/656
[mr !655]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/655
[mr !654]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/654
[mr !652]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/652
[mr !651]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/651
[mr !650]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/650
[mr !649]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/649
[mr !648]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/648
[mr !647]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/647
[mr !646]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/646
[mr !645]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/645
[mr !643]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/643
[mr !629]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/629
[mr !625]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/625
[mr !622]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/622
[mr !604]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/604
[mr !594]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/594
[mr !526]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/526
[mr !448]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/448
[mr !408]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/408

[issue #37]: https://gitlab.cern.ch/gaudi/Gaudi/issues/37
[issue #36]: https://gitlab.cern.ch/gaudi/Gaudi/issues/36
[issue #27]: https://gitlab.cern.ch/gaudi/Gaudi/issues/27
[issue #5]: https://gitlab.cern.ch/gaudi/Gaudi/issues/5

[Unreleased]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r3...master
[v30r3]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r2...v30r3
[v29r4]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v29r3...v29r4
[v28r2p1]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v28r2...v28r2p1
