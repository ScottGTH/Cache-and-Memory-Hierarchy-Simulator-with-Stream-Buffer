					CPU, Computer Architechture Project, Using C++


1. Type "make" to build.  (Type "make clean" first if you already compiled and want to recompile from scratch.)

2. Run trace reader:

   To run without throttling output:
   ./sim 32 8192 4 262144 8 3 10 ../example_trace.txt

   To run with throttling (via "less"):
   ./sim 32 8192 4 262144 8 3 10 ../example_trace.txt | less

   To run and confirm that all requests in the trace were read correctly:
   ./sim 32 8192 4 262144 8 3 10 ../example_trace.txt > echo_trace.txt
   diff ../example_trace.txt echo_trace.txt
