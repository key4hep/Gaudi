Code Style Conventions {#code-style}
======================

Having a common formatting convention throughout the code makes it more easy to
follow for every developer.

Ideally, every source file in %Gaudi should follow the same conventions, but with
many contributors and no tool to enforce the conventions, the sources tend to
get chaotic. So, in the context of [GAUDI-1176](https://its.cern.ch/jira/browse/GAUDI-1176),
we added customized rules for the tool [`clang-format`](http://clang.llvm.org/docs/ClangFormat.html)
and helpers to use it.

We also imported a tool from Python source tree (`reindent.py`) that is used to
fix the indentation of Python scripts to use the standard 4-spaces indentation.

To avoid massive changes that are only changing the format of files, we
encourage developers and contributors to apply the format rules to the files
they modify.


Formatting the code
-------------------

There are several ways to apply the formating rules via the `clang-format` tool.
For example, the [`clang-format` documentation page](http://clang.llvm.org/docs/ClangFormat.html)
shows how to integrate its use in popular editors like Vim and Emacs, and IDEs
like [Qt Creator](https://www.qt.io/ide/) have plugins to call automatically
`clang-format`. In this cases, to format %Gaudi sources, one you specify the
style _file_.

For the cases where integration with an editor or an IDE is not possible, or
when the command line is preferred, we added a wrapper script in the build
directory (`apply-formatting`) that simplifies the call to `clang-format` and
`reindent.py`. To use it you need to pass to it the path to the files you want
to reformat (only C++ and Python files are taken into account). For example to
apply the formatting rules to all files modified between two commits one can do
something like:

    cd gaudi_src_dir
    gaudi_build_dir/apply_formatting $(git diff --name-only origin/master HEAD)
