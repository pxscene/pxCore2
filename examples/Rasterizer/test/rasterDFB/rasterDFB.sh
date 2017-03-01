export NODE_PATH=/mnt/nfs/env
export LD_LIBRARY_PATH=/lib:/mnt/nfs/env/:$LD_LIBRARY_PATH
export RT_LOG_LEVEL=error

./rasterDFB $@ 2>/dev/null

#printf "\n\n"

#./rasterDFB $@


#./gdb_rng15 ./rasterDFB $@
