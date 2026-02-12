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
import difflib
import os
import pprint
import re
import sys
import xml.sax.saxutils as XSS
from pathlib import Path
from subprocess import PIPE, Popen
from typing import Any, Dict, List


class CodeWrapper:
    def __init__(self, code, language) -> None:
        self.code = code
        self.language = language

    def __str__(self) -> str:
        return f'<pre><code class="language-{self.language}">{XSS.escape(self.code)}</code></pre>'


def platform_matches(unsupported_platforms: List[str]):
    platform_id = get_platform()
    return any(re.search(p, platform_id) for p in unsupported_platforms)


# merci https://stackoverflow.com/a/33300001
def str_representer(dumper, data):
    if "\n" in data:
        return dumper.represent_scalar("tag:yaml.org,2002:str", data, style="|")
    return dumper.represent_scalar("tag:yaml.org,2002:str", data)


def kill_tree(ppid, sig):
    """
    Send a signal to a process and all its child processes (starting from the
    leaves).
    """
    ps_cmd = ["ps", "--no-headers", "-o", "pid", "--ppid", str(ppid)]
    # Note: start in a clean env to avoid a freeze with libasan.so
    # See https://sourceware.org/bugzilla/show_bug.cgi?id=27653
    get_children = Popen(ps_cmd, stdout=PIPE, stderr=PIPE, env={})
    children = map(int, get_children.communicate()[0].split())
    for child in children:
        kill_tree(child, sig)
    try:
        os.kill(ppid, sig)
    except OSError as err:
        if err.errno != 3:  # No such process
            raise


def which(executable):
    """
    Locates an executable in the executables path ($PATH) and returns the full
    path to it.  An application is looked for with or without the '.exe' suffix.
    If the executable cannot be found, None is returned
    """
    if os.path.isabs(executable):
        if not os.path.isfile(executable):
            if executable.endswith(".exe"):
                if os.path.isfile(executable[:-4]):
                    return executable[:-4]
            else:
                executable = os.path.split(executable)[1]
        else:
            return executable
    for d in os.environ.get("PATH").split(os.pathsep):
        fullpath = os.path.join(d, executable)
        if os.path.isfile(fullpath):
            return fullpath
        elif executable.endswith(".exe") and os.path.isfile(fullpath[:-4]):
            return fullpath[:-4]
    return None


def get_platform():
    """
    Return the platform Id defined in CMTCONFIG or SCRAM_ARCH.
    """
    arch = "None"
    # check architecture name
    if "BINARY_TAG" in os.environ:
        arch = os.environ["BINARY_TAG"]
    elif "CMTCONFIG" in os.environ:
        arch = os.environ["CMTCONFIG"]
    elif "SCRAM_ARCH" in os.environ:
        arch = os.environ["SCRAM_ARCH"]
    elif os.environ.get("ENV_CMAKE_BUILD_TYPE", "") in (
        "Debug",  # -O0 -g
        "FastDebug",  # -Og -g (LHCb only)
        "Developer",  # same as Debug, but with many warnings enabled
        "",  # no options (equivalent to -O0)
    ):
        arch = "unknown-dbg"
    elif os.environ.get("ENV_CMAKE_BUILD_TYPE", "") in (
        "Release",  # -O3 -DNDEBUG
        "MinSizeRel",  # -Os -DNDEBUG
        "RelWithDebInfo",  # -O2 -g -DNDEBUG (-O3 for LHCb)
    ):
        arch = "unknown-opt"
    return arch


def expand_reference_file_name(reference):
    # if no file is passed, do nothing
    if not reference:
        return reference

    # function to split an extension in constituents parts
    def platform_split(p):
        return set(re.split(r"[-+]", p)) if p else set()

    # get all the files whose name start with the reference filename
    dirname, basename = os.path.split(reference)
    if not dirname:
        dirname = "."

    for suffix in (".yaml", ".yml"):
        if basename.endswith(suffix):
            prefix = f"{basename[:-(len(suffix))]}."
            break
    else:
        # no special suffix matched, fallback on no suffix
        prefix = f"{basename}."
        suffix = ""

    flags_slice = slice(len(prefix), -len(suffix) if suffix else None)

    def get_flags(name):
        """
        Extract the platform flags from a filename, return None if name does not match prefix and suffix
        """
        if name.startswith(prefix) and name.endswith(suffix):
            return platform_split(name[flags_slice])
        return None

    platform = platform_split(get_platform())
    if "do0" in platform:
        platform.add("dbg")
    candidates = [
        (len(flags), name)
        for flags, name in [
            (get_flags(name), name)
            for name in (os.listdir(dirname) if os.path.isdir(dirname) else [])
        ]
        if flags and platform.issuperset(flags)
    ]
    if candidates:  # take the one with highest matching
        # FIXME: it is not possible to say if x86_64-slc5-gcc43-dbg
        #        has to use yaml.x86_64-gcc43 or yaml.slc5-dbg
        candidates.sort()
        return os.path.join(dirname, candidates[-1][1])
    return os.path.join(dirname, basename)


def filter_dict(d: Dict[str, Any], ignore_re: re.Pattern) -> Dict[str, Any]:
    """
    Recursively filter out keys from the dictionary that match the ignore pattern.
    """
    filteredDict = {}
    for k, v in d.items():
        if not ignore_re.match(k):
            if isinstance(v, dict):
                filteredDict[k] = filter_dict(v, ignore_re)
            else:
                filteredDict[k] = v
    return filteredDict


def compare_dicts(d1: Dict[str, Any], d2: Dict[str, Any], ignore_re: str = None) -> str:
    """
    Compare two dictionaries and return the diff as a string, ignoring keys that match the regex.
    """
    ignore_re = re.compile(ignore_re)
    filtered_d1 = filter_dict(d1, ignore_re)
    filtered_d2 = filter_dict(d2, ignore_re)

    return "\n" + "\n".join(
        difflib.unified_diff(
            pprint.pformat(filtered_d1).splitlines(),
            pprint.pformat(filtered_d2).splitlines(),
        )
    )


# signature of the print-out of the histograms
h_count_re = re.compile(
    r"^(.*)(?:SUCCESS|INFO)\s+Booked (\d+) Histogram\(s\) :\s+([\s\w=-]*)"
)


def _parse_ttree_summary(lines, pos):
    """
    Parse the TTree summary table in lines, starting from pos.
    Returns a tuple with the dictionary with the digested informations and the
    position of the first line after the summary.
    """
    result = {}
    i = pos + 1  # first line is a sequence of '*'
    count = len(lines)

    def splitcols(l):
        return [f.strip() for f in l.strip("*\n").split(":", 2)]

    def parseblock(ll):
        r = {}
        delta_i = 0
        cols = splitcols(ll[0])

        if len(ll) == 3:
            # default one line name/title
            r["Name"], r["Title"] = cols[1:]
        elif len(ll) == 4:
            # in case title is moved to next line due to too long name
            delta_i = 1
            r["Name"] = cols[1]
            r["Title"] = ll[1].strip("*\n").split("|")[1].strip()
        else:
            assert False

        cols = splitcols(ll[1 + delta_i])
        r["Entries"] = int(cols[1])

        sizes = cols[2].split()
        r["Total size"] = int(sizes[2])
        if sizes[-1] == "memory":
            r["File size"] = 0
        else:
            r["File size"] = int(sizes[-1])

        cols = splitcols(ll[2 + delta_i])
        sizes = cols[2].split()
        if cols[0] == "Baskets":
            r["Baskets"] = int(cols[1])
            r["Basket size"] = int(sizes[2])
        r["Compression"] = float(sizes[-1])

        return r

    def nextblock(lines, i):
        delta_i = 1
        dots = re.compile(r"^\.+$")
        stars = re.compile(r"^\*+$")
        count = len(lines)
        while (
            i + delta_i < count
            and not dots.match(lines[i + delta_i][1:-1])
            and not stars.match(lines[i + delta_i])
        ):
            delta_i += 1
        return i + delta_i

    if i < (count - 3) and lines[i].startswith("*Tree"):
        i_nextblock = nextblock(lines, i)
        result = parseblock(lines[i:i_nextblock])
        result["Branches"] = {}
        i = i_nextblock + 1
        while i < (count - 3) and lines[i].startswith("*Br"):
            if i < (count - 2) and lines[i].startswith("*Branch "):
                # skip branch header
                i += 3
                continue
            i_nextblock = nextblock(lines, i)
            if i_nextblock >= count:
                break
            branch = parseblock(lines[i:i_nextblock])
            result["Branches"][branch["Name"]] = branch
            i = i_nextblock + 1

    return (result, i)


def _parse_histos_summary(lines, pos):
    """
    Extract the histograms infos from the lines starting at pos.
    Returns the position of the first line after the summary block.
    """
    global h_count_re
    h_table_head = re.compile(
        r'(?:SUCCESS|INFO)\s+(1D|2D|3D|1D profile|2D profile) histograms in directory\s+"(\w*)"'
    )
    h_short_summ = re.compile(r"ID=([^\"]+)\s+\"([^\"]*)\"\s+(.*)")

    nlines = len(lines)

    # decode header
    m = h_count_re.search(lines[pos])
    name = m.group(1).strip()
    total = int(m.group(2))
    header = {}
    for k, v in [x.split("=") for x in m.group(3).split()]:
        header[k] = int(v)
    pos += 1
    header["Total"] = total

    summ = {}
    while pos < nlines:
        m = h_table_head.search(lines[pos])
        if m:
            t, d = m.groups(1)  # type and directory
            t = t.replace(" profile", "Prof")
            pos += 1
            if pos < nlines:
                l = lines[pos]
            else:
                l = ""
            cont = {}
            if l.startswith(" | ID"):
                # table format
                titles = [x.strip() for x in l.split("|")][1:-1]
                pos += 1
                while pos < nlines and lines[pos].startswith(" |"):
                    l = lines[pos]
                    values = [x.strip() for x in l.split("|")][1:]
                    hcont = {}
                    for i in range(len(titles)):
                        hcont[titles[i]] = values[i]
                    cont[hcont["ID"]] = hcont
                    pos += 1
            elif l.startswith(" ID="):
                while pos < nlines and lines[pos].startswith(" ID="):
                    values = [
                        x.strip() for x in h_short_summ.search(lines[pos]).groups()
                    ]
                    cont[values[0]] = values
                    pos += 1
            else:  # not interpreted
                raise RuntimeError("Cannot understand line %d: '%s'" % (pos, l))
            if d not in summ:
                summ[d] = {}
            summ[d][t] = cont
            summ[d]["header"] = header
        else:
            break
    if not summ:
        # If the full table is not present, we use only the header
        summ[name] = {"header": header}
    return summ, pos


def find_histos_summaries(stdout):
    """
    Scan stdout to find ROOT Histogram summaries and digest them.
    """
    outlines = stdout.splitlines() if hasattr(stdout, "splitlines") else stdout
    nlines = len(outlines) - 1
    summaries = {}
    global h_count_re

    pos = 0
    while pos < nlines:
        summ = {}
        # find first line of block:
        match = h_count_re.search(outlines[pos])
        while pos < nlines and not match:
            pos += 1
            match = h_count_re.search(outlines[pos])
        if match:
            summ, pos = _parse_histos_summary(outlines, pos)
        summaries.update(summ)
    return summaries


def find_ttree_summaries(stdout):
    """
    Scan stdout to find ROOT TTree summaries and digest them.
    """
    stars = re.compile(r"^\*+$")
    outlines = stdout.splitlines() if hasattr(stdout, "splitlines") else stdout
    nlines = len(outlines)
    trees = {}

    i = 0
    while i < nlines:  # loop over the output
        # look for
        while i < nlines and not stars.match(outlines[i]):
            i += 1
        if i < nlines:
            tree, i = _parse_ttree_summary(outlines, i)
            if tree:
                trees[tree["Name"]] = tree

    return trees


def file_path_for_class(cls):
    return Path(sys.modules[cls.__module__].__file__)


def _format_path(path: List[str]) -> str:
    """Format a path list as a bracket-notation string for error messages."""
    if not path:
        return "root"
    return "".join(f"[{p!r}]" for p in path)


def _floats_close(a: float, b: float, rtol: float, atol: float) -> bool:
    """Check if two floats are close within relative and absolute tolerance."""
    # Handle exact equality first (includes inf == inf case)
    if a == b:
        return True
    # Handle cases where difference is infinite (e.g., inf vs -inf)
    diff = abs(a - b)
    if diff == float("inf"):
        return False
    return diff <= atol + rtol * max(abs(a), abs(b))


def _compare(
    obj1: Any,
    obj2: Any,
    path: List[str],
    rtol: float,
    atol: float,
    max_differences: int,
    differences: List[str],
) -> None:
    """
    Recursively compare two objects and append difference descriptions to the list.

    Handles dicts, lists, floats (with tolerance), and other types (exact equality).
    Stops early if max_differences is reached.
    """
    if len(differences) >= max_differences:
        return

    if type(obj1) is not type(obj2):
        differences.append(
            f"Type mismatch at {_format_path(path)}: {type(obj1).__name__} vs {type(obj2).__name__}"
        )
    elif isinstance(obj1, dict):
        keys1, keys2 = set(obj1.keys()), set(obj2.keys())
        for key in keys1 - keys2:
            if len(differences) >= max_differences:
                return
            differences.append(
                f"Extra key in first object at {_format_path(path)}: {key!r}"
            )
        for key in keys2 - keys1:
            if len(differences) >= max_differences:
                return
            differences.append(
                f"Extra key in second object at {_format_path(path)}: {key!r}"
            )
        for key in keys1 & keys2:
            if len(differences) >= max_differences:
                return
            _compare(
                obj1[key],
                obj2[key],
                path + [key],
                rtol,
                atol,
                max_differences,
                differences,
            )
    elif isinstance(obj1, list):
        if len(obj1) != len(obj2):
            differences.append(
                f"List length mismatch at {_format_path(path)}: {len(obj1)} vs {len(obj2)}"
            )
        else:
            for i, (item1, item2) in enumerate(zip(obj1, obj2)):
                if len(differences) >= max_differences:
                    return
                _compare(
                    item1, item2, path + [i], rtol, atol, max_differences, differences
                )
    elif isinstance(obj1, float):
        if not _floats_close(obj1, obj2, rtol, atol):
            differences.append(
                f"Float mismatch at {_format_path(path)}: {obj1} vs {obj2} "
                f"(diff={abs(obj1 - obj2)}, rtol={rtol}, atol={atol})"
            )
    elif obj1 != obj2:
        differences.append(
            f"Value mismatch at {_format_path(path)}: {obj1!r} vs {obj2!r}"
        )


def assert_objects_equal(
    obj1: Any,
    obj2: Any,
    rtol: float = 1e-9,
    atol: float = 0.0,
    max_differences: int = 10,
) -> None:
    """
    Assert that two JSON-like objects are equal, with tolerance for floating-point values.

    Args:
        obj1: First object to compare (can be dict, list, or primitive types)
        obj2: Second object to compare
        rtol: Relative tolerance for float comparisons (default: 1e-9)
        atol: Absolute tolerance for float comparisons (default: 0.0)
        max_differences: Maximum number of differences to report before stopping (default: 10)

    Raises:
        AssertionError: If the objects are not equal, with a detailed message
                       showing differences found (up to max_differences).

    Example:
        >>> assert_objects_equal({"a": 1.0}, {"a": 1.0})  # passes
        >>> assert_objects_equal({"a": 1.0}, {"a": 1.001}, atol=0.01)  # passes
        >>> assert_objects_equal({"a": 1}, {"a": 2})  # raises AssertionError
    """
    # pytest will hide this frame from the reported traceback
    __tracebackhide__ = True

    differences: List[str] = []
    _compare(obj1, obj2, [], rtol, atol, max_differences, differences)
    if differences:
        truncated = len(differences) >= max_differences
        msg = f"Objects differ ({len(differences)} difference(s) found"
        if truncated:
            msg += ", output truncated"
        msg += "):\n"
        msg += "\n".join(f"  - {d}" for d in differences)
        assert False, msg
