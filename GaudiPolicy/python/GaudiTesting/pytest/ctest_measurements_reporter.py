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

# This plugin integrates with pytest to capture and report test results
# in a format compatible with CTest using DartMeasurement tags

# Key functions and hooks include:
# - sanitize_for_xml: Sanitizes a string by escaping non-XML characters.
# - pytest_configure: Initializes a dictionary to store test suite properties.
# - pytest_runtest_makereport: Collects information about failing tests and
# exceptions during test execution.
# - pytest_sessionfinish: Outputs the collected test information in a format
# suitable for CTest.


def sanitize_for_xml(data):
    bad_chars = re.compile("[\x00-\x08\x0b\x0c\x0e-\x1f\ud800-\udfff\ufffe\uffff]")

    def quote(match):
        "helper function"
        return "".join("[NON-XML-CHAR-0x%2X]" % ord(c) for c in match.group())

    return bad_chars.sub(quote, data)


def pytest_configure(config):
    config._testsuite_properties = {}


def pytest_runtest_makereport(item, call):
    if (
        call.when == "call"
        and call.excinfo is not None
        and call.excinfo.typename != "Skipped"
    ):
        if hasattr(item, "cls") and item.cls is not None:
            full_test_name = f"{item.cls.__name__}.{item.name}"
            property_name = f"{item.cls.__name__}.failing_tests"
        else:
            full_test_name = item.name
            property_name = "failing_tests"

        item.session.config._testsuite_properties.setdefault(property_name, []).append(
            full_test_name
        )
        item.user_properties.append(("exception_info", str(call.excinfo.value)))


def pytest_sessionstart(session):
    # make sure CTest does not drop output lines on successful tests
    print("CTEST_FULL_OUTPUT")


def pytest_sessionfinish(session, exitstatus):
    if not hasattr(session, "items"):
        # no test run, nothing to report
        return
    if os.environ.get("DISABLE_CTEST_MEASUREMENTS") == "1":
        # user requested to disable CTest measurements printouts
        return

    testsuite_properties = session.config._testsuite_properties
    results = list(testsuite_properties.items())
    prefix = ""

    sources = {}
    for item in session.items:
        prefix = (
            f"{item.cls.__name__}.{item.name}"
            if hasattr(item, "cls") and item.cls is not None
            else item.name
        )
        for name, value in item.user_properties:
            if name == "source_code":
                # strip the suffix with the parameters from the test name
                # before recording the source code
                base_name = re.sub(r"\[[^[]*\]$", "", prefix)
                sources[f"{base_name}.{name}"] = value
            else:
                results.append((f"{prefix}.{name}", value))
    results.extend(sources.items())

    ignore_keys = {"test_fixture_setup.completed_process"}
    template = (
        '<DartMeasurement type="text/string" name="{name}">{value}</DartMeasurement>'
    )

    to_print = [
        (key, value)
        for key, value in results
        if not any(key.endswith(ignore_key) for ignore_key in ignore_keys) and value
    ]
    for key, value in to_print:
        sanitized_value = XSS.escape(sanitize_for_xml(str(value)))
        # workaround for a limitation of CTestXML2HTML
        key = key.replace("/", "_")
        print(template.format(name=key, value=sanitized_value), end="")
