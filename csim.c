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
#include <strings.h>
#include <sys/queue.h>


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
*	int time_stamp	   	   used to denote the time at which a particular data element was accessed. Used to tell which lines
*						   came in at what time and which element could be considered for candidacy as the least recently used
*						   element.
*	      
*	========
*	Returns
*	========
*	
*	Nothing. It is a constructor.
*/

					
typedef struct {

	int valid_bit;
	memory_address tag;
	char* block;
	int time_stamp;
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



/* Function that prints out the usage options of the program to the user. Ends the program as this 
*  only executes when the user inputs bad arguments or explicitly asks for help with the "-h" flag.
*
*	==========
*	Arguments
*	==========	
*
*	Char* argv[] --> character array representing the options passed in by the user
*
*	========
*	Returns
*	========
*
*	void, prints the usage options to standard out.
*/
void usage (char* argv[])
{
	//go through and print out any relevant information for command line arguments
	//to use the program
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    //end the program
    exit(0);
}


/*  Testing function to help debug the cache.  Useful for finding out which data entered which set/line based on tag.
*
*	=========
*	Arguments
*	=========
*
*	cache the_cache --> the fully constructed cache object containing all lines, sets, and blocks
*	
*	cache_stats cache_statistics --> struct that contains all "accounting information" (e.g. number of sets, lines, block_size, etc).
*
*	=======
*	Returns
*	=======
*
*	void, simply prints out debugging information to the screen
*/
void print_cache(cache the_cache, cache_stats cache_statistics){


	//get the number of sets, which is 2^s
	long long num_sets = pow(2.0, cache_statistics.s);
	//get the number of lines
	int num_lines = cache_statistics.E;

	//prirnt out the number of sets and number of lines
	printf("The number of sets is: %lli\n", num_sets);
	printf("The number of lines in each set is: %i\n", num_lines);

	//for each set
	for (int i=0; i < num_sets; i++){
		//grab the current set
		cache_set current_set = the_cache.sets[i];
		//print a "header" to delineate the current set
		printf("Set %i\n-----------------------------------------------------------\n", i);
		//for each line
		for(int j=0; j < num_lines; j++){
			//grab the current line
			cache_set_line current_line = current_set.cache_lines[j];
			//print out a delineating block
			printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			//print out the current line's valid bit, tag, and time stamp
			printf("Line %i's members: Valid Bit=%i, Tag=%llu, Time Stamp=%i\n", j, current_line.valid_bit, current_line.tag, current_line.time_stamp);
			//finishing delineating block
			printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		}
		//write a line to delineate the end of the set
		printf("-----------------------------------------------------------\n");
	}

}




/* Function to check if a number is a power of 2
*
*	=========
*	Arguments
*	=========
*
*	int input --> the number we want to know is a power of two or not
*
*	=======
*	Returns
*	=======
*
*	bool, indicating if the numner was a power of two or not
*/
bool is_power_of_two (int input){
//fun bit comparison trick to see it a number is a power of two
if((input &(input-1)) == 0 && input !=0 ){
	//if the number was a power of two, return true
	return true;
} else {
	//if not, return false
	return false;
}

}

/* Function to build the cache based on user-supplied parameters. Uses S, E, and B to build an outer cache container with S sets, E lines per
*  set, and B blocks in each line. 
*
*	=========
*	Arguments
*	=========
*
*	long long num_sets --> long long integer that represents the number of sets that will be in the cache
*
*	int associativity --> integer indicating how many lines per set will be in the cache
*
*	long long block_size --> integer indicating how many blocks in each line there will be
*
*	=======
*	Returns
*	=======
*
*	fully constructed cache type object with all sets, lines and blocks
*/
cache initialize_cache(long long num_sets, int associativity, long long block_size){
	//make a new cache object that will be returned once parameters have been applied
	cache constructed_cache;

	//declare a temporary cache_set object to hold the cache_set_lines
	cache_set temp_cache_set;
	//declare a temporary cache set line 
	cache_set_line temp_cache_set_line;

	//set the number of cache sets in the returned cache object.
	//we want to allocate a cache_set pointer that has as much memory as a
	//cache_set pointer times the number of sets that we want to build


	constructed_cache.sets = (cache_set*) malloc(sizeof(cache_set) * num_sets);

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
			//set the time_stamp of the line to 0 as it has not been accessed yet
			temp_cache_set_line.time_stamp =0;
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


/* Function to return the index of the least recently used element in the cache. Goes through the current (i.e. only valid) set
*  line by line and determines which element has the lowest time stamp. Once it finds this element, it returns the index of the line
*  containing the least recently used element. This function is also responsible for updating the time stamps of other elements as they
*  are accessed in order to maintain the current time stamp in order to set the time stamp for future accesses.
*
*	=========
*	Arguments
*	=========
*
*	cache_set selected_set --> cache_set object representing the currently selected set that the least recently used element will be in
*
*	cache_stats cache_statistics --> cache_stats object for grabbing the number of lines per set in order to iterate through all lines
*
*	int* time_stamp_container --> integer array that holds 2 elements: holds the most recently used element (representing value of the most current
*								  time stamp) and the least recently used element (representing the value of time stamp of the least recently used element)
*								  . These values need to be maintained and updated as elements are accessed and evicted.
*/
int find_LRU_index(cache_set selected_set, cache_stats cache_statistics, int * time_stamp_container){

	//need to know the number of lines we have to loop through
	int num_lines = cache_statistics.E;

	//initialize both of these variables to value of the time stamp found in the set's
	//first line
	int highest_time_stamp = selected_set.cache_lines[0].time_stamp; //will be used by run_simulation to set other lines' time stamps
	int lowest_time_stamp = selected_set.cache_lines[0].time_stamp; //used to find current the lowest time stamp

	//variable that will be returned after logic checks
	int lowest_time_stamp_index = 0;

	//loop through all of the lines, find the lowest_time_stamp_index
	for(int i=1; i < num_lines; i++){

		cache_set_line current_line = selected_set.cache_lines[i];
		//if the lowest time stamp we have is greater than the time stamp at the current line, set the lowest time stamp
		//to be this new lowest value
		if(lowest_time_stamp > current_line.time_stamp){
			//grab the index of this element, it is the least recently used
			lowest_time_stamp_index = i;
			//set this so that further checks for lowest time stamp can be conducted as the loop progresses
			lowest_time_stamp = current_line.time_stamp;

		}
		//if the highest time stamp is less than another line's time stamp, update the highest time stamp
		if(highest_time_stamp < current_line.time_stamp ){

			highest_time_stamp = current_line.time_stamp;
		}
	}

	//set the values for lowest time stamp and highest time stamp in the time_stamp_container

	//first element in this container holds the lowest time stamp
	time_stamp_container[0] = lowest_time_stamp;
	//second element in thsi container holds the highest time stamp
	time_stamp_container[1] = highest_time_stamp;

	//found the index of the least recently used element, return the index
	return lowest_time_stamp_index;

}


/* Function that frees all dynamically allocated memory to a cache object. Goes through all blocks per line, all the lines per set, and
*  all sets in the cache object.
*
*	=========
*	Arguments
*	=========
*
*	cache the_cache --> the complete cache object	
*
*	long long num_sets --> long long integer representing the number of sets in the cache
*
*	long long block_size --> long long integer representing the size of blocks in each line
*
*	int associativity --> integer representing the number of lines per set in the cache
*
*	=======
*	Returns
*	=======
*
*	void, the function simply releases all dynamically allocated memory in the cache object
*/
void free_allocated_memory(cache the_cache, long long num_sets, long long block_size, int associativity){
	//for each cache set, free the lines in the cache set
	for (int i=0; i < num_sets; i++){
		//grab the current set
		cache_set current_set = the_cache.sets[i];
		//check to make sure we are not going into memory we had not allocated
		if (current_set.cache_lines != NULL){
			//if the current_set.cache_set_line is not NULL, then we need to go to each of the lines and free up their blocks
			for (int j=0; j < block_size; j++){
				//declare a temp to hold the current line
				cache_set_line current_line = current_set.cache_lines[j];
				//make sure the block pointer is not NULL
				if (current_line.block != NULL){
					//free the associated block memory
					free(current_line.block);
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

/* Function to find the index of an empty line. Checks the valid bit for each line in a selected set and checks if the 
*  line is not valid. If it is not valid, the index of that line in the set is returned.
*
*	=========
*	Arguments
*	=========
*
*	cache_set seleced_set --> the cache_set object that represents the current set we are trying to find an empty
*							  line in.
*
*	cache_stats cache_statistics --> cache_stats object so that we can grab the number of lines per set from it in order
*									 to iterate through all of the lines of the selected set
*
*	=======
*	Returns
*	=======
*
*	integer, representing the index of an empty line in the current set
*/
int find_empty_line(cache_set selected_set, cache_stats cache_statistics){

	//first we need to know how many lines there are per set
	int num_lines = cache_statistics.E;
	//next, we need a temporary line variable to hold what the current line is
	cache_set_line current_line;
	 //safguard in case this returns something weird
	int line_found = -1;
	for(int i=0; i < num_lines; i++){
		//grab the current line in the set
		current_line = selected_set.cache_lines[i];
		//determine if the line is not valid (i.e. has no data in it). If yes, return the index of that line
		if(current_line.valid_bit == 0){
			
			//found an empty line, return the index of that line
			line_found = i;
			break;
		}
	}
	//have to add this or the compiler gets grumpy
	return line_found;

}




/* Function to simulate accesses to the cache. Causes changes in statistical data regarding hits, misses, and evictions. 
*  Takes in a memory address corresponding to the incoming data, attempts to find that item in the cache. If so, it was a hit. Otherwise, it was
*  a miss or an eviction. If it was a cold miss, the data item is stored in the cache.
*  The function performs evictions based on least recently used element, and as such needs to keep track of and
*  update time stamps for data elements in the cache as well. 
*  Once the function has completed determining hits, misses, or evictions, it returns a cache_stats object that is used by main
*  to report the total number of hits, misses, and evictions to standard out.
*
*	=========
*	Arguments
*	=========
*
*	cache main_cache --> the complete cache object that data passes through and is evaluated on hits, misses, and evictions
*
*	cache_stats cache_statistics --> cache_stats object that is used to hold the relevant statistical information for reporting
*									 hits, misses, and evictions. Also contains data members for S, s, E, b, and B.
*
*	memory_address address --> 64 bit memory address that represents an incoming data item into the cache. The data item is 
*							   tried against the cache, where it is evaluated as a hit, miss, or it evicts another data item.
*
*	=======
*	Returns
*	=======
*	
*	cache_stats object, updated based on the number of hits, misses, and evictions that occurred during evaluation of
*	an incoming data item. 
*/
cache_stats run_simulation(cache main_cache, cache_stats cache_statistics, memory_address address){

	//need some variables to hold some statistical information

	//need a variable that tells us if the cache is full or not
	int line_is_full = 1;

	//need a variable to hold the number of lines that we are dealing with from the cache
	int num_lines = cache_statistics.E;

	//need to keep track of the previous number of hits so that it increases when hit occurs
	int previous_hits = cache_statistics.num_hits;

	//need to know the size of the tag given the parameters that were passed in by the user.
	//This is found by taking the size of a memory address (64 bits) and decreasing this value
	//by the value of the number of set bits and by the value of the number of block bits
	int tag_size = (64 - (cache_statistics.s + cache_statistics.b));

	//printf("Tag Size: %i\n", tag_size);

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
	memory_address left_shifted_temp = address << (tag_size);

	//Next, we will right shift the left_shifted_temp variable by (tag_size + number of block offset bits) in order
	//to recover the set index
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
		cache_set_line current_line = selected_set.cache_lines[i];
		//if the current line is valid, then there is data in it and needs to be looked at
		if(current_line.valid_bit){
			//if the current line's tag and the incoming tag are identical, then the data was in the cache.
			if(current_line.tag == incoming_tag){
				//increment the number of hits
				cache_statistics.num_hits++;
				
				//data was accessed, increment the number of accesses
				current_line.time_stamp++;
				//since we modified the members of the line, we need to reflect that in the selected_set
				selected_set.cache_lines[i] = current_line;
			}

		}
		//We looked at the line and the line was not valid (i.e. empty), that means the data was not in the cache. 
		if(current_line.valid_bit != 1)
		{
			//This also means that the cache was not full, so change the cache_is_full flag to false
			if (line_is_full){
				//printf("Someone's valid bit was 0, cache is not full\n");
				line_is_full = 0;

			}
			
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

	int* time_stamp_container = (int*) malloc(sizeof(int*) * 2);

	//When an

	//Now that we have a structure that we can hold our MRU and LRU values, we need to find the index of the line
	//in the set that contains the LRU element

	int LRU_index = find_LRU_index(selected_set, cache_statistics, time_stamp_container);

	//Now that we have the index of the least recently used element, we need to deal with the cases
	//of either:
	//
	//The cache was full and we need to evict
	//
	//The cache was not full and we can store the line in cache

	//Case 1: cache was full and we need to evict
	if (line_is_full){
		//we need to evict someone from the cache

		//first we increment the eviction counter
		cache_statistics.num_evictions++;

		//next we need to evict someone, so we set the tag in the cache at the LRU_index to be the tag of the
		//incoming data
		selected_set.cache_lines[LRU_index].tag = incoming_tag;

		//now we modify the time stamp of the element in the line to reflect its access time, which is 
		//the highest current time stamp + 1
		selected_set.cache_lines[LRU_index].time_stamp = time_stamp_container[1] + 1;
		//store the selected set in the cache at the set_index position
		main_cache.sets[set_index] = selected_set;
	}
	//else there was room in the cache and we just need to find a line in the selected set to put it in
	else {

		//get the index of an empty line in the currently selected cache set
		int empty_line_index = find_empty_line(selected_set, cache_statistics);

		//printf("there was room in the cache");

		//set the tag of the empty line with the tag of the incoming data
		selected_set.cache_lines[empty_line_index].tag = incoming_tag;
		//set the time stamp of the line to be that of the current maximum time stamp + 1 
		selected_set.cache_lines[empty_line_index].time_stamp = time_stamp_container[1] + 1;
		//set the valid bit on the line to 1 to indicate that there is data in the line
		selected_set.cache_lines[empty_line_index].valid_bit = 1;
		//reflect change in the selected set back into the cache object
		main_cache.sets[set_index] = selected_set;

	}
	//now we are done with the time_stamp_container object so we can free it
	free(time_stamp_container);
	//we have modified all of the members of the cache_statistics struct, return it to main
	return cache_statistics;
}




/* Main program */

int main(int argc, char **argv)
{
    //declare cache object
    cache this_cache;
    //declare cache_stats object that will hold all relevant values for cache simulation
    cache_stats cache_statistics;
    
    //declare variables for num sets and block size

    //num_sets = 2^s
    long long num_sets;
    //block_size = 2^b
    long long block_size;

    //declare file pointer so we can access the trace files 
    FILE* file_pointer;

    //declare character to hold the type of cache interaction
    char interaction_type;
    //declare variable to hold the incoming memory address from the trace_file 
    memory_address address;
    //declare variable to hold the size of the interaction
    int size;
    //declare character pointer to point to the trace file
    char* trace_file;

    char options;
    while( (options=getopt(argc,argv,"s:E:b:t:v:h")) != -1){
        switch(options){
        case 's':
            cache_statistics.s = atoi(optarg);
            break;
        case 'E':
            cache_statistics.E = atoi(optarg);
            break;
        case 'b':
            cache_statistics.b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            //verbose_mode = 1;
            break;
        case 'h':
            usage(argv);
            exit(0);
        default:
            usage(argv);
            exit(1);
        }
    }
    //checks to make sure that the user has not entered invalid values for s, E, b, and the trace_file
    if (cache_statistics.s == 0 || 
    	cache_statistics.E == 0 || 
    	cache_statistics.b == 0 || 
    	trace_file == NULL || 
    	cache_statistics.E > 8 || 
    	!(is_power_of_two(cache_statistics.E))) {
    	//display error message and exit with code 1
        printf("%s: Missing required command line argument\n", argv[0]);
        usage(argv);
        exit(1);
    }

   	//set the values of previously declared variables

   	//num_sets = 2^s
    num_sets = pow(2.0, cache_statistics.s);
    //block_size = 2^b
    block_size = pow(2.0, cache_statistics.b);
    //Zero out the counts of num_hits, num_misses, and num_evictions to start with
    cache_statistics.num_hits = 0;
    cache_statistics.num_misses = 0;
    cache_statistics.num_evictions = 0;


    //run cache initialization function that will build an empty cache according to user specifications
    this_cache = initialize_cache(num_sets, cache_statistics.E, block_size);

    //set the file pointer to point to the open trace_file
    file_pointer = fopen(trace_file,"r");

    
    //start reading in data from the file:
   	if (file_pointer != NULL) {
   		//as long as the values for interaction_type, address, and size keep coming in, keep reading them
        while (fscanf(file_pointer, " %c %llx,%d", &interaction_type, &address, &size) == 3) {
        	//differentiate simulation based on interaction_type
            switch(interaction_type) {
            	//Instruction load is to be ignored. No simulation here.
                case 'I':
                break;
                //Load interacts with cache once, simulate once.
                case 'L':
                    cache_statistics = run_simulation(this_cache, cache_statistics, address);
                break;
                //Store interacts with cache once, simulate once.
                case 'S':
                    cache_statistics = run_simulation(this_cache, cache_statistics, address);
                break;
                //Modify interacts with cache twice:
                //Once for the load.
                //Once for the modify.
                //Simulate twice.
                case 'M':
                    cache_statistics = run_simulation(this_cache, cache_statistics, address);
                    cache_statistics = run_simulation(this_cache, cache_statistics, address);	
                break;
                //default condition for safety
                default:
                break;
            }
        }
    }

    //print the results of the simulation as per the assignment specifications
    printSummary(cache_statistics.num_hits, cache_statistics.num_misses, cache_statistics.num_evictions);

    //run function to free all heap memory allocated
    free_allocated_memory(this_cache, num_sets, cache_statistics.E, block_size);
    //close the file so as not to cause issues
    fclose(file_pointer);

    return 0;
}





