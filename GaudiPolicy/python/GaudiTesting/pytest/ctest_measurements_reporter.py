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
import os
import re
import xml.sax.saxutils as XSS
from collections import defaultdict

# This plugin integrates with pytest to capture and report test results
# in a format compatible with CTest using DartMeasurement tags
# Key functions and hooks include:
# - sanitize_for_xml: Sanitize a string by escaping non-XML characters.
# - pytest_report_header: Add strings to the pytest header.
# - pytest_runtest_logreport: Collect test results and durations.
# - pytest_sessionfinish: Output the collected test information in a format
# suitable for CTest.

results = {}


def sanitize_for_xml(data):
    bad_chars = re.compile("[\x00-\x08\x0b\x0c\x0e-\x1f\ud800-\udfff\ufffe\uffff]")

    def quote(match):
        "helper function"
        return "".join("[NON-XML-CHAR-0x%2X]" % ord(c) for c in match.group())

    return bad_chars.sub(quote, data)


def pytest_report_header(config, start_path, startdir):
    # make sure CTest does not drop output lines on successful tests
    return "CTEST_FULL_OUTPUT"


def pytest_runtest_logreport(report):
    # collect user properties
    head_line = report.head_line
    results.update(
        {
            f"{head_line}.{k}": v
            for k, v in report.user_properties
            if f"{head_line}.{k}" not in results
        }
    )

    # collect test outcome
    if not report.passed:
        results[f"{report.head_line}.outcome"] = report.outcome
    else:
        results.setdefault(f"{report.head_line}.outcome", "passed")

    # collect test duration
    if report.when == "call":
        results[f"{report.head_line}.duration"] = round(report.duration, 2)


def pytest_sessionfinish(session, exitstatus):
    if not hasattr(session, "items"):
        # no test run, nothing to report
        return
    if os.environ.get("DISABLE_CTEST_MEASUREMENTS") == "1":
        # user requested to disable CTest measurements printouts
        return

    outcomes = defaultdict(list)
    for key in results:
        if key.endswith(".outcome"):
            outcomes[results[key]].append(key[:-8])
    results.update(
        (f"outcome.{outcome}", sorted(tests)) for outcome, tests in outcomes.items()
    )

    ignore_keys = {"test_fixture_setup.completed_process"}
    template = (
        '<DartMeasurement type="text/string" name="{name}">{value}</DartMeasurement>'
    )

    to_print = [
        (key, value)
        for key, value in results.items()
        if not any(key.endswith(ignore_key) for ignore_key in ignore_keys) and value
    ]
    to_print.sort()
    for key, value in to_print:
        sanitized_value = XSS.escape(sanitize_for_xml(str(value)))
        # workaround for a limitation of CTestXML2HTML
        key = key.replace("/", "_")
        print(template.format(name=key, value=sanitized_value), end="")
