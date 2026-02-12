#####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
from pathlib import Path

import pytest
from GaudiTesting import utils


@pytest.mark.parametrize(
    "reference,platform,files,expected",
    [
        # unstructured
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-opt",
            ["my.ref", "my.ref.dbg"],
            "refs/my.ref",
        ),
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-dbg",
            ["my.ref", "my.ref.dbg"],
            "refs/my.ref.dbg",
        ),
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-dbg",
            ["my.ref", "my.ref.dbg", "my.ref.dbg-centos7"],
            "refs/my.ref.dbg",
        ),
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-dbg",
            ["my.ref", "my.ref.dbg-centos7"],
            "refs/my.ref",
        ),
        (
            "refs/my.ref",
            "x86_64-centos7-gcc13-dbg",
            ["my.ref", "my.ref.dbg", "my.ref.dbg-centos7"],
            "refs/my.ref.dbg-centos7",
        ),
        # structured
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-opt",
            ["my_ref.yaml", "my_ref.dbg.yaml"],
            "refs/my_ref.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg.yaml"],
            "refs/my_ref.dbg.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg.yaml", "my_ref.dbg-centos7.yaml"],
            "refs/my_ref.dbg.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg-centos7.yaml"],
            "refs/my_ref.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-centos7-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg.yaml", "my_ref.dbg-centos7.yaml"],
            "refs/my_ref.dbg-centos7.yaml",
        ),
        (
            "refs/my_ref.yml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yml", "my_ref.dbg.yml"],
            "refs/my_ref.dbg.yml",
        ),
        # invalid override
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.yaml.dbg"],
            "refs/my_ref.yaml",
        ),
        # missing
        ("refs/my_ref.yaml", "x86_64-el9-gcc13-opt", [], "refs/my_ref.yaml"),
    ],
)
def test_expand_reference_file(reference, platform, files, expected, monkeypatch):
    monkeypatch.setattr(utils, "get_platform", lambda: platform)
    monkeypatch.setattr(os, "listdir", lambda _: files)
    monkeypatch.setattr(os.path, "isdir", lambda _: True)

    assert utils.expand_reference_file_name(reference) == expected


class Dummy:
    pass


def test_file_for_class():
    assert utils.file_path_for_class(Dummy) == Path(__file__)


# =============================================================================
# Tests for _format_path helper
# =============================================================================


@pytest.mark.parametrize(
    "path,expected",
    [
        ([], "root"),
        (["key"], "['key']"),
        ([0], "[0]"),
        (["a", "b", "c"], "['a']['b']['c']"),
        ([0, 1, 2], "[0][1][2]"),
        (["data", 0, "value"], "['data'][0]['value']"),
        (["key with spaces"], "['key with spaces']"),
    ],
)
def test_format_path(path, expected):
    assert utils._format_path(path) == expected


# =============================================================================
# Tests for _floats_close helper
# =============================================================================


@pytest.mark.parametrize(
    "a,b,rtol,atol,expected",
    [
        # Exact match
        (1.0, 1.0, 1e-9, 0.0, True),
        # Within rtol
        (1.0, 1.0 + 1e-10, 1e-9, 0.0, True),
        # Outside rtol
        (1.0, 1.001, 1e-9, 0.0, False),
        # Within atol
        (0.0, 1e-10, 0.0, 1e-9, True),
        # Outside atol
        (0.0, 1e-8, 0.0, 1e-9, False),
        # Combined tolerance
        (1.0, 1.01, 0.005, 0.005, True),
        # Negative values
        (-1.0, -1.0 - 1e-10, 1e-9, 0.0, True),
        # Zero comparison
        (0.0, 0.0, 1e-9, 0.0, True),
        # Near-zero with atol
        (1e-15, 2e-15, 0.0, 1e-14, True),
        # Large values
        (1e10, 1e10 + 1, 1e-9, 0.0, True),
    ],
)
def test_floats_close(a, b, rtol, atol, expected):
    assert utils._floats_close(a, b, rtol, atol) == expected


# =============================================================================
# Tests for assert_objects_equal - Success cases
# =============================================================================


@pytest.mark.parametrize(
    "obj1,obj2,rtol,atol",
    [
        # Primitive equality
        (1, 1, 1e-9, 0.0),
        ("hello", "hello", 1e-9, 0.0),
        (True, True, 1e-9, 0.0),
        (False, False, 1e-9, 0.0),
        (None, None, 1e-9, 0.0),
        # Float within default tolerance
        (1.0, 1.0 + 1e-10, 1e-9, 0.0),
        # Float with custom rtol
        (100.0, 101.0, 0.02, 0.0),
        # Float with custom atol
        (0.0, 0.001, 1e-9, 0.01),
        # Empty structures
        ({}, {}, 1e-9, 0.0),
        ([], [], 1e-9, 0.0),
        # Simple dict
        ({"a": 1}, {"a": 1}, 1e-9, 0.0),
        # Multiple keys
        ({"a": 1, "b": 2}, {"a": 1, "b": 2}, 1e-9, 0.0),
        # Different key order
        ({"a": 1, "b": 2}, {"b": 2, "a": 1}, 1e-9, 0.0),
        # Nested dict
        ({"a": {"b": 1}}, {"a": {"b": 1}}, 1e-9, 0.0),
        # Dict with float values
        ({"x": 1.0}, {"x": 1.0 + 1e-10}, 1e-9, 0.0),
        # Simple list
        ([1], [1], 1e-9, 0.0),
        # Multiple elements
        ([1, 2, 3], [1, 2, 3], 1e-9, 0.0),
        # Nested list
        ([[1, 2], [3, 4]], [[1, 2], [3, 4]], 1e-9, 0.0),
        # List with floats
        ([1.0, 2.0], [1.0 + 1e-10, 2.0], 1e-9, 0.0),
        # Mixed types in list
        ([1, "a", None], [1, "a", None], 1e-9, 0.0),
        # Dict containing list
        ({"items": [1, 2, 3]}, {"items": [1, 2, 3]}, 1e-9, 0.0),
        # List containing dict
        ([{"a": 1}, {"b": 2}], [{"a": 1}, {"b": 2}], 1e-9, 0.0),
        # Deep nesting
        (
            {"a": {"b": {"c": [1, 2, {"d": 3.0}]}}},
            {"a": {"b": {"c": [1, 2, {"d": 3.0}]}}},
            1e-9,
            0.0,
        ),
        # Empty nested structures
        ({"a": {}, "b": []}, {"a": {}, "b": []}, 1e-9, 0.0),
    ],
)
def test_assert_objects_equal_success(obj1, obj2, rtol, atol):
    utils.assert_objects_equal(obj1, obj2, rtol=rtol, atol=atol)


# =============================================================================
# Tests for assert_objects_equal - Failure cases
# =============================================================================


@pytest.mark.parametrize(
    "obj1,obj2,expected_errors",
    [
        # Type mismatches
        (1, "1", ["Type mismatch at root", "int vs str"]),
        ([], {}, ["Type mismatch at root", "list vs dict"]),
        (1, 1.0, ["Type mismatch at root", "int vs float"]),
        (None, 0, ["Type mismatch at root", "NoneType vs int"]),
        ({"a": 1}, {"a": "1"}, ["Type mismatch at ['a']", "int vs str"]),
        # Extra keys
        ({"a": 1, "b": 2}, {"a": 1}, ["Extra key in first object at root", "'b'"]),
        ({"a": 1}, {"a": 1, "b": 2}, ["Extra key in second object at root", "'b'"]),
        ({"a": 1}, {"b": 1}, ["Extra key in first", "Extra key in second"]),
        (
            {"x": {"a": 1}},
            {"x": {}},
            ["Extra key in first object at ['x']", "'a'"],
        ),
        # List length mismatches
        ([1, 2, 3], [1, 2], ["List length mismatch at root", "3 vs 2"]),
        ([1], [1, 2], ["List length mismatch at root", "1 vs 2"]),
        ({"a": [1, 2]}, {"a": [1]}, ["List length mismatch at ['a']", "2 vs 1"]),
        # Value mismatches (non-float)
        (1, 2, ["Value mismatch at root", "1", "2"]),
        ("hello", "world", ["Value mismatch at root", "'hello'", "'world'"]),
        (True, False, ["Value mismatch at root", "True", "False"]),
        ({"a": {"b": 1}}, {"a": {"b": 2}}, ["Value mismatch at ['a']['b']"]),
    ],
)
def test_assert_objects_equal_failure(obj1, obj2, expected_errors):
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2)
    error_msg = str(exc_info.value)
    for expected in expected_errors:
        assert expected in error_msg, f"Expected '{expected}' in '{error_msg}'"


@pytest.mark.parametrize(
    "obj1,obj2,rtol,atol,expected_errors",
    [
        # Float mismatches with tolerance info
        (1.0, 2.0, 1e-9, 0.0, ["Float mismatch at root", "1.0", "2.0", "diff="]),
        (
            1.0,
            1.1,
            0.05,
            0.0,
            ["Float mismatch at root", "rtol=0.05"],
        ),
        (
            1.0,
            1.1,
            1e-9,
            0.01,
            ["Float mismatch at root", "atol=0.01"],
        ),
        (
            {"x": 1.0},
            {"x": 2.0},
            1e-9,
            0.0,
            ["Float mismatch at ['x']"],
        ),
        (
            [1.0, 2.0],
            [1.0, 3.0],
            1e-9,
            0.0,
            ["Float mismatch at [1]"],
        ),
    ],
)
def test_assert_objects_equal_float_failure(obj1, obj2, rtol, atol, expected_errors):
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2, rtol=rtol, atol=atol)
    error_msg = str(exc_info.value)
    for expected in expected_errors:
        assert expected in error_msg, f"Expected '{expected}' in '{error_msg}'"


# =============================================================================
# Tests for multiple differences
# =============================================================================


def test_assert_objects_equal_multiple_differences():
    """Test that multiple differences are all reported."""
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal({"a": 1, "b": 2}, {"a": 10, "b": 20})
    error_msg = str(exc_info.value)
    assert "2 difference(s) found" in error_msg
    assert "['a']" in error_msg
    assert "['b']" in error_msg


def test_assert_objects_equal_mixed_error_types():
    """Test that different error types are all reported."""
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal({"a": 1, "b": [1]}, {"a": "1", "b": [1, 2]})
    error_msg = str(exc_info.value)
    assert "2 difference(s) found" in error_msg
    assert "Type mismatch" in error_msg
    assert "List length mismatch" in error_msg


# =============================================================================
# Edge case tests
# =============================================================================


def test_assert_objects_equal_deep_nesting():
    """Test deeply nested structures."""
    deep1 = {"a": {"b": {"c": {"d": {"e": {"f": 1}}}}}}
    deep2 = {"a": {"b": {"c": {"d": {"e": {"f": 1}}}}}}
    utils.assert_objects_equal(deep1, deep2)

    deep3 = {"a": {"b": {"c": {"d": {"e": {"f": 2}}}}}}
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(deep1, deep3)
    assert "['a']['b']['c']['d']['e']['f']" in str(exc_info.value)


def test_assert_objects_equal_special_floats():
    """Test special float values."""
    # Infinity
    utils.assert_objects_equal(float("inf"), float("inf"))
    utils.assert_objects_equal(float("-inf"), float("-inf"))

    with pytest.raises(AssertionError):
        utils.assert_objects_equal(float("inf"), float("-inf"))

    # NaN is not equal to itself
    with pytest.raises(AssertionError):
        utils.assert_objects_equal(float("nan"), float("nan"))


def test_assert_objects_equal_zero_variants():
    """Test that 0.0 and -0.0 are considered equal."""
    utils.assert_objects_equal(0.0, -0.0)
    utils.assert_objects_equal({"x": 0.0}, {"x": -0.0})


def test_assert_objects_equal_unicode_keys():
    """Test unicode in dictionary keys."""
    utils.assert_objects_equal({"café": 1, "日本語": 2}, {"café": 1, "日本語": 2})

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal({"café": 1}, {"café": 2})
    assert "café" in str(exc_info.value)


def test_assert_objects_equal_large_list():
    """Test with a large list."""
    large1 = list(range(1000))
    large2 = list(range(1000))
    utils.assert_objects_equal(large1, large2)

    large3 = list(range(1000))
    large3[500] = 999999
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(large1, large3)
    assert "[500]" in str(exc_info.value)


def test_assert_objects_equal_bool_vs_int():
    """Test that bool and int are considered different types."""
    # In Python, bool is a subclass of int, but type() distinguishes them
    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(True, 1)
    assert "Type mismatch" in str(exc_info.value)
    assert "bool vs int" in str(exc_info.value)


# =============================================================================
# Tests for max_differences parameter
# =============================================================================


def test_assert_objects_equal_max_differences_default():
    """Test that default max_differences is 10."""
    # Create object with 20 differences
    obj1 = {f"key{i}": i for i in range(20)}
    obj2 = {f"key{i}": i + 100 for i in range(20)}

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2)
    error_msg = str(exc_info.value)

    # Should report exactly 10 differences (the default)
    assert "10 difference(s) found" in error_msg
    assert "output truncated" in error_msg


def test_assert_objects_equal_max_differences_custom():
    """Test custom max_differences value."""
    obj1 = {f"key{i}": i for i in range(20)}
    obj2 = {f"key{i}": i + 100 for i in range(20)}

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2, max_differences=5)
    error_msg = str(exc_info.value)

    assert "5 difference(s) found" in error_msg
    assert "output truncated" in error_msg


def test_assert_objects_equal_max_differences_not_truncated():
    """Test that fewer differences than max doesn't show truncated message."""
    obj1 = {"a": 1, "b": 2}
    obj2 = {"a": 10, "b": 20}

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2, max_differences=10)
    error_msg = str(exc_info.value)

    assert "2 difference(s) found" in error_msg
    assert "output truncated" not in error_msg


def test_assert_objects_equal_max_differences_large_value():
    """Test with large max_differences to get all errors."""
    obj1 = {f"key{i}": i for i in range(20)}
    obj2 = {f"key{i}": i + 100 for i in range(20)}

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2, max_differences=100)
    error_msg = str(exc_info.value)

    assert "20 difference(s) found" in error_msg
    assert "output truncated" not in error_msg


def test_assert_objects_equal_max_differences_nested():
    """Test max_differences with nested structures."""
    obj1 = {"level1": {f"key{i}": i for i in range(15)}}
    obj2 = {"level1": {f"key{i}": i + 100 for i in range(15)}}

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2, max_differences=3)
    error_msg = str(exc_info.value)

    assert "3 difference(s) found" in error_msg
    assert "output truncated" in error_msg


def test_assert_objects_equal_max_differences_list():
    """Test max_differences with list differences."""
    obj1 = list(range(50))
    obj2 = [x + 100 for x in range(50)]

    with pytest.raises(AssertionError) as exc_info:
        utils.assert_objects_equal(obj1, obj2, max_differences=7)
    error_msg = str(exc_info.value)

    assert "7 difference(s) found" in error_msg
    assert "output truncated" in error_msg
