# Changelog
All notable changes to this project will be documented in this file.

Project Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased][]
### Deprecated
- Fully deprecate `DeclareFactoryEntries.h` ([mr !648][])

### Changed
- Implemented a new property for IoComponentMgr to hold search patterns for
  direct I/O input file names ([mr !646][])
- Allow retrieval of const services, lost with [mr !526][] ([mr !647][])
- Add renamePFN and deletePFN methods to FileCatalog ([mr !645][])
- Do string conversion in stream operator for `AlgsExecutionStates::State`
  ([mr !643][])
- Use `apply` in `Gaudi::Functional` ([mr !622][])

### Added
- [Change log](CHANGELOG.md)

## Old versions
Details about old versions of the project can be found in the
[ReleaseNotes](ReleaseNotes) folder and in the `release.notes` files in
[GaudiRelease/doc](GaudiRelease/doc).

[mr !648]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/648
[mr !647]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/647
[mr !646]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/646
[mr !645]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/645
[mr !643]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/643
[mr !622]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/622
[mr !526]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/526

[Unreleased]: https://gitlab.cern.ch/gaudi/Gaudi/compare/v30r2...master
