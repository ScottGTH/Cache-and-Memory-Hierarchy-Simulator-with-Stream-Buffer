#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>
#include <tuple>
#include <list>
#include <vector>
#include <bitset>
#include <string>
#include "cache_block.h"
#include "sim.h"
#include <iomanip>
#include <math.h>
#include <inttypes.h>
/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/
int main (int argc, char *argv[]) {
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

   // Exit with an error if the number of command-line arguments is incorrect.
   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];

   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
   std::cout << std::endl;


   bool N_L1_V = (params.L2_SIZE > 0);
   
   Cache* L1;
   Cache* L2;
   
   if (params.L1_SIZE !=0){
      L1 = new Cache(params.L1_SIZE, params.BLOCKSIZE, params.L1_ASSOC, params.PREF_N, params.PREF_M, N_L1_V, 1);
      
      if(N_L1_V){
         L2 = new Cache(params.L2_SIZE, params.BLOCKSIZE, params.L2_ASSOC, params.PREF_N, params.PREF_M, 0, 2);
         L1->Next_L_Ca = L2;
      //}else{
        // L2 = new Cache(params.L2_SIZE, params.BLOCKSIZE, params.L2_ASSOC, params.PREF_N, params.PREF_M, 0);
      }

   }else{
      printf("Error occurs!");
   }

   int num_i = log2(params.BLOCKSIZE);

   uint32_t b_addr;

   // Read requests from the trace file and echo them back.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
      b_addr = addr >> num_i;
      /*if (rw == 'r'){
         printf("r %x\n", addr);
         L1->rd_req(b_addr);
      }else if (rw == 'w'){
         printf("w %x\n", addr);
         L1->wr_req(b_addr);
      }else {
         printf("Error: Unknown request type %c.\n", rw);
	      exit(EXIT_FAILURE);
      }*/
      L1->req(rw, b_addr);
    }
    
L1->contents();

if(N_L1_V){
   std::cout << std::endl;
   L2->contents();
}


float L1_miss_rate = (float) (L1->rd_miss + L1->wr_miss) / (float) (L1->rd + L1->wr);
float L2_miss_rate;

if(N_L1_V){
   L2_miss_rate = (float) L2->rd_miss / (float) L2->rd;
}else{
   L2_miss_rate = 0;
}



uint32_t mem_traffic;

if(N_L1_V){
   mem_traffic = L1->mem_traffic + L2->mem_traffic;
}else{
   mem_traffic = L1->mem_traffic;
}

uint32_t N_L2 = 0;

/*printf("===== Simulation results =====\n"
         "a. number of L1 reads:             %u\n"
         "b. number of L1 read misses:       %u\n"
         "c. number of L1 writes:            %u\n"
         "d. number of L1 write misses:      %u\n"
         "e. L1 miss rate:                   %.4f\n"
         "f. number writebacks from L1:      %u\n"
         "g. number of L2 reads:             %u\n"
         "h. number of L2 read misses:       %u\n"
         "i. number of L2 writes:            %u\n"
         "j. number of L2 write misses:      %u\n"
         "k. L2 miss rate:                   %.4f\n"
         "l. number of writebacks from L2:   %u\n"
         "m. total memory traffic:           %u\n", L1.rd, L1.rd_miss, L1.wr, L1.wr_miss,
         l1_miss_rate, L1.wr_back, L2.rd, L2.rd_miss, L2.wr,
         L2.wr_miss, l2_miss_rate, L2.wr_back, (L1.mem_traffic + L2.mem_traffic));*/

   printf("===== Measurements =====\n");
   std::cout << std::setw(31) << std::left << "a. L1 reads:" << L1->rd << std::endl;
   std::cout << std::setw(31) << std::left << "b. L1 read misses:" << L1->rd_miss << std::endl;
   std::cout << std::setw(31) << std::left << "c. L1 writes:" << L1->wr << std::endl;
   std::cout << std::setw(31) << std::left << "d. L1 write misses:" << L1->wr_miss << std::endl;
   std::cout << std::setw(31) << std::left << "e. L1 miss rate:" << std::fixed << std::setprecision(4) << L1_miss_rate << std::endl;
   std::cout << std::setw(31) << std::left << "f. L1 writebacks:" << L1->wr_back << std::endl;
   std::cout << std::setw(31) << std::left << "g. L1 prefetches:" << L1->prefetch << std::endl;
   if(N_L1_V){
      std::cout << std::setw(31) << std::left << "h. L2 reads (demand):" << L2->rd << std::endl;
      std::cout << std::setw(31) << std::left << "i. L2 read misses (demand):" << L2->rd_miss << std::endl;
      std::cout << std::setw(31) << std::left << "j. L2 reads (prefetch):" << L2->rd_with_pref << std::endl;
      std::cout << std::setw(31) << std::left << "k. L2 read misses (prefetch):" << L2->read_miss_wtih_pref << std::endl;
      std::cout << std::setw(31) << std::left << "l. L2 writes:" << L2->wr << std::endl;
      std::cout << std::setw(31) << std::left << "m. L2 write misses:" << L2->wr_miss << std::endl;
      std::cout << std::setw(31) << std::left << "n. L2 miss rate:" << std::fixed << std::setprecision(4) << L2_miss_rate << std::endl;
      std::cout << std::setw(31) << std::left << "o. L2 writebacks:" << L2->wr_back << std::endl;
      std::cout << std::setw(31) << std::left << "p. L2 prefetches:" << L2->prefetch << std::endl;
      std::cout << std::setw(31) << std::left << "q. memory traffic:" << mem_traffic << std::endl;
   }else{
      std::cout << std::setw(31) << std::left << "h. L2 reads (demand):" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "i. L2 read misses (demand):" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "j. L2 reads (prefetch):" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "k. L2 read misses (prefetch):" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "l. L2 writes:" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "m. L2 write misses:" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "n. L2 miss rate:" << std::fixed << std::setprecision(4) << L2_miss_rate << std::endl;
      std::cout << std::setw(31) << std::left << "o. L2 writebacks:" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "p. L2 prefetches:" << N_L2 << std::endl;
      std::cout << std::setw(31) << std::left << "q. memory traffic:" << mem_traffic << std::endl;
   }
   
   return(0);
}
