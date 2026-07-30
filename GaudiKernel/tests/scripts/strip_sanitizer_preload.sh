#!/bin/sh
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
#
# Removes the given sanitizer runtime libraries from LD_PRELOAD, preserving any
# other preload already present, then execs the real command. Used by tests
# that are deliberately built without support for one or more sanitizers (e.g.
# because they define their own allocation operators), where LD_PRELOAD may
# still carry that sanitizer's runtime ambiently (set outside the checkout,
# before ctest itself runs) for every other test. Preloading it into such a
# test anyway aborts immediately with "<Sanitizer> runtime does not come first
# in initial library list", not a real test failure.
#
# CMake's ENVIRONMENT/ENVIRONMENT_MODIFICATION test properties can only set,
# append to, or reset an environment variable wholesale -- there is no way to
# remove individual entries from an existing value using them alone, hence
# this script.
#
# Usage: strip_sanitizer_preload.sh <lib1> [<lib2> ...] -- <command> [<arg> ...]
# Each <libN> is matched against both the bare filename and a full path ending
# in "/<libN>" (LD_PRELOAD entries may be either, depending on how they were
# resolved upstream).

libs=
while [ "$#" -gt 0 ] && [ "$1" != "--" ]; do
  libs="${libs:+$libs }$1"
  shift
done
shift # drop the "--" separator

new=
save_ifs=$IFS
IFS=:
for entry in $LD_PRELOAD; do
  # $LD_PRELOAD was already split into $entry above, based on IFS=: -- restore the
  # default IFS for the rest of this iteration so the (space-separated) $libs list
  # below splits correctly instead of being read as a single word.
  IFS=$save_ifs
  skip=
  for lib in $libs; do
    case "$entry" in
      "$lib" | */"$lib") skip=1 ;;
    esac
  done
  [ -n "$skip" ] || new="${new:+$new:}$entry"
  IFS=:
done
IFS=$save_ifs

LD_PRELOAD="$new" exec "$@"
