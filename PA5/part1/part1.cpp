/****************************************************************
 *
 * Author: Monty McConnell
 * Title: CSCE451 PA5 part1.cpp
 * Date: 5/6/2022
 * Description: Virtual Memory Manager assuming infinite memory space
 *
 ****************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

/******************************************************
 * Declarations
 ******************************************************/
// #Define'd sizes
#define FRAMES 256
#define FRAME_SIZE 256
#define TABLE_SIZE 16
#define TABLE_ENTRIES 256
#define PAGE_SIZE 256
#define NOT_FOUND -1
// #define ADDRESS_SIZE 16

FILE * backing_store;
FILE * address_file;
FILE * correct_file;

// Make the TLB array
// Need pages associated with frames (could be 2D array, or C++ list, etc.)
int page_TLB[TABLE_SIZE];
int frame_TLB[TABLE_SIZE];

// Make the Page Table
// Again, need pages associated with frames (could be 2D array, or C++ list, etc.)
int pageTable[TABLE_ENTRIES];

// Make the memory
// Memory array (easiest to have a 2D array of size x frame_size)
int memory[FRAMES][FRAME_SIZE];

// Other Initializations
int frame_num = -1;
signed char backing_value[PAGE_SIZE];
bool TLB_hit = false;
int free_frame = 0;

char address[12];
int logical_address = 0;

// Counters
int page_faults = 0;
int TLB_hits = 0;
int TLB_entries = 0;
int addresses = 0;



/******************************************************
 * Function Declarations
 ******************************************************/

/***********************************************************
 * Function: get_frame_TLB - tries to find the frame number in the TLB
 * Parameters: page_num
 * Return Value: the frame number, else NOT_FOUND if not found
 ***********************************************************/
int get_frame_TLB(int page_num);

/***********************************************************
 * Function: get_frame_pagetable - tries to find the frame in the page table
 * Parameters: page_num
 * Return Value: page number, else NOT_FOUND if not found (page fault)
 ***********************************************************/
int get_frame_pagetable(int page_num);

/***********************************************************
 * Function: update_page_table - update the page table with frame info
 * Parameters: page_num
 * Return Value: none
 ***********************************************************/
void update_page_table(int page_num);

/***********************************************************
 * Function: update_TLB - update TLB (FIFO)
 * Parameters: page_num, frame_num
 * Return Value: none
 ***********************************************************/
void update_TLB(int page_num, int frame_num);

/***********************************************************
 * Function: print_stats - prints stats for grading purposes
 * Parameters: none
 * Return Value: none
 ***********************************************************/
void print_stats();


/******************************************************
 * Assumptions:
 *   If you want your solution to match follow these assumptions
 *   1. In Part 1 it is assumed memory is large enough to accommodate
 *      all frames -> no need for frame replacement
 *   2. Part 1 solution uses FIFO for TLB updates
 *   3. In the solution binaries it is assumed a starting point at frame 0,
 *      subsequently, assign frames sequentially
 *   4. In Part 2 you should use 128 frames in physical memory
 ******************************************************/

int main(int argc, char * argv[]) {
		// argument processing
		// For Part2: read in whether this is FIFO or LRU strategy
		
		backing_store = fopen(argv[1], "rb");
		address_file = fopen(argv[2], "r");
		correct_file = fopen("correct.txt","w+");
		// initialization
		for(int i = 0; i < TABLE_ENTRIES; i++){
			pageTable[i] = -1;
		}
		for(int i = 0; i < TABLE_SIZE; i++){
			page_TLB[i] = -1;
			frame_TLB[i] = -1;
		}
        // read addresses.txt
		while(fgets(address, 12, address_file) != NULL) {
				// pull addresses out of the file
			logical_address = atoi(address);
			
			
			frame_num = NOT_FOUND;
			TLB_hit = false;
				// Step 0:
				// get page number and offset
				//   bit twiddling
			int offset = logical_address & 0xFF;
			int page_num = logical_address >> 8;

				// need to get the physical address (frame + offset):
				// Step 1: check in TLB for frame
				//   if !get_frame_TLB() -> :(

			get_frame_TLB(page_num);
			
				//     Step 2: not in TLB, look in page table
			if(frame_num == NOT_FOUND){
				get_frame_pagetable(page_num);
			}
				//     if !get_frame_pagetable() -> :(
				//       PAGE_FAULT!
				//       Step 3:
				//       dig up frame in BACKING_STORE.bin (backing_store_to_memory())
				//       bring in frame page# x 256
				//       store in physical memory
				//       Step 4:
				//       update page table with corresponding frame from storing
				//         into physical memory
			if(frame_num == NOT_FOUND){
				fseek(backing_store, page_num * PAGE_SIZE, SEEK_SET);
				fread(backing_value, sizeof(signed char), PAGE_SIZE, backing_store);
				for(int i = 0; i < PAGE_SIZE; i++) {
					memory[free_frame][i] = backing_value[i];
				}
				update_page_table(page_num);
				
			}
				//   Step 5: (always) update TLB when we find the frame  
				//     update TLB (updateTLB())
			if(TLB_hit == false) {
				update_TLB(page_num,frame_num);
			}
				//   Step 6: read val from physical memory
			fprintf(correct_file,"Virtual address: %d Physical address: %d Value: %d\n", logical_address, (frame_num << 8) | offset, memory[frame_num][offset]);
			addresses++;
		}

		// output useful information for grading purposes
		print_stats();

		fclose(backing_store);
		fclose(address_file);
}

void print_stats(){
	double fault_rate = (double)page_faults / (double)addresses;
	double hit_rate = (double)TLB_hits / (double)addresses;
	fprintf(correct_file, "Number of Translated Addresses = %d\n", addresses);
	fprintf(correct_file, "Page Faults = %d\n", page_faults);
	fprintf(correct_file, "Page Fault Rate = %.3f\n", fault_rate);
	fprintf(correct_file, "TLB Hits = %d\n", TLB_hits);
	fprintf(correct_file, "TLB Hit Rate = %.3f\n", hit_rate);
}

int get_frame_TLB(int page_num){
	for(int i = 0; i < TABLE_SIZE; i++){
		if(page_TLB[i] == page_num){
			frame_num = frame_TLB[i];
			TLB_hits++;
			TLB_hit = true;
			break;
		}

	}
	return frame_num;
}

int get_frame_pagetable(int page_num){
	for(int i = 0; i <= page_num; i++){
		if(page_num == i) {
			frame_num = pageTable[i];
			return page_num;
		}
		
	}
	return NOT_FOUND;
}

void update_page_table(int page_num){
	pageTable[page_num] = free_frame;
	free_frame++;
	page_faults++;
	frame_num = free_frame - 1;
}

void update_TLB(int page_num, int frame_num){
	for(int i = 0; i < TLB_entries; i++) {
		if(page_TLB[i] == page_num) {
			break;
		}
	}
	if(TLB_entries >= TABLE_SIZE){
		TLB_entries = 0;
	}
	page_TLB[TLB_entries] = page_num;
	frame_TLB[TLB_entries] = frame_num;
	TLB_entries++;
}