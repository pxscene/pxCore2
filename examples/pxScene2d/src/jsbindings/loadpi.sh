
export HOR=`tvservice -j -s | sed -e s/.*],//g -e s/@.*//g -e s/\ //g | sed -e s/x.*//g -e s/\ *//g`
export VER=`tvservice -j -s | sed -e s/.*],//g -e s/@.*//g -e s/\ //g | sed -e s/.*x//g -e s/\ *//g`
echo $HOR
echo $VER
./load.sh $1 
