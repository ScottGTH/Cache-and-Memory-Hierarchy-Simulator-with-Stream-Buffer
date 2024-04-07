CC = g++
OPT = -O3
OPT = -std=c++11 #-g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

# List all your .cc/.cpp files here (source files, excluding header files)
# SIM_SRC = sim.cc
SIM_SRC = sim.cc cache_block.cc

# List corresponding compiled object files here (.o files)
# SIM_OBJ = sim.o
SIM_OBJ = sim.o cache_block.o

#################################

# default rule
all: sim
	@echo "my work is done here..."

# rule for making sim
sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH sim-----------"


# generic rule for converting any .cc file to any .o file
.cc.o:
	$(CC) $(CFLAGS) -c $*.cc

# generic rule for converting any .cpp file to any .o file
.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp


# type "make clean" to remove all .o files plus the sim binary
clean:
	rm -f *.o sim


# type "make clobber" to remove all .o files (leaves sim binary)
clobber:
	rm -f *.o


run_example:
	./sim 16 1024 1 8192 4 0 0 ../example_trace.txt

run_example_less:
	./sim 16 1024 1 8192 4 0 0 ../example_trace.txt | less


# Validation run without Prefetch
run1:
	./sim 16 1024 1 0 0 0 0 gcc_trace.txt > run1_output.txt
diff1:
	diff -iw run1_output.txt ./validation_traces/val1.16_1024_1_0_0_0_0_gcc.txt || exit 0

run2:
	./sim 32 1024 2 0 0 0 0 gcc_trace.txt > run2_output.txt
diff2:
	diff -iw run2_output.txt ./validation_traces/val2.32_1024_2_0_0_0_0_gcc.txt || exit 0

run3:
	./sim 16 1024 1 8192 4 0 0 gcc_trace.txt > run3_output.txt
diff3:
	diff -iw run3_output.txt ./validation_traces/val3.16_1024_1_8192_4_0_0_gcc.txt || exit 0

run4:
	./sim 32 1024 2 12288 6 0 0 gcc_trace.txt > run4_output.txt
diff4:
	diff -iw run4_output.txt ./validation_traces/val4.32_1024_2_12288_6_0_0_gcc.txt || exit 0


# Validation run with Prefetch
run5:
	./sim 16 1024 1 0 0 1 4 gcc_trace.txt > run5_output.txt
diff5:
	diff -iw run5_output.txt ./validation_traces/val5.16_1024_1_0_0_1_4_gcc.txt || exit 0




# L1 memory hierachy: BLCOKSIZE = 32, SIZE range = 1 KB ~ 1 MB = 1024 B ~ 1048576 B, ASSOC = 1, 2, 4, 8, fully_associative  
exp1:
	run=1; l1_size=1024 ; while [[ $$l1_size -le 1048576 ]] ; do \
	    l1_assoc=1 ; while [[ $$l1_assoc -le 8 ]] ; do \
		echo "run:"$$run $$l1_size $$l1_assoc ; \
		./sim 32 $$l1_size $$l1_assoc 0 0 0 0 gcc_trace.txt > ./exp1_output/exp1.output_$$l1_assoc_$$l1_size.txt ;\
		((l1_assoc = l1_assoc * 2)) ; \
		((run = run+1)); \
	    done ; \
	((l1_assoc = l1_size / 32)); \
	echo "run:"$$run $$l1_size $$l1_assoc "(full assoc)"; \
	./sim 32 $$l1_size $$l1_assoc 0 0 0 0 gcc_trace.txt > ./exp1_output/exp1.output_full.$$l1_size.txt ;\
        ((l1_size = l1_size * 2 )) ; \
	((run = run+1)); \
	done

# L1 memory hierachy: BLCOKSIZE = 32, SIZE range = 1 KB ~ 8 KB = 1024 B ~ 8192 B, ASSOC = {1, 2, 4, 8, fully_associative}  
# L2 memory hierachy: BLCOKSIZE = 32, SIZE = 16 KB = 16384 B, ASSOC = 8  
exp3: 
	run=1; l1_size=1024 ; while [[ $$l1_size -le 8192 ]] ; do \
	    l1_assoc=1 ; while [[ $$l1_assoc -le 8 ]] ; do \
		echo "run:"$$run $$l1_size $$l1_assoc ; \
		./sim 32 $$l1_size $$l1_assoc 16384 8 0 0 gcc_trace.txt > ./exp3_output/exp3.output_$$l1_assoc.$$l1_size.txt ;\
		((l1_assoc = l1_assoc * 2)) ; \
		((run = run+1)); \
	    done ; \
	((l1_assoc = l1_size / 32)); \
	echo "run:"$$run $$l1_size $$l1_assoc "(full assoc)"; \
	./sim 32 $$l1_size $$l1_assoc 16384 8 0 0 gcc_trace.txt > ./exp3_output/exp3.output_full.$$l1_size.txt ;\
        ((l1_size = l1_size * 2 )) ; \
	((run = run+1)); \
	done

# L1 memory hierachy: BLCOKSIZE = {16, 32, 64, 128}, SIZE range = 1 KB ~ 32 KB = 1024 B ~ 32768 B, ASSOC = 4  
exp4:
	run=1; l1_size=1024 ; while [[ $$l1_size -le 32768 ]] ; do \
	    blk_size=16 ; while [[ $$blk_size -le 128 ]] ; do \
		echo "run:"$$run $$l1_size $$blk_size ; \
		./sim $$blk_size $$l1_size 4 0 0 0 0 gcc_trace.txt > ./exp4_output/exp4.output_$$l1_size.$$blk_size.txt ;\
		((blk_size = blk_size * 2)) ; \
		((run = run+1)); \
	    done ; \
        ((l1_size = l1_size * 2 )) ; \
	done


# L1 memory hierachy: BLCOKSIZE = 32, SIZE range = {1 KB, 2 KB, 4 KB, 8 KB} = {1024, 2048, 4096, 8192} B, ASSOC = 4  
# L2 memory hierachy: BLCOKSIZE = 32, SIZE range = {16 KB, 32 KB, 64 KB} = {16384, 32768, 65536} B, ASSOC = 8  
exp5:
	run=1; l1_size=1024 ; while [[ $$l1_size -le 8192 ]] ; do \
	    l2_size=16384 ; while [[ $$l2_size -le 65536 ]] ; do \
		if [ $$l2_size -gt $$l1_size ]; then \
		    echo "run:"$$run $$l1_size $$l2_size ; \
		    ./sim 32 $$l1_size 4 $$l2_size 8 0 0 gcc_trace.txt > ./exp5_output/exp5.output_$$l2_size.$$l1_size.txt ;\
		    ((run = run+1)); \
		fi; \
		((l2_size = l2_size * 2)) ; \
	    done ; \
        ((l1_size = l1_size * 2 )) ; \
	done