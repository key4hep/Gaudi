---
version: v31r0
date: 2019-02-11
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett

# Release Notes
The main reason for this major release is to the change of the `Algorithm` base
class with a re-entrant `execute` method ([#47](https://gitlab.cern.ch/gaudi/Gaudi/issues/47), [!703](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/703)). For backward compatibility the
base class for algorithms is `Gaudi::Algorithm`, while the name `Algorithm` in
the top level namespace is used for a backward compatible implementation. This
change also required a backward incompatible change in the `IAlgorithm`
interface, so some code may require adaptation.

We also took the occasion to update the version of `clang-format` we use from
3.9 to 7, and to switch Python formatting from `autopep8` to `yapf` ([!788](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/788)).
Because of the differences (fixes and new features) in the versions of
`clang-format`, we also updated the style rules.

As of this version, Gaudi requires C++17 compatible compiler.
C++14 compatibility is not required in merge requests, nor guaranteed on the
_master_ branch.

On top of that, we have the usual collection of minor fixes and improvements.

### Changed
- Use gcc 8 instead of gcc 7, and drop gcc 6.2 in CI jobs ([!836](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/836))
- Make new counters work with Gaudi::Algorithm ([#57](https://gitlab.cern.ch/gaudi/Gaudi/issues/57), [!834](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/834), [!839](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/839))
- Update clang-format version and style rules ([!788](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/788))
- Make `DataObjectHandle::{put,getOrCreate}` const ([!833](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/833))
- Simplify IAlgExecStateSvc interface ([!775](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/775))
- Simplify Algorithm constructor ([!766](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/766))
- Improve the scheduler's execution flow simulator and fix the intra-event occupancy logger ([!819](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/819))
- Reentrant Algorithm base class ([#47](https://gitlab.cern.ch/gaudi/Gaudi/issues/47), [!703](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/703))

### Added
- Introduced `CPUCrunchSvc` ([!840](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/840))
- Added a boolean parameter to turn on verbose view algorithm status ([!835](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/835))
- Added `MsgStream` support for counters ([!826](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/826))
- Message: Add option to print full event ID ([!824](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/824))
- IoComponentMgr: Add interface to retrieve all filenames of component ([!823](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/823))
- Add example for `Consumer<void()>` ([!797](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/797))

### Fixed
- Fix lookup/use of `nosetests` for LCG 95 ([!842](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/842))
- Check coding conventions in _build_ stage of Gitlab-CI ([!848](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/848))
- Make `gaudi_add_compile_test` also work for succeeding compilations ([!844](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/844))
- Fix build of Gaudi with LCG 95rc1 in ATLAS context ([!847](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/847))
- Explicitly define `StatusCode` constants ([!841](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/841))
- Fix handling of `EventContext` in `Gaudi::Functional::Transformer` arguments ([!837](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/837))
- `Extensions` to allow `EventIDRanges` limited by both time-stamps and run/LB pairs ([!831](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/831))
- Improve ROOT dictionary dependency computation ([!825](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/825))
- Fix issues in preparation for LCG 95 ([!820](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/820))
- Improve QMT test wrapper ([!812](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/812))
- Improvements for tests ([!812](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/812), [!817](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/817))
- Fixes/workrounds for thread-safety ([!815](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/815), [!829](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/829))
- Code clean up ([!808](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/808), [!811](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/811), [!814](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/814), [!809](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/809), [!827](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/827), [!843](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/843))
- Fixes to thread local initialization ([!816](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/816), [!807](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/807))
