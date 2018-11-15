---
version: v30r5
date: 2018-11-15
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett

# Release Notes
This release is mainly to revert [mr !462](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/462) (Reimplemenation of `DataHandle`), which
caused problems in ATLAS code.

**Seupersede v30r4**

## Changed
- refer error in databroker when multiple algorithms declare the same output
  and remove some error messages ([mr !799](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/799))
- Remove redundant code from `GaudiKernel/StdArrayAsProperty.h` ([mr !790](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/790))
- Use generic `boost::callable_traits` over hand-rolled solution ([mr !785](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/785))
- Revert "Reimplementation of `DataHandle` ([mr !462](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/462))" ([mr !794](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/794))

## Added
- Added printing of Inputs/Outputs for `HiveDataBroker` in debug mode (!801)
- Support Boost >= 1.67 Python library name convention (!743)

## Fixed
- Reduce dependency on LHCb env tools in CI jobs ([mr !805](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/805))
- Make sure algorithms managed by `AlgorithmManager` are correctly initialized and started ([mr !804](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/804))
- `test_LBCORE_716`: Use the default linker and not the system linker ([mr !783](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/783))
- `CountersUnitTest`: Fix C++14 compiler warning ([mr !795](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/795))
