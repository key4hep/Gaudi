# Changelog
All notable changes to this project will be documented in this file.

Project Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased][]
### Deprecated
- Fully deprecate `DeclareFactoryEntries.h` ([mr !648][])

### Changed
- Applied new formatting (see [mr !651][], [mr !448][])
- Remove unreferenced code ([mr !658][])
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
- Fixed the list of files used for formatting check in Gitlab-CI ([mr !664][])
- Avoid (false positive) clang warning ([mr !656][])
- Handle incompatible values for DataObjectHandleBase props ([mr !650][])
- Fixed bug in THistSvc introduced with [mr !594][] ([mr !652][])

### Added
- [Change log](CHANGELOG.md) ([mr !649][])


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

[mr !664]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/664
[mr !662]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/662
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
