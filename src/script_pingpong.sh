#!/bin/bash

WHITE_RANK_A=0
WHITE_RANK_B=0
BLACK_RANK_A=61
BLACK_RANK_B=1
NITER=100
NSCAMB=1000000
declare -a WHITE_RANK=(0 0 0)
declare -a BLACK_RANK=(1 7 61)

export DEFINES=
export ARGS=
export RUN_PCI=

make clean
make test_pingpong_ch_sim_ref_ad1_macro_udn
make test_pingpong_ch_sim_ref_ad1_macro_sm_null
make test_pingpong_ch_sim_ref_ad1_macro_sm_fence

RUN_PCI=test_pingpong_ch_sim_ref_ad1_macro_udn
for ((i=0; i<${#BLACK_RANK[@]}; i++)) ; do
    ARGS="-w ${WHITE_RANK[$i]} -b ${BLACK_RANK[$i]} -n $NITER -m $NSCAMB"
    make run_pci
done

RUN_PCI=test_pingpong_ch_sim_ref_ad1_macro_sm_fence
for ((i=0; i<${#BLACK_RANK[@]}; i++)) ; do
    ARGS="-w ${WHITE_RANK[$i]} -b ${BLACK_RANK[$i]} -n $NITER -m $NSCAMB"
    make run_pci
done

RUN_PCI=test_pingpong_ch_sim_ref_ad1_macro_sm_null
for ((i=0; i<${#BLACK_RANK[@]}; i++)) ; do
    ARGS="-w ${WHITE_RANK[$i]} -b ${BLACK_RANK[$i]} -n $NITER -m $NSCAMB"
    make run_pci
done

exit 0;

ARGS="-w $WHITE_RANK_B -b $BLACK_RANK_B -n $NITER -m $NSCAMB"
make run_pci

RUN_PCI=test_pingpong_ch_sim_ref_ad1_macro_sm_fence
ARGS="-w $WHITE_RANK_A -b $BLACK_RANK_A -n $NITER -m $NSCAMB"
make run_pci
ARGS="-w $WHITE_RANK_B -b $BLACK_RANK_B -n $NITER -m $NSCAMB"
make run_pci

RUN_PCI=test_pingpong_ch_sim_ref_ad1_macro_sm_null
ARGS="-w $WHITE_RANK_A -b $BLACK_RANK_A -n $NITER -m $NSCAMB"
make run_pci
ARGS="-w $WHITE_RANK_B -b $BLACK_RANK_B -n $NITER -m $NSCAMB"
make run_pci

