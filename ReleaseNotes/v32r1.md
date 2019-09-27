---
version: v32r1
date: 2019-07-18
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Stefan Roiser

# Release Notes
Bugfix release, with some backward compatible changes, mostly meant to pick up
[LCG 96](http://lcginfo.cern.ch/release/96/) and [ROOT 6.18/00](https://root.cern.ch/content/release-61800).

**Note**: Because of the updated version of Boost in LCG 96, you should use a recent version of CMake (>= 3.14).

### Changed
- Add algorithm name to warning in `HiveDataBroker` ([gaudi/Gaudi!923](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/923))
- Make `Configurable.getGaudiType` a `classmethod` ([gaudi/Gaudi!929](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/929))
- Prefer `xyz_v<T>` over `xyz<T>::value`, and `xyz_t<T>` over `typename xyz<T>::type` ([gaudi/Gaudi!925](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/925))
- Prefer `std::{variant,optional}` over `boost::{variant,optional}` ([gaudi/Gaudi!926](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/926))

### Added
- Specify `JOB_POOL` for `genreflex` custom commands ([gaudi/Gaudi!933](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/933))
- Add an example `MergingTransformer` algorithm ([gaudi/Gaudi!934](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/934))
- Add loop unroll hint macros to `GaudiKernel/Kernel.h` ([gaudi/Gaudi!931](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/931))
- Add `MsgCounter` ([gaudi/Gaudi!921](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/921))
- Introduce a `MergingMultiTransformer` ([gaudi/Gaudi!901](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/901))

### Fixed
- Removed a few unused `.cpp` files ([gaudi/Gaudi!945](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/945))
- Print counters in Gaudi::Algorithm::finalize ([gaudi/Gaudi!943](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/943), [gaudi/Gaudi!946](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/946), [gaudi/Gaudi!947](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/947))
- Make asynceventprocessor tests more stable ([gaudi/Gaudi!940](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/940))
- Small fixes to toolchain ([gaudi/Gaudi!938](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/938))
- Use the `AlgContext` guard constructor without `EventContext` for single thread ([gaudi/Gaudi!973](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/973), [gaudi/Gaudi#72](https://gitlab.cern.ch/gaudi/Gaudi/issues/72), [gaudi/Gaudi#73](https://gitlab.cern.ch/gaudi/Gaudi/issues/73))
- Hide spurious warning from `clang-8` ([gaudi/Gaudi!941](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/941))
- Check that the `clang-format` command found can be executed ([gaudi/Gaudi!935](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/935))
- Improve scheduler stall debug output ([gaudi/Gaudi!930](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/930))
- Fix `CMake` modules test on special architectures ([gaudi/Gaudi!927](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/927))
- Fix handling of strings with quotes in properties ([gaudi/Gaudi!919](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/919))
- Remove reference to removed `gaudimain.runSerialOld` ([gaudi/Gaudi!918](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/918))
- Restore backward compatibility after [gaudi/Gaudi!878](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/878) ([gaudi/Gaudi!917](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/917))
- Revert [gaudi/Gaudi!869](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/869): *Create a queue for all schedule-able algorithms* ([gaudi/Gaudi!924](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/924))
- Add `GaudiKernel` linkage to Counters unit test ([gaudi/Gaudi!920](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/920))
- Misc. sanitizer fixes ([gaudi/Gaudi!916](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/916))
- Fix `distcc`/`icecc` support ([gaudi/Gaudi!911](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/911))
- Made the `LibLZMA` dependency in `GaudiKernel` optional ([gaudi/Gaudi!909](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/909))
- `GaudiKernel`: check *likely* macros before defining them ([gaudi/Gaudi!912](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/912))
- Fix `IOpaqueAddress` lifetime in `EvtStoreSvc` ([gaudi/Gaudi!914](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/914))
