.. _release-process:

Preparing a Gaudi release
=========================

Most of the work is automated by the ``utils/update_version.py`` script, but a
few manual steps and checks are required.

Branch model
------------

* Regular releases are prepared on a topic branch (for example
  ``release/41.0``) based on ``master``, submitted as a merge request targeting
  ``master``, and tagged from ``master`` once merged.
* Patch releases for an older major series are prepared on a branch created from
  the latest tag of that series (for example ``v40-patches`` from ``v40r4``) and
  tagged from that branch.

Prerequisites
-------------

* A checkout with the target branch checked out and all tags fetched
  (``git fetch --tags``).  The script derives the list of changes from the last
  tag reachable from the branch.
* A GitLab API token with the ``read_api`` scope, used to resolve the GitLab
  handles of the contributors.  The token is read from the ``GITLAB_TOKEN``
  environment variable or, as a fallback, from a file containing only the token
  (``~/.gitlab_token`` by default, or the path given by ``GITLAB_TOKEN_FILE``).
* The ``pre-commit`` hooks installed (``pre-commit install``).

Updating the version numbers
----------------------------

Run::

    python3 utils/update_version.py <version> [date]

where ``<version>`` is the version to release (for example ``v41r0``, or
``41.0``) and the optional ``date`` (``YYYY-MM-DD``) defaults to today.  Use
``--dry-run`` (``-n``) to preview the changes without modifying any file.

The script updates:

* ``CMakeLists.txt`` -- the CMake project version;
* ``docs/source/conf.py`` -- the ``version`` and ``release`` strings;
* ``CITATION.cff`` -- ``version`` and ``date-released``;
* ``pixi.toml`` -- the project version (in the ``[workspace]`` and ``[package]``
  sections);
* ``CHANGELOG.md`` -- a new draft section (see below).

Curating the changelog
----------------------

The script writes a flat, reverse-chronological list of the merge-request (and
commit) titles since the last tag, together with the list of contributors.  It
must be edited by hand:

* remove the entries that are not user-facing, in particular the
  release-preparation commits themselves (``Bump version to ...``,
  ``Update change log ...``);
* move each remaining entry into the ``### Changed``, ``### Added`` or
  ``### Fixed`` section, keeping the *Keep a Changelog* style used by the
  previous releases, and drop the sections that end up empty;
* add a short summary paragraph at the top of the section;
* clean up the contributor list: drop bot accounts, remove duplicated handles,
  and replace by hand the names that could not be resolved to a GitLab handle.

.. note::

   The list of changes is produced with ``git describe --tags --abbrev=0``, so it
   covers everything since the last tag *reachable from the current branch*.  On
   a branch that descends from an older tag than the latest release of the series
   (which is the case for the separate ``vXX-patches`` branches) the list is
   wider than the release.

.. note::

   A change that is both merged to ``master`` and backported to a patch branch
   appears in the changelog sections of all the releases that contain it, with
   the merge-request number used on each branch (which may differ).  This is
   expected: dropping such entries from a release would make its section
   incomplete.  What must be dropped are only the non-user-facing entries listed
   above.

Committing and merging
----------------------

Keep the release commit separate from unrelated changes.  Follow the historical
message convention::

    Bump version to <tag> and update change log

Push the branch and open a merge request targeting ``master``.  Once it is
merged, create the tag and the release from the GitLab web interface
(*Repository > Tags > New tag*, or *Repository > Releases > New release*), which
also creates the ``/-/releases/<tag>`` page linked by the changelog.

After the release
-----------------

A maintainer must add the new version to the Gaudi record on Zenodo
(https://zenodo.org/records/22663692, *New version*), so that the release gets
its own DOI.  This step is manual and cannot be done from the GitLab CI.

.. note::

   AI-assisted releases must follow the attribution rules of the
   :doc:`../contribution_guide`.
