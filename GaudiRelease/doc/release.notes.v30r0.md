---
version: v30r0
date: 2017-11-17
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

For this major release we started introducing backward incompatible changes
(which will probably continue in the v30rX series).

* [Build System](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&milestone_title=v30r0&scope=all&state=merged)
  * Handle correctly removed dictionary dependencies (mr [!413](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/413))
  * Improved GitLab-CI configuration (use CentOS7 + gcc7, mr [!433](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/433))
  * Improved behaviour of `get_host_binary_tag.py` for unknown distribution detection fails (mr [!471](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/471))
  * Add support for CTest stdout compression in XML reports (mr [!453](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/453))
* [C++ Framework](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&milestone_title=v30r0&scope=all&state=merged)
  * Modernization and simplification
    * Simplify `GaudiSequencer` (mr [!483](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/483))
    * Modernize `HistoDef` (mr [!492](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/492)), `FileMgr` (mr [!493](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/493)), `HepRndmEngines` (mr [!494](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/494)), `genconf` (mr [!437](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/437))
    * Change `IDataManagerSvc` and `IDataProviderSvc` to `use boost::string_ref` (mr [!474](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/474), [!508](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/508))
    * Prefer C++14 `..._t<T>` over `typename ...<T>::type` (mr [!411](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/411))
  * Clean up
    * Remove deprecated header `cbrt.h` (mrs [!486](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/486))
    * Update regex in `PluginService` (mr [!504](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/504))
    * Remove Phoenix v2 scaffolding (mr [!447](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/447))
    * Remove unnecessary `long`/`StatusCode` conversions (mr [!495](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/495))
    * Remove `IssueSeverity` support from `StatusCode` (mr [!477](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/477))
    * Remove include of deprecated `GaudiKernel/AlgFactory.h` header (mr [!414](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/414))
  * Bugfixes
    * Fix abnormal termination caused by use of an invalid context when AlgExecStateSvc is verbose (mrs [!475](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/475))
    * Fix unnamed lock in example code (mr [!487](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/487))
    * StatusCode check fixes (mr [!463](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/463))
    * MacOS Fixes (mr [!464](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/464))
    * Fix compilation on gcc5.4 (Ubuntu 16.04) (mr [!465](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/465))
    * Fix messaging performance regression (mr [!428](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/428))
    * Fix various printouts (mr [!416](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/416))
    * Add missing header guard to `invoke.h` (mr [!457](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/457))
    * Implement rigorous locking in `HiveWhiteBoard` (mr [!388](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/388), [!432](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/432))
    * Tweak overload resolution of `TupleObj::farray` for gcc7 (mr [!430](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/430))
    * Fix warnings (mr [!423](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/423))
    * Fixed handling of data dependencies in `ToolHandles` (mr [!429](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/429), [!501](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/501))
    * Fixed use of messages in `ApplicationMgr::i_startup` (mr [!436](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/436))
  * Improvements
    * Add compile-time warning about ignoring returned `StatusCode` (mr [!386](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/386))
    * Reverse the order in which `IoComponentMgr` io_finalizes components (mr [!469](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/469))
    * Add `TaggedBool` (mr [!435](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/435))
* [Configuration](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=configuration&milestone_title=v30r0&scope=all&state=merged)
  * Fixes for CI tests (mr [!503](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/503))
  * MacOS Fixes (mr [!464](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/464))
* [Documentation](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=documentation&milestone_title=v30r0&scope=all&state=merged)
  * Add documentation for `apply-formatting` target (mr [!484](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/484))
  * Update build instructions (mr [!410](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/410))
* [Task Scheduling](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=task+scheduling&milestone_title=v30r0&scope=all&state=merged)
  * Removed `DataFlowManager` (mr [!472](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/472))
  * Added protection on event slot creation in `HiveWhiteBoard` (mr [!502](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/502))
  * Made `PrecedenceSvc` recognize data loader algorithm (mr [!473](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/473), [!506](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/506))
  * Improvements to temporal and topological execution flow tracer [!444](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/444)
  * Optimized `AvalancheScheduler` by avoiding the use of algo names and prefering algo indexes (mr [!482](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/482))
  * Replaced all `std::bind` by lambdas in `AvalancheScheduler` (mr [!481](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/481))
  * Fixed race condition in `AvalancheScheduler` (mr [!479](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/479))
  * Dropped unused member `m_updateNeeded` in `AvalancheScheduler` (mr [!478](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/478))
  * Simplified (aka optimized) usage of ttb tasks in `AvalancheScheduler` (mr [!452](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/452))
  * Add `DecisionNode` negation/inversion to `AvalancheScheduler` (mr [!434](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/434))
  * Fix task-based conditions handling (mr [!431](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/431))
  * Remove forward scheduler and connected components (mr [!412](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/412))
* [Tests](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=tests&milestone_title=v30r0&scope=all&state=merged)
  * Migrate GaudiHive tests to QMT format (mr [!419](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/419))
  * Fixes for CI tests (mr [!503](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/503))
  * Fixed handling of data dependencies in `ToolHandles` (mr [!429](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/429), [!501](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/501))
  * Made `PrecedenceSvc` recognize data loader algorithm (mr [!473](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/473), [!506](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/506))
  * Added test for the `TimelineSvc` (mr [!415](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/415))
  * Add support for CTest stdout compression in XML reports (mr [!453](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/453))
  * Made multi_input tests more reliable (mr [!468](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/468))


The full list of changes can be found on [Gitlab](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?milestone_title=v30r0&scope=all&state=merged).

**Externals version**: [LCG 91](http://lcgsoft.web.cern.ch/lcgsoft/release/91/)
