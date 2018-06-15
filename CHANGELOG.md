# Changelog
All notable changes to this project will be documented in this file.

Project Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased][]
### Deprecated
- Print warning if begin/endRun are used in serial jobs ([mr !666][])
- Fully deprecate `DeclareFactoryEntries.h` ([mr !648][])

### Changed
- FileMgr: replace integer with bitset ([mr !667][])
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

[mr !696]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/696
[mr !695]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/695
[mr !694]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/694
[mr !692]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/692
[mr !691]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/691
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

[issue #27]: https://gitlab.cern.ch/gaudi/Gaudi/issues/27
[issue #5]: https://gitlab.cern.ch/gaudi/Gaudi/issues/5

[Unreleased]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r2...master
[v29r4]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v29r3...v29r4
[v28r2p1]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v28r2...v28r2p1
