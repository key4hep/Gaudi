---
version: v29r1
date: 2017-10-25
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

This is a bugfix release including these changes:

* [Build System](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&milestone_title=v29r1&scope=all&state=merged)
  * Improved GitLab-CI configuration (use CentOS7 + gcc7, [mr !441](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/441), [mr !460](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/460))
  * Add support for CTest stdout compression in XML reports ( [mr !460](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/460))
* [C++ Framework](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&milestone_title=v29r1&scope=all&state=merged)
  * Fixed performance regression in messaging ([GAUDI-1319](https://its.cern.ch/jira/projects/GAUDI/issues/GAUDI-1319), [mr !427](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/427))
  * Fixed issue with mulltiple runs of *thistwrite* test ([GAUDI-1316](https://its.cern.ch/jira/projects/GAUDI/issues/GAUDI-1316), [mr !427](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/427))
  * Fixed various printouts ([mr !438](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/438))
  * Added `Gaudi::tagged_bool` class ([mr !440](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/440))
  * Fixed compilation error in overload resolution of `TupleObj::farray` for gcc7 ([mr !446](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/446))
  * Added missing header guard to `GaudiKernel/invoke.h` ([mr !459](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/459))
* [Task Scheduling](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=task+scheduling&milestone_title=v29r1&scope=all&state=merged)
  * Fixed task-based conditions handling ([mr !439](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/439))

The full list of changes can be found on [Gitlab](https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?milestone_title=v29r1&scope=all&state=merged).

**Externals version**: [LCG 91](http://lcgsoft.web.cern.ch/lcgsoft/release/91/)
