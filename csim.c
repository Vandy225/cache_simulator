/*
* Title: csim.c
* Written By: Joshua Vandenhoek
* Student ID#: 001142936
* Written For: CPSC 4210
* Date: February 9, 2018
*
* Purpose: csim.c is a program that is intended to report the total number of 
*	hits, misses, and evictions of data items by simulating an arbitrarily sized
*	cache memory as defined by the user. For the replacment policy, we have been 
*	required to use LRU (or least recently used). 
*	
*/


//necessary include statements
#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <sys/queue.h>

//struct and custom data type declarations


//This custom data type is a 64 bit integer designed to hold
//the (converted to binary) memory address that comes from the valgrind output
typedef unsigned long long int memory_address;

/*Struct that defines a set line. 
*
*	========
*	Members
*	========
*
*	int valid_bit	 	   used to hold the valid bit for the set line.
*	
*	memory_address tag	   used to hold the tag bits for the set line. I used a memory_address
*						   type because the size of the tag bits may be greater than the normal 32 bits
*						   that an int will hold.
*
*	char* block 		   used to hold the information contained within a block. Needs to be a char pointer because
*						   the block size is variable (taken from the user at runtime), 
*						   and thus the number of elements contained in it are variable.
*
*
*	int num_accesses	   used to tell how many times a particular line has been accesssed for the purposes of
*						   determining candidacy for being chosen as the least recently used element.
*	      
*	========
*	Returns
*	========
*	
*	Nothing. It is a constructor.
*/

//TO DO --> consider using access_count to denote least-recently used member instead of queue if queue is a pain					
typedef struct {

	bool valid_bit;
	memory_address tag;
	char* block;
	int num_accesses;
} cache_set_line;

/* Struct that defines a cache set
*
* 	========
*	Members
*	========
*
*	cache_set_line* 	   cache_lines pointer to a group of cache set lines. Depending on the value of E, 
*						   this set will contain between 1-8 cache_set_lines per cache_set.
*
*	========
*	Returns
*	========
*	
*	Nothing. It is a constructor
*/
typedef struct {
	cache_set_line* cache_lines;
}cache_set;



/* Struct that defines the entire cache. Composed of a number of sets.
*
*	========
*	Members
*	========
*
*	cache_set* sets 	   pointer to the group of sets that composes the cache. Variable depending on the value of S
*						   taken from the user at runtime.
*
*	========
*	Returns
*	========
*	
*	Nothing. It is a constructor.
*/
typedef struct  {
	cache_set* sets;
}cache;

/* Struct to hold all of the parameters needed to construct the cache and determine number of hits and misses. 
*
*	========
*	Members
*	========
*
*	int s 				   integer to hold the number of set index bits (passed in from the user) 
*
* 	int S 				   integer to hold the number of sets in the cache (given by 2^s)
*
*	int E 				   integer to hold the number of lines per set in the cache (i.e. defines the associativity
*						   of the cache)
*
*	int b 				   integer to hold the number of bits in the block offset portion of the data (passed in by user)
*
*	int B 				   integer to hold the block size of blocks in the cache (given by 2^b)
*
*	int num_hits	       integer to hold the total number of hits when running a trace
*
*	int num_misses		   integer to hold the total number of misses when running a trace
*
*	int num_evictions	   integer to hold the total number of data evictions when running a trace
*
*	========
*	Returns 
*	========
*
*	Nothing. It is a constructor.
*	
*/ 
typedef struct  {
	int s;
	int S;
	int E;
	int b;
	int B;
	int num_hits;
	int num_misses;
	int num_evictions;
}cache_stats;



/* Function to print how to use the program
*	
*
*/
void usage (char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    //printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}




//print the cache
//TO DO --> implement cache printer
void print_cache() {

}

//function to check if a number is a power of 2
bool is_power_of_two (int input){

if((input &(input-1)) == 0 && input !=0 ){
	return true;
} else {
	return false;
}

}


cache initialize_cache(long long num_sets, long long block_size, int associativity){
	//make a new cache object that will be returned once parameters have been applied
	cache constructed_cache;

	//declare a temporary cache_set object to hold the cache_set_lines
	cache_set temp_cache_set;
	//declare a temporary cache set line 
	cache_set_line temp_cache_set_line;

	//set the number of cache sets in the returned cache object.
	//we want to allocate a cache_set pointer that has as much memory as a
	//cache_set pointer times the number of sets that we want to build


	constructed_cache.sets = (cache_set*) malloc((sizeof(cache_set) * num_sets));

	//Now that we have set the pointer in the cache object, we need to allocate lines to
	//it.

	//the cache object is an array of sets, so we need a loop to put cache_sets in that array

	for(int i=0; i < num_sets; i++){
		//allocate memory for a set line in the temporary cache_set object and set it
		//there will be (associativity) lines per cache_set
		temp_cache_set.cache_lines = (cache_set_line*) malloc(sizeof(cache_set_line) * associativity);

		//now that we have allocated all of the set lines per set, we will instantiate the sets in the 
		//returned cache object
		
		constructed_cache.sets[i] = temp_cache_set;

		//now, we have only made the appropriate number of sets with the appropriate number of lines. We need to
		//initialize the members of the lines with the appropriate values

		for(int j=0; j < associativity; j++){
			

			//set the valid bit to 0
			temp_cache_set_line.valid_bit = 0;
			//initialize the tag to nothing
			temp_cache_set_line.tag = 0;
			//set the num_accesses of the line to 0 as it has not been accessed yet
			temp_cache_set_line.num_accesses =0;
			//create a block of size block_size for each line
			char* temp_block = (char*) malloc(sizeof(char*) * block_size);
			//assign the block to the temp_cache_set_line
			temp_cache_set_line.block = temp_block;
			//assign the set line to the temporary_cache_set
			temp_cache_set.cache_lines[j] = temp_cache_set_line;



		} 



	}

	//now that we have constructed the cache, we need to return it so it can be operated on
	return constructed_cache;
}

//Function that frees all allocated memory to a cache object

void free_allocated_memory(cache the_cache, long long num_sets, long long block_size, int associativity){
	//for each cache set, free the lines in the cache set

	for (int i=0; i < num_sets; i++){
		//grab the current set
		cache_set current_set = the_cache.sets[i];
		//check to make sure we are not going into memory we had not allocated
		if (current_set.cache_lines != NULL){
			//if the current_set.cache_set_line is not NULL, then we need to go to each of the lines and free up their blocks
			for (int j=0; j < associativity; j++){
				//declare a temp to hold the current line
				cache_set_line current_line = current_set.cache_lines[j];
				//make sure the block pointer is not NULL
				if (current_line.block != NULL){
					//free the associated block memory
					free (current_line.block);
				}

			}		
			
		}
		//now that the blocks have been de-allocated, free up the cache_set_line pointer
		free(current_set.cache_lines);
	}
	//finally, free up the sets from the cache_object
	if(the_cache.sets != NULL){
		free(the_cache.sets);
	}

}


//TO DO: implement this function
cache_statistics run_simulation(cache main_cache, cache_stats cache_statistics, memory_address address, queue LRU_queue){

	//need some variables to hold some statistical information

	//need a variable that tells us if the cache is full or not
	bool cache_is_full = true;

	//need a variable to hold the number of lines that we are dealing with from the cache
	int num_lines = cache_statistics.E;

	//need to keep track of the previous number of hits so that it increases when hit occurs
	int previous_hits = cache_statistics.num_hits;

	//need to know the size of the tag given the parameters that were passed in by the user.
	//This is found by taking the size of a memory address (64 bits) and decreasing this value
	//by the value of the number of set bits and by the value of the number of block bits
	int tag_size = (64 - (cache_statistics.s + cache_statistics.b));

	//We need a way to find which set the new data is trying to fit into (i.e. the index of the set).
	//To do this, we can do some clever bit shifting.

	//Let's assume the following parameters:

	//+-------------------------------------------------------------+
	//| tag bits (48)   | set index bits (8) | byte offset bits (8) |
	//+-------------------------------------------------------------+

	//To get the set index by itself, first we can left shift by the value of the tag bits to get rid of them (48 in this case):

	//+------------------------------------------------------------------------+
	//| set index(8)   | block offset bits(8) | a bunch of 0's (tag size = 48) |
	//+------------------------------------------------------------------------+

	//Then, we can right shift the whole thing by (tag size + byte offset) to get the set index in the least significant position:

	//+------------------------------------------------------------------------+
	//| a bunch of 0's (tag size + block offset = 8 + 48 = 56) | set index(8)  |
	//+------------------------------------------------------------------------+

	//Which leaves us with the set index

	//First, we will left shift the passed in address by the size of the tag in order to strip off the tag bits.
	//TO DO: data type may need to be unsigned long long
	memory_address left_shifted_temp = address << (tag_size);
	//Next, we will right shift the left_shifted_temp variable by (tag_size + number of block offset bits) in order
	//to recover the set index
	//TO DO: data type may need to be unsigned long long
	memory_address set_index = left_shifted_temp >> (tag_size + cache_statistics.b);

	//Next, in order to determine if our data is already in the cache, we need to know what the tag is for the
	//incoming data. We can get the tag bits from the memory address by right shifting the memory address by (set bits + block offset bits):

	memory_address incoming_tag = address >> (cache_statistics.s + cache_statistics.b);

	//Now that we know which set we are trying to put data in, we need to hold it in a variable

	cache_set selected_set = main_cache.sets[set_index];

	//Now we have a set that we can try and put data in, and a tag associated with the incoming data. Now we need to loop through
	//all of the lines in the selected set and determine if the data is in the cache.

	for (int i=0; i < num_lines; i++){
		//grab the current line under consideration
		cache_set_line current_line = selected_set.cache_set_line[i];
		//if the current line is valid, then there is data in it and needs to be looked at
		if(current_line.valid_bit){
			//if the current line's tag and the incoming tag are identical, then the data was in the cache.
			if(current_line.tag == incoming_tag){
				//increment the number of hits
				cache_statistics.num_hits++;
				//TO DO: maybe add the element to the LRU queue which would simulate the number of accesses
				//LRU_queue.push(current_line)
				//data was accessed, increment the number of accesses
				current_line.num_accesses++
				//since we modified the members of the line, we need to reflect that in the selected_set
				selected_set.cache_set_lines = current_line;
			}

		} 
		//We looked at the line and the line was not valid, that means the data was not in the cache. This also means that
		//the cache was not full, so change the cache_full flag to false
		else if(!(current_line.valid) && (cache_full)){
			cache_full = false;
		}
	}
	//We have looked through all lines in the set. now we want to see if the number of hits was incremented by comparing the data
	//member of cache_statistics with our variable previous_hits. If it was, we had a cache hit and we should return the cache_statistics 
	//object. If not, then we know it was a miss and we need to do some more processing.
	if(cache_statistics.num_hits == previous_hits){
		//this means that hits was not incremented and was thus a miss. Increment number of misses and process more.
		cache_statistics.num_misses++;
	}
	else {
		//the number of hits was incremented, and we had a hit. Return the cache_statistics object.
		return cache_statistics;
	}

	//Now that we have gone this far, we had a cache miss. 

	//We need to know the counts of the least used line and the most used counts. To do this, we will 
	//declare ourselves an int* that holds 2 elements: 
	//arr[0] will contain the count of the most used line
	//arr[1] will contain the count of the least used line. 
	//We need these values because they will change as we continually try and find the least used element in order
	//to replace it.

	int* LRU_tracked_lines = (int*) malloc(sizeof(int*) * 2);

	//TO DO: This may be the place where I implement a queue to hold the LRU elements
	//When an

	//Now that we have a structure that we can hold our MRU and LRU values, we need to find the index of the line
	//in the set that contains the LRU element

	int LRU_index = find_LRU_index()






	//we have modified all of the members of the cache_statistics struct, return it to main
	return cache_statistics;




}




//main program

int main(int argc, char** argv)
{
	/*
	//by default, set verbose mode to 0
	int verbose_mode = 0;*/
	

	//initialize an empty cache object
	cache main_cache;

	//initialize an empty cache_stats object
	cache_stats cache_statistics;

	//declare a file pointer in order to process the tracefile

	FILE* file_pointer;

	//need a variable to store the value passed in the -t argument for the trace

	char* trace_file;

	//TO DO --> May need to make sure that the cache_statistics are set to 0

	//next thing to do is parse arguments from the command line

	//have a character that will hold the argument
	char argument;

	//run a while loop to process the arguments
	//will check for invalid arguments later
	while ((argument=getopt(argc, argv, "s:E:b:t:v:h")) != -1){
		//switch statement to handle what to do with the argument
		//need atoi to change the strings into integers to store in the cache_statistics struct.
		switch (argument){
			//get the number of set bits from the user, store in cache_statistics
			case 's': 
			cache_statistics.s = atoi(optarg);
			break;
			
			//get the number of lines per set from the user
			case 'E':
			cache_statistics.E = atoi(optarg);
			break;
			
			//get the number of bits in the byte offset from the user
			case 'b':
			cache_statistics.b = atoi(optarg);
			break;
			
			//get the name of the tracefile
			case 't':
			trace_file = optarg;
			break;
			
			//enable verbose mode
			/*
			case 'v':
			verbose_mode = 1;
			break;*/
			
			//print usage
			case 'h':
			usage(argv);
			//exit normally
			exit(0);
			
			//if bad arguments, then print usage and exit
			default:
			usage(argv);
			//print code 1 to indicate bad exit
			exit(1);
		}
	}
	//Check to make sure that the parameter setting in cache_statistics
	//conforms to the rules
	if (cache_statistics.s == 0 || 
		cache_statistics.E == 0 || 
		cache_statistics.E > 8 ||
		!(is_power_of_two(cache_statistics.E)) ||
		cache_statistics.b == 0 ||
		trace_file == NULL)
	{
		printf("Bad argument value\n\n");
		usage(argv);
	}
	//use the parameters gathered from the user to get some useful variables

	//make sure that the variables are large enough to deal with large numbers
	//set a variable num_sets to 2^s to determine the number of sets
	long long num_sets = pow(2.0, cache_statistics.s);
	//set a variable block_size to s^b to determine the block size
	long long block_size = pow(2.0, cache_statistics.b);

	//initialize ppropriate parameters in the cache_statistics struct regarding hits, misses, and evictions

	cache_statistics.num_hits=0;
	cache_statistics.num_misses=0;
	cache_statistics.num_evictions=0;

	//now that we have the building blocks for a cache, we need to initialize the main_cache object with these values
	//pass in the number of sets, block size, and associativity when constructing the cache

	main_cache = initialize_cache(num_sets, block_size, cache_statistics.E);

	//TO DO: maybe declare queue here

	//now that we have initialized the cache, we need to parse the input that is coming in from the trace files

	//point the file pointer at the supplied trace file
	file_pointer = fopen(trace_file, "r");

	//declare variables that will hold the contents from fscanf
	//character variable to hold the type of instruction (I, M, S, L)
	char instruction;
	//64 bit integer to hold the memory address
	memory_address address;
	//integer to hold the size of the operation being performed
	int size;

	//make sure that the file pointer is not NULL
	if (file_pointer != NULL){
		//use fscanf to retrieve input from the trace file, stop when you do not get 3 operands
		while(fscanf(file_pointer, " %c %lli,%i",&instruction, &address, &size)==3){
			//use switch statement to deal with the different cases of instruction.
			//Now we are actually running the simulations on the cache
			switch(instruction){
				case 'I':
				//Instruction load does not interact with the cache, just continue on
				break;
				//Modify instruction involves a load and a modify, need to simulate the cache
				//twice
				case 'M':
				cache_statistics = run_simulation(main_cache, cache_statistics, address);
				cache_statistics = run_simulation(main_cache, cache_statistics, address);
				break;
				//Store operation interacts with the cache once, simulate once
				case 'S':
				cache_statistics = run_simulation(main_cache, cache_statistics, address);
				break;
				//Load operation interacts with the cache once, simulate once
				case 'L':
				cache_statistics = run_simulation(main_cache, cache_statistics, address);


			}
		}
	}





	//
    printSummary(0, 0, 0);
    return 0;
}
