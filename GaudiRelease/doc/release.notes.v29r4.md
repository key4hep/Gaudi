---
version: v29r4
date: 2018-04-17
supported_builds: [CMake]
---
Package Coordinators: Marco Clemencic, Charles Leggett, Benedikt Hegner

# Release Notes

This is a bugfix release including these changes:

- [Bug Fix][]
  - Add test and fix of version propagation in KeyedContainer move ([mr !662][], [issue #27][])
  - Handle incompatible values for DataObjectHandleBase props ([mr !654][])
- [Build System][]
  - Improve Gitlab-CI configuration ([mr !655][])

The full list of changes can be found on [Gitlab][].

**Externals version**: [LCG 93](http://lcginfo.cern.ch/release/93/)


[Gitlab]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?scope=all&state=merged&milestone_title=v29r4

[issue #27]: https://gitlab.cern.ch/gaudi/Gaudi/issues/27

[mr !662]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/662
[mr !655]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/655
[mr !654]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/654

[Bug Fix]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=bug+fix&scope=all&state=merged&milestone_title=v29r4
[Build System]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=build+system&scope=all&state=merged&milestone_title=v29r4
[C++ Framework]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=C%2B%2B+framework&scope=all&state=merged&milestone_title=v29r4
[Code Cleanup]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=code+cleanup&scope=all&state=merged&milestone_title=v29r4
[Configuration]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=configuration&scope=all&state=merged&milestone_title=v29r4
[Documentation]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=documentation&scope=all&state=merged&milestone_title=v29r4
[Interactivity]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=interactivity&scope=all&state=merged&milestone_title=v29r4
[Performance]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=Performance&scope=all&state=merged&milestone_title=v29r4
[Task Scheduling]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=task+scheduling&scope=all&state=merged&milestone_title=v29r4
[Tests]: https://gitlab.cern.ch/gaudi/Gaudi/merge_requests?label_name%5B%5D=tests&scope=all&state=merged&milestone_title=v29r4
