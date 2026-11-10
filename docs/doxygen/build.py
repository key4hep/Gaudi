#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Build the Doxygen documentation of Gaudi (no CMake configuration needed).

The documentation is created in OUTPUT/NAME (by default build/site/doxygen/local),
where it is found by the Sphinx documentation built in build/site (see the
"docs" Pixi task), and the list of the documentations in OUTPUT is updated.

The configuration is in docs/doxygen/Doxyfile: only the settings depending on
the environment are added here.
"""

import argparse
import os
import shutil
import subprocess
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# Doxygen tag file for the documentation of the standard library
LIBSTDCXX_URL = "https://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen"


def git(*args):
    return subprocess.run(
        ["git", *args], cwd=ROOT, check=True, capture_output=True, text=True
    ).stdout.strip()


def project_version():
    """Version of the project shown in the documentation."""
    if "CI_COMMIT_REF_NAME" in os.environ:
        return f"{os.environ['CI_COMMIT_REF_NAME']} ({os.environ['CI_COMMIT_SHA'][:8]})"
    return f"{git('describe', '--tags', '--always')} ({git('rev-parse', 'HEAD')[:8]})"


def get_tagfile(workdir):
    """Download the tag file for the standard library (None if not possible)."""
    tagfile = workdir / "libstdc++.tag"
    try:
        with urllib.request.urlopen(f"{LIBSTDCXX_URL}/libstdc++.tag", timeout=60) as r:
            tagfile.write_bytes(r.read())
    except OSError as err:
        if not tagfile.exists():
            print(f"warning: cannot get the libstdc++ tag file: {err}", file=sys.stderr)
            return None
        print(f"warning: using an old libstdc++ tag file ({err})", file=sys.stderr)
    return tagfile


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument(
        "--name",
        default="local",
        help="name of the documentation (for example the branch or tag name)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=ROOT / "build" / "site" / "doxygen",
        help="directory where to put the documentation (default: %(default)s)",
    )
    parser.add_argument(
        "--workdir",
        type=Path,
        default=ROOT / "build" / "doxygen-work",
        help="directory for the intermediate files (default: %(default)s)",
    )
    args = parser.parse_args()

    workdir = args.workdir.resolve()
    workdir.mkdir(parents=True, exist_ok=True)
    shutil.rmtree(workdir / "html", ignore_errors=True)

    config = [
        (ROOT / "docs" / "doxygen" / "Doxyfile").read_text(),
        f'PROJECT_NUMBER = "{project_version()}"',
        f'OUTPUT_DIRECTORY = "{workdir}"',
        f'WARN_LOGFILE = "{workdir / "warnings.log"}"',
        "QUIET = YES",
    ]
    tagfile = get_tagfile(workdir)
    if tagfile:
        config.append(f'TAGFILES = "{tagfile}={LIBSTDCXX_URL}"')

    print(f"Running Doxygen (warnings in {workdir / 'warnings.log'})...")
    subprocess.run(
        ["doxygen", "-"], input="\n".join(config), text=True, cwd=ROOT, check=True
    )

    destination = args.output.resolve() / args.name
    shutil.rmtree(destination, ignore_errors=True)
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(workdir / "html", destination)
    subprocess.run(
        [ROOT / "ci-utils" / "doxygen-eos", "index", destination.parent], check=True
    )
    print(f"Doxygen documentation in {destination}")


if __name__ == "__main__":
    main()
