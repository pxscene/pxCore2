#!/bin/bash
# This script will run the embedded spark.sh within the OSX package and enable logging to console
cd Spark.app/Contents/MacOS
DS=log ./spark.sh 
