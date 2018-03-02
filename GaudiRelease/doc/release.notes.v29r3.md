---
version: v29r3
date: 2018-03-02
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

This is a bugfix release including these changes:

*   [Bug Fix][]
    *   Fixed Python formatting ([mr !589][])
    *   Fix compilation with C++17 ([mr !599][])
    *   Fixed typo in CMake option name ([mr !609][])
*   [Build System][]
    *   Remove restrictions on names of installed headers ([mr !614][])
    *   Fix distcc support and add icecream ([mr !619][])
*   [Configuration][]
    *   Updated versions ([mr !602][])
    *   Externalize shared python tools ([mr !618][])

The full list of changes can be found on [Gitlab][].

**Externals version**: [LCG 93](http://lcginfo.cern.ch/release/93/)


[Gitlab]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?scope=all&state=merged&milestone_title=v29r3

[mr !589]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/589
[mr !599]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/599
[mr !602]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/602
[mr !609]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/609
[mr !614]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/614
[mr !618]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/618
[mr !619]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/619

[Bug Fix]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=bug+fix&scope=all&state=merged&milestone_title=v29r3
[Build System]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&scope=all&state=merged&milestone_title=v29r3
[C++ Framework]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&scope=all&state=merged&milestone_title=v29r3
[Code Cleanup]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=code+cleanup&scope=all&state=merged&milestone_title=v29r3
[Configuration]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=configuration&scope=all&state=merged&milestone_title=v29r3
[Documentation]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=documentation&scope=all&state=merged&milestone_title=v29r3
[Interactivity]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=interactivity&scope=all&state=merged&milestone_title=v29r3
[Performance]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=Performance&scope=all&state=merged&milestone_title=v29r3
[Task Scheduling]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=task+scheduling&scope=all&state=merged&milestone_title=v29r3
[Tests]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=tests&scope=all&state=merged&milestone_title=v29r3
