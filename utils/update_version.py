#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 2023-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import datetime
import re
import sys
from collections.abc import Callable, Iterable
from difflib import unified_diff
from subprocess import run
from typing import Union

import click


def normalize_version(version: str) -> tuple[str, str]:
    """
    Convert a version in format "vXrY" or "X.Y" in the pair ("X.Y", "vXrY").

    >>> normalize_version("v37r0")
    ('37.0', 'v37r0')
    >>> normalize_version("37.0.1")
    ('37.0.1', 'v37r0p1')
    """
    # extract the digits
    numbers = re.findall(r"\d+", version)
    return (
        ".".join(numbers),
        "".join("{}{}".format(*pair) for pair in zip("vrpt", numbers)),
    )


class Fields:
    """
    Helper to carry the allowed fields for formatting replacement strings.

    >>> f = Fields("v37r1", datetime.date(2023, 9, 25))
    >>> f
    Fields('37.1', datetime.date(2023, 9, 25))
    >>> f.data
    {'cmake_version': '37.1', 'tag_version': 'v37r1', 'date': datetime.date(2023, 9, 25)}
    """

    def __init__(self, version: str, date: datetime.date):
        cmake_version, tag_version = normalize_version(version)
        self._data = dict(
            cmake_version=cmake_version,
            tag_version=tag_version,
            date=date,
        )

    def __repr__(self):
        return (
            f"Fields({repr(self._data['cmake_version'])}, {repr(self._data['date'])})"
        )

    @property
    def data(self):
        return self._data


class ReplacementRule:
    """
    Helper to replace lines with patterns or applying functions.

    >>> r = ReplacementRule(r"^version: ", "version: {cmake_version}")
    >>> f = Fields("v1r1", datetime.date(2023, 9, 25))
    >>> r("nothing to change\\n", f)
    'nothing to change\\n'
    >>> r("version: 1.0\\n", f)
    'version: 1.1\\n'
    """

    def __init__(
        self,
        pattern: Union[str, re.Pattern],
        replace: Union[str, Callable[[str, Fields], str]],
    ):
        self.pattern = re.compile(pattern)
        if isinstance(replace, str):
            replace = f"{replace.rstrip()}\n"
            self.replace = lambda _line, fields: replace.format(**fields.data)
        else:
            self.replace = replace

    def __call__(self, line: str, fields: Fields) -> str:
        if self.pattern.match(line):
            return self.replace(line, fields)
        return line


class FileUpdater:
    def __init__(
        self, filename: str, rules: Iterable[Union[ReplacementRule, tuple[str, str]]]
    ):
        self.filename = filename
        self.rules = [
            r if isinstance(r, ReplacementRule) else ReplacementRule(*r) for r in rules
        ]

    def _apply_rules(self, line: str, fields: Fields) -> str:
        for rule in self.rules:
            line = rule(line, fields)
        return line

    def __call__(self, fields: Fields) -> tuple[str, list[str], list[str]]:
        with open(self.filename) as f:
            old = f.readlines()
        return self.filename, old, [self._apply_rules(line, fields) for line in old]


def update_changelog(fields: Fields) -> tuple[str, list[str], list[str]]:
    """
    Special updater to fill draft changelog entry.
    """
    latest_tag = run(
        ["git", "describe", "--tags", "--abbrev=0"], capture_output=True, text=True
    ).stdout.strip()
    # This formats the git log as a rough markdown list
    # - collect the log formatting it such that we can machine parse it
    changes_txt = run(
        ["git", "log", "--first-parent", "--format=%s<=>%b|", f"{latest_tag}.."],
        capture_output=True,
        text=True,
    ).stdout
    # - removing trailing separator and make it a single line
    changes_txt = " ".join(changes_txt.strip().rstrip("|").splitlines())
    # - normalize issues and merge requests links
    changes = (
        changes_txt.replace("Closes #", "gaudi/Gaudi#")
        .replace("See merge request ", "")
        .split("|")
    )
    # - split the messages and format the list
    changes = [
        f"- {msg.strip()} ({', '.join(refs.split())})\n"
        if refs.strip()
        else f"- {msg.strip()}\n"
        for change in changes
        for msg, refs in ([change.split("<=>", 1)] if "<=>" in change else [])
    ]

    filename = "CHANGELOG.md"
    with open(filename) as f:
        old = f.readlines()
    for idx, line in enumerate(old):
        if line.startswith("## ["):
            break

    data = old[:idx]
    data.extend(
        [
            "## [{tag_version}](https://gitlab.cern.ch/gaudi/Gaudi/-/releases/{tag_version}) - {date}\n".format(
                **fields.data
            ),
            "\n",
            "### Changed\n",
            "### Added\n",
            "### Fixed\n",
            "\n",
        ]
    )
    data.extend(changes)
    data.extend(["\n", "\n"])
    data.extend(old[idx:])

    return filename, old, data


@click.command()
@click.argument("version", type=str)
@click.argument(
    "date",
    type=click.DateTime(("%Y-%m-%d",)),
    metavar="[DATE]",
    default=datetime.datetime.now(),
)
@click.option(
    "--dry-run",
    "-n",
    default=False,
    is_flag=True,
    help="only show what would change, but do not modify the files",
)
def update_version(version: str, date: datetime.datetime, dry_run: bool):
    """
    Helper to easily update the project version number in all needed files.
    """
    fields = Fields(version, date.date())
    click.echo(
        "Bumping version to {cmake_version} (tag: {tag_version})".format(**fields.data)
    )

    for updater in [
        FileUpdater(
            "CMakeLists.txt",
            [(r"^project\(Gaudi VERSION", "project(Gaudi VERSION {cmake_version}")],
        ),
        FileUpdater(
            "CITATION.cff",
            [
                (r"^version: ", "version: {tag_version}"),
                (r"^date-released: ", "date-released: '{date}'"),
            ],
        ),
        FileUpdater(
            "docs/source/conf.py",
            [
                (r"^version = ", 'version = "{cmake_version}"'),
                (r"^release = ", 'release = "{tag_version}"'),
            ],
        ),
        update_changelog,
    ]:
        filename, old, new = updater(fields)

        if old != new:
            if dry_run:
                sys.stdout.writelines(
                    unified_diff(
                        old,
                        new,
                        fromfile=f"a/{filename}",
                        tofile=f"b/{filename}",
                    )
                )
            else:
                click.echo(f"updated {filename}")
                with open(filename, "w") as f:
                    f.writelines(new)


if __name__ == "__main__":
    update_version()
