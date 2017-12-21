---
version: v29r2
date: 2017-12-20
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

This is a bugfix release including these changes:

*   [Build System](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&milestone_title=v29r2&scope=all&state=merged)
    *   Cache os calls to make xenv faster ([mr !548](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/548))
    *   Add support for +cov optimization subtype ([mr !547](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/547))
    *   Proper use of C++ GSL and Range-v3 in CMake ([mr !520](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/520))
*   [Configuration](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=configuration&milestone_title=v29r2&scope=all&state=merged)
    *   Fixed use of zlib ([mr !561](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/561))
    *   Update versions for Gaudi v29r2 ([mr !550](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/550))
*   [Tests](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=tests&milestone_title=v29r2&scope=all&state=merged)
    *   Made multi_input tests more reliable ([mr !554](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/554))
    *   Migrate GaudiHive tests to QMT format ([mr !564](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/564))
    *   Fixes for CI tests ([mr !565](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/565))
*   [C++ Framework](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&milestone_title=v29r2&scope=all&state=merged)
    *   StatusCode check fixes ([mr !562](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/562))
    *   Fix unnamed lock in example code ([mr !559](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/559))
    *   Remove IssueSeverity support from StatusCode ([mr !557](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/557))
    *   GaudiAlg FunctionalDetails apply range size check in debug builds only ([mr !556](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/556))
    *   Update regex in PluginService ([mr !558](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/558))
    *   Reverse the order in which IoComponentMgr io_finalizes components ([mr !555](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/555))
    *   Sort components by name in HistorySvc dump ([mr !563](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/563))

The full list of changes can be found on [Gitlab](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?milestone_title=v29r2&scope=all&state=merged).

**Externals version**: [LCG 92](http://lcginfo.cern.ch/release/92/)
