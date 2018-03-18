---
version: v30r2
date: 2018-03-16
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

This release contains several bugfixes and improvements, and some backward
incompatible change.

## Backward Incompatible Changes
*   Dropped backward compatibility with XyzFactory classes ([mr !420][])
    *   obsolete `XyzFactory.h` header files and macros `DECLARE_XYZ_FACTORY`
        have been removed
*   Add category support to `StatusCode` ([mr !514][])
    *   `StatusCode` internal code is now an enum class and conversion to/from
        integer has been made _explicit_
*   Remove obsolete MT components ([mr !546][], [mr !608][])
    *   unused GaudiMT and GaudiMTTools have been removed
*   Externalize shared python tools ([mr !612][])
    *   a couple of Python tools have been moved to independent projects
*   Dropped unused `ICounterSvc`, its implementation and its tests ([mr !628][])
    *   not used

## Deprecations
*   Add deprecation warning in `PropertyProxy` ([mr !633][])
    *   properties with `[[deprecated]]` in the doc string print a deprecation
        warning when assigned
*   Deprecate Algorithm begin/end run methods ([mr !542][])
    *   `beginRun` and `endRun` methods of `Algorithm` are deprecated

## All Merge Requests
*   [Bug Fix][]
    *   EventView isolation fix, and added testing ([mr !540][])
    *   TaggedBool: use a more specific header guard name ([mr !584][])
    *   Fix formatting of current master ([mr !586][])
    *   `[[deprecated]]` does not compile with using statements ([mr !587][])
    *   Fixed Python formatting ([mr !588][])
    *   Make uncompiled exported headers compilable ([mr !590][])
    *   Fix compilation with gcc 5.4 (Ubuntu 16.04) ([mr !607][])
    *   Allow updating of registered Timeline Event records ([mr !610][])
    *   Fix compilation with C++17 ([mr !616][])
    *   Speed up TimelineSvc ([mr !620][])
    *   Remove constexpr from StatusCode default constructor for Coverity scans ([mr !630][])
    *   Fix deduction of property return values ([mr !634][])
    *   AvalancheSchedulerSvc: Fix undefined output ordering ([mr !635][])
    *   Fix warnings seen when compiling with gcc8 ([mr !638][])
*   [Build System][]
    *   Fixed typo in CMake option name ([mr !578][])
    *   Fix distcc support and add icecream ([mr !585][])
    *   Require Python 2.7 until Python 3 migration is complete ([mr !606][])
    *   Externalize shared python tools ([mr !612][])
    *   Remove restrictions on names of installed headers ([mr !615][])
    *   GaudiProjectConfig: Fix generated `__init__.py` files to handle symlinks ([mr !636][])
*   [C++ Framework][]
    *   Add thread friendly `THistSvc` ([mr !455][])
    *   Make `Gaudi::Property<T>` work even if `T` is only copy-constructible ([mr !470][])
    *   Version of [!508][] closer to the original [!474][] version ([mr !509][])
    *   Implement EventIDBase ordering in a generic way by specifying projections ([mr !510][])
    *   Add category support to `StatusCode` ([mr !514][])
    *   Use tool type as default name for auto registered ToolHandles ([mr !535][])
    *   Deprecate Algorithm begin/end run methods ([mr !542][])
    *   Remove obsolete MT components ([mr !546][])
    *   CommonMessaging: allow msgLevel to be called early ([mr !566][])
    *   Add support for `Gaudi::Range_` to `DataObjectHandle` ([mr !571][])
    *   Demoted a few `AvalancheSchedulerSvc` messages ([mr !576][])
    *   Add Range & Selection support to Gaudi::Functional ([mr !579][])
    *   Added support for direct reading over WebDAV to IoComponentMgr ([mr !580][])
    *   Use `GaudiKernel/compose.h` ([mr !582][])
    *   ToolHandle: add TaggedBool parameter to control retrieval ([mr !583][])
    *   ToolHandle: throw exception if retrieval fails ([mr !591][])
    *   Revert some interface changes of `THistSvc` ([mr !594][])
    *   ToolHandle: add EnableTool tagged bool to control retrieval ([mr !595][])
    *   Add `declareTool(handle)` convenience method to Algorithm ([mr !598][])
    *   CommonMessaging: Improved inlining and branch predicition ([mr !600][])
    *   Use deduction guide in C++17 version of compose and rename it to overloaded ([mr !621][])
    *   StatusCode: have setters return reference to `*this` ([mr !627][])
    *   ThreadInitTask: MT fix ([mr !631][])
*   [Code Cleanup][]
    *   Dropped backward compatibility with XyzFactory classes ([mr !420][])
    *   Avoid C-style casts ([mr !526][])
    *   Change Tuple ownership implementation ([mr !534][])
    *   Revisit in-failure and in-stall console dumps of scheduler state, plus few other misc. things ([mr !541][])
    *   Use `detected_or` idiom to simplify `Range.h` ([mr !572][])
    *   Remove unused IIncidentListener inheritance from RndmEngine ([mr !573][])
    *   Extend `detected_or` implementation and use it in CommonMessaging ([mr !574][])
    *   Cleanup `GaudiKernel/reverse.h` ([mr !581][])
    *   Restore thread-wise task schedule reporting in scheduler state dumps ([mr !593][])
    *   Remove deprecated header `GaudiKernel/ThreadGaudi.h` ([mr !608][])
    *   Simplify TupleObj::columns ([mr !623][])
    *   EventIDBase: improve printout ([mr !626][])
    *   Dropped unused `ICounterSvc`, its implementation and its tests ([mr !628][])
    *   Add deprecation warning in `PropertyProxy` ([mr !633][])
    *   Remove deprecation warning from ExtraInputs and ExtraOutputs properties ([mr !639][])
*   [Configuration][]
    *   Update Gaudi and LCG versions, and release notes ([mr !601][])
*   [Task Scheduling][]
    *   Adapt RunSimulator visitor to new downward navigation gears in the CF PRG realm ([mr !539][])
*   [Tests][]
    *   Add test build of public headers ([mr !592][])
    *   Small improvement to the TimelineSvc test ([mr !611][])
    *   Add `gaudi_add_compile_test` and StatusCode compilation test ([mr !613][])

The full list of changes can also be found on [Gitlab][].

**Externals version**: [LCG 93](http://lcginfo.cern.ch/release/93/)


[Gitlab]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?scope=all&state=merged&milestone_title=v30r2

[!508]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/508
[!474]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/474

[mr !420]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/420
[mr !455]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/455
[mr !470]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/470
[mr !509]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/509
[mr !510]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/510
[mr !514]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/514
[mr !526]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/526
[mr !534]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/534
[mr !535]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/535
[mr !539]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/539
[mr !540]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/540
[mr !541]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/541
[mr !542]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/542
[mr !546]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/546
[mr !566]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/566
[mr !571]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/571
[mr !572]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/572
[mr !573]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/573
[mr !574]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/574
[mr !576]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/576
[mr !578]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/578
[mr !579]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/579
[mr !580]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/580
[mr !581]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/581
[mr !582]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/582
[mr !583]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/583
[mr !584]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/584
[mr !585]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/585
[mr !586]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/586
[mr !587]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/587
[mr !588]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/588
[mr !590]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/590
[mr !591]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/591
[mr !592]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/592
[mr !593]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/593
[mr !594]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/594
[mr !595]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/595
[mr !598]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/598
[mr !600]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/600
[mr !601]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/601
[mr !606]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/606
[mr !607]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/607
[mr !608]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/608
[mr !610]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/610
[mr !611]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/611
[mr !612]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/612
[mr !613]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/613
[mr !615]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/615
[mr !616]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/616
[mr !620]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/620
[mr !621]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/621
[mr !623]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/623
[mr !626]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/626
[mr !627]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/627
[mr !628]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/628
[mr !630]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/630
[mr !631]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/631
[mr !633]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/633
[mr !634]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/634
[mr !635]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/635
[mr !636]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/636
[mr !638]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/638
[mr !639]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/639

[Bug Fix]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=bug+fix&scope=all&state=merged&milestone_title=v30r2
[Build System]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&scope=all&state=merged&milestone_title=v30r2
[C++ Framework]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&scope=all&state=merged&milestone_title=v30r2
[Code Cleanup]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=code+cleanup&scope=all&state=merged&milestone_title=v30r2
[Configuration]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=configuration&scope=all&state=merged&milestone_title=v30r2
[Documentation]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=documentation&scope=all&state=merged&milestone_title=v30r2
[Interactivity]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=interactivity&scope=all&state=merged&milestone_title=v30r2
[Performance]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=Performance&scope=all&state=merged&milestone_title=v30r2
[Task Scheduling]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=task+scheduling&scope=all&state=merged&milestone_title=v30r2
[Tests]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=tests&scope=all&state=merged&milestone_title=v30r2
