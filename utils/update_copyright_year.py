#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import datetime
import os
import re
from argparse import ArgumentParser

COPYRIGHT_SIGNATURE = re.compile(r"\bcopyright\b", re.I)


def update_year(line: str, year: int) -> str:
    """
    Replace the end year of a range with the given year.
    """
    match = re.search(r"(\d{4}-)?(\d{4})", line)
    if match:
        range_start, range_end = match.groups()
        if range_start:
            line = re.sub(r"\d{4}-?\d{4}", f"{range_start}{year:04d}", line)
        elif int(range_end) < year:
            line = re.sub(r"\d{4}", f"{range_end}-{year:04d}", line)
            # if we have a line that is part of a table with borders, so we
            # try to remove 5 spaces to compensate the added start of range
            line = re.sub(r"\s{0,5}( [*#])$", r"\1", line)

    return line


def main():
    parser = ArgumentParser(
        "Simple script to update the copyright year in the files passed on the command line"
    )
    parser.add_argument(
        "--check-only",
        action="store_true",
        default=os.environ.get("CHECK_ONLY"),
        help="Check for presence of copyright (do not update years)",
    )
    parser.add_argument(
        "--no-check-only",
        dest="check_only",
        action="store_false",
        help="Check for presence of copyright (do not update years)",
    )
    parser.add_argument(
        "-Y",
        "--year",
        type=int,
        default=datetime.datetime.now().year,
        help="Year to use [default: current year]",
    )
    parser.add_argument("files", nargs="+", help="Files to update")

    args = parser.parse_args()

    exp = re.compile(r"[Cc]opyright.*(\d{4}-)?\d{4} CERN")

    missing_copyright = []
    for file in args.files:
        with open(file, "r") as f:
            content = f.readlines()

        if not any(COPYRIGHT_SIGNATURE.search(line) for line in content):
            missing_copyright.append(file)

        elif not args.check_only:
            # update the copyright year
            new_content = [
                update_year(line, args.year) if exp.search(line) else line
                for line in content
            ]

            if new_content != content:
                with open(file, "w") as f:
                    f.writelines(new_content)

    if missing_copyright:
        print(
            "The following files are missing a copyright line:\n  - "
            + "\n  - ".join(missing_copyright)
        )
        exit(1)


if __name__ == "__main__":
    main()
