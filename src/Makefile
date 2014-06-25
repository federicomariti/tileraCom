ifndef TILERA_ROOT
$(error The 'TILERA_ROOT' environment variable is not set.)
endif

BIN = $(TILERA_ROOT)/bin/

CC = $(BIN)tile-gcc
CFLAGS = -g3 $(DEFINES) -O3 -Wall -pedantic $(DEBUG) 
LOADLIBES = -ltmc -lpthread -lm


TILE_MONITOR = $(BIN)tile-monitor

EXECUTABLES = \
test_ch_sym_ref_sm \
test_ch_sym_ref_sm_no \
test_ch_sym_ref_udn \
test_pingpong_ch_sym_ref_ad1_macro_udn\
test_pingpong_ch_sym_ref_ad1_macro_sm_fence\
test_pingpong_ch_sym_ref_ad1_macro_sm_null\
meter_ch_sym_ref_ad1_udn \
meter_ch_sym_ref_ad1_sm_null \
meter_ch_sym_ref_ad1_sm_fence\
meter_ch_sym_ref_ad1_sm_nullack\
meter_ch_sym_ref_ad1_sm_no\
meter_ch_asymin_ref_ad1_udn\
meter_ch_asymin_ref_ad1_sm_struct\
meter_ch_asymin_ref_ad1_sm_struct_b\
meter_ch_asymin_ref_ad1_sm_param\
meter_ch_asymin_ref_ad1_sm_param_b\
meter_ch_asymin_ref_ad1_sm_pthr\
verify_dataplane\
mytest\
mytest_macro\
benchmark\
benchmark_float

OBJECTS=\
ch_sym_ref_ad1_udn.o \
ch_sym_ref_ad1_sm_fence.o \
ch_sym_ref_ad1_sm_null.o \
ch_sym_ref_ad1_sm_nullack.o \
ch_sym_ref_ad1_sm_no.o \
ch_asymin_ref_ad1_udn.o \
ch_asymin_ref_ad1_sm_struct.o \
ch_asymin_ref_ad1_sm_struct_b.o \
ch_asymin_ref_ad1_sm_param.o \
ch_asymin_ref_ad1_sm_param_b.o 

#\
#ch_sym_ref_udn.o \
#ch_asymin_ref_udn.o

ifndef ARGS
ARGS =
endif

ifndef MONITOR_CMDS
MONITOR_CMDS =
endif

ifndef MONITOR_OPTIONS
MONITOR_OPTIONS =
endif

ifndef MONITOR_PRE
MONITOR_PRE =
endif

ifdef PROFILE
PROFILE_PRE=  \
--profile-init \
--profile-kernel \
--profile-flags '--callgraph=20' \
--profile-events "--event=ONE:4000000 --event=MP_BUNDLE_RETIRED:4000000 --event=RD_MISS:10000 --event=WR_MISS:10000" \
--profile-start
PROFILE_POST= \
--profile-stop \
--profile-capture samples \
--profile-analyze samples
endif

MONITOR_COMMON_ARGS = \
$(MONITOR_OPTIONS)\
--batch-mode \
--hvc vmlinux-pci-nonet.hvc \
--hvx dataplane=0-61 \
--mkdir /opt/test \
--cd /opt/test \
$(MONITOR_CMDS)\
--upload $^ $^\
$(PROFILE_PRE)\
--run -+- $^ $(ARGS) -+-\
$(PROFILE_POST) \
--

SIM_MONITOR_COMMON_ARGS = \
$(MONITOR_OPTIONS) \
--batch-mode \
--hvx dataplane=0-61 \
--upload $^ $^\
--run -+- $^ $(ARGS) -+-\

MONITOR_COMMON_ARGS_2 = \
$(MONITOR_COMMON_ARGS_PRE)\
--batch-mode \
--hvc vmlinux-pci-nonet.hvc \
--hvx dataplane=0-61 \
--mkdir /opt/test \
--cd /opt/test \
--upload $(RUN_PCI) $(RUN_PCI)\
$(PROFILE_PRE)\
--run -+- $(RUN_PCI) $(ARGS) -+-\
$(PROFILE_POST) \
--

# For faster execution, also specify the "--functional" option to run
# the simulator in functional mode, as opposed to the default
# timing-accurate mode.
#   --tile 4x4@0,0
#   --tile 5,0 \
#   --tile 1,0
SIMULATOR_ARGS = \
--image tile64 \
--sim-arg --profile-disable-at-startup --sim-args -+- --xml-profile $(PROFILEDIR)/profile.xml -+- \
--sim-args -+- --output-file $(PROFILEDIR)/ttv -+-

PCI_ARGS = \
--pci \
--tile 8x8 \

.PHONY: all clean $(EXECUTABLES)

all: clean $(OBJECTS)

clean:
	rm -f *.o *~ $(EXECUTABLES) a.out

############################################################################

# OLD: generic ASYNCH DEGREE

ch_sym_ref_sm.o: ch_sym_ref_sm.c ch_sym_ref_sm.h
test_ch_sym_ref_sm.o: test_ch_sym_ref_sm.c ch_sym_ref_sm.h

test_ch_sym_ref_sm: test_ch_sym_ref_sm.o ch_sym_ref_sm.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

run_pci_test_ch_sym_ref_sm: test_ch_sym_ref_sm
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS) 
run_sym_test_ch_sym_ref_sm: test_ch_sym_ref_sm
	$(TILE_MONITOR) $(SIMULATOR_ARGS) $(SIM_MONITOR_COMMON_ARGS)
run_sym_func_test_ch_sym_ref_sm: test_ch_sym_ref_sm
	$(TILE_MONITOR) --functional $(SIMULATOR_ARGS) $(SIM_MONITOR_COMMON_ARGS)

############################################################################

# OLD: generic ASYNCH DEGREE

ch_sym_ref_sm_no.o: ch_sym_ref_sm_no.c ch_sym_ref_sm.h
test_ch_sym_ref_sm_no.o: test_ch_sym_ref_sm.c ch_sym_ref_sm.h

test_ch_sym_ref_sm_no: test_ch_sym_ref_sm.o ch_sym_ref_sm_no.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

run_pci_test_ch_sym_ref_sm_no: test_ch_sym_ref_sm_no
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

############################################################################

# OLD: generic ASYNCH DEGREE

ch_sym_ref_udn.o: ch_sym_ref_udn.c ch_sym_ref_udn.h

test_ch_sym_ref_udn.o: test_ch_sym_ref_udn.c ch_sym_ref_udn.h
test_ch_sym_ref_udn: test_ch_sym_ref_udn.o ch_sym_ref_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_test_ch_sym_ref_udn: test_ch_sym_ref_udn
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

############################################################################

# channels

ch_sym_ref_ad1_sm.o: ch_sym_ref_ad1_sm_private.h ch_sym_ref_ad1_sm.h ch_sym_ref_ad1_sm.c
ch_sym_ref_ad1_sm_null.o: ch_sym_ref_ad1_sm_null_private.h ch_sym_ref_ad1_sm_null.h ch_sym_ref_ad1_sm_null.c
ch_sym_ref_ad1_sm_fence.o: ch_sym_ref_ad1_sm_fence_private.h ch_sym_ref_ad1_sm_fence.h ch_sym_ref_ad1_sm_fence.c
ch_sym_ref_ad1_sm_nullack.o: ch_sym_ref_ad1_sm_nullack_private.h ch_sym_ref_ad1_sm_nullack.h ch_sym_ref_ad1_sm_nullack.c
ch_sym_ref_ad1_sm_no.o: ch_sym_ref_ad1_sm_no_private.h ch_sym_ref_ad1_sm_no.h ch_sym_ref_ad1_sm_no.c
ch_sym_ref_ad1_udn.o: ch_sym_ref_ad1_udn_private.h ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_udn.c
ch_asymin_ref_ad1_udn.o: ch_asymin_ref_ad1_udn_private.h ch_asymin_ref_ad1_udn.h ch_asymin_ref_ad1_udn.c
ch_asymin_ref_ad1_sm_struct.o: ch_asymin_ref_ad1_sm_struct_private.h ch_asymin_ref_ad1_sm_struct.h ch_asymin_ref_ad1_sm_struct.c
ch_asymin_ref_ad1_sm_struct_b.o: ch_asymin_ref_ad1_sm_struct_b_private.h ch_asymin_ref_ad1_sm_struct_b.h ch_asymin_ref_ad1_sm_struct_b.c
ch_asymin_ref_ad1_sm_param.o: ch_asymin_ref_ad1_sm_param_private.h ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_param.c
ch_asymin_ref_ad1_sm_param_b.o: ch_asymin_ref_ad1_sm_param_b_private.h ch_asymin_ref_ad1_sm_param_b.h ch_asymin_ref_ad1_sm_param_b.c

# meter compile and runnable for each channel

meter_ch_sym_ref_ad1_udn.o: meter_ch_sym_ref_ad1.c ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_SYM_UDN -o $@
meter_ch_sym_ref_ad1_udn: meter_ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_sym_ref_ad1_udn: meter_ch_sym_ref_ad1_udn
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_sym_ref_ad1_sm_fence.o: meter_ch_sym_ref_ad1.c ch_sym_ref_ad1_sm_fence.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_SYM_SM_FENCE -o $@
meter_ch_sym_ref_ad1_sm_fence: meter_ch_sym_ref_ad1_sm_fence.o\
	ch_sym_ref_ad1_sm_fence.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_sym_ref_ad1_sm_fence: meter_ch_sym_ref_ad1_sm_fence
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_sym_ref_ad1_sm_null.o: meter_ch_sym_ref_ad1.c ch_sym_ref_ad1_sm_null.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_SYM_SM_NULL -o $@
meter_ch_sym_ref_ad1_sm_null: meter_ch_sym_ref_ad1_sm_null.o\
	ch_sym_ref_ad1_sm_null.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_sym_ref_ad1_sm_null: meter_ch_sym_ref_ad1_sm_null
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_sym_ref_ad1_sm_nullack.o: meter_ch_sym_ref_ad1.c\
	ch_sym_ref_ad1_sm_nullack.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_SYM_SM_NULLACK -o $@
meter_ch_sym_ref_ad1_sm_nullack: meter_ch_sym_ref_ad1_sm_nullack.o\
	ch_sym_ref_ad1_sm_nullack.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_sym_ref_ad1_sm_nullack: meter_ch_sym_ref_ad1_sm_nullack
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_sym_ref_ad1_sm_no.o: meter_ch_sym_ref_ad1.c ch_sym_ref_ad1_sm_no.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_SYM_SM_NO -o $@
meter_ch_sym_ref_ad1_sm_no: meter_ch_sym_ref_ad1_sm_no.o\
	ch_sym_ref_ad1_sm_no.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_sym_ref_ad1_sm_no: meter_ch_sym_ref_ad1_sm_no
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_asymin_ref_ad1_udn.o: meter_ch_sym_ref_ad1.c ch_asymin_ref_ad1_udn.h \
	ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_ASYMIN_UDN -o $@
meter_ch_asymin_ref_ad1_udn: meter_ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_udn.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_asymin_ref_ad1_udn: meter_ch_asymin_ref_ad1_udn
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_asymin_ref_ad1_sm_struct.o: meter_ch_sym_ref_ad1.c \
	ch_asymin_ref_ad1_sm_struct.h ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_ASYMIN_SM_STRUCT -o $@
meter_ch_asymin_ref_ad1_sm_struct: meter_ch_asymin_ref_ad1_sm_struct.o \
	ch_asymin_ref_ad1_sm_struct.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_asymin_ref_ad1_sm_struct: meter_ch_asymin_ref_ad1_sm_struct
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_asymin_ref_ad1_sm_struct_b.o: meter_ch_sym_ref_ad1.c \
	ch_asymin_ref_ad1_sm_struct_b.h ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_ASYMIN_SM_STRUCT_B -o $@
meter_ch_asymin_ref_ad1_sm_struct_b: meter_ch_asymin_ref_ad1_sm_struct_b.o \
	ch_asymin_ref_ad1_sm_struct_b.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_asymin_ref_ad1_sm_struct_b: meter_ch_asymin_ref_ad1_sm_struct_b
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_asymin_ref_ad1_sm_param.o: meter_ch_sym_ref_ad1.c \
	ch_asymin_ref_ad1_sm_param.h ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_ASYMIN_SM_PARAM -o $@
meter_ch_asymin_ref_ad1_sm_param: meter_ch_asymin_ref_ad1_sm_param.o \
	ch_asymin_ref_ad1_sm_param.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_asymin_ref_ad1_sm_param: meter_ch_asymin_ref_ad1_sm_param
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_asymin_ref_ad1_sm_param_b.o: meter_ch_sym_ref_ad1.c \
	ch_asymin_ref_ad1_sm_param_b.h ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_ASYMIN_SM_PARAM_B -o $@
meter_ch_asymin_ref_ad1_sm_param_b: meter_ch_asymin_ref_ad1_sm_param_b.o \
	ch_asymin_ref_ad1_sm_param_b.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_asymin_ref_ad1_sm_param_b: meter_ch_asymin_ref_ad1_sm_param_b
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

meter_ch_asymin_ref_ad1_sm_pthr.o: meter_ch_sym_ref_ad1.c \
	ch_asymin_ref_ad1_sm_pthr.h ch_sym_ref_ad1_udn.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -DMETER_ASYMIN_SM_PTHR -o $@
meter_ch_asymin_ref_ad1_sm_pthr: meter_ch_asymin_ref_ad1_sm_pthr.o \
	ch_asymin_ref_ad1_sm_pthr.o ch_sym_ref_ad1_udn.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_meter_ch_asymin_ref_ad1_sm_pthr: meter_ch_asymin_ref_ad1_sm_pthr
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)



verify_dataplane.o: verify_dataplane.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
verify_dataplane: verify_dataplane.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_verify_dataplane: verify_dataplane
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

mytest.o: mytest.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
mytest: mytest.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_mytest: mytest
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

mytest_macro.o: mytest_macro.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
mytest_macro: mytest_macro.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_mytest_macro: mytest_macro
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark.o: benchmark.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_struct.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark: benchmark.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_struct.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark: benchmark
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

asdf.o: asdf.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
asdf: asdf.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_asdf: asdf
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_udn.o: benchmark_udn.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_struct.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_udn: benchmark_udn.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_struct.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_udn: benchmark_udn
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_float.o: benchmark_float.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_struct.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_float: benchmark_float.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_struct.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_float: benchmark_float
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_falseSharing.o: benchmark_falseSharing.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_struct.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_falseSharing: benchmark_falseSharing.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_struct.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_falseSharing: benchmark_falseSharing
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)


benchmark_falseSharing_float.o: benchmark_falseSharing_float.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_struct.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_falseSharing_float: benchmark_falseSharing_float.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_struct.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_falseSharing_float: benchmark_falseSharing_float
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_suite.o: benchmark_suite.c \
	ch_sym_ref_ad1_udn.h \
	ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h \
	ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param_b.h 
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_suite: benchmark_suite.o \
	ch_sym_ref_ad1_udn.o \
	ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o \
	ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param_b.o 
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_suite: benchmark_suite
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)
run_sim_benchmark_suite: benchmark_suite
	$(TILE_MONITOR) --functional $(SIMULATOR_ARGS) $(SIM_MONITOR_COMMON_ARGS)


benchmark_suite_working.o: benchmark_suite_working.c ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_struct.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_suite_working: benchmark_suite_working.o ch_sym_ref_ad1_udn.o ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_struct.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_suite_working: benchmark_suite_working
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

unit_test_sym_ch.o: unit_test_sym_ch.c \
	ch_sym_ref_ad1_udn.h \
	ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h \
	ch_sym_ref_ad1_sm_null.h \
	ch_sym_ref_ad1_sm_no.h \
	ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h \
	ch_asymin_ref_ad1_sm_param_b.h \
	ch_asymin_ref_ad1_sm_struct.h \
	ch_asymin_ref_ad1_sm_struct_b.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
unit_test_sym_ch: unit_test_sym_ch.o \
	ch_sym_ref_ad1_udn.o \
	ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o \
	ch_sym_ref_ad1_sm_null.o \
	ch_sym_ref_ad1_sm_no.o \
	ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o \
	ch_asymin_ref_ad1_sm_param_b.o \
	ch_asymin_ref_ad1_sm_struct.o \
	ch_asymin_ref_ad1_sm_struct_b.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_unit_test_sym_ch: unit_test_sym_ch
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

test.o: test.c channels.h ch_sym_ref_ad1_udn.h ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h ch_sym_ref_ad1_sm_null.h \
	ch_sym_ref_ad1_sm_no.h ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param.h ch_asymin_ref_ad1_sm_param_b.h \
	ch_asymin_ref_ad1_sm_struct.h ch_asymin_ref_ad1_sm_struct_b.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
test: test.o ch_sym_ref_ad1_udn.o  ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o ch_sym_ref_ad1_sm_null.o \
	ch_sym_ref_ad1_sm_no.o ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param.o ch_asymin_ref_ad1_sm_param_b.o \
	ch_asymin_ref_ad1_sm_struct.o ch_asymin_ref_ad1_sm_struct_b.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_test: test
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_suite_2.o: benchmark_suite_2.c \
	channels.h \
	ch_sym_ref_ad1_udn.h \
	ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h \
	ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param_b.h \
	ch_asymin_ref_ad1_sm_struct_b.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_suite_2: benchmark_suite_2.o \
	ch_sym_ref_ad1_udn.o \
	ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o \
	ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param_b.o \
	ch_asymin_ref_ad1_sm_struct_b.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_suite_2: benchmark_suite_2
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_sequential.o: benchmark_sequential.c error_handler.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_sequential: benchmark_sequential.o 
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_sequential: benchmark_sequential
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)



benchmark_pipelinemulticast_suite.o: benchmark_pipelinemulticast_suite.c \
	channels.h \
	ch_sym_ref_ad1_udn.h \
	ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h \
	ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param_b.h \
	ch_asymin_ref_ad1_sm_struct_b.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_pipelinemulticast_suite: benchmark_pipelinemulticast_suite.o \
	ch_sym_ref_ad1_udn.o \
	ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o \
	ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param_b.o \
	ch_asymin_ref_ad1_sm_struct_b.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_pipelinemulticast_suite: benchmark_pipelinemulticast_suite
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)

benchmark_linearmulticast_suite.o: benchmark_linearmulticast_suite.c \
	channels.h \
	ch_sym_ref_ad1_udn.h \
	ch_sym_ref_ad1_sm_fence.h \
	ch_sym_ref_ad1_sm_nullack.h \
	ch_asymin_ref_ad1_udn.h \
	ch_asymin_ref_ad1_sm_param_b.h \
	ch_asymin_ref_ad1_sm_struct_b.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@ 
benchmark_linearmulticast_suite: benchmark_linearmulticast_suite.o \
	ch_sym_ref_ad1_udn.o \
	ch_sym_ref_ad1_sm_fence.o \
	ch_sym_ref_ad1_sm_nullack.o \
	ch_asymin_ref_ad1_udn.o \
	ch_asymin_ref_ad1_sm_param_b.o \
	ch_asymin_ref_ad1_sm_struct_b.o
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@
run_pci_benchmark_linearmulticast_suite: benchmark_linearmulticast_suite
	$(TILE_MONITOR) $(PCI_ARGS) $(MONITOR_COMMON_ARGS)
