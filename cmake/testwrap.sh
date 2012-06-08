#!/bin/sh
[ -f $1 ] && . $1
shift
cd $1
shift
exec $*
