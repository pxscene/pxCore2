#!/bin/sh
export CCACHE_DISABLE=true
retval=0
for var in "$@"
do
sh $var
retval=$?
if [ "$retval" -eq 0 ]
then
echo "$var succeeded !!!"
else
echo "$var failed !!!"
exit $retval;
fi
done
exit $retval;
