#!/bin/sh
CCACHE_DISABLE=true sh $1
retval=$?
if [ "$retval" -eq 0 ]
then
sh $2
retval=$?
fi
return $retval;
