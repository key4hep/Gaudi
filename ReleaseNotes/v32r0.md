---
version: v32r0
date: 2019-05-18
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Stefan Roiser

# Release Notes
This release features many fixes and improvements, but also a few backward incompatible changes:
- removed  `GaudiGSL` package (in preparation for the licensing under Apache 2) ([gaudi/Gaudi!879](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/879))
- change of `IEventProcessor` interface ([gaudi/Gaudi!877](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/877))
- change (const correctness) of custom containers interfaces ([gaudi/Gaudi!896](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/896))
- removed a few unused components: `HLTEventLoopMgr` ([gaudi/Gaudi!876](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/876)), `AlgErrorAuditor` ([gaudi/Gaudi!874](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/874))
- drop support for Python *custom event loop* in `gaudirun.py` ([gaudi/Gaudi!878](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/878))

Among the various additions you can find:
- `Counter`-based timers ([gaudi/Gaudi!787](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/787))
- example of an interface for accessing conditions, which require experiment specific implementation ([gaudi/Gaudi!838](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/838))
- a new interface (`Gaudi::Interfaces::IAsyncEventProcessor`) to allow decoupling of
  the main thread from the event processing thread(s) ([gaudi/Gaudi!878](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/878))
- a simplified application steering class (`Gaudi::Application`) ([gaudi/Gaudi!878](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/878))

### Changed
- Update `clang-format` to 8 ([gaudi/Gaudi!872](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/872))
- Factor TES path related code out of `GaudiCommon` ([gaudi/Gaudi!904](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/904))
- More explicit management of `EventContext` in `IEventProcessor` ([gaudi/Gaudi!877](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/877))
- Small clean up in `EventLoopMgr` implementations ([gaudi/Gaudi!876](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/876))
- Remove `GaudiGSL` ([gaudi/Gaudi!879](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/879))
- Do not return non-const pointers to elements of const containers ([gaudi/Gaudi!896](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/896))
- Drop backwards compatibility code ([gaudi/Gaudi!883](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/883), [gaudi/Gaudi!900](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/900))
- `HiveDataBroker`: throw exception in case of same output locations in two different algorithms ([gaudi/Gaudi!858](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/858))
- Delete `AlgErrorAuditor` ([gaudi/Gaudi!874](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/874))
- `PrecedenceSvc`: demote 'Verifying rules' message to debug ([gaudi/Gaudi!873](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/873))
- Replace `boost::string_ref` with `std::string_view` ([gaudi/Gaudi!859](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/859))

### Added
- Add optional extension of `EventProcessor` for asynchronous processing ([gaudi/Gaudi!878](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/878))
- Add example of `ConditionAccessorHolder` implementation for integration in Functional ([gaudi/Gaudi!838](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/838))
- Bits for the cross-experiment study on heterogeneous workflows ([gaudi/Gaudi!828](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/828))
- Generalize `Gaudi::Functional` to support LHCb-style conditions handles ([gaudi/Gaudi!899](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/899))
- Add a synthetic performance stress test for the scheduler ([gaudi/Gaudi!865](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/865))
- New Minimal Event Store: `EvtStoreSvc` ([gaudi/Gaudi!882](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/882))
- Add Counter-based timers ([gaudi/Gaudi!787](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/787))
- Add support for `Gaudi::Algorithm` as base class of `Functional` algorithms ([gaudi/Gaudi!897](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/897))
- Create a queue for all schedule-able algorithms ([gaudi/Gaudi!869](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/869))
- Add support for specific architectures in BINARY_TAG ([gaudi/Gaudi!853](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/853))
- Add `getIfExits` for `DataHandle`s with `AnyDataWrapper` ([gaudi/Gaudi!854](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/854))
- Add support for `std::chrono::duration` in counters ([gaudi/Gaudi!849](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/849))
- Add the possibility to ask for Mode in python data handle property ([gaudi/Gaudi!845](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/845))

### Fixed
- Tweaks to `Gaudi::Functional` ([gaudi/Gaudi!903](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/903))
- `HiveDataBroker`: Improve error message when unknown requested input ([gaudi/Gaudi!906](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/906))
- Avoid copy of `EventContext` in `Gaudi::Utils::AlgContext` ([gaudi/Gaudi!902](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/902))
- Reverse order of tbb `task_scheduler_init` and `global_control` ([gaudi/Gaudi!895](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/895))
- Make `StatusCodeSvc` thread-safe ([gaudi/Gaudi!885](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/885))
- `ThreadPoolSvc`: prefer `std::unique_ptr` ([gaudi/Gaudi!889](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/889))
- Ensure thread termination tools only run in initialized threads ([gaudi/Gaudi!887](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/887))
- Updates for MacOS and clang ([gaudi/Gaudi!888](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/888), [gaudi/Gaudi!894](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/894), [gaudi/Gaudi!893](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/893))
- Enable `std::iterator_traits<vector_of_const_<T>::iterator>` ([gaudi/Gaudi!855](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/855))
- Fix `MessageSvc::reinitialize` ([gaudi/Gaudi!871](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/871))
- Fixes to `EventIDRange` ([gaudi/Gaudi!860](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/860))
- `ApplicationMgr`: Call all state transition for `MessageSvc` and `JobOptionsSvc` ([gaudi/Gaudi!866](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/866))
- Fix tunneling of CF decisions to inactive graph regions in multi-parent configurations ([gaudi/Gaudi!863](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/863))
- Use `[[fallthrough]]` attribute instead of the _fallthrough_ comment ([gaudi/Gaudi!862](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/862))
- Restructure PRG sources and add visitor for Concurrent/Prompt CF inconsistency detection ([gaudi/Gaudi!861](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/861))
- Sort data dependencies in `HiveDataBroker` debug printout ([gaudi/Gaudi!868](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/868))
- Avoid displaying unused Binomial counters ([gaudi/Gaudi!830](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/830))
- Python 2 & 3 compatibility ([gaudi/Gaudi!832](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/832), [gaudi/Gaudi!852](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/852))
