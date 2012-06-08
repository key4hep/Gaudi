#!/bin/csh
[ -f $1 ] && source $1
shift
cd $1
shift
exec $*
