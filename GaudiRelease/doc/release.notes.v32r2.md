---
version: v32r2
date: 2019-09-27
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Stefan Roiser

# Release Notes
This is mostly a bugfix release, but there are nonetheless a couple of
important changes:
- now Gaudi can be used with Python 3 ([gaudi/Gaudi#56](https://gitlab.cern.ch/gaudi/Gaudi/issues/56))
- the `IAsyncEventProcessor`experimental interface introduced in v32r0 ([gaudi/Gaudi!878](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/878))
  has been replaced with `IQueueingEventProcessor` ([gaudi/Gaudi!966](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/966))
- it's now possible to get type information from Python version of `DataHandle`s

### Changed
- Replace `IAsyncEventProcessor` with `IQueueingEventProcessor` ([gaudi/Gaudi!966](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/966))

### Added
- GaudiKernel `instructionsetLevel`: Add additional AVX512 levels ([gaudi/Gaudi!954](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/954))
- Add Python 3 support ([gaudi/Gaudi#56](https://gitlab.cern.ch/gaudi/Gaudi/issues/56), [gaudi/Gaudi!875](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/875), [gaudi/Gaudi!975](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/975), [gaudi/Gaudi#88](https://gitlab.cern.ch/gaudi/Gaudi/issues/88), [gaudi/Gaudi!976](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/976))
- Add vecwid256 microarchitecture option to enable `-mprefer-vector-width=256` ([gaudi/Gaudi!965](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/965))
- Add type information to Python representation of `DataHandle`s ([gaudi/Gaudi!951](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/951))

### Fixed
- Backward compatible interface of `IAlgorithm` for `GaudiPython` ([gaudi/Gaudi#75](https://gitlab.cern.ch/gaudi/Gaudi/issues/75), [gaudi/Gaudi!977](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/977))
- Misc fixes for `GaudiMP` ([gaudi/Gaudi#87](https://gitlab.cern.ch/gaudi/Gaudi/issues/87), [gaudi/Gaudi#75](https://gitlab.cern.ch/gaudi/Gaudi/issues/75), [gaudi/Gaudi!973](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/973))
- Remove unneeded change in refactoring of TES handling ([gaudi/Gaudi#70](https://gitlab.cern.ch/gaudi/Gaudi/issues/70), [gaudi/Gaudi!971](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/971))
- Various ToolHandleArray bug fixes ([gaudi/Gaudi!960](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/960))
- Functional ranges v3 0.9.x warning suppression ([gaudi/Gaudi!968](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/968))
- Fix clang `-Wpotentially-evaluated-expression` warning ([gaudi/Gaudi!970](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/970))
- Switch from Boost test minimal (deprecated) to single header variant ([gaudi/Gaudi!948](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/948))
- Fixes for MacOS ([gaudi/Gaudi!956](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/956))
- Make `ContainedObject` assignment consistent with copy construction ([gaudi/Gaudi!967](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/967))
- `THistSvc::io_reinit`: Handle case of empty new filename ([gaudi/Gaudi!959](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/959))
- `EventSelector` - Always set context ptr to null on release ([gaudi/Gaudi!961](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/961))
- Minor improvements to sanitizers ([gaudi/Gaudi!955](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/955))
- `ChronoStatSvc` - Clear maps in finalize ([gaudi/Gaudi!958](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/958))
- Minor updates to `Gaudi::Functional` implementation ([gaudi/Gaudi!952](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/952))
- `THistSvc`: Fix crash in MP if root file already exists ([gaudi/Gaudi!949](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/949))
- Fix gcc9 warnings ([gaudi/Gaudi!962](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/962))
