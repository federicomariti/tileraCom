#!/bin/bash


#declare -a MASK=(0 0 0 0 0 0 1 0 0 1)
declare -a MASK=(1 0 0 1 1 1 1 0 1 1)
#declare -a MASK=(1 0 0 0 0 0 0 0 0 0)
#declare -a MASK=(0 1 1 1 1 0 0 0 0 0)

declare -a TODO=( \
    run_pci_meter_ch_asymin_ref_ad1_udn \
    run_pci_meter_ch_asymin_ref_ad1_sm_struct \
    run_pci_meter_ch_asymin_ref_ad1_sm_struct_b \
    run_pci_meter_ch_asymin_ref_ad1_sm_param \
    run_pci_meter_ch_asymin_ref_ad1_sm_param_b \
    run_pci_meter_ch_sym_ref_ad1_udn \
    run_pci_meter_ch_sym_ref_ad1_sm_fence \
    run_pci_meter_ch_sym_ref_ad1_sm_null \
    run_pci_meter_ch_sym_ref_ad1_sm_nullack \
    run_pci_meter_ch_sym_ref_ad1_sm_no \
)

declare -a WHITE_RANK=(8 8 7)
declare -a BLACK_RANK=(9 23 56)

export DEFINES=
export ARGS=
export RUN_PCI=
export MONITOR_CMDS="--mount-same outs_meter"
export MONITOR_OPTIONS=
#export MONITOR_OPTIONS="--debug-next"

#export DEFINES='-DHANDLE_DEADLOCK'

function work {

    make clean

    for ((j=0; j<${#TODO[@]}; j++)) ; do
	if [ $((MASK[$j])) -eq 0 ] ; then continue ; fi

	for ((i=0; i<${#BLACK_RANK[@]}; i++)) ; do

	    for ((k=0; k<${#NSCAMB[@]}; k++)) ; do
		ARGS="-w ${WHITE_RANK[$i]} -b ${BLACK_RANK[$i]} -n ${NITER[$k]} -m ${NSCAMB[$k]}"
		make ${TODO[$j]}
	    done

	done

    done
}


declare -a NITER=( 1     1      1)
declare -a NSCAMB=(10000 100000 1000)
#declare -a NITER=( 1    )
#declare -a NSCAMB=(10000)
EXTERNAL_ITERATIONS=5


for ((v=0; v<EXTERNAL_ITERATIONS; v++)) ; do

    for ((u=2; u<3; u++)) ; do

	DEFINES="-DTEST_TYPE=$u"

	work
    done

done