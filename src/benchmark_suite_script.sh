#!/bin/bash

######################################################################## 
## exec_suite
##   Esegue il programma benchmark_suite, prende in ingresso i
##   seguenti parametri:
##   1. implementazione: nel formato "-s udn -a udn"
##   2. suite length: intero, numero di iterazioni del benchmark
##   3. stream size: intero, numero di elementi dello stream
##   4. interarrival time: intero, in cicli di clock
##   5. dataSize: intero
##   6. dataSize: intero
##   ...
########################################################################
function exec_suite {
    if [ $# -eq 0 ] ; then
	fixDataSize="50 96 200 500 1000"
	echo "Warning: no data specified. Used $fixDataSize"
	declare -a dataSize=($fixDataSize)
	local suite_length=2
	local streamSize=100
	local interarrival=1
	local impl="-s udn -a udn"
	#local impl="-s sm_fence -a sm"
    else
	impl="$1" ; shift 
	suite_length=$1 ; shift
	streamSize=$1 ; shift
	interarrival=$1 ; shift
	declare -a dataSize=($*)
    fi

    echo "[INFO] benchmark_suite_script: impl = $impl  suite length = $suite_length stream size = $streamSize interarrial = $interarrival data sizes = ${dataSize[@]}"

    local targetName="benchmark_suite"
    local todo="run_pci_benchmark_suite"
#    local targetName="benchmark_linearmulticast_suite"
#    local todo="run_pci_benchmark_linearmulticast_suite"
#    local targetName="benchmark_pipelinemulticast_suite"
#    local todo="run_pci_benchmark_pipelinemulticast_suite"
    local maxCpu=59
    local i=0

    export ARGS=
    export MONITOR_CMDS='--mount-same outs_benchmark_suite'
#    export MONITOR_CMDS='--mount-same outs_benchmark_linearmulticast_suite'
#    export MONITOR_CMDS='--mount-same outs_benchmark_pipelinemulticast_suite'
#    export MONITOR_OPTIONS='--debug-next'

    echo "[INFO] benchmakri_suite_script: defines=$DEFINES"
    echo "[INFO] benchmark_suite_script: rm ${targetName}.o"
    rm "${targetName}.o"

    for ((i=0; i<${#dataSize[@]}; i++)) ; do 
	ARGS="-l ${suite_length} -K ${streamSize} -i ${interarrival} -M ${dataSize[$i]} ${impl}"
	echo "[INFO] benchmark_suite_script: ARGS=$ARGS"
	make $todo
    done
}

#############################################################################

#DATASIZES="50 96 200 500 1000 2000"
#DATASIZES="1000 2000"
#DATASIZES="50 96"
#DATASIZES="30 42"

#DATASIZES="56 112 224 280 336 392"
#DATASIZES="56 112 168 224 280 336 392"
#DATASIZES="280 336 392"

#############################################################################


#DATASIZES="24"
#DATASIZES="56 112 168 224 280 336 392"
#DEFINES+=' -DASYM_TIME'
#DEFINES+=' -DCALC_TIME'
#DEFINES+=' -DDONT_WRITE_RESULT'
#DEFINES+=' -DDEBUG'
# 0: implementazione canali, 1: suite length, 2: stream size, 3: interarrival time, 4: data sizes

export DEFINES=
#declare -a TEMPI_INTERARRIVO=(181 362 725 1450 2000)
#declare -a TEMPI_INTERARRIVO=(3000 4000 5000 6000 7000 8000 9000 10000)
#declare -a TEMPI_INTERARRIVO=(11000 12000 15000 20000 35000 50000 80000)
declare -a TEMPI_INTERARRIVO=(100000 150000 200000 250000)
DATASIZES="56 168 280"
LENGTH_OF_SUITE=5
LENGTH_OF_STREAM=500

DEFINES='-DGATHER=0 -DDATATYPE=1 -DCALC_TIME'

DATASIZES="56"
#declare -a TEMPI_INTERARRIVO=(20000 10000 7000 4000)
declare -a TEMPI_INTERARRIVO=(4000)
#declare  -a TEMPI_INTERARRIVO=(9000 8000 6000 5000 3000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    # exec_suite "-s udn -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

DATASIZES="168"
#declare -a TEMPI_INTERARRIVO=(80000 50000 35000 20000)
declare -a TEMPI_INTERARRIVO=(4000)
#declare -a TEMPI_INTERARRIVO=(30000 27500 25000 22500 20000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    # exec_suite "-s udn -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

DATASIZES="280"
#declare -a TEMPI_INTERARRIVO=(150000 100000 80000 50000)
declare -a TEMPI_INTERARRIVO=(4000)
#declare -a TEMPI_INTERARRIVO=(70000 60000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    # exec_suite "-s udn -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

################################################################################
#                               EXIT !!!                                       #
exit 0                                                                         #
#                                                                              #
################################################################################


################################################################################

DEFINES='-DGATHER=0 -DDATATYPE=0 -DCALC_TIME'

DATASIZES="56"
declare -a TEMPI_INTERARRIVO=(20000 10000 7000 4000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    sleep 5
done

DATASIZES="168"
declare -a TEMPI_INTERARRIVO=(150000 80000 50000 35000 20000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    sleep 5
done

DATASIZES="280"
declare -a TEMPI_INTERARRIVO=(250000 150000 100000 80000 50000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    sleep 5
done

################################################################################

DEFINES='-DGATHER=0 -DDATATYPE=0 -DMULT_TIME'

DATASIZES="56"
declare -a TEMPI_INTERARRIVO=(20000 10000 7000 4000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    sleep 5
done

DATASIZES="168"
declare -a TEMPI_INTERARRIVO=(150000 80000 50000 35000 20000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    sleep 5
done

DATASIZES="280"
declare -a TEMPI_INTERARRIVO=(250000 150000 100000 80000 50000)
for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    sleep 5
done



################################################################################
#                               EXIT !!!                                       #
exit 0                                                                         #
#                                                                              #
################################################################################


DEFINES='-DGATHER=0 -DDATATYPE=0 -DASYM_TIME'

for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

DEFINES='-DGATHER=0 -DDATATYPE=0 -DCALC_TIME'

for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

############################## #################### ########### ##### #### ### ## #
## Float Type
############################## #################### ########### ##### #### ### ## #

DEFINES='-DGATHER=0 -DDATATYPE=1'

for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

DEFINES='-DGATHER=0 -DDATATYPE=1 -DASYM_TIME'

for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done

DEFINES='-DGATHER=0 -DDATATYPE=1 -DCALC_TIME'

for ((i=0; i<${#TEMPI_INTERARRIVO[@]}; i++)) ; do 
    exec_suite "-s udn -a udn" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
    exec_suite "-s sm_fence -a sm" ${LENGTH_OF_SUITE} ${LENGTH_OF_STREAM} ${TEMPI_INTERARRIVO[$i]} ${DATASIZES}
done