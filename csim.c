/*
Name: Sally Lee
Email: sallylee@wustl.edu
Purpose: A cache simulator that takes a valgrind memory trace as input, 
		simulates the hit/miss behavior of a cache memory on this trace, 
		and outputs the total number of hits, misses, evictions, dirty bytes evicted, 
		dirty bytes active in the cache, and back-to-back references to the same address within a given set.
*/

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h> //for fopen
#include <string.h> //for atoi
#include <math.h> //for pow()


typedef struct block{
  unsigned long tag;
  int dirty;
  int valid;
}block_t;


typedef struct set{
  block_t * blocks; //array of blocks
  int size;
}set_t;





int main(int argc, char * argv[])
{
  
  int s = 0; //# of set index bits
  int E = 0; //# of blocks per set
  int b = 0; //# of block bits
  char* tracefile = 0; //name of valgrind trace to replay 
  
  char opt;

  
  //argument parser to get s, E, b parameters
  while((opt = getopt(argc, argv, "s:E:b:t:")) != -1){
  	switch(opt){ 
		
		case 's':
			s = atoi(optarg);
		case 'E':
			E = atoi(optarg);
		case 'b':
			b = atoi(optarg);
		case 't':
			tracefile = optarg; 

      
   	 }
  }
  
  
  int num_sets = (int) pow(2,s);
  
  
  //array of sets
  set_t * arr = (set_t *) calloc(num_sets, sizeof(set_t)); 
  
  if(arr == NULL){
    return -1;
  }

  //allocate space for array of blocks in each set
  for(int iter = 0; iter < num_sets; iter++){ 
    block_t * arr_blocks = (block_t *) calloc(E, sizeof(block_t));
    if(arr_blocks == NULL){
      return -1;
    }
    arr[iter].blocks = arr_blocks;
  }
  

  int hit_count = 0;
  int miss_count = 0;
  int eviction_count = 0;
  int dirty_bytes_evicted = 0;
  int dirty_bytes_active = 0;
  int double_references = 0;
 
  /*Purpose: executes the load operation
    Arguments: tag = tag of the block that is being searched for in the cache
			   set_index = the index of the set that is being searched in the cache
			   blocks = the array of blocks in the set being searched
  */
  void load(unsigned long tag, int set_index, block_t * blocks){
	    
		int hit = 0; // a boolean where	 miss: hit = 0
			     	 //					  hit: hit = 1

		// counts hits from load
		for(int i = 0; i < E; i++){		// E = # of blocks possible in a set
			//block_t curr = blocks[i];

			if(blocks[i].valid == 1 && blocks[i].tag == tag){

				hit_count++;

				if(i == 0){	// mru is always the first block in the set
					double_references++;
				}

				block_t copy = blocks[i];
				/* scoot down every block by 1 index and put most recently accessed block at index 0 */
				for(int j = i; j > 0; j--){
					blocks[j] = blocks[j - 1];
				}
				blocks[0] = copy;
				hit = 1;
				
				break;
			}
		}
		//finished counting hits from load


		//counts misses from load
		if(!hit){
			miss_count++;
			
			/* insert data into cache */
			if(arr[set_index].size < E){ //if there's space in the set
				//scoot down every block in the set so mru's at index 0 and lru's at index size
				for(int i = arr[set_index].size; i > 0; i--){
					blocks[i] = blocks[i - 1];
				}
				arr[set_index].size++;
										
			}else{ //no space left in the set

				if(blocks[E-1].dirty == 1){ //blocks[E - 1] is lru => will be evicted
					dirty_bytes_active -= (int) pow(2, b);
					dirty_bytes_evicted += (int) pow(2, b);  // B = pow(2,b) = number of bytes per block?
				}

				//scoot down every block in the set to make space for MRU at index 0 and so lru's at the last index 
				for(int i = E - 1; i > 0; i--){
					blocks[i] = blocks[i - 1];
				}
				eviction_count++;

			}
			//move MRU block to the front of the set
			blocks[0].tag = tag;
			blocks[0].dirty = 0;
			blocks[0].valid = 1;
		}
		//finished counting misses from load

  }//end of load function definition



  /*Purpose: executes the store operation
    Arguments: tag = tag of the block that is being searched for in the cache
			   set_index = the index of the set that is being searched in the cache
			   blocks = the array of blocks in the set being searched
  */
  void store(unsigned long tag, int set_index, block_t * blocks){

	  		int hit = 0; // a boolean where	 miss: hit = 0
			     	 	 //					  hit: hit = 1
			
			
			//counts hits from store

			for(int i = 0; i < E; i++){

				if(blocks[i].valid == 1 && blocks[i].tag == tag){

                    hit_count++;

					if(i == 0){	// mru is always the first block in the set
						double_references++;
					}
                    
					block_t copy = blocks[i];

					//scoot down every block by 1 index and put most recently accessed block at index 0
					for(int j = i; j > 0; j--){
						blocks[j] = blocks[j - 1];
					}
					blocks[0] = copy;
                    hit = 1;

					if(blocks[0].dirty != 1){
						blocks[0].dirty = 1;
						dirty_bytes_active += (int) pow(2, b);  // B = pow(2,b) = number of bytes per block?
					}
					
                    break;
                }
            }//finished counting hits from store


			// counts misses from store
            if(!hit){
                miss_count++;
				
				dirty_bytes_active += (int) pow(2, b);  // B = pow(2,b) = number of bytes per block?

				/* insert data into cache */
				if(arr[set_index].size < E){ //if there's space in the set

					//scoot down every block in the set so mru's at index 0 and lru's at index size
					for(int i = arr[set_index].size; i > 0; i--){
						blocks[i] = blocks[i - 1];
					}
				
					arr[set_index].size++;
					 						
				}else{ //no space left in the set

					if(blocks[E - 1].dirty == 1){ 
						dirty_bytes_active -= (int) pow(2, b);
						dirty_bytes_evicted += (int) pow(2, b);  // B = pow(2,b) = number of bytes per block?
					}

					/* evict lru (block at index E-1) */
					for(int i = E - 1; i > 0; i--){
                        blocks[i] = blocks[i - 1];
                    }
                    eviction_count++;

				}
				blocks[0].tag = tag;
				blocks[0].dirty = 1;
				blocks[0].valid = 1;
            }//finished counting misses from store
			
  }//end of store function definition


  //open trace file
  FILE * file = fopen(tracefile, "r");
  
  if(file != NULL){
	
  	char operation;
	unsigned int size;
	unsigned long address;


	while(fscanf(file, "%c %lx, %d", &operation, &address, &size) != EOF){

		unsigned long tag = address >> s >> b;
		int set_index = (address >> b) % num_sets;  
		block_t * blocks = arr[set_index].blocks;


		if(operation == 'M'){

			load(tag, set_index, blocks);
			store(tag, set_index, blocks);

		}else if(operation == 'L'){

			load(tag, set_index, blocks);
			
		}else if(operation == 'S'){

			store(tag, set_index, blocks);

		}
	}

	fclose(file); 

  }else{

	printf("cannot open file");

  }
  
  for(int iter = 0; iter < num_sets; iter++){ 
	  free(arr[iter].blocks);
    
  }
  free(arr);

  printSummary(hit_count, miss_count, eviction_count, dirty_bytes_evicted, dirty_bytes_active, double_references);

  return 0;
}