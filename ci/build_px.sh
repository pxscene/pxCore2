#This script is changed to accomodate running multiple scripts
#!/bin/sh
export CCACHE_DISABLE=true
retval=0
for var in "$@"
do
echo "Starting $var..."
begin=$(date +%s)
sh $var
retval=$?
end=$(date +%s)
tottime=$(expr $end - $begin)
echo "script $var took $tottime seconds"
if [ "$retval" -eq 0 ]
then
echo "$var succeeded !!!"
else
echo "$var failed !!!"
exit $retval;
fi
done
exit $retval;
