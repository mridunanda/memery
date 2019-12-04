#include "cd2mem.h"
#include <memory.h>
#include <stdlib.h>
#include <iostream>
#define PTR_SIZE 8
using namespace std;

extern uintptr_t starting_addr;
extern uintptr_t ending_addr;
extern char* memblock;


uintptr_t ascii_hex_to_ptr(char* hexstring){
    return (uintptr_t)strtol(hexstring, NULL, 0);
}

unsigned char read_byte(uintptr_t addr){
    return *(memblock - (char*)starting_addr + (char*)addr);
}

unsigned int read_int(uintptr_t addr){
    return *((unsigned int*)memblock - (unsigned int*)starting_addr + (unsigned int*)addr);
}

uintptr_t heap_start(){
    return starting_addr;
}

uintptr_t heap_end(){
    return ending_addr;
}

void set_heap_start(uintptr_t addr){
    starting_addr = addr;
}

void set_heap_end(uintptr_t addr){
    ending_addr = addr;
}

uintptr_t to_addr(uintptr_t addr) {
	FILE *fptr = fopen("temp1.txt", "a");
	uintptr_t potential_addr = addr - (uintptr_t) starting_addr;
	fprintf(fptr, "Addr: %lu\n", potential_addr);
	fclose(fptr);
	return potential_addr / 8;
}
	
// will grab the next 8 bytes of mem block
uintptr_t get_val(uintptr_t rel_start){
	FILE *fptr = fopen("temp.txt", "a");
	char res[19];
	memset(res, 0, 19);
	sprintf(res + strlen(res), "%s", "0x");
	
	for(int i = 7; i>=0; i--){ 	
		char temp[3];
		memset(temp, 0, 3);
		sprintf(temp + strlen(temp), "%hhx", *(memblock+rel_start+i));
		if(strlen(temp) == 1){
			sprintf(res + strlen(res), "%s", "0");
		}
		sprintf(res + strlen(res), "%hhx", *(memblock+rel_start+i));
	}
	fprintf(fptr, "Value: %s\n", res);
	fclose(fptr);
	uintptr_t lu_res = strtoul(res, NULL, 16);
	return lu_res;
}

void reset_seeloop(struct mem_ptr* p_arr, uintptr_t index, unsigned int offset) {
	while (p_arr[index].type == 1 && p_arr[index].seeloop) {
		p_arr[index].seeloop = 0;
		index = p_arr[index].addr + offset;
	}
}

int find_chain_len(struct mem_ptr* p_arr, uintptr_t index, unsigned int offset, struct mem_struct **pre_ds) {
	int depth = 0;
	uintptr_t reset_index = index;

    // *ds stores second return value --- pre-existing data structure
    *pre_ds = NULL;

	// Count how many pointers we can chase (i.e. nodes in the data structure)
	while (p_arr[index].type == 1) { 
		// we have encountered a previously seen node on current iteration (indicating some sort of loop in the data structure)
		if (p_arr[index].seeloop == 1) {
			//cout << "FOUND A LOOP" << endl;
			break;
		} else if (p_arr[index].ds) { // we have encountered a pre-existing data structure
            *pre_ds = p_arr[index].ds;
            break;
        }
		p_arr[index].seeloop = 1;
		//cout << "CURRENT INDEX: " << index << endl;
        index = p_arr[index].addr + offset; // we should find a pointer at the pointed-to address plus offset
		//cout << "NEXT INDEX: " << index << " TYPE: " << p_arr[index].type << endl;
        depth++;
    }
	reset_seeloop(p_arr, reset_index, offset);
	return depth;
}

void assign_chain_ds(struct mem_ptr* p_arr, uintptr_t index, unsigned int offset, struct mem_struct* ds) {
	uintptr_t reset_index = index;
	while (p_arr[index].type == 1) { 
		if (p_arr[index].seeloop == 1 || p_arr[index].ds) {
			break;
		}
		p_arr[index].seeloop = 1;
		p_arr[index].ds = ds;
        index = p_arr[index].addr + offset;
    }
	reset_seeloop(p_arr, reset_index, offset);
}	

void assign_root(struct mem_ptr* p_arr, uintptr_t index) {
	// update per struct property
	p_arr[index].isroot = 1;
	// update per ds property
	cout << "ASSIGN ROOT FUNC" << endl;
	// SEG FAULTING HERE
	p_arr[index].ds->roots->push_back(index);
}

void upgrade_root(struct mem_ptr* p_arr, uintptr_t index, uintptr_t pointing_to_node) {
	cout << "UPGRADE ROOT FUNC" << endl;
	// update per struct property
	p_arr[pointing_to_node].isroot = 0;
	p_arr[index].isroot = 1;
	// update per ds property
	// SEG FAULTING HERE 
	p_arr[index].ds->roots->remove(pointing_to_node);
	p_arr[index].ds->roots->push_back(index);
}
